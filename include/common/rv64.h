#ifndef __COMMON_RV64__H__
#define __COMMON_RV64__H__

#include "common/types.h"
/* RV64内联汇编指令 */
/**
 * GCC扩展内联汇编支持:
 * static inline _type _name(_arg x)
 * {
 *      ...
 *      asm volatile("instruction" : "OutputOprands" : "InputOprands")
 *      ...
 *      return...
 * }
 * _type表示返回值类型
 * _name表示C函数名称
 * _arg表示传递参数类型
 * instruction表示汇编指令：使用%0作为参数的占位符
 * OutputOprands:"=r"(x)
 * InputOprands:"r"(x)
 * 注意，asm中的冒号之间的相对顺序不能变，如果没有输出或没有输入应用空白符表示，但冒号要有
 */

/* 获取当前核心的id */
static inline uint64_t get_hartid(void) 
{
	uint64_t hart_id;
	asm volatile("csrr %0, mhartid" : "=r"(hart_id) : );
	return hart_id;
}

/* 获取当前核心的运行模式 */
static inline uint64_t get_mode(void) 
{
	uint64_t mode;
	asm volatile("csrr %0, mstatus" : "=r"(mode) : );
	return mode;
}

/* 向tp寄存器中写入数据，默认情况下用来保存hartid*/
static inline void write_tp(uint64_t x) 
{
	asm volatile("mv tp, %0" : : "r"(x));
}

/* S-mode层级的寄存器sie PG.94 */
/**
 * |15 - 14|   13 |12 - 10| 9  |8 - 6| 5  |4 - 2| 1  | 0 | 
 * |   0   |LCOFIE|   0   |SEIE|  0  |STIE|  0  |SSIE| 0 |
 */
#define SIE_SEIE (1L << 9) /* 启动外设中断 */
#define SIE_STIE (1L << 5) /* 启动核内定时器中断 */
#define SIE_SSIE (1L << 1) /* 启动核间中断 */
static inline uint64_t read_sie(void) 
{
	uint64_t x;
	asm volatile("csrr %0, sie" : "=r"(x));
	return x;
}
static inline void write_sie(uint64_t x) 
{
	asm volatile("csrw sie, %0" : : "r"(x));
}

/**
 * S-mode层级的Trap-Vector Base Address
 * |63  -  2|1  -  0|
 * |base_adr|  mode |
 */
static inline void write_stvec(uint64_t addr) 
{
	asm volatile("csrw stvec, %0" : : "r"(addr));
}

static inline uint64_t r_stvec(void) 
{
	uint64_t addr;
	asm volatile("csrr %0, stvec" : "=r"(addr));
	return addr;
}
#endif  /* __COMMON_RV64__H__ */