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
#define KERNEL_DATA_SIZE ((uint64_t)pmTop() - (uint64_t)__text_end) /* 内核数据段大小，ld脚本确保该值4KB对齐*/
#endif /* __MMU_MMU__H__*/