/**
* Copyright (C), Goome Technologies Co., Ltd. 2009-2019. All rights reserved.
* File name : app_rtc_alarm.c
* Author:  Version:	Date:
* Description:rtc
* Function List:
* History:
  1.Date: 2019/4/10
    Author: Chris.Lu
    Modification: Created file
*/
#include "stm8l15x_rtc.h"
#include "time.h"
#include "bsp_rtc_alarm.h"
#include "app_rtc_alarm.h"
#include "gm_type.h"

static RTC_DateTypeDef s_date;
static RTC_TimeTypeDef s_time;
 
static struct tm s_tm;
static time_t s_rtc_clock = 0;

#define BCD2HEX(n)              ((((n)>>4)*10) + ((n)&0x0f))  // 0x13 = 10+3
#define HEX2BCD(n)              (((((n)/10)%10)<<4)  +  ((n)%10))


void app_rtc_alarm_power_up_init(void)
{
	u8 bcdtime[6] = {0x19,0x04,0x10,0x17,0x00,0x00};

    delayms(1000);
    #if 0
	u8 bcdtime2[6] = {0};

	u32 tim_v1 = 0;

	u32 tim_v2 = 0;

	tim_v1 = app_bcd_to_time_value(bcdtime);

	app_get_ur_rtc_time(bcdtime2,NULL);

	tim_v2 = app_bcd_to_time_value(bcdtime2);

	if(tim_v2 < tim_v1)
    #endif
    app_rtc_time_update(bcdtime);
	
	s_rtc_clock = 0;
}


void app_rtc_alarm_enter_lpm_init(u16 clock , u8 week_day)
{
	bsp_update_alarm_rtc_time(clock,week_day,RTC_Format_BCD);
}

void app_rtc_alarm_exit_lpm_init(void)
{
	s_rtc_clock = 0;
}


void app_rtc_time_update(u8 *bcdtim)
{
	s_date.RTC_Year    = bcdtim[0];
	s_date.RTC_Month   = (RTC_Month_TypeDef)bcdtim[1];
	s_date.RTC_Date    = bcdtim[2];
	s_date.RTC_WeekDay = (RTC_Weekday_TypeDef)date_trans_to_week_day(BCD2HEX(bcdtim[2]),BCD2HEX(bcdtim[1]),BCD2HEX(bcdtim[0]));

	s_time.RTC_H12     = RTC_H12_AM;
	s_time.RTC_Hours   = bcdtim[3];
	s_time.RTC_Minutes = bcdtim[4];
	s_time.RTC_Seconds = bcdtim[5];
	
	bsp_set_rtc_time(&s_date,&s_time,RTC_Format_BCD);

	s_tm.tm_year = BCD2HEX(bcdtim[0]) + 2000;

	s_tm.tm_mon  = BCD2HEX(bcdtim[1]);

	s_tm.tm_mday = BCD2HEX(bcdtim[2]);

	s_tm.tm_hour = BCD2HEX(bcdtim[3]);

	s_tm.tm_min  = BCD2HEX(bcdtim[4]);

	s_tm.tm_sec  = BCD2HEX(bcdtim[5]);

	s_rtc_clock  = mktime(s_tm);
}

void app_get_ur_rtc_time(u8 *bcdtim ,u8 *week_day)
{
	
	bsp_get_cur_rtc_time(&s_date, &s_time, RTC_Format_BCD);

	if(bcdtim == NULL) return;
	
	bcdtim[0] = (s_date.RTC_Year);
	
	bcdtim[1] = (s_date.RTC_Month);

	bcdtim[2] = (s_date.RTC_Date);

	bcdtim[3] = (s_time.RTC_Hours);

	bcdtim[4] = (s_time.RTC_Minutes);

	bcdtim[5] = (s_time.RTC_Seconds);

	if(week_day == NULL) return;

	*week_day = s_date.RTC_WeekDay;
}


void app_get_cur_time_value(time_t* time_v)
{
	u8 bcdtim[6] = {0};

	app_get_ur_rtc_time(bcdtim,NULL);

	s_tm.tm_year = (u32)BCD2HEX(bcdtim[0]) + (u32)2000ul;

	s_tm.tm_mon  = BCD2HEX(bcdtim[1]);

	s_tm.tm_mday = BCD2HEX(bcdtim[2]);

	s_tm.tm_hour = BCD2HEX(bcdtim[3]);

	s_tm.tm_min  = BCD2HEX(bcdtim[4]);

	s_tm.tm_sec  = BCD2HEX(bcdtim[5]);

	*time_v = mktime(s_tm);
	
}

time_t app_bcd_to_time_value(u8 *bcdtim)
{
	s_tm.tm_year = (u32)BCD2HEX(bcdtim[0]) + (u32)2000ul;

	s_tm.tm_mon  = BCD2HEX(bcdtim[1]);

	s_tm.tm_mday = BCD2HEX(bcdtim[2]);

	s_tm.tm_hour = BCD2HEX(bcdtim[3]);

	s_tm.tm_min  = BCD2HEX(bcdtim[4]);

	s_tm.tm_sec  = BCD2HEX(bcdtim[5]);

	return mktime(s_tm);
}

/*UTC时间戳*/
time_t app_get_rtc_clock(void)
{
	/*特征值,代码编辑北京时间20190410150228*/
	if(s_rtc_clock <= (u32)1554879748ul)  /*非法的时间*/
	{
		app_get_cur_time_value(&s_rtc_clock);
	}

	return s_rtc_clock;
		
}

/**1s调用一次*/
void app_rtc_clock_statistics(void)
{
	s_rtc_clock++;
}

void app_get_rtc_alarm_time(u8 *bcd)
{
	
	RTC_AlarmTypeDef *ra = NULL;
	
	ra = bsp_get_alarm_rtc_time();

	bcd[3] = ra->RTC_AlarmTime.RTC_Hours;

	bcd[4] = ra->RTC_AlarmTime.RTC_Minutes;

	bcd[5] = 0;

	if(ra->RTC_AlarmDateWeekDaySel == RTC_AlarmDateWeekDaySel_WeekDay)
	{
		bcd[6] = ra->RTC_AlarmDateWeekDay;
	}
	else
	{
		bcd[6] = 0;
	}
}
	

