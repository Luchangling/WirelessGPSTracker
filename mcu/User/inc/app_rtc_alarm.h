/**
* Copyright (C), Goome Technologies Co., Ltd. 2009-2019. All rights reserved.
* File name : app_rtc_alarm.h
* Author:  Version:	Date:
* Description: app_rtc_alarm.c 的头文件
* Function List:
* History:
  1.Date: 2019/4/10
    Author: Chris.Lu
    Modification: Created file
*/
#ifndef __APP_RTC_ALARM_H__
#define __APP_RTC_ALARM_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#include "time.h"

extern void app_get_cur_time_value(time_t* time_v);
extern time_t app_get_rtc_clock(void);
extern void app_get_ur_rtc_time(u8 *bcdtim ,u8 *week_day);
extern void app_rtc_alarm_enter_lpm_init(u16 clock , u8 week_day);
extern void app_rtc_alarm_exit_lpm_init(void);
extern void app_rtc_alarm_power_up_init(void);
extern void app_rtc_clock_statistics(void);
extern void app_rtc_time_update(u8 *bcdtim);
extern time_t app_bcd_to_time_value(u8 *bcdtim);
extern void app_get_rtc_alarm_time(u8 *bcd);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __APP_RTC_ALARM_H__ */
