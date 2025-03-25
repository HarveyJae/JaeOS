#include "common/platform.h"

# 在bss段分配内核启动栈
.comm k_boot_stack, (NCPU * K_BOOT_STACK_SIZE)

.section .text.boot
.global _start
_start:
        # 执行内核代码的时候，openSBI会自动将内核模式设置为S-Mode，因此不能非法访问M-Mode寄存器
        # 默认 4 hart启动
        # 读取hartid
        # OpenSBI将hartid存入a0，将dtb地址存入a1
        # hart0作为主核执行内核初始化
        # 若非主核，跳转到从核初始化
        bnez a0, secondary
        # 主核清空.bss段
        la t0, __bss_start
        la t1, __bss_end
        bgeu t0, t1, bss_done
bss_loop:
        # 循环清空bss段
        sd zero, (t0)
        addi t0, t0, 8
        bltu t0, t1, bss_loop
bss_done:
        # 设置主核内核栈
        la sp, k_boot_stack
        li t0, K_BOOT_STACK_SIZE
        mv t1, a0
        addi t1, t1, 1
        mul t0, t0, t1
        add sp, sp, t0
        # 保存hart_id和dtb_entry
        mv a2, a0
        mv a3, a1
        # 跳转到C代码执行
        call _main
        j split
secondary:
        # 设置从核内核栈
        # 这逻辑同时适用主核和从核内核栈设置
        la sp, k_boot_stack
        li t0, K_BOOT_STACK_SIZE
        mv t1, a0
        addi t1, t1, 1
        mul t0, t0, t1
        add sp, sp, t0
        # 保存hart_id
        mv a2, a0
        # 跳转到C代码执行
        wfi
        call _main
split:
        # Not to here......
        j split

                           