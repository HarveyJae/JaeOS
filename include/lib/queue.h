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
#endif /* !__LIB_QUEUE__H__*/