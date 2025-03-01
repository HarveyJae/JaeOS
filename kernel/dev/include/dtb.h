#ifndef __DEV_DTB__H__
#define __DEV_DTB__H__
#include "common/types.h"

/* 设备树头部结构体定义 */
typedef struct
{
    uint32_t magic;             /* 魔数，固定为0xd00dfeed */
    uint32_t totalsize;         /* DTB总大小 */
    uint32_t dt_struct_offset;  /* 设备树节点结构偏移量 */
    uint32_t dt_strings_offset; /* 设备树字符串表偏移量 */
    uint32_t mem_rsvmap_offset; /* 保留内存区域偏移量 */
    uint32_t version;           /* 设备树版本 */
    uint32_t last_comp_version; /* 兼容版本 */
    uint32_t boot_cpuid_phys;   /* 引导CPU的物理ID */
} FDT_Head;
/* 节点的属性结构 */
typedef struct
{
    uint32_t len;     /* 属性值长度 */
    uint32_t nameoff; /* 属性名在字符串表的偏移 */
} FDT_Node_Property;

/* 全局内存结构 */
typedef struct
{
    uint64_t start;
    uint64_t size;
} MEM_INFO;
/* 设备树标记值 */
#define FDT_MAGIC 0xd00dfeed      /* 设备树头部魔数 */
#define FDT_BEGIN_NODE 0x00000001 /* node起始标记 */
#define FDT_END_NODE 0x00000002   /* node结束标记 */
#define FDT_PROP 0x00000003       /* node中的属性开始标记 */
#define FDT_NOP 0x00000004
#define FDT_END 0x00000009

int32_t dtb_prase(void);
#endif /* !__DEV_DTB__H__ */