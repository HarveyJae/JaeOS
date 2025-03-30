#include "common/types.h"
#include "lock/mutex.h"
#include "common/rv64.h"
#include "cpu/cpu.h"
mutex_t first_thread_lock; /* 保护第一个线程的创建过程*/
mutex_t td_tid_lock;	   /* 保护线程ID的分配与回收*/
mutex_t wait_lock;		   /* 等待锁:保证父进程等待和子进程退出按顺序依次发生*/
mutex_t pid_lock;		   /* 保护进程ID的分配与回收*/
mutex_t pr_lock;		   /* printf输出语句锁*/
mutex_t kvm_lock;		   /* 虚拟内存映射锁*/

mutex_t *mutexs; /* 进程与线程使用的mutex数组(每个进程或线程对应其中一个mutex)*/
/**
 * @brief 进入临界区：关闭中断，获取锁
 *        不支持多核心
 * @param m
 */
static void mutex_enter_critical(mutex_t *m)
{
	/* 关闭中断并保存之前的中断状态*/
	register_t pre_sie = disable_si();
	/* 检查中断是否已经关闭*/
	if (get_si())
	{
		/* 中断依旧开启中*/
		while (1)
			;
	}
	if (cpu_this.mutex_depth == 0)
	{
		cpu_this.sstatus = pre_sie;
	}
	/* 记录锁*/
	cpu_this.mutexs[cpu_this.mutex_depth] = m;
	/* 更新锁深度*/
	cpu_this.mutex_depth++;
}
/**
 * @brief 离开临界区：释放锁，恢复中断
 *
 * @param m
 */
static void mutex_leave_critical(mutex_t *m)
{
	/* 判断中断是否处于关闭状态*/
	if (get_si())
	{
		/* 中断依旧开启中*/
		while (1)
			;
	}
	if (cpu_this.mutex_depth == 0)
	{
		/* mutex error*/
		while (1)
			;
	}
	/* 获取待解锁*/
	if (cpu_this.mutexs[cpu_this.mutex_depth - 1] == m)
	{
		/* 获取成功*/
		cpu_this.mutexs[cpu_this.mutex_depth - 1] = NULL;
	}
	else
	{
		/* 锁没有按顺序释放：error*/
		while (1)
			;
	}
	/* 更新锁的深度*/
	cpu_this.mutex_depth--;
	if (cpu_this.mutex_depth == 0)
	{
		restore_si(cpu_this.sstatus);
	}
}
/**
 * @brief 初始化锁
 *
 * @param m mutex指针
 * @param m_name 避免局部变量
 * @param m_type mutex的底层类型：自旋锁/睡眠锁
 */
void mutex_init(mutex_t *m, char *m_name, uint8_t m_type)
{
	if (m == NULL)
	{
		/* 内存错误*/
		while (1)
			;
	}
	m->mutex_name = (uint8_t *)m_name;
	m->mutex_owner = NULL;
	m->mutex_type = m_type;
	m->mutex_depth = 0;
}
/**
 * @brief 自旋锁加锁(互斥锁mutex)
 *
 * @param mutex
 */
void mutex_lock(mutex_t *m)
{
	/* 底层使用自旋锁*/
	if (m->mutex_type & MUTEX_TYPE_SPIN)
	{
		/* 进入临界区*/
		mutex_enter_critical(m);
		/* 判断当前锁是否支持重入*/
		if (m->mutex_type & MUTEX_RECURSE)
		{
			/* 增加重入深度，不在此退出临界区*/
			m->mutex_depth++;
		}
		else
		{
			/* 不支持重入：判断当前锁是否可以获取*/
			if (m->mutex_depth == 1)
			{
				/* 不能重入，离开临界区*/
				mutex_leave_critical(m);
				while (1)
					;
			}
			/* 获取自旋锁*/
			m->mutex_depth = 1;
		}
	}
	else
	{
		/* 不支持睡眠锁*/
		while (1)
			;
	}
}
/**
 * @brief 自旋锁解锁
 *
 * @param m
 */
void mutex_unlock(mutex_t *m)
{
	if (m->mutex_type & MUTEX_TYPE_SPIN)
	{
		if (m->mutex_depth > 1)
		{
			/* 判断锁是否可重入*/
			if (m->mutex_type & MUTEX_RECURSE)
			{
				/* 减少重入深度*/
				m->mutex_depth--;
				/* 离开临界区*/
				mutex_leave_critical(m);
			}
			else
			{
				/* 不可重入锁恶意重入*/
				while (1)
					;
			}
		}
		else
		{
			/* 非重入*/
			m->mutex_depth = 0;
			/* 离开临界区*/
			mutex_leave_critical(m);
		}
	}
	else
	{
		/* 不支持睡眠锁*/
		while (1)
			;
	}
}