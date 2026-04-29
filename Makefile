-include config.mk

PREFIX ?= /usr/local
BINDIR := $(PREFIX)/bin

INSTALL ?= install

CC ?= gcc

SRC=$(wildcard src/*.c)
INCLUDE_DIR=include

# Base warning flags shared by both debug and release
WARN_FLAGS := -Wall -Wextra

# Debug flags
ifeq ($(DEBUG),yes)
OPT_FLAGS := -Og -g3
WARN_FLAGS += \
	-Wshadow -Wformat=2 -Wnull-dereference -Wcast-align -Wstrict-prototypes -Wmissing-prototypes \
	-Wpedantic -Wconversion -Wsign-conversion -Wdouble-promotion -Wundef -Wunused \
	-Wfloat-equal -Waggregate-return -Wswitch-default -Winline
else
OPT_FLAGS := -O3 -DNDEBUG -flto=auto
WARN_FLAGS += -Werror
endif

ifeq ($(SANITIZE),yes)
OPT_FLAGS += -fsanitize=address,undefined -fno-omit-frame-pointer
endif

ifeq ($(NATIVE),yes)
OPT_FLAGS += -march=native -mtune=native
endif

CFLAGS := $(OPT_FLAGS) $(WARN_FLAGS) \
	-I$(INCLUDE_DIR) \
	$(NCURSES_CFLAGS)
ifeq ($(WIDE_NCURSES),yes)
CFLAGS += -DWIDE_NCURSES
endif
LDFLAGS := $(NCURSES_LIBS)

.PHONY: build clean distclean run install \
        test test-unit test-integration test-dumps

build: itree

clean:
	rm -f *.o itree tests/unit/runner

distclean: clean
	rm -f config.mk

run: build
	./itree

itree: $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o itree $(LDFLAGS)

install: build
	$(INSTALL) -d $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 itree $(DESTDIR)$(BINDIR)/itree

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------

# Sources for the unit-test binary: all of src/ except main.c, plus all
# tests/unit/*.c. The runner links ncurses (for KEY_* constants) but never
# calls initscr(), so it is safe to run in any environment.
TEST_SRC := $(filter-out src/main.c,$(SRC)) $(wildcard tests/unit/*.c)
TEST_CFLAGS := $(filter-out -Werror,$(CFLAGS)) -Itests -Itests/unit
TEST_LDFLAGS := $(LDFLAGS)

tests/unit/runner: $(TEST_SRC) tests/greatest.h
	$(CC) $(TEST_CFLAGS) $(TEST_SRC) -o $@ $(TEST_LDFLAGS)

test-unit: tests/unit/runner
	./tests/unit/runner

test-integration: itree
	./tests/integration/run.sh

test-dumps: itree
	python3 tests/dumps/test_rep.py
	./tests/dumps/runner.py

test: test-unit test-integration test-dumps
