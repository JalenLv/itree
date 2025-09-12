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
// Init a dynamic array pointer
#define DA_INIT(type) (type *){0}
// Push an item to the dynamic array
#define DA_PUSH(type, arr, item) do { \
	if ((arr)->count == (arr)->capacity) { \
		(arr)->capacity = (arr)->capacity ? (arr)->capacity * 2 : 1; \
		(arr)->items = (type *)realloc((arr)->items, (arr)->capacity * sizeof(type)); \
	} \
	(arr)->items[(arr)->count++] = item; \
} while(0)
// Pop an item from the dynamic array
#define DA_POP(type, arr) ((arr)->count > 0 ? (type)(arr)->items[--(arr)->count] : NULL)
// Get an item from the dynamic array by index
#define DA_GET(type, arr, index) ((index) < (arr)->count ? (type)(arr)->items[index] : NULL)

#endif // HELPERS_H