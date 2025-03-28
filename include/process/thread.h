#ifndef __PROCESS_THREAD__H__
#define __PROCESS_THREAD__H__

#include "common/types.h"
#include "lib/list.h"
#include "lib/queue.h"
#include "lock/mutex.h"
#include "process/proc.h"

#define MAX_PATH_LEN (128)
#define MAX_THREAD_NAME_LEN (MAX_PATH_LEN + 1)

/**
 * @brief 线程结构体
 * 
 */
typedef struct thread
{
	mutex_t td_lock;				   /* 线程锁*/
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
	ptr_t td_kstack;				   /* 内核栈所在页的首地址(t_endzero/copy_addr)*/
	sigeventq_t td_sigqueue;		   /* 待处理信号队列*/
} thread_t;

#endif /* !__PROCESS_THREAD__H__*/