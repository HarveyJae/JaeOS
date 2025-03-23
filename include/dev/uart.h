#ifndef __DEV_UART__H__
#define __DEV_UART__H__

#include "common/types.h"

#define UART_TX 0x00
#define UART_RX 0x04
#define UART_TX_FULL_MASK 0x80000000
#define UART_RX_EMPTY_MASK 0x80000000
#define UART_RX_DATA_MASK 0x000000FF

void uart_init(void);
int8_t uart_getchar(void);
void uart_putchar(uint8_t ch);
#endif  /* !__DEV_UART__H__ */