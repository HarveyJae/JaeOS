#ifndef __TRAP_TRAP__H__
#define __TRAP_TRAP__H__
#include "common/types.h"

/**
 * @brief 内核异常上下文(寄存器帧)
 *        在发生异常或中断时，保存被中断代码的完整寄存器状态
 */
typedef struct
{
	uint64_t ra;
	uint64_t sp;
	uint64_t gp;
	uint64_t tp;
	uint64_t t0;
	uint64_t t1;
	uint64_t t2;
	uint64_t s0;
	uint64_t s1;
	uint64_t a0;
	uint64_t a1;
	uint64_t a2;
	uint64_t a3;
	uint64_t a4;
	uint64_t a5;
	uint64_t a6;
	uint64_t a7;
	uint64_t s2;
	uint64_t s3;
	uint64_t s4;
	uint64_t s5;
	uint64_t s6;
	uint64_t s7;
	uint64_t s8;
	uint64_t s9;
	uint64_t s10;
	uint64_t s11;
	uint64_t t3;
	uint64_t t4;
	uint64_t t5;
	uint64_t t6;
} ktrapframe_t;
void set_trap_handle(void);

#endif  /* !__TRAP_TRAP__H__*/