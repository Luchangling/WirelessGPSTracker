/**
* Copyright (C), Goome Technologies Co., Ltd. 2009-2019. All rights reserved.
* File name : app_ticks_timer.c
* Author:  Version:	Date:
* Description:系统定时器
* Function List:
* History:
  1.Date: 2019/4/11
    Author: Chris.Lu
    Modification: Created file
*/
#include "app.h"

#include "app_rtc_alarm.h" 

#include "system_ticks.h"

void system_timer_ticks_per_1000ms(void)
{
	//app_rtc_clock_statistics();
}

void system_timer_ticks_per_100ms(void)
{
}

void system_timer_ticks_per_10ms(void)
{
    #if 1

    system_ticks_statistics_process();
    
    #else
	static u8  per_100ms = 0;

	static u8  per_1000ms = 0;

	
	
	per_100ms++;

	if(per_100ms  >= 10)
	{
		system_timer_ticks_per_100ms();
		
		per_100ms = 0;	

		per_1000ms++;

		if(per_1000ms >= 10)
		{
			per_1000ms = 0;

			system_timer_ticks_per_1000ms();
		}
	}
    #endif
}
