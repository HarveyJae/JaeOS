#include "common/types.h"
#include "dev/dtb.h"
#include "mmu/pmm.h"
#include "mmu/mmu.h"
#include "lib/printf.h"

uint64_t pageleft = 0;
uint64_t npage = 0;
Page *pages = NULL;
PageList pageFreeList;

extern char end[]; /* .ld文件中定义的堆起始地址(JaeOS不区分堆栈)*/
/**
 * @brief 物理内存初始化
 * 
 */
void pmmInit(void)
{
    /* 初始化内存页数组*/
    early_printf("[JaeOS]Physical Memory Init Start: 0x%0lX\n", end);
    u64 freemem = PGROUNDUP((u64)end); // 空闲内存页的起始地址
    npage = memInfo.size / PAGE_SIZE;  // 内存页数

    // 内存管理模块的数组
    pages = pmInitPush(freemem, npage * sizeof(Page), &freemem); // 初始化内存页数组

    // 进程管理模块的数组
    extern proc_t *procs;
    procs = pmInitPush(freemem, NPROC * sizeof(proc_t), &freemem);
    extern thread_t *threads;
    threads = pmInitPush(freemem, NTHREAD * sizeof(thread_t), &freemem);
    extern void *sigactions;
    sigactions = pmInitPush(freemem, NPROCSIGNALS * NPROC * sizeof(sigaction_t), &freemem);
    extern void *sigevents;
    sigevents = pmInitPush(freemem, NSIGEVENTS * sizeof(sigevent_t), &freemem);

    // 为 VirtIO 驱动分配连续的两页
    extern void *virtioDriverBuffer;
    virtioDriverBuffer = pmInitPush(freemem, 2 * PAGE_SIZE, &freemem);

    // 为磁盘缓存分配内存
    extern void *bufferData;
    bufferData = pmInitPush(freemem, BGROUP_NUM * sizeof(BufferDataGroup), &freemem);
    extern void *bufferGroups;
    bufferGroups = pmInitPush(freemem, BGROUP_NUM * sizeof(BufferGroup), &freemem);
    extern thread_t *threads;
    threads = pmInitPush(freemem, NTHREAD * sizeof(thread_t), &freemem);
    extern proc_t *procs;
    procs = pmInitPush(freemem, NPROC * sizeof(proc_t), &freemem);
    extern void *sigactions;
    sigactions = pmInitPush(freemem, NPROCSIGNALS * NPROC * sizeof(sigaction_t), &freemem);
    extern void *sigevents;
    sigevents = pmInitPush(freemem, NSIGEVENTS * sizeof(sigevent_t), &freemem);
    extern Dirent *dirents;
    dirents = pmInitPush(freemem, MAX_DIRENT * sizeof(Dirent), &freemem);

    // 为内核栈分配内存
    extern void *kstacks;
    kstacks = pmInitPush(freemem, NPROC * TD_KSTACK_PAGE_NUM * PAGE_SIZE, &freemem);

    // 第二部分：初始化空闲链表
    log(MM_GLOBAL, "Physical Memory Freelist Init Start: Freemem = 0x%0lx\n", freemem);
    LIST_INIT(&pageFreeList);
    u64 pageused = (freemem - MEMBASE) >> PAGE_SHIFT; // 已经使用的内存页数
    for (u64 i = 0; i < pageused; i++)
    {
        pages[i].ref = 1;
    }
    log(MM_GLOBAL, "\tTo pages[0:%d) used\n", pageused);
    for (u64 i = pageused; i < npage; i++)
    {
        LIST_INSERT_HEAD(&pageFreeList, &pages[i], link);
    }
    pageleft = npage - pageused;
    log(MM_GLOBAL, "\tFrom pages[%d:%d) free\n", pageused, npage);

    log(MM_GLOBAL, "Physical Memory Init Finished, `pm` Functions Available!\n");
}