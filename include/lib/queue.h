#ifndef __LIB_QUEUE__H__
#define __LIB_QUEUE__H__

/**
 * @brief 尾队列定义(head)
 * @param name 生成的结构体名称
 * @param type 队列中的元素类型
 * @param qual 限定符(const/vloatile/空)
 */
#define __TAILQUEUE_HEAD(name, type, qual)                         \
    struct name                                                    \
    {                                                              \
        qual type *tqh_first;      /* first element */             \
        qual type *qual *tqh_last; /* addr of last next element */ \
    }
#define TAILQ_HEAD(type) __TAILQUEUE_HEAD(, type, ) /* 匿名结构体即可*/

/**
 * @brief 尾队列定义(entry成员)
 * @param type 队列中的元素类型
 */
#define TAILQ_ENTRY(type)                                       \
    struct                                                      \
    {                                                           \
        type *tqe_next;  /* next element */                     \
        type **tqe_prev; /* address of previous next element */ \
    }

/**
 * @brief 初始化队列
 * @param head 队列头地址
 *
 */
#define TAILQ_INIT(head)                       \
    do                                         \
    {                                          \
        (head)->tqh_first = NULL;              \
        (head)->tqh_last = &(head)->tqh_first; \
    } while (0)

/**
 * @brief 头插法插入队列
 * @param head 队列头地址
 * @param elm 插入成员地址
 * @param field 链接字段
 */
#define TAILQ_INSERT_HEAD(head, elm, field)                             \
    do                                                                  \
    {                                                                   \
        if (((elm)->field.tqe_next = (head)->tqh_first) != NULL)        \
            (head)->tqh_first->field.tqe_prev = &(elm)->field.tqe_next; \
        else                                                            \
            (head)->tqh_last = &(elm)->field.tqe_next;                  \
        (head)->tqh_first = (elm);                                      \
        (elm)->field.tqe_prev = &(head)->tqh_first;                     \
    } while (0)

/**
 * @brief 队列为空
 *        当队列为空时，返回true，否则返回false
 * @param head 队列头地址
 *
 */
#define TAILQ_EMPTY(head) ((head)->tqh_first == NULL)

/**
 * @brief 获取队列中的第一个entry
 * @param head 队列头地址
 *
 */
#define TAILQ_FIRST(head) ((head)->tqh_first)

/**
 * @brief 移除队列中的elm对应的entry
 * @param head 队列头地址
 * @param elm 移除成员地址
 * @param field 链接字段
 *
 */
#define TAILQ_REMOVE(head, elm, field)                                     \
    do                                                                     \
    {                                                                      \
        if (((elm)->field.tqe_next) != NULL)                               \
            (elm)->field.tqe_next->field.tqe_prev = (elm)->field.tqe_prev; \
        else                                                               \
            (head)->tqh_last = (elm)->field.tqe_prev;                      \
        *(elm)->field.tqe_prev = (elm)->field.tqe_next;                    \
    } while (0)

/**
 * @brief 将elm成员插入到listelm成员之前
 * @param head 被插入成员地址
 * @param elm 插入成员地址
 * @param field 链接字段
 */
#define TAILQ_INSERT_BEFORE(listelm, elm, field)            \
    do                                                      \
    {                                                       \
        (elm)->field.tqe_prev = (listelm)->field.tqe_prev;  \
        (elm)->field.tqe_next = (listelm);                  \
        *(listelm)->field.tqe_prev = (elm);                 \
        (listelm)->field.tqe_prev = &(elm)->field.tqe_next; \
    } while (0)

/**
 * @brief 将elm成员插入到队尾
 * @param head 队列头地址
 * @param elm 插入成员地址
 * @param field 链接字段
 */
#define TAILQ_INSERT_TAIL(head, elm, field)        \
    do                                             \
    {                                              \
        (elm)->field.tqe_next = NULL;              \
        (elm)->field.tqe_prev = (head)->tqh_last;  \
        *(head)->tqh_last = (elm);                 \
        (head)->tqh_last = &(elm)->field.tqe_next; \
    } while (/*CONSTCOND*/ 0)
#define TAILQ_NEXT(elm, field) ((elm)->field.tqe_next)
#define TAILQ_LAST(head, headname) (*(((struct headname *)((head)->tqh_last))->tqh_last))
#define TAILQ_PREV(elm, headname, field) (*(((struct headname *)((elm)->field.tqe_prev))->tqh_last))
#endif /* !__LIB_QUEUE__H__*/