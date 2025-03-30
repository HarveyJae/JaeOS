#include "common/types.h"
#include "common/rv64.h"
#include "dev/dtb.h"
#include "start/main.h"
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
void _main(uint64_t _hart_id, uint64_t _dtb_entry)
{
	/* 关闭分页机制，虽然复位后改寄存器会自动关闭分页*/
	/* 复位satp寄存器*/
	write_satp(0L);

	/* 启动S-Mode下的外部中断SEIE/定时器中断STIE/软件中断SSIE*/
	write_sie(read_sie() | SIE_SEIE | SIE_STIE);

	/* 读取openSBI提供的dtb地址*/
	dtb_entry = _dtb_entry;
	/* 读取openSBI提供的hartid*/
	hart_id = _hart_id;

	/* 设置临时异常陷入地址*/
	write_stvec((uint64_t)_trap);

	/* call main*/
	main();
}