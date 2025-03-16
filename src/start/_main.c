#include "common/types.h"
#include "common/rv64.h"
/**
 * CPU当前运行在M-mode下
 */
extern uint64_t dtb_entry;
int main(void);
/**
 *
 */
/* 陷阱函数 */
void _trap(void)
{
	while (1)
		;
}
/**
 * @brief _main
 * 		  RISC-V的函数的调用规则，a0寄存器作为第一个参数，a1寄存器作为第二个参数
 *
 * @param _hart_id
 * @param _dtb_entry
 */
void _main(int64_t _hart_id, uint64_t _dtb_entry)
{
	/* 关闭分页机制，虽然复位后改寄存器会自动关闭分页*/
	/* 复位satp寄存器*/
	write_satp(0L);

	/* 启动S-Mode下的外部中断SEIE/定时器中断STIE/软件中断SSIE*/
	write_sie(read_sie() | SIE_SEIE | SIE_STIE | SIE_SSIE);

	/* 读取openSBI提供的dtb地址*/

	/* dtb_entry在bss段中，这是不安全的，应该放到只有主核能操作的区域，或者将dtb的值保存到某个寄存器中*/
	/* 由于主核会清空bss段，而清空的过程中从核会执行该函数，因此该函数只允许寄存器操作，不允许使用内核栈*/
	// dtb_entry = _dtb_entry;

	/* 在每个CPU的tp寄存器中保存hartid*/
	write_tp(_hart_id);

	/* S-Mode下的异常陷入位置*/
	write_stvec((uint64_t)_trap);

	/* call main*/
	main();
}