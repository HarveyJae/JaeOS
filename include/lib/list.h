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
#define __LIST_HEAD(name, type)             \
    struct name                             \
    {                                       \
        type *lh_first; /* first element */ \
    }
#define LIST_HEAD(type) __LIST_HEAD(, type) /* 结构体匿名即可*/
/**
 * @brief 系统级链表定义(entry成员)
 * @param type 链表中的元素类型
 */
#define LIST_ENTRY(type)                                       \
    struct                                                     \
    {                                                          \
        type *le_next;  /* next element */                     \
        type **le_prev; /* address of previous next element */ \
    }

/**
 * @brief 链表初始化
 * @param head 链表头的地址
 */
#define LIST_INIT(head)          \
    do                           \
    {                            \
        (head)->lh_first = NULL; \
    } while (0)

/**
 * @brief 头插法插入链表成员
 * @param head 链表头地址
 * @param elm 插入成员地址
 * @param field 插入链接
 */
#define LIST_INSERT_HEAD(head, elm, field)                           \
    do                                                               \
    {                                                                \
        if (((elm)->field.le_next = (head)->lh_first) != NULL)       \
            (head)->lh_first->field.le_prev = &(elm)->field.le_next; \
        (head)->lh_first = (elm);                                    \
        (elm)->field.le_prev = &(head)->lh_first;                    \
    } while (/*CONSTCOND*/ 0)

#endif /* !__LIB_LIST__H__*/