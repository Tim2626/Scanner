#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <poll.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>

char	*ft_itoa(int n);

typedef int SOCKET;
typedef struct pollfd POLLFD;
typedef struct sockaddr_in SOCKADDR;

typedef struct ip_t {
    uint8_t a;
    uint8_t b;
    uint8_t c;
    uint8_t d;
} ip_t;

char *ip_construct(ip_t ip)
{
    char *ret = malloc(16);
    char *tmp;

    memset(ret, 0, 16);
    tmp = ft_itoa((int)ip.a);
    strcat(ret, tmp);
    free(tmp);
    strcat(ret, ".");
    tmp = ft_itoa((int)ip.b);
    strcat(ret, tmp);
    free(tmp);
    strcat(ret, ".");
    tmp = ft_itoa((int)ip.c);
    strcat(ret, tmp);
    free(tmp);
    strcat(ret, ".");
    tmp = ft_itoa((int)ip.d);
    strcat(ret, tmp);
    free(tmp);
    return ret;
}

void incr_ip(ip_t *ip)
{
    ip->d++;
    if (ip->d == 0) {
        ip->c++;
        if (ip->c == 0) {
            ip->b++;
            if (ip->b == 0) {
                ip->a++;
            }
        }
    }
}

int in_index(int search, int *index, int nb_index)
{
    for (int i = 0; i < nb_index; i++) {
        if (index[i] == search)
            return 1;
    }
    return 0;
}

int quit = 0;

void good_quit(int signum)
{
    if (signum == SIGINT) {
        printf("User terminating scanning\n");
        quit = 1;
    }
}

int contructe_first_ip(char *str, ip_t *ip)
{
    int tmp;

    for (int i = 0; str[i]; i++) {
        if (!isdigit(str[i]) && str[i] != '.')
            return -1;
    }

    tmp = atoi(str);
    if (tmp <= 0 || tmp > 255)
        return -1;
    ip->a = tmp;
    str = strchr(str, '.');
    if (str == NULL)
        return -1;
    str++;
    tmp = atoi(str);
    if (tmp < 0 || tmp > 255)
        return -1;
    ip->b = tmp;
    str = strchr(str, '.');
    if (str == NULL)
        return -1;
    str++;
    tmp = atoi(str);
    if (tmp < 0 || tmp > 255)
        return -1;
    ip->c = tmp;
    str = strchr(str, '.');
    if (str == NULL)
        return -1;
    str++;
    tmp = atoi(str);
    if (tmp < 0 || tmp > 255)
        return -1;
    ip->d = tmp;
    if (strchr(str, '.') != NULL)
        return -1;
    return 1;
}

int main(int argc, char **argv)
{
    SOCKET *sock;
    POLLFD *pollfd;
    char **addresse;
    SOCKADDR addr;
    ip_t ip;
    int socket_flags;
    int *index;
    int nb_index = 0;
    int nb_to_scan;
    uint16_t port;
    int timeout;
    int r;

    if (argc != 5) {
        printf("Usage %s : <first address> <port> <number to scan> <timeout ms(0 = NO)>\n", argv[0]);
        return 1;
    }

    if (contructe_first_ip(argv[1], &ip) < 0) {
        printf("Bad address\n");
        return 1;
    }

    port = atoi(argv[2]);
    if (port == 0) {
        printf("Bad port\n");
        return 1;
    }

    nb_to_scan = atoi(argv[3]);
    if (nb_to_scan <= 0) {
        printf("Bad number to scan\n");
        return 1;
    }

    timeout = atoi(argv[4]);

    sock = malloc(nb_to_scan * sizeof(SOCKET));
    if (sock == NULL) {
        printf("Failed malloc\n");
        return 1;
    }
    pollfd = malloc(nb_to_scan * sizeof(POLLFD));
    if (pollfd == NULL) {
        printf("Failled malloc\n");
        free(sock);
        return 1;
    }
    addresse = malloc(nb_to_scan * sizeof(char *));
    if (addresse == NULL) {
        printf("Failed malloc\n");
        free(pollfd);
        free(sock);
    }
    index = malloc(nb_to_scan * sizeof(int));
    if (index  == NULL) {
        printf("Failed malloc\n");
        free(pollfd);
        free(sock);
        free(addresse);
        return 1;
    }

    signal(SIGINT, good_quit);

    memset(&addr, 0, sizeof(SOCKADDR));
    memset(sock, 0, nb_to_scan * sizeof(SOCKET));
    memset(pollfd, 0, nb_to_scan * sizeof(POLLFD));
    memset(index, 0, nb_to_scan * sizeof(int));


    for (int i = 0; i < nb_to_scan; i++) {
        sock[i] = socket(AF_INET, SOCK_STREAM, 0);
        socket_flags = fcntl(sock[i], F_GETFL, 0);
        fcntl(sock[i], F_SETFL, socket_flags | O_NONBLOCK);
        pollfd[i].fd = sock[i];
        pollfd[i].events = POLLOUT;
        pollfd[i].revents = 0;
    }

    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;

    for (int i = 0; i < nb_to_scan; i++) {
        addresse[i] = ip_construct(ip);
        addr.sin_addr.s_addr = inet_addr(addresse[i]);
        connect(sock[i], (struct sockaddr *)&addr, sizeof(SOCKADDR));
        incr_ip(&ip);
    }

    printf("Start scanning on %s to %s on port %d, timeout = %dms\n\n", addresse[0], addresse[nb_to_scan-1], port, timeout);

    do
    {
        r = poll(pollfd, nb_to_scan, timeout);
        if (r < 0) {
            printf("Unexcepted error, try again\n");
            quit = 1;
            break;
        }
        if (r == 0) {
            printf("Timeout\n");
            quit = 1;
            break;
        }
        for (int i = 0; i < nb_to_scan; i++) {
            if ((pollfd[i].revents & POLLOUT) == POLLOUT && !in_index(i, index, nb_index)) {
                printf("Received from %s\n", addresse[i]);
                fflush(stdout);
                index[nb_index] = i;
                nb_index++;
                pollfd[i].fd = -1;
            }
        }
    } while (!quit);

    free(sock);
    free(index);
    free(pollfd);
    for (int i = 0; i < nb_to_scan; i++)
        free(addresse[i]);
    free(addresse);
    printf("Process cleaned quiting\n");
    return 0;
}