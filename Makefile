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

# itree: $(SRC)
# 	gcc $(CFLAGS) $(SRC) -o itree -lncurses

main.o: $(SRC_DIR)/main.c
	gcc $(CFLAGS) -c $(SRC_DIR)/main.c -o main.o

argparse.o: $(SRC_DIR)/argparse.c
	gcc $(CFLAGS) -c $(SRC_DIR)/argparse.c -o argparse.o

file_tree.o: $(SRC_DIR)/file_tree.c
	gcc $(CFLAGS) -c $(SRC_DIR)/file_tree.c -o file_tree.o

helpers.o: $(SRC_DIR)/helpers.c
	gcc $(CFLAGS) -c $(SRC_DIR)/helpers.c -o helpers.o

draw_tree.o: $(SRC_DIR)/draw_tree.c
	gcc $(CFLAGS) -c $(SRC_DIR)/draw_tree.c -o draw_tree.o

# tui.o: $(SRC_DIR)/tui.c
# 	gcc $(CFLAGS) -c $(SRC_DIR)/tui.c -o tui.o

# itree: main.o argparse.o file_tree.o helpers.o draw_tree.o tui.o
# 	gcc main.o argparse.o file_tree.o helpers.o draw_tree.o tui.o -o itree -lncurses

itree: main.o argparse.o file_tree.o helpers.o draw_tree.o
	gcc main.o argparse.o file_tree.o helpers.o draw_tree.o -o itree