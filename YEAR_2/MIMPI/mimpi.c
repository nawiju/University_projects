/**
* This file is for implementation of MIMPI library.
* */

#include "channel.h"
#include "mimpi.h"
#include "mimpi_common.h"
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>

#define PACKET_SIZE 4000

#define KILL_TAG -69 // kill thread

#define MESSAGE_TAG -70 // message from other process that matches search criteria
#define SPURIOUS_TAG -71 // spurios wakeup
#define FINALIZE_TAG -72 // we are finalizing
#define REMOTE_FINISHED_TAG -73 // other process has finished while we were waiting for message

#define BARRIER_SUCCESS_TAG -420 // barrier tag, used for barrier success
#define BARRIER_FAILED_TAG -421 // barrier tag, used for barrier failure

#define BROADCAST_SUCCESS_TAG -422 // broadcast tag, used for broadcast success]
#define BROADCAST_FAILED_TAG -423 // broadcast tag, used for broadcast failure

#define REDUCE_SUCCESS_TAG -424 // reduce tag, used for reduce success
#define REDUCE_FAILED_TAG -425 // reduce tag, used for reduce failure

struct message_list {
   struct message_list *next;

   void * chunk;
   int chunk_size;
} message_list;

struct request_list {
   struct request_list *next;
   struct message_list *messages;
   struct request_list *tail;

   int tag;
   int count;
} request_list;

struct data_packet {
   int count;
   int tag;
   int chunks;
} data_packet;

pthread_mutex_t *data_mutex; // access the list and other variables by the process or any thread
pthread_cond_t *wait_on_data; // process waits on desired data after checking it does not exist
pthread_t threads[16];
pthread_t group_threads[16];

volatile bool *wait_on_data_flag; // data about the requested message
volatile int *requested_tag;
volatile int *requested_count;
volatile int *requested_source;
volatile int *status;

int *senders[16];

volatile bool alive[16];

struct request_list *messages[16];

void reset_variables() {
    *wait_on_data_flag = false;
    *requested_tag = -1;
    *requested_count = -1;
    *requested_source = -1;
}

// Kill the thread that receives messages from the process with index.
// TODO: add variable to make sure that the thread is stopped rather than checking the list
void kill_thread(int index) {
    //   printf("Killing thread %d\n", index);
    ASSERT_ZERO(pthread_mutex_lock(data_mutex));
    alive[index] = false;

    if (*wait_on_data_flag && *requested_source == index) {
        reset_variables();
        *status = FINALIZE_TAG;
        ASSERT_ZERO(pthread_cond_signal(wait_on_data));
    }

    ASSERT_ZERO(pthread_mutex_unlock(data_mutex));
}


struct message_list* assemble_message(int count, int chunks, int sender) {
    struct message_list *message = malloc(sizeof(struct message_list));
    message->next = NULL;
    message->chunk = malloc(sizeof(PACKET_SIZE));
    message->chunk_size = 100;

    struct message_list *head = message;
    struct message_list *tail = message;

    for (int i = 0; i < chunks; i++)  {
        int bytes_to_read = count - i * PACKET_SIZE >= PACKET_SIZE
                            ? PACKET_SIZE : count - PACKET_SIZE * i;

        struct message_list *to_add = malloc(sizeof(struct message_list));
        to_add->next = NULL;
        to_add->chunk = malloc(bytes_to_read);
        to_add->chunk_size = bytes_to_read;

        ssize_t read = chrecv(sender, to_add->chunk, bytes_to_read);
        ASSERT_SYS_OK(read);

        tail->next = to_add;
        tail = tail->next;
    } 

    struct message_list *new_head = head->next;
    free(head->chunk);
    free(head);

    return new_head;
}

// Thread function for the receival of messages from other processes.
void * receive_messages(void * arg) {
    int index = *((int*) arg);
    int sender = index + 20 + MIMPI_World_rank() * 16;

    while (true) {
        void * buffer = malloc(sizeof(struct data_packet));

        ssize_t info = chrecv(sender, buffer, sizeof(struct data_packet));

        ASSERT_SYS_OK(info);

        struct data_packet *packet = buffer;

        int chunks = packet->chunks;
        int tag = packet->tag;
        int count = packet->count;

        if (tag == KILL_TAG)  { // killing the thread
            free(buffer);
            kill_thread(index);
            break;
        } else if (tag <= -1 && tag >= -16) { // killing the process
            free(buffer);
            ASSERT_ZERO(pthread_mutex_lock(data_mutex));
            alive[index] = false;

            if (*wait_on_data_flag && *requested_source == index) {
                reset_variables();
                *status = REMOTE_FINISHED_TAG;
                ASSERT_ZERO(pthread_cond_signal(wait_on_data));
            }
            ASSERT_ZERO(pthread_mutex_unlock(data_mutex));
            break;
        }

        free(buffer);

        struct message_list *message = malloc(sizeof(struct message_list));
        message->next = NULL;
        message->chunk = malloc(sizeof(PACKET_SIZE));
        message->chunk_size = 100;

        struct message_list *head = message;
        struct message_list *tail = message;

        for (int i = 0; i < chunks; i++)  {
            int bytes_to_read = count - i * PACKET_SIZE >= PACKET_SIZE
                                ? PACKET_SIZE : count - PACKET_SIZE * i;

            struct message_list *to_add = malloc(sizeof(struct message_list));
            to_add->next = NULL;
            to_add->chunk = malloc(bytes_to_read);
            to_add->chunk_size = bytes_to_read;

            ssize_t read = chrecv(sender, to_add->chunk, bytes_to_read);
            ASSERT_SYS_OK(read);

            tail->next = to_add;
            tail = tail->next;
        } 

        struct message_list *new_head = head->next;
        free(head->chunk);
        free(head);


      //  struct message_list *new_head = assemble_message(count, chunks, sender);
        struct request_list *new_request = malloc(sizeof(struct request_list));
        
        new_request->next = NULL;
        new_request->messages = new_head;
        new_request->tag = tag;
        new_request->count = count;
        new_request->tail = new_request;

        ASSERT_ZERO(pthread_mutex_lock(data_mutex));

        messages[index]->tail->next = new_request;
        messages[index]->tail = messages[index]->tail->next;

        if (*wait_on_data_flag && tag == *requested_tag && *requested_count == count
            && *requested_source == index) {
            reset_variables();
            *status = MESSAGE_TAG;
            ASSERT_ZERO(pthread_cond_signal(wait_on_data));
        }
        
        ASSERT_ZERO(pthread_mutex_unlock(data_mutex));     
    }

    return (void *) 0;
}

// Prepare environment for MIMPI. Declare and initialize all variables and mutexes.
void prepare_env(int size) {
    wait_on_data_flag = malloc(sizeof(bool));
    requested_tag = malloc(sizeof(int));
    requested_count = malloc(sizeof(int));
    requested_source = malloc(sizeof(int));
    data_mutex = malloc(sizeof(pthread_mutex_t));
    wait_on_data = malloc(sizeof(pthread_cond_t));
    status = malloc(sizeof(int));

    *status = SPURIOUS_TAG;

    reset_variables();
    
    for (int i = 0; i < 16; i++)
        if (i >= size) alive[i] = false;
        else alive[i] = true;

    ASSERT_ZERO(pthread_mutex_init(data_mutex, NULL));
    ASSERT_ZERO(pthread_cond_init(wait_on_data, NULL));
}

// Create a thread that will be responsible for the receival of messages.
// Each thread receives messages from one process.
void create_thread(int i) {
    pthread_t thread;

    senders[i] = malloc(sizeof(int));
    *senders[i] = i;

    ASSERT_ZERO(pthread_create(&thread, NULL, receive_messages, senders[i]));
    threads[i] = thread;

    messages[i] = malloc(sizeof(struct request_list));
    messages[i]->next = NULL;
    messages[i]->messages = NULL;
    messages[i]->tag = -1;
    messages[i]->count = -1;
    messages[i]->tail = messages[i];
}

// Initialize MIMPI. Prepare environment and create threads.
void MIMPI_Init(bool enable_deadlock_detection) {
    channels_init();

    int rank = MIMPI_World_rank();
    int size = MIMPI_World_size();

    prepare_env(size);
    
    for (int i = 0; i < size; i++) {
        if (i == rank) continue;
        create_thread(i);
    }
}

// Send kill signals to all threads.
void send_kill_signals(int size, int rank) {
    for (int i = 0; i < size; i++) {
       if (i == rank) continue;

       struct data_packet *data = malloc(sizeof(struct data_packet));
       data->tag = KILL_TAG;
       data->count = KILL_TAG;
       data->chunks = KILL_TAG;

       ssize_t sent = chsend(300 + rank + 16 * i, data, sizeof(struct data_packet));
       free(data);
       ASSERT_SYS_OK(sent);

       struct data_packet *data2 = malloc(sizeof(struct data_packet));
       data2->tag = -1 * rank - 1;
       data2->count = -1 * rank - 1;
       data2->chunks = -1 * rank - 1;

       ssize_t sent2 = chsend(300 + i + 16 * rank, data2, sizeof(struct data_packet));
       free(data2);
       ASSERT_SYS_OK(sent2);

       if (sent != sizeof(struct data_packet)) fatal("Too little sent");
       if (sent2 != sizeof(struct data_packet)) fatal("Too little sent");
   }
}

// Clean environment. Destroy mutexes and free memory.
void clean_env(int size, int rank) {
    ASSERT_ZERO(pthread_mutex_destroy(data_mutex));
    ASSERT_ZERO(pthread_cond_destroy(wait_on_data));

    free((void*)wait_on_data_flag);
    free((void*)requested_tag);
    free((void*)requested_count);
    free((void*)requested_source);
    free((void*)data_mutex);
    free((void*)wait_on_data);
    free((void*)status);


    for (int i = 0; i < size; i++) {
        if (i == rank) continue;

        struct request_list *head = messages[i];

        while (head != NULL) {

            struct message_list *to_freee = head->messages;

            while (to_freee != NULL) {
                struct message_list *to_free2 = to_freee;
                to_freee = to_freee->next;
                free(to_free2->chunk);
                free(to_free2);
            }

            struct request_list *to_free = head;
            head = head->next;
            free(to_free);
        }

        free(senders[i]);
    }

    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            ASSERT_SYS_OK(close(20 + i * 16 + j));
            ASSERT_SYS_OK(close(300 + i * 16 + j));
        }
    }

    for (int i = 0; i < size; i++) {
        for (int j = 1; j < 9; j++) {
            ASSERT_SYS_OK(close(560 + i * 8 + j));
            ASSERT_SYS_OK(close(830 + i * 8 + j));
        }
    }
}

// TODO: send kill signals to all processes and manage synchronization at finalize
void MIMPI_Finalize() {
   // printf("Finalizing rank %d\n", MIMPI_World_rank());
    int size = MIMPI_World_size();
    int rank = MIMPI_World_rank();

    ssize_t sent = 0;
    sent++;
    int * message_to_parent = malloc(sizeof(int));
    *message_to_parent = KILL_TAG;

    ASSERT_ZERO(pthread_mutex_lock(data_mutex));
    
    if (alive[(rank - 1) / 2]) {
        if (rank % 2 == 0) sent = chsend(560 + ((rank - 1 )/ 2) * 8 + 4, message_to_parent, sizeof(int));
        else sent = chsend(560 + ((rank - 1) / 2) * 8 + 2, message_to_parent, sizeof(int));
    }

    if (rank * 2 + 1 < size && alive[rank * 2 + 1]) {
        sent = chsend(560 + rank * 8 + 6, message_to_parent, sizeof(int));
    }

    if (rank * 2 + 2 < size && alive[rank * 2 + 2]) {
        sent = chsend(560 + rank * 8 + 8, message_to_parent, sizeof(int));
    }

    free(message_to_parent);

    ASSERT_ZERO(pthread_mutex_unlock(data_mutex));

    send_kill_signals(size, rank);

    for (int i = 0; i < size; i++) {
        if (i == rank) continue;
        ASSERT_ZERO(pthread_join(threads[i], NULL));
    }

    clean_env(size, rank);

    channels_finalize();
}


int MIMPI_World_size() {
    char *cnt = getenv("world_count");
    if (cnt == NULL) fatal("No world count");
    return cnt ? atoi(cnt) : 1;
}


int MIMPI_World_rank() {
    char *rank = getenv("world_rank");
    if (rank == NULL) fatal("No world rank");
    return rank ? atoi(rank) : 0;
}


MIMPI_Retcode MIMPI_Send(
    void const *data,
    int count,
    int destination,
    int tag
) {
    if (destination == MIMPI_World_rank()) return MIMPI_ERROR_ATTEMPTED_SELF_OP;
    if (destination >= MIMPI_World_size() || destination < 0) return MIMPI_ERROR_NO_SUCH_RANK;

    ASSERT_ZERO(pthread_mutex_lock(data_mutex));
    if (!alive[destination]) {
        ASSERT_ZERO(pthread_mutex_unlock(data_mutex));
        return MIMPI_ERROR_REMOTE_FINISHED;
    }
    ASSERT_ZERO(pthread_mutex_unlock(data_mutex));

    int chunks = count / PACKET_SIZE + (count % PACKET_SIZE != 0);

    struct data_packet *data_info = malloc(sizeof(struct data_packet));
    data_info->count = count;
    data_info->tag = tag;
    data_info->chunks = chunks;

    ssize_t sent = chsend(300 + destination * 16 + MIMPI_World_rank(), data_info, sizeof(struct data_packet)); 
    ASSERT_SYS_OK(sent);
    free(data_info);

    if (sent != sizeof(struct data_packet)) fatal("Too little sent");
    
    int offset = 0;
    int chunk_size = count > PACKET_SIZE ? PACKET_SIZE : count;

    for (int i = 0; i < chunks; i++) {
        void * chunk = malloc(chunk_size);
        memcpy(chunk, data + offset, chunk_size);

        ssize_t sent = chsend(300 + destination * 16 + MIMPI_World_rank(), chunk, chunk_size);
        ASSERT_SYS_OK(sent);

        free(chunk);
        
        if (sent == -1 && errno == EPIPE) {
            ASSERT_ZERO(pthread_mutex_lock(data_mutex));
            alive[destination] = false;
            ASSERT_ZERO(pthread_mutex_unlock(data_mutex));
            return MIMPI_ERROR_REMOTE_FINISHED;
        }

        offset += PACKET_SIZE;
        chunk_size = count - offset > PACKET_SIZE ? PACKET_SIZE : count - offset;
    }

    return MIMPI_SUCCESS;
}


struct message_list* find_message(int count, int tag, int source, bool *found,
                                  struct request_list **prev, struct request_list **request) {

    struct message_list *message = NULL;

    while (*request != NULL && !(*found)) {
        if (((*request)->tag == tag || tag == MIMPI_ANY_TAG) && (*request)->count == count) {
            *found = true;
            message = (*request)->messages;
            (*prev)->next = (*request)->next;

            if ((*request)->next == NULL) messages[source]->tail = *prev;

            break;
        }
        
        *prev = *request;
        *request = (*request)->next;
    }

    return message;
}

MIMPI_Retcode MIMPI_Recv(
    void *data,
    int count,
    int source,
    int tag
) {
    if (source == MIMPI_World_rank()) return MIMPI_ERROR_ATTEMPTED_SELF_OP;
    if (source >= MIMPI_World_size() || source < 0) return MIMPI_ERROR_NO_SUCH_RANK;

    // printf("Receiving from %d rank: %d\n", source, MIMPI_World_rank());
    
    ASSERT_ZERO(pthread_mutex_lock(data_mutex));

    struct request_list *request = messages[source]->next;
    struct request_list *prev = messages[source];

    bool found = false;
    struct message_list *message = find_message(count, tag, source, &found, &prev, &request);

    if (!found) {
        *wait_on_data_flag = true;
        *requested_tag = tag;
        *requested_count = count;
        *requested_source = source;
        *status = SPURIOUS_TAG;

        if (!alive[source]) {
            ASSERT_ZERO(pthread_mutex_unlock(data_mutex));
            return MIMPI_ERROR_REMOTE_FINISHED;
        }

        while (*wait_on_data_flag) {
            ASSERT_ZERO(pthread_cond_wait(wait_on_data, data_mutex));

            // printf("Woke up %d tag %d\n", MIMPI_World_rank(), *status);

            if (*status == REMOTE_FINISHED_TAG) {
                request = messages[source]->next;
                prev = messages[source];

                message = find_message(count, tag, source, &found, &prev, &request);

                if (!found) {
                    ASSERT_ZERO(pthread_mutex_unlock(data_mutex));
                    return MIMPI_ERROR_REMOTE_FINISHED;
                }

                struct message_list *head = message;

                int offset = 0;

                while (head != NULL) {
                    memcpy(data + offset, head->chunk, head->chunk_size);   
                    offset += head->chunk_size;
                    struct message_list *to_free = head;
                    head = head->next;
                    free(to_free->chunk);
                    free(to_free);
                }

                free(request);
                
                ASSERT_ZERO(pthread_mutex_unlock(data_mutex));
                return MIMPI_SUCCESS;
            } else if (*status == FINALIZE_TAG) {
                request = messages[source]->next;
                prev = messages[source];

                message = find_message(count, tag, source, &found, &prev, &request);

                if (!found) {
                    ASSERT_ZERO(pthread_mutex_unlock(data_mutex));
                    return MIMPI_ERROR_REMOTE_FINISHED;
                }

                struct message_list *head = message;

                int offset = 0;

                while (head != NULL) {
                    memcpy(data + offset, head->chunk, head->chunk_size);   
                    offset += head->chunk_size;
                    struct message_list *to_free = head;
                    head = head->next;
                    free(to_free->chunk);
                    free(to_free);
                }

                free(request);

                ASSERT_ZERO(pthread_mutex_unlock(data_mutex));
                return MIMPI_SUCCESS;
            }

            *status = SPURIOUS_TAG;
        }

        request = messages[source]->next;
        prev = messages[source];

        message = find_message(count, tag, source, &found, &prev, &request);
    }

    if (!found) {
        return MIMPI_ERROR_REMOTE_FINISHED;
    }// unsure

    struct message_list *head = message;

    int offset = 0;

    while (head != NULL) {
        memcpy(data + offset, head->chunk, head->chunk_size);   
        offset += head->chunk_size;
        struct message_list *to_free = head;
        head = head->next;
        free(to_free->chunk);
        free(to_free);
    }

    free(request);

    ASSERT_ZERO(pthread_mutex_unlock(data_mutex));

    return MIMPI_SUCCESS;
}

MIMPI_Retcode MIMPI_Barrier() {
    int sum = 1;
    int rank = MIMPI_World_rank();
    int size = MIMPI_World_size();
    int child1 = rank * 2 + 1;
    int child2 = rank * 2 + 2;

    // wait for childrens' signal


    bool child1_alive = true;
    bool child2_alive = true;

    if (child1 < size) {
        ASSERT_ZERO(pthread_mutex_lock(data_mutex));
        if (!alive[child1]) {
            child1_alive = false;
            ASSERT_ZERO(pthread_mutex_unlock(data_mutex));
        } else {
            ASSERT_ZERO(pthread_mutex_unlock(data_mutex));
            int *message = malloc(sizeof(int));
            ssize_t read = chrecv(560 + rank * 8 + 1, message, sizeof(int));

            if (*message == KILL_TAG) {
                child1_alive = false;
                free(message);
            } else if (read == 0) child1_alive = false;
            else {
                ASSERT_SYS_OK(read);
                sum += *message;
                free(message);
            }
        }
    }

    if (child2 < size) {
        ASSERT_ZERO(pthread_mutex_lock(data_mutex));
        if (!alive[child2]) {
            child2_alive = false;
            ASSERT_ZERO(pthread_mutex_unlock(data_mutex));
        } else {
            ASSERT_ZERO(pthread_mutex_unlock(data_mutex));

            int *message = malloc(sizeof(int));
            ssize_t read = chrecv(560 + rank * 8 + 3, message, sizeof(int));
        
            if (*message == KILL_TAG) {
                child1_alive = false;
                free(message);
            } else if (read == 0) child2_alive = false;
            else {
                ASSERT_SYS_OK(read);
                sum += *message;
                free(message);
            }
        }
    }

    // signal the parent
    int *message = malloc(sizeof(int));
    ssize_t read;
    bool parent_alive = true;
    *message = child2_alive && child1_alive ? BARRIER_SUCCESS_TAG : BARRIER_FAILED_TAG;

    if (rank != 0) {
        ASSERT_ZERO(pthread_mutex_lock(data_mutex));
        if (!alive[(rank - 1) / 2]) {
            ASSERT_ZERO(pthread_mutex_unlock(data_mutex));
            parent_alive = false;
        } else {
            ASSERT_ZERO(pthread_mutex_unlock(data_mutex));

            int *message_to_parent = malloc(sizeof(int));
            *message_to_parent = sum;
            ssize_t sent;

            if (rank % 2 == 0) sent = chsend(560 + ((rank - 1 )/ 2) * 8 + 4, message_to_parent, sizeof(int));
            else sent = chsend(560 + ((rank - 1) / 2) * 8 + 2, message_to_parent, sizeof(int));

            if (sent == -1 && errno == EPIPE) {
                *message = BARRIER_FAILED_TAG;
                parent_alive = false;
            } else ASSERT_SYS_OK(sent);

            free(message_to_parent);
        }
    }

    // wait for parent's signal


    if (rank != 0 && parent_alive) {
        if (rank % 2 == 0) read = chrecv(560 + ((rank - 1) / 2) * 8 + 7, message, sizeof(int));
        else read = chrecv(560 + ((rank - 1) / 2) * 8 + 5, message, sizeof(int));

        if (*message == KILL_TAG) {
            parent_alive = false;
            *message = BARRIER_FAILED_TAG;
        } else if (read == 0) parent_alive = false;
        else {
            ASSERT_SYS_OK(read);
        }
    } else {
        *message = BARRIER_FAILED_TAG;
    }

    // signal the children


    if (rank == 0) {
        if (sum != MIMPI_World_size()) *message = BARRIER_FAILED_TAG;
        else *message = BARRIER_SUCCESS_TAG;
    }
    
    if (child1 < size && child1_alive) {
        ssize_t sent = chsend(560 + rank * 8 + 6, message, sizeof(int));
        ASSERT_SYS_OK(sent);
    }

    if (child2 < size && child2_alive) {
        ssize_t sent = chsend(560 + rank * 8 + 8, message, sizeof(int));
        ASSERT_SYS_OK(sent);
    }

    if (*message == BARRIER_FAILED_TAG)  {
        free(message);
        return MIMPI_ERROR_REMOTE_FINISHED;
    } else {
        free(message);
        return MIMPI_SUCCESS;
    }
}


MIMPI_Retcode MIMPI_Bcast(
 void *data,
 int count,
 int root
) {
    int sum = 1;
    int rank = MIMPI_World_rank();
    int size = MIMPI_World_size();

    int child1 = rank >= root ? (rank * 2 + 1 - root) % size : (size - root + rank) * 2 - (size - root) + 1;
    int child2 = rank >= root ? (rank * 2 + 2 - root) % size : (size - root + rank) * 2 - (size - root) + 2;
    int parent = rank > root ? ((rank + root - 1) / 2) % size : ((rank + root + size - 1) / 2) % size;

    bool left_child = rank > root ? (rank - root) % 2 == 1 : ((size - root) + rank ) % 2 == 1;

    bool child1_alive = ((rank <= root && child1 < root && child1 > rank) || (rank >= root &&
                ((child1 > root && child1 < size && child1 > rank) || child1 < root))) && child1 != rank;

    bool child2_alive = ((rank <= root && child2 < root && child1 > rank) || (rank >= root &&
                ((child2 > root && child2 < size && child2 > rank) || child2 < root))) && child2 != rank;

    // wait for children


    if (child1_alive) {
        int *message = malloc(sizeof(int));
        ssize_t read = chrecv(830 + rank * 8 + 1, message, sizeof(int));
        ASSERT_SYS_OK(read);
        sum += *message;
        free(message);
    }

    if (child2_alive) {
        int *message = malloc(sizeof(int));
        ssize_t read = chrecv(830 + rank * 8 + 3, message, sizeof(int));
        ASSERT_SYS_OK(read);
        sum += *message;
        free(message);
    }

    // signal to parent


    bool parent_alive = true;

    if (rank != root && parent_alive && parent < size) {
        int *message = malloc(sizeof(int));
        *message = sum;
        ssize_t sent;

        if (left_child) sent = chsend(830 + parent * 8 + 2, message, sizeof(int));
        else sent = chsend(830 + parent * 8 + 4, message, sizeof(int));      

        ASSERT_SYS_OK(sent);
        free(message);

        if (sent == -1 && errno == EPIPE) parent_alive = false;
    }

    // wait for parent's signal
    // send message to children and pass it on while saving it


    int size_to_read = sizeof(struct data_packet) + count < PACKET_SIZE ? sizeof(struct data_packet) + count : PACKET_SIZE;

    void * packet = malloc(size_to_read);

    if (rank != root && parent_alive && parent < size) {
        ssize_t read;
        void * buffer = malloc(size_to_read);

        if (left_child) read = chrecv(830 + parent * 8 + 5, buffer, size_to_read);
        else read = chrecv(830 + parent * 8 + 7, buffer, size_to_read);

        ASSERT_SYS_OK(read);

        struct data_packet *data_info = (struct data_packet*)buffer;
        int tag = data_info->tag;
        int count = data_info->count;
        int chunks = data_info->chunks;

        memcpy(packet, buffer, size_to_read);

        if (tag == BROADCAST_SUCCESS_TAG) {
            int read_count = PACKET_SIZE - sizeof(struct data_packet) > count ? count : PACKET_SIZE - sizeof(struct data_packet);

            memcpy(data, buffer + sizeof(struct data_packet), read_count);

            int offset = read_count;
            count -= read_count;

            for (int i = 0; i < chunks; i++) {
                int bytes_to_read = count - i * PACKET_SIZE >= PACKET_SIZE
                                    ? PACKET_SIZE : count - PACKET_SIZE * i;

                void * chunk = malloc(bytes_to_read);

                if (left_child) read = chrecv(830 + rank * 8 + 5, chunk, bytes_to_read);
                else read = chrecv(830 + rank * 8 + 7, chunk, bytes_to_read);

                ASSERT_SYS_OK(read);

                memcpy(data + offset, chunk, bytes_to_read);
                offset += bytes_to_read;
                free(chunk);
            }

            free(buffer);
        } else if (rank != root && !parent_alive) {
            struct data_packet *data_info = (struct data_packet*)packet;
            data_info->tag = BROADCAST_FAILED_TAG;
            data_info->count = 0;
            data_info->chunks = 0;
        }
    }

    // signal to children


    int write_count = 0;
    int chunks = 0;

    if (rank == root) {
        struct data_packet *data_info = (struct data_packet*)packet;
        if (sum == MIMPI_World_size()) {
            data_info->tag = BROADCAST_SUCCESS_TAG;
            data_info->count = count;

            write_count = PACKET_SIZE - sizeof(struct data_packet) > count ? count : PACKET_SIZE - sizeof(struct data_packet);
            chunks = (count - write_count) / PACKET_SIZE + ((count - write_count) % PACKET_SIZE != 0);

            data_info->chunks = chunks;
            memcpy(packet + sizeof(struct data_packet), data, write_count);
        } else {
            data_info->tag = BROADCAST_FAILED_TAG;
            data_info->count = 0;
            data_info->chunks = 0;
        }
    }

    if (child1_alive) {
        ssize_t sent = chsend(830 + rank * 8 + 6, packet, size_to_read);
        ASSERT_SYS_OK(sent);
    }

    if (child2_alive) {
        ssize_t sent = chsend(830 + rank * 8 + 8, packet, size_to_read);
        ASSERT_SYS_OK(sent);
    }

    if (*(int*)(packet + sizeof(int)) == BROADCAST_FAILED_TAG) {
        free(packet);
        return MIMPI_ERROR_REMOTE_FINISHED;
    } else {
        for (int i = 0; i < chunks; i++) {
            int bytes_to_read = count - i * PACKET_SIZE - write_count >= PACKET_SIZE
                                ? PACKET_SIZE : count - PACKET_SIZE * i - write_count;

            void *chunk = malloc(bytes_to_read);
            memcpy(chunk, data + write_count + i * PACKET_SIZE, bytes_to_read);

            if (child1_alive) {
                ssize_t sent = chsend(830 + rank * 8 + 6, chunk, bytes_to_read);
                ASSERT_SYS_OK(sent);
            }

            if (child2_alive) {
                ssize_t sent = chsend(830 + rank * 8 + 8, chunk, bytes_to_read);
                ASSERT_SYS_OK(sent);
            }
            free(chunk);
        }

        free(packet);
        return MIMPI_SUCCESS;
    }

    return MIMPI_SUCCESS;
}


MIMPI_Retcode MIMPI_Reduce(
    void const *send_data,
    void *recv_data,
    int count,
    MIMPI_Op op,
    int root
) {
    int sum = 1;
    int rank = MIMPI_World_rank();
    int size = MIMPI_World_size();

    int child1 = rank >= root ? (rank * 2 + 1 - root) % size : (size - root + rank) * 2 - (size - root) + 1;
    int child2 = rank >= root ? (rank * 2 + 2 - root) % size : (size - root + rank) * 2 - (size - root) + 2;
    int parent = rank > root ? ((rank + root - 1) / 2) % size : ((rank + root + size - 1) / 2) % size;

    bool left_child = rank > root ? (rank - root) % 2 == 1 : ((size - root) + rank ) % 2 == 1;

    bool child1_alive = ((rank <= root && child1 < root && child1 > rank) || (rank >= root &&
                ((child1 > root && child1 < size && child1 > rank) || child1 < root))) && child1 != rank;

    bool child2_alive = ((rank <= root && child2 < root && child1 > rank) || (rank >= root &&
                ((child2 > root && child2 < size && child2 > rank) || child2 < root))) && child2 != rank;

    // wait for children

    int read_count = PACKET_SIZE - sizeof(struct data_packet) > count ? count : PACKET_SIZE - sizeof(struct data_packet);

    void * new_send_data = malloc(count);
    memcpy(new_send_data, send_data, count);

    if (child1_alive) {
        void * recv_buffer = malloc(sizeof(struct data_packet) + read_count);
        ssize_t read = chrecv(830 + rank * 8 + 1, recv_buffer, sizeof(struct data_packet) + read_count);
        ASSERT_SYS_OK(read);

        struct data_packet *data_info = (struct data_packet*)recv_buffer;
        
        sum += data_info->count;
        int chunks = data_info->chunks;

        for (int j = 0; j < read_count; j++) {
            if (op == MIMPI_SUM)
                ((uint8_t*)new_send_data)[j] += ((uint8_t*)recv_buffer)[sizeof(struct data_packet) + j];
            else if (op == MIMPI_PROD)
                ((uint8_t*)new_send_data)[j] *= ((uint8_t*)recv_buffer)[sizeof(struct data_packet) + j];
            else if (op == MIMPI_MAX)
                ((uint8_t*)new_send_data)[j] = ((uint8_t*)new_send_data)[j] >= ((uint8_t*)recv_buffer)[sizeof(struct data_packet) + j]
                                            ? ((uint8_t*)new_send_data)[j] : ((uint8_t*)recv_buffer)[sizeof(struct data_packet) + j];
            else if (op == MIMPI_MIN)
                ((uint8_t*)new_send_data)[j] = ((uint8_t*)new_send_data)[j] <= ((uint8_t*)recv_buffer)[sizeof(struct data_packet) + j]
                                            ? ((uint8_t*)new_send_data)[j] : ((uint8_t*)recv_buffer)[sizeof(struct data_packet) + j];
        }

        for (int i = 0; i < chunks; i++) {
            int bytes_to_read = count - i * PACKET_SIZE - read_count >= PACKET_SIZE
                                ? PACKET_SIZE : count - PACKET_SIZE * i - read_count;

            void * chunk = malloc(bytes_to_read);

            read = chrecv(830 + rank * 8 + 1, chunk, bytes_to_read);
            ASSERT_SYS_OK(read);

            for (int j = 0; j < bytes_to_read; j++) {
                if (op == MIMPI_SUM)
                    ((uint8_t*)(new_send_data + read_count + i * PACKET_SIZE))[j] += ((uint8_t*)chunk)[j];
                else if (op == MIMPI_PROD)
                    ((uint8_t*)(new_send_data + read_count + i * PACKET_SIZE))[j] *= ((uint8_t*)chunk)[j];
                else if (op == MIMPI_MAX)
                    ((uint8_t*)(new_send_data + read_count + i * PACKET_SIZE))[j] = ((uint8_t*)(new_send_data + read_count + i * PACKET_SIZE))[j] >= ((uint8_t*)chunk)[j]
                                                ? ((uint8_t*)(new_send_data + read_count + i * PACKET_SIZE))[j] : ((uint8_t*)chunk)[j];
                else if (op == MIMPI_MIN)
                        ((uint8_t*)(new_send_data + read_count + i * PACKET_SIZE))[j] = ((uint8_t*)(new_send_data + read_count + i * PACKET_SIZE))[j] <= ((uint8_t*)chunk)[j]
                                                    ? ((uint8_t*)(new_send_data + read_count + i * PACKET_SIZE))[j] : ((uint8_t*)chunk)[j];
            }

            free(chunk);
        }

        free(recv_buffer);
    }

    if (child2_alive) {
        void * recv_buffer = malloc(sizeof(struct data_packet) + read_count);
        ssize_t read = chrecv(830 + rank * 8 + 3, recv_buffer, sizeof(struct data_packet) + read_count);
        ASSERT_SYS_OK(read);

        struct data_packet *data_info = (struct data_packet*)recv_buffer;
      
        sum += data_info->count;
        int chunks = data_info->chunks;

        for (int j = 0; j < read_count; j++) {
            if (op == MIMPI_SUM)
                ((uint8_t*)new_send_data)[j] += ((uint8_t*)recv_buffer)[sizeof(struct data_packet) + j];
            else if (op == MIMPI_PROD)
                ((uint8_t*)new_send_data)[j] *= ((uint8_t*)recv_buffer)[sizeof(struct data_packet) + j];
            else if (op == MIMPI_MAX)
                ((uint8_t*)new_send_data)[j] = ((uint8_t*)new_send_data)[j] >= ((uint8_t*)recv_buffer)[sizeof(struct data_packet) + j]
                                            ? ((uint8_t*)new_send_data)[j] : ((uint8_t*)recv_buffer)[sizeof(struct data_packet) + j];
            else if (op == MIMPI_MIN)
                ((uint8_t*)new_send_data)[j] = ((uint8_t*)new_send_data)[j] <= ((uint8_t*)recv_buffer)[sizeof(struct data_packet) + j]
                                            ? ((uint8_t*)new_send_data)[j] : ((uint8_t*)recv_buffer)[sizeof(struct data_packet) + j];
        }

        for (int i = 0; i < chunks; i++) {
            int bytes_to_read = count - i * PACKET_SIZE - read_count >= PACKET_SIZE
                                ? PACKET_SIZE : count - PACKET_SIZE * i - read_count;

            void * chunk = malloc(bytes_to_read);

            read = chrecv(830 + rank * 8 + 1, chunk, bytes_to_read);
            ASSERT_SYS_OK(read);

            for (int j = 0; j < bytes_to_read; j++) {
                if (op == MIMPI_SUM)
                    ((uint8_t*)(new_send_data + read_count + i * PACKET_SIZE))[j] += ((uint8_t*)chunk)[j];
                else if (op == MIMPI_PROD)
                    ((uint8_t*)(new_send_data + read_count + i * PACKET_SIZE))[j] *= ((uint8_t*)chunk)[j];
                else if (op == MIMPI_MAX)
                    ((uint8_t*)(new_send_data + read_count + i * PACKET_SIZE))[j] = ((uint8_t*)(new_send_data + read_count + i * PACKET_SIZE))[j] >= ((uint8_t*)chunk)[j]
                                                ? ((uint8_t*)(new_send_data + read_count + i * PACKET_SIZE))[j] : ((uint8_t*)chunk)[j];
                else if (op == MIMPI_MIN)
                        ((uint8_t*)(new_send_data + read_count + i * PACKET_SIZE))[j] = ((uint8_t*)(new_send_data + read_count + i * PACKET_SIZE))[j] <= ((uint8_t*)chunk)[j]
                                                    ? ((uint8_t*)(new_send_data + read_count + i * PACKET_SIZE))[j] : ((uint8_t*)chunk)[j];
            }

            free(chunk);
        }

        free(recv_buffer);
    }

    // signal to parent

    bool parent_alive = true;

    if (rank != root) {
        void * message = malloc(sizeof(struct data_packet) + read_count);
        
        struct data_packet *data_info = (struct data_packet*)message;
        data_info->count = sum;
        data_info->tag = -1;
        data_info->chunks = (count - read_count) / PACKET_SIZE + ((count - read_count) % PACKET_SIZE != 0);

        memcpy(message + sizeof(struct data_packet), new_send_data, read_count);

        ssize_t sent;

        if (left_child) sent = chsend(830 + parent * 8 + 2, message, sizeof(struct data_packet) + read_count);
        else sent = chsend(830 + parent * 8 + 4, message, sizeof(struct data_packet) + read_count);

        ASSERT_SYS_OK(sent);
        if (sent == -1 && errno == EPIPE) parent_alive = false;

        for (int i = 0; i < data_info->chunks; i++) {
            int bytes_to_read = count - i * PACKET_SIZE - read_count >= PACKET_SIZE
                                ? PACKET_SIZE : count - PACKET_SIZE * i - read_count;

            if (left_child) sent = chsend(830 + parent * 8 + 2, new_send_data + read_count + i * PACKET_SIZE, bytes_to_read);
            else sent = chsend(830 + parent * 8 + 4, new_send_data + read_count + i * PACKET_SIZE, bytes_to_read);

            ASSERT_SYS_OK(sent);
        }

        free(message);
    }

    // wait for parent's signal

    bool end_success = true;
    
    if (rank != root && parent_alive) {
        int *received = malloc(sizeof(int));
        ssize_t read;

        if (left_child)
            read = chrecv(830 + parent * 8 + 5, received, sizeof(int));
        else
            read = chrecv(830 + parent * 8 + 7, received, sizeof(int));

        ASSERT_SYS_OK(read);

        if (*received == REDUCE_FAILED_TAG) end_success = false;
        free(received);
    }

    // if root, save message

    if (rank == root) {
        if (sum != MIMPI_World_size()) end_success = false;
        else {
            memcpy(recv_data, new_send_data, count);
        }
    }
    
    // free message list

    free(new_send_data);

    // signal to children

    if (child1_alive) {
        int *message = malloc(sizeof(int));
        *message = end_success ? REDUCE_SUCCESS_TAG : REDUCE_FAILED_TAG;
        ssize_t sent = chsend(830 + rank * 8 + 6, message, sizeof(int));
        ASSERT_SYS_OK(sent);
        free(message);
    }

    if (child2_alive) {
        int *message = malloc(sizeof(int));
        *message = end_success ? REDUCE_SUCCESS_TAG : REDUCE_FAILED_TAG;
        ssize_t sent = chsend(830 + rank * 8 + 8, message, sizeof(int));
        ASSERT_SYS_OK(sent);
        free(message);
    }       

    if (!end_success) return MIMPI_ERROR_REMOTE_FINISHED;
    else return MIMPI_SUCCESS;
}

