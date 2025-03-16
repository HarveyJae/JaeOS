#ifndef __DEV_UART__H__
#define __DEV_UART__H__

#include "common/types.h"
void uart_init(void);
int8_t uart_getchar(void);
void uart_putchar(uint8_t ch);
#endif  /* !__DEV_UART__H__ */