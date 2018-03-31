PROJECT :=lsp
VERSION :=0.0.1

PREFIX ?=/usr/local
CC ?=cc

MAKEFLAGS += --no-builtin-rules


## Compiler options.
CFLAGS += -std=c11
CFLAGS += -Iinclude
CFLAGS += -fPIC
CFLAGS += -Wall -Wextra -pedantic

# Debug specific flags.
CFLAGS_DBG := $(CFLAGS)
CFLAGS_DBG += -O0
CFLAGS_DBG += -g

# Production specific flags.
CFLAGS += -O3

ifdef DEBUG
CFLAGS := $(CFLAGS_DBG)
endif


## Filenames.
SOURCES := $(wildcard src/**/*.c src/*.c)
OBJECTS := $(patsubst src/%.c,build/%.o,$(SOURCES))
DEPENDENCIES := $(patsubst src/%.c,build/%.d,$(SOURCES))
SRC_DIRS := $(shell find src -type d)
BUILD_DIRS := $(patsubst src%,build%,$(SRC_DIRS))
EXECUTABLE := $(PROJECT)


## Build rules.
all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) main.c
	$(CC) $(CFLAGS) $(LD_FLAGS) $(OBJECTS) $(LIBS) -o $@ main.c

$(OBJECTS) : build/%.o : src/%.c
	@mkdir -p $(BUILD_DIRS)
	$(CC) $(CFLAGS) -c $< -o $@ -MMD

-include ${DEPENDENCIES}


## Distribution
clean:
	rm -f $(EXECUTABLE)
	rm -Rf build/
	rm -Rf $(PROJECT)-$(VERSION)

dist: clean
	mkdir -p $(PROJECT)-$(VERSION)
	cp -r LICENSE Makefile README.mkd src/ include/ $(PROJECT)-$(VERSION)/
	tar -czf $(PROJECT)-$(VERSION).tar.gz $(PROJECT)-$(VERSION)

.PHONY: clean dist all

