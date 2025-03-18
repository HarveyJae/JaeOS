#ifndef __MMU_PMM__H__
#define __MMU_PMM__H__
#include "common/types.h"
/**
 * @brief 通用节点结构
 *        prev使用二级指针，存储的是前一个结构体的next指针的地址，可以直接修改next指针，无需遍历链表
 *
 */
typedef struct
{
    List_Head *next;
    List_Head **prev;
} List_Head;
/* 链表初始化*/
#define LIST_HEAD_INIT(name) {&(name), &(name).next}
#define LIST_HEAD(name) List_Head name = LIST_HEAD_INIT(name)
static inline void list_init(List_Head *head)
{
    head->next = head;
    head->prev = &head->next;
}
static inline void list_add(List_Head *_new, List_Head *head)
{
    _new->next = head->next;
    _new->prev = &head->next;
    head->next->prev = &_new->next;
    head->next = _new;
}
static inline void list_delete(List_Head *entry)
{
    *entry->prev = entry->next;
    entry->next->prev = entry->prev;
}
#define my_offsetof(type, member) \
    ((size_t)(&((type *)0)->member))
#define list_entry(ptr, type, member) \
    ((type *)((char *)(ptr) - my_offsetof(type, member)))
typedef struct
{
    uint32_t ref;
    List_Head link;
} Page;

#endif /* !__MMU_PMM__H__*/