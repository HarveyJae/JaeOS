#include "common/types.h"
#include "lib/list.h"
#include "lib/queue.h"
#include "lock/mutex.h"
#include "mmu/mmu.h"
#include "mmu/vmm.h"
#include "mmu/pmm.h"
#include "process/thread.h"
#include "process/proc.h"
#include "process/tsleep.h"

tsevent_t tsevents[MAX_SLEEP_EVENT_NUM]; /* 线程睡眠事件数组*/
tseventq_t tsevent_freeq;                /* 空闲队列*/
tseventq_t tsevent_usedq;                /* 使用队列*/

/**
 * @brief 线程睡眠初始化
 *
 */
void tsleep_init(void)
{
    /* 初始化队列锁*/
    mutex_init(&tsevent_freeq.tq_lock, "tse_freeq", MUTEX_TYPE_SPIN);
    mutex_init(&tsevent_usedq.tq_lock, "tse_usedq", MUTEX_TYPE_SPIN);
    /* 初始化队列*/
    TAILQ_INIT(&tsevent_freeq.tq_head);
    TAILQ_INIT(&tsevent_usedq.tq_head);
    for (int i = MAX_SLEEP_EVENT_NUM - 1; i >= 0; i--)
    {
        tsevent_t *tse = &tsevents[i];
        tse->tse_waitch = NULL;
        tse->tse_td = NULL;
        /* 插入空闲队列*/
        TAILQ_INSERT_HEAD(&tsevent_freeq.tq_head, tse, tse_freeq);
    }
}
/**
 * @brief 从空闲池分配一个睡眠事件对象，初始化后插入到使用队列
 *
 * @param td 线程对象
 * @param chan 通道标识
 * @param wakeus 唤醒时间戳
 * @return tsevent_t*
 */
static tsevent_t *tse_alloc(thread_t *td, void *chan, uint64_t wakeus)
{
    /* 空闲队列加锁*/
    mutex_lock(&tsevent_freeq.tq_lock);
    if (TAILQ_EMPTY(&tsevent_freeq.tq_head))
    {
        /* 空闲队列为空，超出系统要求*/
        while (1)
            ;
    }
    /* 分配一个睡眠事件对象*/
    tsevent_t *tse = TAILQ_FIRST(&tsevent_freeq.tq_head);
    /* 从空闲队列中移除*/
    TAILQ_REMOVE(&tsevent_freeq.tq_head, tse, tse_freeq);
    /* 空闲队列解锁*/
    mutex_unlock(&tsevent_freeq.tq_lock);

    /* 初始化睡眠事件对象*/
    tse->tse_td = td;
    tse->tse_waitch = chan;
    tse->tse_wakeus = wakeus;

    /* 使用队列加锁*/
    mutex_lock(&tsevent_usedq.tq_lock);
    /* 将睡眠事件插入到合适的位置*/
    if (wakeus)
    {
        /* 事件会自动唤醒*/
        tsevent_t *temp_tse = NULL;
        /* 从头开始遍历，找到第一个比wakeus大的元素，插入到其前面*/
        for (temp_tse = tsevent_usedq.tq_head.tqh_first; temp_tse != NULL; temp_tse = temp_tse->tse_usedq.tqe_next)
        {
            if (temp_tse->tse_wakeus > wakeus || temp_tse->tse_wakeus == 0)
            {
                break;
            }
        }
        if (temp_tse)
        {
            /* 插入到temp_tse前面*/
            TAILQ_INSERT_BEFORE(temp_tse, tse, tse_usedq);
        }
        else
        {
            /* wakeus是队列中最大的唤醒时间，插入到最后*/
            TAILQ_INSERT_TAIL(&tsevent_usedq.tq_head, tse, tse_usedq);
        }
    }
    else
    {
        /* 事件不会自动唤醒*/
        TAILQ_INSERT_TAIL(&tsevent_usedq.tq_head, tse, tse_usedq);
    }
    return tse;
}

static void tse_set_unused(tsevent_t *tse, bool timeout)
{
    assert(mtx_hold(&tsevent_usedq.tseq_lock));
    assert(tse_debug(tse, 1, 0));
    TAILQ_REMOVE(&tsevent_usedq.tseq_head, tse, tse_usedq);
    assert(tse_debug(tse, 0, 0));
    // warn("%08x(%08x) WAS WAKEUP(%d) at %d before %d\n", tse->tse_td->td_tid, tse->tse_td->td_proc->p_pid, timeout, getUSecs(), tse->tse_wakeus);
    tse->tse_wchan = timeout ? (void *)-1 : NULL;
}

static err_t tse_free(tsevent_t *tse)
{
    err_t r = tse->tse_wchan == NULL ? 0 : -ETIMEDOUT;
    tse->tse_td = NULL;
    tse->tse_wchan = NULL;
    tse->tse_wakeus = 0;

    tseq_critical_enter(&tsevent_freeq);
    assert(tse_debug(tse, 0, 0));
    TAILQ_INSERT_HEAD(&tsevent_freeq.tseq_head, tse, tse_freeq);
    assert(tse_debug(tse, 0, 1));
    tseq_critical_exit(&tsevent_freeq);

    return r;
}

// 接口函数

/**
 * @note sleep 的包裹，会额外注册一个超时唤醒事件
 */
err_t tsleep(void *chan, mutex_t *mtx, const char *msg, u64 wakeus)
{
    tsevent_t *tse = tse_alloc(cpu_this()->cpu_running, chan, wakeus);
    // 已经获取了 tsevent_usedq 的锁，释放 mtx 后再睡眠
    // 睡眠时先获取睡眠队列的锁，然后再释放 tsevent_usedq 的锁
    log(0, "%08x(%08x) TSLEEP(%s) UNTIL %lu->%lu\n", cpu_this()->cpu_running->td_tid, cpu_this()->cpu_running->td_proc->p_pid, msg, time_mono_us(), wakeus);
    if (mtx)
    {
        tseq_critical_exit(&tsevent_usedq);
    }
    sleep(chan, mtx ? mtx : &tsevent_usedq.tseq_lock, msg);
    if (mtx)
    {
        tseq_critical_enter(&tsevent_usedq);
    }
    err_t r = tse_free(tse);
    tseq_critical_exit(&tsevent_usedq);
    return r;
}

void twakeup(void *chan)
{
    tseq_critical_enter(&tsevent_usedq);
    tsevent_t *tse;
    while (1)
    {
        int removed = 0;
        TAILQ_FOREACH(tse, &tsevent_usedq.tseq_head, tse_usedq)
        {
            if (tse->tse_wchan == chan)
            {
                tse_set_unused(tse, false);
                removed = 1;
            }
        }
        if (!removed)
            break;
    }
    wakeup(chan);
    tseq_critical_exit(&tsevent_usedq);
}

void tcleanup(thread_t *td)
{
    tseq_critical_enter(&tsevent_usedq);
    tsevent_t *tse;
    while (1)
    {
        int removed = 0;
        TAILQ_FOREACH(tse, &tsevent_usedq.tseq_head, tse_usedq)
        {
            if (tse->tse_td == td)
            {
                tse_set_unused(tse, false);
                removed = 1;
            }
        }
        if (!removed)
            break;
    }
    tseq_critical_exit(&tsevent_usedq);
}

void tsleep_check()
{
    tseq_critical_enter(&tsevent_usedq);
    tsevent_t *tse;
    u64 now = time_mono_us();
    while ((tse = TAILQ_FIRST(&tsevent_usedq.tseq_head)))
    {
        if (tse->tse_wakeus > now || tse->tse_wakeus == 0)
        {
            break;
        }
        void *chan = tse->tse_wchan;
        tse_set_unused(tse, true);
        wakeup(chan);
    }
    tseq_critical_exit(&tsevent_usedq);
}
