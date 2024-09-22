CC = gcc
CFLAGS = -ansi -Wall -Wextra -Werror -pedantic-errors

EXECUTABLE_NAME = symnmf
PROJ_ROOT = .

# directory paths starting at PROJ_ROOT
SRC_DIR = .
H_DIR = .
OBJ_DIR = obj
PY_SRC_DIR = .
PY_H_DIR = .
LIB_DIR = libs# obj directory structure must must SRC_DIR structure
OBJ_LIB_DIR = ${OBJ_DIR}/libs

_H_DEPS = symnmf.h ${LIB_DIR}/matrix.h ${LIB_DIR}/utils.h
_C_DEPS = symnmf.c main.c ${LIB_DIR}/matrix.c 

C_DEPS = $(call add_path_prefix, ${_C_DEPS}, ${SRC_DIR})
H_DEPS = $(call add_path_prefix, ${_H_DEPS}, ${H_DIR})

OBJS = $(patsubst %.c,${OBJ_DIR}/%.o,${_C_DEPS})

define add_path_prefix
$(patsubst %,$(2)/%,$(1))
endef

all: directories ${OBJS}
	$(CC) $(CFLAGS) -o $(EXECUTABLE_NAME) $(OBJS)

${OBJ_DIR}/%.o: ${SRC_DIR}/%.c ${H_DEPS} | directories
	$(CC) $(CFLAGS) -c $< -o $@

directories:
	mkdir -p ${OBJ_DIR}

# flags for compile_commands.json
PY_CFLAGS = -Wall -Wextra ${PY_C_INCLUDES}
PY_C_INCLUDES = $(shell python3-config --includes)

clangd:
	./gen_compile_commands.sh "${CC}" "-xc ${CFLAGS}" "-xc ${PY_CFLAGS}"

clean:
	rm -rf ${OBJ_DIR}

.PHONY: clangd clean
