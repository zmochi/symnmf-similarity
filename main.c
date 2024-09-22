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
    const char *file_contents;
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

    return 0;
}

int free_filebuf(char *buf, size_t file_size) {
    if ( munmap(buf, file_size) != 0 ) {

        LOG_ERR1("Couldn't unmap file: %s", strerror(errno));
        return 1;
    }

    return 0;
}

struct matrix *parse_data_to_matrix(char *data, size_t data_size) {
#define REALLOC_MUL   2
#define INIT_NUM_VECS (1 << 5) /* 32 */

    struct matrix  *matrix;
    size_t          vec_index, vec_elem, vec_dim = 0, num_vecs = 0;
    size_t         *points_dims = malloc(sizeof(size_t) * INIT_NUM_VECS);
    matrix_element *vec;
    size_t          data_offset = 0, points_capacity = INIT_NUM_VECS;
    char           *num, *endptr;

    /* replaces every NUM_DELIM or LINE_DELIM instances in a null byte to use
     * strtod() on each number later, and records the dimension of each vector
     * in points_dims */
    while ( data_offset < data_size ) {
        switch ( data[data_offset] ) {
            case NUM_DELIM:
                vec_dim++;
                data[data_offset] = NUL_BYTE;
                break;

            case LINE_DELIM:
                points_dims[num_vecs] = vec_dim;
                vec_dim = 0;
                num_vecs++;
                data[data_offset] = NUL_BYTE;
                if ( num_vecs > points_capacity ) {
                    vec = realloc(points_dims, REALLOC_MUL * points_capacity);
                    points_capacity *= REALLOC_MUL;
                }
                break;
        }
        data_offset++;
    }

    matrix = get_new_matrix(num_vecs, points_dims[0]);
    vec = malloc(sizeof(matrix_element *) * num_vecs);
    data_offset = 0;
    num = data;

    for ( vec_index = 0; vec_index < num_vecs; vec_index++ ) {
        vec = malloc(sizeof(matrix_element) * points_dims[vec_index]);
        /* check all vectors have matching dimensions */
        if ( points_dims[vec_index] != matrix->num_cols )
            RETURN_ERR("input vectors have mismatching dimensions", NULL);

        for ( vec_elem = 0; vec_elem < points_dims[vec_index]; vec_elem++ ) {
            vec[vec_elem] = strtod(num, &endptr);

            if ( *endptr == NUL_BYTE )
                num = endptr + 1;
            else
                /* num couldn't be parsed into double */
                return NULL;
        }

        if ( set_matrix_vec(matrix, vec_index, vec) != 0 ) {
            LOG_ERR("Couldn't set matrix vector");
            return NULL;
        }
    }

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

int main(int argc, char **argv) {
    FILE          *usr_file;
    size_t         usr_filesize;
    char          *data;
    struct matrix *input_matrix, *output_matrix;
#define main_free_matrices                                                     \
    do {                                                                       \
        free_matrix(input_matrix);                                             \
        free_matrix(output_matrix);                                            \
    } while ( 0 )

    if ( argc != 2 ) {
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

    free_filebuf(data, usr_filesize);
    fclose(usr_file);

    output_matrix =
        get_new_matrix(input_matrix->num_rows, input_matrix->num_cols);

    switch ( parse_usr_goal(argv[1]) ) {
        case SYM:
            if ( sym_matrix(input_matrix, output_matrix) != 0 ) {
                main_free_matrices;
                RETURN_ERR("Couldn't calculate similarity matrix",
                           EXIT_FAILURE);
            }
            break;

        case DDG:
            if ( sym_matrix(input_matrix, input_matrix) != 0 ) {
                main_free_matrices;
                RETURN_ERR("Couldn't calculate similarity matrix",
                           EXIT_FAILURE);
            }
            if ( deg_matrix(input_matrix, output_matrix) != 0 ) {
                main_free_matrices;
                RETURN_ERR("Couldn't calculate diagonal degree matrix",
                           EXIT_FAILURE);
            }
            break;

        case NORM:
            if ( W_matrix(input_matrix, output_matrix) != 0 ) {
                main_free_matrices;
                RETURN_ERR("Couldn't calculate normalized similarity matrix",
                           EXIT_FAILURE);
            }
            break;

        default:
            main_free_matrices;
            RETURN_ERR("Unknown user goal flag", EXIT_FAILURE);
    }

    print_matrix(output_matrix);

    main_free_matrices;

    return EXIT_SUCCESS;
}
