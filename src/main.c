#include <stdio.h>
#include <stdlib.h>

#include <time.h>

#include "defines.h"

int parse_options(int argc, char **argv, params_t *params);

int start_scan(params_t params, int flags);

int main(int argc, char **argv)
{
    params_t params;
    int flags = parse_options(argc, argv, &params);

    if (flags < 0)
        return EXIT_FAILURE;

    srand(time(NULL));

    if (start_scan(params, flags) < 0)
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}