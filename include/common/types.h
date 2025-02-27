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

/* NULL定义 */
#define NULL ((void *)0)

/* 页表项PTE定义 */
typedef uint64_t pte_t;

#endif  /* !__COMMON_TYPES__H__ */