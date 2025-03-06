#include "uart.h"
#include "sbi.h"


#ifdef  USE_QEMU_VIRT
void uart_putchar(int64_t ch)
{
    sbi_putchar(ch);
}
int64_t uart_getchar(void)
{
    return sbi_getchar().value;
}
void uart_init(void)
{
    /* 当使用QEMU的时候,初始化函数什么都不需要做 */
    /* do nothing*/
    /* 下面的代码仅测试用 */
    uart_putchar('U');
    uart_putchar('A');
    uart_putchar('R');
    uart_putchar('T');
    uart_putchar(' ');
    uart_putchar('I');
    uart_putchar('n');
    uart_putchar('i');
    uart_putchar('t');
    uart_putchar(' ');
    uart_putchar('S');
    uart_putchar('U');
    uart_putchar('C');
    uart_putchar('C');
    uart_putchar('E');
    uart_putchar('S');
    uart_putchar('S');
    uart_putchar('\n');
}
#endif  //USE_QEMU_VIRT