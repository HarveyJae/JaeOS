#include "start/main.h"
#include "common/rv64.h"
#include "dev/uart.h"
#include "dev/dtb.h"
#include "lib/printf.h"
#include "mmu/pmm.h"
#include "mmu/vmm.h"
#include "trap/trap.h"
#include "dev/timer.h"
#include "dev/plic.h"
#include "process/thread.h"
extern char end[]; /* .ld文件中定义的堆起始地址(JaeOS不区分堆栈)*/
uint64_t hart_id;
/**
 * __sync_synchronize()之前的所有内存操作都会在__sync_synchronize()之后的内存操作之前完成
 * __sync_synchronize()之前的内存操作对其他核心是可见的(结果可见或结果同步)
 * __sync_synchronize()不会确保多核并行指令之间的顺序性，对于原子指令还是可以使用的
 * 对于非原子指令，最好使用锁，并且根据情况选择互斥锁或者自旋锁
 *
 */
// extern MEM_INFO mem_info;
int main()
{
    /* 主核hart0*/
    if (hart_id == 0L)
    {
        /* 初始化串口*/
        uart_init();
        early_printf("\n[JaeOS]UART Init Successful.\n");
        /* 初始化输出函数*/
        printf_init();
        printf("\n[JaeOS]Printf Function Init Successful.\n");
        

        /* 解析设备树dtb*/
        printf("\n[JaeOS]DTB Parse Start.\n");
        dtb_prase(dtb_entry);
        printf("[JaeOS]DTB Parse Successful.\n");

        /* 初始化物理内存模块*/
        printf("\n[JaeOS]Physical Memory Init Start.\n");
        pmm_init();
        printf("[JaeOS]Physical Memory Init Successful.\n");
        
        /* 初始化虚拟内存模块*/
        printf("\n[JaeOS]Virtual Memory Init Start.\n");
        vmm_init();
        printf("\n[JaeOS]Virtual Memory Init Successful.\n");

        /* 使能页表*/
        vm_enable();
        printf("\n[JaeOS]VM Enable Successful.\n");

        /* 设置异常向量表*/
        set_trap_handle();
        printf("\n[JaeOS]Set Trap Vector Successful.\n");

        /* 定时器初始化*/
        timer_init();
        printf("\n[JaeOS]Timer Init Successful.\n");

        /* 初始化PLIC(启动中断)*/
        plic_init(hart_id);
        printf("\n[JaeOS]PLIC Init Successful.\n");
        
        /* 初始化线程*/
        thread_init();
        printf("\n[JaeOS]Thread Init Successful.\n");
        /* Logo打印放到最后*/
        logo_init();
    }
    /* 从核hartx*/
    else
    {
        /* not to here*/
    }
    while (1)
        ;
}
