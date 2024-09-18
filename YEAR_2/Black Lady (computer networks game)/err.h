#ifndef ERR_H
#define ERR_H


/* Error defines */
#define ERROR_MISSING_HOST "Missing host after -h"
#define ERROR_HOST_TOO_LONG "Host name too long"
#define ERROR_MISSING_PORT "Missing port after -p"
#define ERROR_INVALID_PARAMETER "Invalid parameter: %s"
#define ERROR_MISSING_HOST_PARAMETER "Missing host parameter"
#define ERROR_MISSING_PORT_PARAMETER "Missing port parameter"
#define ERROR_MISSING_SEAT_PARAMETER "Missing seat parameter"
#define ERROR_MISSING_FILE "Missing file after -f"
#define ERROR_MISSING_TIMEOUT "Missing timeout after -t"
#define ERROR_NEGATIVE_TIMEOUT "Timeout cannot be negative"
#define ERROR_MISSING_FILE_PARAMETER "Missing file parameter"
#define ERROR_READ "Failed to read"

#define ERROR_CREATE_SOCKET "Failed to create socket"
#define ERROR_CONNECT "Failed to connect to server"
#define ERROR_BIND "Failed to bind socket"
#define ERROR_WRITE_STDOUT "Failed to write to stdout"

#define ERROR_SEND_MESSAGE "Failed to send message"
#define ERROR_CLOSE_SOCKET "Failed to close socket"
#define ERROR_WRITE "Failed to write to socket"
#define ERROR_INCOMPLETE_WRITEN "Failed to write all bytes to socket"
#define ERROR_RECEIVE_MESSAGE "Failed to receive message"
#define ERROR_POLL "Failed to poll"
#define ERROR_LISTEN "unable to listen"
#define ERROR_ACCEPT "unable to accept"
#define ERROR_SETSOCKOPT "unable to set socket options"
#define ERROR_OPEN_FILE "unable to open file"
#define ERROR_FCNTL "unable to set file control options"
#define ERROR_EINTR "interrupted by a signal"


// Print information about a system error and quits.
[[noreturn]] void syserr(const char *fmt, ...);

// Print information about an error and quits.
[[noreturn]] void fatal(const char *fmt, ...);

// Print information about an error and return.
void error(const char *fmt, ...);

#endif
