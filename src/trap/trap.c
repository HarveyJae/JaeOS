#include "common/types.h"
#include "common/rv64.h"
#include "trap/trap.h"
#include "dev/timer.h"

extern char ktrap_vector[]; /* 异常向量表地址*/
/**
 * @brief 设置异常向量表
 *
 * @param handle_addr 异常向量表入口地址
 */
void set_trap_handle(void)
{
    write_stvec((uint64_t)ktrap_vector);
}
/**
 * @brief C内核异常处理函数入口
 *
 * @param ktf
 */
void kernel_trap(ktrapframe_t *ktf)
{
    // 获取陷入中断/异常的原因
    uint64_t trap_cause = read_scause();
    uint64_t trap_type = (trap_cause >> SCAUSE_TRAP_CODE_LEN);
    uint64_t trap_code = (trap_cause & SCAUSE_TRAP_CODE_MASK);

    if (trap_type == SCAUSE_INTERRUPT)
    {
        if (trap_code == INTERRUPT_TIMER)
        {
            /* 定时器中断*/
            timer_interrupt_handler();
        }
        else if (trap_code == INTERRUPT_EXTERNEL)
        {
            /* 外部中断*/
            // trap_device();
        }
        else
        {
            /* 未定义中断*/
            while (1)
                ;
        }
    }
    else
    {
        /* 异常*/
        //printf("Uncaught Exception: sp = 0x%016lx, ra = 0x%016lx\n", ktf->sp, ktf->ra);
        while (1)
            ;
    }
}