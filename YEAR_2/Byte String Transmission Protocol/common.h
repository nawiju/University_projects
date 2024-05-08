#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

/* Defines */
#define CONN_PACKET_TYPE_ID 1
#define CONACC_PACKET_TYPE_ID 2
#define CONRJT_PACKET_TYPE_ID 3
#define DATA_PACKET_TYPE_ID 4
#define ACC_PACKET_TYPE_ID 5
#define RJT_PACKET_TYPE_ID 6
#define RCVD_PACKET_TYPE_ID 7

#define TCP_PROTOCOL_ID 1
#define UDP_PROTOCOL_ID 2
#define UDPR_PROTOCOL_ID 3

#define SUCCESS 0
#define FAILURE 1

/* Packet structs */
typedef struct __attribute__((__packed__)) {
  uint8_t packet_type_id; // 1
  uint64_t session_id;
  uint8_t protocol_id;   // 1 - TCP, 2 - UDP, 3 - UDPR
  uint64_t message_size; // bytes
} CONN;

typedef struct __attribute__((__packed__)) {
  uint8_t packet_type_id; // 2
  uint64_t session_id;
} CONACC;

typedef struct __attribute__((__packed__)) {
  uint8_t packet_type_id; // 3
  uint64_t session_id;
} CONRJT;

typedef struct __attribute__((__packed__)) {
  uint8_t packet_type_id; // 4
  uint64_t session_id;
  uint64_t packet_number;
  uint32_t data_size; // bytes
  char *data;
} DATA;

typedef struct __attribute__((__packed__)) {
  uint8_t packet_type_id; // 5
  uint64_t session_id;
  uint64_t packet_number;
} ACC;

typedef struct __attribute__((__packed__)) {
  uint8_t packet_type_id; // 6
  uint64_t session_id;
  uint64_t packet_number;
} RJT;

typedef struct __attribute__((__packed__)) {
  uint8_t packet_type_id; // 7
  uint64_t session_id;
} RCVD;

/* Extra structs */
typedef struct __attribute__((__packed__)) {
  uint8_t packet_type_id; // 4
  uint64_t session_id;
  uint64_t packet_number;
  uint32_t data_size; // bytes
} DATA_HEADER;

/* Common functions */
uint16_t read_port(char const *string);
struct sockaddr_in get_server_address(char const *host, uint16_t port);
ssize_t readn(int fd, void *vptr, size_t n);
ssize_t writen(int fd, const void *vptr, size_t n);
void install_signal_handler(int signal, void (*handler)(int));

#endif
