#ifndef __MMU_PMM__H__
#define __MMU_PMM__H__
#include "common/types.h"
#include "mmu/mmu.h"
/**
 * @brief 通用节点结构
 *
 */
typedef struct Page
{
    uint32_t ref;
    struct Page *next;
    struct Page *prev;
} Page;
typedef Page PageList;

/* functions*/
void pmm_init(void);
Page *alloc_pt_page(void);
Page *alloc_k_page(void);
uint64_t alloc_km(void);
void free_page(Page *page);
void free_km(uint64_t pa);
void page_ref_inc(Page *_page);
void page_ref_dec(Page *_page);
/* data*/
extern uint64_t pm_start; /* 内存页起始物理地址*/
extern int64_t page_num;  /* 内存页数量*/
extern Page *pages;       /* 内存页数组*/
extern void *kstacks;     /* 内核栈基地址*/

/* 接口函数*/
/**
 * @brief 获取物理页的物理页号
 *
 */
static inline uint64_t __attribute__((warn_unused_result)) Page2Ppn(Page *p)
{
    /* 注意：需要加上基地址*/
    return (p - pages) + (pm_start >> PAGE_SHIFT);
}
/**
 * @brief 获取物理页的起始物理地址
 *
 */
static inline uint64_t __attribute__((warn_unused_result)) Page2Pa(Page *p)
{
    return Page2Ppn(p) << PAGE_SHIFT;
}
/**
 * @brief 获取物理地址对应的物理页
 *
 */
static inline Page *__attribute__((warn_unused_result)) Pa2Page(uint64_t pa)
{
    /* pa必须4KB对齐：定位到pa所在的物理页地址*/
    /* 获取pa所在物理页*/
    return &pages[((uint64_t)ADDRALIGNDOWN(pa, PAGE_SIZE) - pm_start) / PAGE_SIZE];
}
/**
 * @brief 获取物理内存的顶部地址
 *
 */
static inline uint64_t __attribute__((warn_unused_result)) pmTop()
{
    return pm_start + (page_num * PAGE_SIZE);
}

#endif /* !__MMU_PMM__H__*/