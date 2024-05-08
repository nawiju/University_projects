#include <arpa/inet.h>
#include <endian.h>
#include <errno.h>
#include <inttypes.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "common.h"
#include "err.h"
#include "protconst.h"
#include "servers.h"

/* Defines */
#define TCP "tcp"
#define UDP "udp"
#define BUFFER_SIZE 100 * MAX_MESSAGE_SIZE


/* Global variables */
char BUFFER[BUFFER_SIZE];
uint64_t BUFFER_INDEX = 0;


/* Print function */
void print_message(char *message, uint32_t message_size, bool flush) {
  if (message_size + BUFFER_INDEX > BUFFER_SIZE) {
    ssize_t written = write(STDOUT_FILENO, BUFFER, BUFFER_INDEX);
    fflush(stdout);

    if (!valid_write(written, BUFFER_INDEX)) {
      error(ERROR_WRITE);
    }

    BUFFER_INDEX = 0;
  } else {
    memcpy(BUFFER + BUFFER_INDEX, message, message_size);
    BUFFER_INDEX += message_size;
  }

  if (flush) {
    ssize_t written = write(STDOUT_FILENO, BUFFER, BUFFER_INDEX);
    fflush(stdout);

    if (!valid_write(written, BUFFER_INDEX)) {
      error(ERROR_WRITE);
    }

    BUFFER_INDEX = 0;
  }
}


/* Servers */

// TCP server
int run_tcp_server(uint16_t port) {
  // Create and bind a socket
  int socket_fd = tcp_create_socket();
  tcp_bind_and_listen_on_socket(socket_fd, port);

  // Communicate with clients
  while (true) {
    // Accept a connection
    int client_fd = tcp_accept_connection(socket_fd);

    if (client_fd < 0) {
      error(ERROR_TCP_ACCEPT_CONNECTION);
      continue;
    }

    // Read the connection packet
    CONN conn = tcp_read_conn(client_fd);

    if (conn.packet_type_id != CONN_PACKET_TYPE_ID || errno == EAGAIN ||
        errno == ECONNRESET || errno == EPIPE || be64toh(conn.message_size) == 0) {
      close(client_fd);
      continue;
    }

    // Send the connection accept packet
    bool conacc_sent = tcp_send_conacc(client_fd, conn.session_id);

    if (!conacc_sent) {
      close(client_fd);
      continue;
    }

    // Read the data packets
    uint64_t session_id = conn.session_id;
    uint64_t total_data = conn.message_size;
    uint64_t data_received = 0;
    uint64_t packet_number = 0;

    bool success = true;

    while (data_received < total_data) {
      bool invalid_packet = false;
      bool invalid_read = false;

      // Read the data packet header
      DATA_HEADER data = tcp_read_data_packet(
          client_fd, session_id, packet_number, &invalid_packet, &invalid_read);

      if (errno == EAGAIN || invalid_read || errno == ECONNRESET ||
          errno == EPIPE) {
        success = false;
        break;
      }

      // Send the reject packet
      if (invalid_packet) {
        tcp_send_rjt(client_fd, session_id, packet_number);
        error(ERROR_REJECTED);
        success = false;
        break;
      }

      // Read the data packet message
      char message[data.data_size];
      ssize_t read_length = readn(client_fd, message, data.data_size);

      if (!valid_read(read_length, data.data_size)) {
        success = false;
        break;
      }

      // Print the message
      print_message(message, data.data_size, false);

      packet_number++;
      data_received += data.data_size;
    }

    // Flush the buffer
    print_message("", 0, true);

    // Send the received packet
    if (success) {
      tcp_send_rcvd(client_fd, session_id);
    }

    close(client_fd);
  }

  close(socket_fd);
  return 0;
}

// Serve a UDP/UDPR client
void serve_client(CONN conn, struct sockaddr_in client_address,
                  socklen_t client_address_length, int socket_fd, int client) {
  // Send the connection accept packet
  if (client == UDP_PROTOCOL_ID &&
      !udp_send_conacc(conn.session_id, socket_fd, client_address,
                       client_address_length)) {
    // We do not continue the communication with the client since we failed to
    // send the conacc packet
    return;
  }

  uint64_t tries = 0;
  uint64_t data_received = 0;
  uint64_t expected_packet_number = 0;
  uint64_t expected_session_id = be64toh(conn.session_id);
  uint64_t message_size = be64toh(conn.message_size);

  // Read the rest of the data packets
  while (data_received < message_size) {
    tries = client == UDPR_PROTOCOL_ID ? 0 : MAX_RETRANSMITS;
    bool send_response = true;

    while (tries <= MAX_RETRANSMITS) {
      if (client == UDPR_PROTOCOL_ID && send_response) {
        if (expected_packet_number == 0) {
          // Send the connection accept packet
          if (!udp_send_conacc(htobe64(expected_session_id), socket_fd,
                               client_address, client_address_length)) {
            // We do not continue the communication with the client since we
            // failed to send the conacc packet
            return;
          }
        } else if (!udp_send_acc(htobe64(expected_session_id),
                                 htobe64(expected_packet_number - 1), socket_fd,
                                 client_address, client_address_length)) {
          // Send acc packet
          // We do not continue the communication with the client since we
          // failed to send the acc packet
          print_message("", 0, true);
          return;
        }
      }

      send_response = false;

      bool to_continue = false;
      bool to_return = false;
      bool eagain = false;
      struct sockaddr_in this_client_address;
      ssize_t read_length = 0;

      // Wait for the next data packet
      char *buffer = udp_read_data_packet(
          socket_fd, client_address, expected_session_id, &to_continue,
          &to_return, &eagain, &read_length, &this_client_address);

      if (eagain) {
        if (client == UDPR_PROTOCOL_ID) {
          tries++;
          errno = 0;
          send_response = true;
          continue;
        } else {
          error(ERROR_TIMEOUT);
          print_message("", 0, true);
          return;
        }
      } else if (to_return) {
        print_message("", 0, true);
        return;
      } else if (to_continue) {
        if (expected_packet_number == 0 && client == UDPR_PROTOCOL_ID) {
          uint8_t packet_type_id = *((uint8_t *)buffer);

          if (packet_type_id == CONN_PACKET_TYPE_ID) {
            CONN conn = *((CONN *)buffer);

            if (expected_session_id == be64toh(conn.session_id)) {
              tries++;
              send_response = true;
            }
          }
        }

        continue;
      }

      // We have a data packet
      DATA_HEADER data = *((DATA_HEADER *)buffer);
      uint64_t received_session_id = be64toh(data.session_id);
      uint64_t received_packet_number = be64toh(data.packet_number);
      uint32_t received_data_size = be32toh(data.data_size);

      if (received_session_id != expected_session_id ||
          received_packet_number > expected_packet_number ||
          received_data_size > MAX_MESSAGE_SIZE ||
          received_data_size != (read_length - sizeof(DATA_HEADER))) {
        // Send reject packet to client; this data packet is not from the
        // current client
        udp_send_rjt(htobe64(received_session_id),
                     htobe64(received_packet_number), socket_fd,
                     this_client_address,
                     (socklen_t)sizeof(this_client_address));
        error(ERROR_REJECTED);

        if (received_session_id != expected_session_id) {
          continue; // Not our client, we just reject.
        } else {
          print_message("", 0, true);
          return; // We stop the communication with the client
        }
      }

      if (received_packet_number == expected_packet_number - 1 &&
          client == UDPR_PROTOCOL_ID) {
        tries++;
        send_response = true;
        continue;
      }

      if (received_packet_number < expected_packet_number &&
          client == UDPR_PROTOCOL_ID) {
        continue; // We ignore the packet
      }

      // Print the message
      char *message = buffer + sizeof(DATA_HEADER);
      data_received += received_data_size;
      print_message(message, received_data_size, false);

      break;
    }

    if (tries > MAX_RETRANSMITS && client == UDPR_PROTOCOL_ID) {
      error(ERROR_RETRANSMITS_EXCEEDED);
      print_message("", 0, true);
      return;
    }

    expected_packet_number++;
  }

  print_message("", 0, true);

  // Send last acc packet
  if (client == UDPR_PROTOCOL_ID) {
    udp_send_acc(htobe64(expected_session_id),
                 htobe64(expected_packet_number - 1), socket_fd, client_address,
                 client_address_length);
  }

  // Send the received packet
  udp_send_rcvd(htobe64(expected_session_id), socket_fd, client_address,
                client_address_length);
}

// UDP server
void run_udp_server(uint16_t port) {
  // Create socket
  int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);

  if (socket_fd < 0) {
    syserr(ERROR_CREATE_SOCKET);
  }

  // Bind the socket to a concrete address.
  udp_bind_socket(socket_fd, port);

  // Communicate with clients
  while (true) {
    // Read a packet from a client
    char buffer[MAX_MESSAGE_SIZE + sizeof(DATA_HEADER) + 1];
    memset(buffer, 0, sizeof(buffer));

    struct sockaddr_in client_address;
    socklen_t client_address_length = sizeof(client_address);
    ssize_t read_length =
        recvfrom(socket_fd, buffer, sizeof(buffer), 0,
                 (struct sockaddr *)&client_address, &client_address_length);

    // If errno is EAGAIN it means that there is no data to read. Continue to
    // the next iteration of the loop and try again.
    if (errno == EAGAIN) {
      errno = 0;
      continue;
    } else if (read_length < 0) {
      error(ERROR_RECVFROM);
      continue;
    }

    uint8_t packet_type_id = *((uint8_t *)buffer);

    if (packet_type_id == DATA_PACKET_TYPE_ID) {
      DATA_HEADER data = *((DATA_HEADER *)buffer);

      // Send reject packet to client; this data packet is not from the current
      // client
      udp_send_rjt(data.session_id, data.packet_number, socket_fd,
                   client_address, client_address_length);
      continue;
    } else if (packet_type_id == CONN_PACKET_TYPE_ID) {
      CONN conn = *((CONN *)buffer);

      if (read_length != (ssize_t)sizeof(CONN)) {
        error(ERROR_INVALID_RECVFROM);
        continue;
      }

      uint8_t protocol_id = conn.protocol_id;

      if (protocol_id == UDP_PROTOCOL_ID) {
        if (be64toh(conn.message_size) == 0) {
          continue;
        }
        serve_client(conn, client_address, client_address_length, socket_fd,
                     UDP_PROTOCOL_ID);
      } else if (protocol_id == UDPR_PROTOCOL_ID) {
        serve_client(conn, client_address, client_address_length, socket_fd,
                     UDPR_PROTOCOL_ID);
      } else {
        error(ERROR_WRONG_PROTOCOL_ID);
      }
    } else {
      // Unexpected packet type. Client should not send any other packet type
      // but this is here just in case.
      error(ERROR_WRONG_PACKET_TYPE_ID);
    }
  }

  close(socket_fd);
}

/* Main */
int main(int argc, char *argv[]) {
  if (argc != 3) {
    fatal("usage: %s <protocol> <port>", argv[0]);
  }

  const char *protocol = argv[1];
  uint16_t port = read_port(argv[2]);

  signal(SIGPIPE, SIG_IGN);

  if (strcmp(protocol, TCP) == 0) {
    run_tcp_server(port);
  } else if (strcmp(protocol, UDP) == 0) {
    run_udp_server(port);
  }

  fatal("Unknown protocol: %s", protocol);
  return 0;
}