#ifndef __SYMNMF_H
#define __SYMNMF_H

#include <stdlib.h>

struct matrix *sym_matrix(const struct matrix *X);

struct matrix *deg_matrix(const struct matrix *sym_matrix);

struct matrix *W_matrix(const struct matrix *sym, const struct matrix *deg);

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
                          const size_t iter);

#endif /* __SYMNMF_H */
