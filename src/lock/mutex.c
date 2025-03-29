#include "common/types.h"
#include "lock/mutex.h"
#include "common/rv64.h"

mutex_t first_thread_lock; /* 保护第一个线程的创建过程*/
mutex_t td_tid_lock;	   /* 保护线程ID的分配与回收*/
mutex_t wait_lock;		   /* 等待锁:保证父进程等待和子进程退出按顺序依次发生*/
mutex_t pid_lock;          /* 保护进程ID的分配与回收*/

/**
 * @brief 进入临界区：关闭中断(是否需要关闭调度器)
 *        不支持多核心
 * @param mutex
 */
static void mutex_enter_critical(mutex_t *mutex)
{
	/* 关闭中断并保存之前的中断状态*/
	register_t pre_sie = disable_si();
}
/**
 * @brief 加锁(互斥锁mutex)
 *
 * @param mutex
 */
void mutex_lock(mutex_t *mutex)
{
	/* 底层使用自旋锁*/
	if (mutex->mutex_type & MUTEX_TYPE_SPIN)
	{
		/* 进入临界区*/
		lo_critical_enter(m);
		/* 判断锁是否可重入Reentrancy*/
		if (lo_acquired(&m->mtx_lock_object))
		{
			if (m->mtx_type & MTX_RECURSE)
			{
				// 重入，增加重入深度
				m->mtx_depth++;
				mtx_spin_debug("lock[%s] re-entered! (depth:%d)\n",
							   m->mtx_lock_object.lo_name, m->mtx_depth);
				// 离开临界区（自旋互斥量重入不用再套一层临界区）
				lo_critical_leave(m);
			}
			else
			{
				// 不能重入，离开临界区
				lo_critical_leave(m);
				error("mtx_lock: mtx %s(%d) is not re-entrant\n",
					  m->mtx_lock_object.lo_name, m->mtx_type);
			}
		}
		else
		{
			// 非重入，获取自旋锁
			__mtx_lo_lock(m, false);
			m->mtx_depth = 1;
			mtx_spin_debug("lock[%s] acquired!\n", m->mtx_lock_object.lo_name);
		}
	}
	else if (m->mtx_type & MTX_SLEEP)
	{
		// 睡眠互斥量，仅用于 sleep 结束后重新获取
		__mtx_lo_lock(m, true);
	}
	else
	{
		error("mtx_lock: invalid mtx_type %d\n", m->mtx_type);
	}
}