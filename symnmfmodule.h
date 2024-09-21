/* the _ definitions make the arguments go through processing twice, for example
 * STR(MODULE) does not become "MODULE" and instead becomes the value of the
 * MODULE macro */
#define _STR(x)       #x
#define _CONCAT(x, y) x##y
#define STR(x)        _STR(x)
#define CONCAT(x, y)  _CONCAT(x, y)

#define MODULE_NAME symnmfmodule
#define MODULE_DESC "Exposes symnmf(), sym(), ddg() and norm() functions"

#define SYMNMF_METHOD_DOCSTRING                                                \
    "Arguments are, in order: \
	- The initial H matrix of dimensions (n, k) where k is the number of clusters\
	- The normalized similarity matrix W of dimensions (n,n)\
	- The Beta parameter for optimizing H\
	- The epsilon convergence parameter for optimizing H\
	- The maximum number of iterations for optimizing H\
\
	Returns the optimized H matrix."

#define GENERIC_X_MATRIX_DOCSTRING                                             \
    "Receives a single argument, a list[list[float], of length n, each "       \
    "embedded list is a vector/point of dimension d"

#define SYM_METHOD_DOCSTRING                                                   \
    GENERIC_X_MATRIX_DOCSTRING                                                 \
    "Returns the similarity matrix as a list[list[floats]]."
#define DDG_METHOD_DOCSTRING                                                   \
    GENERIC_X_MATRIX_DOCSTRING                                                 \
    "Returns the diagonal degree matrix as a list[list[float]]"
#define NORM_METHOD_DOCSTRING                                                  \
    GENERIC_X_MATRIX_DOCSTRING                                                 \
    "Returns the normalized similarity matrix as a list[list[float]]"
