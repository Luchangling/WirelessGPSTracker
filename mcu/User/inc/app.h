/**
* Copyright (C), Goome Technologies Co., Ltd. 2009-2019. All rights reserved.
* File name : app.h
* Author:  Version:	Date:
* Description: app.c 相关
* Function List:
* History:
  1.Date: 2019/4/9
    Author: Chris.Lu
    Modification: Created file
*/
#ifndef __APP_H__
#define __APP_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#include "app_rtc_alarm.h"
#include "gm_type.h"

typedef enum
{
	EVENT_POWER_UP = 0,//上电初始化

	EVENT_WAIT_LPM,    //等待进入低功耗

    EVENT_PREPARE_ENTER_LPM, //准备进入低功耗

	EVENT_ENTER_LPM,   //进入低功耗

	EVENT_RTC_WKP,     //RTC中断事件

	EVENT_EXTI1_WKP,   //外部中断1 事件

	EVENT_EXTI2_WKP,   //外部中断2 事件

	EVENT_EXIT_LPM     //退出低功耗
	
}SYS_EVENT_ENUM;





typedef struct
{
	u8 event;

	u8 wkp_reason;

	u8 exti1_trigger;

	u8 exti2_trigger;

	u8 calibration_tim_flg;

	union
	{
		u8 server_tim[6]; /*BCD时间服务器校时*/
		struct
		{
			u8 year : 8;
			u8 mon  : 8;
			u8 day  : 8;
			u8 hour : 8;
			u8 min  : 8;
			u8 sec  : 8;
		};
	};

	union
	{
		u8 rtc_tim[7]; /*BCD时间 RTC时间*/
		struct
		{
			u8 rtc_year : 8;
			u8 rtc_mon  : 8;
			u8 rtc_day  : 8;
			u8 rtc_hour : 8;
			u8 rtc_min  : 8;
			u8 rtc_sec  : 8;
			u8 rtc_week_day :8;
		};
	};
	
	union
	{
		u8 next_tim[7];
		struct
		{
			u8 next_year : 8;
			u8 next_mon  : 8;
			u8 next_day  : 8;
			u8 next_hour : 8;
			u8 next_min  : 8;
			u8 next_sec  : 8;
			u8 next_week_day : 8;
		};
	};

	u8 force_work;    /*强制工作*/

	u8 wait_to_sleep; /*等待进入休眠*/

	u8 flash_commit;

	u8 set_fram_default;

	u32 evt_hold_tim;

	u32 evt_hold_max_time;

}system_event_control_struct;


extern u8 send_event_to_main_hadle(SYS_EVENT_ENUM event);
extern void set_wkp_reason(SYS_EVENT_ENUM ev);
extern void set_evt_100ms_flg(void);
extern SYS_EVENT_ENUM get_system_event(void);

extern u8 is_need_change_event_to_lpm(void);

extern void app_enter_lpm_init(void);
extern void app_wait_lpm_process(void);
extern void app_exit_lpm_init(void);
extern void app_power_up_init(void);
extern u32 get_evt_control_register_addr(void);
extern u16 get_evt_control_register_size(void);

extern void exit_lpm_from_exti1_process(void);

extern void exit_lpm_from_exti2_process(void);

extern void exit_lpm_from_rtc_alarm_process(void);

extern void app_exit_lpm_l2_init(void);

extern FAR system_event_control_struct g_evt_ctrl;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __APP_H__ */
