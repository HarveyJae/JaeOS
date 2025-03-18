#ifndef __MMU_MMU__H__
#define __MMU_MMU__H__

#define PAGE_SIZE 0x1000 /* 4KB*/
/**
 * @brief 将地址向上对齐到指定边界
 * @param a     原始地址
 * @param align 对齐值（必须为 2 的幂）
 */
#define ADDRALIGN(a, align) (((a) + (align) - 1) & ~((align) - 1))
#endif /* __MMU_MMU__H__*/