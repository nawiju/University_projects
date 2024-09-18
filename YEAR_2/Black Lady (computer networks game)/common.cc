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

struct sockaddr_in get_server_address_IPv4(char const *host, uint16_t port) {
  struct addrinfo hints;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;  // IPv4
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;

  struct addrinfo *address_result;
  int errcode = getaddrinfo(host, NULL, &hints, &address_result);
  if (errcode != 0) {
    fatal("getaddrinfo: %s", gai_strerror(errcode));
  }

  struct sockaddr_in send_address;
  send_address.sin_family = AF_INET;  // IPv4
  send_address.sin_addr.s_addr =      // IP address
      ((struct sockaddr_in *)(address_result->ai_addr))->sin_addr.s_addr;
  send_address.sin_port = htons(port);  // port from the command line

  freeaddrinfo(address_result);

  return send_address;
}

struct sockaddr_in6 get_server_address_IPv6(char const *host, uint16_t port) {
  struct addrinfo hints;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET6;  // IPv6
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;

  struct addrinfo *address_result;
  int errcode = getaddrinfo(host, NULL, &hints, &address_result);
  if (errcode != 0) {
    fatal("getaddrinfo: %s", gai_strerror(errcode));
  }

  struct sockaddr_in6 send_address;
  memset(&send_address, 0, sizeof(send_address));  // Initialize all fields to 0

  send_address.sin6_family = AF_INET6;  // IPv6
  send_address.sin6_addr =              // IP address
      ((struct sockaddr_in6 *)(address_result->ai_addr))->sin6_addr;
  send_address.sin6_port = htons(port);  // Port from the command line

  // These fields should be set to 0 unless specifically needed
  send_address.sin6_flowinfo = 0;
  send_address.sin6_scope_id = 0;

  freeaddrinfo(address_result);

  return send_address;
}

std::pair<int, std::pair<struct sockaddr_in, struct sockaddr_in6>>
get_server_address_unspecified(char const *host, uint16_t port) {
  struct addrinfo hints;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;  // Allow both IPv4 and IPv6
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;

  struct addrinfo *address_result;
  int errcode = getaddrinfo(host, NULL, &hints, &address_result);
  if (errcode != 0) {
    fatal("getaddrinfo: %s", gai_strerror(errcode));
  }

  // Determine whether the returned address is IPv4 or IPv6
  std::pair<int, std::pair<struct sockaddr_in, struct sockaddr_in6>> result;

  if (address_result->ai_family == AF_INET) {  // IPv4
    struct sockaddr_in send_address;
    send_address.sin_family = AF_INET;
    send_address.sin_addr.s_addr =
        ((struct sockaddr_in *)(address_result->ai_addr))->sin_addr.s_addr;
    send_address.sin_port = htons(port);

    result.first = AF_INET;
    result.second.first = send_address;
  } else {  // IPv6
    struct sockaddr_in6 send_address;
    memset(&send_address, 0, sizeof(send_address));
    send_address.sin6_family = AF_INET6;
    send_address.sin6_addr =
        ((struct sockaddr_in6 *)(address_result->ai_addr))->sin6_addr;
    send_address.sin6_port = htons(port);
    send_address.sin6_flowinfo = 0;
    send_address.sin6_scope_id = 0;

    result.first = AF_INET6;
    result.second.second = send_address;
  }

  freeaddrinfo(address_result);
  return result;
}

// Following two functions come from Stevens' "UNIX Network Programming" book.
// Read n bytes from a descriptor. Use in place of read() when fd is a stream
// socket.
ssize_t readn(int fd, void *vptr, size_t n) {
  ssize_t nleft, nread;
  char *ptr;

  ptr = (char *)vptr;
  nleft = n;
  while (nleft > 0) {
    if ((nread = read(fd, ptr, nleft)) < 0)
      return nread;  // When error, return < 0.
    else if (nread == 0)
      break;  // EOF

    nleft -= nread;
    ptr += nread;
  }
  return n - nleft;  // return >= 0
}

// Write n bytes to a descriptor.
ssize_t writen(int fd, const void *vptr, size_t n) {
  ssize_t nleft, nwritten;
  const char *ptr;

  ptr = (char *)vptr;  // Can't do pointer arithmetic on void*.
  nleft = n;
  while (nleft > 0) {
    if ((nwritten = write(fd, ptr, nleft)) <= 0) return nwritten;  // error

    nleft -= nwritten;
    ptr += nwritten;
  }
  return n;
}

void install_signal_handler(int signal, void (*handler)(int), int flags) {
  struct sigaction action;
  sigset_t block_mask;

  sigemptyset(&block_mask);
  action.sa_handler = handler;
  action.sa_mask = block_mask;
  action.sa_flags = flags;

  if (sigaction(signal, &action, NULL) < 0) {
    syserr("sigaction");
  }
}

// Check if the write was successful. Returns false if not.
bool valid_write(ssize_t written_length, size_t expected_length) {
  if (written_length < 0) {
    error(ERROR_WRITE);
    return false;
  } else if ((size_t)written_length != expected_length) {
    error(ERROR_INCOMPLETE_WRITEN);
    return false;
  }

  return true;
}

void print_log(char *message, uint32_t message_size, bool flush, char *BUFFER,
               size_t *BUFFER_INDEX) {
  if (message_size + *BUFFER_INDEX > BUFFER_SIZE) {
    ssize_t written = write(STDOUT_FILENO, BUFFER, *BUFFER_INDEX);
    fflush(stdout);

    if (!valid_write(written, *BUFFER_INDEX)) {
      error(ERROR_WRITE);
    }

    *BUFFER_INDEX = 0;
  } else {
    memcpy(BUFFER + *BUFFER_INDEX, message, message_size);
    *BUFFER_INDEX += message_size;
  }

  if (flush) {
    ssize_t written = write(STDOUT_FILENO, BUFFER, *BUFFER_INDEX);
    fflush(stdout);

    if (!valid_write(written, *BUFFER_INDEX)) {
      error(ERROR_WRITE);
    }

    *BUFFER_INDEX = 0;
  }
}

std::pair<std::string, long long> get_current_time() {
  auto now = std::chrono::system_clock::now();

  std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);

  std::tm now_tm = *std::localtime(&now_time_t);

  auto duration_since_epoch = now.time_since_epoch();
  auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(
                    duration_since_epoch) %
                1000;
  long long exact_time = std::chrono::duration_cast<std::chrono::milliseconds>(
                             now.time_since_epoch())
                             .count();

  std::ostringstream oss;
  oss << std::put_time(&now_tm, "%Y-%m-%dT%H:%M:%S") << '.' << std::setw(3)
      << std::setfill('0') << millis.count();

  return {oss.str(), exact_time};
}

// Sets the peer address and port based on the socket file descriptor.
void set_peer_address(int socket_fd, NODE &node) {
  struct sockaddr_storage peer_address;
  socklen_t peer_address_length = sizeof(peer_address);

  if (getpeername(socket_fd, (struct sockaddr *)&peer_address,
                  &peer_address_length) == -1) {
    syserr("getpeername");
  }

  char peer_address_string[INET6_ADDRSTRLEN];
  int peer_port;

  if (peer_address.ss_family == AF_INET) {  // IPv4
    struct sockaddr_in *ipv4 = (struct sockaddr_in *)&peer_address;
    inet_ntop(AF_INET, &(ipv4->sin_addr), peer_address_string, INET_ADDRSTRLEN);
    peer_port = ntohs(ipv4->sin_port);
  } else if (peer_address.ss_family == AF_INET6) {  // IPv6
    struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)&peer_address;
    inet_ntop(AF_INET6, &(ipv6->sin6_addr), peer_address_string,
              INET6_ADDRSTRLEN);
    peer_port = ntohs(ipv6->sin6_port);
  }

  node.address = std::string(peer_address_string);
  node.port = peer_port;
}

// Sets own address and port based on the socket file descriptor.
void set_own_address(int socket_fd, NODE &node) {
  struct sockaddr_storage own_address;
  socklen_t own_address_length = sizeof(own_address);

  if (getsockname(socket_fd, (struct sockaddr *)&own_address,
                  &own_address_length) == -1) {
    syserr("getsockname");
  }

  char own_address_string[INET6_ADDRSTRLEN];
  int own_port;

  if (own_address.ss_family == AF_INET) {  // IPv4
    struct sockaddr_in *ipv4 = (struct sockaddr_in *)&own_address;
    inet_ntop(AF_INET, &(ipv4->sin_addr), own_address_string, INET_ADDRSTRLEN);
    own_port = ntohs(ipv4->sin_port);
  } else {  // IPv6
    struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)&own_address;
    inet_ntop(AF_INET6, &(ipv6->sin6_addr), own_address_string,
              INET6_ADDRSTRLEN);
    own_port = ntohs(ipv6->sin6_port);
  }

  node.address = std::string(own_address_string);
  node.port = own_port;
}