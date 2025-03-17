#include "dev/dtb.h"
#include "lib/printf.h"
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

/* 全局地址 */
MEM_INFO mem_info;
/**
 * @brief 获取big-endian编码的文件数据，最大支持64数据
 *
 * @param field
 * @param size
 * @return uint64_t
 */
static uint64_t get_big_endian_data(uint8_t *field, uint32_t size)
{
    /* big:0x1234    field[0] = 12    field[1] = 34*/
    switch (size)
    {
    case 1:
    {
        return (uint64_t)*field;
    }
    case 2:
    {
        return (uint64_t)field[1] | (uint64_t)field[0] << 8;
    }
    case 4:
    {
        return (uint64_t)field[3] | (uint64_t)field[2] << 8 | (uint64_t)field[1] << 16 | (uint64_t)field[0] << 24;
    }
    case 8:
    {
        return (uint64_t)field[7] | (uint64_t)field[6] << 8 | (uint64_t)field[5] << 16 | (uint64_t)field[4] << 24 | (uint64_t)field[3] << 32 | (uint64_t)field[2] << 40 | (uint64_t)field[1] << 48 | (uint64_t)field[0] << 56;
    }
    default:
    {
        /* 不正确的数据大小*/
        while (1)
            ;
    }
    }
}
/**
 * @brief 解析设备树头部
 *
 * @param fdt_head
 */
static void dtb_head_parse(FDT_Head *fdt_head)
{
    /* 正确获取大端编码数据*/
    fdt_head->magic = (uint32_t)get_big_endian_data((uint8_t *)&(fdt_head->magic), sizeof(fdt_head->magic));
    fdt_head->totalsize = (uint32_t)get_big_endian_data((uint8_t *)&(fdt_head->totalsize), sizeof(fdt_head->totalsize));
    fdt_head->dt_struct_offset = (uint32_t)get_big_endian_data((uint8_t *)&(fdt_head->dt_struct_offset), sizeof(fdt_head->dt_struct_offset));
    fdt_head->dt_strings_offset = (uint32_t)get_big_endian_data((uint8_t *)&(fdt_head->dt_strings_offset), sizeof(fdt_head->dt_strings_offset));
    fdt_head->mem_rsvmap_offset = (uint32_t)get_big_endian_data((uint8_t *)&(fdt_head->mem_rsvmap_offset), sizeof(fdt_head->mem_rsvmap_offset));
    fdt_head->version = (uint32_t)get_big_endian_data((uint8_t *)&(fdt_head->version), sizeof(fdt_head->version));
    fdt_head->last_comp_version = (uint32_t)get_big_endian_data((uint8_t *)&(fdt_head->last_comp_version), sizeof(fdt_head->last_comp_version));
    fdt_head->boot_cpuid_phys = (uint32_t)get_big_endian_data((uint8_t *)&(fdt_head->boot_cpuid_phys), sizeof(fdt_head->boot_cpuid_phys));
    /* 验证设备树头部 */
    if (fdt_head->magic != FDT_MAGIC)
    {
        early_printf("[Error]: Invalid DTB Magic 0x%x\n", fdt_head->magic);
        /* 设备树非法*/
        while (1)
            ;
    }
    /* 打印dtb头部信息*/
    early_printf("[JaeOS]DTB Header:\n");
    early_printf("       Magic Number:                     0x%X\n", fdt_head->magic);
    early_printf("       Total Size:                       0x%X (bytes)\n", fdt_head->totalsize);
    early_printf("       Version:                          %u\n", fdt_head->version);
    early_printf("       Boot CpuID:                       %u\n", fdt_head->boot_cpuid_phys);
    early_printf("       Struct Offset:                    0x%X\n", fdt_head->dt_struct_offset);
    early_printf("       String Offset:                    0x%X\n", fdt_head->dt_strings_offset);
    early_printf("       Memrsv Offset:                    0x%X\n", fdt_head->mem_rsvmap_offset);
}

/* 节点区的字节序是Big-Endian的，因此需要按照Little-Endian读取(但不需要修改原始顺序) */
// static uint32_t read_from_big_endian32(void *ptr)
// {
//     uint32_t num = 0;
//     num |= ((uint32_t)*(uint8_t *)(ptr + 3)) << (32 - 8);
//     num |= ((uint32_t)*(uint8_t *)(ptr + 2)) << (32 - 8 - 8);
//     num |= ((uint32_t)*(uint8_t *)(ptr + 1)) << (32 - 8 - 8 - 8);
//     num |= ((uint32_t)*(uint8_t *)(ptr + 0)) << (32 - 8 - 8 - 8 - 8);
//     return num;
// }
// static uint64_t read_from_big_endian64(void *ptr)
// {
//     uint64_t num = 0;
//     num |= ((uint64_t)*(uint8_t *)(ptr + 7)) << (64 - 8);
//     num |= ((uint64_t)*(uint8_t *)(ptr + 6)) << (64 - 8 - 8);
//     num |= ((uint64_t)*(uint8_t *)(ptr + 5)) << (64 - 8 - 8 - 8);
//     num |= ((uint64_t)*(uint8_t *)(ptr + 4)) << (64 - 8 - 8 - 8 - 8);
//     num |= ((uint64_t)*(uint8_t *)(ptr + 3)) << (64 - 8 - 8 - 8 - 8 - 8);
//     num |= ((uint64_t)*(uint8_t *)(ptr + 2)) << (64 - 8 - 8 - 8 - 8 - 8 - 8);
//     num |= ((uint64_t)*(uint8_t *)(ptr + 1)) << (64 - 8 - 8 - 8 - 8 - 8 - 8 - 8);
//     num |= ((uint64_t)*(uint8_t *)(ptr + 0)) << (64 - 8 - 8 - 8 - 8 - 8 - 8 - 8 - 8);
//     return num;
// }
/**
 * 从字符串表中获取属性名称
 */
// static uint8_t *fdt_get_string(FDT_Head *fdt_head, uint32_t offset)
// {
//     /* 默认认为fdt_head已经从大端序转换为小端序 */
//     return (uint8_t *)fdt_head + fdt_head->dt_strings_offset + offset;
// }
/* 设备树是一个树形结构，一个典型的包含关系：根节点->总线节点->设备节点->寄存器节点...*/
/* 每个节点的解析流程相同 */
/* 对于树形结构的数据，通常采用递归 */
/* 根节点的深度是0 */
// static void *dtb_node_prase(FDT_Head *fdt_head, void *ptr, int8_t *parent, int32_t depth)
// {
//     /* ptr的初始值是设备树节点结构的初始地址 */
//     /* 读取节点的开始标记 */
//     uint32_t token = read_from_big_endian32(ptr);
//     if (token != FDT_BEGIN_NODE)
//     {
//         /* 错误检查 */
//     }
//     /* 开始标记的下一个字节就是node_name的起始 */
//     uint8_t *node_name = (uint8_t *)ptr + 4;
//     /* 打印node_name:node_name以'\0'结尾 */
//     /* 打印当前节点的父节点 */

//     /* node_name之后就是当前节点的属性 */
//     ptr += 4 + strlen(node_name) + 1;
//     /* 由于属性或子节点开头是四字节对齐的，因此需要让ptr四字节对齐才能正确访问(node_name是一字节对齐) */
//     /* 四字节对齐要求地址的最后两位为0，说明能整除4 */
//     /* 为什么要对当前地址+3：如果当前地址是四字节对齐地址，那么+4的话会直接到下一个对齐地址，即错过了4字节的数据，+3可以避免移动到下一个对齐地址 */
//     /* 如果当前地址不是四字节对齐，例如5，+3等于8，也就是刚好能移动到正确的对齐地址，因此+3能确保找到正确的对齐4字节(不一定是首地址) */
//     /* 为什么要& ~3：由于+3后不一定能到对齐首地址，因此需要把最后两位清零，这样得到的才是正确的四字节对齐地址 */
//     ptr = (void *)(((uint64_t)ptr + 3) & ~3);
//     /* 现在ptr正确的指向当前节点的属性 */

//     /* 解析属性和子节点 */
//     while (1)
//     {
//         token = read_from_big_endian32(ptr);

//         /* 解析当前节点属性 */
//         if (token == FDT_PROP)
//         {
//             FDT_Node_Property *fdt_prop = (FDT_Node_Property *)ptr;
//             uint8_t *prop_name = fdt_get_string(fdt_head, read_from_big_endian32(&(fdt_prop->nameoff)));
//             uint32_t prop_len = read_from_big_endian32(&(fdt_prop->len));

//             /* 打印属性信息 */

//             /* 移动指针，fdt_prop后面紧跟着属性值 */
//             ptr += sizeof(FDT_Node_Property);
//             /* 处理特定属性:全局内存布局 */
//             if (strcmp(node_name, "memory@0") == 0 && strcmp(prop_name, "reg") == 0)
//             {
//                 mem_info.start = read_from_big_endian64(ptr);
//                 mem_info.size = read_from_big_endian64(ptr + 8);
//             }
//             ptr += prop_len;

//             /* 四字节对齐 */
//             ptr = (void *)(((uint64_t)ptr + 3) & ~3);
//         }
//         /* 解析子节点 */
//         else if (token == FDT_BEGIN_NODE)
//         {
//             ptr = dtb_node_prase(fdt_head, ptr, node_name, depth + 1);
//             /* 递归解析 */
//         }
//         else if (token == FDT_END_NODE)
//         {
//             ptr += 4; /* 跳过结束标志 */
//             /* 打印结束节点信息和名称 */

//             return ptr;
//         }
//         else if (token == FDT_END)
//         {
//             return ptr;
//         }
//         else if (token == FDT_NOP)
//         {
//             /* 跳过这条无用信息 */
//             ptr += 4;
//         }
//         else
//         {
//             /* 错误检查 */
//         }
//     }
// }

// /* 解析保留内存表*/
// static void dtb_mem_rsvmap_prase(FDT_Head *fdt_head)
// {
//     /* 默认认为fdt_head已经由大端序转换成小端序 */
//     void *ptr = (uint8_t *)fdt_head + fdt_head->mem_rsvmap_offset;

//     early_printf("\n[Memory Reserved Regions]\n");

//     while (1)
//     {
//         uint64_t address = read_from_big_endian64(ptr);
//         uint64_t size = read_from_big_endian64(ptr + 8);

//         early_printf("Start 0x%016llx, Size 0x%llx\n", address, size);
//         if (address == 0 && size == 0)
//         {
//             break;
//         }
//         ptr += 16;
//     }
// }
/**
 * @brief dtb解析函数，主要通过设备树获取硬件设备的内存布局，避免硬编码
 *        设备树的起始地址需要四字节对齐，由SBI确定的
 *
 * @param _dtb_entry
 */
void dtb_prase(uint64_t _dtb_entry)
{
    /* 打印dtb的物理地址*/
    early_printf("[JaeOS]DTB Entry Address: 0x%lX.\n", _dtb_entry);
    FDT_Head *fdt_h = (FDT_Head *)_dtb_entry;

    /* 解析设备树头 */
    dtb_head_parse(fdt_h);

    // /* 解析保留内存 */
    // dtb_mem_rsvmap_prase(fdt_h);

    // /* 获取设备树节点结构的起始地址 */
    // void *struct_ptr = (void *)(fdt_h + fdt_h->dt_struct_offset);
    // /* 递归解析设备树节点 */
    // dtb_node_prase(fdt_h, struct_ptr, "/", 0);

    // /* 打印全局内存信息 */
    // early_printf("\n[Memory Info]\nStart: 0x%016lx, Size:%lu MB\n", mem_info.start, mem_info.size / 1024 / 1024);
    // early_printf("DTB Init Succsee.");
}