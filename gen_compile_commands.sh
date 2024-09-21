#!/usr/bin/env bash

# so C_FILES globbing includes files in project root
shopt -s globstar

# should output with -I for each include
PYTHON_INCLUDES="$(python3-config --includes)"
PROJ_DIR="$(pwd)"
COURSE_COMPILE_CMD="gcc -xc -ansi -Wall -Wextra -Werror -pedantic-errors"
PYTHON_COMPILE_CMD="gcc -xc -Wall -Wextra $PYTHON_INCLUDES"
C_FILES=(**/*.{c,h})
COURSE_COMPILED_FILES=()
PYTHON_COMPILED_FILES=()
COMPILE_COMMANDS_FILENAME=compile_commands.json

echo "[" >$COMPILE_COMMANDS_FILENAME

# separate project C files into module files and regular C files, filtering by 'module' in filename
for file in "${C_FILES[@]}"; do
	if [[ "$file" =~ .*module.* ]]; then
		PYTHON_COMPILED_FILES+=("$file")
	else
		COURSE_COMPILED_FILES+=("$file")
	fi
done

# populate compilation database
for file in "${PYTHON_COMPILED_FILES[@]}"; do
	cat <<EOF >>$COMPILE_COMMANDS_FILENAME
	{
		"command": "$PYTHON_COMPILE_CMD",
		"directory": "$PROJ_DIR",
		"file": "$file"
	},
EOF
done

for file in "${COURSE_COMPILED_FILES[@]}"; do
	cat <<EOF >>$COMPILE_COMMANDS_FILENAME
	{
		"command": "$COURSE_COMPILE_CMD",
		"directory": "$PROJ_DIR",
		"file": "$file"
	},
EOF
done

# remove trailing , and newline from last entry
truncate -s -2 $COMPILE_COMMANDS_FILENAME

echo -ne "\n]" >>$COMPILE_COMMANDS_FILENAME
