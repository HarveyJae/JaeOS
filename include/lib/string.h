#ifndef __LIB_STRING__H__
#define __LIB_STRING__H__

#include "common/types.h"
int32_t strlen(const char *str);
int32_t strcmp(const char *s1, const char *s2);
void *memset(void *dest, int32_t val, int32_t count);
#endif  /* !__LIB_STRING__H__*/