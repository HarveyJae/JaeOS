#ifndef __TRAP_TRAP__H__
#define __TRAP_TRAP__H__
#include "common/types.h"

/**
 * @brief 用户态中断上下文(寄存器帧)
 * 
 */
typedef struct
{
	uint64_t kernel_sp;	   /* 内核的sp指针*/
	uint64_t kernel_satp;  /* 内核页表*/
	uint64_t trap_handler; /* 用户态异常处理函数*/
	uint64_t epc;		   /* 用户epc*/
	uint64_t hartid;
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
	uint64_t ft0;
	uint64_t ft1;
	uint64_t ft2;
	uint64_t ft3;
	uint64_t ft4;
	uint64_t ft5;
	uint64_t ft6;
	uint64_t ft7;
	uint64_t fs0;
	uint64_t fs1;
	uint64_t fa0;
	uint64_t fa1;
	uint64_t fa2;
	uint64_t fa3;
	uint64_t fa4;
	uint64_t fa5;
	uint64_t fa6;
	uint64_t fa7;
	uint64_t fs2;
	uint64_t fs3;
	uint64_t fs4;
	uint64_t fs5;
	uint64_t fs6;
	uint64_t fs7;
	uint64_t fs8;
	uint64_t fs9;
	uint64_t fs10;
	uint64_t fs11;
	uint64_t ft8;
	uint64_t ft9;
	uint64_t ft10;
	uint64_t ft11;
} trapframe_t;
/**
 * @brief 内核态中断上下文(寄存器帧)
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

#endif /* !__TRAP_TRAP__H__*/