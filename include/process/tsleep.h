#ifndef __PROCESS_TSLEEP__H__
#define __PROCESS_TSLEEP__H__

#include "common/types.h"
#include "process/thread.h"

#define MAX_SLEEP_EVENT_NUM 128 /* 线程睡眠事件数量*/

/**
 * @brief 线程睡眠事件结构
 *
 */
typedef struct tsleepevent
{
    thread_t *tse_td;               /* 关联的线程对象*/
    void *tse_waitch;               /* 等待的通道标识(如锁/信号量)*/
    uint64_t tse_wakeus;            /* 预定唤醒时间戳(微秒)*/
    TAILQ_ENTRY(struct tsleepevent) /* 拼接注释*/
    tse_freeq;                      /* 空闲队列链接*/
    TAILQ_ENTRY(struct tsleepevent) /* 拼接注释*/
    tse_usedq;                      /* 使用队列链接*/
} tsevent_t;

/**
 * @brief 线程睡眠事件队列类型
 *
 */
typedef struct
{
    TAILQ_HEAD(tsevent_t) /* 拼接注释*/
    tq_head;              /* 线程睡眠事件队列*/
    mutex_t tq_lock;      /* 互斥锁(是否需要)*/
} tseventq_t;
#endif /* !__PROCESS_TSLEEP__H__*/