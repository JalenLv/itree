#ifndef HELPERS_H
#define HELPERS_H

/**
 * concatenates multiple c strings into a newly allocated string.
 * The caller is responsible for freeing the returned string.
 */
char *concat_NULL(const char *str1, ...);
#define concat(str1, ...) concat_NULL(str1, __VA_ARGS__, NULL)

/**
 * Dynamic array macros.
 */
#include <stdlib.h>
#include <string.h>
// Define the fields of a dynamic array for a given type
// Usage: typedef struct { DA_FIELDS(int); } IntArray;
#define DA_FIELDS(type) \
	int count; \
	int capacity; \
	type *items
// Free the dynamic array
// Use this only when the elements do not need to be freed individually
#define DA_FREE(type, arr) do { \
	if ((arr)->items != NULL) { \
		free((arr)->items); \
		(arr)->items = NULL; \
		(arr)->count = 0; \
		(arr)->capacity = 0; \
	} \
} while(0)
// Push an item to the dynamic array
#define DA_PUSH(type, arr, item) do { \
	if ((arr)->count == (arr)->capacity) { \
		(arr)->capacity = (arr)->capacity ? (arr)->capacity * 2 : 1; \
		(arr)->items = (type *)realloc((arr)->items, (arr)->capacity * sizeof(type)); \
	} \
	(arr)->items[(arr)->count++] = item; \
} while(0)
// Pop an item from the dynamic array
#define DA_POP(type, arr) ((arr)->count > 0 ? (type)(arr)->items[--(arr)->count] : (type){0})
// Get an item from the dynamic array by index
#define DA_GET(type, arr, index) (((index) < (arr)->count && (index) >= 0) ? (type)(arr)->items[index] : (type){0})
// Get a pointer to an item from the dynamic array by index
#define DA_GET_PTR(type, arr, index) (((index) < (arr)->count && (index) >= 0) ? (type)&((arr)->items[index]) : NULL)
// Remove an item from the dynamic array by index
#define DA_REMOVE(type, arr, index) do { \
	if ((index) < (arr)->count && (index) >= 0) { \
		memmove(&(arr)->items[index], &(arr)->items[index + 1], ((arr)->count - (index) - 1) * sizeof(type)); \
		(arr)->count--; \
	} \
} while(0)
// Remove a range of items from the dynamic array [start, end)
#define DA_REMOVE_RANGE(type, arr, start, end) do { \
	if ((start) >= 0 && (end) <= (arr)->count && (start) < (end)) { \
		memmove(&(arr)->items[start], &(arr)->items[end], ((arr)->count - (end)) * sizeof(type)); \
		(arr)->count -= ((end) - (start)); \
	} \
} while(0)
// Insert a DA `arr2insert` into another DA `arr` at a given index
#define DA_INSERT(type, arr, index, arr2insert) do { \
	if ((arr2insert)->count > 0 && (index) >= 0 && (index) <= (arr)->count) { \
		while ((arr)->count + (arr2insert)->count > (arr)->capacity) { \
			(arr)->capacity = (arr)->capacity ? (arr)->capacity * 2 : 1; \
			(arr)->items = (type *)realloc((arr)->items, (arr)->capacity * sizeof(type)); \
		} \
		memmove(&(arr)->items[(index) + (arr2insert)->count], &(arr)->items[index], ((arr)->count - (index)) * sizeof(type)); \
		memcpy(&(arr)->items[index], (arr2insert)->items, (arr2insert)->count * sizeof(type)); \
		(arr)->count += (arr2insert)->count; \
	} \
} while(0)

/**
 * Dynamic deque macros.
 */
// Define the fields of a dynamic deque for a given type
// Usage: typedef struct { DQ_FIELDS(int); } IntDeque;
#define DQ_FIELDS(T) \
	T *items;		 \
	size_t head;		 \
	size_t count;		 \
	size_t capacity
// Resize and reorganize the deque when full
#define DQ_RESIZE(T, deque) do { \
	size_t new_capacity = (deque)->capacity ? (deque)->capacity * 2 : 8; \
	T *new_items = (T *)malloc(new_capacity * sizeof(T)); \
	if ((deque)->count > 0) { \
		size_t tail = ((deque)->head + (deque)->count) % (deque)->capacity; \
		if ((deque)->head < tail) { \
			memcpy(new_items, (deque)->items + (deque)->head, (deque)->count * sizeof(T)); \
		} else { \
			size_t first_part = (deque)->capacity - (deque)->head; \
			memcpy(new_items, (deque)->items + (deque)->head, first_part * sizeof(T)); \
			memcpy(new_items + first_part, (deque)->items, tail * sizeof(T)); \
		} \
	} \
	free((deque)->items); \
	(deque)->items = new_items; \
	(deque)->capacity = new_capacity; \
	(deque)->head = 0; \
} while(0)
// Push an item to the back of the deque
#define DQ_PUSH_BACK(T, deque, item) do { \
	if ((deque)->count == (deque)->capacity) { \
		DQ_RESIZE(T, deque); \
	} \
	size_t tail = ((deque)->head + (deque)->count) % (deque)->capacity; \
	(deque)->items[tail] = item; \
	(deque)->count++; \
} while(0)
// Push an item to the front of the deque
#define DQ_PUSH_FRONT(T, deque, item) do { \
	if ((deque)->count == (deque)->capacity) { \
		DQ_RESIZE(T, deque); \
	} \
	(deque)->head = ((deque)->head - 1 + (deque)->capacity) % (deque)->capacity; \
	(deque)->items[(deque)->head] = item; \
	(deque)->count++; \
} while(0)
// Pop an item from the back of the deque
#define DQ_POP_BACK(T, deque) \
	((deque)->count > 0 ? \
		((deque)->count--, \
		 (deque)->items[((deque)->head + (deque)->count) % (deque)->capacity]) \
		: (T){0})
// Pop an item from the front of the deque
#define DQ_POP_FRONT(T, deque) \
	((deque)->count > 0 ? \
		((deque)->count--, \
		 (deque)->items[(deque)->head], \
		 (deque)->head = ((deque)->head + 1) % (deque)->capacity, \
		 (deque)->items[((deque)->head - 1 + (deque)->capacity) % (deque)->capacity]) \
		: (T){0})
// Get an item from the deque by index
#define DQ_GET(T, deque, index) \
	(((index) < (deque)->count && (index) >= 0) ? \
		(T)(deque)->items[((deque)->head + (index)) % (deque)->capacity] : (T){0})
// Get the front item of the deque
#define DQ_FRONT(T, deque) DQ_GET(T, deque, 0)
// Get the back item of the deque
#define DQ_BACK(T, deque) DQ_GET(T, deque, (deque)->count - 1)

/** 
 * Minimum and Maximum macros
 */
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

/**
 * Open and close IO streams
 */
#include "argparse.h"
#include <stdio.h>
int open_io(Args *args, FILE **output);
void close_io(FILE *output);

/**
 * TODO macro
 */
#define TODO() do { \
	fprintf(stderr, "Error: TODO at %s:%d\n", __FILE__, __LINE__); \
	exit(1); \
} while(0)

#endif // HELPERS_H
