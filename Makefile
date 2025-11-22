-include config.mk

PREFIX ?= /usr/local
BINDIR := $(PREFIX)/bin

SRC=$(wildcard src/*.c)
INCLUDE_DIR=include

INSTALL ?= install

CFLAGS=-Wall -Wextra -O3 \
	-I$(INCLUDE_DIR)

.PHONY: build clean distclean run install

build: itree

clean:
	rm -f *.o itree

distclean: clean
	rm -f config.mk

run: build
	./itree

itree: $(SRC)
	gcc $(CFLAGS) $(SRC) -o itree -lncurses

install: build
	$(INSTALL) -d $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 itree $(DESTDIR)$(BINDIR)/itree
