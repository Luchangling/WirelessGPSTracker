/**
 * Copyright @ Goome Technologies Co., Ltd. 2009-2019. All rights reserved.
 * File name:        system_state.c
 * Author:           王志华       
 * Version:          1.0
 * Date:             2019-03-12
 * Description:      全局系统状态管理
 * Others:           
 * Function List:    
    1. 
    
 * History: 
    1. Date:         2019-03-12
       Author:       王志华
       Modification: 创建初始版本
    2. Date: 		 
	   Author:		 
	   Modification: 

 */

#pragma diag_suppress 870 

#include <gm_fs.h>
#include <gm_stdlib.h>
#include <gm_gprs.h>
#include "system_state.h"
#include "utility.h"
#include "applied_math.h"
#include "config_service.h"
#include "log_service.h"
#include "gprs.h"
#include "gps_service.h"
#include "socket.h"

#define GPS_ON_TIME_SECONDS (5*SECONDS_PER_MIN)
#define SYSTEM_STATE_FILE L"Z:\\goome\\GmStatusFile\0"
#define MAGIC_NUMBER 0xABCDDCBA



typedef struct
{
	//数据长度（不包含前3个字段,从status_bits开始算起）
	U16 data_len;

	//16比特CRC（不包含前3个字段,从status_bits开始算起）
    U16 crc;
	
	//特征值
	U32 magic;
	
    U32 status_bits;

	BootReason boot_reason;

	//系统状态（工作,休眠）
	SystemWorkState work_state;

	//车辆状态（运动,静止）
	VehicleState vehicle_state;

	//上电次数
	U32 power_on_count;

	//重启次数
	U32 reboot_count;

	//启动时间
	U32 start_time;

	//外部电池电压等级
	U8 extern_battery_voltage_grade;

	//历程（单位:公里）
	U32 mileage;

	//当前可执行文件的校验合
	U32 check_sum;

	GSensorType gsensor_type;

    u32 last_good_time;  // 上次网络正常的时间
    
    u32 call_ok_count;   //CURRENT_GPRS_INIT->CURRENT_GPRS_CALL_OK 次数

    statistical_info_struct statis; /*统计信息*/

	u8 ip_cache[SOCKET_INDEX_MAX][4];

    U32 reboot_for_power_on_counts;

    U16 reboot_for_net_counts;

    U16 reboot_for_gps_counts;

    U16 reboot_for_upgrade_counts;

    U16 reboot_for_command_counts;

    U16 reboot_for_exception_counts;

    U16 reboot_for_checkpara_counts;

    U16 reboot_for_other_counts;

    u32 expmu_smsp_comm_err_counts;

    u32 expmu_smsp_comm_eatablish_long_time;

    u32 expmu_smsp_comm_cannot_establish;

    u32 expmu_rtc_tim_not_match;
}SystemState,*PSystemState;

static SystemState s_system_state;

static void init_para(void);

static GM_ERRCODE read_state_from_file(void); 

static GM_ERRCODE save_state_to_file(void); 

GM_ERRCODE system_state_create(void)
{
	GM_ERRCODE ret = GM_SUCCESS;

    init_para();

    ret = read_state_from_file();
	
	return ret;
}


static void init_para(void)
{
	char *addr = NULL;
	s_system_state.data_len = sizeof(s_system_state) - 2*sizeof(U16) - sizeof(U32);
	s_system_state.magic = MAGIC_NUMBER;
	s_system_state.status_bits = 0;
	s_system_state.boot_reason = GM_RREBOOT_UNKNOWN;
	s_system_state.work_state = GM_SYSTEM_STATE_WORK;
	s_system_state.vehicle_state = VEHICLE_STATE_RUN;
	s_system_state.power_on_count = 0;
	s_system_state.reboot_count = 0;
	s_system_state.extern_battery_voltage_grade = 0;
	s_system_state.mileage = 0;
	s_system_state.gsensor_type = GSENSOR_TYPE_UNKNOWN;
	
	addr = config_service_get_pointer(CFG_SERVERADDR);
	GM_memset(s_system_state.ip_cache, 0, sizeof(s_system_state.ip_cache));
    if (GM_strstr(addr, GOOME_GPSOO_DNS) > 0)
    {
        GM_ConvertIpAddr(CONFIG_GOOCAR_SERVER_IP,s_system_state.ip_cache[SOCKET_INDEX_MAIN]);
    }

    if (GM_strstr(addr, GOOME_LITEDEV_DNS) > 0)
    {
         GM_ConvertIpAddr(CONFIG_LITE_SERVER_IP,s_system_state.ip_cache[SOCKET_INDEX_MAIN]);
    }
	GM_ConvertIpAddr(CONFIG_AGPS_SERVER_IP,s_system_state.ip_cache[SOCKET_INDEX_AGPS]);
    GM_ConvertIpAddr(CONFIG_LOG_SERVER_IP,s_system_state.ip_cache[SOCKET_INDEX_LOG]);
    GM_ConvertIpAddr(GOOME_UPDATE_SERVER_IP,s_system_state.ip_cache[SOCKET_INDEX_UPDATE]);
    GM_ConvertIpAddr(CONFIG_SERVER_IP,s_system_state.ip_cache[SOCKET_INDEX_CONFIG]);

	
}

GM_ERRCODE system_state_destroy(void)
{
	return GM_SUCCESS;
}

GM_ERRCODE system_state_reset(void)
{
	init_para();
    s_system_state.reboot_count = 0;
	s_system_state.reboot_for_power_on_counts = 0;
	s_system_state.reboot_for_net_counts = 0;
    s_system_state.reboot_for_gps_counts = 0;
	s_system_state.reboot_for_upgrade_counts = 0;
	s_system_state.reboot_for_command_counts = 0;
	s_system_state.reboot_for_exception_counts = 0;
	s_system_state.reboot_for_other_counts = 0;
	s_system_state.reboot_for_checkpara_counts = 0;
    s_system_state.expmu_smsp_comm_err_counts = 0;
    s_system_state.expmu_smsp_comm_eatablish_long_time = 0;
    s_system_state.expmu_smsp_comm_cannot_establish = 0;
    s_system_state.expmu_rtc_tim_not_match = 0;
    GM_memset((u8 *)&s_system_state.statis,0,sizeof(statistical_info_struct));
	return save_state_to_file();
}


GM_ERRCODE system_state_timer_proc(void)
{
	s_system_state.start_time++;
	return GM_SUCCESS;
}

U32 system_state_get_status_bits(void)
{
	return s_system_state.status_bits;
}

U32 system_state_get_start_time()
{
	return s_system_state.start_time;
}

BootReason system_state_get_boot_reason(bool add_counts)
{
    s_system_state.reboot_count++;
    if(add_counts)
    {
        switch (s_system_state.boot_reason)
        {
            case GM_RREBOOT_UNKNOWN:
            {
                s_system_state.reboot_for_exception_counts++;
            }
            break;
            
            case GM_REBOOT_CMD:
            {
                s_system_state.reboot_for_command_counts++;
            }
            break;
            
            case GM_REBOOT_UPDATE:
            {
                s_system_state.reboot_for_upgrade_counts++;
            }
            break;
            
            case GM_REBOOT_POWER_ON:
            {
                s_system_state.reboot_for_power_on_counts++;
            }
            break;
            
            case GM_REBOOT_GPRS:
            {
                s_system_state.reboot_for_net_counts++;
            }
            break;

            case GM_REBOOT_GPS:
            {
                s_system_state.reboot_for_gps_counts++;
            }
            break;

            
            case GM_REBOOT_CHECKPARA:
            {
                s_system_state.reboot_for_checkpara_counts++;
            }
            break;
            
            default:
            {
                s_system_state.reboot_for_other_counts++;
            }
            
            break;
        }
        
        LOG(INFO,"system_state_get_boot_reason");
        save_state_to_file();
    }
    
    return s_system_state.boot_reason;
}


GM_ERRCODE system_state_set_boot_reason(const BootReason boot_reason)
{
    s_system_state.boot_reason = boot_reason;
    
    if (GM_REBOOT_POWER_ON == boot_reason)
	{
		return GM_SUCCESS;
	}
   
	return save_state_to_file();
}

SystemWorkState system_state_get_work_state(void)
{
	return s_system_state.work_state;
}

GM_ERRCODE system_state_set_work_state(const SystemWorkState work_state)
{
	if(s_system_state.work_state != work_state)
	{
		s_system_state.work_state = work_state;
		
		//休眠或者唤醒，全部传感器报警状态清除，电压报警和其它状态不变
		s_system_state.status_bits &=0x0007ffff;
		return save_state_to_file();
	}
	return GM_SUCCESS;
}

GM_ERRCODE system_state_set_vehicle_state(const VehicleState vehicle_state)
{
	if(s_system_state.vehicle_state != vehicle_state)
	{
		LOG(INFO, "vehicle_state from %d to %d", s_system_state.vehicle_state,vehicle_state);
		s_system_state.vehicle_state = vehicle_state;
		
		return save_state_to_file();
	}
	return GM_SUCCESS;
}

VehicleState system_state_get_vehicle_state(void)
{
	return s_system_state.vehicle_state;
}

static GM_ERRCODE read_state_from_file()
{
	U32 file_len = 0;
    S32 file_handle = -1;
	S32 ret = -1;
    U16 crc = 0;
    
    file_handle = GM_FS_Open(SYSTEM_STATE_FILE, GM_FS_READ_ONLY | GM_FS_ATTR_ARCHIVE | GM_FS_CREATE);
    if (file_handle < 0)
    {
        LOG(ERROR,"Failed to open system state file[%d]", file_handle);
        return GM_SYSTEM_ERROR;
    }
    
    ret = GM_FS_Read(file_handle, (void *)&s_system_state, sizeof(s_system_state), &file_len);
    if (ret < 0)
    {
        LOG(ERROR,"Failed to read system state file[%d]", ret);
		init_para();
        GM_FS_Close(file_handle);
        return GM_SYSTEM_ERROR;
    }
    
    GM_FS_Close(file_handle);
    
    
    crc = applied_math_calc_common_crc16((U8*)&s_system_state.status_bits, s_system_state.data_len);
    
    
    if (s_system_state.magic != MAGIC_NUMBER)
    {
        LOG(ERROR,"magic error:%x, ", s_system_state.magic);
		init_para();
        return GM_SYSTEM_ERROR;
    }
    
    if (s_system_state.crc != crc)
    {
        LOG(ERROR,"crc error:%x, %x", s_system_state.crc, crc);
		init_para();
        return GM_SYSTEM_ERROR;
    }
	return GM_SUCCESS;
}

static GM_ERRCODE save_state_to_file(void)
{
	U32 file_len = 0;
    S32 file_handle = -1;
	S32 ret = -1;
    
    file_handle = GM_FS_Open(SYSTEM_STATE_FILE, GM_FS_READ_WRITE | GM_FS_ATTR_ARCHIVE | GM_FS_CREATE);
    if (file_handle < 0)
    {
        LOG(ERROR,"Failed to open system state file[%d]", file_handle);
        return GM_SYSTEM_ERROR;
    }

    s_system_state.last_good_time = gprs_get_last_good_time();
    s_system_state.call_ok_count = gprs_get_call_ok_count();
	
    s_system_state.crc = applied_math_calc_common_crc16((u8*)&s_system_state.status_bits, s_system_state.data_len);
    
    ret = GM_FS_Write(file_handle, (void *)&s_system_state, sizeof(s_system_state), &file_len);
    if (ret < 0)
    {
        LOG(ERROR,"Failed to write system state file[%d]", ret);
        GM_FS_Close(file_handle);
        return GM_SYSTEM_ERROR;
    }
	
	GM_FS_Commit(file_handle);
    GM_FS_Close(file_handle);
	
	return GM_SUCCESS;
}

bool system_state_is_boot(void)
{
	return GET_BIT0(s_system_state.status_bits);
}

GM_ERRCODE system_state_set_boot(bool state)
{
	if (state)
	{
		SET_BIT0(s_system_state.status_bits);
		s_system_state.power_on_count++;
		
		system_state_set_has_reported_gps_since_boot(false);
		s_system_state.start_time = 0;
	}
	else
	{
		CLR_BIT0(s_system_state.status_bits);
		s_system_state.reboot_count++;
	}
	
	return GM_SUCCESS;
}

bool system_state_has_reported_gps_since_boot(void)
{
	return GET_BIT1(s_system_state.status_bits);
}

GM_ERRCODE system_state_set_has_reported_gps_since_boot(bool state)
{
	if (state == system_state_has_reported_gps_since_boot())
	{
		return GM_SUCCESS;
	}

	if (state)
	{
		SET_BIT1(s_system_state.status_bits);
	}
	else
	{
		CLR_BIT1(s_system_state.status_bits);
	}
	LOG(INFO,"system_state_set_has_reported_gps_since_boot");
	return save_state_to_file();
}


U32 system_state_get_poweron_counts(void)
{
	return s_system_state.power_on_count;
}

U32 system_state_get_reboot_counts(const BootReason boot_reason)
{
    switch (boot_reason)
    {
        case GM_RREBOOT_UNKNOWN:
        {
            return s_system_state.reboot_for_exception_counts;
        }
        
        case GM_REBOOT_CMD:
        {
            return s_system_state.reboot_for_command_counts;
        }
        
        case GM_REBOOT_UPDATE:
        {
            return s_system_state.reboot_for_upgrade_counts;
        }
        
        case GM_REBOOT_POWER_ON:
        {
            return s_system_state.reboot_for_power_on_counts;
        }
        
        case GM_REBOOT_GPRS:
        {
            return s_system_state.reboot_for_net_counts;
        }

        case GM_REBOOT_GPS:
        {
            return s_system_state.reboot_for_gps_counts;
        }
        
        case GM_REBOOT_CHECKPARA:
        {
            return s_system_state.reboot_for_checkpara_counts;
        }
        
        default:
        {
            return s_system_state.reboot_for_other_counts;
        }
    }
}

bool system_state_get_has_reported_static_gps(void)
{
	return GET_BIT2(s_system_state.status_bits);
}

GM_ERRCODE system_state_set_has_reported_static_gps(bool state)
{
	if (state == system_state_get_has_reported_static_gps())
	{
		return GM_SUCCESS;
	}

	if (state)
	{
		SET_BIT2(s_system_state.status_bits);
	}
	else
	{
		CLR_BIT2(s_system_state.status_bits);
	}
	return save_state_to_file();
}

bool system_state_get_reported_gps_since_modify_ip(void)
{
	return GET_BIT3(s_system_state.status_bits);
}

GM_ERRCODE system_state_set_reported_gps_since_modify_ip(bool state)
{
	if (state == system_state_get_reported_gps_since_modify_ip())
	{
		return GM_SUCCESS;
	}

	if (state)
	{
		SET_BIT3(s_system_state.status_bits);
	}
	else
	{
		CLR_BIT3(s_system_state.status_bits);
	}
	return save_state_to_file();
}


bool system_state_get_has_started_charge(void)
{
	return GET_BIT4(s_system_state.status_bits);
}

GM_ERRCODE system_state_set_has_started_charge(bool state)
{
	if (state == system_state_get_has_started_charge())
	{
		return GM_SUCCESS;
	}

	if (state)
	{
		SET_BIT4(s_system_state.status_bits);
	}
	else
	{
		CLR_BIT4(s_system_state.status_bits);
	}
	return save_state_to_file();
}

bool system_state_get_acc_is_line_mode(void)
{
	return GET_BIT5(s_system_state.status_bits);
}

GM_ERRCODE system_state_set_acc_is_line_mode(bool state)
{
	if (state == system_state_get_acc_is_line_mode())
	{
		return GM_SUCCESS;
	}

	if (state)
	{
		SET_BIT5(s_system_state.status_bits);
	}
	else
	{
		CLR_BIT5(s_system_state.status_bits);
	}
	return save_state_to_file();
}

bool system_state_get_device_relay_state(void)
{
	return GET_BIT6(s_system_state.status_bits);
}

GM_ERRCODE system_state_set_device_relay_state(bool state)
{
	if (state == system_state_get_device_relay_state())
	{
		return GM_SUCCESS;
	}

	if (state)
	{
		SET_BIT6(s_system_state.status_bits);
	}
	else
	{
		CLR_BIT6(s_system_state.status_bits);
	}
	return save_state_to_file();
}

bool system_state_get_user_relay_state(void)
{
	return GET_BIT7(s_system_state.status_bits);
}

GM_ERRCODE system_state_set_user_relay_state(bool state)
{
	if (state == system_state_get_user_relay_state())
	{
		return GM_SUCCESS;
	}

	if (state)
	{
		SET_BIT7(s_system_state.status_bits);
	}
	else
	{
		CLR_BIT7(s_system_state.status_bits);
	}
	return save_state_to_file();
}

bool system_state_get_defence(void)
{
	return GET_BIT8(s_system_state.status_bits);
}
GM_ERRCODE system_state_set_defence(bool state)
{
	if (state == system_state_get_defence())
	{
		return GM_SUCCESS;
	}

	if (state)
	{
		SET_BIT8(s_system_state.status_bits);
	}
	else
	{
		CLR_BIT8(s_system_state.status_bits);
	}
	return save_state_to_file();
}

bool system_state_get_test_result(void)
{
	return GET_BIT9(s_system_state.status_bits);
}
GM_ERRCODE system_state_set_test_result(bool state)
{
	if (state == system_state_get_defence())
	{
		return GM_SUCCESS;
	}

	if (state)
	{
		SET_BIT9(s_system_state.status_bits);
	}
	else
	{
		CLR_BIT9(s_system_state.status_bits);
	}
	return save_state_to_file();
}

bool system_state_is_cold_boot(void)
{
	return GET_BIT10(s_system_state.status_bits);
}

GM_ERRCODE system_state_set_cold_boot(bool state)
{
	if (state == system_state_is_cold_boot())
	{
		return GM_SUCCESS;
	}

	if (state)
	{
		SET_BIT10(s_system_state.status_bits);
	}
	else
	{
		CLR_BIT10(s_system_state.status_bits);
	}
	LOG(INFO,"system_state_set_cold_boot");
	return save_state_to_file();
}


U8 system_state_get_extern_battery_voltage_grade(void)
{
	return s_system_state.extern_battery_voltage_grade;
}


GM_ERRCODE system_state_set_extern_battery_voltage_grade(U8 voltage_grade)
{
	if (voltage_grade == s_system_state.extern_battery_voltage_grade)
	{
		return GM_SUCCESS;
	}

	else
	{
		s_system_state.extern_battery_voltage_grade = voltage_grade;
		return save_state_to_file();
	}
}

bool system_state_get_power_off_alarm(void)
{
	return GET_BIT16(s_system_state.status_bits);
}

GM_ERRCODE system_state_set_power_off_alarm(bool state)
{
	if (state == system_state_get_power_off_alarm())
	{
		return GM_SUCCESS;
	}

	if (state)
	{
		SET_BIT16(s_system_state.status_bits);
	}
	else
	{
		CLR_BIT16(s_system_state.status_bits);
	}
	return save_state_to_file();
}

bool system_state_get_battery_low_voltage_alarm(void)
{
	return GET_BIT17(s_system_state.status_bits);
}


bool system_state_get_shake_alarm(void)
{
	return GET_BIT19(s_system_state.status_bits);
}

GM_ERRCODE system_state_set_shake_alarm(bool state)
{
	if (state == system_state_get_shake_alarm())
	{
		return GM_SUCCESS;
	}

	if (state)
	{
		SET_BIT19(s_system_state.status_bits);
	}
	else
	{
		CLR_BIT19(s_system_state.status_bits);
	}
	return save_state_to_file();
}

bool system_state_get_overspeed_alarm(void)
{
	return GET_BIT20(s_system_state.status_bits);
}

GM_ERRCODE system_state_set_overspeed_alarm(bool state)
{
	if (state == system_state_get_overspeed_alarm())
	{
		return GM_SUCCESS;
	}

	if (state)
	{
		SET_BIT20(s_system_state.status_bits);
	}
	else
	{
		CLR_BIT20(s_system_state.status_bits);
	}
	return save_state_to_file();
}

bool system_state_get_fakecell_alarm(void)
{
	return GET_BIT21(s_system_state.status_bits);
}

GM_ERRCODE system_state_set_fakecell_alarm(bool state)
{
	if (state == system_state_get_fakecell_alarm())
	{
		return GM_SUCCESS;
	}

	if (state)
	{
		SET_BIT21(s_system_state.status_bits);
	}
	else
	{
		CLR_BIT21(s_system_state.status_bits);
	}
	return save_state_to_file();
}

bool system_state_get_high_voltage_alarm(void)
{
	return GET_BIT22(s_system_state.status_bits);
}

GM_ERRCODE system_state_set_high_voltage_alarm(bool state)
{
	if (state == system_state_get_high_voltage_alarm())
	{
		return GM_SUCCESS;
	}

	if (state)
	{
		SET_BIT22(s_system_state.status_bits);
	}
	else
	{
		CLR_BIT22(s_system_state.status_bits);
	}
	return save_state_to_file();
}


bool system_state_get_collision_alarm(void)
{
	return GET_BIT23(s_system_state.status_bits);
}

GM_ERRCODE system_state_set_collision_alarm(bool state)
{
	if (state == system_state_get_collision_alarm())
	{
		return GM_SUCCESS;
	}

	if (state)
	{
		SET_BIT23(s_system_state.status_bits);
	}
	else
	{
		CLR_BIT23(s_system_state.status_bits);
	}
	return save_state_to_file();
}

bool system_state_get_speed_up_alarm(void)
{
	return GET_BIT24(s_system_state.status_bits);
}

GM_ERRCODE system_state_set_speed_up_alarm(bool state)
{
	if (state == system_state_get_speed_up_alarm())
	{
		return GM_SUCCESS;
	}

	if (state)
	{
		SET_BIT24(s_system_state.status_bits);
	}
	else
	{
		CLR_BIT24(s_system_state.status_bits);
	}
	return save_state_to_file();
}

bool system_state_get_speed_down_alarm(void)
{
	return GET_BIT25(s_system_state.status_bits);
}

GM_ERRCODE system_state_set_speed_down_alarm(bool state)
{
	if (state == system_state_get_speed_down_alarm())
	{
		return GM_SUCCESS;
	}

	if (state)
	{
		SET_BIT25(s_system_state.status_bits);
	}
	else
	{
		CLR_BIT25(s_system_state.status_bits);
	}
	return save_state_to_file();
}

bool system_state_get_turn_over_alarm(void)
{
	return GET_BIT26(s_system_state.status_bits);
}

GM_ERRCODE system_state_set_turn_over_alarm(bool state)
{
	if (state == system_state_get_turn_over_alarm())
	{
		return GM_SUCCESS;
	}

	if (state)
	{
		SET_BIT26(s_system_state.status_bits);
	}
	else
	{
		CLR_BIT26(s_system_state.status_bits);
	}
	return save_state_to_file();
}

bool system_state_get_sharp_turn_alarm(void)
{
	return GET_BIT27(s_system_state.status_bits);
}

GM_ERRCODE system_state_set_sharp_turn_alarm(bool state)
{
	if (state == system_state_get_sharp_turn_alarm())
	{
		return GM_SUCCESS;
	}

	if (state)
	{
		SET_BIT27(s_system_state.status_bits);
	}
	else
	{
		CLR_BIT27(s_system_state.status_bits);
	}
	return save_state_to_file();
}


bool system_state_get_remove_alarm(void)
{
	return GET_BIT28(s_system_state.status_bits);
}

GM_ERRCODE system_state_set_remove_alarm(bool state)
{
	if (state == system_state_get_remove_alarm())
	{
		return GM_SUCCESS;
	}

	if (state)
	{
		SET_BIT28(s_system_state.status_bits);
	}
	else
	{
		CLR_BIT28(s_system_state.status_bits);
	}
	return save_state_to_file();
}


bool system_state_get_move_alarm(void)
{
	return GET_BIT29(s_system_state.status_bits);
}

GM_ERRCODE system_state_set_move_alarm(bool state)
{
	if (state == system_state_get_move_alarm())
	{
		return GM_SUCCESS;
	}

	if (state)
	{
		SET_BIT29(s_system_state.status_bits);
	}
	else
	{
		CLR_BIT29(s_system_state.status_bits);
	}
	return save_state_to_file();
}

void system_state_set_mileage(U32 mileage)
{
	if (mileage == s_system_state.mileage)
	{
		return;
	}

	s_system_state.mileage = mileage;
	if (util_clock()%(10*SECONDS_PER_MIN) == 0)
	{
		save_state_to_file();
	}
}

U32 system_state_get_mileage(void)
{
	return s_system_state.mileage;
}

void system_state_set_bin_checksum(U32 check_sum)
{
	if (check_sum == s_system_state.check_sum)
	{
		return;
	}

	else
	{
		s_system_state.check_sum = check_sum;
		save_state_to_file();
	}
}

U32 system_state_get_bin_checksum(void)
{
	return s_system_state.check_sum;
}

void system_state_set_gsensor_type(GSensorType gsensor_type)
{
	if (gsensor_type == s_system_state.gsensor_type)
	{
		return;
	}

	else
	{
		s_system_state.gsensor_type = gsensor_type;
		save_state_to_file();
	}
}

GSensorType system_state_get_gsensor_type(void)
{
	return s_system_state.gsensor_type;
}

u32 system_state_get_last_good_time(void)
{
	return s_system_state.last_good_time;
}

u32 system_state_get_call_ok_count(void)
{
	return s_system_state.call_ok_count;
}

statistical_info_struct *system_state_get_statis_pointer(void)
{
    return &s_system_state.statis;
}

void system_state_set_static_info(void)
{
    save_state_to_file();
}

void system_state_set_ip_cache(U8 index,const U8* ip)
{
	GM_memcpy(s_system_state.ip_cache[index], ip, 4);
	save_state_to_file();
}

void system_state_get_ip_cache(U8 index,U8* ip)
{
	GM_memcpy(ip, s_system_state.ip_cache[index], 4);
}

void system_state_set_expmu_comm_err_count(void)
{
    s_system_state.expmu_smsp_comm_err_counts++;
}

void system_state_set_expmu_smsp_comm_eatablish_long_time(void)
{
    s_system_state.expmu_smsp_comm_eatablish_long_time++;
}

void system_state_set_expmu_smsp_comm_cannot_establish(void)
{
    s_system_state.expmu_smsp_comm_cannot_establish++;
}

void system_state_set_rtc_time_not_match_count(void)
{
    s_system_state.expmu_rtc_tim_not_match++;
}


u32 system_state_get_expmu_comm_err_count(void)
{
    return s_system_state.expmu_smsp_comm_err_counts;
}

u32 system_state_get_expmu_smsp_comm_eatablish_long_time(void)
{
    return s_system_state.expmu_smsp_comm_eatablish_long_time;
}

u32 system_state_get_expmu_smsp_comm_cannot_establish(void)
{
    return s_system_state.expmu_smsp_comm_cannot_establish;
}

u32 system_state_get_rtc_time_not_match_count(void)
{
    return s_system_state.expmu_rtc_tim_not_match;
}



