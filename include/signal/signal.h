#ifndef __SIGNAL_SIGNAL__H__
#define __SIGNAL_SIGNAL__H__

#include "common/types.h"
#include "trap/trap.h"
#include "lib/queue.h"

#define MAX_SIGNAL_NUM 128   /* 支持的最大信号数量*/
#define MAX_SIGEVENT_NUM 256 /* 支持的最大信号事件数*/

typedef union
{
    int32_t si_int;
    void *si_ptr;
} sigval_t;

/**
 * @brief 信号集合
 *
 */
typedef struct
{
    /* +7表示向上取整，避免信号位数截断*/
    uint8_t ss_byte[(MAX_SIGNAL_NUM + 7) / 8]; /* 字节数组，每个位表示一个信号*/
} sigset_t;
/**
 * @brief 信号结构体(Linux2.6兼容:gibc)
 *
 */
typedef struct
{
    int32_t si_signo;     /* 信号编号*/
    int32_t si_errno;     /* 信号关联错误码*/
    int32_t si_code;      /* 信号来源代码*/
    int32_t si_trapno;    /* 引发硬件生成信号的陷阱编号(在大多数处理器架构中未使用)*/
    pid_t si_pid;         /* 发送信号的进程pid*/
    uid_t si_uid;         /* 发送进程的真实用户id*/
    int32_t si_status;    /* 子进程退出状态或信号值*/
    int64_t si_utime;     /* 进程消耗的用户时间*/
    int64_t si_stime;     /* 进程消耗的CPU时间*/
    sigval_t si_value;    /* 信号附加数据*/
    int32_t si_overrun;   /* 定时器超限次数，表示定时器到期后因队列满未能及时处理的次数*/
    int32_t si_timerid;   /* 内核内部定时器ID*/
    void *si_addr;        /* 引发故障的内存地址*/
    int64_t si_band;      /* I/O事件类型的位掩码*/
    int32_t si_fd;        /* 信号关联的文件描述符*/
    int16_t si_addr_lsb;  /* 地址的最低有效位*/
    void *si_lower;       /* 地址越界的下界*/
    void *si_upper;       /* 地址越界的上界*/
    int si_pkey;          /* 触发页错误的保护密钥*/
    void *si_call_addr;   /* 系统调用指令的地址*/
    int si_syscall;       /* 被拦截的系统调用编号*/
    unsigned int si_arch; /* 系统调用的 CPU 架构*/
} siginfo_t;

/**
 * @brief 事件信号处理
 *
 */
typedef struct sigevent
{
    trapframe_t se_restoretf;    /* 用户态上下文*/
    sigset_t se_restoremask;     /* 处理信号前的信号集合*/
    int32_t se_signo;            /* 触发的信号编号*/
    int32_t se_status;           /* 信号附加状态信息*/
    uint64_t se_usiginfo;        /* 用户态信号结构体*/
    uint64_t se_uuctx;           /* 用户态上下文*/
    TAILQ_ENTRY(struct sigevent) /* 拼接注释*/
    se_link;                     /* 内核管理的信号事件队列成员*/
} sigevent_t;

/**
 * @brief 信号队列类型定义
 *
 */
typedef struct
{
    TAILQ_HEAD(sigevent_t) /* 拼接注释*/
    tq_head;               /* 信号事件队列*/
} sigeventq_t;

/**
 * @brief 引用自musl的k_sigaction结构体
 *        声明信号动作
 *
 */
typedef struct
{
    void (*sa_handler)(int);
    uint64_t sa_flags;
    void (*sa_restorer)(void);
    sigset_t sa_mask;
} sigaction_t;

/**
 * @brief 信号动作集合(每个线程对应一组动作集合)
 * 
 */
typedef struct
{
    sigaction_t handler[MAX_SIGNAL_NUM];
} sighandler_set_t;


/* data*/
extern sigevent_t *sigevents;
extern sighandler_set_t *sigactions;
/* functions*/
void signal_init(void);
#endif /* !__SIGNAL_SIGNAL__H__*/