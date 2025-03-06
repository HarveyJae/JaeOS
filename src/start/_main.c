#include "common/types.h"
/**
 * CPU当前运行在M-mode下
 */
// extern uint64_t dtb_entry;
extern int main(void);
/**
 * C函数的调用规则，a0寄存器作为第一个参数，a1寄存器作为第二个参数
 */
/* 陷阱函数 */
void _trap(void)
{
    while (1);
}
void _main(uint64_t hartid, uint64_t _dtb_entry) {
	// 如果设备树没有默认地址，设置dtb_entry
	// if (dtb_entry == 0) 
    // {
	// 	dtb_entry = _dtb_entry;
	// }
	// // enable some supervisor interrupt
	// write_sie(read_sie() | SIE_SEIE | SIE_STIE); // 不启用核间中断

	// // 在每个CPU的tp寄存器中保存hartid
	// write_tp(hartid);
	// write_stvec((uint64_t)_trap);

	main();
}