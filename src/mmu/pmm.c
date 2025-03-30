#include "common/types.h"
#include "dev/dtb.h"
#include "mmu/pmm.h"
#include "mmu/mmu.h"
#include "lib/printf.h"
#include "lib/string.h"
#include "driver/virtio.h"
#include "process/thread.h"
#include "process/proc.h"
#include "lock/mutex.h"
/**
 * @brief 可用物理内存的起始地址
 *
 */
uint64_t pm_start = 0;
/**
 * @brief 物理内存页数量
 *
 */
int64_t page_num = 0;
/**
 * @brief 已用的内存页数量
 *
 */
int64_t usedpage_num = 0;
/**
 * @brief 未使用的内存页数量
 *
 */
int64_t leftpage_num = 0;
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
/**
 * @brief 内核栈基地址
 *
 */
void *kstacks;

extern char end[]; /* .ld文件中定义的堆栈起始地址(JaeOS不区分堆栈)*/
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
    uint64_t _size = (uint64_t)ADDRALIGNUP(size, PAGE_SIZE);
    /* 新的空闲内存起始地址*/
    *freemem_addr = start_addr + _size;
    early_printf("[JaeOS]%s Used: <0x%016lX> ~ <0x%016lX>\n", name, start_addr, *freemem_addr);
    return memset((void *)start_addr, 0, _size);
}
/**
 * @brief 空闲链表初始化
 *
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
 * @brief 分配页表物理内存页(是否需要加锁)
 *
 * @return Page*
 */
Page *alloc_pt_page(void)
{
    Page *page = free_list.next;
    while (page->next != &free_list)
    {
        /* 若free_list中存在空闲页，但不在这个范围内，也不会导致死循环*/
        /* 循环判断条件在第一次遇到free_list的时候终止*/
        /* 分配*/
        page->prev->next = page->next;  // 前驱节点指向后继
        page->next->prev = page->prev;  // 后继节点指向前驱
        page->next = page->prev = NULL; // 隔离已分配页

        /* 分配前清空当前页的数据*/
        memset((void *)Page2Pa(page), 0, PAGE_SIZE);
        /* 更新剩余页状态*/
        __atomic_fetch_sub(&leftpage_num, 1, __ATOMIC_RELAXED);
        return page;
        /* 更新page*/
        page = page->next;
    }
    /* 没有空闲页可以分配*/
    return NULL;
}
/**
 * @brief 在内核地址空间申请一个物理页，并返回物理页
 *
 * @return Page *
 */
Page *alloc_k_page(void)
{
    Page *page = free_list.next;
    while (page->next != &free_list)
    {
        /* 分配*/
        page->prev->next = page->next;  // 前驱节点指向后继
        page->next->prev = page->prev;  // 后继节点指向前驱
        page->next = page->prev = NULL; // 隔离已分配页

        /* 分配前清空当前页的数据*/
        memset((void *)Page2Pa(page), 0, PAGE_SIZE);
        /* 更新剩余页状态*/
        __atomic_fetch_sub(&leftpage_num, 1, __ATOMIC_RELAXED);
        return page;
        /* 更新page*/
        page = page->next;
    }
    /* 没有空闲页可以分配*/
    return NULL;
}
/**
 * @brief 释放物理内存页(是否需要加锁)
 *
 * @param page 内存页结构体指针
 */
void free_page(Page *page)
{
    /* 头插法插入链表*/
    page->next = free_list.next;
    page->prev = &free_list;
    free_list.next->prev = page;
    free_list.next = page;

    /* 更新剩余页状态*/
    __atomic_fetch_add(&leftpage_num, 1, __ATOMIC_RELAXED);
}
/**
 * @brief 增加页面的引用计数
 *
 * @param _page
 */
void page_ref_inc(Page *_page)
{
    if (_page == NULL)
    {
        while (1)
            ;
    }
    /* 需要确保原子性：原子递增*/
    __atomic_fetch_add(&(_page->ref), 1, __ATOMIC_RELAXED);
}
void page_ref_dec(Page *_page)
{
    if (_page == NULL || _page->ref == 0)
    {
        while (1)
            ;
    }
    /* 需要确保原子性：原子递减*/
    __atomic_fetch_sub(&(_page->ref), 1, __ATOMIC_RELAXED);
    if (_page->ref == 0)
    {
        free_page(_page);
    }
}
/**
 * @brief 在内核申请一个物理页，返回其物理地址
 *        与alloc_k_page()相比，多了增加页面引用
 *
 * @return uint64_t
 */
uint64_t alloc_km(void)
{
    // mtx_lock(&kvmlock);
    Page *p = alloc_k_page();
    page_ref_inc(p); /* 增加页面引用*/
    // mtx_unlock(&kvmlock);
    return Page2Pa(p);
}
/**
 * @brief 释放内核空间的物理页
 *
 * @param pa
 */
void free_km(uint64_t pa)
{
    // mtx_lock(&kvmlock);
    Page *p = Pa2Page(pa);
    page_ref_dec(p); /* 减少页面引用*/
    // mtx_unlock(&kvmlock);
}
/**
 * @brief 物理内存管理模块初始化
 *
 */
void pmm_init(void)
{
    /* 空闲内存页的起始地址(4KB页对齐)*/
    /* 内核结束后的第一个物理页地址，4KB对齐能够确保所有物理页地址都是4KB对齐*/
    uint64_t freemem_start_addr = (uint64_t)ADDRALIGNUP((uint64_t)end, PAGE_SIZE);

    /* 物理内存的起始地址，包括openSBI和内核*/
    pm_start = mem_info.start;

    /* 物理内存页数量(包括openSBI和内核)*/
    page_num = mem_info.size / PAGE_SIZE;

    /* 初始化内存页数组*/
    pages = pm_init(freemem_start_addr, page_num * sizeof(Page), &freemem_start_addr, "Physical Memory Page Array");

    /* 全局线程数组*/
    threads = pm_init(freemem_start_addr, MAX_THREAD_NUM * sizeof(thread_t), &freemem_start_addr, "Global Thread Array");
    /* 全局进程数组*/
    procs = pm_init(freemem_start_addr, MAX_PROC_NUM * sizeof(proc_t), &freemem_start_addr, "Global Proc Array");

    /* 全局mutex数组*/
    mutexs = pm_init(freemem_start_addr, (MAX_PROC_NUM + MAX_THREAD_NUM) * sizeof(mutex_t), &freemem_start_addr, "Global Mutex Array");

    // extern void *sigactions;
    // sigactions = pmInitPush(freemem, NPROCSIGNALS * NPROC * sizeof(sigaction_t), &freemem);
    // extern void *sigevents;
    // sigevents = pmInitPush(freemem, NSIGEVENTS * sizeof(sigevent_t), &freemem);

    /* 初始化virt io*/
    // virtio_buffer = pm_init(freemem_start_addr, 2 * PAGE_SIZE, &freemem_start_addr, "Virt IO Buffer");

    // 为磁盘缓存分配内存
    // extern void *bufferData;
    // bufferData = pmInitPush(freemem, BGROUP_NUM * sizeof(BufferDataGroup), &freemem);
    // extern void *bufferGroups;
    // bufferGroups = pmInitPush(freemem, BGROUP_NUM * sizeof(BufferGroup), &freemem);

    /* 内核栈内存*/
    kstacks = pm_init(freemem_start_addr, MAX_THREAD_NUM * TD_KSTACK_SIZE, &freemem_start_addr, "Kernel Stack");

    /* 确保freemem_start_addr 4KB对齐*/
    freemem_start_addr = (uint64_t)ADDRALIGNUP(freemem_start_addr, PAGE_SIZE);
    /* 已经使用的内存页数量*/
    usedpage_num = (freemem_start_addr - pm_start) / PAGE_SIZE;
    /* 初始化空闲链表*/
    freelist_init();
    for (uint64_t i = 0; i < usedpage_num; i++)
    {
        pages[i].ref = 1;
    }
    printf("[JaeOS]Physical Memory Pages[0:%d] used\n", usedpage_num - 1);
    /* 添加空闲页到空闲链表*/
    for (uint64_t i = usedpage_num; i < page_num; i++)
    {
        /* 头插法使得pmTop()成为第一块空闲页*/
        /* 因此后续的内存分页从高地址向低地址分配*/
        freelist_insert(&pages[i]);
    }
    /* 未使用的内存页数量*/
    leftpage_num = page_num - usedpage_num;
    printf("[JaeOS]Physical Memory Pages[%d:%d] left\n", usedpage_num, page_num - 1);
    printf("[JaeOS]Physical Memory Init Finished\n");
}