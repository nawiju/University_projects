#ifndef SERVERS_H
#define SERVERS_H

#include <stdint.h>

#include "common.h"
#include "protconst.h"

#define MAX_MESSAGE_SIZE 64000

/* TCP functions */
int tcp_create_socket();
void tcp_bind_and_listen_on_socket(int socket_fd, uint16_t port);
int tcp_accept_connection(int socket_fd);
bool valid_read(ssize_t read_length, size_t expected_length);
bool tcp_send_conacc(int client_fd, uint64_t session_id);
void tcp_send_rjt(int client_fd, uint64_t session_id, uint64_t packet_number);
void tcp_send_rcvd(int client_fd, uint64_t session_id);
bool tcp_print_message(int client_fd, uint32_t message_size);
DATA_HEADER tcp_read_data_packet(int socket_fd, uint64_t expected_session_id,
                                 uint64_t expected_packet_number,
                                 bool *invalid_packet, bool *invalid_read);
CONN tcp_read_conn(int client_fd);

/* UDP/UDPR functions*/
void udp_bind_socket(int socket_fd, uint16_t port);
bool udp_send_rcvd(uint64_t session_id, int socket_fd,
                   struct sockaddr_in client_address,
                   socklen_t client_address_length);
bool udp_send_conrjt(uint64_t session_id, int socket_fd,
                     struct sockaddr_in client_address,
                     socklen_t client_address_length);
bool udp_send_rjt(uint64_t session_id, uint64_t packet_number, int socket_fd,
                  struct sockaddr_in client_address,
                  socklen_t client_address_length);
bool udp_send_acc(uint64_t session_id, uint64_t packet_number, int socket_fd,
                  struct sockaddr_in client_address,
                  socklen_t client_address_length);
bool udp_send_conacc(uint64_t session_id, int socket_fd,
                     struct sockaddr_in client_address,
                     socklen_t client_address_length);
char *udp_read_data_packet(int socket_fd,
                           struct sockaddr_in expected_client_address,
                           uint64_t expected_session_id, bool *to_continue,
                           bool *to_return, bool *eagain, ssize_t *read_length,
                           struct sockaddr_in *this_client_address);

/* Common functions */
bool valid_write(ssize_t written_length, size_t expected_length);

#endif