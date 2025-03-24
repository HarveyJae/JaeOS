#ifndef __MMU_VMM__H__
#define __MMU_VMM__H__
#include "common/types.h"
#include "mmu/mmu.h"
#include "mmu/pmm.h"
/**
 * @brief 基于SV39的页表结构
 *        SV39模式规定虚拟地址为39位，物理地址为56位，页表深度为3级
 *        支持512GB的虚拟地址空间和64PB的物理地址空间
 *        <虚拟地址>：|63------39|38------30|29------21|20------12|11-------0|
 *                   |-Reversed-|--VPN[2]--|--VPN[1]--|--VPN[0]--|--Offset--|
 *                   |---25bit--|---9bit---|---9bit---|---9bit---|---12bit--|
 *                   VPN[2]，用于索引三级页表的页表项(pte)
 *                   VPN[1]，用于索引二级页表的页表项(pte)
 *                   VPN[0]，用于索引一级页表的页表项(pte)
 *                   offset，页内偏移，4KB空间的页内偏移
 *                   Reserved，保留地址，必须和va[38]相同，否则会触发page fault
 *        <物理地址>：|63------54|53------28|27------19|18------10|9-8|7|6|5|4|3|2|1|0|
 *                   |-Reversed-|--PPN[2]--|--PPN[1]--|--PPN[0]--|RSW|D|A|G|U|X|W|R|V|
 *                   PPN[2]，物理页号，对应于三级页表的pte
 *                   PPN[1]，物理页号，对应于二级页表的pte
 *                   PPN[0]，物理页号，对应于一级页表的pte
 *                   offset，页内偏移，和虚拟地址一致
 *                   Reserved，保留地址，0
 *                   RSW，保留给操作系统使用
 *                   D，脏位，标记该页是否被修改过
 *                   A，访问位，标记该页是否被访问过
 *                   G，全局映射。
 *                   U，U-Mode可访问
 *                   X，可执行
 *                   W，可写
 *                   R，可读
 *                   V，有效位，1表示该pte有效
 *        虚拟地址到物理地址的转换：
 *                              1.获取根页表基址：从satp寄存器获取根页表(三级页表)的物理地址
 *                              2.用虚拟地址的VPN[2]作为索引，找到三级页表(level 2)中的pte，检查pte的V/R/W/X权限位是否合法
 *                                若pte指向大页(1G)，直接合成物理地址
 *                              3.用虚拟地址的VPN[1]作为索引，找到二级页表(level 1)中的pte，检查pte的V/R/W/X权限位是否合法
 *                                若pte指向大页(2M)，直接合成物理地址
 *                              4.用虚拟地址的VPN[0]作为索引，找到一级页表(level 0)中的pte，检查pte的V/R/W/X权限位是否合法
 *                                作为最终的物理地址所在页(4KB)
 *                              5.将上述一级页表中的pte中的PPN[2:0]与offset合并，作为最终的物理地址
 */

/* pte权限位*/
#define PTE_V (1 << 0) /* 有效位(Valid)*/
#define PTE_R (1 << 1) /* 可读位(Readable)*/
#define PTE_W (1 << 2) /* 可写位(Writable)*/
#define PTE_X (1 << 3) /* 可执行位(Executable)*/
#define PTE_U (1 << 4) /* 用户位(U-Mode)*/
#define PTE_G (1 << 5) /* 全局位(Global)*/
#define PTE_A (1 << 6) /* 访问位(Accessed)*/
#define PTE_D (1 << 7) /* 脏位(Dirty)*/
/* 用户自定义权限位*/
#define PTE_COW (1 << 8)    /* 写时复制位*/
#define PTE_SHARED (1 << 9) /* 共享位*/

/* pte转换相关宏*/
#define PTE_PPN_SHIFT (10ull)                                    /* pte中物理页号的偏移量*/
#define PTE_PPN0_SHIFT (10ull)                                   /* PPN[0]的偏移量*/
#define PTE_PPN1_SHIFT (19ull)                                   /* PPN[1]的偏移量*/
#define PTE_PPN2_SHIFT (28ull)                                   /* PPN[2]的偏移量*/
#define PTE_PPN0_LEN (9)                                         /* PPN[0]的位数*/
#define PTE_PPN1_LEN (9)                                         /* PPN[1]的位数*/
#define PTE_PPN2_LEN (26)                                        /* PPN[2]的位数*/
#define PTE_PPN_LEN (PTE_PPN0_LEN + PTE_PPN1_LEN + PTE_PPN2_LEN) /* PPN的位数*/
#define PTE_PPN0_MASK ((1ull << PTE_PPN0_LEN) - 1)               /* PPN[0]的掩码*/
#define PTE_PPN1_MASK ((1ull << PTE_PPN1_LEN) - 1)               /* PPN[0]的掩码*/
#define PTE_PPN2_MASK ((1ull << PTE_PPN2_LEN) - 1)               /* PPN[0]的掩码*/
#define PTE_PPN_MASK ((1ull << PTE_PPN_LEN) - 1)                 /* PPN的掩码*/
#define PTE_PERM_LEN (10)                                        /* PERM的位数*/
#define PTE_PERM_MASK ((1ull << PTE_PERM_LEN) - 1)               /* PERM的掩码*/
#define PT_LEVELS (3ull)                                         /* 页表级数*/
#define PT_LEVEL_2 (2)                                           /* 三级页表level*/
#define PT_LEVEL_1 (1)                                           /* 二级页表level*/
#define PT_LEVEL_0 (0)                                           /* 一级页表level*/
#define PT_INDEX_LEN (9ull)                                      /* 页表项索引长度(VPN字段长度)*/
#define PT_INDEX_MAX (1ull << PT_INDEX_LEN)                      /* 索引最大值512，实际索引是0 - 511*/
#define PT_INDEX_MASK (PT_INDEX_MAX - 1)                         /* VPN[x]掩码*/

/**
 * @brief satp寄存器格式：|63------60|59------51|50------44|43------0|
 *                       |---MODE---|-Reserved-|---ASID---|---PPN---|
 *                       |---4bit---|---9bit---|---7bit---|--44bit--|
 *                      1.MODE：0 = BARE  1 = SV32  8 = SV39
 *                      2.ASID：地址空间标识符
 *                      3.PPN：根页表中页表项(只有一项)的PPN(必须4KB对齐)
 *
 */

/* 配置MODE字段*/
#define SATP_MODE_SHIFT (60)

/**
 * @brief 获取VPN[level]
 * @param va 虚拟地址
 * @param level 页表级数(0 ~ 2)
 */
static inline uint64_t __attribute__((warn_unused_result)) get_pte_index(uint64_t va, uint64_t level)
{
    uint64_t shift = level * PT_INDEX_LEN + PAGE_SHIFT;
    return (va >> shift) & PT_INDEX_MASK;
}
/**
 * @brief 获取pte中的权限位(0 ~ 9)
 * @param pte 对应页表中的pte
 */
static inline uint64_t __attribute__((warn_unused_result)) get_pte_permissions(uint64_t pte)
{
    // return pte & 0x3FF
    return pte & ((1ull << PTE_PPN_SHIFT) - 1);
}
/**
 * @brief 将物理地址转换为pte(去掉offset域，增加perm域)
 *
 * @param pa
 * @return pte_t
 */
static inline pte_t __attribute__((warn_unused_result)) Pa2Pte(uint64_t pa)
{
    uint64_t _ppn = pa >> PAGE_SHIFT;
    uint64_t _ppn_0 = _ppn & PTE_PPN0_MASK;
    uint64_t _ppn_1 = ((_ppn >> PTE_PPN0_LEN) & PTE_PPN1_MASK) << PTE_PPN0_LEN;
    uint64_t _ppn_2 = ((_ppn >> (PTE_PPN0_LEN + PTE_PPN1_LEN)) & PTE_PPN2_MASK) << (PTE_PPN0_LEN + PTE_PPN1_LEN);
    return (_ppn_0 | _ppn_1 | _ppn_2) << PTE_PPN_SHIFT;
}
/**
 * @brief 将pte转换为物理地址(取出PPN域，扩展Offset域)
 * @param pte
 */
static inline uint64_t __attribute__((warn_unused_result)) Pte2Pa(pte_t pte)
{
    uint64_t _ppn_0 = (pte >> PTE_PPN0_SHIFT) & PTE_PPN0_MASK;
    uint64_t _ppn_1 = ((pte >> PTE_PPN1_SHIFT) & PTE_PPN1_MASK) << PTE_PPN0_LEN;
    uint64_t _ppn_2 = ((pte >> PTE_PPN2_SHIFT) & PTE_PPN2_MASK) << (PTE_PPN0_LEN + PTE_PPN1_LEN);
    return (_ppn_0 | _ppn_1 | _ppn_2) << PAGE_SHIFT;
}
/**
 * @brief 将page转换成对应的pte，即使得pte中的PPN位置存储page的物理页号PPN
 *        pages[PPN]即可定位到物理页
 *
 * @param _page
 * @return pte_t
 */
static inline pte_t __attribute__((warn_unused_result)) Page2Pte(Page *_page)
{
    uint64_t _pa = Page2Pa(_page);
    return Pa2Pte(_pa);
}
/**
 * @brief 获取pte中PPN表示的物理页
 *
 * @param pte
 * @return Page*
 */
static inline Page *__attribute__((warn_unused_result)) Pte2Page(pte_t pte)
{
    uint64_t _pa = Pte2Pa(pte);
    return Pa2Page(_pa);
}
/* functions*/
void vmm_init(void);
void vm_enable(void);
#endif /* !__MMU_VMM__H__*/