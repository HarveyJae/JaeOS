#ifndef __COMMON_TYPES__H__
#define __COMMON_TYPES__H__

/* 无符号类型 */
typedef unsigned long uint64_t;
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

/* 有符号类型 */
typedef signed long int64_t;
typedef signed int int32_t;
typedef signed short int16_t;
typedef signed char int8_t;

/* 寄存器类型*/
/* RV-64对应64bit寄存器*/
typedef uint64_t register_t;

/* NULL定义 */
#define NULL ((void *)0)

/* 页表项PTE定义 */
typedef uint64_t pte_t;

/**
 * @brief 将地址向上对齐到指定边界
 * @param a     原始地址
 * @param align 对齐值（必须为 2 的幂）
 */
#define ADDRALIGN(a, align) (((a) + (align) - 1) & ~((align) - 1))
#endif  /* !__COMMON_TYPES__H__ */