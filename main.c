#include "libs/matrix.h"
#include "libs/utils.h"
#include "symnmf.h"

#include <errno.h>    /* for errno */
#include <stdio.h>    /* for file reading ops */
#include <string.h>   /* for strerror() */
#include <sys/mman.h> /* for mmap() */
#include <sys/stat.h> /* for fstat() */

#define NUM_DELIM  ','
#define LINE_DELIM '\n'
#define NUL_BYTE   '\0'

#define MB (1 << 20)
const off_t MAX_FILESIZE = 1024 * MB; /* 1GB */

const char *USR_GOAL_SYM = "sym";
const char *USR_GOAL_DDG = "ddg";
const char *USR_GOAL_NORM = "norm";

enum usr_goal { SYM, DDG, NORM, UNKNOWN };

/**
 * @brief loads file into memory with read write permissions
 *
 * @param file open file to load from, opened with fopen()
 * @param filesize ptr to variable holding the size of the file
 * @return buffer holding the contents of the file, or NULL on failure. must
 * call free_filebuf() to free the buffer after use
 */
char *load_filebuf(FILE *file, size_t *filesize) {
    char       *file_contents;
    struct stat file_stat;
    int         fd = fileno(file);

    if ( fstat(fd, &file_stat) != 0 ) {
        LOG_ERR1("Couldn't get file status: %s", strerror(errno));
        return NULL;
    }

    if ( file_stat.st_size > MAX_FILESIZE ) LOG_ABORT("File too big");

    file_contents = mmap(NULL, file_stat.st_size, PROT_READ | PROT_WRITE,
                         MAP_PRIVATE, fd, 0);

    if ( file_contents == MAP_FAILED ) {
        LOG_ERR1("mmap: %s", strerror(errno));
        return NULL;
    }

    *filesize = file_stat.st_size;

    return file_contents;
}

int free_filebuf(char *buf, size_t file_size) {
    if ( munmap(buf, file_size) != 0 ) {

        LOG_ERR1("Couldn't unmap file: %s", strerror(errno));
        return 1;
    }

    return 0;
}

/**
 * @brief parses a matrix in the form of (x.xxxx,x.xxxx\nx.xxxx...) from a
 * buffer to a `struct matrix`.
 *
 * @param data data buffer containing matrix to parse
 * @param data_size length of data buffer
 * @return returns struct matrix* containing the data, or NULL if buffer can't
 * be parsed. matrix must be free'd by user.
 */
struct matrix *parse_data_to_matrix(char *data, size_t data_size) {
#define REALLOC_MUL   2
#define INIT_NUM_VECS (1 << 5) /* 32 */

    struct matrix *matrix;
    size_t         vec_index, vec_elem, vec_dim = 0, num_vecs = 0;
    /* dynamically allocated since this may be resized */
    size_t *points_dims = malloc(sizeof(size_t) * INIT_NUM_VECS),
           points_capacity = INIT_NUM_VECS;
    matrix_element *vec;
    size_t          data_offset = 0;
    char           *num, *endptr;

    if ( points_dims == NULL ) {
        free(points_dims);
        LOG_ABORT("Malloc failure");
    }

    /* initialize this value since its used later as a base for number of
     * columns in matrix */
    points_dims[0] = 0;

    /* replaces every NUM_DELIM or LINE_DELIM instances with a null byte to use
     * strtod() on each number later, and records the dimension of each vector
     * in points_dims */
    while ( data_offset < data_size ) {
        switch ( data[data_offset] ) {
            case NUM_DELIM:
                vec_dim++;
                data[data_offset] = NUL_BYTE;
                break;

            case LINE_DELIM:
                vec_dim++;
                points_dims[num_vecs++] = vec_dim;
                vec_dim = 0;
                data[data_offset] = NUL_BYTE;

                if ( num_vecs >= points_capacity ) {
                    points_dims =
                        realloc(points_dims, REALLOC_MUL * points_capacity *
                                                 sizeof(points_dims[0]));
                    points_capacity *= REALLOC_MUL;
                    if ( points_dims == NULL ) {
                        free(points_dims);
                        return NULL;
                    }
                }
                break;
        }
        data_offset++;
    }

    matrix = get_new_matrix(num_vecs, points_dims[0]);
    if ( !matrix ) return NULL;
    data_offset = 0;
    num = data;

    for ( vec_index = 0; vec_index < num_vecs; vec_index++ ) {
        /* check all vectors have matching dimensions */
        if ( points_dims[vec_index] != points_dims[0] )
            RETURN_ERR("input vectors have mismatching dimensions", NULL);

        /* becomes part of matrix, shouldn't be free'd */
        vec = malloc(sizeof(matrix_element) * points_dims[0]);
        if ( vec == NULL ) {
            free(vec);
            free(points_dims);
            return NULL;
        }

        for ( vec_elem = 0; vec_elem < points_dims[0]; vec_elem++ ) {
            vec[vec_elem] = strtod(num, &endptr);

            if ( *endptr == NUL_BYTE )
                num = endptr + 1;
            else
                /* num couldn't be parsed into double */
                return NULL;
        }

        if ( set_matrix_vec(matrix, vec_index, vec, points_dims[0]) != 0 ) {
            LOG_ERR("Couldn't set matrix vector");
            return NULL;
        }
    }

    free(points_dims);

    return matrix;
}

enum usr_goal parse_usr_goal(const char *goal_str) {
    if ( strcmp(goal_str, USR_GOAL_SYM) == 0 ) return SYM;
    if ( strcmp(goal_str, USR_GOAL_DDG) == 0 ) return DDG;
    if ( strcmp(goal_str, USR_GOAL_NORM) == 0 ) return NORM;

    return UNKNOWN;
}

void print_matrix(struct matrix *matrix) {
    m_index         i, j;
    matrix_element *vec;

    for ( i = 0; i < matrix->num_rows; i++ ) {
        vec = get_matrix_vec(matrix, i);

        for ( j = 0; j < matrix->num_cols; j++ ) {
            printf("%.4f", vec[j]);
            if ( j < matrix->num_cols - 1 ) printf("%c", NUM_DELIM);
        }

        printf("%c", LINE_DELIM);
    }
}
/**
 * @brief calculates final matrix based on @goal
 *
 * @param input_matrix ptr to input matrix of dimensions Nxd
 * @param goal goal from `enum usr_goal`
 * @param output_matrix ptr to matrix of dimensions NxN
 */
int calc_matrix(struct matrix *input_matrix, enum usr_goal goal,
                struct matrix *output_matrix) {
    struct matrix *intermediate_matrix;
    size_t         N;

    N = input_matrix->num_rows;

    intermediate_matrix = get_new_matrix(N, N);
    if ( !intermediate_matrix ) return EXIT_FAILURE;

#define calc_matrix_free                                                       \
    do {                                                                       \
        free_matrix(intermediate_matrix);                                      \
    } while ( 0 )

    switch ( goal ) {
        case SYM:
            if ( sym_matrix(input_matrix, output_matrix) != 0 ) {
                calc_matrix_free;
                RETURN_ERR("Couldn't calculate similarity matrix",
                           EXIT_FAILURE);
            }
            break;

        case DDG:
            if ( sym_matrix(input_matrix, intermediate_matrix) != 0 ) {
                calc_matrix_free;
                RETURN_ERR("Couldn't calculate similarity matrix",
                           EXIT_FAILURE);
            }
            if ( deg_matrix(intermediate_matrix, output_matrix) != 0 ) {
                calc_matrix_free;
                RETURN_ERR("Couldn't calculate diagonal degree matrix",
                           EXIT_FAILURE);
            }
            break;

        case NORM:
            if ( sym_matrix(input_matrix, intermediate_matrix) != 0 ) {
                calc_matrix_free;
                RETURN_ERR("Couldn't calculate similarity matrix",
                           EXIT_FAILURE);
            }
            if ( W_matrix(intermediate_matrix, output_matrix) != 0 ) {
                calc_matrix_free;
                RETURN_ERR("Couldn't calculate normalized similarity matrix",
                           EXIT_FAILURE);
            }
            break;

        default:
            calc_matrix_free;
            RETURN_ERR("Unknown user goal flag", EXIT_FAILURE);
    }

    calc_matrix_free;

    return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
    FILE          *usr_file;
    size_t         usr_filesize, N;
    char          *data;
    struct matrix *input_matrix, *output_matrix;

    if ( argc != 3 ) {
        LOG1("Usage: %s <[sym | ddg | norm]> <filepath>", argv[0]);
        return EXIT_FAILURE;
    }

    usr_file = fopen(argv[2], "r");
    if ( usr_file == NULL ) {
        LOG_ERR1("Couldn't open filepath: %s", strerror(errno));
        return EXIT_FAILURE;
    }

    data = load_filebuf(usr_file, &usr_filesize);
    if ( data == NULL ) return EXIT_FAILURE;

    input_matrix = parse_data_to_matrix(data, usr_filesize);

    if ( input_matrix == NULL ) LOG_ABORT("parse matrix error");

    N = input_matrix->num_rows;

    free_filebuf(data, usr_filesize);
    fclose(usr_file);

    /* all goals require an output matrix of size NxN so allocate once here and
     * pass to parse_usr_goal() */
    output_matrix = get_empty_matrix(N, N);
    if ( !output_matrix ) LOG_ABORT("malloc failure");

    if ( calc_matrix(input_matrix, parse_usr_goal(argv[1]), output_matrix) !=
         EXIT_SUCCESS ) {
        goto free;
        LOG_ABORT("malloc failure");
    };

    print_matrix(output_matrix);

free:
    free_matrix(input_matrix);
    free_matrix(output_matrix);

    return EXIT_SUCCESS;
}
