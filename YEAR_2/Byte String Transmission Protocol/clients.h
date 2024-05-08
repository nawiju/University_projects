#ifndef CLIENTS_H
#define CLIENTS_H

#include <inttypes.h>
#include <limits.h>
#include <stdlib.h>

#include "common.h"

/* Constants used in the program */

#if RAND_MAX / 256 >= 0xFFFFFFFFFFFFFF
#define LOOP_COUNT 1
#elif RAND_MAX / 256 >= 0xFFFFFF
#define LOOP_COUNT 2
#elif RAND_MAX / 256 >= 0x3FFFF
#define LOOP_COUNT 3
#elif RAND_MAX / 256 >= 0x1FF
#define LOOP_COUNT 4
#else
#define LOOP_COUNT 5
#endif

#define MAX_MESSAGE_SIZE 1000

/* Structures used in the program */
typedef struct message_node {
  struct message_node *next;
  char *message;
  uint64_t message_size;
} message_node;

typedef struct message_queue {
  struct message_node *head;
  struct message_node *tail;
} message_queue;

/* Shared functions */
uint64_t rand_uint64(void);
message_queue *create_message_queue();
uint64_t read_from_stdin(message_queue **queue);
void clean_memory(message_queue *queue);
int create_socket(message_queue *queue, const char *host, uint16_t port,
                  int protocol);
void close_program(int socket_fd, message_queue *queue);

/* TCP functions */
bool check_write(ssize_t written_length, size_t expected_length);
bool check_read(ssize_t read_length, size_t expected_length);
bool tcp_send_data_packet(int socket_fd, uint64_t session_id,
                          uint64_t packet_number, uint32_t data_size,
                          char *data);
bool tcp_receive_rcvd(int socket_fd, uint64_t session_id);
bool tcp_receive_rjt(int socket_fd, uint64_t session_id,
                     uint64_t total_packets);

/* UDP/UDPR functions */
bool check_sendto(ssize_t sent_length, size_t expected_length);
bool udp_receive_conacc(int socket_fd, uint64_t session_id, bool *failed,
                        bool *eagain_errno);
bool udp_send_data_packet(int socket_fd, uint64_t session_id,
                          uint64_t packet_number, uint32_t data_size,
                          char *data, struct sockaddr_in server_address);
bool udp_send_conn(int socket_fd, struct sockaddr_in server_address,
                   uint64_t message_size, uint64_t session_id,
                   uint8_t protocol);
bool udpr_receive_acc(int socket_fd, struct sockaddr_in server_address,
                      uint64_t session_id, uint64_t packet_number,
                      bool *eagain_errno, bool *to_continue, bool *send_packet);

#endif
