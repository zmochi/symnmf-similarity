#include "libs/matrix.h"
#include "libs/utils.h"
#include "symnmf.h"

#include <errno.h>
#include <stdio.h>    /* for file reading ops */
#include <string.h>   /* for strerror() */
#include <sys/mman.h> /* for mmap() */
#include <sys/stat.h>

#define MB (1 << 20)
const off_t MAX_FILESIZE = 1024 * MB; /* 1GB */

const char *USR_GOAL_SYM = "sym";
const char *USR_GOAL_DDG = "ddg";
const char *USR_GOAL_NORM = "norm";

enum usr_goals { SYM, DDG, NORM };

/**
 * @brief loads file into memory, read-only
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

#ifdef MAP_WITH_WRITE
#define PROT_PERMS (PROT_READ | PROT_WRITE)
#define MAP_TYPE   MAP_PRIVATE
#else
#define PROT_PERMS (PROT_READ)
#define MAP_TYPE   MAP_SHARED
#endif
    file_contents = mmap(NULL, file_stat.st_size, PROT_PERMS, MAP_TYPE, fd, 0);

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
#define INIT_VECSIZE 100
#define NUM_DELIM    ','
#define LINE_DELIM   '\n'
#define NUL_BYTE     '\0'

    size_t          i = 0, vec_num_elements = 0;
    matrix_element *vec = malloc(sizeof(matrix_element) * INIT_VECSIZE);
    size_t          data_offset = 0;

#ifdef MAP_WITH_WRITE

    while ( data_offset < data_size ) {
        for ( i = 0; i < ARR_SIZE(num); i++ ) {
            num[i] = data[data_offset];
            if ( data[data_offset] == NUM_DELIM ) {
                vec_num_elements++;
                data[data_offset] = NUL_BYTE;
            }
        }
    }
#else  /* buf is readonly */
#define INIT_NUMBUF_SIZE 512
    char nums_line[INIT_NUMBUF_SIZE];

    while ( data_offset < data_size ) {
        switch ( data[data_offset] ) {
            case NUM_DELIM:
                vec_num_elements++;
                nums_line[i] = NUL_BYTE;
                break;
            case LINE_DELIM:
                vec_num_elements++;
                nums_line[i] = NUL_BYTE;
                populate_vec(nums_line, vec, &vec_num_elements);
                break;
            default:
                nums_line[i] = data[data_offset];
        }

        i++;
        data_offset++;
    }
#endif /* ifdef MAP_WITH_WRITE */
}

int main(int argc, char **argv) {
    FILE          *usr_file;
    size_t         usr_filesize;
    char          *data;
    struct matrix *output_matrix;

    usr_file = fopen(argv[2], "r");
    data = load_filebuf(usr_file, &usr_filesize);
    output_matrix = parse_data_to_matrix(data);

    // parse goal and filename
    switch ( parse_usr_goal(argv[1]) ) {
        case SYM:
        case DDG:
        case NORM:
        default:
            LOG_ABORT("Unknown user goal flag");
    }

    print_matrix(output_matrix);
}
