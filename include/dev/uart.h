#ifndef __DEV_UART__H__
#define __DEV_UART__H__

#include "../../../include/common/types.h"
#include "../../hal/include/sbi.h"
#define USE_QEMU_VIRT
void uart_putchar(int64_t ch);
int64_t uart_getchar(void);
void uart_init(void);
#endif  /* !__DEV_UART__H__ */