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


# Default target
default: all


## Library rules.
LIB_SRCS := $(wildcard src/**/*.c src/*.c)
LIB_DIRS := $(patsubst src%,build/lib%,$(shell find src -type d))
LIB_OBJS := $(patsubst src/%.c,build/lib/%.o,$(LIB_SRCS))
LIB_DEPS := $(patsubst src/%.c,build/lib/%.d,$(LIB_SRCS))
LIB_OUT := build/lib$(PROJECT).so

$(LIB_OBJS) : build/lib/%.o : src/%.c
	@mkdir -p $(LIB_DIRS)
	$(CC) $(CFLAGS) -c $< -o $@ -MMD
-include ${LIB_DEPS}

$(LIB_OUT): $(LIB_OBJS)
	$(CC) -shared $(CFLAGS) $(LDFLAGS) $(LIB_OBJS) -o $@


## Test rules.
TEST_SRCS := $(wildcard test/**/*.c test/*.c)
TEST_DIRS := $(patsubst test%,build/test%,$(shell find test -type d))
TEST_OBJS := $(patsubst test/%.c,build/test/%.o,$(TEST_SRCS))
TEST_DEPS := $(patsubst test/%.c,build/test/%.d,$(TEST_SRCS))
TEST_OUT := build/run-lib$(PROJECT)-tests

$(TEST_OBJS) : build/test/%.o : test/%.c
	@mkdir -p $(TEST_DIRS)
	$(CC) $(CFLAGS) -c $< -o $@ -MMD
-include ${TEST_DEPS}

$(TEST_OUT): $(LIB_OUT) $(TEST_OBJS)
	$(CC) $(CFLAGS) \
		$(LDFLAGS) $(LIBS) \
		$(TEST_OBJS) \
		-Lbuild -llsp -Wl,-rpath=./build -lcriterion \
		-o $@


## Executable rules.
BIN_SRCS := main.c
BIN_DIRS := build/bin
BIN_OBJS := build/bin/main.o
BIN_DEPS := build/bin/main.d
BIN_OUT := build/$(PROJECT)

$(BIN_OBJS) : build/bin/%.o : %.c
	@mkdir -p $(BIN_DIRS)
	$(CC) $(CFLAGS) -c $< -o $@ -MMD
-include ${BIN_DEPS}

$(BIN_OUT): $(LIB_OUT) $(BIN_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIBS) $(BIN_OBJS) -o $@ -Lbuild -llsp


all: $(BIN_OUT) $(LIB_OUT) $(TEST_OUT)


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

