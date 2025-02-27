#ifndef __HAL_SBI__H__
#define __HAL_SBI__H__

#include "common/types.h"

/* SBI的标准返回结构 */
typedef struct
{
    /* data */
    uint64_t error;
    uint64_t value;
} SBI_RET;

/* SBI错误码定义 */
#define SBI_SUCCESS 0            /* 调用成功 */
#define SBI_ERR_FAILED -1        /* 通用错误 */
#define SBI_ERR_NOT_SUPPORTED -2 /* 扩展未实现 */
#define SBI_ERR_INVALID_PARAM -3 /* 参数非法 */
/* HSM扩展下对Hart状态的定义 */
/* 由sbi_hart_get_status函数返回 */
#define SBI_HSM_HART_STATUS_STOPPED 0   /* Hart已停止 */
#define SBI_HSM_HART_STATUS_STARTING 1  /* Hart正在启动中 */
#define SBI_HSM_HART_STATUS_STARTED 2   /* Hart已启动并正在运行 */
#define SBI_HSM_HART_STATUS_SUSPENDED 3 /* Hart已挂起 */
#define SBI_HSM_HART_STATUS_UNKNOWN 4   /* Hart状态未知 */
/* SBI扩展 */
#define SBI_BASE_EXTEXSION_ID 0x10         /* 基础扩展 */
#define SBI_TIMER_EXTENSION_ID 0x54494D45  /* 定时器扩展 */
#define SBI_IPI_EXTENSION_ID 0x735049      /* 核间中断扩展 */
#define SBI_CONSOLE_EXTENSION_ID 0x434FE53 /* 控制台扩展 */
#define SBI_RFENCE_EXTEXSION_ID 0x52464E43 /* 内存一致性扩展 */
#define SBI_HSM_EXTEXSION_ID 0x48534D      /* 多核管理扩展 */
#define SBI_SRST_EXTEXSION_ID 0x53525354   /* 系统复位扩展 */

/* SBI函数ID */
/* BASE_EXTENSION */
#define SBI_GET_SPEC_VERSION 0 /* 获取SBI规范版本号 */
#define SBI_GET_IMPL_ID 1      /* 获取SBI实现的ID */
#define SBI_GET_IMPL_VERSION 2 /* 获取SBI实现的版本号 */
#define SBI_PROBE_EXTENSION 3  /* 检查某个扩展是否实现 */
#define SBI_GET_MVENDORID 4    /* 获取机器厂商ID */
#define SBI_GET_MARCHID 5      /* 获取机器架构ID */
#define SBI_GET_MIMPID 6       /* 获取机器实现ID */
/* TIMER_EXTENSION */
#define SBI_SET_TIMER 0 /* 设置定时器的中断时间 */
/* IPI_EXTENSION */
#define SBI_SEND_IPI 0 /* 向指定的CPU发送IPI */
/* RFENCE */
#define SBI_REMOTE_FENCE_I 0        /* 远程指令缓存刷新 */
#define SBI_REMOTE_FENCE_VMA 1      /* 远程虚拟内存地址刷新 */
#define SBI_REMOTE_FENCE_VMA_ASID 2 /*带ASID的远程虚拟内存地址刷新 */
/* HSM_EXTENSION */
#define SBI_HART_START 0      /* 启动指定的CPU核 */
#define SBI_HART_STOP 1       /* 停止当前的CPU核 */
#define SBI_HART_GET_STATUS 2 /* 获取指定的CPU核状态 */
#define SBI_HART_SUSPEND 3    /* 挂起指定的CPU核 */
#define SBI_HART_RESUME 4     /* 恢复指定的CPU核 */
/* SRST_EXTENSION */
#define SBI_SYSTEM_RESET 0 /* 复位系统 */



/**
 * SBI的所有函数的参数都需要时uint64_t类型
 */
#define SBI_ECALL(ext_id, func_id, a0, a1, a2, a3, a4) \
({ \
	uint64_t error = 0, value = 0; \
	register uint64_t _a0 asm("a0") = (uint64_t)(a0);\
	register uint64_t _a1 asm("a1") = (uint64_t)(a1);\
	register uint64_t _a2 asm("a2") = (uint64_t)(a2);\
	register uint64_t _a3 asm("a3") = (uint64_t)(a3);\
	register uint64_t _a4 asm("a4") = (uint64_t)(a4);\
	register uint64_t _a6 asm("a6") = (uint64_t)(func_id);\
	register uint64_t _a7 asm("a7") = (uint64_t)(ext_id);\
	asm volatile("ecall" \
            : "+r" (a0), "+r" (a1) \
			: "r" (a2), "r" (a3), "r" (a4), "r" (a6), "r" (a7) \
			: "memory"); \
	(SBI_RET){error, value};\
})
#endif /* !__HAL_SBI__H__ */