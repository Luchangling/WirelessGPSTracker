/**
 * Copyright @ Goome Technologies Co., Ltd. 2009-2019. All rights reserved.
 * File name:        hard_ware.h
 * Author:           王志华       
 * Version:          1.0
 * Date:             2019-03-01
 * Description:      简单硬件（开关）读写操作封装,包括读取ADC的数值
 * Others:      
 * Function List:    
    1. 创建hard_ware模块
    2. 销毁hard_ware模块
    3. 定时处理入口
    4. 获取ACC输入状态
    5. 设置GPS LED状态
    6. 设置GSM LED状态
    7. 设置断油电IO状态
    8. 获取供电电源电压值
    9. 设置内置电池充电状态（GS05）
 * History: 
    1. Date:         2019-03-01
       Author:       王志华
       Modification: 创建初始版本
    2. Date: 		 
	   Author:		 
	   Modification: 

 */

#include <gm_time.h>
#include <gm_timer.h>
#include <gm_gpio.h>
#include <gm_adc.h>
#include <gm_power.h>
#include <gm_memory.h>
#include <gm_system.h>
#include <gm_stdlib.h>
#include "g_sensor.h"
#include "hard_ware.h"
#include "gps.h"
#include "uart.h"
#include "system_state.h"
#include "log_service.h"
#include "config_service.h"
#include "utility.h"
#include "gps_service.h"
#include "applied_math.h"
#include "auto_test.h"
#include "update_file.h"


//输出:GPS开关
#define GM_GPS_POWER_CTROL_GPIO_PIN  GM_GPIO19


typedef struct
{
	bool inited;
	
	
	U16 timer_ms;

	

	bool gps_led_is_on;
	bool gsm_led_is_on;
	
}HardWare, *PHardWare;

static HardWare s_hard_ware;

static GM_ERRCODE init_gpio(void);

/**
 * Function:   创建hard_ware模块
 * Description:创建hard_ware模块
 * Input:	   无
 * Output:	   无
 * Return:	   GM_SUCCESS——成功；其它错误码——失败
 * Others:	   管脚(参看电路图及管脚功能表)
 */
GM_ERRCODE hard_ware_create(void)
{
	GM_ERRCODE ret = GM_SUCCESS;

	if (s_hard_ware.inited)
	{
		return GM_SUCCESS;
	}

	ret = init_gpio();
	if (GM_SUCCESS != ret)
	{
		LOG(ERROR,"Failed to init GPIO!");
	}
	
	s_hard_ware.inited = false;

	s_hard_ware.timer_ms = 0;
	s_hard_ware.gps_led_is_on = false;
	s_hard_ware.gsm_led_is_on = false;

	LOG(DEBUG, "hard_ware_create");
	

	s_hard_ware.inited = true;
	
	return GM_SUCCESS;
}

static GM_ERRCODE init_gpio(void)
{
	S32 ret = GM_SUCCESS;
	

	ret = GM_GpioInit(GM_GPS_POWER_CTROL_GPIO_PIN, PINDIRECTION_OUT, PINLEVEL_HIGH, PINPULLSEL_DISABLE);
	if (GM_SUCCESS != ret)
	{
	   LOG(FATAL,"Failed to init GM_GPS_POWER_CTROL_GPIO_PIN!");
	   return GM_HARD_WARE_ERROR;
	}

	
	return GM_SUCCESS;
}

/**
 * Function:   销毁hard_ware模块
 * Description:销毁hard_ware模块
 * Input:	   无
 * Output:	   无
 * Return:	   GM_SUCCESS——成功；其它错误码——失败
 * Others:	   
 */
GM_ERRCODE hard_ware_destroy(void)
{
	s_hard_ware.inited = false;
	return GM_SUCCESS;
}

GM_ERRCODE hard_ware_timer_proc(void)
{
	return GM_SUCCESS;
}



/**
 * Function:   设置GPS LED状态
 * Description:
 * Input:	   state:true——亮灯；false——灭灯
 * Output:	   无
 * Return:	   GM_SUCCESS——成功；其它错误码——失败
 * Others:	   
 */
GM_ERRCODE hard_ware_set_gps_led(bool is_on)
{
	S32 ret = 0;


	if (false == s_hard_ware.inited)
	{
		return GM_NOT_INIT;
	}

	if (s_hard_ware.gps_led_is_on == is_on)
	{
		return GM_SUCCESS;
	}
	else
	{
		s_hard_ware.gps_led_is_on = is_on;
			
		ret = GM_IsinkBacklightCtrl(is_on,is_on);
		if (0 == ret)
		{
			return GM_SUCCESS;
		}
		else
		{
			LOG(ERROR,"Failed to GM_IsinkBacklightCtrl,ret=%d!",ret);
			return GM_HARD_WARE_ERROR;
		}
	}
}

/**
 * Function:   设置GSM LED状态
 * Description:
 * Input:	   state:true——亮灯；false——灭灯
 * Output:	   无
 * Return:	   GM_SUCCESS——成功；其它错误码——失败
 * Others:	   
 */
GM_ERRCODE hard_ware_set_gsm_led(bool is_on)
{
	S32 ret = 0;
	if (false == s_hard_ware.inited)
	{
		return GM_NOT_INIT;
	}

	if (s_hard_ware.gsm_led_is_on == is_on)
	{
		return GM_SUCCESS;
	}
	else
	{
		s_hard_ware.gsm_led_is_on = is_on;

		ret = GM_KpledLevelCtrl(is_on,is_on);
		if (0 == ret)
		{
			return GM_SUCCESS;
		}
		else
		{
			LOG(ERROR,"Failed to GM_KpledLevelCtrl,ret=%d!",ret);
			return GM_HARD_WARE_ERROR;
		}
	}
}


static const char* get_boot_reason_str(BootReason reason)
{
	switch (reason)
	{
		case GM_RREBOOT_UNKNOWN:
			return "意外重启";
		case GM_REBOOT_CMD:
			return "指令重启";
    	case GM_REBOOT_UPDATE:
			return "升级重启";
    	case GM_REBOOT_POWER_ON:
			return "掉电重启";
		case GM_REBOOT_GPRS:
			return "掉线重启";
		case GM_REBOOT_AUTO_TEST:
			return "测试结束";
		default:
			return "未知";
	}
}


void upload_boot_log(void)
{
	JsonObject* p_log_root = NULL;
	char kernel_version[50] = {0};
	//GPSChipType gps_chip_type = GM_GPS_TYPE_UNKNOWN;
	//char gsensor_type_str[20] = {0};
	char check_sum_str[9] = {0};

    u16 jval = 0;

    u8 zone  = 0;

    char temp[100] = {0};

    u32 app_check_sum;

    statistical_info_struct *sta = NULL;
			   
	p_log_root = json_create();
	json_add_string(p_log_root, "event", "boot");
	
	json_add_string(p_log_root, "reason", get_boot_reason_str(system_state_get_boot_reason(true)));
	system_state_set_boot_reason(GM_RREBOOT_UNKNOWN);
	
	json_add_string(p_log_root, "app version", VERSION_NUMBER);
	
	//json_add_string(p_log_root, "app build time", SW_APP_BUILD_DATE_TIME);

    app_check_sum = update_filemod_get_checksum(UPDATE_TARGET_IMAGE);
	GM_snprintf(check_sum_str, 8, "%4X", app_check_sum);

	system_state_set_bin_checksum(app_check_sum);

	json_add_string(p_log_root, "app check sum",check_sum_str);
	
    GM_ReleaseVerno((U8*)kernel_version);
	GM_sprintf(kernel_version + GM_strlen(kernel_version), "(%s)",GM_BuildDateTime());
	json_add_string(p_log_root, "kernel version", kernel_version);
	
	json_add_string(p_log_root, "server", (char*)config_service_get_pointer(CFG_SERVERADDR));
    #if 0
	json_add_int(p_log_root, "system state bits", system_state_get_status_bits());

	json_add_int(p_log_root, "exceptions reboots", system_state_get_reboot_counts(GM_RREBOOT_UNKNOWN));
    json_add_int(p_log_root, "power on counts", system_state_get_reboot_counts(GM_REBOOT_POWER_ON));
	json_add_int(p_log_root, "GPRS reboots", system_state_get_reboot_counts(GM_REBOOT_GPRS));
	json_add_int(p_log_root, "GPS reboots", system_state_get_reboot_counts(GM_REBOOT_GPS));
	json_add_int(p_log_root, "command reboots", system_state_get_reboot_counts(GM_REBOOT_CMD));
	json_add_int(p_log_root, "upgrade reboots", system_state_get_reboot_counts(GM_REBOOT_UPDATE));
	json_add_int(p_log_root, "check params reboots", system_state_get_reboot_counts(GM_REBOOT_CHECKPARA));
	
	
	json_add_int(p_log_root, "gprs_last_good", system_state_get_last_good_time());
	json_add_int(p_log_root, "gprs_ok_count", system_state_get_call_ok_count());
    #endif
    sta = system_state_get_statis_pointer();

    jval = GM_sprintf(temp,"%d/%d/%d",sta->total_send_gps_count,sta->send_gps_success_count,sta->send_gps_error_count);
    temp[jval] = 0;
    json_add_string(p_log_root, "send gps Total/Success/Fail",temp);

    jval = GM_sprintf(temp,"%d/%d/%d",sta->get_config_from_cfg_srv_count,sta->get_config_from_sms_count,sta->get_config_form_uart_count);
    temp[jval] = 0;
    json_add_string(p_log_root,"get cfg from Server/Sms/Uart",temp);

    jval = GM_sprintf(temp,"0x%x/0x%x",sta->gprs_data_traffic_downstream,sta->gprs_data_traffic_upstream);
    temp[jval] = 0;
    json_add_string(p_log_root,"gprs traffic Down/Up",temp);

    zone  = config_service_get_zone();
    util_utc_sec_to_bcdtime_base2000(sta->pre_second_wkp_time,(U8*)check_sum_str,zone);
    jval = GM_sprintf(temp,"%02x%02x%02x%02x%02x%02x",check_sum_str[0],check_sum_str[1],check_sum_str[2],\
        check_sum_str[3],check_sum_str[4],check_sum_str[5]);
    util_utc_sec_to_bcdtime_base2000(sta->pre_first_wkp_time,(U8*)check_sum_str,zone);
    jval += GM_sprintf(&temp[jval]," %02x%02x%02x%02x%02x%02x",check_sum_str[0],check_sum_str[1],check_sum_str[2],\
        check_sum_str[3],check_sum_str[4],check_sum_str[5]);
    temp[jval] = 0;
    json_add_string(p_log_root,"recent two wkp tim",temp);

    jval = GM_sprintf(temp,"%d/%d/%d/%d",(int)sta->pre_report_fail_reason,sta->pre_csq,sta->pre_creg,sta->pre_cgreg);

    json_add_string(p_log_root,"pre wkp state fail reason/csq/creg/cgreg",temp);
    
	log_service_upload(INFO,p_log_root);
}


static void hard_ware_reboot_atonce(void)
{
    //gps_service_save_to_history_file();
	if (0 == GM_SystemReboot())
	{
		LOG(FATAL,"Failed to GM_SystemReboot!");
	}
}


/**
 * Function:   重启
 * Description:重启整个系统软件
 * Input:	   无
 * Output:	   无
 * Return:	   GM_SUCCESS——成功；其它错误码——失败
 * Others:	   
 */
GM_ERRCODE hard_ware_reboot(const BootReason reason,U16 delay_seconds)
{
    //快速重复调用这个函数启动定时器会取消之前设置的定时器，导致无法重启，因此要用个静态变量记录下来
    static bool has_reboot = false;
    system_state_set_boot_reason(reason);

    if (0 == delay_seconds)
    {
        hard_ware_reboot_atonce();
        has_reboot = true;
    }
    else
    {
        if(!has_reboot)
        {
            GM_StartTimer(GM_TIMER_HARDWARE_REBOOT,delay_seconds*TIM_GEN_1SECOND,hard_ware_reboot_atonce);
            has_reboot = true;
        }
    }
    return GM_SUCCESS;
}


/**
 * Function:   休眠
 * Description:使系统休眠
 * Input:	   无
 * Output:	   无
 * Return:	   GM_SUCCESS——成功；其它错误码——失败
 * Others:	   
 */
GM_ERRCODE hard_ware_sleep(void)
{
	if (0 == GM_SleepEnable())
	{
		return GM_HARD_WARE_ERROR;
	}
	else
	{
		//上传日志
		//JsonObject* p_log_root = json_create();;
		//json_add_string(p_log_root, "event", ("sleep"));
		//log_service_upload(INFO,p_log_root, false);
		return GM_SUCCESS;
	}
}

GM_ERRCODE hard_ware_close_gps(void)
{
	GM_GpioSetLevel(GM_GPS_POWER_CTROL_GPIO_PIN, PINLEVEL_LOW);
	return GM_SUCCESS;
}


/**
 * Function:   唤醒
 * Description:使系统唤醒
 * Input:	   无
 * Output:	   无
 * Return:	   GM_SUCCESS——成功；其它错误码——失败
 * Others:	   
 */
GM_ERRCODE hard_ware_awake(void)
{
	if (0 == GM_SleepDisable())
	{
		return GM_HARD_WARE_ERROR;
	}
	else
	{
		//上传日志
		//JsonObject* p_log_root = json_create();
		//json_add_string(p_log_root, "event", ("awake"));
	    //log_service_upload(INFO,p_log_root, false);
		return GM_SUCCESS;
	}
}

GM_ERRCODE hard_ware_open_gps(void)
{
	GM_GpioSetLevel(GM_GPS_POWER_CTROL_GPIO_PIN, PINLEVEL_HIGH);
	return GM_SUCCESS;
}

