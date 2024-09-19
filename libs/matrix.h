#ifndef __MATRIX_H
#define __MATRIX_H

typedef double matrix_element;
typedef int    index;

/* opaque struct for internal use, define in matrix.c */
struct private_matrix;

/* public struct so user can get dimensions of matrix */
struct matrix {
    struct private_matrix *hidden_field;
    index                  num_rows, num_cols;
};

struct matrix *get_new_matrix(index rows, index cols);

int free_matrix(struct matrix *);

/**
 * @brief sets matrix[i, j] to @elem
 * TODO: how to
 *
 * @return previous element at index [i, j]
 */
matrix_element set_matrix_elem(struct matrix *matrix, index i, index j,
                               matrix_element elem);
/**
 * @brief gets element at index [i, j]
 *
 * @return element at index [i, j]
 */
matrix_element get_matrix_elem(struct matrix *matrix, index i, index j);

/* !! make sure to handle overflow properly !! */

/**
 * @brief multiplies matrices and stores the new matrix in existing matrix
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
int pow_matrix(struct matrix *matrix, int pow);

matrix_element frobenius_norm(struct matrix *matrix);

#endif
