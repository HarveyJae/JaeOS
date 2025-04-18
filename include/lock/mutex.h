#ifndef __LOCK_MUTEX__H__
#define __LOCK_MUTEX__H__
#include "common/types.h"
#include "process/thread.h"

#define MUTEX_TYPE_SPIN (0x01)	/* mutex底层基于自旋锁实现*/
#define MUTEX_TYPE_SLEEP (0x02) /* mutex底层基于睡眠锁实现*/

#define MUTEX_RECURSE (0x04) /* 锁可重入*/

#define MAX_MUTEX_NUM (128) /* CPU支持的最多mutex数量(可重入深度)*/

/**
 * @brief mutex类型定义
 *
 */
typedef struct mutex
{
	const uint8_t *mutex_name;	/* 用于标识不同的互斥锁mutex实例*/
	uint64_t mutex_locked;		/* 记录互斥锁mutex的状态信息*/
	struct thread *mutex_owner; /* 拥有该互斥锁mutex的线程*/
	void *mutex_data;			/* 互斥锁mutex的实际数据*/
	uint8_t mutex_type;			/* 互斥锁mutex的底层实现类型*/
	uint8_t mutex_depth;		/* 互斥锁mutex的锁深度*/
} mutex_t;

/* functions*/
void mutex_init(mutex_t *m, char *m_name, uint8_t m_type);
void mutex_lock(mutex_t *m);
void mutex_unlock(mutex_t *m);
/* data*/
extern mutex_t wait_lock;
extern mutex_t td_tid_lock;
extern mutex_t first_thread_lock;
extern mutex_t pid_lock;
extern mutex_t pr_lock;
extern mutex_t kvm_lock;
extern mutex_t sigevent_lock;

extern mutex_t *mutexs;
#endif /* !__LOCK_MUTEX__H__*/