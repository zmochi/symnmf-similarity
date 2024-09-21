#include "matrix.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
int main(void) { /*for checks, delete later*/
    m_index rows, cols, i, j;
    matrix *my_mat;
    rows = 5;
    cols = 5;
    my_mat = get_empty_matrix(rows, cols);
    for ( i = 0; i < rows; i++ ) {
        for ( j = 0; j < cols; j++ ) {
            printf("%f ", my_mat->data[i][j]);
        }
        printf("\n");
    }
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

int free_matrix(struct matrix *);
