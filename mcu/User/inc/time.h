/**
* Copyright (C), Goome Technologies Co., Ltd. 2009-2019. All rights reserved.
* File name : time.h
* Author:  Version:	Date:
* Description: time.c ��ͷ�ļ�
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
	u32 tm_weekday; /*���� ��*/
    u32 tm_sec; /* �� ? ȡֵ����Ϊ[0,59] */
    u32 tm_min; /* �� - ȡֵ����Ϊ[0,59] */
    u32 tm_hour; /* ʱ - ȡֵ����Ϊ[0,23] */
    u32 tm_mday; /* һ�����е����� - ȡֵ����Ϊ[1,31] */
    u32 tm_mon; /* �·ݣ���һ�¿�ʼ��0����һ�£� - ȡֵ����Ϊ[0,11] */
    u32 tm_year; /* ��ݣ���ֵ����ʵ����ݼ�ȥ1900 */
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
