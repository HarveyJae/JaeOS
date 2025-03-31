#ifndef __PROCESS_PROC__H__
#define __PROCESS_PROC__H__

#include "common/types.h"
#include "lib/list.h"
#include "lib/queue.h"
#include "trap/trap.h"
#include "lock/mutex.h"
#define MAX_PROC_NUM (128) /* 最大进程数量*/

/**
 * @brief 进程状态枚举
 *
 */
typedef enum
{
    UNUSED = 0, /* 空闲*/
    USED,       /* 非空闲*/
    SLEEPING,   /* 等待事件(阻塞)*/
    RUNNABLE,   /* 就绪*/
    RUNNING,    /* 正在运行*/
    ZOMBIE      /* 僵尸进程*/
} state_t;
/**
 * @brief 进程运行时间类型
 *
 */
typedef struct
{
    int64_t tms_utime;  /* 用户态运行时间*/
    int64_t tms_stime;  /* 内核态运行时间*/
    int64_t tms_cutime; /* 用户态子进程运行时间*/
    int64_t tms_cstime; /* 内核态子进程运行时间*/
    int64_t tms_ustart; /* 开始时间*/
} times_t;
/**
 * @brief 进程结构体
 *
 */
typedef struct proc
{
    struct mutex *p_lock;     /* 进程锁(需要分配内存)*/
    uint64_t mutex_index;     /* mutex在mutexs中的索引*/
    LIST_ENTRY(struct proc)   /* 拼接注释*/
    p_list;                   /* 进程空闲链表entry*/
    TAILQ_HEAD(struct thread) /* 拼接注释*/
    p_threadsq;               /* 进程的线程队列*/
    state_t p_status;         /* 进程状态*/
    pid_t p_pid;              /* 进程id*/
    uintptr_t p_brk;          /* 进程的堆顶地址*/
    uintptr_t p_pt;           /* 进程页表根地址*/
    trapframe_t *p_trapframe; /* 用户态上下文指针*/
    err_t p_exitcode;         /* 进程退出码*/
    times_t p_times;          /* 进程运行时间(清零起始地址p_startzero_addr)*/
    // thread_fs_t p_fs_struct;  /* 文件系统相关字段*/
    struct proc *p_parent;  /* 父进程(清零结束地址p_endzero_addr)*/
    LIST_HEAD(struct proc)  /* 拼接注释*/
    p_children;             /* 子进程列表*/
    LIST_ENTRY(struct proc) /* 拼接注释*/
    p_sibling;              /* 父进程的子进程列表entry*/
} proc_t;

/**
 * @brief 进程队列类型(全局锁)
 *        基于链表
 */
typedef struct proclist
{
    LIST_HEAD(proc_t) /* 拼接注释*/
    pl_list;          /* 进程队列*/
} proclist_t;

/* functions*/
void proc_init(void);
/* data*/
extern proc_t *procs;
#endif /* !__PROC__H__*/