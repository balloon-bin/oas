.PHONY: all clean clean-objects run sanitize validate

CC=clang
LD=clang
CFLAGS=-Wall -Wextra -Wpedantic -O0 -g3 -std=c23 -fno-omit-frame-pointer -fno-optimize-sibling-calls
LDFLAGS?=

SOURCES = $(shell find src/ -type f -name '*.c')
OBJECTS = $(SOURCES:.c=.o)
DEPENDENCIES = $(SOURCES:.c=.d)
TARGET?=oas
OUTPUTS=oas oas-asan oas-msan
RUNARGUMENTS=-tokens test.asm

all: $(TARGET)
	

run: $(TARGET)
	./$(TARGET) $(RUNARGUMENTS)

sanitize:
	make CFLAGS="$(CFLAGS) -fsanitize=address,undefined" LDFLAGS="-fsanitize=address,undefined" TARGET="oas-asan" clean-objects all
	make CFLAGS="$(CFLAGS) -fsanitize=memory -fsanitize-memory-track-origins=2" LDFLAGS="-fsanitize=memory -fsanitize-memory-track-origins=2" TARGET="oas-msan" clean-objects all 
	make clean-objects

validate:
	./validate.sh

$(TARGET): $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

-include $(DEPENDENCIES)

clean-objects:
	rm -f $(OBJECTS) $(DEPENDENCIES)

clean: clean-objects
	rm -f $(TARGET) $(OUTPUTS)
	rm -rf reports/
