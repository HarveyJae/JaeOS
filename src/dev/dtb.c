#include "dtb.h"
#include <string.h>
#include "jaeio.h"
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
/* 设备树入口地址 */
uint64_t dtb_entry = 0;

/* 全局地址 */
MEM_INFO mem_info;

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
static uint32_t read_from_big_endian32(void *ptr)
{
    uint32_t num = 0;
    num |= ((uint32_t)*(uint8_t *)(ptr + 3)) << (32 - 8);
    num |= ((uint32_t)*(uint8_t *)(ptr + 2)) << (32 - 8 - 8);
    num |= ((uint32_t)*(uint8_t *)(ptr + 1)) << (32 - 8 - 8 - 8);
    num |= ((uint32_t)*(uint8_t *)(ptr + 0)) << (32 - 8 - 8 - 8 - 8);
    return num;
}
static uint64_t read_from_big_endian64(void *ptr)
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
/**
 * 从字符串表中获取属性名称
 */
static uint8_t *fdt_get_string(FDT_Head *fdt_head, uint32_t offset)
{
    /* 默认认为fdt_head已经从大端序转换为小端序 */
    return (uint8_t *)fdt_head + fdt_head->dt_strings_offset + offset;
}
/* 设备树是一个树形结构，一个典型的包含关系：根节点->总线节点->设备节点->寄存器节点...*/
/* 每个节点的解析流程相同 */
/* 对于树形结构的数据，通常采用递归 */
/* 根节点的深度是0 */
static void *dtb_node_prase(FDT_Head *fdt_head, void *ptr, int8_t *parent, int32_t depth)
{
    /* ptr的初始值是设备树节点结构的初始地址 */
    /* 读取节点的开始标记 */
    uint32_t token = read_from_big_endian32(ptr);
    if (token != FDT_BEGIN_NODE)
    {
        /* 错误检查 */
    }
    /* 开始标记的下一个字节就是node_name的起始 */
    uint8_t* node_name = (uint8_t *)ptr + 4;
    /* 打印node_name:node_name以'\0'结尾 */
    /* 打印当前节点的父节点 */

    /* node_name之后就是当前节点的属性 */
    ptr += 4 + strlen(node_name) + 1;
    /* 由于属性或子节点开头是四字节对齐的，因此需要让ptr四字节对齐才能正确访问(node_name是一字节对齐) */
    /* 四字节对齐要求地址的最后两位为0，说明能整除4 */
    /* 为什么要对当前地址+3：如果当前地址是四字节对齐地址，那么+4的话会直接到下一个对齐地址，即错过了4字节的数据，+3可以避免移动到下一个对齐地址 */
    /* 如果当前地址不是四字节对齐，例如5，+3等于8，也就是刚好能移动到正确的对齐地址，因此+3能确保找到正确的对齐4字节(不一定是首地址) */
    /* 为什么要& ~3：由于+3后不一定能到对齐首地址，因此需要把最后两位清零，这样得到的才是正确的四字节对齐地址 */
    ptr = (void *)(((uint64_t)ptr + 3) & ~3);
    /* 现在ptr正确的指向当前节点的属性 */

    /* 解析属性和子节点 */
    while (1)
    {
        token = read_from_big_endian32(ptr);

        /* 解析当前节点属性 */
        if (token == FDT_PROP)
        {
            FDT_Node_Property *fdt_prop = (FDT_Node_Property *)ptr;
            uint8_t *prop_name = fdt_get_string(fdt_head, read_from_big_endian32(&(fdt_prop->nameoff)));
            uint32_t prop_len = read_from_big_endian32(&(fdt_prop->len));

            /* 打印属性信息 */

            /* 移动指针，fdt_prop后面紧跟着属性值 */
            ptr += sizeof(FDT_Node_Property);
            /* 处理特定属性:全局内存布局 */
            if (strcmp(node_name, "memory@0") == 0 && strcmp(prop_name, "reg") == 0)
            {
                mem_info.start = read_from_big_endian64(ptr);
                mem_info.size = read_from_big_endian64(ptr + 8);
            }
            ptr += prop_len;

            /* 四字节对齐 */
            ptr = (void *)(((uint64_t)ptr + 3) & ~3);
        }
        /* 解析子节点 */
        else if (token == FDT_BEGIN_NODE)
        {
            ptr = dtb_node_prase(fdt_head, ptr, node_name, depth + 1);
            /* 递归解析 */
        }
        else if (token == FDT_END_NODE)
        {
            ptr += 4;/* 跳过结束标志 */
            /* 打印结束节点信息和名称 */

            return ptr;
        }
        else if (token == FDT_END)
        {
            return ptr;
        }
        else if (token == FDT_NOP)
        {
            /* 跳过这条无用信息 */
            ptr += 4;
        }
        else
        {
            /* 错误检查 */
        }
    }
}

/* 解析保留内存表*/
static void dtb_mem_rsvmap_prase(FDT_Head *fdt_head)
{
    /* 默认认为fdt_head已经由大端序转换成小端序 */
    void *ptr = (uint8_t *)fdt_head + fdt_head->mem_rsvmap_offset;

    early_printf("\n[Memory Reserved Regions]\n");

    while (1)
    {
        uint64_t address = read_from_big_endian64(ptr);
        uint64_t size = read_from_big_endian64(ptr + 8);

        early_printf("Start 0x%016llx, Size 0x%llx\n", address, size);
        if (address == 0 && size == 0)
        {
            break;
        }
        ptr += 16;
    }
}
/* 主解析函数 */
int32_t dtb_prase(void)
{
    /* 可以打印dtb的物理地址 */
    /* 将DTB地址转换为设备树头结构 */
    FDT_Head *fdt_h = (FDT_Head *)dtb_entry;

    /* 解析设备树头 */
    dtb_head_parse(fdt_h);

    /* 验证设备树头部 */
    if (fdt_h->magic != FDT_MAGIC)
    {
        early_printf("Error: Invalid DTB magic 0x%08x\n", fdt_h->magic);
        return -1;
    }

    /* 打印DTB信息 */
    early_printf("DTB Version: %d, Total Size: %d KB\n", fdt_h->version, fdt_h->totalsize / 1024);

    /* 解析保留内存 */
    dtb_mem_rsvmap_prase(fdt_h);

    /* 获取设备树节点结构的起始地址 */
    void *struct_ptr = (void *)(fdt_h + fdt_h->dt_struct_offset);
    /* 递归解析设备树节点 */
    dtb_node_prase(fdt_h, struct_ptr, "/", 0);  

    /* 打印全局内存信息 */
    early_printf("\n[Memory Info]\nStart: 0x%016lx, Size:%lu MB\n", mem_info.start, mem_info.size / 1024 / 1024);
    early_printf("DTB Init Succsee.");

    return 0;
}