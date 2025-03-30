#!/bin/bash

set -euo pipefail

# Start with static analysis
scan-build -o reports/ -plist-html --status-bugs make clean all

# Run the sanitizer builds and valgrind
make clean sanitize all

ARGUMENTS=("-tokens" "-text")
while IFS= read -r INPUT_FILE; do
    for ARGS in ${ARGUMENTS[@]}; do
        ./oas-asan $ARGS $INPUT_FILE > /dev/null
        ./oas-msan $ARGS $INPUT_FILE > /dev/null
        valgrind --leak-check=full --error-exitcode=1 ./oas $ARGS $INPUT_FILE >/dev/null
    done
done < <(find tests/input/ -type f -name '*.asm')
