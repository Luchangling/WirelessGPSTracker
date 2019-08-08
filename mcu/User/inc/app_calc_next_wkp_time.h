
#ifndef __APP_CALC_NEXT_WKP_TIME__
#define __APP_CALC_NEXT_WKP_TIME__

#include "gm_type.h"
#include "app_fram_manger.h"


typedef struct
{
	u32 day;
	u32 time_point;

}WeekkModeTimePointStruct;

typedef struct
{
	u32 time_point[4];
}ClockModeTimePointStruct;

typedef struct
{
	u32 interval;
}TrackeModeTimeValueStruct;


typedef struct
{
	u32 pos_mode;
	u32 loop_start_tim;
	u32 loop_interval; /*��λ����*/
	u32 loop_stop_tim;
}PlatLoopModeStruct;

typedef struct
{
	u32 pos_mode;
	u32 utc_time_point;
}PlatTimePiontModeStruct;


typedef union
{
	PlatTimePiontModeStruct plat_tp[3];

	PlatLoopModeStruct plat_loop;

	TrackeModeTimeValueStruct track;

	ClockModeTimePointStruct clock;

	WeekkModeTimePointStruct week;
	
}WorkModeParamUnion;

enum
{
	ALARM_CLOCK_MODE = 1,           //闹钟模式
   
	TRACK_MODE = 3,                 //追踪模式
	WEEK_ALARM_MODE = 4,            //星期模式
	
	PLATFORM_CYCLIC_MODE = 5,       //平台循环模式
	PLATFORM_TIMEPOINT_MODE = 6,    //平台时间点模式

	PLATFORM_MODE = 7, 

	WORK_MODE_INVALID,
};

#define ONE_DAY_TOTAL_SEC ((u32)24*(u32)3600)

#define ONE_WEEK_TOTAL_SEC (ONE_DAY_TOTAL_SEC*(u32)7)

#define DEVICE_NEED_SLEEP_TIME  ((u32)3*(u32)60)

//#define UTC_TO_LOCAL_SEC ((U32)g_Para.zone*3600ul)
#define UTC_TO_LOCAL_SEC(cur) (u32)(g_para.zone&0x80 > 0?((u32)cur - (u32)3600*(u32)(g_para.zone&0x7F)):((u32)cur + (u32)3600*(u32)g_para.zone))

#define LOCAL_TO_UTC_SEC(cur) (u32)(g_para.zone&0x80 > 0?((u32)cur + (u32)3600*(u32)(g_para.zone&0x7F)):((u32)cur - (u32)3600*(u32)g_para.zone))


extern u32 get_next_sleep_time_point(u8 *week_day);

extern u32 week_day_to_utc_time(u8 week_day,u32 time_point);

#endif

