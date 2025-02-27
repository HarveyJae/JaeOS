#ifndef __COMMON_PARAM__H__
#define __COMMON_PARAM__H__

/* 该宏定义用来确定硬件平台的CPU核心数量 */
/* 由cmake/make定义，如未定义则报错 */
#ifndef NCPU
#error NCPU not defined
#endif /* !NCPU */


#endif  /* __COMMON_PARAM__H__ */