#ifndef __MMU_MMU__H__
#define __MMU_MMU__H__
#include "dev/dtb.h"
/* 内核起始地址*/
extern char __text_start[];
/* 内核代码段结束地址*/
extern char __text_end[];
/* 内核数据段结束地址*/
extern char end[];

#define PAGE_SIZE 0x1000 /* 4KB*/
#define PAGE_SHIFT 12    /* 页内偏移位数(offset)*/

#define PAGE_TABLE_SIZE 0x6400000 /* 页表总大小：100MB*/

/* UART0的物理内存起始地址*/
#define UART0_BASE ((uint64_t)ADDRALIGNUP(0x10000000ul, PAGE_SIZE))

/* RTC的物理内存起始地址*/
#define RTC_BASE ((uint64_t)ADDRALIGNUP(0x101000ul, PAGE_SIZE))

/* VirtIO的物理内存起始地址*/
#define VIRTIO_0_BASE ((uint64_t)ADDRALIGNUP(0x10001000ul, PAGE_SIZE))

/* PLIC的物理内存起始地址*/
#define PLIC_BASE ((uint64_t)ADDRALIGNUP(0x0c000000ul, PAGE_SIZE))

/* 内核的物理内存起始地址*/
#define KERNEL_BASE ((uint64_t)ADDRALIGNUP((uint64_t)__text_start, PAGE_SIZE))

/* 内核的代码段物理内存起始地址*/
#define KERNEL_TEXT_BASE KERNEL_BASE
#define KERNEL_TEXT_SIZE (((uint64_t)__text_end - (uint64_t)__text_start)) /* 内核代码段大小，ld脚本确保该值4KB对齐*/

/* 内核的数据段物理内存起始地址*/
#define KERNEL_DATA_BASE ((uint64_t)__text_end)
#define KERNEL_DATA_SIZE (((uint64_t)pmTop() - (uint64_t)PAGE_TABLE_SIZE) - (uint64_t)__text_end) /* 内核数据段大小，ld脚本确保该值4KB对齐*/
#define KERNEL_DATA_END (KERNEL_DATA_BASE + KERNEL_DATA_SIZE)

/* 页表结构的起始地址*/
#define PAGE_TABLE_BASE ((uint64_t)((uint64_t)pmTop() - (uint64_t)PAGE_TABLE_SIZE)) /* 页表地址范围不能被虚拟地址污染*/
#define PAGE_TABLE_END (PAGE_TABLE_BASE + PAGE_TABLE_SIZE)

/**
 * @brief JaeOS虚拟内存布局
 *
 * +---------------------+  <--高地址MAX_VMA = 0x3FFFFFFFFFFFFF(256G)
 * |       PAGE_SIZE      |
 * +---------------------+  <-- TRAMPOLINE(跳板代码)
 * |       PAGE_SIZE      |
 * +---------------------+  <-- SIGNAL_TRAMPOLINE(信号跳板)
 * |       PAGE_SIZE      |
 * +---------------------+  <-- TRAPFRAME(陷阱帧)
 * |       PAGE_SIZE      |
 * +---------------------+  <-- STACKTOP(内核栈顶)
 * |      TD_KSTACK(0)    | <-- 线程0的内核栈(内核态使用,用户态不可访问)
 * |----------------------|
 * |       PAGE_SIZE      |     (不可访问)
 * |----------------------|
 * |      TD_KSTACK(1)    | <-- 线程1的内核栈(内核态使用，用户态不可访问)
 * |----------------------|
 * |       PAGE_SIZE      |     (不可访问)
 * |----------------------|
 * |      TD_KSTACK(x)    |
 * |----------------------|
 * |       PAGE_SIZE      |
 * |----------------------|
 * +---------------------+  <-- USTACKTOP(用户栈顶) = STACKTOP
 * |         32KB         |     (用户态初始栈)
 * +---------------------+  <-- TD_USTACK_INIT_BOTTOM
 * |----------------------|
 * |         256KB        |     (用户栈扩展空间)
 * +---------------------+  <-- TD_USTACK_BOTTOM
 */

#define MAX_VMA ((1ul << (9 + 9 + 9 + 12 - 1)) - 1) /* 256G 0x0 ~ 0x3FFFFFFFFFFFFF*/
#define MIN_USER_VMA 0x10000                         /* 用户空间的最低可用地址*/

#define TRAMPOLINE_VMA (MAX_VMA + 1 - PAGE_SIZE)           /* 跳板代码起始地址*/
#define SIGNAL_TRAMPOLINE_VMA (TRAMPOLINE_VMA - PAGE_SIZE) /* 信号跳板起始地址*/
#define TRAPFRAME_VMA (SIGNAL_TRAMPOLINE_VMA - PAGE_SIZE)  /* 陷阱帧起始地址*/
#define STACKTOP_VMA (TRAPFRAME_VMA - PAGE_SIZE)           /* 内核栈顶*/
#define USTACKTOP_VMA STACKTOP_VMA                         /* 用户栈顶*/

/* 线程内核栈*/
#define TD_KSTACK_PAGE_NUM (2)                                                       /* 每个线程内核栈占用页数*/
#define TD_KSTACK_SIZE (TD_KSTACK_PAGE_NUM * PAGE_SIZE)                              /* 每个线程内核栈大小*/
#define TD_KSTACK_VMA(p) (STACKTOP_VMA - (((p) + 1) * (TD_KSTACK_SIZE + PAGE_SIZE))) /* 每个线程内核栈的起始虚拟地址，P表示线程编号*/

/* 线程用户栈*/
#define TD_USTACK_PAGE_NUM (72)                                                  /* 用户栈占用的总页数*/
#define TD_USTACK_SIZE (TD_USTACK_PAGE_NUM * PAGE_SIZE)                          /* 用户栈占用的总大小*/
#define TD_USTACK_INIT_PAGE_NUM (8)                                              /* 用户栈初始空间页数*/
#define TD_USTACK_INIT_SIZE (TD_USTACK_INIT_PAGE_NUM * PAGE_SIZE)                /* 用户栈初始空间大小*/
#define TD_USTACK_INIT_BOTTOM_VMA (USTACKTOP_VMA - TD_USTACK_INIT_SIZE)          /* 用户栈初始空间底部*/
#define TD_USTACK_EXTEND_PAGE_NUM (TD_USTACK_PAGE_NUM - TD_USTACK_INIT_PAGE_NUM) /* 用户栈扩展空间页数*/
#define TD_USTACK_EXTEND_SIZE (TD_USTACK_EXTEND_PAGE_NUM * PAGE_SIZE)            /* 用户栈扩展空间大小*/
#define TD_USTACK_BOTTOM_VMA (TD_USTACK_INIT_BOTTOM_VMA - TD_USTACK_EXTEND_SIZE) /* 用户栈扩展空间底部*/

#endif /* __MMU_MMU__H__*/