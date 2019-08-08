
#ifndef __CALAC_WKP_TIME_H__
#define  __CALAC_WKP_TIME_H__

/*计算下次唤醒时间点 UTC*/
u32 get_next_wake_up_time_point(u8 *week_day  ,u32 curtime);

/*重新计算下个时间点*/
u32 recalc_next_wkp_time(u32 cur_time ,u32 first_next_time);


#endif

