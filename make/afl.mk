.PHONY: fuzz distclean

CC=afl-clang-fast
LD=afl-clang-fast
BUILD_DIR=build/afl/

-include make/base.mk

fuzz: $(BUILD_DIR)$(TARGET)
	mkdir -p reports/afl
	afl-fuzz -i tests/input -o reports/afl -m none -- ./$< -tokens @@

distclean: clean
	rm -rf reports/afl
