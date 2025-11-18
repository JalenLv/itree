SRC_DIR=src
SRC=$(wildcard $(SRC_DIR)/*.c)
INCLUDE_DIR=include

CFLAGS=-Wall -Wextra -O3 \
	-I$(INCLUDE_DIR)

.PHONY: build clean run

build: itree

clean:
	rm -f *.o itree

run: build
	./itree

itree: $(SRC)
	gcc $(CFLAGS) $(SRC) -o itree -lncurses