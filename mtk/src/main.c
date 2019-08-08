/**
 * Copyright Goome Technologies Co., Ltd. 2009-2019. All rights reserved.
 * File name:        main.c
 * Author:           Chris.Lu     
 * Version:          1.0
 * Date:             2019-06-10
 * Description:      APP Main Function
 * Others:      
 * Function List:    
    1. APP
 * History: 
    1. Date:         2019-06-10
       Author:       Chris.Lu
       Modification: First version
    2. Date: 		 
	   Author:		 
	   Modification: 

 */

#include <gm_type.h>
#include <gm_timer.h>
#include <gm_uart.h>
#include <gm_system.h>
#include "main.h"
#include "error_code.h"
#include "utility.h"
#include "hard_ware.h"
#include "led.h"
#include "system_state.h"
#include "config_service.h"
//#include "g_sensor.h"
#include "uart.h"
//#include "watch_dog.h"
#include "log_service.h"
//#include "relay.h"
#include "gsm.h"
#include "sms.h"
#include "gps.h"
#include "gprs.h"
#include "expmu.h"
#include "auto_test.h"
#include "expmu_service.h"
#include "gm_app.h"
extern s32 TK_dll_set_sb(S32 val);
extern void app_main_entry(void);
extern void DLL_init_SB(void);

extern const s32 service_feature;
u32 dll_version=0;
u32 dll_edition=0;
s32 g_exportfunc[3]={0};

void Service_Entry(dll_struct* dll)
{
	TK_dll_set_sb(service_feature);

	dll_version = dll->header.version;
	dll_edition = dll->header.feature;
	dll->export_func_count = sizeof(g_exportfunc) / sizeof(g_exportfunc[0]);
	dll->export_funcs = g_exportfunc;
	
	dll->export_funcs[0] = (S32)DLL_init_SB;
	dll->export_funcs[1] = (S32)app_main_entry;
}


void DLL_init_SB(void)
{
	TK_dll_set_sb(service_feature);
}

void app_main_entry(void)
{
    //system ticks
	util_create();

     //uart model create
    uart_create();

    system_state_create();

    //Read current config para
    config_service_read_from_local();
    
    //GSM model create
    gsm_create();

    //gprs model create
    gprs_create();

    //sms model create
    sms_create();

    hard_ware_create();

    led_create();

    gps_create();

    expmu_create();

    //延迟10秒再开始自检，否则还没有获取到IMEI会误入自检
    #if 0 /*当前版本不进入自检*/
    if(GM_REBOOT_POWER_ON == system_state_get_boot_reason(false))
	{
		GM_StartTimer(GM_TIMER_SELF_CHECK_START, 30*TIM_GEN_1SECOND, timer_self_test_start);
		LOG(INFO,"It may enter self check 30 seconds later.");
	}
	#endif
	//g_sensor_create();
	GM_StartTimer(GM_TIMER_MAIN_UPLOAD_LOG, 2*TIM_GEN_1SECOND, upload_boot_log);


    GM_CreateKalTimer(0);
    
    GM_StartKalTimer(0, kal_timer_1s_proc, GM_TICKS_1_SEC);

    GM_StartTimer(GM_TIMER_10MS_MAIN, TIM_GEN_10MS, timer_10ms_proc);

    GM_StartTimer(GM_TIMER_1S_MAIN, TIM_GEN_1SECOND, timer_1s_proc);
    
}


//app main cycle
void timer_10ms_proc(void)
{
	GM_StartTimer(GM_TIMER_10MS_MAIN, TIM_GEN_10MS, timer_10ms_proc);

    expmu_service_timer_proc();

    expmu_timer_proc();
    
	led_timer_proc();

    //gsm_timer_proc();

    //g_sensor_timer_proc();
    gprs_timer_proc();

    uart_timer_proc();

}


void kal_timer_1s_proc(void* p_arg)
{
    util_timer_proc();

    system_state_timer_proc();

}


void timer_1s_proc(void)
{
    GM_StartTimer(GM_TIMER_1S_MAIN, TIM_GEN_1SECOND, timer_1s_proc);

    auto_test_timer_proc();
}


