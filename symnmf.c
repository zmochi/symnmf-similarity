#include "libs/matrix.h"
#include "libs/utils.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>

static const int success = 0, err = 1;

static matrix_element vec_distance_squared(const matrix_element *x_i,
                                           const matrix_element *x_j,
                                           const m_index         dim) {
    size_t         k;
    matrix_element sum = 0;

    for ( k = 0; k < dim; k++ )
        sum += pow(x_i[k] - x_j[k], 2);

    return sum;
}

static matrix_element calc_sym_elem(const matrix_element *x_i,
                                    const matrix_element *x_j,
                                    const m_index         dim) {
    return exp(-vec_distance_squared(x_i, x_j, dim) / 2);
}

int sym_matrix(const struct matrix *X, struct matrix *sym) {
    size_t          i, j;
    m_index         d = X->num_cols, N = X->num_rows;
    matrix_element *x_i, *x_j;
    matrix_element  a_ij;

    ASSERT_MATRIX_DIM(sym, N, N);

    for ( i = 0; i < N; i++ ) {
        x_i = get_matrix_vec(X, i);

        for ( j = i; j < N; j++ ) {
            x_j = get_matrix_vec(X, j);

            if ( i == j )
                a_ij = 0;
            else {
                a_ij = calc_sym_elem(x_i, x_j, d);
                assert(a_ij > 0);
            }

            set_matrix_elem(sym, i, j, a_ij);
        }

        /* matrix is symmetric, set matching elements that were already
         * calculated */
        for ( j = i; j < N; j++ ) {
            set_matrix_elem(sym, j, i, get_matrix_elem(sym, i, j));
        }
    }

    return success;
}

int deg_matrix(const struct matrix *sym_matrix, struct matrix *deg) {
    size_t         i, j;
    m_index        n = sym_matrix->num_rows;
    matrix_element d_i;

    ASSERT_SQUARE_MATRIX(sym_matrix);
    ASSERT_MATRIX_DIM(deg, n, n);

    for ( i = 0; i < n; i++ ) {
        d_i = 0;

        for ( j = 0; j < n; j++ )
            d_i += get_matrix_elem(sym_matrix, i, j);

        set_matrix_elem(deg, i, i, d_i);
    }

    return success;
}

int W_matrix(const struct matrix *sym, struct matrix *W) {
    m_index        n = sym->num_rows;
    struct matrix *prod = get_new_matrix(n, n), *deg = get_empty_matrix(n, n);

#define free_w                                                                 \
    do {                                                                       \
        free_matrix(prod);                                                     \
        free_matrix(deg);                                                      \
    } while ( 0 )

    if ( deg_matrix(sym, deg) != 0 ) {
        RETURN_ERR("Couldn't calculate degree matrix", err);
        free_w;
    }

    if ( copy_matrix(deg, prod) != 0 ) {
        RETURN_ERR("Couldn't copy deg matrix", err);
        free_w;
    }

    if ( pow_matrix(prod, prod, -1.0 / 2.0) != 0 ) {
        RETURN_ERR("Couldn't raise left_prod to power -1/2", err);
        free_w;
    }

    if ( multiply_matrices(prod, sym, W) != 0 ) {
        RETURN_ERR("Couldn't multiply matrices prod * sym into w", err);
        free_w;
    }

    if ( multiply_matrices(W, prod, W) != 0 ) {
        RETURN_ERR("Couldn't multiply matrices sym * w into w", err);
        free_w;
    }

    return success;
}

/**
 * @brief helper function for calculating H(t+1)
 *
 * @param H H(t)
 * @param W normalized similarity matrix
 * @param aux_matrices an array of 4 matrices for internal use, of dimensions
 * ((n,k), (k,n), (n,n), (n,k)) respectively. this avoids the overhead of
 * allocating and freeing a new matrix on each call to this function
 * @param beta beta parameter for calculation
 * @param next_H matrix to store H(t+1) in
 * @return 0 on success, 1 on failure
 */
static int calc_next_H(const struct matrix *H, const struct matrix *W,
                       struct matrix **aux_matrices, const double beta,
                       struct matrix *next_H) {
    size_t         i, j;
    m_index        n = H->num_rows, k = H->num_cols;
    struct matrix *W_H = aux_matrices[0], *H_HT_H = aux_matrices[3],
                  *HT = aux_matrices[1], *H_HT = aux_matrices[2];

    ASSERT_MATRIX_DIM(W_H, n, k);
    ASSERT_MATRIX_DIM(HT, k, n);
    ASSERT_MATRIX_DIM(H_HT, n, n);
    ASSERT_MATRIX_DIM(H_HT_H, n, k);

    if ( transpose_matrix(H, HT) != 0 ) RETURN_ERR("Couldn't transpose H", err);

    if ( multiply_matrices(W, H, W_H) != 0 )
        RETURN_ERR("Couldn't multiply matrices W*H", err);

    if ( multiply_matrices(H, HT, H_HT) != 0 )
        RETURN_ERR("Couldn't multiply matrices H*H^T", err);

    if ( multiply_matrices(H_HT, H, H_HT_H) != 0 )
        RETURN_ERR("Couldn't multiply matrices (H*H^T)*H", err);

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

    return success;
}

int optimize_H(const struct matrix *init_H, const struct matrix *W,
               const double beta, const double epsilon, const size_t iter,
               struct matrix *updated_H) {

    m_index        n = init_H->num_rows, k = init_H->num_cols;
    size_t         t = 0;
    struct matrix *old_H = get_new_matrix(n, k), *diff_H = get_new_matrix(n, k);
    struct matrix *aux_matrices[4];

    ASSERT_SQUARE_MATRIX(W);

    aux_matrices[0] = get_new_matrix(n, k);
    aux_matrices[1] = get_new_matrix(k, n);
    aux_matrices[2] = get_new_matrix(n, n);
    aux_matrices[3] = get_new_matrix(n, k);
#define free_optimize_H                                                        \
    do {                                                                       \
        free_matrix(aux_matrices[0]);                                          \
        free_matrix(aux_matrices[1]);                                          \
        free_matrix(aux_matrices[2]);                                          \
        free_matrix(aux_matrices[3]);                                          \
        free_matrix(old_H);                                                    \
        free_matrix(diff_H);                                                   \
    } while ( 0 )

    if ( copy_matrix(init_H, old_H) != 0 ) {
        RETURN_ERR("Couldn't copy init_H into old_H", 1);
        free_optimize_H;
    }

    while ( t < iter ) {
        if ( calc_next_H(old_H, W, aux_matrices, beta, updated_H) != 0 ) {
            RETURN_ERR("Couldn't calculate next H", err);
            free_optimize_H;
        }

        if ( subtract_matrices(updated_H, old_H, diff_H) != 0 ) {
            RETURN_ERR("Couldn't subtract matrices", err);
            free_optimize_H;
        }

        if ( copy_matrix(updated_H, old_H) != 0 ) {
            RETURN_ERR("Couldn't copy updated H into old H", err);
            free_optimize_H;
        }

        if ( squared_frobenius_norm(diff_H) < epsilon ) break;

        t++;
    }

    free_optimize_H;

    return success;
}
