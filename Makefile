PROJECT :=lsp
VERSION :=0.0.1

PREFIX ?=/usr/local
CC ?=cc

MAKEFLAGS += --no-builtin-rules


## Compiler options.
CFLAGS += -std=c11
CFLAGS += -g
CFLAGS += -Iinclude
CFLAGS += -fPIC
CFLAGS += -Wall -Wextra -pedantic

# Debug specific flags.
CFLAGS_DBG := $(CFLAGS)
CFLAGS_DBG += -O0

# Production specific flags.
CFLAGS += -O3

ifdef DEBUG
CFLAGS := $(CFLAGS_DBG)
endif


## Linker options.
LDFLAGS += -Wl,--as-needed
LDFLAGS += -Wl,--no-undefined


## Filenames.
LIB_SRCS := $(wildcard src/**/*.c src/*.c)
LIB_DIRS := $(patsubst src%,build/lib%,$(shell find src -type d))
LIB_OBJS := $(patsubst src/%.c,build/lib/%.o,$(LIB_SRCS))
LIB_DEPS := $(patsubst src/%.c,build/lib/%.d,$(LIB_SRCS))
LIB_OUT := build/lib$(PROJECT).so

BIN_SRCS := main.c
BIN_DIRS := build/bin
BIN_OBJS := build/bin/main.o
BIN_DEPS := build/bin/main.d
BIN_OUT := build/$(PROJECT)


## Build rules.
all: $(BIN_OUT) $(LIB_OUT)

$(LIB_OBJS) : build/lib/%.o : src/%.c
	@mkdir -p $(LIB_DIRS)
	$(CC) $(CFLAGS) -c $< -o $@ -MMD
-include ${LIB_DEPS}

$(LIB_OUT): $(LIB_OBJS)
	$(CC) -shared $(CFLAGS) $(LDFLAGS) $(LIB_OBJS) -o $@


$(BIN_OBJS) : build/bin/%.o : %.c
	@mkdir -p $(BIN_DIRS)
	$(CC) $(CFLAGS) -c $< -o $@ -MMD
-include ${BIN_DEPS}

$(BIN_OUT): $(LIB_OUT) $(BIN_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIBS) $(BIN_OBJS) -o $@ -Lbuild -llsp


## Distribution
clean:
	rm -f $(BIN_OUT)
	rm -Rf build/
	rm -Rf $(PROJECT)-$(VERSION)

dist: clean
	mkdir -p $(PROJECT)-$(VERSION)
	cp -r LICENSE Makefile README.mkd src/ include/ $(PROJECT)-$(VERSION)/
	tar -czf $(PROJECT)-$(VERSION).tar.gz $(PROJECT)-$(VERSION)

.PHONY: clean dist all

