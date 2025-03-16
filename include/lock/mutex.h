#ifndef __LOCK_MUTEX__H__
#define __LOCK_MUTEX__H__
#include "common/types.h"

#define MUTEX_TYPE_SPIN 0x01  /* mutex底层基于自旋锁实现*/
#define MUTEX_TYPE_SLEEP 0x02 /* mutex底层基于睡眠锁实现*/
typedef struct
{
	const uint8_t *mutex_name; /* 用于标识不同的互斥锁mutex实例*/
	uint64_t mutex_locked;	   /* 记录互斥锁mutex的状态信息*/
	void *mutex_data;		   /* 互斥锁mutex的实际数据*/
	uint8_t mutex_type;		   /* 互斥锁mutex的底层实现类型*/
	uint8_t mutex_depth;	   /* 互斥锁mutex的锁深度*/
} mutex_t;

#endif /* !__LOCK_MUTEX__H__*/