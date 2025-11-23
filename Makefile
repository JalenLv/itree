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

.PHONY: build clean distclean run install

build: itree

clean:
	rm -f *.o itree

distclean: clean
	rm -f config.mk

run: build
	./itree

itree: $(SRC)
	gcc $(CFLAGS) $(SRC) -o itree $(LDFLAGS)

install: build
	$(INSTALL) -d $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 itree $(DESTDIR)$(BINDIR)/itree
