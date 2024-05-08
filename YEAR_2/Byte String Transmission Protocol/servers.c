#include <arpa/inet.h>
#include <endian.h>
#include <errno.h>
#include <inttypes.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
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
#define QUEUE_LENGTH 69
#define BUFFER_SIZE 1024 * 1024
#define FAILED_CONN                                                            \
  (CONN) { 0, 0, 0, 0 }
#define FAILED_DATA_HEADER                                                     \
  (DATA_HEADER) { 0, 0, 0, 0 }

/* Common functions */
// Check if the read was successful. Returns false if not.
bool valid_read(ssize_t read_length, size_t expected_length) {
  if (read_length < 0 || errno == EAGAIN) {
    error(ERROR_INVALID_READN);
    errno = 0;
    return false;
  } else if (read_length == 0) {
    error(ERROR_CONNECTION_CLOSED);
    return false;
  } else if ((size_t)read_length != expected_length) {
    error(ERROR_INCOMPLETE_READN);
    return false;
  }

  return true;
}

// Check if the write was successful. Returns false if not.
bool valid_write(ssize_t written_length, size_t expected_length) {
  if (written_length < 0) {
    error(ERROR_WRITEN);
    return false;
  } else if ((size_t)written_length != expected_length) {
    error(ERROR_INCOMPLETE_WRITEN);
    return false;
  }

  return true;
}

/* TCP functions */
// Create a TCP socket. Returns the socket file descriptor.
int tcp_create_socket() {
  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

  if (socket_fd < 0) {
    syserr(ERROR_CREATE_SOCKET);
  }

  return socket_fd;
}

// The server binds the socket to a concrete address and listens for incoming
// connections. Throws an error if it fails and exits.
void tcp_bind_and_listen_on_socket(int socket_fd, uint16_t port) {
  // Bind the socket to a concrete address
  struct sockaddr_in server_address;
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = htonl(INADDR_ANY);
  server_address.sin_port = htons(port);

  if (bind(socket_fd, (struct sockaddr *)&server_address,
           (socklen_t)sizeof server_address) < 0) {
    syserr("bind");
  }

  // Listen for incoming connections
  if (listen(socket_fd, QUEUE_LENGTH) < 0) {
    syserr("listen");
  }

  // Find out what port the server is actually listening on
  socklen_t length = (socklen_t)sizeof server_address;

  if (getsockname(socket_fd, (struct sockaddr *)&server_address, &length) < 0) {
    syserr("getsockname");
  }
}

// Accept a connection on the socket. Returns the client file descriptor. If it
// fails, returns -1.
int tcp_accept_connection(int socket_fd) {
  struct sockaddr_in client_address;
  int client_fd = accept(socket_fd, (struct sockaddr *)&client_address,
                         &((socklen_t){sizeof(client_address)}));

  if (client_fd < 0) {
    return -1;
  }

  // Set timeouts for the client socket
  struct timeval to = {.tv_sec = MAX_WAIT, .tv_usec = 0};
  setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &to, sizeof to);
  setsockopt(client_fd, SOL_SOCKET, SO_SNDTIMEO, &to, sizeof to);

  return client_fd;
}

// Read a connection packet from the client. Returns the connection packet. If
// it fails, returns a connection packet with all fields set to 0. This is
// evaluated as a failed connection in the main function.
CONN tcp_read_conn(int fd) {
  uint8_t packet_type_id;
  ssize_t read_length = readn(fd, &packet_type_id, sizeof(uint8_t));

  if (!valid_read(read_length, sizeof(uint8_t))) {
    return FAILED_CONN; // Invalid packet, evaluated as failed connection
  } else if (packet_type_id != CONN_PACKET_TYPE_ID) {
    error(ERROR_WRONG_PACKET_TYPE_ID);
    return FAILED_CONN;
  }

  void *conn_buffer = malloc(sizeof(CONN));

  if (conn_buffer == NULL) {
    error(ERROR_MALLOC);
    return FAILED_CONN;
  }

  *((uint8_t *)conn_buffer) = packet_type_id;

  read_length =
      readn(fd, conn_buffer + sizeof(uint8_t), sizeof(CONN) - sizeof(uint8_t));

  if (!valid_read(read_length, sizeof(CONN) - sizeof(uint8_t))) {
    free(conn_buffer);
    return FAILED_CONN;
  }

  CONN conn_packet_temp = *((CONN *)conn_buffer);
  CONN conn_packet = {
      conn_packet_temp.packet_type_id, be64toh(conn_packet_temp.session_id),
      conn_packet_temp.protocol_id, be64toh(conn_packet_temp.message_size)};
  free(conn_buffer);

  if (conn_packet.protocol_id != 1) {
    error(ERROR_WRONG_PROTOCOL_ID);
    return FAILED_CONN;
  }

  return conn_packet;
}

// Read a data packet from the client. Returns the data packet. If it fails,
// returns a data packet with all fields set to 0. This is evaluated as a failed
// data packet in the main function.
DATA_HEADER tcp_read_data_packet(int socket_fd, uint64_t expected_session_id,
                                 uint64_t expected_packet_number,
                                 bool *invalid_packet, bool *invalid_read) {
  uint8_t packet_type_id;
  uint64_t session_id;
  uint64_t packet_number;
  uint32_t data_size;

  ssize_t read_length = readn(socket_fd, &packet_type_id, sizeof(uint8_t));

  if (!valid_read(read_length, sizeof(uint8_t))) {
    *invalid_read = true;
    return FAILED_DATA_HEADER;
  } else if (packet_type_id != DATA_PACKET_TYPE_ID) {
    *invalid_read =
        true; // Unexpected packet type; fail and close the connection
    return FAILED_DATA_HEADER;
  }

  read_length = readn(socket_fd, &session_id, sizeof(uint64_t));

  if (!valid_read(read_length, sizeof(uint64_t))) {
    *invalid_read = true;
    return FAILED_DATA_HEADER;
  } else if (expected_session_id != be64toh(session_id)) {
    *invalid_packet = true;
    return FAILED_DATA_HEADER;
  }

  read_length = readn(socket_fd, &packet_number, sizeof(uint64_t));

  if (!valid_read(read_length, sizeof(uint64_t))) {
    *invalid_read = true;
    return FAILED_DATA_HEADER;
  } else if (expected_packet_number != be64toh(packet_number)) {
    *invalid_packet = true;
    return FAILED_DATA_HEADER;
  }

  read_length = readn(socket_fd, &data_size, sizeof(uint32_t));

  if (!valid_read(read_length, sizeof(uint32_t))) {
    *invalid_read = true;
    return FAILED_DATA_HEADER;
  } else if (be32toh(data_size) > MAX_MESSAGE_SIZE) {
    *invalid_packet = true;
    return FAILED_DATA_HEADER;
  }

  DATA_HEADER data_packet = {packet_type_id, be64toh(session_id),
                             be64toh(packet_number), be32toh(data_size)};
  return data_packet;
}

// Send a connection accept packet to the client. Returns true if the write was
// successful, false otherwise.
bool tcp_send_conacc(int client_fd, uint64_t session_id) {
  CONACC connacc = {CONACC_PACKET_TYPE_ID, htobe64(session_id)};
  ssize_t written_length = writen(client_fd, &connacc, sizeof(CONACC));

  if (!valid_write(written_length, sizeof(CONACC))) {
    return false;
  }

  return true;
}

// Send a connection reject packet to the client. Does not return anything as it
// does not affect the main function. If there is an error, it is printed to
// stderr.
void tcp_send_rjt(int client_fd, uint64_t session_id, uint64_t packet_number) {
  RJT rjt = {RJT_PACKET_TYPE_ID, htobe64(session_id), htobe64(packet_number)};
  ssize_t written_length = writen(client_fd, &rjt, sizeof(RJT));
  valid_write(written_length, sizeof(RJT));
}

// Send a received packet to the client. Does not return anything as it does not
// affect the main function. If there is an error, it is printed to stderr.
void tcp_send_rcvd(int client_fd, uint64_t session_id) {
  RCVD rcvd = {RCVD_PACKET_TYPE_ID, htobe64(session_id)};
  ssize_t written_length = writen(client_fd, &rcvd, sizeof(RCVD));
  valid_write(written_length, sizeof(RCVD));
}

// UDP/UDPR functions
// Bind the socket to a concrete address. Throws an error if it fails and exits.
void udp_bind_socket(int socket_fd, uint16_t port) {
  struct sockaddr_in server_address;
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = htonl(INADDR_ANY);
  server_address.sin_port = htons(port);

  struct timeval to = {.tv_sec = MAX_WAIT, .tv_usec = 0};
  setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &to, sizeof to);
  setsockopt(socket_fd, SOL_SOCKET, SO_SNDTIMEO, &to, sizeof to);

  int rcvbuf_size = BUFFER_SIZE;
  setsockopt(socket_fd, SOL_SOCKET, SO_RCVBUF, &rcvbuf_size,
             sizeof(rcvbuf_size));

  if (bind(socket_fd, (struct sockaddr *)&server_address,
           (socklen_t)sizeof(server_address)) < 0) {
    syserr(ERROR_BIND);
  }
}

// Reads the next data packet that is received and determines what action should
// be taken by the main function. Returns the buffer with the received packet.
// eagain set to true if the error is EAGAIN.
// to_continue set to true if the main function should continue with the next
// iteration, mainly because the packet is not from the current client.
// to_return set to true if the main function should return, mainly because the
// packet is unexpected or faulty in some way
char *udp_read_data_packet(int socket_fd,
                           struct sockaddr_in expected_client_address,
                           uint64_t expected_session_id, bool *to_continue,
                           bool *to_return, bool *eagain, ssize_t *read_length,
                           struct sockaddr_in *this_client_address) {
  static char buffer[MAX_MESSAGE_SIZE + sizeof(DATA_HEADER) + 1];
  memset(buffer, 0, sizeof(buffer));
  struct sockaddr_in client_address;
  socklen_t client_address_length = sizeof(client_address);

  // Read next packet
  *read_length =
      recvfrom(socket_fd, buffer, sizeof(buffer), 0,
               (struct sockaddr *)&client_address, &client_address_length);
  uint8_t packet_type_id = *((uint8_t *)buffer);

  *this_client_address = client_address;

  if (errno == EAGAIN) {
    errno = 0;
    *eagain = true;
  } else if (*read_length < 0) {
    error(ERROR_RECVFROM);
    *to_return = true;
  } else if (packet_type_id == CONN_PACKET_TYPE_ID) {
    // Send connection reject packet to client; this connection packet is not
    // from the current client
    CONN tried_conn = *((CONN *)buffer);

    // If the current client is trying to connect again, ignore it
    if (be64toh(tried_conn.session_id) == expected_session_id &&
        client_address.sin_port == expected_client_address.sin_port &&
        client_address.sin_addr.s_addr ==
            expected_client_address.sin_addr.s_addr) {
      *to_continue = true;
      return buffer;
    }

    udp_send_conrjt(tried_conn.session_id, socket_fd, client_address,
                    client_address_length);
    *to_continue = true;
  } else if (packet_type_id != DATA_PACKET_TYPE_ID) {
    // Any other unexpected packet type. Client should not send any other packet
    // type but just in case
    error(ERROR_WRONG_PACKET_TYPE_ID);

    uint64_t received_session_id = *((uint64_t *)(buffer + 1));

    if (received_session_id != expected_session_id) {
      *to_continue =
          true; // If this is not our current client, no need to react
    } else {
      *to_return = true; // We stop the communication with the client, because
                         // we received an unexpected packet
    }
  }

  return buffer;
}

// Sends a received packet to the client. Returns true if the write was
// successful, false otherwise.
bool udp_send_rcvd(uint64_t session_id, int socket_fd,
                   struct sockaddr_in client_address,
                   socklen_t client_address_length) {
  RCVD rcvd = {RCVD_PACKET_TYPE_ID, session_id};
  ssize_t written_length =
      sendto(socket_fd, &rcvd, sizeof(RCVD), 0,
             (struct sockaddr *)&client_address, client_address_length);
  return valid_write(written_length, sizeof(RCVD));
}

// Sends a connection reject packet to the client. Returns true if the write was
// successful, false otherwise.
bool udp_send_conrjt(uint64_t session_id, int socket_fd,
                     struct sockaddr_in client_address,
                     socklen_t client_address_length) {
  CONRJT conrjt = {CONRJT_PACKET_TYPE_ID, session_id};
  ssize_t written_length =
      sendto(socket_fd, &conrjt, sizeof(CONRJT), 0,
             (struct sockaddr *)&client_address, client_address_length);
  return valid_write(written_length, sizeof(CONRJT));
}

// Sends a reject packet to the client. Returns true if the write was
// successful, false otherwise.
bool udp_send_rjt(uint64_t session_id, uint64_t packet_number, int socket_fd,
                  struct sockaddr_in client_address,
                  socklen_t client_address_length) {
  RJT rjt = {RJT_PACKET_TYPE_ID, session_id, packet_number};
  ssize_t written_length =
      sendto(socket_fd, &rjt, sizeof(RJT), 0,
             (struct sockaddr *)&client_address, client_address_length);
  return valid_write(written_length, sizeof(RJT));
}

bool udp_send_acc(uint64_t session_id, uint64_t packet_number, int socket_fd,
                  struct sockaddr_in client_address,
                  socklen_t client_address_length) {
  ACC acc = {ACC_PACKET_TYPE_ID, session_id, packet_number};
  ssize_t written_length =
      sendto(socket_fd, &acc, sizeof(ACC), 0,
             (struct sockaddr *)&client_address, client_address_length);
  return valid_write(written_length, sizeof(ACC));
}

bool udp_send_conacc(uint64_t session_id, int socket_fd,
                     struct sockaddr_in client_address,
                     socklen_t client_address_length) {
  CONACC conacc = {CONACC_PACKET_TYPE_ID, session_id};
  ssize_t written_length =
      sendto(socket_fd, &conacc, sizeof(CONACC), 0,
             (struct sockaddr *)&client_address, client_address_length);
  return valid_write(written_length, sizeof(CONACC));
}