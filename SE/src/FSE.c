#include <string.h>
#include "FSE.h"

void strtrim(char *dest, const char *src, size_t len)
{
    while (src[--len] == ' ') { }

    strncpy(dest, src, ++len);
    dest[len] = '\0';
}
