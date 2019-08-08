/**
* Copyright (C), Goome Technologies Co., Ltd. 2009-2019. All rights reserved.
* File name : system_ticks.h
* Author:  Version:	Date:
* Description: system_ticks.c 的头文件
* Function List:
* History:
  1.Date: 2019/4/9
    Author: Chris.Lu
    Modification: Created file
*/
#ifndef __SYSTEM_TICKS_H__
#define __SYSTEM_TICKS_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

extern uint32_t get_system_ticks(void);
extern void system_ticks_statistics_enter_lpm_init(void);
extern void system_ticks_statistics_exit_lpm_init(void);
extern void system_ticks_statistics_power_up_init(void);
extern void system_ticks_statistics_process(void);
extern u16 clock(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __SYSTEM_TICKS_H__ */
