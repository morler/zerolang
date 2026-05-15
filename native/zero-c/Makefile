CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -Os
ROOT := ../..
OUT := $(ROOT)/.zero/bin/zero
SRC := $(wildcard src/*.c)

.PHONY: all clean install-local

all: $(OUT)

$(OUT): $(SRC) include/zero.h
	@mkdir -p $(dir $(OUT))
	$(CC) $(CFLAGS) -Iinclude $(SRC) -o $(OUT)

clean:
	rm -f $(OUT)

install-local: $(OUT)
	@printf 'installed %s\n' "$(OUT)"
