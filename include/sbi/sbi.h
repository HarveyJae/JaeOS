#ifndef __HAL_SBI__H__
#define __HAL_SBI__H__

#include "../../../include/common/types.h"

/* SBI的标准返回结构 */
typedef struct
{
	/* data */
	uint64_t error;
	uint64_t value;
} SBI_RET;

/* SBI错误码定义 */
#define SBI_SUCCESS 0			 /* 调用成功 */
#define SBI_ERR_FAILED -1		 /* 通用错误 */
#define SBI_ERR_NOT_SUPPORTED -2 /* 扩展未实现 */
#define SBI_ERR_INVALID_PARAM -3 /* 参数非法 */
/* HSM扩展下对Hart状态的定义 */
/* 由sbi_hart_get_status函数返回 */
#define SBI_HSM_HART_STATUS_STOPPED 0	/* Hart已停止 */
#define SBI_HSM_HART_STATUS_STARTING 1	/* Hart正在启动中 */
#define SBI_HSM_HART_STATUS_STARTED 2	/* Hart已启动并正在运行 */
#define SBI_HSM_HART_STATUS_SUSPENDED 3 /* Hart已挂起 */
#define SBI_HSM_HART_STATUS_UNKNOWN 4	/* Hart状态未知 */
/* SBI扩展 */
#define SBI_BASE_EXTEXSION_ID 0x10				  /* 基础扩展 */
#define SBI_TIMER_EXTENSION_ID 0x54494D52		  /* 定时器扩展 */
#define SBI_IPI_EXTENSION_ID 0x735049			  /* 核间中断扩展 */
#define SBI_DEBUG_CONSOLE_EXTENSION_ID 0x4442434E /* 控制台扩展 */
#define SBI_RFENCE_EXTEXSION_ID 0x52464E43		  /* 内存一致性扩展 */
#define SBI_HSM_EXTEXSION_ID 0x48534D			  /* 多核管理扩展 */
#define SBI_SRST_EXTEXSION_ID 0x53525354		  /* 系统复位扩展 */

/* SBI函数ID */
/* BASE_EXTENSION */
#define SBI_GET_SPEC_VERSION 0 /* 获取SBI规范版本号 */
#define SBI_GET_IMPL_ID 1	   /* 获取SBI实现的ID */
#define SBI_GET_IMPL_VERSION 2 /* 获取SBI实现的版本号 */
#define SBI_PROBE_EXTENSION 3  /* 检查某个扩展是否实现 */
#define SBI_GET_MVENDORID 4	   /* 获取机器厂商ID */
#define SBI_GET_MARCHID 5	   /* 获取机器架构ID */
#define SBI_GET_MIMPID 6	   /* 获取机器实现ID */
/* DEBUG_CONSOLE_EXTENSION */
#define SBI_PUT_CHAR 0 /* 向SBI控制的串口输出字符 */
#define SBI_GET_CHAR 1 /* 从SBI控制的串口中获取字符 */
/* TIMER_EXTENSION */
#define SBI_SET_TIMER 0 /* 设置定时器的中断时间 */
/* IPI_EXTENSION */
#define SBI_SEND_IPI 0 /* 向指定的CPU发送IPI */
/* RFENCE */
#define SBI_REMOTE_FENCE_I 0		/* 远程指令缓存刷新 */
#define SBI_REMOTE_FENCE_VMA 1		/* 远程虚拟内存地址刷新 */
#define SBI_REMOTE_FENCE_VMA_ASID 2 /*带ASID的远程虚拟内存地址刷新 */
/* HSM_EXTENSION */
#define SBI_HART_START 0	  /* 启动指定的CPU核 */
#define SBI_HART_STOP 1		  /* 停止当前的CPU核 */
#define SBI_HART_GET_STATUS 2 /* 获取指定的CPU核状态 */
#define SBI_HART_SUSPEND 3	  /* 挂起指定的CPU核 */
#define SBI_HART_RESUME 4	  /* 恢复指定的CPU核 */
/* SRST_EXTENSION */
#define SBI_SYSTEM_RESET 0 /* 复位系统 */

/**
 * SBI的所有函数的参数都需要时uint64_t类型
 * 注意，这里不依赖GCC支持的宏语句表达式({})，宏展开后代码可读性差，因此采用内联汇编函数的形式
 */
/*
#define SBI_ECALL(ext_id, func_id, a0, a1, a2, a3, a4)                                                                 \
	({                                                                                                             \
		uint64_t error = 0, value = 0;                                                                    \
		register uint64_t _a0 asm("a0") = (uint64_t)(a0);                                            \
		register uint64_t _a1 asm("a1") = (uint64_t)(a1);                                            \
		register uint64_t _a2 asm("a2") = (uint64_t)(a2);                                            \
		register uint64_t _a3 asm("a3") = (uint64_t)(a3);                                            \
		register uint64_t _a4 asm("a4") = (uint64_t)(a4);                                            \
		register uint64_t _a6 asm("a6") = (uint64_t)(func_id);                                       \
		register uint64_t _a7 asm("a7") = (uint64_t)(ext_id);                                        \
		asm volatile("ecall\n"                                                                                 \
				 "mv %[error], a0\n"                                                                       \
				 "mv %[value], a1\n"                                                                       \
				 : "+r"(_a0), "+r"(_a1), [error] "=r"(error), [value] "=r"(value)                            \
				 : "r"(_a2), "r"(_a3), "r"(_a4), "r"(_a6), "r"(_a7)                                             \
				 : "memory");                                                                              \
		(SBI_RET){error, value};                                                                         \
	})
*/

/**
 * 通过内联函数汇编的形式实现sbi_ecall
 */
static inline SBI_RET sbi_ecall(uint64_t ext_id, uint64_t func_id, uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4)
{
	register uint64_t __ext_id asm("a7") = ext_id;
	register uint64_t __func_id asm("a6") = func_id;
	register uint64_t __a0 asm("a0") = a0;
	register uint64_t __a1 asm("a1") = a1;
	register uint64_t __a2 asm("a2") = a2;
	register uint64_t __a3 asm("a3") = a3;
	register uint64_t __a4 asm("a4") = a4;

	uint64_t error, value;

	asm volatile("ecall\n"
				 "mv %[error], a0\n"
				 "mv %[value], a1\n"
				 : [__a0] "+r"(__a0), [__a1] "+r"(__a1), [error] "=r"(error), [value] "=r"(value)
				 : [__ext_id] "r"(__ext_id), [__func_id] "r"(__func_id), [__a2] "r"(__a2), [__a3] "r"(__a3), [__a4] "r"(__a4)
				 : "memory");
	return (SBI_RET){error, value};
}
/* 串口控制 */
static inline SBI_RET sbi_putchar(uint64_t ch)
{
	return sbi_ecall(SBI_DEBUG_CONSOLE_EXTENSION_ID,
					 SBI_PUT_CHAR,
					 ch,
					 (uint64_t)0, (uint64_t)0, (uint64_t)0, (uint64_t)0);
}
static inline SBI_RET sbi_getchar(void)
{
	return sbi_ecall(SBI_DEBUG_CONSOLE_EXTENSION_ID,
					 SBI_GET_CHAR,
					 (uint64_t)0, (uint64_t)0, (uint64_t)0, (uint64_t)0, (uint64_t)0);
}
/* Base Extension */
/* 获取硬件架构 */
static inline SBI_RET sbi_get_marchid(void)
{
	return sbi_ecall(SBI_BASE_EXTEXSION_ID,
					 SBI_GET_MARCHID,
					 (uint64_t)0, (uint64_t)0, (uint64_t)0, (uint64_t)0, (uint64_t)0);
}
/**
 * 执行此函数后，函数会在stime_value时刻触发一次时钟中断
 * 注意是时刻而不是时间间隔，因此中断时需要将这个值刷新为“当前时间+中断间隔”
 * 该时钟是核内时钟，每个核的时钟不同
 * QEMU VIRT的时钟频率是10MHz
 */
static inline SBI_RET sbi_set_timer(uint64_t stime_value)
{
	return sbi_ecall(SBI_TIMER_EXTENSION_ID,
					 SBI_SET_TIMER,
					 stime_value,
					 (uint64_t)0, (uint64_t)0, (uint64_t)0, (uint64_t)0);
}
static inline SBI_RET sbi_send_ipi(uint64_t hart_mask, uint64_t hart_mask_base)
{
	return sbi_ecall(SBI_IPI_EXTENSION_ID,
					 SBI_SEND_IPI,
					 hart_mask,
					 hart_mask_base,
					 (uint64_t)0, (uint64_t)0, (uint64_t)0);
}
static inline SBI_RET sbi_rfence_fence_i(uint64_t hart_mask, uint64_t hart_mask_base)
{
	return sbi_ecall(SBI_RFENCE_EXTEXSION_ID,
					 SBI_REMOTE_FENCE_I,
					 hart_mask,
					 hart_mask_base,
					 (uint64_t)0, (uint64_t)0, (uint64_t)0);
}
static inline SBI_RET sbi_rfence_fence_vma(uint64_t hart_mask, uint64_t hart_mask_base, uint64_t start_addr, uint64_t size)
{
	return sbi_ecall(SBI_RFENCE_EXTEXSION_ID,
					 SBI_REMOTE_FENCE_VMA,
					 hart_mask,
					 hart_mask_base,
					 start_addr,
					 size,
					 (uint64_t)0);
}
static inline SBI_RET sbi_rfence_fence_vma_asid(uint64_t hart_mask, uint64_t hart_mask_base, uint64_t start_addr, uint64_t size, uint64_t asid)
{
	return sbi_ecall(SBI_RFENCE_EXTEXSION_ID,
					 SBI_REMOTE_FENCE_VMA_ASID,
					 hart_mask,
					 hart_mask_base,
					 start_addr,
					 size,
					 asid);
}
/**
 * 启动hartid
 * start_addr是该hart在S态启动时的初始地址
 * opaque是传递给hart的第二个参数(a1)
 */
static inline SBI_RET sbi_hart_start(uint64_t hart_id, uint64_t start_addr, uint64_t opaque)
{
	return sbi_ecall(SBI_HSM_EXTEXSION_ID,
					 SBI_HART_START,
					 hart_id,
					 start_addr,
					 opaque,
					 (uint64_t)0,
					 (uint64_t)0);
}
/**
 * 请求SBI关闭发起调用的核
 * 必须在S-Mode下执行
 */
static inline SBI_RET sbi_hart_stop(void)
{
	return sbi_ecall(SBI_HSM_EXTEXSION_ID,
					 SBI_HART_STOP,
					 (uint64_t)0, (uint64_t)0, (uint64_t)0, (uint64_t)0, (uint64_t)0);
}
static inline SBI_RET sbi_hart_get_status(uint64_t hart_id)
{
	return sbi_ecall(SBI_HSM_EXTEXSION_ID,
					 SBI_HART_GET_STATUS,
					 hart_id,
					 (uint64_t)0, (uint64_t)0, (uint64_t)0, (uint64_t)0);
}
static inline SBI_RET sbi_hart_suspend(uint64_t suspend_type, uint64_t resume_addr, uint64_t opaque)
{
	return sbi_ecall(SBI_HSM_EXTEXSION_ID,
					 SBI_HART_SUSPEND,
					 suspend_type,
					 resume_addr,
					 opaque,
					 (uint64_t)0, (uint64_t)0);
}
static inline SBI_RET sbi_system_reset(uint64_t reset_type, uint64_t reset_reason)
{
	return sbi_ecall(SBI_SRST_EXTEXSION_ID,
					 SBI_SYSTEM_RESET,
					 reset_type,
					 reset_reason,
					 (uint64_t)0, (uint64_t)0, (uint64_t)0);
}
#endif /* !__HAL_SBI__H__ */