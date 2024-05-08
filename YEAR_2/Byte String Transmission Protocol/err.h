#ifndef ERR_H
#define ERR_H

#include <stdnoreturn.h>

/* Error messages */
#define ERROR_READ "unable to read from stdin"
#define ERROR_MALLOC "unable to malloc"
#define ERROR_WRONG_PACKET_TYPE_ID "wrong packet type id"
#define ERROR_WRONG_PROTOCOL_ID "wrong protocol id"
#define ERROR_CREATE_SOCKET "unable to create a socket"
#define ERROR_CONNECTION_CLOSED "connection closed"
#define ERROR_UNKNOWN_PROTOCOL "unknown protocol"
#define ERROR_WRONG_SESSION_ID "wrong session id"
#define ERROR_REJECTED "rejected packet"
#define ERROR_WRONG_PACKET_NUMBER "wrong packet number"
#define ERROR_TIMEOUT "timeout"
#define ERROR_BIND "unable to bind"
#define ERROR_WRITE "unable to write to stdout"

#define ERROR_INCOMPLETE_READN "incomplete readn"
#define ERROR_INCOMPLETE_WRITEN "incomplete writen"
#define ERROR_READN "unable to read from the server/client"
#define ERROR_INVALID_READN "invalid readn"
#define ERROR_TCP_ACCEPT_CONNECTION "accepting connection error"
#define ERROR_CONNECT "unable to connect to the server"
#define ERROR_WRITEN "unable to write to the server"

#define ERROR_INCOMPLETE_SENDTO "incomplete sendto"
#define ERROR_SENDTO "invalid sendto"
#define ERROR_RECVFROM "recvfrom error"
#define ERROR_INVALID_RECVFROM "invalid recvfrom"
#define ERROR_CONRJT "connection rejected"
#define ERROR_RETRANSMITS_EXCEEDED "retransmits exceeded"
#define ERROR_WRONG_ADDRESS "wrong address"
#define ERROR_FAILED_SEND_CONN "failed to send connection"

// Print information about a system error and quits.
noreturn void syserr(const char *fmt, ...);

// Print information about an error and quits.
noreturn void fatal(const char *fmt, ...);

// Print information about an error and return.
void error(const char *fmt, ...);

#endif
