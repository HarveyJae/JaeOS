#include "dev/dtb.h"
#include "lib/printf.h"
#include "lib/string.h"
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
    early_printf("       Magic Number:                     0x%X\n", fdt_head->magic);             /* 0xD00DFEED*/
    early_printf("       Total Size:                       0x%X (bytes)\n", fdt_head->totalsize); /* 0x16EE*/
    early_printf("       Version:                          %u\n", fdt_head->version);             /* 17*/
    early_printf("       Boot CpuID:                       %u\n", fdt_head->boot_cpuid_phys);     /* 0*/
    early_printf("       Struct Offset:                    0x%X\n", fdt_head->dt_struct_offset);  /* 0x38*/
    early_printf("       String Offset:                    0x%X\n", fdt_head->dt_strings_offset); /* 0x11E4*/
    early_printf("       Memrsv Offset:                    0x%X\n", fdt_head->mem_rsvmap_offset); /* 0x28*/
}
/**
 * @brief 从字符串表中获取属性名称(起始地址)
 *
 * @param fdt_head
 * @param offset
 * @return uint8_t*
 */
static uint8_t *fdt_get_string(FDT_Head *fdt_head, uint32_t offset)
{
    return (uint8_t *)fdt_head + fdt_head->dt_strings_offset + offset;
}
/**
 * @brief 设备树是一个树形结构，一个典型的包含关系：根节点->总线节点->设备节点->寄存器节点...
 *        每个节点的解析流程相同，对于树形结构的数据，通常采用递归，根节点的深度是0
 *
 * @param fdt_head
 * @param ptr        初始值是struct_node的初始地址
 * @param parent
 * @param depth
 * @return uint8_t *
 */
static uint8_t *dtb_node_parse(FDT_Head *fdt_head, uint8_t *ptr, uint8_t *parent, int32_t depth)
{
    /* 打印适配*/
    uint8_t prop_width = 25;
    /* 获取dtb结束地址*/
    uint8_t *dtb_end = (uint8_t *)fdt_head + fdt_head->totalsize;
    /* 读取节点的开始标记*/
    uint32_t token = get_big_endian_data(ptr, sizeof(uint32_t));
    if (token != FDT_BEGIN_NODE)
    {
        /* 错误检查*/
        early_printf("[Error]: Expected FDT_BEGIN_NODE 0x%08X\n", token);
        return NULL;
    }

    /* 更新ptr*/
    ptr += 4;

    /* 读取节点名称*/
    uint8_t *node_name = (uint8_t *)ptr;
    uint32_t name_length = strlen((const char *)node_name) + 1; /* 包含'\0'*/
    /* null节点名称命名*/
    if (*node_name == '\0')
    {
        node_name = (uint8_t *)"null";
    }
    /* 更新ptr*/
    ptr += name_length;
    /* 四字节对齐(字符串会破坏对齐规则)*/
    ptr = (uint8_t *)ADDRALIGN((uint64_t)ptr, 4);

    /* 打印node*/
    early_printf("       [Node]%s:\n", node_name);
    early_printf("         <Parent Node>:%s\n", parent);

    /* 解析属性和子节点*/
    while (ptr < dtb_end)
    {
        token = get_big_endian_data(ptr, sizeof(uint32_t));

        /* 更新ptr*/
        ptr += 4;

        switch (token)
        {
        case FDT_PROP:
        {
            uint8_t *prop_start = ptr - 4;
            FDT_Node_Property *fdt_prop = (FDT_Node_Property *)prop_start;
            /* 获取属性字段*/
            fdt_prop->len = get_big_endian_data((uint8_t *)fdt_prop + 4, sizeof(uint32_t));
            fdt_prop->nameoff = get_big_endian_data((uint8_t *)fdt_prop + 8, sizeof(uint32_t));
            /* 获取属性名*/
            uint8_t *prop_name = fdt_get_string(fdt_head, fdt_prop->nameoff);
            /* 属性数据*/
            uint8_t *prop_data = (uint8_t *)fdt_prop + sizeof(FDT_Node_Property);
            uint64_t _start = get_big_endian_data(prop_data, sizeof(uint64_t));
            uint64_t _size = get_big_endian_data(prop_data + 8, sizeof(uint64_t));
            /* 打印属性信息*/
            early_printf("         <Prop>%s:", prop_name);
            /* 填充space*/
            for (uint8_t i = 0; i < prop_width - strlen((const char *)prop_name); i++)
            {
                early_printf(" ");
            }
            early_printf("Start'0x%016X'  Size'0x%016x'\n",_start, _size);
            /* 处理特定属性:全局内存布局*/
            if (strcmp((const char *)node_name, "memory@80000000") == 0 && strcmp((const char *)prop_name, "reg") == 0)
            {
                mem_info.start = _start;
                mem_info.size = _size;
            }
            /* 更新ptr*/
            ptr += sizeof(FDT_Node_Property) + fdt_prop->len - 4;
            /* 四字节对齐*/
            ptr = (uint8_t *)ADDRALIGN((uint64_t)ptr, 4);
            break;
        }
        case FDT_BEGIN_NODE:
        {
            ptr = dtb_node_parse(fdt_head, ptr - 4, node_name, depth + 1);
            break;
        }
        case FDT_END_NODE:
        {
            return ptr;
        }
        case FDT_END:
        {
            return ptr;
        }
        default:
        {
            early_printf("[Error]: Expected token 0x%08X\n", token);
            return NULL;
        }
        }
    }
    return NULL;
}
/**
 * @brief 解析保留内存表(Reserved Memory Map)
 *
 * @param fdt_head
 */
static void dtb_mem_rsvmap_parse(FDT_Head *fdt_head)
{
    uint8_t index = 0;
    /* rsvmp的起始地址*/
    uint8_t *mem_rsvmap = (uint8_t *)fdt_head + fdt_head->mem_rsvmap_offset;

    early_printf("[JaeOS]Reserved Memory Map:\n");
    while (1)
    {
        uint64_t address = get_big_endian_data(mem_rsvmap, sizeof(uint64_t));
        uint64_t size = get_big_endian_data(mem_rsvmap + 8, sizeof(uint64_t));

        early_printf("       [%2d]Reserved Physical Memory Start at 0x%016lX, Size 0x%lX\n", index++, address, size);
        if (address == 0 && size == 0)
        {
            break;
        }
        mem_rsvmap += 16;
    }
}
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

    /* 解析设备树头*/
    dtb_head_parse(fdt_h);

    /* 解析保留内存*/
    dtb_mem_rsvmap_parse(fdt_h);

    /* 获取设备树节点结构的起始地址*/
    uint8_t *struct_node = (uint8_t *)(fdt_h) + fdt_h->dt_struct_offset;
    /* 递归解析设备树节点*/
    early_printf("[JaeOS]DTB Struct Node:\n");
    dtb_node_parse(fdt_h, struct_node, (uint8_t *)"/", 0);

    /* 打印全局内存信息*/
    early_printf("[JaeOS]Memory Info:\n");
    early_printf("       Start: 0x%016lX, Size:%lu MB\n", mem_info.start, mem_info.size / 1024 / 1024);
    early_printf("[JaeOS]DTB Parse Successful.\n");
}