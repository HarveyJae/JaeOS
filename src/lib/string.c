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
/**
 * @brief 支持64位对齐加速
 * 
 * @param dest 
 * @param val 
 * @param count 
 * @return void* 
 */
void *memset(void *dest, int32_t val, int32_t count) 
{
    uint8_t *d = dest;
    const uint8_t v = (uint8_t)val;
    uint64_t ptr = (uint64_t)d;

    /* 非对齐前导字节处理 */
    while (count && (ptr % sizeof(uint64_t))) {
        *d++ = v;
        ptr++;
        count--;
    }

    /* 64位对齐加速 */
    if (count >= sizeof(uint64_t)) {
        const uint64_t pattern = v * 0x0101010101010101ULL;
        uint64_t *q = (uint64_t *)d;
        
        /* 展开循环提升流水线效率 */
        for (int32_t i = count / sizeof(uint64_t); i >= 8; i -= 8) {
            q[0] = pattern;
            q[1] = pattern;
            q[2] = pattern;
            q[3] = pattern;
            q[4] = pattern;
            q[5] = pattern;
            q[6] = pattern;
            q[7] = pattern;
            q += 8;
        }
        
        /* 处理剩余块 */
        while (count >= sizeof(uint64_t)) {
            *q++ = pattern;
            count -= sizeof(uint64_t);
        }
        
        d = (uint8_t *)q;
    }

    /* 尾部剩余字节 */
    while (count--) {
        *d++ = v;
    }

    return dest;
}