/**
* Copyright (C), Goome Technologies Co., Ltd. 2009-2019. All rights reserved.
* File name : app.c
* Author:  Version:	Date:
* Description:
* Function List:
* History:
  1.Date: 2019/4/9
    Author: Chris.Lu
    Modification: Created file
*/

#include "app_rtc_alarm.h"
#include "app_calc_next_wkp_time.h"

u32 week_day_to_utc_time(u8 week_day,u32 time_point)
{
	u32 curtime = 0;

	u8 cnt = 0,cur_day = 0;

    app_get_cur_time_value(&curtime);

	cur_day = ((curtime%ONE_WEEK_TOTAL_SEC)/ONE_DAY_TOTAL_SEC + (u32)4ul);
			
	if(cur_day > 7)
    {
		cur_day = cur_day%7;
	}
    
	cnt = week_day < cur_day?(week_day + 7 - cur_day):(week_day - cur_day);
				
	curtime = (curtime/ONE_DAY_TOTAL_SEC + (u32)cnt)*ONE_DAY_TOTAL_SEC + (time_point/(u32)100ul)*(u32)3600ul + (time_point%(u32)100ul)*(u32)60ul;
				
	return LOCAL_TO_UTC_SEC(curtime);
}


u32 calc_error_mode_time_point(u32 curtime)
{
	u32 next_time = 0;

	next_time = ((UTC_TO_LOCAL_SEC(curtime))/ONE_DAY_TOTAL_SEC)*ONE_DAY_TOTAL_SEC + (u32)12ul*(u32)3600ul + (u32)30ul*(u32)60ul;

	//if(((curtime + UTC_TO_LOCAL_SEC)%ONE_DAY_TOTAL_SEC + DEVICE_NEED_SLEEP_TIME) > (12*3600 + 30*60))
	//{
		next_time += ONE_DAY_TOTAL_SEC;
	//}


	return LOCAL_TO_UTC_SEC(next_time);//next_time;
}


u32 calc_alarm_clock_mode_time_point(u32 curtime)
{
	ClockModeTimePointStruct *clk = NULL;

	u8 i = 0; 

	u32 cur_sec = (UTC_TO_LOCAL_SEC(curtime))%ONE_DAY_TOTAL_SEC;

	u32 clock_sec = 0;

	u32 next_time = 0xffffffff;

	clk = (ClockModeTimePointStruct *)g_para.mod_param;

	for(i = 0 ; i < 4 ; i++)
	{
		
		if((clk->time_point[i]/(u32)100ul >= (u32)24ul)||(clk->time_point[i]%(u32)100ul >= (u32)60ul)) break;
		
		clock_sec = (clk->time_point[i]/(u32)100ul)*(u32)3600ul + (clk->time_point[i]%(u32)100ul)*(u32)60ul;
		
		if(clock_sec < (cur_sec + DEVICE_NEED_SLEEP_TIME))
		{
			clock_sec += ONE_DAY_TOTAL_SEC;
		}

		if(next_time > clock_sec)
		{
			next_time = clock_sec;
		}

	}


	if(next_time == 0xffffffff)
	{
		next_time = calc_error_mode_time_point(curtime);

		return next_time;
	}
	else
	{
		
		next_time += (((UTC_TO_LOCAL_SEC(curtime))/ONE_DAY_TOTAL_SEC)*ONE_DAY_TOTAL_SEC);
	}
	

	return LOCAL_TO_UTC_SEC(next_time);

	
}


u32 calc_track_mode_next_sleep_time(u32 curtime)
{
	TrackeModeTimeValueStruct *track = NULL;

	u32 next_time = 0;


	track = (TrackeModeTimeValueStruct *)g_para.mod_param;

	if(track->interval >= 6)
	{
		next_time = curtime;
		while((next_time - curtime) < DEVICE_NEED_SLEEP_TIME)
		next_time += (u32)track->interval*(u32)60ul;

	}

	return next_time;
}

u32 calc_week_mode_next_sleep_time(u32 curtime , u8 *week_day)
{
	WeekkModeTimePointStruct *week = NULL;

	u8  i = 0 ,cur_day = 0 ,next_day = 0;

	u8 day_arry[7] = {0};
	
	u32 week_time = 0 ,cur_clock  = 0,next_time = 0;

	u32 j = 10 ,k = 1;

	week = (WeekkModeTimePointStruct *)&g_para.mod_param;


	if(week->day<= (u32)7654321ul)
	{
		for(i = 0;i < 7 ; i++)
		{
			day_arry[i] = ((week->day%j)/k);
			
			//printf("\r\n day_arry[%d] = %d,%d,%d\r\n",i,day_arry[i],j,k);	

			if((day_arry[i] == 0)||(day_arry[i] > 7))break;
		

			j *= 10;

			k *= 10;
			
			
		}

		cur_day = (((UTC_TO_LOCAL_SEC(curtime))%ONE_WEEK_TOTAL_SEC)/ONE_DAY_TOTAL_SEC + (u32)4ul);
		
		cur_clock = (UTC_TO_LOCAL_SEC(curtime))%ONE_DAY_TOTAL_SEC;
		
		week_time = ((u32)week->time_point/100ul)*(u32)3600ul + ((u32)week->time_point%100ul)*(u32)60ul;
		
		
		if(cur_day > 7)
		{
			cur_day = cur_day%7;
		}
		
		//next_day = 0;
		
		next_day = 0xff;
		
		for(k = 0 ; k < i ; k++)
		{
			if(day_arry[k] == 0)continue;
	
			if((cur_day > day_arry[k])||((cur_day == day_arry[k])&&(week_time < cur_clock + DEVICE_NEED_SLEEP_TIME)))
			{
				day_arry[k] += 7;
				
			}

			if(next_day > day_arry[k])
			{
				next_day = day_arry[k];
			}	
		}
		
		if(next_day == 0xff)
		{

			next_day = 0;

			next_time = calc_error_mode_time_point(curtime);
		}
		else
		{
			if(next_day > 7) next_day -= 7;
			
			next_time = week->time_point;//mtk_time_to_mcu_bcd_time(week->time_point) ;
		}
	
	}
	else
	{

		next_day = 0;
		/*12:30*/
		next_time = calc_error_mode_time_point(curtime);
	}

	*week_day = next_day;
	
	return next_time;
}


u32 calc_plat_loop_mode_time_point(u32 curtime)
{
	PlatLoopModeStruct *loop = NULL;

	u32 next_time = 0ul;

	loop = (PlatLoopModeStruct *)g_para.mod_param;


	if(loop->loop_start_tim > curtime)
	{
		if(loop->loop_start_tim - curtime > ONE_WEEK_TOTAL_SEC)
		{
			/*非法的时间设置默认值12:30*/
			next_time = calc_error_mode_time_point(curtime);

			return next_time;
		}
		else if(loop->loop_start_tim - curtime > DEVICE_NEED_SLEEP_TIME)
		{
			next_time = loop->loop_start_tim - 120ul;

			return next_time;
		}
	}

	if((loop->loop_interval < 6ul)&&(loop->loop_interval > 0ul))
	{
		/*cannot enter lpm*/
		if((loop->loop_stop_tim == 0ul)||((curtime + loop->loop_interval*60ul) < loop->loop_stop_tim))
		{
			return 0;
		}
		else
		{
			/*12:30*/
			next_time = calc_error_mode_time_point(curtime);
		}
		
	}
	else 
	{
		/*need enter LPM*/
		if((loop->loop_interval > 0ul)&&((loop->loop_stop_tim == 0ul)||((curtime + loop->loop_interval*60ul) < loop->loop_stop_tim)))
		{
			next_time = loop->loop_start_tim;
			
			while(next_time  < (curtime + DEVICE_NEED_SLEEP_TIME))
			{
				next_time += loop->loop_interval*60ul;
			}

		}
		else
		{
			/*12:30*/
			next_time = calc_error_mode_time_point(curtime);
		}
	}

	return next_time;

}

u32 calc_plat_time_point_mode_time_point(u32 curtime)
{
	PlatTimePiontModeStruct *tp = NULL;

	u8 i = 0;

	u32 next_time = 0xFFFFFFFF;

	tp = (PlatTimePiontModeStruct *)g_para.mod_param;

	for(i = 0; i < 3 ; i++)
	{
		if(tp[i].utc_time_point == 0) continue;

		if(tp[i].utc_time_point > (curtime + DEVICE_NEED_SLEEP_TIME))
		{
			if(next_time > tp[i].utc_time_point)
			{
				next_time = tp[i].utc_time_point;

			}
			
		}
		
	}

	if((next_time == 0xFFFFFFFF)||(next_time - curtime  > ONE_WEEK_TOTAL_SEC))
	{
		next_time = calc_error_mode_time_point(curtime);
	}

	return next_time;
}


u32 get_next_sleep_time_point(u8 *week_day)
{
	u32 next_time = 0;
    time_t cur_time = 0;
	
	app_get_cur_time_value(&cur_time);

    cur_time = LOCAL_TO_UTC_SEC(cur_time);


	if(cur_time%60) cur_time+=60;

    *week_day = 0;

	switch (g_para.workmode)
	{
		case ALARM_CLOCK_MODE:
		{
			next_time = calc_alarm_clock_mode_time_point(cur_time);
			
			break;
		}
		case TRACK_MODE:
		{
			next_time = calc_track_mode_next_sleep_time(cur_time);
			
			break;
		}
		case WEEK_ALARM_MODE:
		{
			next_time = calc_week_mode_next_sleep_time(cur_time,week_day);
			
			break;
		}
		case PLATFORM_CYCLIC_MODE:
		{
			next_time = calc_plat_loop_mode_time_point(cur_time);
			
			break;
		}
		case PLATFORM_TIMEPOINT_MODE:
		{
			next_time = calc_plat_time_point_mode_time_point(cur_time);
			
			break;
		}
		default:
		{
			next_time = calc_error_mode_time_point(cur_time);
			
			break;
		}
		
	}

	if((next_time > 0)&&(*week_day == 0))
	{
		if((next_time - cur_time) < 120)
		{
			if(cur_time%60 >= 20)
			{
				/*往后推迟*/
				next_time += 60;
			}
		}
	}

	return next_time;
}



