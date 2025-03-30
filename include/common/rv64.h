#ifndef __COMMON_RV64__H__
#define __COMMON_RV64__H__

#include "types.h"
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
 * instruction表示汇编指令：使用%0作为参数的占位符，但%0(1/2/3/4)这样的可读性差，推荐使用符号表示
 * OutputOprands:"=r"(x)
 * InputOprands:"r"(x)
 * 注意，asm中的冒号之间的相对顺序不能变，如果没有输出或没有输入应用空白符表示，但冒号要有
 */

/**
 * @brief 获取当前核心的id，S-Mode下是非法指令
 *
 * @return uint64_t
 */
static inline uint64_t read_mhartid(void)
{
	uint64_t hartid;
	asm volatile("csrr %[hart_id], mhartid" : [hart_id] "=r"(hartid) : :);
	/* =r会约束编译器将hart_id变量分配到寄存器中，因此这里不需要声明memory */
	/* 但是这个分配是随机的，编译器会随机选择通用寄存器进行分配 */
	/* 如果想显示的分配到某个寄存器，需要将变量和寄存器显示关联 */
	/* register uint64_t hart_id asm("a7")，这会将hart_id变量强制分配到a7寄存器 */
	return hartid;
}
/**
 * @brief 获取当前核心的运行模式，S-Mode下是非法指令
 *
 *
 * @return uint64_t
 */
static inline uint64_t read_mstatus(void)
{
	uint64_t mode;
	asm volatile("csrr %[mode], mstatus" : [mode] "=r"(mode) :);
	return mode;
}
/**
 * @brief 更改当前核心的运行模式，S-Mode下是非法指令
 *
 * @param mode
 */
static inline void write_mstatus(uint64_t mode)
{
	asm volatile("csrw mstatus, %[mode]" : : [mode] "r"(mode));
}
/**
 * @brief 写mepc寄存器，M-Mode的异常返回地址，可以配合mret实现模式切换，S-Mode下是非法指令
 *
 * @param addr
 */
static inline void write_mepc(uint64_t addr)
{
	asm volatile("csrw mepc, %[addr]" : : [addr] "r"(addr));
}
/**
 * @brief 写medeleg寄存器，异常委托，S-Mode下是非法指令
 *
 * @param x
 */
static inline void write_medeleg(uint64_t x)
{
	asm volatile("csrw medeleg, %[x]" : : [x] "r"(x));
}
/**
 * @brief 写mideleg寄存器，中断委托，S-Mode下是非法指令
 *
 * @param x
 */
static inline void write_mideleg(uint64_t x)
{
	asm volatile("csrw mideleg, %[x]" : : [x] "r"(x));
}
/**
 * @brief 写satp寄存器，S-Mode指令
 *
 * @param x
 */
static inline void write_satp(uint64_t x)
{
	asm volatile("csrw satp, %[x]" : : [x] "r"(x));
}
/**
 * @brief 读取a0寄存器，U-Mode指令，注意该指令只允许在内核启动过程中(s0，s1不能被污染)调用，opensbi会将hartid存储在s0寄存器中
 *
 * @return uint64_t
 */
static inline uint64_t read_hart_id(void)
{
	uint64_t hart_id;
	asm volatile("mv %[hart_id], a2" : [hart_id] "=r"(hart_id));
	return hart_id;
}
/**
 * @brief 读取a1寄存器，U-Mode指令，注意该指令只允许在内核启动过程中(s0，s1不能被污染)调用，opensbi会将dtb_entry存储在s1寄存器中
 *
 * @return uint64_t
 */
static inline uint64_t read_dtb_entry(void)
{
	uint64_t dtb_entry;
	asm volatile("mv %[dtb_entry], a3" : [dtb_entry] "=r"(dtb_entry));
	return dtb_entry;
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
	asm volatile("csrr %[x], sie" : [x] "=r"(x) :);
	return x;
}
static inline void write_sie(uint64_t x)
{
	asm volatile("csrw sie, %[x]" : : [x] "r"(x));
}

/**
 * S-mode层级的Trap-Vector Base Address
 * |63  -  2|1  -  0|
 * |base_adr|  mode |
 */
static inline void write_stvec(uint64_t addr)
{
	asm volatile("csrw stvec, %[addr]" : : [addr] "r"(addr));
}

static inline uint64_t read_stvec(void)
{
	uint64_t addr;
	asm volatile("csrr %[addr], stvec" : [addr] "=r"(addr) :);
	return addr;
}
/* S-mode层级的寄存器status*/
#define SSTATUS_SPP_MASK (1L << 8)
#define SSTATUS_SPIE_MASK (1L << 5)
#define SSTATUS_UPIE_MASK (1L << 4)
#define SSTATUS_SIE_MASK (1L << 1)
#define SSTATUS_UIE_MASK (1L << 0)
/**
 * @brief 关闭S-Mode Interupt
 *
 * @return uint64_t
 */
static inline uint64_t disable_si(void)
{
	/* csrrc，原子读-清除-写指令，i表示立即数*/
	/* 将原数据写入ret寄存器，清除立即数指定的位(0 - 31)*/
	uint64_t ret;
	asm volatile("csrrci %[ret], sstatus, %[sie_mask]" : [ret] "=&r"(ret) : [sie_mask] "i"(SSTATUS_SIE_MASK));
	return (ret & (SSTATUS_SIE_MASK));
	/* 返回之前SIE域的值*/
}
/**
 * @brief 开启S-Mode Interrupt
 *
 * @return uint64_t
 */
static inline uint64_t enable_si(void)
{
	/* csrrs，原子读-置位-写指令，i表示立即数*/
	/* 将数据写入ret寄存器，置位立即数指定的位(0 - 31)*/
	uint64_t ret;
	asm volatile("csrrsi %[ret], sstatus, %[sie_mask]" : [ret] "=&r"(ret) : [sie_mask] "i"(SSTATUS_SIE_MASK));
	return (ret & (SSTATUS_SIE_MASK));
	/* 返回之前SIE域的值*/
}
/**
 * @brief 恢复sstatus之前的值
 *
 * @param sie
 */
static inline void restore_si(uint64_t sie)
{
	if (sie)
	{
		asm volatile("csrs sstatus, %0" : : "r"(SSTATUS_SIE_MASK));
	}
	else
	{
		asm volatile("csrc sstatus, %0" : : "r"(SSTATUS_SIE_MASK));
	}
}
/**
 * @brief 读取sstatus的值
 * 
 * @return uint64_t 
 */
static inline uint64_t read_sstatus(void)
{
	uint64_t val;
	asm volatile("csrr %[val], sstatus" : [val] "=r"(val));
	return val;
}
/**
 * @brief 判断当前的中断状态
 * 
 * @return uint64_t 
 *         0：中断关闭
 *         1: 中断开启
 */
static inline uint64_t get_si(void)
{
	uint64_t val = read_sstatus();
	return val & SSTATUS_SIE_MASK;
}
/**
 * @brief 获取中断/异常原因: |63|62------0|
 *                    1.63位：1表示中断，0表示异常
 *                    2.62-0：原因代码
 *
 *
 * @return uint64_t
 */
#define SCAUSE_TRAP_CODE_LEN (63)
#define SCAUSE_TRAP_CODE_MASK ((1ul << SCAUSE_TRAP_CODE_LEN) - 1)
#define SCAUSE_EXCEPTION (0ul)
#define SCAUSE_INTERRUPT (1ul)
#define INTERRUPT_TIMER (5)	   /* 定时器中断*/
#define INTERRUPT_EXTERNEL (9) /* 外部中断*/
static inline uint64_t read_scause(void)
{
	uint64_t cause;
	asm volatile("csrr %[cause], scause" : [cause] "=r"(cause));
	return cause;
}

/**
 * @brief 读取时钟计数器的值
 *
 * @return uint64_t
 */
static inline uint64_t read_rdtime(void)
{
	uint64_t tcount;
	asm volatile("rdtime %[tcount]" : [tcount] "=r"(tcount));
	return tcount;
}
#endif /* __COMMON_RV64__H__ */