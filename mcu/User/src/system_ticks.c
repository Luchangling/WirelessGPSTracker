/**
* Copyright (C), Goome Technologies Co., Ltd. 2009-2019. All rights reserved.
* File name : system_ticks.c
* Author:  Version:	Date:
* Description:获取和计算系统ticks
* Function List:
* History:
  1.Date: 2019/4/9
    Author: Chris.Lu
    Modification: Created file
*/
#include "gm_type.h"

static u32 s_system_ticks = 0;

static u16 s_system_sec = 0;

void system_ticks_statistics_power_up_init(void)
{
	s_system_ticks = 0;

    s_system_sec = 0;
}

void system_ticks_statistics_enter_lpm_init(void)
{
	s_system_ticks = 0;

    s_system_sec = 0;
}

void system_ticks_statistics_exit_lpm_init(void)
{
	s_system_ticks = 0;

    s_system_sec = 0;
}

void system_ticks_statistics_process(void)
{
	s_system_ticks++;

    if((s_system_ticks != 0) && (s_system_ticks%100 == 0))
    {
        s_system_sec++;
        
    }
}

/*per 10ms*/
u32 get_system_ticks(void)
{
	return s_system_ticks;
}

u16 clock(void)
{    
    return s_system_sec;
}

