#include <stdlib.h>

#ifndef __UTILS_H
#define __UTILS_H

#define LOG_ERR(fmt, ...) fprintf(stderr, "ERROR: " fmt "\n", ##__VA_ARGS__)

#define LOG_ABORT(fmt, ...)                                                    \
    do {                                                                       \
        LOG_ERR(fmt, ##__VA_ARGS__);                                           \
        exit(1);                                                               \
    } while ( 0 )

#endif /* __UTILS_H */
