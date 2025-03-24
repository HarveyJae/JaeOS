#include "common/types.h"
#include "common/platform.h"
#include "mmu/mmu.h"
#include "mmu/vmm.h"
#include "mmu/pmm.h"
#include "sbi/sbi.h"
#include "lib/printf.h"
#include "common/rv64.h"
/**
 * @brief 内核虚拟地址空间的三级页表(根页表 level 2)第一个pte(只有一个pte)指向的物理页的PPN
 *
 */
uint64_t kernel_root_pte_pa;
/* .text作为内核代码段*/
extern char __text_end[]; /* .ld文件中定义的.text段结束地址*/
/**
 * @brief 刷新所有核心的TLB，只许成功，不许失败
 *
 * @param va
 */
void tlb_flush(uint64_t va)
{
    /* 获取va所在页的起始地址*/
    va = ADDRALIGNDOWN(va, PAGE_SIZE);
    /* 调用SBI，所有核心刷新TLB*/
    SBI_RET ret = sbi_rfence_fence_vma((1ull << NCPU) - 1, 0, va, PAGE_SIZE);
    if (ret.error)
    {
        while (1)
            ;
    }
}
/**
 * @brief 当pte有效且指向合法的物理页时，减少对物理页面的引用计数
 *        取消映射或页面换出时调用该函数释放pte对物理页面的引用
 *
 * @param pte
 */
static inline void unmap_pte2page(pte_t pte)
{
    if ((pte & PTE_V) && Pte2Pa(pte) >= pm_start)
    {
        page_ref_dec(Pte2Page(pte));
    }
}
/**
 * @brief 当pte有效且指向合法的物理页时，增加对物理页面的引用计数
 *        页面映射或页面换入时调用该函数增加pte对物理页面的引用
 *
 * @param pte
 */
static inline void map_pte2page(pte_t pte)
{
    if ((pte & PTE_V) && Pte2Pa(pte) >= pm_start)
    {
        page_ref_inc(Pte2Page(pte));
    }
}
/**
 * @brief 修改pte中的内容
 *
 * @param pte
 * @param value 必须构成一个完整的pte
 */
static void pte_modify(pte_t *pte, pte_t value)
{
    /* 获取pte中的物理地址*/
    uint64_t old_pa = Pte2Pa(*pte);
    /* 获取传入值的物理地址*/
    uint64_t new_pa = Pte2Pa(value);

    /* 检测是否需要修改pte中的PPN*/
    if (old_pa == new_pa)
    {
        /* 不需要修改*/
        /* 仅修改权限*/
        *pte = value;
    }
    else
    {
        /* 需要修改*/
        /* 释放old物理页*/
        unmap_pte2page(*pte);
        *pte = value;
        map_pte2page(*pte);
    }
}
/**
 * @brief 遍历虚拟地址va对应的页表，返回最终pte的指针(即va对应的物理页)，如果中间页表不存在且create_flag = true，则自动创建页表
 *        页表项、物理页与缺页异常的关系：
 *          1.页表项pte：页表项负责将虚拟地址映射到物理地址，每个pte包含以下关键信息：
 *                      a.物理页号PPN：指向物理页的起始地址
 *                      b.权限位PERM
 *          2.pte的层级结构：
 *                      a.中间层级的pte：PPN表示下一级页表的物理页的起始地址
 *                      b.最后层级的pte：指向实际数据物理页的起始地址
 *          3.物理页：根据pte的层级结构可知每个pte都指向一个物理页(中间层级物理页或实际数据物理页)
 *          4.完整映射512G的虚拟地址：
 *                      a.level-2:1个中间层级物理页(512个pte，每个pte覆盖1G范围)
 *                      b.level-1:512个中间层级物理页(每个level-2 pte对应一个level-1 pt，其中每个pte覆盖2M范围)
 *                      c.level-0:262144(512 * 512)个中间层级物理页(每个level-1 pte对应一个level-0 pt，其中每个pt覆盖4K范围)
 *                      d.总共需要1 + 512 + 262144个物理页作为页表结构的中间层级物理页
 *                      e.映射范围：(262144 * 512) * 4K = 512G
 *          5.缺页异常：
 *                      a.中间层级pte无效：触发缺页异常，需要操作系统分配中间层级物理页
 *                      b.最终层级pte无效：触发缺页异常，需要操作系统分配实际数据物理页
 *
 * @param page_table_address 顶级页表的起始地址(第一个pte的地址)
 * @param va
 * @param create_flag
 * @return pte_t*
 */
static pte_t *walk_page_table(uint64_t page_table_address, uint64_t va, uint64_t create_flag)
{
    /* 获取顶级页表(页表的第一个pte)*/
    /* 同时，根页表只有一个页表项*/
    pte_t *current_pt = (pte_t *)page_table_address;

    /* 遍历三级页表(level 2 -> level 1 -> level 0)*/
    for (int8_t i = PT_LEVELS - 1; i >= 0; i--)
    {
        /* 获取pte*/
        pte_t *current_pte = current_pt + get_pte_index(va, i);
        /* 检测当前页表项是否存在*/
        if (*current_pte & PTE_V)
        {
            /* 存在*/
            /* 处理下一级页表(页表项中存储下一级页表的物理地址)*/
            current_pt = (pte_t *)Pte2Pa(*current_pte);
        }
        else
        {
            /* 不存在*/
            /* 创建下一级页表*/
            if (create_flag)
            {
                /* 分配中间层级物理页*/
                Page *new_page = alloc_page();
                /* 将新的中间层级物理页地址写入当前页表项*/
                pte_modify(current_pte, Page2Pte(new_page) | PTE_V);
                /* 刷新tlb*/
                tlb_flush(va);
                /* 将新页表的物理赋值给current_pt*/
                current_pt = (pte_t *)Page2Pa(new_page);
            }
            else
            {
                /* 页表项不存在时必须创建*/
                while (1)
                    ;
            }
        }
    }

    /* 返回最后一级页表项*/
    return (pte_t *)(current_pt + get_pte_index(va, PT_LEVEL_0));
}
/**
 * @brief 建立物理地址到虚拟地址的映射
 *
 * @param pa 物理地址(满足对齐规则)
 * @param va 虚拟地址(满足对齐规则)
 * @param len 映射范围
 * @param perm 权限
 */
static void map_pa2va(uint64_t pa, uint64_t va, uint64_t len, uint64_t perm)
{
    /* 按页遍历内存区域*/
    for (uint64_t i = 0; i < len; i += PAGE_SIZE)
    {
        pte_t *temp_pte = walk_page_table(kernel_root_pte_pa, va + i, true);
        /* 映射*/
        *temp_pte = (Pa2Pte(pa + i) | perm | PTE_V);
    }
    /* 当前不支持大页映射*/
}
/**
 * @brief 开启虚拟内存管理
 * 
 */
void vm_enable(void)
{
    uint64_t satp_mode = 8ul << SATP_MODE_SHIFT;
    uint64_t satp_ppn = (kernel_root_pte_pa >> PAGE_SHIFT) & PTE_PPN_MASK;

    /* 写satp寄存器*/
    write_satp(satp_mode | satp_ppn);

    /* 刷新TLB(必须的操作，否则会导致旧的TLB缓存未清空，地址翻译错误)*/
    asm volatile("sfence.vma zero, zero");
}
/**
 * @brief
 *
 */
void vmm_init(void)
{
    /* 获取内核虚拟地址空间的根页表*/
    /* 根页表只有一个页表项，这里直接获取跟页表项的PPN(物理地址)*/
    /* 根页表pte指向的物理页必须是4KB对齐，在初始化pmm时，已经确保了所有物理页地址都是4KB对齐*/
    kernel_root_pte_pa = Page2Pa(alloc_page());
    early_printf("[JaeOS]kernel_root_pt:%016lX\n", kernel_root_pte_pa);

    /* MMIO，内存映射I/O*/
    /* 硬件MMIO：将硬件设备的寄存器或内存映射到物理地址空间中，使得CPU可以直接通过内存读写指令访问硬件设备，这一步主要由硬件厂商负责*/
    /* 软件MMIO：建立虚拟地址到设备物理地址的映射，当CPU的satp寄存器被启用后，所有的内存访问(包括内核)都必须经过页表的转换*/
    /* 采用直接映射的方式，即物理地址映射到相同的虚拟地址，便于驱动访问*/

    /* 映射UART(确保地址4KB对齐)*/
    map_pa2va(UART0_BASE, UART0_BASE, PAGE_SIZE * 1, PTE_R | PTE_W);
    early_printf("[JaeOS]UART0 Map Successful.\n");

    /* 映射VirtIO_0(1页大小)*/
    map_pa2va(VIRTIO_0_BASE, VIRTIO_0_BASE, PAGE_SIZE * 1, PTE_R | PTE_W);
    early_printf("[JaeOS]VIRTIO_0 Map Successful.\n");

    /* 映射RTC(1页大小)*/
    map_pa2va(RTC_BASE, RTC_BASE, PAGE_SIZE * 1, PTE_R | PTE_W);
    early_printf("[JaeOS]RTC Map Successful.\n");

    /* 映射中断控制器PLIC(4MB)*/
    map_pa2va(PLIC_BASE, PLIC_BASE, PAGE_SIZE * 1024, PTE_R | PTE_W);
    early_printf("[JaeOS]PLIC Map Successful.\n");

    /* 内核代码段*/
    map_pa2va(KERNEL_TEXT_BASE, KERNEL_TEXT_BASE, KERNEL_TEXT_SIZE, PTE_R | PTE_X);
    early_printf("[JaeOS]KERNEL_TEXT Map Successful.\n");

    /* 内核数据段*/
    map_pa2va(KERNEL_DATA_BASE, KERNEL_DATA_BASE, KERNEL_DATA_SIZE, PTE_R | PTE_W);
    early_printf("[JaeOS]KERNEL_DATA Map Successful.\n");
}