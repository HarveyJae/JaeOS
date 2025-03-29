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

#endif /* !__LIB_QUEUE__H__*/