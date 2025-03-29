#include "process/thread.h"
#include "process/proc.h"
#include "lock/mutex.h"
#include "lib/list.h"
#include "lib/queue.h"
#include "mmu/mmu.h"
#include "mmu/vmm.h"
threadq_t thread_runq;   /* 运行队列(全局)*/
threadq_t thread_freeq;  /* 空闲队列(全局)*/
threadq_t thread_sleepq; /* 睡眠队列(全局)*/

thread_t *threads;        /* 全局线程数组*/


void *kstacks; /* 内核栈基地址*/

/**
 * @brief 线程初始化
 *
 */
void thread_init(void)
{
    /* 初始化相关锁*/
    mtx_init(&first_thread_lock, "first_thread_lock", 0, MUTEX_TYPE_SPIN);
    mtx_init(&td_tid_lock, "td_tid_lock", false, MUTEX_TYPE_SPIN);
    mtx_init(&wait_lock, "wait_lock", false, MUTEX_TYPE_SPIN);
    mtx_init(&thread_runq.tq_lock, "thread_runq", false, MUTEX_TYPE_SPIN);
    mtx_init(&thread_freeq.tq_lock, "thread_freeq", false, MUTEX_TYPE_SPIN);
    mtx_init(&thread_sleepq.tq_lock, "thread_sleepq", false, MUTEX_TYPE_SPIN);
    TAILQ_INIT(&thread_runq.tq_head);
    TAILQ_INIT(&thread_freeq.tq_head);
    TAILQ_INIT(&thread_sleepq.tq_head);
    for (int i = MAX_THREAD_NUM - 1; i >= 0; i--)
    {
        /* 获取当前线程*/
        thread_t *td = &threads[i];
        /* 插入空闲线程队列:td_freeq字段确保正确插入空闲队列中*/
        TAILQ_INSERT_HEAD(&thread_freeq.tq_head, td, td_freeq);
        /* 初始化线程锁*/
        mtx_init(&td->td_lock, "thread", false, MUTEX_TYPE_SPIN | MUTEX_RECURSE);
        /* 初始化线程内核栈*/
        td->td_kstack = (uintptr_t)kstacks + TD_KSTACK_SIZE * i;
        /* 将内核线程栈映射到内核页表*/
        for (int j = 0; j < TD_KSTACK_PAGE_NUM; j++)
        {
            uintptr_t pa = td->td_kstack + j * PAGE_SIZE;
            uintptr_t va = TD_KSTACK_VMA(i) + j * PAGE_SIZE;
            pt_map(kernel_root_pte_pa, va, pa, PTE_R | PTE_W);
        }
    }
}