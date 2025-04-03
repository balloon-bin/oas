CC?=clang
LD?=clang
CFLAGS?=-Wall -Wextra -Wpedantic -O2 -std=c23 -flto -fomit-frame-pointer -DNDEBUG -D_POSIX_C_SOURCE=200809L
LDFLAGS?=-flto -s -Wl,--gc-sections
BUILD_DIR?=build/release/

-include make/base.mk
