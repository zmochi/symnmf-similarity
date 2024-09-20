#ifndef __MATRIX_H
#define __MATRIX_H

typedef double matrix_element;
typedef int    index;

/* opaque struct for internal use, define in matrix.c */
struct private_matrix;

/* public struct so user can get dimensions of matrix */
struct matrix {
    struct private_matrix *internal;
    index                  num_rows, num_cols;
};

/**
 * @brief returns new matrix filled with trash data, does not fail
 */
struct matrix *get_new_matrix(index rows, index cols);

/**
 * @brief returns zeroed-out new matrix, does not fail
 */
struct matrix *get_empty_matrix(index rows, index cols);

int free_matrix(struct matrix *);

/**
 * @brief sets matrix[i, j] to @elem
 * TODO: how to indicate i or j out of bounds?
 *
 * @return previous element at index [i, j]
 */
matrix_element set_matrix_elem(struct matrix *matrix, index i, index j,
                               matrix_element elem);
/**
 * @brief gets element at index [i, j]
 * TODO: how to indicate i or j out of bounds?
 *
 * @return element at index [i, j]
 */
matrix_element get_matrix_elem(const struct matrix *matrix, index i, index j);

/**
 * @brief returns the i'th column of the matrix as an array
 *
 * @param matrix matrix to get column from
 * @param i index of column
 * @return the i'th column of the matrix, free'd when its matrix is free'd
 */
matrix_element *get_matrix_vec(const struct matrix *matrix, index i);

/* !! make sure to handle overflow properly !! */

/**
 * @brief multiplies matrices and stores the new matrix in existing matrix
 * does this work if m1==m2==result?? does it work if m2==result || m1 ==
 * result?
 *
 * @param result matrix to store the product in
 * @return 0 on success, 1 on failure
 */
int multiply_matrices(const struct matrix *m1, const struct matrix *m2,
                      struct matrix *result);

/**
 * @brief tranposes a matrix in-place
 *
 * @param matrix pointer to matrix to be transposed
 * @return 0 on success, 1 on failure
 */
int transpose_matrix(struct matrix *matrix);

/**
 * @brief copies the matrix in first argument into the matrix pointed to by the
 * second argument
 *
 * @param original matrix to copy
 * @param copy matrix to store copy in
 * @return 0 on success, 1 on failure
 */
int copy_matrix(const struct matrix *original, struct matrix *copy);

/* !! make sure to handle overflow properly !! */

/**
 * @brief raises a matrix to power in-place
 *
 * @param matrix ptr to matrix to multiply against itself
 * @param pow exponent
 * @return 0 on success, 1 on failure
 */
int pow_matrix(struct matrix *matrix, double pow);

matrix_element frobenius_norm(struct matrix *matrix);

#endif
