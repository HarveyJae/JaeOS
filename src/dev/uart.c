#include "common/types.h"
#include "dev/uart.h"
#include "mmu/mmu.h"
#define __io_br() \
    do            \
    {             \
    } while (0) // 读操作前的屏障（此处为空）
#define __io_ar() __asm__ __volatile__("fence i,r" : : : "memory"); // 读操作后的屏障
#define __io_bw() __asm__ __volatile__("fence w,o" : : : "memory"); // 写操作前的屏障
#define __io_aw() \
    do            \
    {             \
    } while (0) // 写操作后的屏障（此处为空）
/**
 * @brief
 *
 * @param addr
 * @return uint32_t
 */
static inline uint32_t __raw_readl(const volatile void *addr)
{
    uint32_t val;

    asm volatile("lw %[val], 0(%[addr])" : [val] "=r"(val) : [addr] "r"(addr));
    return val;
}
/**
 * @brief
 *
 * @param val
 * @param addr
 */
static inline void __raw_writel(uint32_t val, volatile void *addr)
{
    asm volatile("sw %[val], 0(%[addr])" : : [val] "r"(val), [addr] "r"(addr));
}
/**
 * @brief 
 * 
 */
#define readl(c)	({ uint32_t __v; __io_br(); __v = __raw_readl(c); __io_ar(); __v; })
/**
 * @brief 
 * 
 */
#define writel(v,c)	({ __io_bw(); __raw_writel((v),(c)); __io_aw(); })
/**
 * @brief
 *
 */
void uart_init(void)
{
    /* nothing to do*/
    /* sbi help us init virt-uart*/
}
/**
 * @brief
 *
 * @return int
 */
int8_t uart_getchar(void)
{
    uint32_t *uart_rx = (uint32_t *)(UART0_BASE + UART_RX);
    uint32_t ret = readl(uart_rx);
    /* No Data*/
    if (ret & UART_RX_EMPTY_MASK)
    {
        return -1;
    }
    /* return DATA*/
    return ret & UART_RX_DATA_MASK;
}
void uart_putchar(uint8_t ch)
{
    uint32_t *uart_tx = (uint32_t *)(UART0_BASE + UART_TX);
    while (readl(uart_tx) & UART_TX_FULL_MASK)
        ;
    writel(ch, uart_tx);
}