#include "common/platform.h"

# 在bss段分配内核启动栈
.comm k_boot_stack, (NCPU * K_BOOT_STACK_SIZE)

.section .text.boot
.global _start
_start:
        # 默认 4 hart启动
        # 读取hartid
        # OpenSBI将hartid存入a0，将dtb地址存入a1
        # hart0作为主核执行内核初始化
        la sp, k_boot_stack

        # hartx初始化内核栈
        li t0, K_BOOT_STACK_SIZE
        mv t1, a0
        addi t1, t1, 1
        mul t0, t0, t1
        add sp, sp, t0
        # 跳转到C代码执行
        call _main

split:
        # Not to here......
        j split
                           