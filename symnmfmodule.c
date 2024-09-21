#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "libs/matrix.h"
#include "symnmf.h"
#include "symnmfmodule.h"

#define HANDLE_PY_FAIL(ret)                                                    \
    do {                                                                       \
        if ( !ret ) return NULL;                                               \
    } while ( 0 )

/**
 * @brief converts python matrix (list of lists of float) to C matrix (struct
 * matrix*)
 *
 * @param py_matrix python matrix to convert
 * @param matrix matrix to store conversion in
 * @return 0 on success, 1 on failure
 */
static int py_matrix_to_c(PyObject *py_matrix, struct matrix *matrix) {
    int       success = 0, err = 1;
    PyObject *py_row_i, *py_matrix_elem;
    m_index   py_rows = PyList_Size(py_matrix);

    py_row_i = PyList_GetItem(py_matrix, 0);
    /* if PyList_GetItem fails, IndexError is set. so no need to set err
     * msg */
    HANDLE_PY_FAIL(py_row_i);

    m_index py_cols = PyList_Size(py_row_i);

    if ( py_rows != matrix->num_rows || py_cols != matrix->num_cols ) {
        PyErr_SetString(
            PyExc_ValueError,
            "C code: py_matrix and C matrix have mismatching dimensions");
        return err;
    }

    for ( size_t i = 0; i < py_rows; i++ ) {
        for ( size_t j = 0; j < py_cols; j++ ) {
            py_row_i = PyList_GetItem(py_matrix, i);
            HANDLE_PY_FAIL(py_row_i);

            if ( (m_index)PyList_Size(py_row_i) != py_cols ) {
                PyErr_SetString(PyExc_ValueError,
                                "Python matrix lists have differing lengths");
                return err;
            }

            py_matrix_elem = PyList_GetItem(py_row_i, j);
            HANDLE_PY_FAIL(py_matrix_elem);

            if ( !PyFloat_CheckExact(py_matrix_elem) ) {
                PyErr_SetString(PyExc_ValueError,
                                "Python matrix contains non-float element");
                return err;
            }

            set_matrix_elem(matrix, i, j, PyFloat_AS_DOUBLE(py_matrix_elem));
        }
    }

    return success;
}

/**
 * @brief converts a struct matrix* to a python matrix where each row in the
 * matrix becomes a python list
 *
 * @param matrix matrix to convert
 * @return the python list on success, on error sets python error string and
 * returns NULL
 */
static PyObject *c_matrix_to_py(struct matrix *matrix) {
    int       status;
    m_index   num_rows = matrix->num_rows, num_cols = matrix->num_cols;
    PyObject *main_pylist = PyList_New(num_rows), *row_i, *py_elem;

    if ( !main_pylist ) {
        PyErr_SetString(PyExc_RuntimeError, "Couldn't create new python list");
        return NULL;
    }

    for ( m_index i = 0; i < num_rows; i++ ) {
        row_i = PyList_New(num_cols);
        if ( !row_i ) {
            PyErr_SetString(PyExc_RuntimeError,
                            "Couldn't create new python list");
            return NULL;
        }

        for ( m_index j = 0; j < num_cols; j++ ) {
            py_elem = PyFloat_FromDouble(get_matrix_elem(matrix, i, j));
            if ( !py_elem ) {
                PyErr_SetString(
                    PyExc_RuntimeError,
                    "Couldn't convert C matrix element to python float");
                return NULL;
            }

            status = PyList_SetItem(row_i, j, py_elem);
            /* IndexError should be set if above call fails */
            if ( status != 0 ) return NULL;
        }

        status = PyList_SetItem(main_pylist, i, row_i);
        /* IndexError should be set if above call fails */
        if ( status != 0 ) return NULL;
    }

    return main_pylist;
}

/**
 * @brief converts a python list of vectors (x_1, ..., x_N) to a struct matrix*
 * where each row in the matrix is a vector
 *
 * @param py_X python list of vectors
 * @return an **allocated** struct matrix* which must be free'd later on
 * success, on error sets error string and returns NULL
 */
static struct matrix *get_py_X_matrix(PyObject *py_X) {
    struct matrix *X_matrix;
    m_index        N, d;

    if ( !PyList_CheckExact(py_X) ) {
        PyErr_SetString(PyExc_TypeError, "Passed non-list type");
        return NULL;
    }

    N = PyList_Size(py_X);
    if ( N == 0 ) {
        PyErr_SetString(PyExc_ValueError, "Passed empty list");
        return NULL;
    }

    d = PyList_Size(PyList_GetItem(py_X, 0));

    X_matrix = get_new_matrix(N, d);

    if ( py_matrix_to_c(py_X, X_matrix) != 0 ) {
        free_matrix(X_matrix);
        return NULL;
    }

    return X_matrix;
}

static PyObject *symnmf(PyObject *self, PyObject *args) {
    int            status;
    PyObject      *py_init_H, *py_W, *py_optimized_H;
    double         epsilon, beta;
    size_t         iter;
    struct matrix *init_H, *W, *optimized_H;

    status = PyArg_ParseTuple(args, "OOffn", &py_init_H, &py_W, &beta, &epsilon,
                              &iter);
    HANDLE_PY_FAIL(status);

    if ( !PyList_CheckExact(py_init_H) || !PyList_CheckExact(py_W) ) {
        PyErr_SetString(
            PyExc_TypeError,
            "initial H and W matrices should be regular python lists");
        return NULL;
    }

    m_index n = PyList_Size(py_init_H);
    if ( n == 0 ) {
        PyErr_SetString(PyExc_ValueError, "initial H is empty");
        return NULL;
    }
    m_index k = PyList_Size(PyList_GetItem(py_init_H, 0));

    init_H = get_new_matrix(n, k);
    W = get_new_matrix(n, n);
    optimized_H = get_new_matrix(n, k);
#define symnmf_free                                                            \
    do {                                                                       \
        free_matrix(init_H);                                                   \
        free_matrix(W);                                                        \
        free_matrix(optimized_H);                                              \
    } while ( 0 )

    /* py_matrix_to_c sets python err string and returns NULL on failure */
    if ( py_matrix_to_c(py_init_H, init_H) != 0 ) {
        symnmf_free;
        return NULL;
    }
    if ( py_matrix_to_c(py_W, W) != 0 ) {
        symnmf_free;
        return NULL;
    }

    optimize_H(init_H, W, beta, epsilon, iter, optimized_H);
    py_optimized_H = c_matrix_to_py(optimized_H);

    symnmf_free;

    return py_optimized_H;
}

static PyObject *sym(PyObject *self, PyObject *args) {
    /* args should be list of N vectors of dimension d */
    int            status;
    PyObject      *py_X, *py_sym;
    m_index        N;
    struct matrix *matrix_X, *matrix_sym;

    status = PyArg_ParseTuple(args, "O", &py_X);
    HANDLE_PY_FAIL(status);

    /* warning: allocates new matrix */
    if ( !(matrix_X = get_py_X_matrix(py_X)) ) return NULL;

    N = matrix_X->num_rows;

    matrix_sym = get_new_matrix(N, N);
#define sym_free                                                               \
    do {                                                                       \
        free_matrix(matrix_X);                                                 \
        free_matrix(matrix_sym);                                               \
    } while ( 0 )

    if ( sym_matrix(matrix_X, matrix_sym) != 0 ) {
        PyErr_SetString(PyExc_RuntimeError,
                        "Couldn't calculate symiliarity matrix");
        sym_free;
        return NULL;
    }

    if ( !(py_sym = c_matrix_to_py(matrix_sym)) ) {
        sym_free;
        return NULL;
    }

    sym_free;

    return py_sym;
}

static PyObject *ddg(PyObject *self, PyObject *args) {
    /* args should be X matrix */
    int            status;
    PyObject      *py_X, *py_deg;
    m_index        N;
    struct matrix *matrix_X, *matrix_deg, *matrix_sym;

    status = PyArg_ParseTuple(args, "O", &py_X);
    HANDLE_PY_FAIL(status);

    /* warning: allocates new matrix */
    if ( !(matrix_X = get_py_X_matrix(py_X)) ) return NULL;

    N = matrix_X->num_rows;

    matrix_sym = get_new_matrix(N, N);
    matrix_deg = get_new_matrix(N, N);
#define ddg_free                                                               \
    do {                                                                       \
        free_matrix(matrix_X);                                                 \
        free_matrix(matrix_sym);                                               \
        free_matrix(matrix_deg);                                               \
    } while ( 0 )

    if ( sym_matrix(matrix_X, matrix_sym) != 0 ) {
        PyErr_SetString(PyExc_RuntimeError,
                        "Could not calculate similarity matrix");
        ddg_free;
        return NULL;
    }

    if ( deg_matrix(matrix_sym, matrix_deg) != 0 ) {
        PyErr_SetString(PyExc_RuntimeError, "Couldn't calculate degree matrix");
        ddg_free;
        return NULL;
    }

    if ( !(py_deg = c_matrix_to_py(matrix_deg)) ) {
        ddg_free;
        return NULL;
    }

    ddg_free;

    return py_deg;
}

static PyObject *norm(PyObject *self, PyObject *args) {
    /* args should be X matrix */
    int            status;
    PyObject      *py_X, *py_W;
    m_index        N;
    struct matrix *matrix_X, *matrix_sym, *matrix_W;

    status = PyArg_ParseTuple(args, "O", &py_X);
    HANDLE_PY_FAIL(status);

    /* warning: allocates new matrix */
    if ( !(matrix_X = get_py_X_matrix(py_X)) ) return NULL;

    N = matrix_X->num_rows;

    matrix_sym = get_new_matrix(N, N);
    matrix_W = get_new_matrix(N, N);
#define norm_free                                                              \
    do {                                                                       \
        free_matrix(matrix_X);                                                 \
        free_matrix(matrix_sym);                                               \
        free_matrix(matrix_W);                                                 \
    } while ( 0 )

    if ( sym_matrix(matrix_X, matrix_sym) != 0 ) {
        PyErr_SetString(PyExc_RuntimeError,
                        "Could not calculate similarity matrix");
        norm_free;
        return NULL;
    }

    if ( W_matrix(matrix_sym, matrix_W) != 0 ) {
        PyErr_SetString(PyExc_RuntimeError,
                        "Couldn't calculate normalized similarity matrix");
        norm_free;
        return NULL;
    }

    if ( !(py_W = c_matrix_to_py(matrix_W)) ) return NULL;

    norm_free;

    return py_W;
}

static PyMethodDef symnmfmodule_methods[] = {
    /* clang-format off */
    {
        "symnmf",
        symnmf,
        METH_VARARGS,
        SYMNMF_METHOD_DOCSTRING
    },
    {
        "sym",
        sym,
        METH_VARARGS,
        SYM_METHOD_DOCSTRING
    },
    {

        "ddg",
        ddg,
        METH_VARARGS,
        DDG_METHOD_DOCSTRING
    },
    {
        "norm",
        norm,
        METH_VARARGS,
        NORM_METHOD_DOCSTRING
    },
    {
        NULL, NULL, 0, NULL
    }
    /* clang-format on */
};

static PyModuleDef symnmfmodule = {
    /* clang-format off */
    PyModuleDef_HEAD_INIT,
    STR(MODULE_NAME),
    MODULE_DESC,
    -1,
    symnmfmodule_methods
    /* clang-format on */
};

#define INIT_FUNC_NAME CONCAT(PyInit_, MODULE_NAME)
PyMODINIT_FUNC INIT_FUNC_NAME(void) { return PyModule_Create(&symnmfmodule); }
