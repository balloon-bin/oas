.PHONY: all clean

CC?=clang
LD?=clang
CFLAGS?=-Wall -Wextra -Wpedantic -O0 -g3 -std=c23 -fno-omit-frame-pointer -fno-optimize-sibling-calls -D_POSIX_C_SOURCE=200809L
LDFLAGS?=
BUILD_DIR?=build/debug/

SOURCES?=$(shell find src/ -type f -name '*.c')
OBJECTS=$(patsubst %.c,$(BUILD_DIR)%.o,$(SOURCES))
DEPENDENCIES=$(OBJECTS:.o=.d)
TARGET?=oas

all: $(BUILD_DIR)$(TARGET)
	

$(BUILD_DIR)$(TARGET): $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^

$(BUILD_DIR)%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

-include $(DEPENDENCIES)

clean:
	rm -rf $(BUILD_DIR)
