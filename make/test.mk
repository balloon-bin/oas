.PHONY: test

BUILD_DIR=build/test/
TARGET=oas-tests
SOURCES = $(filter-out src/main.c, $(shell find src/ tests/ -type f -name '*.c'))
-include make/base.mk

test: $(BUILD_DIR)$(TARGET)
	$(BUILD_DIR)$(TARGET)
