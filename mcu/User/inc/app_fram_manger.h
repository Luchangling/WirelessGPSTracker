/**
* Copyright (C), Goome Technologies Co., Ltd. 2009-2019. All rights reserved.
* File name : app_fram_manger.h
* Author:  Version:	Date:
* Description: app_fram_manger.c 
* Function List:
* History:
  1.Date: 2019/4/22
    Author: Chris.Lu
    Modification: Created file
*/
#ifndef __APP_FRAM_MANGER_H__
#define __APP_FRAM_MANGER_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#include "app_calc_next_wkp_time.h"

#define MAGIC_NUM 0xA5

#define USER_PARA_START_ADDR 0

#define USER_BKP_SATRT_ADDR  512


typedef struct
{
	u8 factory_mark;/*有效标志，作为恢复出厂操作*/

	union
	{
		u8 next_time[7]; /*下次唤醒时间*/
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
	
	u8 exti1_trigger; /*外部中断1触发方式*/

	u8 exti2_trigger; /*外部中断2触发方式*/

    u8 workmode;      /*当前系统的工作模式*/

    u8 mod_param[24]; /*工作模式内容*/

    u8 zone;          /*时区*/

    u8 pwr_up[2];     /*上电次数*/

    u8 rtc_wkp[2];    /*RTC唤醒次数*/

    u8 exit1_wkp[2];  /*外部中断1唤醒次数*/

    u8 exit2_wkp[2];  /*外部中断2唤醒次数*/

    u8 wkp_time[7];   /*当前唤醒时间*/

	u8 sum;           /*和校验*/
	
}Fram_Para_Struct;


extern void app_fram_para_enter_lpm_init(void);

extern void app_fram_para_exit_lpm_init(void);

extern void app_fram_para_powerup_init(void);

extern void app_fram_para_wait_lpm_commit(void);

extern u8 app_fram_read(u16 offset, u8* pdata, u16 len);

extern u8 app_fram_write(u16 offset, const u8* pdata, u16 len);

extern u8 check_para_file_is_vaild(Fram_Para_Struct *para);

extern u8 get_para_sum_vlue(Fram_Para_Struct *para);

extern void set_para_to_default_value(void);

extern void app_para_write_to_fram(void);

extern Fram_Para_Struct g_para;

extern Fram_Para_Struct g_para_bkp;

#define MCU_WKP_STATISTICS(type) ((*(u16 *)(g_para.type))++)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __APP_FRAM_MANGER_H__ */
