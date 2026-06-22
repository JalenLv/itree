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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DA_FIELDS(T)                                                           \
    size_t count;                                                              \
    size_t capacity;                                                           \
    T *items;                                                                  \
    _Static_assert(1, "DA_FIELDS requires trailing semicolon")

#define DA_DEFINE(ContainerT, T)                                               \
    static inline void ContainerT##_free(ContainerT *arr) {                    \
        if (arr && arr->items) {                                               \
            free(arr->items);                                                  \
            arr->items = NULL;                                                 \
            arr->count = 0;                                                    \
            arr->capacity = 0;                                                 \
        }                                                                      \
    }                                                                          \
    static inline int ContainerT##_resize(ContainerT *arr) {                   \
        if (!arr) {                                                            \
            fprintf(stderr, "Invalid array pointer\n");                        \
            return -1;                                                         \
        }                                                                      \
        size_t new_capacity = arr->capacity ? arr->capacity * 2 : 8;           \
        T *new_items = (T *)realloc(arr->items, new_capacity * sizeof(T));     \
        if (new_items) {                                                       \
            arr->items = new_items;                                            \
            arr->capacity = new_capacity;                                      \
            return 0;                                                          \
        } else {                                                               \
            perror("Failed to resize array");                                  \
            return -1;                                                         \
        }                                                                      \
    }                                                                          \
    static inline int ContainerT##_push(ContainerT *arr, const T *item) {      \
        if (!arr || !item) {                                                   \
            fprintf(stderr, "Invalid pointer\n");                              \
            return -1;                                                         \
        }                                                                      \
        if (arr->count == arr->capacity && ContainerT##_resize(arr) != 0)      \
            return -1;                                                         \
        arr->items[arr->count++] = *item;                                      \
        return 0;                                                              \
    }                                                                          \
    static inline int ContainerT##_empty(const ContainerT *arr) {              \
        if (!arr) {                                                            \
            fprintf(stderr, "Invalid array pointer\n");                        \
            return -1;                                                         \
        }                                                                      \
        return arr->count == 0;                                                \
    }                                                                          \
    static inline int ContainerT##_pop(ContainerT *arr, T *out) {              \
        if (!arr || !out) {                                                    \
            fprintf(stderr, "Invalid pointer\n");                              \
            return -1;                                                         \
        }                                                                      \
        if (ContainerT##_empty(arr)) {                                         \
            fprintf(stderr, "Pop from empty array\n");                         \
            return -1;                                                         \
        }                                                                      \
        *out = arr->items[--arr->count];                                       \
        return 0;                                                              \
    }                                                                          \
    static inline int ContainerT##_get(const ContainerT *arr, size_t index,    \
                                       T *out) {                               \
        if (!arr || !out) {                                                    \
            fprintf(stderr, "Invalid pointer\n");                              \
            return -1;                                                         \
        }                                                                      \
        if (index >= arr->count) {                                             \
            fprintf(stderr, "Index out of bounds\n");                          \
            return -1;                                                         \
        }                                                                      \
        *out = arr->items[index];                                              \
        return 0;                                                              \
    }                                                                          \
    static inline int ContainerT##_get_ptr(const ContainerT *arr,              \
                                           size_t index, T **out) {            \
        if (!arr || !out) {                                                    \
            fprintf(stderr, "Invalid pointer\n");                              \
            return -1;                                                         \
        }                                                                      \
        if (index >= arr->count) {                                             \
            fprintf(stderr, "Index out of bounds\n");                          \
            return -1;                                                         \
        }                                                                      \
        *out = &arr->items[index];                                             \
        return 0;                                                              \
    }                                                                          \
    static inline int ContainerT##_remove_range(ContainerT *arr, size_t begin, \
                                                size_t end) {                  \
        if (!arr) {                                                            \
            fprintf(stderr, "Invalid array pointer\n");                        \
            return -1;                                                         \
        }                                                                      \
        if (begin >= arr->count || end > arr->count || begin >= end) {         \
            fprintf(stderr, "Invalid range\n");                                \
            return -1;                                                         \
        }                                                                      \
        memmove(&arr->items[begin], &arr->items[end],                          \
                (arr->count - end) * sizeof(T));                               \
        arr->count -= (end - begin);                                           \
        return 0;                                                              \
    }                                                                          \
    static inline int ContainerT##_remove(ContainerT *arr, size_t index) {     \
        return ContainerT##_remove_range(arr, index, index + 1);               \
    }                                                                          \
    static inline int ContainerT##_insert(                                     \
        ContainerT *arr, const ContainerT *other, size_t index) {              \
        if (!arr || !other) {                                                  \
            fprintf(stderr, "Invalid pointer\n");                              \
            return -1;                                                         \
        }                                                                      \
        if (index > arr->count) {                                              \
            fprintf(stderr, "Index out of bounds\n");                          \
            return -1;                                                         \
        }                                                                      \
        while (arr->count + other->count > arr->capacity) {                    \
            if (ContainerT##_resize(arr) != 0)                                 \
                return -1;                                                     \
        }                                                                      \
        memmove(&arr->items[index + other->count], &arr->items[index],         \
                (arr->count - index) * sizeof(T));                             \
        memcpy(&arr->items[index], other->items, other->count * sizeof(T));    \
        arr->count += other->count;                                            \
        return 0;                                                              \
    }                                                                          \
    typedef int (*ContainerT##_cmp_fn_t)(const void *, const void *);          \
    static inline int ContainerT##_sort_range(ContainerT *arr, size_t begin,   \
                                              size_t end,                      \
                                              ContainerT##_cmp_fn_t cmp) {     \
        if (!arr || !cmp) {                                                    \
            fprintf(stderr, "Invalid pointer\n");                              \
            return -1;                                                         \
        }                                                                      \
        if (begin >= arr->count || end > arr->count || begin >= end) {         \
            fprintf(stderr, "Invalid range\n");                                \
            return -1;                                                         \
        }                                                                      \
        qsort(&arr->items[begin], end - begin, sizeof(T), cmp);                \
        return 0;                                                              \
    }                                                                          \
    static inline int ContainerT##_sort(ContainerT *arr,                       \
                                        ContainerT##_cmp_fn_t cmp) {           \
        if (!arr) {                                                            \
            fprintf(stderr, "Invalid array pointer\n");                        \
            return -1;                                                         \
        }                                                                      \
        return ContainerT##_sort_range(arr, 0, arr->count, cmp);               \
    }                                                                          \
    _Static_assert(1, "DA_DEFINE requires trailing semicolon")

/**
 * Dynamic deque macros.
 */
// Define the fields of a dynamic deque for a given type
// Usage: typedef struct { DQ_FIELDS(int); } IntDeque;
#define DQ_FIELDS(T) 	\
	T *items;		 	\
	size_t head;		\
	size_t count;		\
	size_t capacity

// Resize and reorganize the deque when full
#define DQ_RESIZE(T, deque) do { 																\
	size_t new_capacity = (deque)->capacity ? (deque)->capacity * 2 : 8; 						\
	T *new_items = (T *)malloc(new_capacity * sizeof(T)); 										\
	if ((deque)->count > 0) { 																	\
		size_t tail = ((deque)->head + (deque)->count) % (deque)->capacity; 					\
		if ((deque)->head < tail) { 															\
			memcpy(new_items, (deque)->items + (deque)->head, (deque)->count * sizeof(T)); 		\
		} else { 																				\
			size_t first_part = (deque)->capacity - (deque)->head; 								\
			memcpy(new_items, (deque)->items + (deque)->head, first_part * sizeof(T)); 			\
			memcpy(new_items + first_part, (deque)->items, tail * sizeof(T)); 					\
		} 																						\
	} 																							\
	free((deque)->items); 																		\
	(deque)->items = new_items; 																\
	(deque)->capacity = new_capacity; 															\
	(deque)->head = 0; 																			\
} while (0)

// Push an item to the back of the deque
#define DQ_PUSH_BACK(T, deque, item) do { 										\
	if ((deque)->count == (deque)->capacity) { 									\
		DQ_RESIZE(T, deque); 													\
	} 																			\
	size_t tail = ((deque)->head + (deque)->count) % (deque)->capacity; 		\
	(deque)->items[tail] = item; 												\
	(deque)->count++; 															\
} while (0)

// Push an item to the front of the deque
#define DQ_PUSH_FRONT(T, deque, item) do { 												\
	if ((deque)->count == (deque)->capacity) { 											\
		DQ_RESIZE(T, deque); 															\
	} 																					\
	(deque)->head = ((deque)->head - 1 + (deque)->capacity) % (deque)->capacity; 		\
	(deque)->items[(deque)->head] = item; 												\
	(deque)->count++; 																	\
} while (0)

// Pop an item from the back of the deque
#define DQ_POP_BACK(T, deque) 														\
	((deque)->count > 0 ? 															\
		((deque)->count--, 															\
		 (deque)->items[((deque)->head + (deque)->count) % (deque)->capacity]) 		\
		: (T){0})

// Pop an item from the front of the deque
#define DQ_POP_FRONT(T, deque) 																\
	((deque)->count > 0 ? 																	\
		((deque)->count--, 																	\
		 (deque)->items[(deque)->head], 													\
		 (deque)->head = ((deque)->head + 1) % (deque)->capacity, 							\
		 (deque)->items[((deque)->head - 1 + (deque)->capacity) % (deque)->capacity]) 		\
		: (T){0})

// Get an item from the deque by index
#define DQ_GET(T, deque, index) 														\
	(((index) < (deque)->count && (index) >= 0) ? 										\
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
#define TODO(msg) _Static_assert(0, "TODO: " msg)

/**
 * USED macro
 */
#define USED(x) (void)(x)

#endif // HELPERS_H
