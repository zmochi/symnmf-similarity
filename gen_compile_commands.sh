#!/usr/bin/env bash

PYTHON_INCLUDES="$(python3-config --includes)"
PROJ_DIR="$(pwd)"

cat <<EOF >compile_commands.json
[
	{
		"command": "gcc -xc -ansi -Wall -Wextra -Werror -pedantic-errors",
		"directory": "$PROJ_DIR",
		"file": "symnmf.c"
	},
	{
		"command": "gcc -xc -ansi -Wall -Wextra -Werror -pedantic-errors",
		"directory": "$PROJ_DIR",
		"file": "symnmf.h"
	},
	{
		"command": "gcc -xc -Wall -Wextra $PYTHON_INCLUDES",
		"directory": "$PROJ_DIR",
		"file": "symnmfmodule.c"
	},
	{
		"command": "gcc -xc -Wall -Wextra $PYTHON_INCLUDES",
		"directory": "$PROJ_DIR",
		"file": "symnmfmodule.h"
	}
]
EOF
