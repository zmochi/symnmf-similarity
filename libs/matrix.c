#include "matrix.h"
#include "utils.h"
#include <math.h>
#include <stdlib.h>
/*
int main(void) {
    m_index         i, j, rows, cols;
    matrix         *my_mat;
    matrix_element *vec;
    matrix_element *get_vec;
    matrix         *m1, *m2, *result;

    rows = 5;
    cols = 5;

    my_mat = get_empty_matrix(rows, cols);

    set_matrix_elem(my_mat, 2, 3, 54);
    printf("%f\n", get_matrix_elem(my_mat, 2, 3));

    vec = malloc(cols * sizeof(matrix_element));

    for ( j = 0; j < cols; j++ ) {
        vec[j] = j;
    }

    set_matrix_vec(my_mat, 0, vec);
    get_vec = get_matrix_vec(my_mat, 0);

    for ( j = 0; j < cols; j++ ) {
        printf("%f ", get_vec[j]);
    }
    printf("\n");

    for ( i = 0; i < rows; i++ ) {
        for ( j = 0; j < cols; j++ ) {
            printf("%f ", my_mat->data[i][j]);
        }
        printf("\n");
    }

    m1 = get_empty_matrix(2, 3);
    m2 = get_empty_matrix(3, 4);
    result = get_empty_matrix(m1->num_rows, m2->num_cols);
    for ( i = 0; i < m1->num_rows; i++ ) {
        for ( j = 0; j < m1->num_cols; j++ ) {
            set_matrix_elem(m1, i, j, i + j);
        }
    }

    for ( i = 0; i < m2->num_rows; i++ ) {
        for ( j = 0; j < m2->num_cols; j++ ) {
            set_matrix_elem(m2, i, j, i + j);
        }
    }
    multiply_matrices(m1, m2, result);
    for ( i = 0; i < result->num_rows; i++ ) {
        for ( j = 0; j < result->num_cols; j++ ) {
            printf("%f ", result->data[i][j]);
        }
        printf("\n");
    }

    result = get_empty_matrix(m1->num_cols, m1->num_rows);

    transpose_matrix(m1, result);
    for ( i = 0; i < m1->num_rows; i++ ) {
        for ( j = 0; j < m1->num_cols; j++ ) {
            printf("%f ", m1->data[i][j]);
        }
        printf("\n");
    }
    printf("\n");

    for ( i = 0; i < result->num_rows; i++ ) {
        for ( j = 0; j < result->num_cols; j++ ) {
            printf("%f ", result->data[i][j]);
        }
        printf("\n");
    }
    printf("\n");

    result = get_empty_matrix(m1->num_rows, m1->num_cols);
    copy_matrix(m1, result);
    for ( i = 0; i < result->num_rows; i++ ) {
        for ( j = 0; j < result->num_cols; j++ ) {
            printf("%f ", result->data[i][j]);
        }
        printf("\n");
    }

    printf("frobinuis: %f", squared_frobenius_norm(m1));

    printf("\n");

    m1 = get_empty_matrix(3, 3);
    for ( i = 0; i < m1->num_rows; i++ ) {
        m1->data[i][i] = 3;
    }
    result = get_empty_matrix(m1->num_rows, m1->num_cols);
    pow_matrix(m1, result, -0.5);
    for ( i = 0; i < result->num_rows; i++ ) {
        for ( j = 0; j < result->num_cols; j++ ) {
            printf("%f ", result->data[i][j]);
        }
        printf("\n");
    }

    free_matrix(my_mat);

    return 0;
}
*/
matrix *get_new_matrix(m_index rows, m_index cols) {
    m_index i;
    matrix *new_mat = malloc(sizeof(matrix));
    CHECK_ALLOC_FAIL(new_mat);
    new_mat->num_cols = cols;
    new_mat->num_rows = rows;
    new_mat->data = malloc(rows * sizeof(matrix_element *));
    CHECK_ALLOC_FAIL(new_mat->data);
    for ( i = 0; i < rows; i++ ) {
        new_mat->data[i] = malloc(cols * sizeof(matrix_element));
        CHECK_ALLOC_FAIL(new_mat->data[i]);
    }

    /* for checks, delete later
    for ( i = 0; i < rows; i++ ) {
          for ( j = 0; j < cols; j++ ) {
              new_mat->data[i][j] = i + j;
          }
      }
  */
    return new_mat;
}

struct matrix *get_empty_matrix(m_index rows, m_index cols) {
    m_index i, j;
    matrix *new_mat;
    new_mat = get_new_matrix(rows, cols);
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
    if ( isolate_result )
        output = get_new_matrix(result->num_rows, result->num_cols);

    ASSERT_MATRIX_DIM(m1, m1->num_rows, m2->num_rows);
    ASSERT_MATRIX_DIM(result, m1->num_rows, m2->num_cols);
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
    m_index i, j;
    ASSERT_SQUARE_MATRIX(matrix);
    ASSERT_MATRIX_DIM(result, matrix->num_rows, matrix->num_cols);
    ASSERT_DIAGONAL_MATRIX(matrix);
    copy_matrix(matrix, result);
    for ( i = 0; i < matrix->num_rows; i++ ) {
        result->data[i][i] = pow(matrix->data[i][i], power);
    }
    return 0;
}

matrix_element squared_frobenius_norm(matrix *matrix) {
    m_index        i, j;
    matrix_element res = 0;
    for ( i = 0; i < matrix->num_rows; i++ ) {
        for ( j = 0; j < matrix->num_cols; j++ ) {
            res += pow(matrix->data[i][j], 2);
        }
    }

    return res;
}
