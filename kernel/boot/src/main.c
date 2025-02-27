#include "init.h"

/* 多核启动 */
volatile static uint32_t hart_started[NCPU]; /* 确定哪个核已经被启动 */
volatile static uint32_t hart_first = 1;     /* 首先被启动标志 */
volatile static uint32_t kern_inited = 0;    /* 核启动阻塞标志 */
volatile static hart_start_lock = 0;         /* 自选锁 */
/**
 * __sync_synchronize()之前的所有内存操作都会在__sync_synchronize()之后的内存操作之前完成
 * __sync_synchronize()之前的内存操作对其他核心是可见的(结果可见或结果同步)
 * __sync_synchronize()不会确保多核并行指令之间的顺序性
 *
 */

void main(void)
{
    while (__sync_lock_test_and_set(&hart_start_lock, 1) == 1)
        ;
    /* 原子的将lock设置为1，并返回旧值，如果返回0，则证明是获取锁的核心 */
    /* 如果返回1，证明锁已经被其他线程获取了，此时需自旋等待释放 */
    /* 避免多个核心同时进入代码块 */
    if (hart_first == 1)
    {
        hart_first = 0;
        __sync_synchronize();                  /* 内存修改是多核可见的，也可以不写，因为有锁 */
        __sync_lock_release(&hart_start_lock); /* 释放锁，避免死锁 */
        
    }
    else
    {

    }
}
