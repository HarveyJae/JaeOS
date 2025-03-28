#ifndef __COMMON_TYPES__H__
#define __COMMON_TYPES__H__

/* 无符号类型*/
typedef unsigned long uint64_t;
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

/* 有符号类型*/
typedef signed long int64_t;
typedef signed int int32_t;
typedef signed short int16_t;
typedef signed char int8_t;

/* bool类型*/
typedef uint8_t bool;
#define true 1
#define false 0

/* 寄存器类型*/
/* RV-64对应64bit寄存器*/
typedef uint64_t register_t;

/* NULL定义 */
#define NULL ((void *)0)

/* 页表项PTE定义 */
typedef uint64_t pte_t;

/* 进程相关定义*/
typedef uint32_t pid_t;     /* 进程号类型*/
typedef uint64_t uintptr_t; /* 地址类型*/
typedef int32_t err_t;      /* 退出类型*/
typedef uint32_t tid_t;     /* 线程号类型*/

/* 信号相关*/
typedef uint32_t uid_t;
/**
 * @brief 将地址向上对齐到指定边界
 * @param a     原始地址
 * @param align 对齐值(必须为 2 的幂)
 */
#define ADDRALIGNUP(a, align) (((a) + (align) - 1) & ~((align) - 1))
/**
 * @brief 将地址向下对齐到指定边界(获取当前地址所在页面的起始地址)
 * @param a     原始地址
 * @param align 对齐值(必须为 2 的幂)
 */
#define ADDRALIGNDOWN(a, align) (((a)) & ~((align) - 1))

#endif /* !__COMMON_TYPES__H__ */