#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <netdb.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "clients.h"
#include "common.h"
#include "err.h"
#include "protconst.h"

/* Defined */
#define UDP "udp"
#define TCP "tcp"
#define UDPR "udpr"

/* Global variables */
message_queue *MESSAGE = NULL;
uint64_t MESSAGE_SIZE = 0;

/* Clients */
void tcp_client(const char *host, uint16_t port) {
  // Create a socket
  int socket_fd = create_socket(MESSAGE, host, port, TCP_PROTOCOL_ID);
  uint64_t session_id = rand_uint64();

  // Send the connection packet; connect to the server
  CONN conn_packet = {CONN_PACKET_TYPE_ID, htobe64(session_id), TCP_PROTOCOL_ID,
                      htobe64(MESSAGE_SIZE)};

  ssize_t written_length = writen(socket_fd, &conn_packet, sizeof(CONN));
  bool failed = check_write(written_length, sizeof(CONN));

  if (failed) {
    close_program(socket_fd, MESSAGE);
    exit(FAILURE);
  }

  // Receive the connection accept packet from the server
  CONACC conacc;
  ssize_t read_length = readn(socket_fd, &conacc, sizeof(CONACC));
  bool invalid_read = check_read(read_length, sizeof(CONACC));

  if (invalid_read) {
    close_program(socket_fd, MESSAGE);
    exit(FAILURE);
  } else if (conacc.packet_type_id != CONACC_PACKET_TYPE_ID) {
    close_program(socket_fd, MESSAGE);
    fatal(ERROR_WRONG_PACKET_TYPE_ID);
  } else if (be64toh(conacc.session_id) != session_id) {
    close_program(socket_fd, MESSAGE);
    fatal(ERROR_WRONG_SESSION_ID);
  }

  // Send the data packets to the server
  message_node *node = MESSAGE->head;
  uint64_t packet_number = 0;

  while (node != NULL) {
    if (tcp_send_data_packet(socket_fd, session_id, packet_number,
                             node->message_size, node->message)) {
      close_program(socket_fd, MESSAGE);
      exit(FAILURE);
    }

    packet_number++;
    node = node->next;
  }

  // Receive the received or rejected packet from the server
  uint8_t packet_type_id;
  read_length = readn(socket_fd, &packet_type_id, sizeof(uint8_t));
  invalid_read = check_read(read_length, sizeof(uint8_t));

  if (invalid_read) {
    close_program(socket_fd, MESSAGE);
    exit(FAILURE);
  }

  if (packet_type_id == RJT_PACKET_TYPE_ID) {
    tcp_receive_rjt(socket_fd, session_id, packet_number);
    close_program(socket_fd, MESSAGE);
    exit(FAILURE);
  } else if (packet_type_id == RCVD_PACKET_TYPE_ID) {
    bool failed = tcp_receive_rcvd(socket_fd, session_id);
    close_program(socket_fd, MESSAGE);

    if (failed) {
      exit(FAILURE);
    }
    
    return;
  }

  close_program(socket_fd, MESSAGE);
  fatal(ERROR_WRONG_PACKET_TYPE_ID);
}

void udp_client(const char *host, uint16_t port) {
  struct sockaddr_in server_address = get_server_address(host, port);
  uint64_t session_id = rand_uint64();
  int flags = 0;

  // Create a socket
  int socket_fd = create_socket(MESSAGE, host, port, UDP_PROTOCOL_ID);

  // Send the connection packet
  if (udp_send_conn(socket_fd, server_address, htobe64(MESSAGE_SIZE),
                    htobe64(session_id), UDP_PROTOCOL_ID)) {
    error(ERROR_FAILED_SEND_CONN);
    close_program(socket_fd, MESSAGE);
    exit(FAILURE);
  }

  // Receive the connection accept packet from the server
  bool failed_conacc = false;
  bool eagain = false;
  bool connection_accepted =
      udp_receive_conacc(socket_fd, session_id, &failed_conacc, &eagain);

  if (failed_conacc) {
    if (errno == EAGAIN) {
      error(ERROR_TIMEOUT);
    }

    close_program(socket_fd, MESSAGE);
    exit(FAILURE);
  } else if (!connection_accepted) {
    close_program(socket_fd, MESSAGE);
    return;
  }

  // Send the data packets to the server
  message_node *node = MESSAGE->head;
  uint64_t packet_number = 0;

  while (node != NULL) {
    if (udp_send_data_packet(socket_fd, session_id, packet_number,
                             node->message_size, node->message,
                             server_address)) {
      close_program(socket_fd, MESSAGE);
      exit(FAILURE);
    }

    packet_number++;
    node = node->next;
  }

  // Receive the received or rejected packet from the server
  char buffer[sizeof(DATA_HEADER) + MAX_MESSAGE_SIZE];
  memset(buffer, 0, sizeof(DATA_HEADER) + MAX_MESSAGE_SIZE);

  struct sockaddr_in server_address_recv;
  socklen_t address_length_temp = (socklen_t)sizeof(server_address_recv);
  ssize_t read_length =
      recvfrom(socket_fd, buffer, sizeof(DATA_HEADER) + MAX_MESSAGE_SIZE, flags,
               (struct sockaddr *)&server_address_recv, &address_length_temp);

  if (read_length < 0) {
    close_program(socket_fd, MESSAGE);
    syserr(ERROR_RECVFROM);
  }

  uint8_t packet_type_id = *((uint8_t *)buffer);

  if (packet_type_id == RJT_PACKET_TYPE_ID) {
    close_program(socket_fd, MESSAGE);
    fatal(ERROR_REJECTED);
  } else if (packet_type_id == RCVD_PACKET_TYPE_ID) {
    RCVD rcvd = *((RCVD *)buffer);

    close_program(socket_fd, MESSAGE);

    if (be64toh(rcvd.session_id) != session_id) {
      fatal(ERROR_WRONG_SESSION_ID);
    } else if (server_address.sin_addr.s_addr !=
                   server_address_recv.sin_addr.s_addr ||
               server_address.sin_port != server_address_recv.sin_port) {
      fatal(ERROR_WRONG_ADDRESS);
    }

    return;
  }

  close_program(socket_fd, MESSAGE);
  fatal(ERROR_WRONG_PACKET_TYPE_ID);
}

void udpr_client(const char *host, uint16_t port) {
  struct sockaddr_in server_address = get_server_address(host, port);
  uint64_t session_id = rand_uint64();
  int flags = 0;

  // Create a socket
  int socket_fd = create_socket(MESSAGE, host, port, UDP_PROTOCOL_ID);
  uint64_t tries = 0;

  while (tries <= MAX_RETRANSMITS) {
    // Send the connection packet
    if (udp_send_conn(socket_fd, server_address, htobe64(MESSAGE_SIZE),
                      htobe64(session_id), UDPR_PROTOCOL_ID)) {
      close_program(socket_fd, MESSAGE);
      exit(FAILURE);
    }

    // Receive the connection accept packet from the server
    bool failed_conacc = false;
    bool eagain_errno = false;
    bool connection_accepted = udp_receive_conacc(
        socket_fd, session_id, &failed_conacc, &eagain_errno);

    if (eagain_errno) {
      tries++;
      errno = 0;
    } else if (failed_conacc) {
      close_program(socket_fd, MESSAGE);
      exit(FAILURE);
    } else if (connection_accepted) {
      break;
    } else if (!connection_accepted) {
      close_program(socket_fd, MESSAGE);
      return;
    }
  }

  if (tries > MAX_RETRANSMITS) {
    error(ERROR_RETRANSMITS_EXCEEDED);
    close_program(socket_fd, MESSAGE);
    return;
  }

  // Send the data packets to the server
  message_node *node = MESSAGE->head;
  uint64_t packet_number = 0;

  while (node != NULL) {
    tries = 0;
    bool send_packet = true;

    while (tries <= MAX_RETRANSMITS) {
      if (send_packet) {
        if (udp_send_data_packet(socket_fd, session_id, packet_number,
                                 node->message_size, node->message,
                                 server_address)) {
          close_program(socket_fd, MESSAGE);
          exit(FAILURE);
        }
      }

      send_packet = false;

      // Receive the acc packet
      bool eagain = false;
      bool to_continue = false;
      bool success = udpr_receive_acc(socket_fd, server_address, session_id,
                                      packet_number, &eagain, &to_continue, 
                                      &send_packet);

      if (send_packet) {
        tries++;
      }

      if (success) {
        // We received the acc packet that we were expecting
        break;
      } else if (eagain) {
        errno = 0;
        continue;
      } else if (to_continue) {
        continue;
      } else {
        // An error occurred and we need to close the program.
        close_program(socket_fd, MESSAGE);
        exit(FAILURE);
      }
    }

    if (tries > MAX_RETRANSMITS) {
      error(ERROR_RETRANSMITS_EXCEEDED);
      close_program(socket_fd, MESSAGE);
      exit(FAILURE);
    }

    packet_number++;
    node = node->next;
  }

  // Receive the received packet from the server
  while (true) {
    char buffer[sizeof(DATA_HEADER) + MAX_MESSAGE_SIZE];
    memset(buffer, 0, sizeof(DATA_HEADER) + MAX_MESSAGE_SIZE);

    socklen_t address_length = (socklen_t)sizeof(server_address);
    ssize_t read_length =
        recvfrom(socket_fd, buffer, sizeof(DATA_HEADER) + MAX_MESSAGE_SIZE,
                 flags, (struct sockaddr *)&server_address, &address_length);

    if (read_length < 0 || errno == EAGAIN) {
      close_program(socket_fd, MESSAGE);
      syserr(ERROR_RECVFROM);
    }

    uint8_t packet_type_id = *((uint8_t *)buffer);

    if (packet_type_id == ACC_PACKET_TYPE_ID) {
      ACC acc = *((ACC *)buffer);

      if (be64toh(acc.session_id) != session_id) {
        close_program(socket_fd, MESSAGE);
        fatal(ERROR_WRONG_SESSION_ID);
      } else if (be64toh(acc.packet_number) > packet_number) {
        close_program(socket_fd, MESSAGE);
        fatal(ERROR_WRONG_PACKET_NUMBER);
      }

      continue;
    } else if (packet_type_id == CONACC_PACKET_TYPE_ID) {
      continue;
    }

    if (packet_type_id != RCVD_PACKET_TYPE_ID) {
      close_program(socket_fd, MESSAGE);
      fatal(ERROR_WRONG_PACKET_TYPE_ID);
    }

    break;
  }

  close_program(socket_fd, MESSAGE);
  return;
}

/* Main */
int main(int argc, char *argv[]) {
  if (argc != 4) {
    fatal("Usage: %s <protocol> <host> <port>", argv[0]);
  }

  const char *protocol = argv[1];
  const char *host = argv[2];
  uint16_t port = read_port(argv[3]);

  if (strcmp(protocol, TCP) == 0) {
    MESSAGE_SIZE = read_from_stdin(&MESSAGE);
    tcp_client(host, port);
    return 0;
  } else if (strcmp(protocol, UDP) == 0) {
    MESSAGE_SIZE = read_from_stdin(&MESSAGE);
    udp_client(host, port);
    return 0;
  } else if (strcmp(protocol, UDPR) == 0) {
    MESSAGE_SIZE = read_from_stdin(&MESSAGE);
    udpr_client(host, port);
    return 0;
  }

  fatal("%s: %s", ERROR_UNKNOWN_PROTOCOL, protocol);
  return 0;
}