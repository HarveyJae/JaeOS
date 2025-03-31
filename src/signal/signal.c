#include "common/types.h"
#include "signal/signal.h"
#include "lock/mutex.h"
#include "lib/string.h"
#include "process/thread.h"

sigevent_t *sigevents;      /* 全局信号事件数组*/
sigeventq_t sigevent_freeq; /* 空闲信号事件队列*/

sighandler_set_t *sigactions; /* 全局信号动作数组*/
/**
 * @brief 初始化信号
 *
 */
void signal_init(void)
{
    /* 初始化信号事件锁*/
    mutex_init(&sigevent_lock, "sigevent_lock", MUTEX_TYPE_SPIN | MUTEX_RECURSE);

    for (int i = MAX_SIGEVENT_NUM - 1; i >= 0; i--)
    {
        TAILQ_INSERT_HEAD(&sigevent_freeq.tq_head, &sigevents[i], se_link);
    }
    /* 初始化信号动作数组*/
    memset(sigactions, 0, sizeof(sigaction_t) * MAX_SIGNAL_NUM * MAX_THREAD_NUM);
}