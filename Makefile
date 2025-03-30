.PHONY: all clean clean-objects clean-reports run sanitize validate fuzz

CC=clang
LD=clang
CFLAGS=-Wall -Wextra -Wpedantic -O0 -g3 -std=c23 -fno-omit-frame-pointer -fno-optimize-sibling-calls -D_POSIX_C_SOURCE=200809L
LDFLAGS?=

SOURCES = $(shell find src/ -type f -name '*.c')
OBJECTS = $(SOURCES:.c=.o)
DEPENDENCIES = $(SOURCES:.c=.d)
TARGET?=oas
OUTPUTS=oas oas-asan oas-msan oas-afl
RUNARGUMENTS?=-tokens tests/input/valid.asm

all: $(TARGET)
	

run: $(TARGET)
	./$(TARGET) $(RUNARGUMENTS)

fuzz:
	make CC="afl-clang-fast" LD="afl-clang-fast" TARGET="oas-afl" clean-objects all
	make clean-objects
	mkdir -p reports/afl
	afl-fuzz -i tests/input -o reports/afl -m none -- ./oas-afl -tokens @@

sanitize:
	make CFLAGS="$(CFLAGS) -fsanitize=address,undefined" \
		LDFLAGS="-fsanitize=address,undefined" \
		TARGET="oas-asan" clean-objects all
	make CFLAGS="$(CFLAGS) -fsanitize=memory -fsanitize-memory-track-origins=2" \
		LDFLAGS="-fsanitize=memory -fsanitize-memory-track-origins=2" \
		TARGET="oas-msan" clean-objects all 
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

clean-reports:
	rm -rf reports/

clean: clean-objects
	rm -f $(TARGET) $(OUTPUTS)
