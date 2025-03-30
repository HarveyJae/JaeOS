#ifndef __PROCESS_THREAD__H__
#define __PROCESS_THREAD__H__

#include "common/types.h"
#include "lib/list.h"
#include "lib/queue.h"
#include "process/proc.h"
#include "lock/mutex.h"
#include "signal/signal.h"
#define MAX_PATH_LEN (128)
#define MAX_THREAD_NAME_LEN (MAX_PATH_LEN + 1) /* 最大线程名*/
#define MAX_THREAD_NUM (256)				   /* 最大线程数量*/

/**
 * @brief 线程结构体
 *
 */
typedef struct thread
{
	struct mutex *td_lock;			   /* 线程锁(需要分配内存)*/
	uint64_t mutex_index;			   /* mutex在mutexs中的索引*/
	proc_t *td_proc;				   /* 线程所属进程*/
	TAILQ_ENTRY(struct thread)		   /* 拼接注释*/
	td_plist;						   /* 所属进程的线程链表entry*/
	TAILQ_ENTRY(struct thread)		   /* 拼接注释*/
	td_runq;						   /* 运行队列entry*/
	TAILQ_ENTRY(struct thread)		   /* 拼接注释*/
	td_sleepq;						   /* 睡眠队列entry*/
	TAILQ_ENTRY(struct thread)		   /* 拼接注释*/
	td_freeq;						   /* 空闲队列entry*/
	tid_t td_tid;					   /* 线程id*/
	state_t td_status;				   /* 线程状态*/
	char td_name[MAX_THREAD_NAME_LEN]; /* 线程名(清零属性区域开始t_startzero_addr)*/
	uintptr_t td_wchan;				   /* 线程睡眠等待的地址*/
	const char *td_wmesg;			   /* 线程睡眠等待的原因*/
	err_t td_exitcode;				   /* 线程退出码*/
	sigevent_t *td_sig;				   /* 线程当前正在处理的信号*/
	trapframe_t td_trapframe;		   /* 用户态上下文*/
	ktrapframe_t td_kcontext;		   /* 内核态上下文*/
	uint8_t td_killed;				   /* 线程是否被杀死*/
	sigset_t td_cursigmask;			   /* 线程正在处理的信号屏蔽字*/
	uint64_t td_ctid;				   /* 清空tid地址标识*/
	uint64_t td_utstamp;			   /* 用户态线程时间戳*/
	uint64_t td_ststamp;			   /* 内核态线程时间戳*/
	sigset_t td_sigmask;			   /* 线程信号屏蔽字(t_startcopy_addr)*/
	uintptr_t td_kstack;			   /* 内核栈所在页的首地址(t_endzero/copy_addr)*/
	TAILQ_HEAD(struct sigevent)		   /* 拼接注释*/
	td_sigqueue;					   /* 待处理信号队列*/
} thread_t;

/**
 * @brief 线程队列类型(全局锁)
 *
 */
typedef struct
{
	TAILQ_HEAD(thread_t) /* 拼接注释*/
	tq_head;			 /* 线程队列*/
} threadq_t;

/* functions*/
void thread_init(void);
/* data*/
extern thread_t *threads;
#endif /* !__PROCESS_THREAD__H__*/