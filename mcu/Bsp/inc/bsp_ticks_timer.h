/**
* Copyright (C), Goome Technologies Co., Ltd. 2009-2019. All rights reserved.
* File name : bsp_ticks_timer.h
* Author:  Version:	Date:
* Description: bsp_ticks_timer.c 的头文件
* Function List:
* History:
  1.Date: 2019/4/9
    Author: Chris.Lu
    Modification: Created file
*/
#ifndef __BSP_TICKS_TIMER_H__
#define __BSP_TICKS_TIMER_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

extern void bsp_ticks_timer_enter_lpm_init(void);
extern void bsp_ticks_timer_exit_lpm_init(void);
extern void bsp_ticks_timer_power_up_init(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __BSP_TICKS_TIMER_H__ */
