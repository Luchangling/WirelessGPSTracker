/**
    功能模块:计算下个时间点
*/

#include "gm_type.h"
#include "expmu.h"
#include "config_service.h"
#include "calc_wkp_time.h"
#include "expmu_service.h"
#include "utility.h"

#define ONE_DAY_TOTAL_SEC ((u32)24*(u32)3600)

#define ONE_WEEK_TOTAL_SEC (ONE_DAY_TOTAL_SEC*(u32)7)

#define DEVICE_NEED_SLEEP_TIME  ((u32)3*(u32)60)


static u32 UTC_TO_LOCAL_SEC(u32 cur)
{
    u8 zone;

    u32 adj_sec = 0;

    config_service_get(CFG_TIME_ZONE,TYPE_BYTE,&zone,sizeof(u8));

    adj_sec = (u32)(zone & 0x7F)*3600;

    if(zone&0x80)
    {
        cur -= adj_sec;
    }
    else
    {
        cur += adj_sec;
    }

    return cur;
}


static u32 LOCAL_TO_UTC_SEC(u32 cur)
{
    u8 zone;

    u32 adj_sec = 0;

    config_service_get(CFG_TIME_ZONE,TYPE_BYTE,&zone,sizeof(u8));

    adj_sec = (u32)(zone & 0x7F)*3600;

    if(zone&0x80)
    {
        cur += adj_sec;
    }
    else
    {
        cur -= adj_sec;
    }

    return cur;
}

static void unified_set_poastion_source(u8 source)
{
    u8 mode = source;

    config_service_set(CFG_POSITION_SOURCE,TYPE_BYTE,&mode,sizeof(u8));
}

/*****************************************************************************
 函 数 名  : calc_error_mode_time_point
 功能描述  : 错误模式,每天12点半上线
 输入参数  : u32 curtime  
 输出参数  : 无
 返 回 值  : 
 日    期  : 2019年3月21日
 作    者  : Chris.Lu
*****************************************************************************/
static u32 calc_error_mode_time_point(u32 curtime)
{
	u32 next_time = 0;

	next_time = ((UTC_TO_LOCAL_SEC(curtime))/ONE_DAY_TOTAL_SEC)*ONE_DAY_TOTAL_SEC + 12*3600 + 30*60;

	next_time += ONE_DAY_TOTAL_SEC;

	unified_set_poastion_source(PLATFORM_LOC_MODE_INVAILD);

	return LOCAL_TO_UTC_SEC(next_time);//next_time;
}

/*****************************************************************************
 函 数 名  : calc_alarm_clock_mode_time_point
 功能描述  : 计算最近得闹钟时间点，返回UTC时间
 输入参数  : u32 curtime  
 输出参数  : 无
 返 回 值  : 
 日    期  : 2019年3月21日
 作    者  : Chris.Lu
*****************************************************************************/
static u32 calc_alarm_clock_mode_time_point(u32 curtime)
{
	ClockModeTimePointStruct *clk = NULL;

	u8 i = 0; 

	u32 cur_sec = (UTC_TO_LOCAL_SEC(curtime))%ONE_DAY_TOTAL_SEC;

	u32 clock_sec = 0;

	u32 next_time = 0xffffffff;

	clk = get_cur_gw_work_mode(NULL);


	for(i = 0 ; i < 4 ; i++)
	{
		/*以出现非法的时间点为截止*/
		if((clk->time_point[i]/100 >= 24)||(clk->time_point[i]%100 >= 60)) break;
		
		clock_sec = (clk->time_point[i]/100)*3600 + (clk->time_point[i]%100)*60;
		
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
		/*时区时间*/
		next_time += (((UTC_TO_LOCAL_SEC(curtime))/ONE_DAY_TOTAL_SEC)*ONE_DAY_TOTAL_SEC);
	}
	
	
	/*返回UTC时间*/
	return LOCAL_TO_UTC_SEC(next_time);

	
}

/*****************************************************************************
 函 数 名  : calc_track_mode_next_sleep_time
 功能描述  : 需要休眠的话，返回UTC时间。不需要休眠返回0
 输入参数  : u32 curtime  
 输出参数  : 无
 返 回 值  : 
 日    期  : 2019年3月21日
 作    者  : Chris.Lu
*****************************************************************************/
u32 calc_track_mode_next_sleep_time(u32 curtime)
{
	TrackeModeTimeValueStruct *track = NULL;

	u32 next_time = 0;

	track = (TrackeModeTimeValueStruct *)get_cur_gw_work_mode(NULL);

	if(track->interval >= TRACK_MODE_NEED_INTO_SLEEP_INTERVAL)
	{
		next_time = curtime + track->interval*60;

	}

	return next_time;
}

static u32 week_day_to_utc_time(u8 week_day,u32 time_point ,u32 cur_sec)
{
	u32 curtime = cur_sec;

	u8 cnt = 0,cur_day = 0;

	cur_day = (((UTC_TO_LOCAL_SEC(curtime))%ONE_WEEK_TOTAL_SEC)/ONE_DAY_TOTAL_SEC + 4);
			
	if(cur_day > 7)
    {
		cur_day = cur_day%7;
	}
    
	cnt = week_day < cur_day?(week_day + 7 - cur_day):(week_day - cur_day);
				
	curtime = ((UTC_TO_LOCAL_SEC(curtime))/ONE_DAY_TOTAL_SEC + cnt)*ONE_DAY_TOTAL_SEC + (time_point/100)*3600 + (time_point%100)*60;
				
	return LOCAL_TO_UTC_SEC(curtime);
}


/*****************************************************************************
 函 数 名  : calc_week_mode_next_sleep_time
 功能描述  : 计算星期模式下次唤醒时间,注意:返回值为时间点，不是UTC值
 			 week_day == 0非法的数据模式 
 输入参数  : u32 curtime   
             u8 *week_day  
 输出参数  : 无
 返 回 值  : 
 日    期  : 2019年3月21日
 作    者  : Chris.Lu
*****************************************************************************/
u32 calc_week_mode_next_sleep_time(u32 curtime , u8 *week_day)
{
	WeekkModeTimePointStruct *week = NULL;

	u8  i = 0 ,cur_day = 0 ,next_day = 0;

	u8 day_arry[7] = {0};
	
	u32 week_time = 0 ,cur_clock  = 0,next_time = 0;

	u32 j = 10 ,k = 1;

	week = (WeekkModeTimePointStruct *)get_cur_gw_work_mode(NULL);
	
	/*小于特征值*/
	if(week->day<= 7654321)
	{
		for(i = 0;i < 7 ; i++)
		{
			day_arry[i] = ((week->day%j)/k);

			if((day_arry[i] == 0)||(day_arry[i] > 7))break;
		

			j *= 10;

			k *= 10;
			
			
		}

		cur_day = (((UTC_TO_LOCAL_SEC(curtime))%ONE_WEEK_TOTAL_SEC)/ONE_DAY_TOTAL_SEC + 4);
		
		cur_clock = (UTC_TO_LOCAL_SEC(curtime))%ONE_DAY_TOTAL_SEC;
		
		week_time = (week->time_point/100)*3600 + (week->time_point%100)*60;
		
		
		if(cur_day > 7)
		{
			cur_day = cur_day%7;
		}
		
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
			/*置为非法*/
			next_day = 0;
			/*12:30*/
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
		/*置为非法*/
		next_day = 0;
		/*12:30*/
		next_time = calc_error_mode_time_point(curtime);
	}

	*week_day = next_day;
    
	if(next_day == 0) return next_time;
    
	return week_day_to_utc_time(next_day,next_time,curtime);
}
/*****************************************************************************
 函 数 名  : calc_plat_loop_time_point
 功能描述  : 平台循环模式时间点判断。返回UTC  0:设备不休眠
 输入参数  : u32 curtime  
 输出参数  : 无
 返 回 值  : 
 日    期  : 2019年3月21日
 作    者  : Chris.Lu
*****************************************************************************/
u32 calc_plat_loop_mode_time_point(u32 curtime)
{
	PlatLoopModeStruct *loop = NULL;

	u32 next_time = 0;

	loop = (PlatLoopModeStruct *)get_cur_gw_work_mode(NULL);

    unified_set_poastion_source(PLATFORM_LOC_MODE_INVAILD);

	if(loop->loop_start_tim > curtime)
	{
		if(loop->loop_start_tim - curtime > ONE_WEEK_TOTAL_SEC)
		{
			/*今天的12:30*/
			next_time = calc_error_mode_time_point(curtime);

			return next_time;
		}
		else if(loop->loop_start_tim - curtime > DEVICE_NEED_SLEEP_TIME)
		{
			next_time = loop->loop_start_tim - 120;

			unified_set_poastion_source(loop->pos_mode);

			return next_time;
		}
	}

	if((loop->loop_interval < 6)&&(loop->loop_interval > 0))
	{
		/*无停止时间*/
		if((loop->loop_stop_tim == 0)||((curtime + loop->loop_interval*60) < loop->loop_stop_tim))
		{
			unified_set_poastion_source(loop->pos_mode);
			
			return 0;
		}
		else
		{
			/*今天的12:30*/
			next_time = calc_error_mode_time_point(curtime);
		}
		
	}
	else /*需要休眠*/
	{
		/*无停止时间，或者未到停止时间*/
		if((loop->loop_interval > 0)&&((loop->loop_stop_tim == 0)||((curtime + loop->loop_interval*60) < loop->loop_stop_tim)))
		{
			next_time = loop->loop_start_tim;
			
			while(next_time  < (curtime + DEVICE_NEED_SLEEP_TIME))
			{
				next_time += loop->loop_interval*60;
			}

			unified_set_poastion_source(loop->pos_mode);
		}
		else
		{
			/*今天的12:30*/
			next_time = calc_error_mode_time_point(curtime);
		}
	}

	return next_time;

}

/*****************************************************************************
 函 数 名  : calc_plat_time_point_mode_time_point
 功能描述  : 时间点模式返回UTC时间，时间点轮换完，每天12点半上线
 输入参数  : u32 curtime  
 输出参数  : 无
 返 回 值  : 
 日    期  : 2019年3月21日
 作    者  : Chris.Lu
*****************************************************************************/
u32 calc_plat_time_point_mode_time_point(u32 curtime)
{
	PlatTimePiontModeStruct *tp = NULL;

	u8 i = 0;

	u32 next_time = 0xFFFFFFFF;

	tp = (PlatTimePiontModeStruct *)get_cur_gw_work_mode(NULL);

	for(i = 0; i < 3 ; i++)
	{
		if(tp[i].utc_time_point == 0) continue;

		if(tp[i].utc_time_point > (curtime + DEVICE_NEED_SLEEP_TIME))
		{
			if(next_time > tp[i].utc_time_point)
			{
				next_time = tp[i].utc_time_point;

				unified_set_poastion_source(tp->pos_mode);
			}
			
		}
		
	}

	if((next_time == 0xFFFFFFFF)||(next_time - curtime  > ONE_WEEK_TOTAL_SEC))
	{
		next_time = calc_error_mode_time_point(curtime);
	}

	return next_time;
}

u32 recalc_next_wkp_time(u32 cur_time ,u32 first_next_time)
{
    u16 rep_gap = 0;

    u32 next_time = 0;

    config_service_get(CFG_REP_GAP,TYPE_SHORT,&rep_gap,sizeof(u16));

    next_time = (cur_time + (u32)rep_gap*60ul);
    
    if(first_next_time < next_time)
    {
        next_time = 0;
    }

    return next_time;
}


u32 get_next_wake_up_time_point(u8 *week_day  ,u32 curtime)
{
    u32 next_time = 0;
    
    u32 cur_time = curtime;

    u8 work_mode = 0;

    //u8 bcdtime[6]= {0};

    if(cur_time%60) cur_time+=60;

    config_service_get(CFG_WORK_MODE,TYPE_BYTE,&work_mode,sizeof(u8));

    switch (work_mode)
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
    /*防止发到MCU的数据非法*/
    if(next_time > 0)
    {
        if((next_time - cur_time) < 120)
        {
            if(cur_time%60 >= 20)
            {
                /*延迟60s唤醒*/
                next_time += 60;
            }
        }
    }

    return next_time;
}


