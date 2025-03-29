OUTPUT_ARCH("riscv")
ENTRY(_start)
SECTIONS
{
    /*确保_start的起始地址是0x80200000*/
    . = 0x80200000;
    .text : 
    {
        PROVIDE(__text_start = .);  /*text段结开始地址，等于0x80200000*/
        *(.text.boot)               /*确保启动代码链接在最前面*/
        *(.text .text.*)
        . = ALIGN(0x1000);          /*4KB字节对齐*/
        PROVIDE(__text_end = .);    /*text段结束地址*/

        PROVIDE(__trampoline_start = .);  /*trampoline起始地址*/
        *(.trampoline)
        . = ALIGN(0x1000);
        ASSERT(. - __trampoline_start == 0x1000, "error: trampoline larger than one page");
        PROVIDE(__trampoline_end = .);    /*trampoline结束地址*/

        PROVIDE(__sigsec_start = .);
	    *(.sigsec)
	    . = ALIGN(0x1000);                /*sigsec开始地址*/
	    ASSERT(. - __sigsec_start == 0x1000, "error: sigSec larger than one page");
        PROVIDE(__sigsec_end = .);        /*sigsec结束地址*/
    }
    .rodata : 
    {
        . = ALIGN(16); /*16字节对齐*/
        *(.srodata .srodata.*)
        . = ALIGN(16);
        *(.rodata .rodata.*)
    }
    .data : 
    {
        . = ALIGN(16); /*16字节对齐*/
        *(.sdata .sdata.*)
        . = ALIGN(16);
        *(.data .data.*)
    }
    .bss : 
    {
        . = ALIGN(16); /*16字节对齐*/
        PROVIDE(__bss_start = .); /*bss段起始地址*/
        *(.sbss .sbss.*)
        . = ALIGN(16);
        *(.bss .bss.*)
        PROVIDE(__bss_end = .); /*bss段结束地址*/
    }
    .initcall_fs : 
    {
        . = ALIGN(8);
        PROVIDE(__initcall_fs_start = .);
        KEEP(*(.initcall_fs))  /* 避免此段被链接器垃圾清理（因为这些符号在代码中没有实际使用到） */
	    PROVIDE(__initcall_fs_end = .);
    }
    . = ALIGN(0x1000);
    PROVIDE(end = .); /*Heap的起始地址*/
}