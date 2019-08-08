/**
* Copyright (C), Goome Technologies Co., Ltd. 2009-2019. All rights reserved.
* File name : time.h
* Author:  Version:	Date:
* Description: time.c 的头文件
* Function List:
* History:
  1.Date: 2019/4/10
    Author: Chris.Lu
    Modification: Created file
*/
#ifndef __TIME_H__
#define __TIME_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#include "gm_type.h"

typedef u32 time_t;

struct tm {
	u32 tm_weekday; /*星期 几*/
    u32 tm_sec; /* 秒 ? 取值区间为[0,59] */
    u32 tm_min; /* 分 - 取值区间为[0,59] */
    u32 tm_hour; /* 时 - 取值区间为[0,23] */
    u32 tm_mday; /* 一个月中的日期 - 取值区间为[1,31] */
    u32 tm_mon; /* 月份（从一月开始，0代表一月） - 取值区间为[0,11] */
    u32 tm_year; /* 年份，其值等于实际年份减去1900 */
};

extern u32 isleap(u32 year);

extern time_t mktime(struct tm dt);

extern u8 date_trans_to_week_day(u8 date,u8 month,u16 year);

extern u32 datetosecond_base2000(const u8 *bcd_data);

extern u32 goome_utc_timer_sec_to_bcd_base2000(u32 utc_sec, u8* rtc_bcd);

extern void delayms(__IO u32 ms);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __TIME_H__ */
