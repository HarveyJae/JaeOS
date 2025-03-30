#ifndef __CPU_CPU__H__
#define __CPU_CPU__H__

#include "common/types.h"
#include "process/thread.h"
#include "lock/mutex.h"
typedef struct
{
    thread_t *cpu_running;          /* CPU正在运行的线程*/
    uint64_t mutex_depth;           /* 锁深度*/
    mutex_t *mutexs[MAX_MUTEX_NUM]; /* 互斥锁数组*/
    register_t sstatus;             /* sstatus之前的值(中断状态)*/
    uint8_t cpu_idle;               /* CPU是否空闲(没有进程执行)*/
} cpu_t;

/* data*/
extern cpu_t cpu_this;
#endif /* !__CPU_CPU__H__*/