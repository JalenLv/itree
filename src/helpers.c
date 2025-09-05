#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

char *concat_NULL(const char *str1, ...)
{
    if (str1 == NULL) {
        return NULL;
    }

    va_list args;
    size_t total_length = strlen(str1);

    va_start(args, str1);
    const char *str;
    while ((str = va_arg(args, const char *)) != NULL) {
        total_length += strlen(str);
    }
    va_end(args);

    char *result = (char *)malloc(total_length + 1);
    if (result == NULL) {
        return NULL;
    }

    strcpy(result, str1);
    va_start(args, str1);
    while ((str = va_arg(args, const char *)) != NULL) {
        strcat(result, str);
    }
    va_end(args);

    return result;
}
