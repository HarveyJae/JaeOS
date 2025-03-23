#include "common/types.h"
#include "common/rv64.h"
#include "trap/trap.h"

extern void ktrap_vector(); /* 异常向量表地址*/
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
    
}