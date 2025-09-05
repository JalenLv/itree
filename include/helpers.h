#ifndef HELPERS_H
#define HELPERS_H

/**
 * concatenates multiple c strings into a newly allocated string.
 * The caller is responsible for freeing the returned string.
 */
char *concat_NULL(const char *str1, ...);
#define concat(str1, ...) concat_NULL(str1, __VA_ARGS__, NULL)

#endif // HELPERS_H