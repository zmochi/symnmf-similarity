#include "libs/matrix.h"
#include "libs/utils.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>

static matrix_element vec_distance_squared(const matrix_element *x_i,
                                           const matrix_element *x_j,
                                           const index           dim) {
    int            k;
    matrix_element sum = 0;

    for ( k = 0; k < dim; k++ )
        sum += pow(x_i[k] - x_j[k], 2);

    return sum;
}

static matrix_element calc_sym_elem(const matrix_element *x_i,
                                    const matrix_element *x_j,
                                    const index           dim) {
    return exp(-vec_distance_squared(x_i, x_j, dim) / 2);
}

struct matrix *sym_matrix(const struct matrix *X) {
    int             i, j;
    index           d = X->num_rows, N = X->num_cols;
    struct matrix  *sym = get_new_matrix(N, N);
    matrix_element *x_i, *x_j;
    matrix_element  a_ij;
    for ( i = 0; i < N; i++ ) {
        x_i = get_matrix_vec(X, i);

        for ( j = 0; j < N; j++ ) {
            x_j = get_matrix_vec(X, j);

            if ( i == j )
                a_ij = 0;
            else
                a_ij = calc_sym_elem(x_i, x_j, d);

            assert(a_ij > 0);
            set_matrix_elem(sym, i, j, a_ij);
        }
    }

    return sym;
}

struct matrix *deg_matrix(const struct matrix *sym_matrix) {
    int            i, j;
    struct matrix *deg;
    index          n = sym_matrix->num_cols;
    matrix_element d_i;
    if ( sym_matrix->num_cols != sym_matrix->num_rows )
        LOG_ABORT("Sym matrix is not a square matrix");

    deg = get_empty_matrix(n, n);

    for ( i = 0; i < n; i++ ) {
        d_i = 0;

        for ( j = 0; j < n; j++ )
            d_i += get_matrix_elem(sym_matrix, i, j);

        set_matrix_elem(deg, i, i, d_i);
    }

    return deg;
}

struct matrix *W_matrix(const struct matrix *sym, const struct matrix *deg) {
    index          n = deg->num_rows;
    struct matrix *prod = get_new_matrix(n, n), *W = get_new_matrix(n, n);

    ASSERT_SQUARE_MATRIX(sym);
    ASSERT_SQUARE_MATRIX(deg);

    if ( copy_matrix(deg, prod) != 0 ) LOG_ABORT("Couldn't copy deg matrix");

    if ( pow_matrix(prod, -1.0 / 2.0) != 0 )
        LOG_ABORT("Couldn't raise left_prod to power -1/2");

    if ( multiply_matrices(prod, sym, W) != 0 )
        LOG_ABORT("Couldn't multiply matrices prod * sym into w");

    if ( multiply_matrices(W, prod, W) != 0 )
        LOG_ABORT("Couldn't multiply matrices sym * w into w");

    free_matrix(prod);

    return W;
}

/**
 * @brief helper function for calculating H(t+1)
 *
 * @param H H(t)
 * @param W normalized similarity matrix
 * @param aux_matrices an array of 3 matrices for internal use, of dimensions
 * ((n,k), (n,k), (k,n)) respectively. this avoids the overhead of allocating
 * and freeing a new matrix on each call to this function
 * @param beta beta parameter for calculation
 * @param next_H matrix to store H(t+1) in
 * @return 0 on success, 1 on failure
 */
int calc_next_H(const struct matrix *H, const struct matrix *W,
                struct matrix **aux_matrices, const double beta,
                struct matrix *next_H) {
    int            i, j;
    index          n = H->num_rows, k = H->num_cols;
    struct matrix *W_H = aux_matrices[0], *H_HT_H = aux_matrices[1],
                  *H_transpose = aux_matrices[2];

    ASSERT_MATRIX_DIM(W_H, n, k);
    ASSERT_MATRIX_DIM(H_HT_H, n, k);
    ASSERT_MATRIX_DIM(H_transpose, k, n);

    if ( transpose_matrix(H, H_transpose) != 0 )
        RETURN_ERR("Couldn't transpose H", 1);

    if ( multiply_matrices(W, H, W_H) != 0 )
        RETURN_ERR("Couldn't multiply matrices W*H", 1);

    if ( multiply_matrices(H, H_transpose, H_HT_H) != 0 )
        RETURN_ERR("Couldn't multiply matrices H*H^T", 1);

    if ( multiply_matrices(H_HT_H, H, H_HT_H) != 0 )
        RETURN_ERR("Couldn't multiply matrices (H*H^T)*H", 1);

    for ( i = 0; i < n; i++ ) {
        for ( j = 0; j < k; j++ ) {
            matrix_element H_ij = get_matrix_elem(H, i, j),
                           W_H_ij = get_matrix_elem(W_H, i, j),
                           H_HT_H_ij = get_matrix_elem(H_HT_H, i, j);

            matrix_element next_H_ij =
                H_ij * (1 - beta + beta * (W_H_ij / H_HT_H_ij));

            set_matrix_elem(next_H, i, j, next_H_ij);
        }
    }

    return 0;
}

/**
 * @brief optimize H according to the algorithm
 *
 * @param init_H initial H of dimensions (n,k)
 * @param W normalized similarity matrix of dimensions (n,n)
 * @param beta beta parameter for calculation
 * @param epsilon epsilon convergence parameter
 * @param iter maximum number of iterations
 * @return the optimized H, or NULL on error
 */
struct matrix *optimize_H(const struct matrix *init_H, const struct matrix *W,
                          const double beta, const double epsilon,
                          const size_t iter) {

    index          n = init_H->num_rows, k = init_H->num_cols;
    size_t         t = 0;
    struct matrix *updated_H = get_new_matrix(n, k),
                  *old_H = get_new_matrix(n, k), *diff_H = get_new_matrix(n, k);
    struct matrix *aux_matrices[3];

    ASSERT_SQUARE_MATRIX(W);

    aux_matrices[0] = get_new_matrix(n, k);
    aux_matrices[1] = get_new_matrix(n, k);
    aux_matrices[2] = get_new_matrix(k, n);

    if ( copy_matrix(init_H, old_H) != 0 )
        RETURN_ERR("Couldn't copy init_H into old_H", NULL);

    while ( t < iter ) {
        if ( calc_next_H(old_H, W, aux_matrices, beta, updated_H) != 0 )
            RETURN_ERR("Couldn't calculate next H", NULL);

        if ( subtract_matrices(updated_H, old_H, diff_H) != 0 )
            RETURN_ERR("Couldn't subtract matrices", NULL);

        if ( squared_frobenius_norm(diff_H) < epsilon ) break;

        t++;
    }

    free_matrix(aux_matrices[0]);
    free_matrix(aux_matrices[1]);
    free_matrix(aux_matrices[2]);
    free_matrix(old_H);
    free_matrix(diff_H);

    return updated_H;
}
