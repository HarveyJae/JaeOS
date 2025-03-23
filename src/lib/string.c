#include "common/types.h"

/**
 * @brief 
 * 
 * @param str 
 * @return int32_t 
 */
int32_t strlen(const char *str)
{
    const char *s = str;
    while (*s != '\0')
    {
        s++;
    }
    return (int32_t)(s - str);
}
/**
 * @brief 
 * 
 * @param s1 
 * @param s2 
 * @return int32_t 
 */
int32_t strcmp(const char *s1, const char *s2)
{
    while (*s1 && (*s1 == *s2))
    {
        s1++;
        s2++;
    }
    return *(const uint8_t *)s1 - *(const uint8_t *)s2;
}
/**
 * @brief 
 * 
 * @param dst 
 * @param c 
 * @param n 
 * @return void* 
 */
void *memset(void *dst, int32_t c, uint64_t n)
{
    uint8_t ch = c;
    int32_t i;
    uint64_t data;
    uint64_t *p;

    if (__builtin_expect(!!(c == 0), 1))
    {
        data = 0;
    }
    else
    {
        data = ((uint64_t)ch << 56) | ((uint64_t)ch << 48) | ((uint64_t)ch << 40) | ((uint64_t)ch << 32) | ((uint64_t)ch << 24) | ((uint64_t)ch << 16) | ((uint64_t)ch << 8) | (uint64_t)ch;
    }

    uint8_t *cdst = (uint8_t *)dst;

    /* 开始的部分不是8对齐的*/
    if (__builtin_expect(!!((uint64_t)cdst % (8 - 1)), 0))
    {
        int32_t sum = 8 - (((uint64_t)cdst) & (8 - 1));
        for (i = 0; i < sum; i++)
        {
            cdst[i] = ch;
        }
        cdst = (uint8_t *)ADDRALIGNUP((uint64_t)cdst, 8);
        n -= sum;
    }
    if (__builtin_expect(!!(((uint64_t)cdst % 8) != 0), 0))
    {
        int32_t sum = 8 - (((uint64_t)cdst) & (8 - 1));
        for (i = 0; i < sum; i++)
        {
            cdst[i] = ch;
        }
        cdst = (uint8_t *)ADDRALIGNUP((uint64_t)cdst, 8);
        n -= sum;
    }
    p = (uint64_t *)cdst;

    /* 以8字节为单位拷贝*/
    for (i = 0; i <= n - 8; i += 8)
    {
        *p++ = data;
    }

    /* 剩下的部分不是8对齐的*/
    if (__builtin_expect((!!(i != n)), 0))
    {
        for (; i < n; i++)
        {
            cdst[i] = ch;
        }
    }
    return dst;
}