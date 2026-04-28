-include config.mk

PREFIX ?= /usr/local
BINDIR := $(PREFIX)/bin

INSTALL ?= install

SRC=$(wildcard src/*.c)
INCLUDE_DIR=include

CFLAGS := -Wall -Wextra -O3 \
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
	gcc $(CFLAGS) $(SRC) -o itree $(LDFLAGS)

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
TEST_CFLAGS := $(CFLAGS) -Itests -Itests/unit
TEST_LDFLAGS := $(LDFLAGS)

tests/unit/runner: $(TEST_SRC) tests/greatest.h
	gcc $(TEST_CFLAGS) $(TEST_SRC) -o $@ $(TEST_LDFLAGS)

test-unit: tests/unit/runner
	./tests/unit/runner

test-integration: itree
	./tests/integration/run.sh

test-dumps: itree
	python3 tests/dumps/test_rep.py
	./tests/dumps/runner.py

test: test-unit test-integration test-dumps
