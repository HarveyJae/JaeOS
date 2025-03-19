#include "common/types.h"
#include "dev/dtb.h"
#include "mmu/pmm.h"
#include "mmu/mmu.h"
#include "lib/printf.h"
#include "lib/string.h"

/**
 * @brief 可用物理内存的起始地址
 *
 */
uint64_t pm_start = 0;
/**
 * @brief 物理内存页数量
 *
 */
uint64_t page_num = 0;
/**
 * @brief 已用的内存页数量
 *
 */
uint64_t usedpage_num = 0;
/**
 * @brief 未使用的内存页数量
 *
 */
uint64_t leftpage_num = 0;
/**
 * @brief 内存页数组
 *
 */
Page *pages = NULL;
/**
 * @brief 全局空闲链表(循环链表)
 *
 */
PageList free_list;

extern char end[]; /* .ld文件中定义的堆起始地址(JaeOS不区分堆栈)*/
/**
 * @brief 内存区域初始化
 *
 * @param start_addr
 * @param size
 * @param freemem_addr
 * @param name 区域名称
 * @return void*
 */
static void *pm_init(uint64_t start_addr, uint64_t size, uint64_t *freemem_addr, const char *name)
{
    /* 初始化区域大小页对齐*/
    uint64_t _size = (uint64_t)ADDRALIGN(size, PAGE_SIZE);
    /* 新的空闲内存起始地址*/
    *freemem_addr = start_addr + _size;
    early_printf("[JaeOS]%s Used: <0x%016lX> ~ <0x%016lX>\n", name, start_addr, *freemem_addr);
    return memset((void *)start_addr, 0, _size);
}
/**
 * @brief 空闲链表初始化
 *
 * @param page
 * @param page_num
 */
static void freelist_init()
{
    free_list.next = &free_list;
    free_list.prev = &free_list;
}
/**
 * @brief 向空闲链表中插入节点
 *
 * @param _new
 */
static void freelist_insert(Page *_new)
{
    /* 头插法插入节点*/
    if (_new->ref == 0)
    {
        _new->next = free_list.next;
        _new->prev = &free_list;
        free_list.next->prev = _new;
        free_list.next = _new;
    }
}

/**
 * @brief 分配物理内存页
 *
 * @return Page*
 */
Page *alloc_page(void)
{
    if (free_list.next == &free_list)
    {
        /* 空闲链表中没有节点(没有空闲的页)*/
        return NULL;
    }
    Page *page = free_list.next;
    page->prev->next = page->next;  // 前驱节点指向后继
    page->next->prev = page->prev;  // 后继节点指向前驱
    page->next = page->prev = NULL; // 隔离已分配页
    /* 更新状态*/
    page->ref = 1;
    /* 分配前清空当前页的数据*/
    memset((void *)Page2Pa(page), 0, PAGE_SIZE);
    return page;
}
/**
 * @brief 释放物理内存页
 *
 * @param page
 */
void free_page(Page *page)
{
    /* 头插法插入链表*/
    page->next = free_list.next;
    page->prev = &free_list;
    free_list.next->prev = page;
    free_list.next = page;

    /* 更新状态*/
    page->ref = 0;
}
/**
 * @brief 物理内存管理模块初始化
 *
 */
void pmmInit(void)
{
    early_printf("[JaeOS]Physical Memory Init Start: 0x%0lX\n", end);
    /* 空闲内存页的起始地址(4KB页对齐)*/
    uint64_t freemem_start_addr = (uint64_t)ADDRALIGN((uint64_t)end, PAGE_SIZE);
    pm_start = freemem_start_addr;
    uint64_t freemem_size = mem_info.size - (freemem_start_addr - mem_info.start);
    /* 已考虑OpenSBI占用的内存，包括内存页数组所占页*/
    page_num = freemem_size / PAGE_SIZE;

    /* 初始化内存页数组*/
    pages = pm_init(freemem_start_addr, page_num * sizeof(Page), &freemem_start_addr, "Physical Memory Page Array");

    // 进程管理模块的数组
    // extern proc_t *procs;
    // procs = pmInitPush(freemem, NPROC * sizeof(proc_t), &freemem);
    // extern thread_t *threads;
    // threads = pmInitPush(freemem, NTHREAD * sizeof(thread_t), &freemem);
    // extern void *sigactions;
    // sigactions = pmInitPush(freemem, NPROCSIGNALS * NPROC * sizeof(sigaction_t), &freemem);
    // extern void *sigevents;
    // sigevents = pmInitPush(freemem, NSIGEVENTS * sizeof(sigevent_t), &freemem);

    // 为 VirtIO 驱动分配连续的两页
    // extern void *virtioDriverBuffer;
    // virtioDriverBuffer = pmInitPush(freemem, 2 * PAGE_SIZE, &freemem);

    // 为磁盘缓存分配内存
    // extern void *bufferData;
    // bufferData = pmInitPush(freemem, BGROUP_NUM * sizeof(BufferDataGroup), &freemem);
    // extern void *bufferGroups;
    // bufferGroups = pmInitPush(freemem, BGROUP_NUM * sizeof(BufferGroup), &freemem);
    // extern thread_t *threads;
    // threads = pmInitPush(freemem, NTHREAD * sizeof(thread_t), &freemem);
    // extern proc_t *procs;
    // procs = pmInitPush(freemem, NPROC * sizeof(proc_t), &freemem);
    // extern void *sigactions;
    // sigactions = pmInitPush(freemem, NPROCSIGNALS * NPROC * sizeof(sigaction_t), &freemem);
    // extern void *sigevents;
    // sigevents = pmInitPush(freemem, NSIGEVENTS * sizeof(sigevent_t), &freemem);
    // extern Dirent *dirents;
    // dirents = pmInitPush(freemem, MAX_DIRENT * sizeof(Dirent), &freemem);

    // 为内核栈分配内存
    // extern void *kstacks;
    // kstacks = pmInitPush(freemem, NPROC * TD_KSTACK_PAGE_NUM * PAGE_SIZE, &freemem);

    /* 确保freemem_start_addr 4KB对齐*/
    freemem_start_addr = (uint64_t)ADDRALIGN(freemem_start_addr, PAGE_SIZE);
    /* 已经使用的内存页数量*/
    usedpage_num = (freemem_start_addr - pm_start) / PAGE_SIZE;
    /* 初始化空闲链表*/
    freelist_init();
    for (uint64_t i = 0; i < usedpage_num; i++)
    {
        pages[i].ref = 1;
    }
    early_printf("[JaeOS]Physical Memory Pages[0:%d] used\n", usedpage_num - 1);
    /* 添加空闲页到空闲链表*/
    for (uint64_t i = usedpage_num; i < page_num; i++)
    {
        freelist_insert(&pages[i]);
    }
    /* 未使用的内存页数量*/
    leftpage_num = page_num - usedpage_num;
    early_printf("[JaeOS]Physical Memory Pages[%d:%d] left\n", usedpage_num, page_num - 1);
    early_printf("[JaeOS]Physical Memory Init Finished\n");
}