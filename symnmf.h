#ifndef __SYMNMF_H
#define __SYMNMF_H

#include "libs/matrix.h"
#include <stdlib.h>

int sym_matrix(const struct matrix *X, struct matrix *sym);

int deg_matrix(const struct matrix *sym_matrix, struct matrix *deg);

int W_matrix(const struct matrix *sym, struct matrix *W);

/**
 * @brief optimize H according to the algorithm
 *
 * @param init_H initial H of dimensions (n,k)
 * @param W normalized similarity matrix of dimensions (n,n)
 * @param beta beta parameter for calculation
 * @param epsilon epsilon convergence parameter
 * @param iter maximum number of iterations
 * @param optimized_H matrix to store optimized H in
 * @return 0 on success, 1 on error
 */
int optimize_H(const struct matrix *init_H, const struct matrix *W,
               const double beta, const double epsilon, const size_t iter,
               struct matrix *optimized_H);

#endif /* __SYMNMF_H */
