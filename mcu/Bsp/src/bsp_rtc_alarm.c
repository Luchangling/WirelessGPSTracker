/**
* Copyright (C), Goome Technologies Co., Ltd. 2009-2019. All rights reserved.
* File name : bsp_rtc.c
* Author:  Version:	Date:
* Description:RTC时钟相关
* Function List:
* History:
  1.Date: 2019/4/9
    Author: Chris.Lu
    Modification: Created file
*/
#include "stm8l15x_clk.h"
#include "gm_type.h"
#include "bsp_watch_dog.h"
#include "stm8l15x_rtc.h"
#include "time.h"

static RTC_AlarmTypeDef  s_rtc_alarm_str;
static RTC_AlarmTypeDef  s_cur_rtc_alarm;

void bsp_rtc_power_up_init(void)
{
    u32 dwWaitOverTimeCnt = 0;

	RTC_InitTypeDef RTC_InitStr;
    
    CLK_LSEConfig(CLK_LSE_ON);
    /* Wait for LSE clock to be ready */
    while ( (CLK_GetFlagStatus(CLK_FLAG_LSERDY) == RESET) && (++dwWaitOverTimeCnt < 5000) )
    {
        delayms(2);//about 2ms
    }
    dwWaitOverTimeCnt = 0; 
    /* wait for 1 second for the LSE Stabilisation */
	delayms((u32)1000);
    
    /* Select LSE (32.768 KHz) as RTC clock source */
    CLK_RTCClockConfig(CLK_RTCCLKSource_LSE, CLK_RTCCLKDiv_1);
    
    CLK_PeripheralClockConfig(CLK_Peripheral_RTC, ENABLE);

    RTC_InitStr.RTC_HourFormat = RTC_HourFormat_24;
    RTC_InitStr.RTC_AsynchPrediv = 0x7F;
    RTC_InitStr.RTC_SynchPrediv = 0x00FF;
    RTC_Init(&RTC_InitStr); 
    RTC_WaitForSynchro();      
}

void bsp_rtc_enter_lpm_init(void)
{
	
}

void bsp_rtc_exit_lpm_init(void)
{
	
}

void bsp_get_cur_rtc_time(RTC_DateTypeDef *date,RTC_TimeTypeDef *time,RTC_Format_TypeDef format)
{
    static u16 timeout = 0;

	timeout = 0xFFFE;
	
    while (RTC_WaitForSynchro() != SUCCESS)
    {
		feed_dog();
		
        timeout--;
        if (timeout == 0)
        {
            break;
        }
    }        
    /* Get the current Time*/
    RTC_GetDate(format, date);
	
    RTC_GetTime(format, time);

}

void bsp_set_rtc_time(RTC_DateTypeDef *date,RTC_TimeTypeDef *time,RTC_Format_TypeDef format)
{
	RTC_SetDate(format, date);

	RTC_SetTime(format, time);
}

void bsp_update_alarm_rtc_time(u16 clock , u8 week_day,RTC_Format_TypeDef format)
{
    RTC_AlarmCmd(DISABLE);
	
    RTC_ITConfig(RTC_IT_ALRA, DISABLE);

	RTC_AlarmStructInit(&s_rtc_alarm_str);
	
	if((week_day >= 1)&&(week_day <= 7))
	{
		s_rtc_alarm_str.RTC_AlarmDateWeekDaySel = RTC_AlarmDateWeekDaySel_WeekDay;
		
        s_rtc_alarm_str.RTC_AlarmDateWeekDay = (week_day & 0x0F);
		
		s_rtc_alarm_str.RTC_AlarmMask = RTC_AlarmMask_None;
	}
	else
	{
		s_rtc_alarm_str.RTC_AlarmMask = RTC_AlarmMask_DateWeekDay;
	}

	s_rtc_alarm_str.RTC_AlarmTime.RTC_Hours = clock >> 8;
	
    s_rtc_alarm_str.RTC_AlarmTime.RTC_Minutes = clock & 0xFF;
	
    s_rtc_alarm_str.RTC_AlarmTime.RTC_Seconds = 0;
	
    s_rtc_alarm_str.RTC_AlarmTime.RTC_H12= RTC_H12_AM;

	RTC_SetAlarm(format, &s_rtc_alarm_str);

    RTC_ITConfig(RTC_IT_ALRA, ENABLE);
	
    RTC_AlarmCmd(ENABLE);
}

RTC_AlarmTypeDef *bsp_get_alarm_rtc_time(void)
{
	RTC_GetAlarm(RTC_Format_BCD, &s_cur_rtc_alarm);

	return &s_cur_rtc_alarm;
}

