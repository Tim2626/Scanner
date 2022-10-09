#ifndef DEFINES_H
# define DEFINES_H

#include <stdint.h>

#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <poll.h>

typedef uint32_t  ipv4_t;
typedef uint16_t  port_t;
typedef uint64_t  timeout_t;
typedef int                 SOCKET;
typedef struct pollfd       POLLFD;
typedef struct sockaddr_in  SOCKADDR;


#include <arpa/inet.h>

#define INET_ADDR(o1,o2,o3,o4) (htonl((o1 << 24) | (o2 << 16) | (o3 << 8) | (o4 << 0)))

typedef struct params {
    port_t     port;
    ipv4_t     start;
    uint64_t   number;
    timeout_t  timeout;
} params_t;

#define FLAG_S 1
#define FLAG_N 2
#define FLAG_T 4
#define FLAG_R 8
#define FLAG_P 16

#endif
