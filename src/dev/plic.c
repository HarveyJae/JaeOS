#include "common/types.h"
#include "dev/plic.h"
/**
 * @brief RV64的中断控制器初始化
 * 
 * @param hart_id 
 */
void plic_init(uint64_t hart_id)
{
    /* 使能VirtIO_0中断*/
    uint32_t *senable_reg = (uint32_t *)PLIC_SENABLE(hart_id);
    *senable_reg = (1 << VIRTIO_0_IR_ID);

	/* 设置中断优先级阈值寄存器*/
    uint32_t *spriority_reg = (uint32_t *)PLIC_SPRIORITY(hart_id);
    *spriority_reg = 0;
}
/**
 * @brief 向PLIC索要当前的中断源ID
 * 
 * @param hart_id 
 * @return uint32_t 
 */
uint32_t plic_claim(uint64_t hart_id) 
{
    uint32_t *sclaim_reg = (uint32_t *)PLIC_SCLAIM(hart_id);
    return *sclaim_reg;
}
/**
 * @brief 告知plic已经处理完了对应中断
 * 
 * @param interrupt_id 处理完的中断源ID
 * @param hart_id 
 */
void plic_complete(uint32_t interrupt_id, uint64_t hart_id) 
{
    uint32_t *sclaim_reg = (uint32_t *)PLIC_SCLAIM(hart_id);
    *sclaim_reg = interrupt_id;
}