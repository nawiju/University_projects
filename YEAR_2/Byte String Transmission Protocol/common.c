#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <netdb.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "common.h"
#include "err.h"

/* Socket and other communication-helping functions */
uint16_t read_port(char const *string) {
  char *endptr;
  errno = 0;
  unsigned long port = strtoul(string, &endptr, 10);
  if (errno != 0 || *endptr != 0 || port > UINT16_MAX) {
    fatal("%s is not a valid port number", string);
  }
  return (uint16_t)port;
}

struct sockaddr_in get_server_address(char const *host, uint16_t port) {
  struct addrinfo hints;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET; // IPv4
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;

  struct addrinfo *address_result;
  int errcode = getaddrinfo(host, NULL, &hints, &address_result);
  if (errcode != 0) {
    fatal("getaddrinfo: %s", gai_strerror(errcode));
  }

  struct sockaddr_in send_address;
  send_address.sin_family = AF_INET; // IPv4
  send_address.sin_addr.s_addr =     // IP address
      ((struct sockaddr_in *)(address_result->ai_addr))->sin_addr.s_addr;
  send_address.sin_port = htons(port); // port from the command line

  freeaddrinfo(address_result);

  return send_address;
}

// Following two functions come from Stevens' "UNIX Network Programming" book.
// Read n bytes from a descriptor. Use in place of read() when fd is a stream
// socket.
ssize_t readn(int fd, void *vptr, size_t n) {
  ssize_t nleft, nread;
  char *ptr;

  ptr = vptr;
  nleft = n;
  while (nleft > 0) {
    if ((nread = read(fd, ptr, nleft)) < 0)
      return nread; // When error, return < 0.
    else if (nread == 0)
      break; // EOF

    nleft -= nread;
    ptr += nread;
  }
  return n - nleft; // return >= 0
}

// Write n bytes to a descriptor.
ssize_t writen(int fd, const void *vptr, size_t n) {
  ssize_t nleft, nwritten;
  const char *ptr;

  ptr = vptr; // Can't do pointer arithmetic on void*.
  nleft = n;
  while (nleft > 0) {
    if ((nwritten = write(fd, ptr, nleft)) <= 0)
      return nwritten; // error

    nleft -= nwritten;
    ptr += nwritten;
  }
  return n;
}

void install_signal_handler(int signal, void (*handler)(int)) {
  struct sigaction action;
  sigset_t block_mask;

  sigemptyset(&block_mask);
  action.sa_handler = handler;
  action.sa_mask = block_mask;
  action.sa_flags = 0;

  if (sigaction(signal, &action, NULL) < 0) {
    syserr("sigaction");
  }
}
