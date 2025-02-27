/**
 * SBI即Supervisor Binary Interface
 * RISC-V规定了该规范。SBI对所有RISC-V硬件平台共性的功能做了抽象，为运行在S模式下的操作系统
 * 提供统一的服务接口，业界常用的SBI固件是OpenSBI，当然也有国内社区贡献者编写的RustSBI。
 * 1.SBI为运行在S-Mode(也可能是U-Mode)下访问M-Mode下的硬件资源提供抽象接口，无需直接操作硬件寄存器。
 * 2.M-Mode下的处理器具有对系统资源的全部访问权限。
 * 3.提高可移植性，SBI类似于HAL，当然也可以直接理解成HAL。
 * 
 * 在没有开启虚拟化扩展的RISC-V系统下：
 *              US ER               U-Mode
 *                ^
 *                |      SysCall  
 *                v  
 *               O S                S-Mode
 *                ^
 *                |      SbiCall
 *                v
 *               SBI                M-Mode
 * 
 * 
 * 调用规则：
 * - 内核通过ecall指令从S-Mode陷入M-Mode
 * - 注意，ecall的行为由stvec指向的陷阱处理程序直接定义，我们在main.c中定义了这个行为。
 * - 但是，SBI的ecall调用会被固件直接截获，不会陷入陷阱。
 * - a7：扩展ID
 * - a6：函数ID
 * - a0-a5：输入参数(最多6个)
 * - a0：错误码
 * - a1：返回值(具体意义由拓展定义)
 * 
 * 常用错误码：
 * - 0：调用成功
 * - -1：通用错误
 * - -2：扩展未实现
 * - -3：参数非法
 * 常用基础扩展0x10：
 * - sbi_get_spec_version()：获取SBI规范版本
 * - sbi_get_impl_id()：获取固件实现ID
 * - sbi_get_mvendorid()：获取厂商ID
 * ...更多扩展见官方文档
 * 
 * SBI通用封装调用：
 * #define SBI_CALL(ext, fn, a0, a1, a2, a3) ({ \
    register uint64_t a7 asm ("a7") = (ext); \
    register uint64_t a6 asm ("a6") = (fn); \
    register uint64_t a0_r asm ("a0") = (a0); \
    register uint64_t a1_r asm ("a1") = (a1); \
    register uint64_t a2_r asm ("a2") = (a2); \
    register uint64_t a3_r asm ("a3") = (a3); \
    asm volatile ("ecall" \
        : "+r" (a0_r), "+r" (a1_r) \
        : "r" (a2_r), "r" (a3_r), "r" (a6), "r" (a7) \
        : "memory" \
    ); \
    // 返回值通过 a0 和 a1 返回 \
})
 * Chapt:https://gitee.com/tinylab/riscv-linux/blob/master/articles/20230612-introduction-to-riscv-sbi.md
 * 
 * 官方文档：https://github.com/riscv-non-isa/riscv-sbi-doc
 * 
 */

 #include "sbi.h"

 