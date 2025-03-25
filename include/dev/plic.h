#ifndef __DEV_PLIC__H__
#define __DEV_PLIC__H__

#include "common/types.h"
#include "mmu/mmu.h"

/**
 * @brief 中断优先级寄存器，设置每个中断源的优先级，值越大优先级越高
 *        每个中断源都有一个优先级寄存器，位宽32bit
 *        中断源ID从1开始
 *
 */
#define PLIC_PRIORITY(interrupt_id) (PLIC_BASE + (0x04 * interrupt_id))
/**
 * @brief 中断待处理状态寄存器，读取该寄存器确定哪些中断源处于待处理状态
 *        每个位表示一个中断源(bit0表示中断源0，1表示待处理)，位宽32bit
 *
 */
#define PLIC_PENDING (PLIC_BASE + 0x1000)
/**
 * @brief M-Mode中断使能寄存器(针对特定Hart)
 *        每个位表示一个中断源(bit2表示中断源2，1表示使能)，位宽32bit
 * 
 */
#define PLIC_MENABLE(hart_id) (PLIC_BASE + 0x2000 + (hart_id) * 0x100)
/**
 * @brief S-Mode中断使能寄存器(针对特定Hart)
 *        每个位表示一个中断源(bit2表示中断源2，1表示使能)，位宽32bit
 * 
 */
#define PLIC_SENABLE(hart_id) (PLIC_BASE + 0x2080 + (hart_id) * 0x100)
/**
 * @brief M-Mode中断优先级阈值寄存器(针对特定Hart)
 *        设置 HART 处理中断的最低优先级阈值，仅优先级高于此值的中断才会被处理
 * 
 */
#define PLIC_MPRIORITY(hart_id) (PLIC_BASE + 0x200000 + (hart_id) * 0x2000)
/**
 * @brief S-Mode中断优先级阈值寄存器(针对特定Hart)
 *        设置 HART 处理中断的最低优先级阈值，仅优先级高于此值的中断才会被处理
 * 
 */
#define PLIC_SPRIORITY(hart_id) (PLIC_BASE + 0x201000 + (hart_id) * 0x2000)
/**
 * @brief M-Mode中断声明寄存器(针对特定Hart)
 *        读取该寄存器以获取当前要处理的中断源ID(uint32_t类型)
 *        写入中断源ID以通知PLIC该中断已处理
 */
#define PLIC_MCLAIM(hart_id) (PLIC_BASE + 0x200004 + (hart_id) * 0x2000)
/**
 * @brief S-Mode中断声明寄存器(针对特定Hart)
 *        读取该寄存器以获取当前要处理的中断源ID(uint32_t类型)
 *        写入中断源ID以通知PLIC该中断已处理
 */
#define PLIC_SCLAIM(hart_id) (PLIC_BASE + 0x201004 + (hart_id) * 0x2000)

/* 中断号定义(RISC-V中不使用IRQ表示中断号)*/
#define VIRTIO_0_IR_ID 1
#define UART0_IR_ID 10
#define RTC_IR_ID 11

/* functions*/
void plic_init(uint64_t hart_id);
uint32_t plic_claim(uint64_t hart_id);
void plic_complete(uint32_t interrupt_id, uint64_t hart_id);
#endif /* !__DEV_PLIC__H__*/