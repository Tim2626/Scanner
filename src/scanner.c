#include <stdlib.h>
#include <stdio.h>

#include "defines.h"

#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/resource.h>


int quit = 0;
void good_quit(int signum) {
    if (signum == SIGINT)
        quit = 1;
}


// FUNCTION TOOK FROM MIRAI
// https://github.com/jgamblin/Mirai-Source-Code/blob/master/mirai/bot/scanner.c
static ipv4_t get_random_ip(void) {
    uint32_t tmp;
    uint8_t o1, o2, o3, o4;

    do
    {
        tmp = rand();

        o1 = tmp & 0xff;
        o2 = (tmp >> 8) & 0xff;
        o3 = (tmp >> 16) & 0xff;
        o4 = (tmp >> 24) & 0xff;
    }
    while (o1 == 127 ||                             // 127.0.0.0/8      - Loopback
          (o1 == 0) ||                              // 0.0.0.0/8        - Invalid address space
          (o1 == 3) ||                              // 3.0.0.0/8        - General Electric Company
          (o1 == 15 || o1 == 16) ||                 // 15.0.0.0/7       - Hewlett-Packard Company
          (o1 == 56) ||                             // 56.0.0.0/8       - US Postal Service
          (o1 == 10) ||                             // 10.0.0.0/8       - Internal network
          (o1 == 192 && o2 == 168) ||               // 192.168.0.0/16   - Internal network
          (o1 == 172 && o2 >= 16 && o2 < 32) ||     // 172.16.0.0/14    - Internal network
          (o1 == 100 && o2 >= 64 && o2 < 127) ||    // 100.64.0.0/10    - IANA NAT reserved
          (o1 == 169 && o2 > 254) ||                // 169.254.0.0/16   - IANA NAT reserved
          (o1 == 198 && o2 >= 18 && o2 < 20) ||     // 198.18.0.0/15    - IANA Special use
          (o1 >= 224) ||                            // 224.*.*.*+       - Multicast
          (o1 == 6 || o1 == 7 || o1 == 11 || o1 == 21 || o1 == 22 || o1 == 26 || o1 == 28 || o1 == 29 || o1 == 30 || o1 == 33 || o1 == 55 || o1 == 214 || o1 == 215) // Department of Defense
    );

    return INET_ADDR(o1,o2,o3,o4);
}

int start_scan(params_t params, int flags) {
    uint32_t number = 0;
    SOCKET   *sock;
    POLLFD   *pollfd;
    SOCKADDR addr;
    struct rlimit limit;
    int r;
    ipv4_t  *ips;
    struct in_addr tmp;

    if (flags & FLAG_R)
        params.start = 0;
    if (!(flags & FLAG_P))
        params.port = 80;
    if (!(flags & FLAG_N))
        params.number = 1;
    if (!(flags & FLAG_T))
        params.timeout = 0;

    sock = malloc(sizeof(SOCKET) * params.number);
    if (sock == NULL) {
        fprintf(stderr, "Critical error\n");
        return -1;
    }
    pollfd = malloc(sizeof(POLLFD) * params.number);
    if (pollfd == NULL) {
        fprintf(stderr, "Critical error\n");
        return -1;
    }
    ips = malloc(sizeof(ipv4_t) * params.number);
    if (ips == NULL) {
        fprintf(stderr, "Critical error\n");
        return -1;
    }


    getrlimit(RLIMIT_NOFILE, &limit);
    limit.rlim_cur = params.number;
    if (params.number > limit.rlim_max)
        limit.rlim_max = params.number;

    if (setrlimit(RLIMIT_NOFILE, &limit) < 0)
        printf("Failed change limit /!\\\n");

    signal(SIGINT, good_quit);

    for (uint64_t i = 0; i < params.number; i++) {
        sock[i] = socket(AF_INET, SOCK_STREAM, 0);
        fcntl(sock[i], F_SETFL, fcntl(sock[i], F_GETFL, 0) | O_NONBLOCK);
        pollfd[i].fd = sock[i];
        pollfd[i].events = POLLOUT;
        pollfd[i].revents = 0;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(params.port);

    for (uint64_t i = 0; i < params.number; i++) {
        if (params.start)
            addr.sin_addr.s_addr = htonl(params.start + i);
        else
            addr.sin_addr.s_addr = get_random_ip();
        ips[i] = addr.sin_addr.s_addr;
        connect(sock[i], (struct sockaddr *)&addr, sizeof(SOCKADDR));
    }

    tmp.s_addr = params.start;
    if (params.start != 0)
        printf("Start scanning %s, %ld ip on port %d\n", inet_ntoa(tmp), params.number, params.port);
    else
        printf("Start scanning %ld random ip on port %d\n", params.number, params.port);
    if (params.timeout == 0)
        printf("No timeout (CTRL-C to quit)\n\n");
    else
        printf("Timeout after %ldms (CTRL-C to quit)\n\n", params.timeout);
    do
    {
        r = poll(pollfd, params.number, params.timeout);
        if (r < 0) {
            perror("");
            quit = 1;
            break;
        }
        if (params.timeout != 0 && r == 0) {
            printf("Timeout\n");
            quit = 1;
            break;
        }
        for (uint64_t i = 0; i < params.number; i++) {
            if ((pollfd[i].revents & POLLOUT) == POLLOUT) {
                tmp.s_addr = ips[i];
                printf("%s\n", inet_ntoa(tmp));
                fflush(stdout);
                close(sock[i]);
                pollfd[i].fd = -1;
                number++;
            }
        }
        if (number == params.number)
            break;
    } while (!quit);
    
    free(pollfd);
    free(sock);
    free(ips);

    return 1;
}