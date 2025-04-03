#!/bin/bash

set -euo pipefail

make analyze debug asan msan

ASAN=build/asan/oas
MSAN=build/msan/oas
DEBUG=build/debug/oas

ARGUMENTS=("tokens" "text" "ast")
while IFS= read -r INPUT_FILE; do
    for ARGS in ${ARGUMENTS[@]}; do
        $ASAN $ARGS $INPUT_FILE > /dev/null
        $MSAN $ARGS $INPUT_FILE > /dev/null
        valgrind --leak-check=full --error-exitcode=1 $DEBUG $ARGS $INPUT_FILE >/dev/null
    done
done < <(find tests/input/ -type f -name '*.asm')
