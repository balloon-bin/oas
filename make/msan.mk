CFLAGS=-Wall -Wextra -Wpedantic -O0 -g3 -std=c23 -fno-omit-frame-pointer -fno-optimize-sibling-calls -D_POSIX_C_SOURCE=200809L -fsanitize=memory
LDFLAGS=-fsanitize=memory
BUILD_DIR=build/msan/

-include make/base.mk
