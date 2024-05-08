#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <netdb.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "clients.h"
#include "common.h"
#include "err.h"
#include "protconst.h"

/* Helper function declarations */
uint64_t rand_uint64(void) {
  srand(time(NULL));
  uint64_t r = 0;
  for (int i = LOOP_COUNT; i > 0; i--) {
    r = r * (RAND_MAX + (uint64_t)1) + rand();
  }
  return r;
}
// Credit to comment in
// https://stackoverflow.com/questions/33010010/how-to-generate-random-64-bit-unsigned-integer-in-c

// Returns the queue structure that will store the message
message_queue *create_message_queue() {
  message_queue *queue = malloc(sizeof(message_queue));

  if (queue == NULL) {
    syserr(ERROR_MALLOC);
  }

  queue->head = NULL;
  queue->tail = NULL;

  return queue;
}

// Frees the memory allocated for the message queue
void clean_memory(message_queue *queue) {
  if (queue != NULL) {
    message_node *node = queue->head;

    while (node != NULL) {
      message_node *next = node->next;
      free(node->message);
      free(node);
      node = next;
    }

    free(queue);
  }
}

// Creates the message queue from the input. Returns the size of the message. If
// an error occurs, the function will clean the memory and exit.
uint64_t read_from_stdin(message_queue **queue) {
  *queue = create_message_queue();
  uint64_t MESSAGE_SIZE = 0;

  while (true) {
    char *buffer = malloc(MAX_MESSAGE_SIZE);
    int offset = 0;

    if (buffer == NULL) {
      clean_memory(*queue);
      syserr(ERROR_MALLOC);
    }

    while (true) {
      ssize_t read_size =
          read(STDIN_FILENO, buffer + offset, MAX_MESSAGE_SIZE - offset);

      if (read_size < 0) {
        free(buffer);
        clean_memory(*queue);
        syserr(ERROR_READ);
      }

      offset += read_size;

      if (offset == MAX_MESSAGE_SIZE || read_size == 0) {
        break;
      }
    }

    if (offset == 0) {
      free(buffer);
      break;
    }

    message_node *node = malloc(sizeof(message_node));
    MESSAGE_SIZE += offset;

    if (node == NULL) {
      free(buffer);
      clean_memory(*queue);
      syserr(ERROR_MALLOC);
    }

    node->message = buffer;
    node->message_size = offset;
    node->next = NULL;

    if ((*queue)->head == NULL) {
      (*queue)->head = node;
      (*queue)->tail = node;
    } else {
      (*queue)->tail->next = node;
      (*queue)->tail = node;
    }
  }

  return MESSAGE_SIZE;
}

/* TCP functions */
// Checks if the write operation was successful. Returns true if failed.
bool check_write(ssize_t written_length, size_t expected_length) {
  if (written_length < 0) {
    if (errno == EPIPE || errno == EAGAIN) {
      error(ERROR_CONNECTION_CLOSED);
      return true;
    }

    error(ERROR_WRITEN);
    return true;
  } else if ((size_t)written_length != expected_length) {
    error(ERROR_INCOMPLETE_WRITEN);
    return true;
  }

  return false;
}

// Checks if the read operation was successful. Returns true if failed.
bool check_read(ssize_t read_length, size_t expected_length) {
  if (read_length < 0) {
    if (errno == EAGAIN) {
      error(ERROR_TIMEOUT);
      return true;
    } else if (errno == ECONNRESET || errno == EPIPE) {
      error(ERROR_CONNECTION_CLOSED);
      return true;
    }
    error(ERROR_READN);
    return true;
  } else if (read_length == 0) {
    error(ERROR_CONNECTION_CLOSED);
    return true;
  } else if ((size_t)read_length != expected_length) {
    error(ERROR_INCOMPLETE_READN);
    return true;
  }

  return false;
}

// Sends a data packet to the server. Returns true if the operation failed.
bool tcp_send_data_packet(int socket_fd, uint64_t session_id,
                          uint64_t packet_number, uint32_t data_size,
                          char *data) {
  char data_buffer[data_size + sizeof(DATA_HEADER)];

  DATA_HEADER data_header = {DATA_PACKET_TYPE_ID, htobe64(session_id),
                             htobe64(packet_number), htobe32(data_size)};
  memcpy(data_buffer, &data_header.packet_type_id, sizeof(uint8_t));
  memcpy(data_buffer + sizeof(uint8_t), &data_header.session_id,
         sizeof(uint64_t));
  memcpy(data_buffer + sizeof(uint8_t) + sizeof(uint64_t),
         &data_header.packet_number, sizeof(uint64_t));
  memcpy(data_buffer + sizeof(uint8_t) + 2 * sizeof(uint64_t),
         &data_header.data_size, sizeof(uint32_t));
  memcpy(data_buffer + sizeof(uint8_t) + 2 * sizeof(uint64_t) +
             sizeof(uint32_t),
         data, data_size);

  int written_length =
      writen(socket_fd, &data_buffer, sizeof(DATA_HEADER) + data_size);
  return check_write(written_length, sizeof(DATA_HEADER) + data_size);
}

// Receives a rcvd packet from the server. Returns true if the operation failed.
bool tcp_receive_rcvd(int socket_fd, uint64_t session_id) {
  uint64_t rcvd_session_id;
  ssize_t read_length = readn(socket_fd, &rcvd_session_id, sizeof(uint64_t));

  if (read_length < 0 ||
      (size_t)read_length != sizeof(RCVD) - sizeof(uint8_t) ||
      errno == EAGAIN) {
    error(ERROR_READN);
    return true;
  }

  if (be64toh(rcvd_session_id) != session_id) {
    error(ERROR_WRONG_SESSION_ID);
    return true;
  }

  return false;
}

// Receives a rjt packet from the server. Returns true if the operation failed.
bool tcp_receive_rjt(int socket_fd, uint64_t session_id,
                     uint64_t total_packets) {
  uint64_t rjt_session_id;
  uint64_t rjt_packet_number;

  ssize_t read_length = readn(socket_fd, &rjt_session_id, sizeof(uint64_t));

  if (!check_read(read_length, sizeof(uint64_t))) {
    return true;
  }

  read_length = readn(socket_fd, &rjt_packet_number, sizeof(uint64_t));

  if (!check_read(read_length, sizeof(uint64_t))) {
    return true;
  }

  if (be64toh(rjt_session_id) != session_id) {
    error(ERROR_WRONG_SESSION_ID);
    return true;
  }

  if (be64toh(rjt_packet_number) >= total_packets) {
    error(ERROR_WRONG_PACKET_NUMBER);
    return true;
  }

  return false;
}

/* UDP/UDPR functions */
// Checks if the sendto operation was successful. Returns true if failed.
bool check_sendto(ssize_t sent_length, size_t expected_length) {
  if (errno == EAGAIN) {
    error(ERROR_TIMEOUT);
    return true;
  } else if (sent_length < 0 || errno == EPIPE) {
    error(ERROR_SENDTO);
    return true;
  } else if ((size_t)sent_length != expected_length) {
    error(ERROR_INCOMPLETE_SENDTO);
    return true;
  }

  return false;
}

// Returns true if the connection was accepted, false otherwise.
// Sets failed to true if an error occurred at some point.
// Sets eagain_errno to true if eagain occurred.
bool udp_receive_conacc(int socket_fd, uint64_t session_id, bool *failed,
                        bool *eagain_errno) {
  char buffer[sizeof(DATA_HEADER) + MAX_MESSAGE_SIZE];
  memset(buffer, 0, sizeof(DATA_HEADER) + MAX_MESSAGE_SIZE);
  int flags = 0;

  struct sockaddr_in receive_address;
  socklen_t address_length = (socklen_t)sizeof(receive_address);
  ssize_t read_length =
      recvfrom(socket_fd, buffer, sizeof(DATA_HEADER) + MAX_MESSAGE_SIZE, flags,
               (struct sockaddr *)&receive_address, &address_length);

  if (errno == EAGAIN) {
    *failed = true;
    *eagain_errno = true;
    return false;
  }

  if (read_length < 0) {
    error(ERROR_RECVFROM);
    *failed = true;
    return false;
  }

  uint8_t packet_type_id = *((uint8_t *)buffer);

  if (packet_type_id == CONRJT_PACKET_TYPE_ID) {
    CONRJT conrjt = *((CONRJT *)buffer);

    if (be64toh(conrjt.session_id) != session_id) {
      error(ERROR_WRONG_SESSION_ID);
      *failed = true;
      return false;
    }

    error(ERROR_CONRJT);
    return false;
  } else if (packet_type_id != CONACC_PACKET_TYPE_ID) {
    error(ERROR_WRONG_PACKET_TYPE_ID);
    *failed = true;
    return false;
  }

  CONACC conacc = *((CONACC *)buffer);

  if (be64toh(conacc.session_id) != session_id) {
    error(ERROR_WRONG_SESSION_ID);
    *failed = true;
    return false;
  }

  return true;
}

// Sends a data packet to the server. Returns true if the operation failed.
bool udp_send_data_packet(int socket_fd, uint64_t session_id,
                          uint64_t packet_number, uint32_t data_size,
                          char *data, struct sockaddr_in server_address) {
  char data_buffer[data_size + sizeof(DATA_HEADER)];
  memset(data_buffer, 0, data_size + sizeof(DATA_HEADER));

  DATA_HEADER data_header = {DATA_PACKET_TYPE_ID, htobe64(session_id),
                             htobe64(packet_number), htobe32(data_size)};
  memcpy(data_buffer, &data_header.packet_type_id, sizeof(uint8_t));
  memcpy(data_buffer + sizeof(uint8_t), &data_header.session_id,
         sizeof(uint64_t));
  memcpy(data_buffer + sizeof(uint8_t) + sizeof(uint64_t),
         &data_header.packet_number, sizeof(uint64_t));
  memcpy(data_buffer + sizeof(uint8_t) + 2 * sizeof(uint64_t),
         &data_header.data_size, sizeof(uint32_t));
  memcpy(data_buffer + sizeof(uint8_t) + 2 * sizeof(uint64_t) +
             sizeof(uint32_t),
         data, data_size);

  ssize_t written_length = sendto(
      socket_fd, &data_buffer, sizeof(DATA_HEADER) + data_size, 0,
      (struct sockaddr *)&server_address, (socklen_t)sizeof(server_address));
  return check_sendto(written_length, sizeof(DATA_HEADER) + data_size);
}

// Sends a connection packet to the server. Returns true if the operation
// failed.
bool udp_send_conn(int socket_fd, struct sockaddr_in server_address,
                   uint64_t message_size, uint64_t session_id,
                   uint8_t protocol) {
  int flags = 0;
  CONN conn_packet = {CONN_PACKET_TYPE_ID, session_id, protocol, message_size};
  ssize_t written_length =
      sendto(socket_fd, &conn_packet, sizeof(CONN), flags,
             (struct sockaddr *)&server_address, sizeof(server_address));
  bool failed = check_sendto(written_length, sizeof(CONN));
  return failed;
}

// Returns true if succeeded in getting a correct acc packet from the server.
// Returns false if an error occurred or the packet did not come fromt the
// server during MAX_WAIT time. Sets eagain_errno to true if eagain occurred.
// Sets to_continue to true if the packet is not the one we are waiting for but
// it is still valid in some way.
bool udpr_receive_acc(int socket_fd, struct sockaddr_in server_address,
                      uint64_t session_id, uint64_t packet_number,
                      bool *eagain_errno, bool *to_continue, 
                      bool *send_packet) {
  char buffer[sizeof(DATA_HEADER) + MAX_MESSAGE_SIZE];
  memset(buffer, 0, sizeof(DATA_HEADER) + MAX_MESSAGE_SIZE);
  int flags = 0;

  struct sockaddr_in server_address_acc;
  socklen_t address_length_acc = (socklen_t)sizeof(server_address);
  ssize_t read_length =
      recvfrom(socket_fd, buffer, sizeof(DATA_HEADER) + MAX_MESSAGE_SIZE, flags,
               (struct sockaddr *)&server_address_acc, &address_length_acc);

  if (errno == EAGAIN) {
    errno = 0;
    *eagain_errno = true;
    *to_continue = true;
    *send_packet = true;
    return false;
  }

  if (read_length < 0) {
    error(ERROR_RECVFROM);
    return false;
  }

  if (server_address.sin_addr.s_addr != server_address_acc.sin_addr.s_addr ||
      server_address.sin_port != server_address_acc.sin_port) {
    error(ERROR_WRONG_ADDRESS);
    return false;
  }

  uint8_t packet_type_id = *((uint8_t *)buffer);

  // We received a conacc packet
  if (packet_type_id == CONACC_PACKET_TYPE_ID) {
    CONACC conacc = *((CONACC *)buffer);

    if (be64toh(conacc.session_id) != session_id) {
      // We received a conacc packet with a wrong session id
      error(ERROR_WRONG_SESSION_ID);
      return false;
    }

    *to_continue = true; // Ignore the conacc packet
    return false;
  }

  if (packet_type_id == ACC_PACKET_TYPE_ID) {
    ACC acc = *((ACC *)buffer);

    if (be64toh(acc.session_id) != session_id) {
      // We received a conacc packet with a wrong session id
      error(ERROR_WRONG_SESSION_ID);
      return false;
    }

    if (be64toh(acc.packet_number) == packet_number - 1) {
      *send_packet = true;
      *to_continue = true;
      return false;
    } else if (be64toh(acc.packet_number) < packet_number) {
      *to_continue = true;
      return false;
    } else if (be64toh(acc.packet_number) == packet_number) {
      return true;
    } else {
      // Throw error if the packet number is greater than the packet number of
      // the packet just sent
      error(ERROR_WRONG_PACKET_NUMBER);
      return false;
    }
  }

  if (packet_type_id == RJT_PACKET_TYPE_ID) {
    RJT rjt = *((RJT *)buffer);

    if (be64toh(rjt.session_id) != session_id) {
      // We received a rjt packet with a wrong session id
      error(ERROR_WRONG_SESSION_ID);
      return false;
    }

    error(ERROR_REJECTED);
    return false;
  }

  error(ERROR_WRONG_PACKET_TYPE_ID); // We received an unexpected packet
  return false;
}

/* Shared functions */
// Creates a socket and connects to the server if it concerns the TCP protocol.
// Returns the socket file descriptor.
int create_socket(message_queue *queue, const char *host, uint16_t port,
                  int protocol) {
  int socket_fd;

  if (protocol == UDP_PROTOCOL_ID) {
    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
  } else {
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  }

  if (socket_fd < 0) {
    clean_memory(queue);
    syserr(ERROR_CREATE_SOCKET);
  }

  if (protocol == TCP_PROTOCOL_ID) {
    struct sockaddr_in server_address = get_server_address(host, port);

    if (connect(socket_fd, (struct sockaddr *)&server_address,
                (socklen_t)sizeof(server_address)) < 0) {
      clean_memory(queue);
      syserr(ERROR_CONNECT);
    }
  }

  struct timeval to = {.tv_sec = MAX_WAIT, .tv_usec = 0};
  setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &to, sizeof to);
  setsockopt(socket_fd, SOL_SOCKET, SO_SNDTIMEO, &to, sizeof to);

  return socket_fd;
}

void close_program(int socket_fd, message_queue *queue) {
  clean_memory(queue);
  close(socket_fd);
}