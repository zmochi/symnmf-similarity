#include "matrix.h"
#include "utils.h"
#include <math.h>
#include <stdlib.h>

matrix *get_new_matrix(m_index rows, m_index cols) {
    m_index i;
    matrix *new_mat = malloc(sizeof(matrix));
    if ( new_mat == NULL ) {
        return NULL;
    };
    new_mat->num_cols = cols;
    new_mat->num_rows = rows;
    new_mat->data = malloc(rows * sizeof(matrix_element *));
    if ( new_mat->data == NULL ) {
        return NULL;
    };
    for ( i = 0; i < rows; i++ ) {
        new_mat->data[i] = malloc(cols * sizeof(matrix_element));
        if ( new_mat->data[i] == NULL ) {
            return NULL;
        };
    }

    return new_mat;
}

struct matrix *get_empty_matrix(m_index rows, m_index cols) {
    m_index i, j;
    matrix *new_mat;
    new_mat = get_new_matrix(rows, cols);
    if ( !new_mat ) return NULL;

    for ( i = 0; i < rows; i++ ) {
        for ( j = 0; j < cols; j++ ) {
            new_mat->data[i][j] = 0;
        }
    }
    return new_mat;
}

int free_matrix(matrix *matrix) {
    m_index i;
    for ( i = 0; i < matrix->num_rows; i++ ) {
        free(matrix->data[i]);
    }
    free(matrix->data);
    free(matrix);
    return 0;
}

matrix_element set_matrix_elem(matrix *matrix, m_index i, m_index j,
                               matrix_element elem) {
    matrix->data[i][j] = elem;
    return matrix->data[i][j];
}

matrix_element get_matrix_elem(const matrix *matrix, m_index i, m_index j) {
    return matrix->data[i][j];
}

matrix_element *get_matrix_vec(const matrix *matrix, m_index i) {
    return matrix->data[i];
}

int set_matrix_vec(struct matrix *matrix, m_index i, matrix_element *vec,
                   m_index size) {
    if ( size != matrix->num_cols ) {
        LOG_ABORT("Vector has wrong dimensions");
    }
    free(matrix->data[i]);
    matrix->data[i] = vec;
    return 0;
}

int multiply_matrices(const matrix *m1, const matrix *m2,
                      struct matrix *result) {
    m_index        i, j, k;
    struct matrix *output = result;
    int            isolate_result = (result == m1 || result == m2);
    if ( isolate_result ) {
        output = get_new_matrix(result->num_rows, result->num_cols);
        if ( !output ) return -1;
    }

    CHECK_RET_MATRIX_DIM(m1, m1->num_rows, m2->num_rows, 1);
    CHECK_RET_MATRIX_DIM(result, m1->num_rows, m2->num_cols, 1);
    for ( i = 0; i < m1->num_rows; i++ ) {
        for ( j = 0; j < m2->num_cols; j++ ) {
            output->data[i][j] = 0;

            for ( k = 0; k < m2->num_rows; k++ ) {
                output->data[i][j] += m1->data[i][k] * m2->data[k][j];
            }
        }
    }

    if ( isolate_result ) {
        copy_matrix(output, result);
        free_matrix(output);
    }

    return 0;
}
int subtract_matrices(const matrix *m1, const matrix *m2,
                      struct matrix *result) {
    m_index i, j;
    ASSERT_MATRIX_DIM(m1, m2->num_rows, m2->num_cols);
    ASSERT_MATRIX_DIM(result, m2->num_rows, m2->num_cols);
    for ( i = 0; i < m2->num_rows; i++ ) {
        for ( j = 0; j < m2->num_cols; j++ ) {
            result->data[i][j] = m1->data[i][j] - m2->data[i][j];
        }
    }

    return 0;
}

int transpose_matrix(const matrix *matrix, struct matrix *transposed) {
    m_index i, j;
    ASSERT_MATRIX_DIM(transposed, matrix->num_cols, matrix->num_rows);
    for ( i = 0; i < transposed->num_rows; i++ )
        for ( j = 0; j < transposed->num_cols; j++ )
            transposed->data[i][j] = matrix->data[j][i];
    return 0;
}
int copy_matrix(const matrix *original, matrix *copy) {
    m_index i, j;
    ASSERT_MATRIX_DIM(copy, original->num_rows, original->num_cols);
    for ( i = 0; i < original->num_rows; i++ )
        for ( j = 0; j < original->num_cols; j++ )
            copy->data[i][j] = original->data[i][j];
    return 0;
}

int pow_matrix(matrix *matrix, struct matrix *result, double power) {
    m_index i;
    ASSERT_SQUARE_MATRIX(matrix);
    ASSERT_MATRIX_DIM(result, matrix->num_rows, matrix->num_cols);
    ASSERT_DIAGONAL_MATRIX(matrix);
    copy_matrix(matrix, result);
    for ( i = 0; i < matrix->num_rows; i++ ) {
        result->data[i][i] = pow(matrix->data[i][i], power);
    }
    return 0;
}

double squared_frobenius_norm(matrix *matrix) {
    m_index i, j;
    double  res = 0;
    for ( i = 0; i < matrix->num_rows; i++ ) {
        for ( j = 0; j < matrix->num_cols; j++ ) {
            res += pow(matrix->data[i][j], 2);
        }
    }

    return res;
}
