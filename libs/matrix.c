#include "matrix.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
int main(void) { /*for checks, delete later*/
    m_index         rows, cols, i, j;
    matrix         *my_mat;
    matrix_element *vec;
    matrix_element *get_vec;

    rows = 5;
    cols = 5;
    my_mat = get_empty_matrix(rows, cols);
    /*
    set_matrix_elem(my_mat, 2, 3, 54);
    printf("%f\n", get_matrix_elem(my_mat, 1, 3));
*/
    vec = malloc(cols * sizeof(matrix_element *));

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

    free_matrix(my_mat);
    return 0;
}

matrix *get_new_matrix(m_index rows, m_index cols) {
    m_index i;
    matrix *new_mat = malloc(sizeof(matrix));
    CHECK_ALLOC_FAIL(new_mat);
    new_mat->num_cols = cols;
    new_mat->num_rows = rows;
    new_mat->data = malloc(rows * sizeof(matrix_element *));
    CHECK_ALLOC_FAIL(new_mat->data);
    for ( i = 0; i < rows; i++ ) {
        new_mat->data[i] = malloc(cols * sizeof(matrix_element *));
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

int free_matrix(struct matrix *matrix) {
    m_index i;
    for ( i = 0; i < matrix->num_rows; i++ ) {
        free(matrix->data[i]);
    }
    free(matrix->data);
    free(matrix);
    return 0;
}

matrix_element set_matrix_elem(struct matrix *matrix, m_index i, m_index j,
                               matrix_element elem) {
    matrix->data[i][j] = elem;
    return matrix->data[i][j];
}

matrix_element get_matrix_elem(const struct matrix *matrix, m_index i,
                               m_index j) {
    return matrix->data[i][j];
}

matrix_element *get_matrix_vec(const struct matrix *matrix, m_index i) {
    return matrix->data[i];
}

int set_matrix_vec(struct matrix *matrix, m_index i, matrix_element *vec) {
    free(matrix->data[i]);
    matrix->data[i] = vec;
    return 1;
}
