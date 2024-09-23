#ifndef __MATRIX_H
#define __MATRIX_H

#include <stdlib.h> /* size_t */

typedef double        matrix_element;
typedef size_t        m_index;
typedef struct matrix matrix;
/* decided to go with public struct matrix implementation, so implement here */
struct matrix {
    m_index          num_rows, num_cols;
    matrix_element **data;
};

/**
 * @brief returns new matrix filled with trash data, does not fail
 */
matrix *get_new_matrix(m_index rows, m_index cols);

/**
 * @brief returns zeroed-out new matrix, does not fail
 */
matrix *get_empty_matrix(m_index rows, m_index cols);

int free_matrix(matrix *matrix);

/**
 * @brief sets matrix[i, j] to @elem
 * TODO: how to indicate i or j out of bounds?
 *
 * @return previous element at index [i, j]
 */
matrix_element set_matrix_elem(matrix *matrix, m_index i, m_index j,
                               matrix_element elem);
/**
 * @brief gets element at index [i, j]
 * TODO: how to indicate i or j out of bounds?
 *
 * @return element at index [i, j]
 */
matrix_element get_matrix_elem(const matrix *matrix, m_index i, m_index j);

/**
 * @brief returns the i'th row/column (not sure if row or column should be a
 * vector) of the matrix as an array
 *
 * @param matrix matrix to get row/column from
 * @param i index of row/column
 * @return the i'th row/column of the matrix, free'd when its matrix is free'd
 */
matrix_element *get_matrix_vec(const matrix *matrix, m_index i);

/**
 * @brief sets a vector in matrix (either row or column)
 *
 * @param vec a malloc'd array of matrix_element to set
 * @return 0 on success, 1 on failure
 */
int set_matrix_vec(matrix *matrix, m_index i, matrix_element *vec,
                   m_index size);

/* !! make sure to handle overflow properly !! */

/**
 * @brief multiplies matrices and stores the new matrix in existing matrix
 * does this work if m1==m2==result?? does it work if m2==result || m1 ==
 * result?
 *
 * @param result matrix to store the product in
 * @return 0 on success, 1 on failure
 */
int multiply_matrices(const matrix *m1, const matrix *m2, matrix *result);

/**
 * @brief calculates m1 - m2 and stores result in last argument
 *
 * @return 0 on success, 1 on failure
 */
int subtract_matrices(const matrix *m1, const matrix *m2, matrix *result);

/**
 * @brief tranposes a matrix
 *
 * @param matrix pointer to matrix to be transposed
 * @param transposed where to store transposed matrix
 * @return 0 on success, 1 on failure
 */
int transpose_matrix(const matrix *matrix, struct matrix *transposed);

/**
 * @brief copies the matrix in first argument into the matrix pointed to by the
 * second argument
 *
 * @param original matrix to copy
 * @param copy matrix to store copy in
 * @return 0 on success, 1 on failure
 */
int copy_matrix(const matrix *original, matrix *copy);

/* !! make sure to handle overflow properly !! */

/**
 * @brief raises a matrix to power in-place
 *
 * @param matrix ptr to matrix to multiply against itself
 * @param pow exponent
 * @return 0 on success, 1 on failure
 */
int pow_matrix(matrix *matrix, struct matrix *result, double power);

matrix_element squared_frobenius_norm(matrix *matrix);

#endif
