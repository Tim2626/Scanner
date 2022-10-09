#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <getopt.h>

#include "defines.h"

int get_start_ip(char *ip, ipv4_t *ipv4) {
    long tmp;
    uint8_t o1, o2, o3, o4;

    tmp = atoi(ip);
    if (tmp > 255 || tmp < 0)
        return -1;
    o1 = tmp;
    ip = strchr(ip, '.');
    if (ip == NULL)
        return -1;
    ip++;
    tmp = atoi(ip);
    if (tmp > 255 || tmp < 0)
        return -1;
    o2 = tmp;
    ip = strchr(ip, '.');
    if (ip == NULL)
        return -1;
    ip++;
    tmp = atoi(ip);
    if (tmp > 255 || tmp < 0)
        return -1;
    o3 = tmp;
    ip = strchr(ip, '.');
    if (ip == NULL)
        return -1;
    ip++;
    tmp = atoi(ip);
    if (tmp > 255 || tmp < 0)
        return -1;
    o4 = tmp;

    *ipv4 = o4 << 24 | o3 << 16 | o2 << 8 | o1;

    return 1;
}

const char *help_msg =
"Usage: %s [OPTION]\n"
"\n"
"Options:\n"
" -s, --start            defines the first ip address to scan (default=option -r)\n"
" -n, --number           defines the number of ip address to scan (default=1)\n"
" -t, --timeout          defines a total timeout in ms (default=none)\n"
" -r, --random           scan random ip addresses, overides -s\n"
" -p, --port             defines a port to scan (default=80)\n"
"     --help             display this help and exit\n";

int parse_options(int argc, char **argv, params_t *params) {
    int ret = 0;
    int c;
    int long_index;
    int tmp;

    if (argc == 1) {
        printf(help_msg, argv[0]);
        return -1;
    }

    const struct option long_options[] = {
            {"start",   required_argument, 0, 's' },
            {"number",  required_argument, 0, 'n' },
            {"timeout", required_argument, 0, 't' },
            {"random",  no_argument,       0, 'r' },
            {"port",    required_argument, 0, 'p'},
            {"help",    no_argument,       0,  1 },
            {0,         0,                 0,  0 }
    };

    while ((c = getopt_long(argc, argv, "s:n:t:rp:", long_options, &long_index)) >= 0)
    {
        switch (c) 
        {
            case 's':
                if (get_start_ip(optarg, &(params->start)) < 0) {
                    fprintf(stderr, "-s, --start, bad start address\n");
                    return -1;
                }
                ret |= FLAG_S;
                break;
            case 'n':
                tmp = atoi(optarg);
                if (tmp <= 0) {
                    fprintf(stderr, "-n, --number, bad number to scan\n");
                    return -1;
                }
                params->number = tmp;
                ret |= FLAG_N;
                break;
            case 't':
                tmp = atoi(optarg);
                if (tmp <= 0) {
                    fprintf(stderr, "-t, --timeout, bad timeout\n");
                    return -1;
                }
                params->timeout = tmp;
                ret |= FLAG_T;
                break;
            case 'r':
                ret |= FLAG_R;
                break;
            case 'p':
                tmp = atoi(optarg);
                if (tmp <= 0 || tmp > UINT16_MAX) {
                    fprintf(stderr, "-p, --port, bad port number");
                    return -1;
                }
                params->port = tmp;
                ret |= FLAG_P;
                break;
            case 1:
                printf(help_msg, argv[0]);
                return -1;
            default:
                return -1;
        }
    }

    if (ret == 0 || !(ret & FLAG_S))
        ret |= FLAG_R;

    return ret;
}