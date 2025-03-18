#include "common/types.h"

int32_t strlen(const char *str)
{
    const char *s = str;
    while (*s != '\0')
    {
        s++;
    }
    return (int32_t)(s - str);
}
int32_t strcmp(const char *s1, const char *s2)
{
    while (*s1 && (*s1 == *s2))
    {
        s1++;
        s2++;
    }
    return *(const uint8_t *)s1 - *(const uint8_t *)s2;
}