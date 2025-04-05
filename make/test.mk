.PHONY: test

CFLAGS?=-Wall -Wextra -Wpedantic -O0 -g3 -std=c23 -fno-omit-frame-pointer -fno-optimize-sibling-calls -D_POSIX_C_SOURCE=200809L -fprofile-instr-generate -fcoverage-mapping
LDFLAGS?=-fprofile-instr-generate
BUILD_DIR=build/test/
TARGET=oas-tests
SOURCES = $(filter-out src/main.c, $(shell find src/ tests/ -type f -name '*.c'))
-include make/base.mk

test: $(BUILD_DIR)$(TARGET)
	mkdir -p reports/coverage
	LLVM_PROFILE_FILE="reports/coverage/tests.profraw" $(BUILD_DIR)$(TARGET)
	llvm-profdata merge -sparse reports/coverage/tests.profraw -o reports/coverage/tests.profdata
	llvm-cov show $(BUILD_DIR)$(TARGET) -instr-profile=reports/coverage/tests.profdata -format=html -output-dir=reports/coverage/html -ignore-filename-regex="tests/.*"
	@echo "--"
	@echo "Test coverage:"
	@echo "file://$$(realpath reports/coverage/html/index.html)"
	@echo "--"

clean:
	rm -rf reports/coverage
