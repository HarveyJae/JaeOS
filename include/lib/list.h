#ifndef __LIB_LIST__H__
#define __LIB_LIST__H__

/*
 * List definitions.
 */

/**
 * @brief  系统级链表定义(head)
 * @param name 结构体名称
 * @param type 链表中的元素类型
 */
#define __LIST_HEAD(name, type)                    \
    struct name                                    \
    {                                              \
        type *lh_first; /* first element */ \
    }
#define LIST_HEAD(type) __LIST_HEAD(, type) /* 结构体匿名即可*/
/**
 * @brief 系统级链表定义(entry成员)
 * @param 
 */
#define LIST_ENTRY(type)                                              \
    struct                                                            \
    {                                                                 \
        type *le_next;  /* next element */                     \
        type **le_prev; /* address of previous next element */ \
    }
#endif /* !__LIB_LIST__H__*/