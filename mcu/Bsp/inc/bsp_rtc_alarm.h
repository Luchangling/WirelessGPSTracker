/**
* Copyright (C), Goome Technologies Co., Ltd. 2009-2019. All rights reserved.
* File name : bsp_rtc_alarm.h
* Author:  Version:	Date:
* Description: bsp_rtc_alarm.c 的头文件
* Function List:
* History:
  1.Date: 2019/4/9
    Author: Chris.Lu
    Modification: Created file
*/
#ifndef __BSP_RTC_ALARM_H__
#define __BSP_RTC_ALARM_H__
#include "stm8l15x_clk.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

extern void bsp_get_cur_rtc_time(RTC_DateTypeDef *date,RTC_TimeTypeDef *time,RTC_Format_TypeDef format);
extern void bsp_rtc_enter_lpm_init(void);
extern void bsp_rtc_exit_lpm_init(void);
extern void bsp_rtc_power_up_init(void);
extern void bsp_set_rtc_time(RTC_DateTypeDef *date,RTC_TimeTypeDef *time,RTC_Format_TypeDef format);
extern void bsp_update_alarm_rtc_time(u16 clock , u8 week_day,RTC_Format_TypeDef format);
extern RTC_AlarmTypeDef *bsp_get_alarm_rtc_time(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __BSP_RTC_ALARM_H__ */
