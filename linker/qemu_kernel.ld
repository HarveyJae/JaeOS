OUTPUT_ARCH("riscv")
ENTRY(_start)
/*内存布局:0x80200000 - 0x88000000  126MB*/
SECTIONS
{
    /*确保_start的起始地址是0x80200000*/
    . = 0x80200000;
    .text : 
    {
        *(.text.boot) /*确保启动代码链接在最前面*/
        *(.text .text.*)
        . = ALIGN(0x1000); /*4KB字节对齐*/
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
    . = ALIGN(0x1000);
    PROVIDE(end = .); /*Heap的起始地址*/
}