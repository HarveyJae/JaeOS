#include "dtb.h"

/**
 * 设备树DTB，Device Tree Blob是内核启动时描述硬件信息的二进制文件，内核通过解析DTB来获取内存布局，外设信息等
 * 设备树可以避免编写内核过程中的硬编码：设备树将硬件描述与内核代码分离
 * 内核启动时，bootloader(对于RV64来说是SBI)将设备树的二进制文件传递给内核
 *
 * 设备树工作原理：
 * 1.bootloader加载：加载DTB到内存，将DTB的地址通过寄存器a1传递给内核，注意内核加载早期不要污染寄存器a1的值
 * 2.内核解析DTB：a.验证DTB头部0xd00dfeed b.递归解析所有节点 c.通过compatible属性匹配内核中的设备驱动
 *
 *
 * Big-Endian to Little-Endian:
 * DTB的字节序列是大端序，而RV64平台采用小端序，因此需要将数据转换成小端序才能正确处理信息
 */
/* 设备树入口默认地址 */
uint64_t dtb_entry = 0;

/* 设备树的起始地址需要四字节对齐，这个是由SBI确定的 */
/* 大端和小端的转换很简单：0xaabbccdd -> 0xddccbbaa，aa和dd换，bb和cc换即可 */
static void big_endian2little(void *start, uint32_t size)
{
    for (uint32_t i = 0; i < size; i += 4)
    {
        uint8_t temp;
        /* 首尾互换 */
        temp = *(uint8_t *)(start + i);
        *(uint8_t *)(start + i) = *(uint8_t *)(start + i + 3);
        *(uint8_t *)(start + i + 3) = temp;
        /* 中间互换 */
        temp = *(uint8_t *)(start + i + 1);
        *(uint8_t *)(start + i + 1) = *(uint8_t *)(start + i + 2);
        *(uint8_t *)(start + i + 2) = temp;
    }
}
static void dtb_head_parse(FDT_Head *fdt_head)
{
    big_endian2little(fdt_head, sizeof(FDT_Head));
    /* 待补充打印信息 */
}

/* 节点区的字节序是Big-Endian的，因此需要按照Little-Endian读取(但不需要修改原始顺序) */
static uint32_t read_from_big_endian32(uint32_t *ptr)
{
    uint32_t num = 0;
    num |= ((uint32_t)*(uint8_t *)(ptr + 3)) << (32 - 8);
    num |= ((uint32_t)*(uint8_t *)(ptr + 2)) << (32 - 8 - 8);
    num |= ((uint32_t)*(uint8_t *)(ptr + 1)) << (32 - 8 - 8 - 8);
    num |= ((uint32_t)*(uint8_t *)(ptr + 0)) << (32 - 8 - 8 - 8 - 8);
    return num;
}
static uint64_t read_from_big_endian64(uint64_t *ptr)
{
    uint64_t num = 0;
    num |= ((uint64_t)*(uint8_t *)(ptr + 7)) << (64 - 8);
    num |= ((uint64_t)*(uint8_t *)(ptr + 6)) << (64 - 8 - 8);
    num |= ((uint64_t)*(uint8_t *)(ptr + 5)) << (64 - 8 - 8 - 8);
    num |= ((uint64_t)*(uint8_t *)(ptr + 4)) << (64 - 8 - 8 - 8 - 8);
    num |= ((uint64_t)*(uint8_t *)(ptr + 3)) << (64 - 8 - 8 - 8 - 8 - 8);
    num |= ((uint64_t)*(uint8_t *)(ptr + 2)) << (64 - 8 - 8 - 8 - 8 - 8 - 8);
    num |= ((uint64_t)*(uint8_t *)(ptr + 1)) << (64 - 8 - 8 - 8 - 8 - 8 - 8 - 8);
    num |= ((uint64_t)*(uint8_t *)(ptr + 0)) << (64 - 8 - 8 - 8 - 8 - 8 - 8 - 8 - 8);
    return num;
}

/* 设备树是一个树形结构，一个典型的包含关系：根节点->总线节点->设备节点->寄存器节点...*/
/* 每个节点的解析流程相同 */
/* 对于树形结构的数据，通常采用递归 */
static void dtb_node_prase(FDT_Head *fdt_head, void *node, int8_t *parent)
{
}
void dtb_prase(void)
{
    /* 可以打印dtb的物理地址 */
    /* 将DTB地址转换为设备树头结构 */
    FDT_Head *fdt_h = (FDT_Head *)dtb_entry;

    /* 解析设备树头 */
    dtb_head_parse(fdt_h);

    /* 获取设备树节点结构的起始地址 */
    void *node = (void *)(fdt_h + fdt_h->dt_struct_offset);

    /* 递归解析设备树节点 */
}