SRC_DIR=src
INCLUDE_DIR=include
EXTERN_DIR=extern
CFLAGS=-Wall -Wextra -g

.PHONY: build clean run

build: itree

clean:
	rm -f *.o itree

run: build
	tree -J | ./itree

main.o: $(SRC_DIR)/main.c
	gcc $(CFLAGS) -c $(SRC_DIR)/main.c -I$(INCLUDE_DIR) -I$(EXTERN_DIR)/cJSON/include -o main.o

cJSON.o: $(EXTERN_DIR)/cJSON/src/cJSON.c
	gcc $(CFLAGS) -c $(EXTERN_DIR)/cJSON/src/cJSON.c -I$(EXTERN_DIR)/cJSON/include -o cJSON.o

argparse.o: $(SRC_DIR)/argparse.c
	gcc $(CFLAGS) -c $(SRC_DIR)/argparse.c -I$(INCLUDE_DIR) -o argparse.o

file_tree.o: $(SRC_DIR)/file_tree.c
	gcc $(CFLAGS) -c $(SRC_DIR)/file_tree.c -I$(INCLUDE_DIR) -I$(EXTERN_DIR)/cJSON/include -o file_tree.o

helpers.o: $(SRC_DIR)/helpers.c
	gcc $(CFLAGS) -c $(SRC_DIR)/helpers.c -I$(INCLUDE_DIR) -o helpers.o

draw_tree.o: $(SRC_DIR)/draw_tree.c
	gcc $(CFLAGS) -c $(SRC_DIR)/draw_tree.c -I$(INCLUDE_DIR) -I$(EXTERN_DIR)/cJSON/include -o draw_tree.o

tui.o: $(SRC_DIR)/tui.c
	gcc $(CFLAGS) -c $(SRC_DIR)/tui.c -I$(INCLUDE_DIR) -I$(EXTERN_DIR)/cJSON/include -o tui.o -lncurses

itree: main.o cJSON.o argparse.o file_tree.o helpers.o draw_tree.o tui.o
	gcc main.o cJSON.o argparse.o file_tree.o helpers.o draw_tree.o tui.o -o itree -lncurses
