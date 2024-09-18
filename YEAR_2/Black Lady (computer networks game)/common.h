#ifndef COMMON_H
#define COMMON_H

#include <arpa/inet.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <netdb.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>

#include <chrono>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <string>
#include <utility>

#include "err.h"

/* Constants */
#define ROUNDS 13

#define SUCCESS 0
#define FAILURE 1

#define BUFFER_SIZE 100000
const std::string ENDING = "\r\n";

/* Structs */
typedef struct NODE {
  uint16_t port;
  std::string address;
} NODE;

/* Functions */
uint16_t read_port(char const *string);
struct sockaddr_in get_server_address_IPv4(char const *host, uint16_t port);
struct sockaddr_in6 get_server_address_IPv6(char const *host, uint16_t port);
std::pair<int, std::pair<struct sockaddr_in, struct sockaddr_in6>>
get_server_address_unspecified(char const *host, uint16_t port);
ssize_t readn(int fd, void *vptr, size_t n);
ssize_t writen(int fd, const void *vptr, size_t n);
void install_signal_handler(int signal, void (*handler)(int), int flags);
bool valid_write(ssize_t written_length, size_t expected_length);
void print_log(char *message, uint32_t message_size, bool flush, char *BUFFER,
               size_t *BUFFER_INDEX);
std::pair<std::string, long long> get_current_time();
void set_peer_address(int socket_fd, NODE &node);
void set_own_address(int socket_fd, NODE &node);

#endif
