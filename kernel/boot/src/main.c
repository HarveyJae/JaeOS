#include "init.h"
#include "main.h"
/* 多核启动 */
volatile static uint32_t hart_started[NCPU]; /* 确定哪个核已经被启动 */
volatile static uint32_t hart_first = 1;     /* 首先被启动标志 */
volatile static uint32_t kern_inited = 0;    /* 核启动阻塞标志 */
volatile static hart_start_lock = 0;         /* 自选锁 */
/**
 * __sync_synchronize()之前的所有内存操作都会在__sync_synchronize()之后的内存操作之前完成
 * __sync_synchronize()之前的内存操作对其他核心是可见的(结果可见或结果同步)
 * __sync_synchronize()不会确保多核并行指令之间的顺序性，对于原子指令还是可以使用的
 * 对于非原子指令，最好使用锁，并且根据情况选择互斥锁或者自旋锁
 *
 */

void main(void)
{
    while (__sync_lock_test_and_set(&hart_start_lock, 1) == 1)
        ;
    /* 原子的将lock设置为1，并返回旧值，如果返回0，则证明是获取锁的核心 */
    /* 如果返回1，证明锁已经被其他线程获取了，此时需自旋等待释放 */
    /* 避免多个核心同时进入代码块 */
    if (hart_first == 1)
    {
        hart_first = 0;
        __sync_synchronize();                  /* 内存修改是多核可见的，也可以不写，因为有锁 */
        __sync_lock_release(&hart_start_lock); /* 释放锁，避免死锁 */
        /* 初始化串口 */
        uart_init();
        /* 检测硬件架构 */
        uint64_t marchid = sbi_get_marchid().value;
        switch (marchid)
        {
        case 0x8000:
        {
            early_printf("Machine Arch is RISC-V 64-bit (RV64I).\n");
            break;
        }
        case 0x8001:
        {
            early_printf("Machine Arch is RISC-V 32-bit (RV32I).\n");
            break;
        }
        case 0x5b7:
        {
            early_printf("Machine Arch is SiFive U54-MC.\n");
            break;
        }
        case 0x6001:
        {
            early_printf("Machine Arch is T-HEAD C906.\n");
            break;
        }
        default:
        {
            early_printf("Machine Arch is Unknown.\n");
            break;
        }  
        }
        /* 检测启动核心 */
        early_printf("JaeOS kernel is booting......\n");
        early_printf("Now on Hart %d(Total:%d CPUs).\n", get_hartid(), NCPU);
        /* 读取DTB */
        #ifdef  USE_QEMU_VIRT
        
        #endif  //USE_QEMU_VIRT
        /* 打印OS logo(即将启动成功之前) */
        early_printf("\n");
        early_printf("        JJJ         AAAAA  EEEEEEEEEE        .\"OOOOOOO\".    .SSSSSSSS.\n");
        early_printf("        JJJ        AA  AA  EEE              OOO\"     \"OOO  SSSS    SSSS\n");
        early_printf("        JJJ       AA   AA  EEE              OOO       OOO  SSSS.\n");
        early_printf("        JJJ      AAA   AA  EEEEEEEEEE       OOO       OOO   \"SSSSS.\n");
        early_printf("        JJJ     AAA    AA  EEEEEEEEEE       OOO       OOO      \"SSSS.\n");
        early_printf(" JJ     JJJ    AAAAAAAAAA  EEE              OOO       OOO        \"SSS\n");
        early_printf(" JJJJJJJJJJ   AAAA    AAA  EEE              OOO\"     \"OOO  SSSS    SSSS\n");
        early_printf(" JJJJJJJJJJ  AAAAA    AAA  EEEEEEEEEE        \".OOOOOOO.\"     \"SSSSSSSS\"\n");
        early_printf("\n");
    }
    else
    {
    }
}
