#include "common/types.h"
#include "common/rv64.h"
extern int main();

/**
 * @brief 临时异常处理函数
 * 
 */
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
void _main()
{
	/* 关闭分页机制，虽然复位后改寄存器会自动关闭分页*/
	/* 复位satp寄存器*/
	write_satp(0L);

	/* 启动S-Mode下的外部中断SEIE/定时器中断STIE/软件中断SSIE*/
	write_sie(read_sie() | SIE_SEIE | SIE_STIE);

	/* 读取openSBI提供的dtb地址*/

	/* dtb_entry在bss段中，这是不安全的，应该放到只有主核能操作的区域，或者将dtb的值保存到某个寄存器中*/
	/* 由于主核会清空bss段，而清空的过程中从核会执行该函数，因此该函数只允许寄存器操作，不允许使用内核栈*/
	/* 解决方案，不要污染a2和a3寄存器，在main函数中读取这两个寄存器，见启动代码*/
	/* 后续需要完善清除bss代码，确保不同的核在清除完bss段之后再进入_main*/
	// dtb_entry = _dtb_entry;

	/* 设置临时异常陷入地址*/
	write_stvec((uint64_t)_trap);

	/* call main*/
	/* 将参数传递给main，main函数不需要返回地址，因此即使启动栈被清零，也没有影响*/
	main();
}