#include "process/thread.h"
#include "process/proc.h"
#include "lock/mutex.h"
#include "lib/list.h"
#include "lib/queue.h"
#include "mmu/mmu.h"
#include "mmu/vmm.h"
#include "mmu/pmm.h"

proc_t *procs;            /* 全局进程数组*/
proclist_t proc_freelist; /* 空闲进程链表*/

extern char trampoline[];   /* trampoline.S的全局符号*/
extern char user_sig_ret[]; /* signal_trampoline.S的全局符号*/
/**
 * @brief 进程初始化
 *
 */
void proc_init(void)
{
    /* 初始化相关锁*/
    mtx_init(&pid_lock, "pid_lock", false, MUTEX_TYPE_SPIN);
    mtx_init(&proc_freelist.pl_lock, "proc_freelist", false, MUTEX_TYPE_SPIN);
    LIST_INIT(&proc_freelist.pl_list);
    for (int i = MAX_PROC_NUM - 1; i >= 0; i--)
    {
        proc_t *p = &procs[i];
        /* 插入空闲进程队列*/
        LIST_INSERT_HEAD(&proc_freelist.pl_list, p, p_list);
        /* 初始化进程锁*/
        mtx_init(&p->p_lock, "proc", false, MUTEX_TYPE_SPIN | MUTEX_RECURSE);
        /* 初始化进程的线程队列*/
        TAILQ_INIT(&p->p_threadsq);
        /* 初始化进程的子进程队列*/
        LIST_INIT(&p->p_children);
        /* 初始化进程的父进程*/
        p->p_parent = NULL;
        /* 初始化进程的页表*/
        p->p_pt = NULL;
        /* 初始化进程的上下文*/
        p->p_trapframe = NULL;
        /* 初始化进程的用户栈*/
        p->p_brk = 0;
    }
}
/**
 * @brief 分配并初始化一个新的用户空间页表(用户页表、用户Trapframe)
 *
 * @param p
 */
void proc_upt_init(proc_t *p)
{
    /* 分配页表*/
    pte_t pt = Pa2Pte(alloc_km());
    p->p_pt = &pt;

    /* TRAMPOLINE_VMA是用户与内核共享的空间，因此需要赋以PTE_G 全局位*/
    pt_map(p->p_pt, TRAMPOLINE_VMA, (uint64_t)trampoline, PTE_R | PTE_X | PTE_G);

    /* SIGNAL_TRAMPOLINE_VMA*/
    pt_map(p->p_pt, SIGNAL_TRAMPOLINE_VMA, (uint64_t)user_sig_ret, PTE_R | PTE_X | PTE_U);

    /* 进程的trapframe*/
    uint64_t page_addr = Page2Pa(alloc_k_page());
    p->p_trapframe = (trapframe_t *)page_addr;
    pt_map(p->p_pt, TRAPFRAME_VMA, (uint64_t)p->p_trapframe, PTE_R | PTE_W);
}
/**
 * @brief 分配并初始化一个新的用户空间栈
 *
 * @param p
 * @param inittd
 */
void proc_ustack_init(proc_t *p, thread_t *inittd)
{
    /* 分配用户栈空间*/
    for (int i = 0; i < TD_USTACK_INIT_PAGE_NUM; i++)
    {
        uint64_t pa = Page2Pa(alloc_k_page());
        uint64_t va = TD_USTACK_INIT_BOTTOM_VMA + i * PAGE_SIZE;
        pt_map(p->p_pt, va, pa, PTE_R | PTE_W | PTE_U);
    }
    /* 分配可拓展的用户栈空间*/
    for (int i = 0; i < TD_USTACK_EXTEND_PAGE_NUM; i++)
    {
        uint64_t va = TD_USTACK_BOTTOM_VMA + i * PAGE_SIZE;
        pt_map(p->p_pt, va, 0, PTE_R | PTE_W | PTE_U);
    }
    /* 初始化用户栈空间指针*/
    inittd->td_trapframe.sp = USTACKTOP_VMA;
    p->p_brk = 0;
}