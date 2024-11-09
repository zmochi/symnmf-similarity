#include <stdio.h>
#include <stdlib.h>

#ifndef __UTILS_H
#define __UTILS_H

#if __STDC_VERSION__ < 199901L
#if __GNUC__ >= 2
#define __func__ __FUNCTION__
#else
#define __func__ "<unknown>"
#endif /* __GNUC__ */
#endif /* __STDC_VERSION__ */

#define PRINT_GENERIC_ERR printf("An Error Has Occurred\n")

#define LOG_ERR(err_msg) PRINT_GENERIC_ERR;
/*fprintf(stderr, "ERROR: at line %d: " err_msg "\n", __LINE__)*/
#define LOG_ERR1(fmt, arg)                                                     \
    PRINT_GENERIC_ERR; /*fprintf(stderr, "ERROR: " fmt "\n", arg)*/
#define LOG1(fmt, arg) /*do nothing*/ /*printf(fmt "\n", arg)*/

#define LOG_ABORT(fmt)                                                         \
    do {                                                                       \
        LOG_ERR(fmt);                                                          \
        exit(1);                                                               \
    } while ( 0 )

#define CHECK_ALLOC_FAIL(ptr)                                                  \
    if ( ptr == NULL ) {                                                       \
        return NULL;                                                           \
    }

#define RETURN_ERR(fmt, ret)                                                   \
    do {                                                                       \
        LOG_ERR(fmt);                                                          \
        return ret;                                                            \
    } while ( 0 )

#define CHECK_RET_MATRIX_DIM(matrix_ptr, rows, cols, retval)                   \
    do {                                                                       \
        if ( matrix_ptr->num_rows != rows || matrix_ptr->num_cols != cols ) {  \
            LOG_ERR("Matrix has wrong dimensions");                            \
            return retval;                                                     \
        }                                                                      \
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
        unsigned int i, j;                                                     \
        for ( i = 0; i < matrix_ptr->num_rows; i++ ) {                         \
            for ( j = 0; j < matrix_ptr->num_cols; j++ ) {                     \
                if ( i != j ) {                                                \
                    if ( matrix_ptr->data[i][j] != 0 ) {                       \
                        LOG_ABORT("The matrix is not diagonal, cannot raise "  \
                                  "to power");                                 \
                    }                                                          \
                }                                                              \
            }                                                                  \
        }                                                                      \
    }

#endif /* __UTILS_H */
