#include <stdlib.h>

#ifndef __UTILS_H
#define __UTILS_H

#define LOG_ERR(err_msg) fprintf(stderr, "ERROR: %s" err_msg "\n", __func__)

#define LOG_ABORT(fmt)                                                         \
    do {                                                                       \
        LOG_ERR(fmt);                                                          \
        exit(1);                                                               \
    } while ( 0 )

#define RETURN_ERR(fmt, ret)                                                   \
    do {                                                                       \
        LOG_ERR(fmt);                                                          \
        return ret;                                                            \
    } while ( 0 )

/* assertions are used for logical program constraints and not runtime errors */

#define ASSERT_MATRIX_DIM(matrix_ptr, rows, cols)                              \
    do {                                                                       \
        if ( matrix_ptr->num_rows != rows || matrix_ptr->num_cols != cols )    \
            LOG_ABORT("Matrix has wrong dimensions");                          \
    } while ( 0 )

#define ASSERT_SQUARE_MATRIX(matrix_ptr)                                       \
    ASSERT_MATRIX_DIM(matrix_ptr, matrix_ptr->num_rows, matrix_ptr->num_rows)

#endif /* __UTILS_H */
