#ifndef __DEV_TIMER__H__
#define __DEV_TIMER__H__

#include "common/types.h"
#include "common/platform.h"

#define JAEOS_TIME_FREQ (1000ul)                                 /* 1KHz*/
#define JAEOS_TIME_CYCLES (QEMU_VIRT_CPU_FREQ / JAEOS_TIME_FREQ) /* 10000cycles: 1ms*/

/* functions*/
void timer_init(void);
void timer_interrupt_handler(void);
#endif /* !__DEV_TIMER__H__*/