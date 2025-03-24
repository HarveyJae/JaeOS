#include "common/types.h"
#include "dev/timer.h"
#include "sbi/sbi.h"
#include "common/rv64.h"

/**
 * @brief QEMU VIRT时钟频率为10MHz，JaeOS时钟频率为1KHz(1ms)
 *        当前函数配置定时器首次触发时间
 */
void timer_init(void)
{
    sbi_set_timer(read_rdtime() + JAEOS_TIME_CYCLES);
}
/**
 * @brief 在发生时钟中断时设置下一个tick的值
 *
 */
static void feed_timer(void)
{
    sbi_set_timer(read_rdtime() + JAEOS_TIME_CYCLES);
}
/**
 * @brief 定时器中断处理函数
 * 
 */
void timer_interrupt_handler(void)
{
    /* 更新时钟tick*/
    feed_timer();
}
/**
 * @brief 启动定时器
 * 
 */
void enable_timer_interrupt(void)
{
    unsigned long sie;
    asm volatile("csrr %0, sie" : "=r"(sie));
    sie |= (1 << 5); // STIE 是第 5 位
    asm volatile("csrw sie, %0" : : "r"(sie));
}