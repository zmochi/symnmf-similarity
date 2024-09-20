#include "libs/matrix.h"
#include "libs/utils.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>

static inline matrix_element vec_distance_squared(const matrix_element *x_i,
                                                  const matrix_element *x_j,
                                                  const index           dim) {
    matrix_element sum = 0;

    for ( int k = 0; k < dim; k++ )
        sum += pow(x_i[k] - x_j[k], 2);

    return sum;
}

static inline matrix_element calc_sym_elem(const matrix_element *x_i,
                                           const matrix_element *x_j,
                                           const index           dim) {
    return exp(-vec_distance_squared(x_i, x_j, dim) / 2);
}

struct matrix *sym_matrix(const struct matrix *X) {
    index           d = X->num_rows, N = X->num_cols;
    struct matrix  *sym = get_new_matrix(N, N);
    matrix_element *x_i, *x_j;
    matrix_element  a_ij;
    for ( int i; i < N; i++ ) {
        x_i = get_matrix_vec(X, i);

        for ( int j; j < N; j++ ) {
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
    index          n = sym_matrix->num_cols;
    matrix_element d_i;
    if ( sym_matrix->num_cols != sym_matrix->num_rows )
        LOG_ABORT("Sym matrix is not a square matrix");

    struct matrix *deg = get_empty_matrix(n, n);

    for ( int i = 0; i < n; i++ ) {
        d_i = 0;

        for ( int j = 0; j < n; j++ )
            d_i += get_matrix_elem(sym_matrix, i, j);

        set_matrix_elem(deg, i, i, d_i);
    }

    return deg;
}

struct matrix *w_matrix(const struct matrix *sym, const struct matrix *deg) {
    if ( sym->num_cols != sym->num_rows )
        LOG_ABORT("Sym matrix is not a square matrix");
    if ( deg->num_cols != deg->num_rows )
        LOG_ABORT("Deg matrix is not a square matrix");

    index          n = deg->num_rows;
    struct matrix *prod = get_new_matrix(n, n), *w = get_new_matrix(n, n);

    if ( copy_matrix(deg, prod) != 0 ) LOG_ABORT("Couldn't copy deg matrix");

    if ( pow_matrix(prod, -1.0 / 2.0) != 0 )
        LOG_ABORT("Couldn't raise left_prod to power -1/2");

    if ( multiply_matrices(prod, sym, w) != 0 )
        LOG_ABORT("Couldn't multiply matrices prod * sym into w");

    if ( multiply_matrices(w, prod, w) != 0 )
        LOG_ABORT("Couldn't multiply matrices sym * w into w");

    if ( free_matrix(prod) != 0 ) LOG_ABORT("Couldn't free prod matrix");

    return w;
}
