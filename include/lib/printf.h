#ifndef __LIB_PRINTF__H__
#define __LIB_PRINTF__H__
#include "common/types.h"
void early_printf(const char *fmt, ...);
void logo_init(void);
void printf_init(void);
void printf(const char *fmt, ...);
#endif /* !__LIB_PRINTF__H__ */