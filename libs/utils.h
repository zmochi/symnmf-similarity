#include <stdlib.h>

#ifndef __UTILS_H
#define __UTILS_H

#define LOG_ERR(err_msg) fprintf(stderr, "ERROR: %s" err_msg "\n", __func__)

#define CHECK_ALLOC_FAIL(ptr)                                                  \
    if ( ptr == NULL ) {                                                       \
        LOG_ERR("an error has occured");                                       \
        exit(1);                                                               \
    }

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

#define ASSERT_DIAGONAL_MATRIX(matrix_ptr)                                     \
    {                                                                          \
        for ( i = 0; i < matrix->num_rows; i++ ) {                             \
            for ( j = 0; j < matrix->num_cols; j++ ) {                         \
                if ( i != j ) {                                                \
                    if ( matrix->data[i][j] != 0 ) {                           \
                        LOG_ABORT("The matrix is not diagonal, cannot raise "  \
                                  "to power");                                 \
                    }                                                          \
                }                                                              \
            }                                                                  \
        }                                                                      \
    }

#endif /* __UTILS_H */
