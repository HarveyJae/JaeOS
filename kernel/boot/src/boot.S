#include "common/asm.h"
#include "memory.h"
.comm k_boot_stack, (NCPU * K_BOOT_STACK_SIZE)
.section .text
.global _start
_start:
        # 设置启动栈
        # 内核刚开始执行，此时CPU运行在M-Mode下
        # RV64启动时，硬件会在寄存器中写入相关信息
        # RISC-V规定：a0存储hartid，a1存储dtb地址
        # with a 4kb stack per CPU.
        # sp = k_boot_stack + ((hartid+1) * 4kb)

        # status = M, a0 = hartid, a1 = dtb
        la sp, k_boot_stack

        li t0, K_BOOT_STACK_SIZE
        mv t1, a0
        addi t1, t1, 1
        mul t0, t0, t1
        add sp, sp, t0

        # jump to start() in start.c
        # pass a0 as hartid
        call init_c

        # 未定义的行为，通常不会执行到这里
        j .
