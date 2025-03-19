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

/* data*/
extern uint64_t pm_start;
extern uint64_t page_num;
extern Page *pages;
/* 接口函数*/
/**
 * @brief 获取物理页的物理页号(索引值)
 * 
 */
static inline uint64_t __attribute__((warn_unused_result)) Page2Ppn(Page *p)
{
    return p - pages;
}
/**
 * @brief 获取物理页的起始物理地址
 * 
 */
static inline uint64_t __attribute__((warn_unused_result)) Page2Pa(Page *p)
{
    return pm_start + (Page2Ppn(p) * PAGE_SIZE);
}
/**
 * @brief 获取物理地址对应的物理页
 * 
 */
static inline Page *__attribute__((warn_unused_result)) Pa2Page(uint64_t pa)
{
    return &pages[(pa - pm_start) / PAGE_SIZE];
}
/**
 * @brief 获取物理内存的顶部地址
 * 
 */
static inline uint64_t __attribute__((warn_unused_result)) pmTop()
{
    return pm_start + (page_num * PAGE_SIZE);
}

/* functions*/
void pmmInit(void);
#endif /* !__MMU_PMM__H__*/