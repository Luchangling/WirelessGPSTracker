/**
 * Copyright @ Goome Technologies Co., Ltd. 2009-2019. All rights reserved.
 * File name:        command.h
 * Author:           王志华       
 * Version:          1.0
 * Date:             2019-04-19
 * Description:      
 * Others:      
 * Function List:    
    1. 创建command模块
    2. 销毁command模块
    3. command模块定时处理入口
 * History: 
    1. Date:         2019-04-19
       Author:       王志华
       Modification: 创建初始版本
    2. Date: 		 
	   Author:		 
	   Modification: 

 */

#include <stdarg.h>
#include <gm_stdlib.h>
#include <gm_timer.h>
#include <gm_system.h>
#include <gm_memory.h>
#include "command.h"
#include "utility.h"
#include "gsm.h"
#include "applied_math.h"
#include "hard_ware.h"
#include "config_service.h"
#include "log_service.h"
#include "system_state.h"
#include "gprs.h"
#include "gps_service.h"
#include "main.h"
#include "agps_service.h"
#include "gm_sms.h"
//#include "g_sensor.h"
#include "expmu.h"
#include "system_state.h"
#include "expmu_service.h"
#include "auto_test.h"
#include "stdlib.h"
#include "led.h"
#include "wifi_service.h"


#pragma diag_suppress 870 

#define CMD_NAME_MAX_LEN 15

#define GPS_MAX_INTERVAL_MINUTE (24*60*7)  
#define GPS_MAX_INTERVAL_SECOND (24*60*7*60)

/*工作模式设置的来源 1:协议292922 0:其他*/
static u8 is_get_work_mode_from_22 = 0;

typedef enum
{
	CMD_2929_SET,

    CMD_WRITEIMEI,

    CMD_IMEI,
    
    CMD_ICCID,

    CMD_IMSI,

    CMD_SYSTEM_RESET,

    CMD_LOG_LEVEL,

    CMD_L3_SLEEP,

    CMD_CHPWD,

    CMD_POWERON_SEND_COUNT,

    CMD_REPORT_GAP,

    CMD_TEST,
    CMD_EXIT,

    CMD_PMTK,
    
    CMD_FACTORY_ALL,
	CMD_FACTORY,

    CMD_VER,

    CMD_DEVICE_TYPE,

    CMD_RECORD,

    CMD_RESULT,

    CMD_DEVNO,
	
	CMD_CMD_MAX
}CommandID;

typedef struct
{
    char cmd_name[CMD_NAME_MAX_LEN];
    CommandID cmd_id;
}CommandInfo;

UploadWorkModeStruct s_upload_mod;

static CommandInfo s_cmd_infos[CMD_CMD_MAX + 1] = 
{
    {"<SPGM" , CMD_2929_SET},
    {"GOOIMEI",CMD_WRITEIMEI},
	{"IMEI",CMD_IMEI},
	{"ICCID",CMD_ICCID},
	{"IMSI",CMD_IMSI},
    {"RESET",CMD_SYSTEM_RESET},
    {"LOGLEVEL",CMD_LOG_LEVEL},
    {"GOOSLEEP",CMD_L3_SLEEP},
    {"CHPWD",CMD_CHPWD},
    {"PWRONSENDCOUNT",CMD_POWERON_SEND_COUNT},
	{"REPORT_GAP",CMD_REPORT_GAP},
    {"TEST",CMD_TEST},
    {"EXIT",CMD_EXIT},
	{"PMTK",CMD_PMTK},
	{"FACTORYALL",CMD_FACTORY_ALL},
	{"FACTORY",CMD_FACTORY},
    {"VERSION",CMD_VER},
    {"DEVICETYPE",CMD_DEVICE_TYPE},
    {"RECORD",CMD_RECORD},
    {"RESULT",CMD_RESULT},
    {"DEVICENO",CMD_DEVNO},
};

static CommandID get_cmd_id(const char* cmd_name);
static const char* set_success_rsp(CommandReceiveFromEnum from);
static const char* set_fail_rsp(CommandReceiveFromEnum from);
//static const char* get_protocol_str(U16 lang,CommandReceiveFromEnum from,U8 protocol_type);
//static const char* get_enable_str(U16 lang,CommandReceiveFromEnum from,bool is_on);
static void cmd_reboot(void);
static char command_scan(const char* p_command, const char* p_format, ...);

static CommandID get_cmd_id(const char* cmd_name)
{
	U8 index = 0;
	for (index = 0; index < CMD_CMD_MAX + 1; ++index)
	{
		if (!GM_strcmp(cmd_name, s_cmd_infos[index].cmd_name))
		{
			return s_cmd_infos[index].cmd_id;
		}
	}
	return CMD_CMD_MAX;
}

static const char set_success_rsp_en[] = "Exec Success.";
//static const char set_success_rsp_ch[] = "设置成功";
static const char set_fail_rsp_en[] = "Exec failure!";
//static const char set_fail_rsp_ch[] = "指令错误";

static const char* set_success_rsp(CommandReceiveFromEnum from)
{
	
	return set_success_rsp_en;

}
static const char* set_fail_rsp(CommandReceiveFromEnum from)
{
    return set_fail_rsp_en;

}


typedef enum 
{

    OLD_CMD_PSW,
    OLD_CMD_CELL,
    OLD_CMD_UDP_IP,
    OLD_CMD_GPS_POWE_MOD,
    OLD_CMD_MAIN_DNS,
    OLD_TIMING_NUMBER,
    OLD_DEV_APN,
    OLD_WKM_CLOCK_MODE,
    OLD_WKM_TRACK_MODE,
    OLD_WKM_WEEK_MODE,
    OLD_WKM_PLAT_MODE,
    OLD_LIGHT_ALARM,
    OLD_WIFI_STATE,
    OLD_TIME_ZONE,

    OLD_MAX_CMD
    
}Old_cmd_enum;


enum 
{
    NEED_OP_NOONE = 0,
    NEED_OP_RESET = 1,
    NEED_OP_END_GPRS = 2,
    NEED_OP_SAVE_FILE = 3,
    NEED_OP_CHANGE_DNS = 4,
    NEED_OP_INVALID
};



typedef struct
{
    char cmd[5];
    Old_cmd_enum index; 

}Old_set_cmd_info;

Old_set_cmd_info s_old_cmd_info[OLD_MAX_CMD] = {
    {"*P:",         OLD_CMD_PSW},
    {"*N:",         OLD_CMD_CELL},
    {"*U:",         OLD_CMD_UDP_IP},
    {"*K:",         OLD_CMD_GPS_POWE_MOD},
    {"*Q:",         OLD_CMD_MAIN_DNS},
    {"*E:",         OLD_TIMING_NUMBER},
    {"*A:",         OLD_DEV_APN},
    {"*R:",         OLD_WKM_CLOCK_MODE},
    {"*D:",         OLD_WKM_TRACK_MODE},
    {"*W:",         OLD_WKM_WEEK_MODE},
    {"*G:",         OLD_LIGHT_ALARM},
    {"*F:",         OLD_WKM_PLAT_MODE},
    {"*3:",         OLD_WIFI_STATE},
    {"*Z:",         OLD_TIME_ZONE},
};

/*****************************************************************************
 函 数 名  : check_same_number_in_string
 功能描述  : 检查数字串中是否有重复的数
 输入参数  : u8 *in  
 输出参数  : 无
 返 回 值  : 1 :无重复且合法
 日    期  : 2019年3月19日
 作    者  : Chris.Lu
*****************************************************************************/
u8 check_same_number_in_string(u8 *in)
{
	u8 value = 0;

	u16 len  = 0;

	while(IS_ARABIC_NUM(in[len]))
	{
		if((in[len] == '0')||(in[len] > '7'))
		{
			return 0;
		}
 
		if(value &(1 << (util_chr(in[len] - 1))))
		{
			return 0;
		}

		value |= (1 << (util_chr(in[len] - 1)));

		len ++;
	}

	return 1;
}



static u8 get_param_context(u8 *in , u8* out)
{
    u8 i = 0;

    while(in[i] != '>' && in[i] != '*' && in[i] >= 0x20 && in[i] < 0x7E)
    {
        out[i] = in[i];

        i++;
    }

    return i;
}

static u8 get_sub_cmd_id(u8 *pdata) 
{
    u8  i = 0;

    char  cmd[100] = {0};

    if(pdata[0] != '*') return 0xFF;

    cmd[0] = '*';
    
    get_param_context(&pdata[1],(u8 *)&cmd[1]);


    for(i = 0 ; i < OLD_MAX_CMD ; i++)
    {
        if(GM_strstr(cmd,s_old_cmd_info[i].cmd)!= NULL)
        {
            break;
        }
    }

    if(i < OLD_MAX_CMD)
    {
         return s_old_cmd_info[i].index;
    }

   return 0xff;
}


static u8 old_cmd_set_cell_num(u8 *pdata , u16 len)
{
    char cellnum[17] = {0};
    
    if(len == 11)
    {
        if(util_string_number_counter(pdata) == 11)
        {
            if((GM_memcmp(pdata, "13000000000", 11) >= 0) &&\
               (GM_memcmp(pdata, "14599999999", 11) <= 0))
           {
                GM_memcpy((u8 *)cellnum, (u8 *)pdata, 11);
            
                config_service_set(CFG_DEVICE_NUMBER,TYPE_STRING,(U8 *)cellnum,GM_strlen(cellnum));

                LOG(INFO,"get cell num %s",cellnum);

                return NEED_OP_SAVE_FILE;
           }
        }
    }
    else
    {
        LOG(ERROR,"old cmd set cell num len(%d) %s error",len,pdata);
    }

    return NEED_OP_NOONE;
}

static u8 old_cmd_set_main_dns(char *pdata , u16 len)
{
    char dns_buf[100] = {0};

    u32 port;

    u8 i = 0 , ret = 0;

    i = GM_sscanf(pdata,"%[^:]:%d",dns_buf,&port);

    if(i != 2)
    {
        i = GM_sscanf(pdata,"%[^,],%d",dns_buf,&port);

        if(i != 2)
        {
            return 0;
        }
    }

    if(GM_strlen(pdata) < 50)
    {
        GM_sprintf(&dns_buf[GM_strlen(dns_buf)],":%d",port);
        
        config_service_set(CFG_SERVERADDR,TYPE_STRING,pdata,GM_strlen(dns_buf));

        ret = NEED_OP_SAVE_FILE;
    }

    return ret;
}

static u8 old_cmd_set__udp_ip(char *pdata , u16 len)
{
    u32 port = 0 ,ip[4] = {0};

    u8 ret = 0 ,i = 0;

    i = GM_sscanf(pdata, "%d.%d.%d.%d:%d", ip,ip+1,ip+2,ip+3,&port);

    if(i != 5)
    {
        i = GM_sscanf(pdata, "%d.%d.%d.%d,%d", ip,ip+1,ip+2,ip+3,&port);
    }

    if(i == 5)
    {
        for(i = 0; i < 4 ; i ++)
		{
			if(ip[i] > 255)
			{
				return 0;
			}
		}

        config_service_set(CFG_SERVERADDR,TYPE_STRING,pdata,GM_strlen(pdata));

        ret = NEED_OP_SAVE_FILE;
    }

    return ret;
}


u8 set_pmu_byte_turn(u8 *in , u8 *out , u8 size)
{
    u8 i = 0 , j = 0;

    j = size - 1;

    for(i = 0 ; i< size ; i++)
    {
        out[i] = in[j];

        j--;
    }

    return size;
       
}



u8 gw_get_pmu_work_mode_data(u8 *pOut)
{
	void *ptr = NULL;
	u8 mode = 0;
	u16 i,j = 0;
	//u32 utc_time = 0;
	ClockModeTimePointStruct *clock = NULL;
	WeekkModeTimePointStruct *week  = NULL;
	TrackeModeTimeValueStruct*track = NULL;
	PlatLoopModeStruct *loop        = NULL;
	PlatTimePiontModeStruct  *tp    = NULL;
	//u8 bcdtime[6];
	
    ptr = get_cur_gw_work_mode(&mode);

    switch(mode)
    {
    	case ALARM_CLOCK_MODE:
    		clock = (ClockModeTimePointStruct *)ptr;
    		
    		for(i = 0 ; i < 4 ; i++)
    		{
                if(clock->time_point[i]<2400)
                {
                    j += set_pmu_byte_turn((u8 *)&clock->time_point[i],&pOut[j],4); 
                }
    			else
    			{
                    pOut[j++] = 0xFF;
                    pOut[j++] = 0xFF;
                    pOut[j++] = 0XFF;
                    pOut[j++] = 0xFF;
                }
    		}
    		
    		
    		break;
    	case WEEK_ALARM_MODE:
    		week = (WeekkModeTimePointStruct *)ptr;

            j = set_pmu_byte_turn((u8 *)&week->day, &pOut[j], 4);

            j += set_pmu_byte_turn((u8 *)&week->time_point, &pOut[j], 4);
    		
    		break;
    	case TRACK_MODE:
    		track = (TrackeModeTimeValueStruct *)ptr;

            j = set_pmu_byte_turn((u8 *)&track->interval, &pOut[j], 4);
    		
    		break;
    	case PLATFORM_CYCLIC_MODE:
    		loop = (PlatLoopModeStruct *)ptr;

            j += set_pmu_byte_turn((u8 *)&loop->pos_mode, &pOut[j], 4);

    		j += set_pmu_byte_turn((u8 *)&loop->loop_start_tim, &pOut[j], 4);

            j += set_pmu_byte_turn((u8 *)&loop->loop_interval, &pOut[j], 4);

            j += set_pmu_byte_turn((u8 *)&loop->loop_stop_tim, &pOut[j], 4);

    	
    		break;
    	case PLATFORM_TIMEPOINT_MODE:
    		tp = (PlatTimePiontModeStruct *)ptr;
    		
    		for(i = 0; ((i < 3)&&(tp[i].utc_time_point > 0)); i ++)
    		{

                j += set_pmu_byte_turn((u8 *)&tp[i].pos_mode, &pOut[j], 4);

                j += set_pmu_byte_turn((u8 *)&tp[i].utc_time_point, &pOut[j], 4);

    		}

    		break;
    	default:
    		//j = GM_sprintf(pOut,"\r\nWork Mode Err %d:",mode);
    		break;
    }

    return j;
}

/*安装一个上传工作模式的数据包*/
void install_one_upload_mod_data(void)
{
    void *ptr = NULL;

    u8 week_idx = 0,weekid = 0,idxa = 0,index = 0,mode = 0;

    u8 pData[40] = {0};

    u32 j = 0,k = 0;

    ClockModeTimePointStruct *clock = NULL;
	WeekkModeTimePointStruct *week = NULL;
	TrackeModeTimeValueStruct*track = NULL;
    
    ptr = get_cur_gw_work_mode(&mode);

    switch(mode)
    {
        case ALARM_CLOCK_MODE:
            clock = (ClockModeTimePointStruct *)ptr;
            pData[index++] = 0x00;
            for (idxa =0; clock->time_point[idxa] < 2400; idxa++)
            {
                pData[index++] = HEX2BCD(clock->time_point[idxa]/100);//((clock->time_point[idxa]/1000)&0x0f << 4)|((clock->time_point[idxa]%100/10)&0x0f);

                pData[index++] = HEX2BCD(clock->time_point[idxa]%100);//((clock->time_point[idxa]%100/10)&0x0f << 4)|((clock->time_point[idxa]%10)&0x0f);;
            }
            break;
        case WEEK_ALARM_MODE:
            week  = (WeekkModeTimePointStruct *)ptr;
            pData[index++] = 0x02;
            weekid = 0;
			j = 10;
			k = 1;
			for(idxa = 0;idxa< 7 ; idxa++)
			{
				week_idx  = ((week->day%j)/k);
				
				if(week->day < 8 && week->day > 0)
				{
					if((weekid&(1<<(week_idx -1))) == 0)
					{
						weekid |= (1<<(week_idx - 1));
					}
				}

				j *= 10;

				k *= 10;
				
				
			} 	
            pData[index++] = weekid;
            pData[index++] = HEX2BCD(week->time_point/100);//((week->day/1000)&0x0f << 4)|((week->day%100/10)&0x0f);
            pData[index++] = HEX2BCD(week->time_point%100);
            break;
        case TRACK_MODE:
            track  = (TrackeModeTimeValueStruct *)ptr;
            if(track->interval > 0xFFFF)
            {
                pData[index++] = 0x11;
                pData[index++] = (u8)((track->interval>> 24) & 0xff);
                pData[index++] = (u8)((track->interval>> 16) & 0x0ff);
                pData[index++] = (u8)((track->interval>> 8) & 0xff);
                pData[index++] = (u8)track->interval  & 0xff;
            }
            else
            {
                pData[index++] = 0x01;
                pData[index++] = (u8)((track->interval>> 8) & 0x00ff);
                pData[index++] = (u8)track->interval  & 0x00ff;
            }
            break;
        case PLATFORM_CYCLIC_MODE:
        case PLATFORM_TIMEPOINT_MODE:
             pData[index++] = 0x03;
            break;
        default:
            index = 0;
            break;
    }

    if(index > 0 && index < 10)
    {
        GM_memcpy(s_upload_mod.data,pData,index);

        s_upload_mod.datalen = index;
    }

    return;

}

/*****************************************************************************
 函 数 名  : save_gw_work_mode_to_file
 功能描述  : 保存工作模式的相关参数
 输入参数  : u8 mode  
             u32 *in  
 输出参数  : 无
 返 回 值  : 
 日    期  : 2019年3月19日
 作    者  : Chris.Lu
*****************************************************************************/
void save_gw_work_mode_to_file(u8 mode,u32 *in)
{
	u16 max_len = 0;

	u8 value = 0 , pre_mode = 0,work_mode = 0;

    u8 para[25] = {0};

    u8 wkm[WORK_MODE_PARAM_LEN] = {0};

    work_mode = mode;

	switch(work_mode)
	{
		case ALARM_CLOCK_MODE:
	
			max_len = sizeof(ClockModeTimePointStruct);

			value = 0xff;

			break;
		case TRACK_MODE:

			max_len = sizeof(TrackeModeTimeValueStruct);

			break;
		case WEEK_ALARM_MODE:

			max_len = sizeof(WeekkModeTimePointStruct);

			break;
		case PLATFORM_TIMEPOINT_MODE :

			max_len = sizeof(PlatTimePiontModeStruct)*3;

			break;

		case PLATFORM_CYCLIC_MODE:

			max_len = sizeof(PlatLoopModeStruct);

			break;
		default:
			LOG(ERROR,"save gw work mode aram error!! mode = %d",mode);
			return;
		
	}

    config_service_get(CFG_WORK_MODE,TYPE_BYTE,&pre_mode , sizeof(work_mode));

	config_service_set(CFG_WORK_MODE,TYPE_BYTE,&work_mode , sizeof(work_mode));

	GM_memset((u8 *)wkm,value,WORK_MODE_PARAM_LEN);

	GM_memcpy((u8 *)wkm,(u8 *)in,max_len);

    config_service_set(CFG_WORK_MODE_PARAM,TYPE_ARRY,wkm,WORK_MODE_PARAM_LEN);

    para[0] = work_mode;


    /*工作模式变化上传一条位置数据*/
    if(!is_get_work_mode_from_22)
    {
        //gps_service_push_one_gps();

        install_one_upload_mod_data();

        expmu_service_destory();
    }
    
    value = gw_get_pmu_work_mode_data(&para[1]);

    if(value > 0)smsp_comm_pmu_workmode_set(para, 25);

    //goome_file_param_save();
}


u16 get_u16_data(u8* pdata)
{
    return (u32)(pdata[0] << 8) | (u32)(pdata[1]);
}

u32 get_u32_data(u8* pdata)
{
    return (u32)(pdata[0] << 24) | (u32)(pdata[1] << 16) | (u32)(pdata[2] << 8) | (u32)(pdata[3]);
}


static u8 parse_platform_cycle_mode(u8* pdata, u16 len)
{
    u16 interval = 0;
    u32 begintime = 0;

    u32 content[6] = {0};


    if(len != (u16)0x0E)
    {
        LOG(WARN,"mode:%d len:%d err!\r\n", pdata[0], len);
        return 0;
    }

    begintime = get_u32_data(pdata + 2);
    if(begintime <= 0)
    {
        return 0;
    }
    
    interval = get_u16_data(pdata + 6);
    if(interval <= 0 || interval > GPS_MAX_INTERVAL_MINUTE)
    {
        LOG(WARN,"interval:%u > GPS_MAX_INTERVAL:%u", interval, GPS_MAX_INTERVAL_MINUTE);
        return 0;
    }

    content[0] = pdata[1];

    content[1] = begintime;

    content[2] = interval;

    content[3] = get_u32_data(pdata + 8);
    
    save_gw_work_mode_to_file(PLATFORM_CYCLIC_MODE, content);


    return 1;
}

static u8 parse_platform_timepoint_mode(u8* pdata, u16 len)
{
    u32 t1 = 0;
    u32 t2 = 0;
    u32 t3 = 0;
    u32 timetosecond = 0;
    u32 content[6] = {0};
	PlatTimePiontModeStruct *tp = (PlatTimePiontModeStruct *)content;
	
    if(len != (u16)0x12)
    {
        return 0;
    }

    t1 = get_u32_data(pdata + 2);
    t2 = get_u32_data(pdata + 7);
    t3 = get_u32_data(pdata + 12);

    timetosecond = util_get_utc_time();

    if((t1 == 0) && (t2 != 0 || t3 != 0))
    {
        return 0;
    }
    
    if(t1 > 0 && t1 - timetosecond > GPS_MAX_INTERVAL_SECOND)
    {
        return 0;
    }
    
    if(t2 > 0 && t2 - t1 > GPS_MAX_INTERVAL_SECOND)
    {
        return 0;
    }

    if(t3 > 0 && t3 - t2 > GPS_MAX_INTERVAL_SECOND)
    {
        return 0;
    }

	tp[0].pos_mode = pdata[1];
	tp[0].utc_time_point = t1;

	tp[1].pos_mode = pdata[6];
	tp[1].utc_time_point = t2;

	tp[2].pos_mode = pdata[11];
	tp[2].utc_time_point = t3;

    save_gw_work_mode_to_file(PLATFORM_TIMEPOINT_MODE, content);
	
    return 1;
}

void deal_platform_mode(u8* pdata, u16 len)
{

    is_get_work_mode_from_22 = 1;
    
    if(len > 3)
    {
        u8 y = 0;

        if(pdata[0] == 0x00)  
        {
            //29 29 22 000E 00 00 5A49B948 05A0 00000000 6B 0D
            y = parse_platform_cycle_mode(pdata, len);
        }
        else if(pdata[0] == 0x01) 
        {
            y = parse_platform_timepoint_mode(pdata, len);
        }
        else
        {
            LOG(WARN,"mode:%d len:%d err!\r\n", pdata[0], len);
            is_get_work_mode_from_22 = 0;
            return;
        }

        if(y)config_service_save_to_local();

    }

    is_get_work_mode_from_22 = 0;
}

/*获取上报工作模式结构体*/
UploadWorkModeStruct *get_work_mode_upload_data(void)
{
    return &s_upload_mod;
}


/*****************************************************************************
 函 数 名  : get_old_config_para_number
 功能描述  : 获取参数字符串中的参数个数
 输入参数  : const u8 *input  
 输出参数  : 无
 返 回 值  : 
 日    期  : 2019年3月18日
 作    者  : Chris.Lu
*****************************************************************************/
int get_old_config_para_number(const u8 *input)
{
	u8 *ptr = (u8 *)input;

	s8 num = 0;

	if(!ptr) return -1;

	if((*ptr == '*')||(*ptr == '>'))
	{
		return 0;
	}

	while(*ptr != 0)
	{
		if(*ptr == ',')
		{
			num++;
		}

		if((*ptr == '*')||(*ptr == '>'))
		{
			break;
		}
		ptr++;
	}

	return num + 1;
}


/*****************************************************************************
 函 数 名  : find_special_char
 功能描述  : 找到一个特殊的字符串，并将偏移量返回
 输入参数  : const u8 *input  
             u8 chr           
 输出参数  : 无
 返 回 值  : 
 日    期  : 2019年3月18日
 作    者  : Chris.Lu
*****************************************************************************/
int find_special_char(const u8 *input , u8 chr)
{
	u8 *ptr = (u8 *)input;

	if(!ptr) return -1;

	if((*ptr == '*')||(*ptr == '>'))
	{
		return -1;
	}

	while(*ptr != 0)
	{
		
		if(*ptr == chr)
		{
			break;
		}

		if((*ptr == '*')||(*ptr == '>')||(*ptr == 0))
		{
			break;
		}
		ptr++;
	}

	if(*ptr == 0) return -1;

	if(*ptr != chr) return -1;

	return (ptr - input);
}


static u8 old_cmd_set_wkm_clock(char *pdata , u16 len)
{
    u8 ret = 0 ,m1 = 0,i = 0,idx = 0;
    
    u32 timepoint[4] = {0xffff,0xffff,0xffff,0xffff};

    idx = get_old_config_para_number((u8 *)pdata);

    if((idx <=4 )&&(idx > 0))
    {
    	for(i = 0; i < idx ; i++)
    	{
    		timepoint[i] = GM_atoi(&pdata[m1]);

    		if((timepoint[i] >= 2400)||(timepoint[i]/100 >= 24)||(timepoint[i]%100 >= 60))
    		{
    			LOG(INFO,"clock time point value error exit!! %s",&pdata[m1]);

    			return 0;
    		}

    		find_special_char((u8 *)&pdata[m1], ',');

    		m1 += (find_special_char((u8 *)&pdata[m1], ',') + 1);
    		
    		LOG(INFO,"clock(%d): %d",i,timepoint[i]);
    	}
    	
        save_gw_work_mode_to_file(ALARM_CLOCK_MODE , timepoint);

        ret = NEED_OP_SAVE_FILE;
    }
    else
    {
    	LOG(ERROR,"config Clock timepoint error-> %s",pdata);

    }

    return ret;
					
}


static u8 old_cmd_set_wkm_track_mode(char *pdata , u16 len)
{
    u8  ret = 0;
    u32 interval = 0;

    if(len > 5) return 0;

    interval = (u32)GM_atoi(pdata);
    
    if((interval > 0)&&(interval< 1440))
	{
		LOG(INFO,"config workmode TrackMode interval %d",interval);

		save_gw_work_mode_to_file(TRACK_MODE, &interval);

        ret = NEED_OP_SAVE_FILE;
	}
	else
	{
		LOG(ERROR,"config workmode TrackMode error -->%s", pdata);

	}

    return ret;
}

static u8 old_cmd_set_wkm_week_mode(char *pdata , u16 len)
{
    u8 i = 0,ret = 0, onoff = 0;
    u32 week_tim[2] = {0};

    int idx = 0;
    
    if(len  > 15) return 0;

    idx = get_old_config_para_number((const u8 *)pdata);

    if(idx == 3)
	{
		onoff = GM_atoi(pdata);

		if(onoff == 0)
		{
			LOG(ERROR,"config param WeekMode error!! -> %s",pdata);
			return 0;
		}
		/*获取到 ','的偏移量*/
		idx = find_special_char((u8 *)pdata, ',');

		if(idx < 0)
		{
			LOG(ERROR,"config param WeekDay error!! -> %s",pdata);
			return 0;
		}
		else
		{
			i = idx + 1;
		}

		if(!IS_ARABIC_NUM(pdata[i]))
		{
			LOG(ERROR,"config param err empty !!");
			return 0;
		}
		else
		{
			LOG(INFO,"config param week day %s",&pdata[i]);
		}

		if(check_same_number_in_string((u8*)&pdata[i]) == 0)
		{
			LOG(ERROR,"config param week day in same num!! break;");

			return 0;
		}
		
		week_tim[0] = GM_atoi(&pdata[i]);

		if(week_tim[0] == 0)
		{
			LOG(ERROR,"config param err week_day == 0 break");
			return 0;
		}

		idx = find_special_char((u8 *)&pdata[i], ',');

		if(idx < 0)
		{
			LOG(ERROR,"config param WeekTim error!! -> %s",pdata);
			return 0;
		}
		else
		{
			i += (idx + 1);
		}

		if(i > GM_strlen(pdata))
		{
			LOG(ERROR,"config param WeekTime len error %d > %d",i,GM_strlen(pdata));
			return 0;
		}
		
		week_tim[1] = GM_atoi(&pdata[i]);

        if((week_tim[1] >= 2400)||(week_tim[1]/100 >= 24)||(week_tim[1]%100 >= 60))
        {
            LOG(ERROR,"config Clock timepoint error-> %d",week_tim[1]);

            return 0;
        }

		LOG(INFO,"config param WeekMode %d , %d , %d",onoff,week_tim[0],week_tim[1]);

		save_gw_work_mode_to_file(WEEK_ALARM_MODE, week_tim);

        ret = NEED_OP_SAVE_FILE;
		
	}
	else
	{
		LOG(ERROR,"config Clock timepoint error-> %s",pdata);

	}

    return ret;

    
}
#if 0
static u8 old_cmd_set_wkm_plat_mode(const char *msg_buf,u16 len)
{
	int idx = 0;

	u8 ret = 0,cnt = 0,m1 = 0 ;

	u32 content[6] = {0};

	//u32 temp = 0;

	PlatLoopModeStruct *loop = NULL;

	PlatTimePiontModeStruct *t_point = NULL;

    s32 mode = 0;

    mode = GM_atoi(msg_buf);

	if(mode > 1)
	{
		LOG(ERROR,"config param err - UnkonowMode!!");
        
		return 0;
	}
	
	/*平台循环模式*/
	if(mode == 0)
	{
		m1 = 0;
		
		for(cnt = 0 ; cnt < 4 ; cnt++)
		{
			idx = find_special_char((u8 *)&msg_buf[m1], ',');

			if(idx < 0)
			{
				LOG(ERROR,"config param LoopMode %d ->[%s] err!",cnt,&msg_buf[m1]);

				return 0;
			}

			m1 += (idx + 1);
			
			content[cnt] = GM_atoi(&msg_buf[m1]);
		}

		idx = find_special_char((u8 *)&msg_buf[m1],',');

		if(idx >= 0)
		{
			LOG(ERROR,"config param PlatPoint error %s",&msg_buf[m1]);

			return 0;
		}

		loop = (PlatLoopModeStruct *)content;

		/*判断定位模式在这个范围*/
		if(loop->pos_mode  >= PLATFORM_LOC_MODE_INVAILD)
		{
			LOG(ERROR,"config param pos mode error!! mod : %d",loop->pos_mode);

			return 0;
		}

		/*判断时间间隔的有效性 0 - 10080*/
		if((loop->loop_interval == 0)||(loop->loop_interval > 10080))
		{
			LOG(ERROR,"config param loop interval err %d",loop->loop_interval);

			return 0;
		}

		if(loop->loop_start_tim == 0)
		{
			loop->loop_start_tim = (u32)util_get_utc_time();
		}
		
		for(cnt = 0; cnt < 4 ; cnt ++)
		{
			LOG(INFO,"content[%d] = %d",cnt , content[cnt]);
		}
		save_gw_work_mode_to_file(PLATFORM_CYCLIC_MODE, content);

        ret = NEED_OP_SAVE_FILE;
		
	}
	else
	{
		for(cnt = 0 ; cnt < 6 ; cnt++)
		{
			idx = find_special_char((u8 *)&msg_buf[m1], ',');

			if(idx < 0)
			{
				LOG(ERROR,"config param TimPointMode %d ->[%s] err!",cnt,&msg_buf[m1]);

				return 0;
			}

			m1 += (idx + 1);
			
			content[cnt] = GM_atoi(&msg_buf[m1]);
			
		}

		idx = find_special_char((u8 *)&msg_buf[m1],',');

		if(idx >= 0)
		{
			LOG(ERROR,"config param PlatPoint error %s",&msg_buf[m1]);

			return 0;
		}

		t_point = (PlatTimePiontModeStruct *)content;

		for(cnt = 0 ; cnt < 3 ;cnt++)
		{
			if(t_point[cnt].pos_mode >= PLATFORM_LOC_MODE_INVAILD)
			{
				LOG(ERROR,"config param TimePoint PosMode Error!!");
				
				return 0;
			}
            #if 0
			if(temp > t_point[cnt].utc_time_point)
			{
				LOG(ERROR,"config param TimePoint error!!");

				return 0;
			}
            #endif

			temp = t_point[cnt].utc_time_point;
		}

		for(cnt = 0; cnt < 6 ; cnt ++)
		{
			LOG(INFO,"content[%d] = %d",cnt , content[cnt]);
		}

		save_gw_work_mode_to_file(PLATFORM_TIMEPOINT_MODE, content);

        ret = NEED_OP_SAVE_FILE;
	}

	return ret;
}
#endif

static u8 old_cmd_set_dev_apn(char *pdata , u16 len)
{
    char apn[3*GOOME_APN_MAX_LENGTH]  = {0};
    char usr[3*GOOME_APN_MAX_LENGTH]  = {0};
    char psw[3*GOOME_APN_MAX_LENGTH]  = {0};

    u8 i = 0,ret = 0;


    if(len >= 3*GOOME_APN_MAX_LENGTH || len  == 0)return 0;

    i = GM_sscanf(pdata,"%[^,],%[^,],%s",apn,usr,psw);

    if(i > 0)
    {
        config_service_set(CFG_APN_NAME,TYPE_STRING,apn,GM_strlen(apn));

        config_service_set(CFG_APN_USER,TYPE_STRING,usr,GM_strlen(usr));

        config_service_set(CFG_APN_PWD ,TYPE_STRING,psw,GM_strlen(psw));

        ret = NEED_OP_SAVE_FILE;

    }

    return ret;
}

static u8 old_cmd_set_light_sensor(char *pdata , u16 len)
{
    s32 cnt = 0;

    u8  light_state = 0;

    cnt = GM_atoi(pdata);

    if(cnt >= 0)
    {

        if(cnt == 0)
        {
            light_state = TRG_RISING;
        }
        else
        {
            light_state = TRG_NO_ONE;
        }

        if(light_state > 0)
        {
            config_service_set(CFG_LIGHT_STATE,TYPE_BYTE,&light_state,1);

            smsp_set_exit_triger_mode(light_state);
        }


        return NEED_OP_SAVE_FILE;
    }

    return 0;
}

static u8 old_cmd_set_wifi_state(char *pdata , u16 len)
{
    s32 cnt = 0;

    u8  wifi_state = 0;

    cnt = GM_atoi(pdata);

    if(cnt >= 0)
    {
        wifi_state = cnt;

        config_service_set(CFG_WIFI_STATE,TYPE_BYTE,&wifi_state,sizeof(u8));

        if(wifi_state == 0)
        {
            led_set_wifi_state(GM_LED_OFF);

            wifi_power_off();
        }

        return NEED_OP_SAVE_FILE;
    }

    return 0;
}

static u8 old_cmd_set_time_zone(char *pdata , u16 len)
{
    s32 cnt = 0;

    u8 zone = 0;

    cnt = GM_atoi(pdata);

    if(cnt <= 12 && cnt >= -12)
    {
        zone = abs(cnt);
        
        if(cnt < 0)
        {
            zone |= 0x80;
        }

        config_service_set(CFG_TIME_ZONE,TYPE_BYTE,&zone,sizeof(u8));

        expmu_service_destory();

        smsp_set_pmu_time_zone(zone);

        return NEED_OP_SAVE_FILE;
    }

    return 0;
}

u16 get_workmode_string(char *pout)
{
	u8 mode;

	u16  i = 0,j = 0;

	PlatTimePiontModeStruct *plat_tp = NULL;
	
	ClockModeTimePointStruct *clock = NULL;

	TrackeModeTimeValueStruct *track = NULL;

	WeekkModeTimePointStruct *week = NULL;

	PlatLoopModeStruct *plat_loop = NULL;

	void *ptr = NULL;
	
	ptr = get_cur_gw_work_mode(&mode);

	switch(mode)
	{
		case ALARM_CLOCK_MODE:

			clock = (ClockModeTimePointStruct *)ptr;

			j = GM_sprintf(pout,"*R:");

			for(i = 0 ; i < 4 ; i++)
			{
				if(clock->time_point[i] < 2400)
				{
					j += GM_sprintf(&pout[j],"%04d",clock->time_point[i]);
				}
				else
				{
					 break;
				}

				pout[j++] = ',';
				
			}
			break;
		case TRACK_MODE:

			track = (TrackeModeTimeValueStruct *)ptr;

			j = GM_sprintf(pout,"*D:%03d",track->interval);

			break;
		case WEEK_ALARM_MODE:

			week = (WeekkModeTimePointStruct *)ptr;

			j = GM_sprintf(pout,"*W:1,%d,%04d",week->day,week->time_point);

			break;
		case PLATFORM_TIMEPOINT_MODE :

			plat_tp = (PlatTimePiontModeStruct *)ptr;

			j = GM_sprintf(pout,"*F:1");
	
			for(i = 0; i < 3 ; i ++)
			{
				if(plat_tp[i].utc_time_point > 0)
				{
					j += GM_sprintf(&pout[j],",%d,%d",plat_tp->pos_mode,plat_tp->utc_time_point);
				}
				else
				{
					break;
				}
			}
			
			break;

		case PLATFORM_CYCLIC_MODE:

			plat_loop = (PlatLoopModeStruct *)ptr;

			j = GM_sprintf(pout,"*F:0,%d,%d,%d,%d",plat_loop->pos_mode,plat_loop->loop_start_tim,plat_loop->loop_interval,plat_loop->loop_stop_tim);

			break;
		default:
			//ptmsg("save gw work mode aram error!! mode = %d",mode);
			break;
		
	}

	return j;
	
}



static u16 old_2929_cmd_response(char*pdata , u16 len)
{
    u16 rep_gap,j=0;

    u8 mode = 0;

    j += GM_sprintf((char*)&pdata[j], "<GM");

    if(util_is_valid_dns(config_service_get_pointer(CFG_SERVERADDR),GM_strlen(config_service_get_pointer(CFG_SERVERADDR))))
    {
        j += GM_sprintf((char*)&pdata[j], "*Q:%s",config_service_get_pointer(CFG_SERVERADDR));
    }
    else
    {
        j += GM_sprintf((char*)&pdata[j],"*U:%s",config_service_get_pointer(CFG_SERVERADDR));
    }
  
    j += GM_sprintf((char*)&pdata[j], "*A:%.25s", config_service_get_pointer(CFG_APN_NAME));

    if (GM_strlen(config_service_get_pointer(CFG_APN_USER)) > 0)
    {
        j += GM_sprintf((char*)&pdata[j], ",%.16s", config_service_get_pointer(CFG_APN_USER));
    }

    if (GM_strlen(config_service_get_pointer(CFG_APN_PWD)) > 0)
    {
        j += GM_sprintf((char*)&pdata[j], ",%.16s", config_service_get_pointer(CFG_APN_PWD));
    }
    
    j += GM_sprintf((char*)&pdata[j],"*N:%s",config_service_get_pointer(CFG_DEVICE_NUMBER));

    config_service_get(CFG_WORK_MODE,TYPE_BYTE,&mode,sizeof(u8));

    j += GM_sprintf((char*)&pdata[j], "*B:%d",mode);

    j += get_workmode_string(&pdata[j]);

    config_service_get(CFG_LIGHT_STATE,TYPE_BYTE,&mode,sizeof(u8));

    j += GM_sprintf((char*)&pdata[j], "*G:%d",mode==TRG_RISING?0:1);

    j += GM_sprintf(&pdata[j], "*GP:");
    
    if(gps_is_fixed())
    {
        j += GM_sprintf(&pdata[j], "*OK");
    }
    else
    {
        j += GM_sprintf(&pdata[j], "*NO");
    }

    j += GM_sprintf((char*)&pdata[j], "*CGREG:%d", gsm_get_creg_state());
    
    j += GM_sprintf((char*)&pdata[j], "*CSQ:%d", gsm_get_csq());

    config_service_get(CFG_REP_GAP,TYPE_SHORT,&rep_gap,sizeof(u16));

    j += GM_sprintf((char*)&pdata[j], "*gap:%d", rep_gap);

    config_service_get(CFG_TIME_ZONE,TYPE_BYTE,&mode,sizeof(u8));

    j += GM_sprintf((char*)&pdata[j], "*zone:");

    if(mode & 0x80)
    {
        j += GM_sprintf((char*)&pdata[j], "-%d",(mode&0x7F));
    }
    else
    {
        j += GM_sprintf((char*)&pdata[j], "%d",(mode&0x7F));
    }

    //j += GM_sprintf((char*)&pdata[j], "*GPS:%d,%d,%d", gps_get_satellites_inview(),gps_get_max_snr(),gps_get_satellites_good());
    
    config_service_get(CFG_WIFI_STATE,TYPE_BYTE,&mode,sizeof(u8));
    // j += GM_sprintf((char*)&pdata[j], "*1R:%d", g_Para.restartcount);
    j += GM_sprintf((char*)&pdata[j], "*wf:%d",mode );
    

    pdata[j++]='>';

    return j;
}

void delay_gprs_destory(void)
{
    gprs_destroy();
}

void delay_gprs_change_dns(void)
{
    gps_service_change_config();
}

static u8 old_2929_cmd_set(CommandReceiveFromEnum from , char* input , u16 len)
{
    u16 jval = 0,index = 0;

    u8 option = 0,ret = 0,paramlen = 0,psw = 1;

    char param[100] = {0};

    statistical_info_struct *sta = NULL;


    while(jval < len)
    {
        
        index = get_sub_cmd_id((u8 *)&input[jval]);

        if(index >= OLD_MAX_CMD) break;

        if(!psw) break;

        jval += GM_strlen(s_old_cmd_info[index].cmd);

        GM_memset(param,0,100);

        paramlen = get_param_context((u8 *)&input[jval],(u8 *)param);

        jval += paramlen;

        switch(index)
        {
            case OLD_CMD_PSW:
            {
                #if 0
                if((GM_memcmp((char *)param, (char *)config_service_get_pointer(CFG_SMS_PWD), GM_strlen(param)) != 0)&& \
                   (from != COMMAND_GPRS))
                {
                    psw = 1;
                }
                #else
                psw = 1;
                #endif
                break;
            }

            case OLD_CMD_CELL:
            {
                ret = old_cmd_set_cell_num((u8 *)param,paramlen);

                if(ret != NEED_OP_NOONE)
                {
                    option |= (1 << ret);

                    option |= (1 << NEED_OP_END_GPRS);
                }
                
                break;
            }

            case OLD_CMD_MAIN_DNS:
            {
                ret = old_cmd_set_main_dns(param,paramlen);

                if(ret != NEED_OP_NOONE)
                {
                    option |= (1 << ret);

                    option |= (1 << NEED_OP_CHANGE_DNS);
                }
            
                break;
            }

            case OLD_CMD_UDP_IP:
            {
                ret = old_cmd_set__udp_ip(param,paramlen);

                if(ret != NEED_OP_NOONE)
                {
                    option |= (1 << ret);

                    option |= (1 << NEED_OP_CHANGE_DNS);
                }
                
                break;
            }

            case OLD_DEV_APN:
            {
                ret = old_cmd_set_dev_apn(param,paramlen);

                if(ret != NEED_OP_NOONE)
                {
                    option |= (1 << ret);

                    option |= (1 << NEED_OP_END_GPRS);
                }
                break;
            }

            case OLD_WKM_CLOCK_MODE:
            {
                ret = old_cmd_set_wkm_clock(param,paramlen);
                if(ret != NEED_OP_NOONE)
                {
                    option |= (1 << ret);
                }
                break;
            }

            case OLD_WKM_TRACK_MODE:
            {
                ret = old_cmd_set_wkm_track_mode(param,paramlen);

                if(ret != NEED_OP_NOONE)
                {
                    option |= (1 << ret);
                }
                break;
            }

            case OLD_WKM_WEEK_MODE:
            {
                ret = old_cmd_set_wkm_week_mode(param,paramlen);

                if(ret != NEED_OP_NOONE)
                {
                    option |= (1 << ret);
                }
                break;
            }
            case OLD_WKM_PLAT_MODE:
            {
                #if 0
                ret = old_cmd_set_wkm_plat_mode(param,paramlen);

                if(ret != NEED_OP_NOONE)
                {
                    option |= (1 << ret);
                }
                #endif
                break;
            }
            case OLD_LIGHT_ALARM:
            {
                ret = old_cmd_set_light_sensor(param,paramlen);

                if(ret != NEED_OP_NOONE)
                {
                    option |= (1 << ret);
                }
                
                break;
            }
            case OLD_WIFI_STATE:
            {
                ret = old_cmd_set_wifi_state(param,paramlen);

                if(ret != NEED_OP_NOONE)
                {
                    option |= (1 << ret);
                }
                break;
            }
            case OLD_TIME_ZONE:
            {
                ret = old_cmd_set_time_zone(param,paramlen);

                if(ret != NEED_OP_NOONE)
                {
                    option |= (1 << ret);
                }
                break;
            }
            
        }
        
    }


    if(option & (1 << NEED_OP_SAVE_FILE))
    {
        config_service_save_to_local();

        sta = system_state_get_statis_pointer();

        switch(from)
        {
            case COMMAND_UART:
                sta->get_config_form_uart_count++;
                break;
            case COMMAND_GPRS:
                sta->get_config_from_cfg_srv_count++;
                break;
            case COMMAND_SMS:
                sta->get_config_from_sms_count++;
                break;
            default:
                break;
        }
    }


    if(option & (1 << NEED_OP_END_GPRS))
    {
        if(from == COMMAND_GPRS)
        {
            GM_StartTimer(GM_TIMER_GPRS_DESTORY_LATER, 3*TIM_GEN_1SECOND, delay_gprs_destory);
        }
        else
        {
            gps_service_destroy();
        }
    }
    else if(option & (1 << NEED_OP_CHANGE_DNS))
    {
        if(from == COMMAND_GPRS)
        {
            GM_StartTimer(GM_TIMER_GPRS_DESTORY_LATER, 3*TIM_GEN_1SECOND, delay_gprs_change_dns);
        }
        else
        {
            gps_service_change_config();
        }
    }
    
    return option;
}


GM_ERRCODE command_on_receive_data(CommandReceiveFromEnum from, char* p_cmd, u16 cmd_len, char* p_rsp, void * pmsg)
{	
	char cmd_name[CMD_NAME_MAX_LEN] = {0};
	char* p_cmd_content = NULL;
	char para_num = 0;
    u16 jval = 0;
	CommandID cmd_id = CMD_CMD_MAX;
    //gm_sms_new_msg_struct* p_sms = NULL;
	//U16 lang = 0;

	if (NULL == p_cmd || 0 == cmd_len || cmd_len > (CMD_MAX_LEN - 1) || NULL == p_rsp)
	{
		return GM_PARAM_ERROR;
	}

	LOG(INFO,"From %d received cmd:%s",from,p_cmd);
	
	p_cmd_content = (char*)GM_MemoryAlloc(cmd_len + 1);
	GM_memcpy(p_cmd_content, p_cmd, cmd_len);
	p_cmd_content[cmd_len] = 0;

	util_remove_char((U8*)p_cmd_content,cmd_len,' ');
	util_remove_char((U8*)p_cmd_content,cmd_len,'\r');
	util_remove_char((U8*)p_cmd_content,cmd_len,'\n');
	cmd_len = GM_strlen(p_cmd_content);
	

	command_scan((char*)p_cmd_content, "s", cmd_name);
	util_string_upper((U8*)cmd_name,GM_strlen(cmd_name));
	cmd_id = get_cmd_id((char *)cmd_name);
	switch (cmd_id)
	{
        case CMD_2929_SET:
        {
            old_2929_cmd_set(from,&p_cmd_content[GM_strlen(cmd_name)],cmd_len - GM_strlen(cmd_name));

            jval = old_2929_cmd_response(p_rsp,CMD_MAX_LEN);

            jval += GM_sprintf(&p_rsp[jval], "\r\n");

            jval += GM_sprintf(&p_rsp[jval], set_success_rsp(from));
        }
        break;
        
        case CMD_WRITEIMEI:
		{	            
			char imei[32] = {0};

            if(from == COMMAND_GPRS)
			{
				break;
            }
            
            para_num = command_scan((char*)p_cmd_content, "s;s", cmd_name,imei);
			if(para_num == 2)
			{
				if (GM_SUCCESS == gsm_set_imei((U8*)imei))
				{
					GM_strcpy(p_rsp, set_success_rsp(from));
				}
				else
				{
					GM_strcpy(p_rsp, set_fail_rsp(from));
				}
			}
			else
			{
				GM_strcpy(p_rsp, set_fail_rsp(from));
			}
            
		}
        break;
        
        case CMD_IMEI:
		{
			para_num = command_scan((char*)p_cmd_content, "s", cmd_name);
			if (para_num == 1)
			{	
				char imei[32] = {0};
				gsm_get_model_imei((U8*)imei);
	            GM_snprintf((char*)p_rsp, CMD_MAX_LEN, "IMEI:%s", imei);
			}
			else
			{
				GM_strcpy(p_rsp, set_fail_rsp(from));
			}
		}
		break;
		
		case CMD_ICCID:
		{
			para_num = command_scan((char*)p_cmd_content, "s", cmd_name);
			if (para_num == 1)
			{
				char iccid[32] = {0};
				gsm_get_iccid((U8*)iccid);
				GM_snprintf((char*)p_rsp, CMD_MAX_LEN, "ICCID:%s", iccid);
			}
			else
			{
				GM_strcpy(p_rsp, set_fail_rsp(from));
			}
		}
		break;
		
		case CMD_IMSI:
		{
			para_num = command_scan((char*)p_cmd_content, "s", cmd_name);
			if (para_num == 1)
			{
				char imsi[32] = {0};
				gsm_get_imsi((U8*)imsi);
				GM_snprintf((char*)p_rsp, CMD_MAX_LEN, "IMSI:%s", imsi);
			}
			else
			{
				GM_strcpy(p_rsp, set_fail_rsp(from));
			}
		}
		break;
        
        case CMD_SYSTEM_RESET:
		{
			para_num = command_scan((char*)p_cmd_content, "s", cmd_name);
			if (para_num == 1)
			{
                smsp_set_pmu_reset();
                
				//延时重启
				GM_StartTimer(GM_TIMER_CMD_REBOOT, 5*TIM_GEN_1SECOND, cmd_reboot);
				
				GM_strcpy(p_rsp, "System is restaring");

			}
			else
			{
				//错误
				GM_strcpy(p_rsp, set_fail_rsp(from));
			}
		}
		break;
        
        case CMD_LOG_LEVEL:
		{
            LogLevel log_level = WARN;
            LogLevel upload_level = FATAL;
			para_num = command_scan((char*)p_cmd_content, "s;ii", cmd_name,&log_level,&upload_level);
			if(para_num == 1)
			{
                log_service_get_level(&log_level, &upload_level);
				GM_snprintf((char*)p_rsp, CMD_MAX_LEN, "Log level:%d , Upload level:%d",log_level, upload_level);
			}
			else if(para_num == 2)
			{
                upload_level = FATAL;
				if (log_level <= FATAL)
				{
					log_service_set_level(log_level, upload_level);
					GM_memcpy(p_rsp, set_success_rsp(from), CMD_MAX_LEN);
				}
				else
				{
					GM_memcpy(p_rsp, set_fail_rsp(from), CMD_MAX_LEN);
				}
			}
			else if(para_num == 3)
			{
				if (log_level <= FATAL && upload_level > DEBUG && upload_level <= FATAL)
				{
					log_service_set_level(log_level, upload_level);
					GM_memcpy(p_rsp, set_success_rsp(from), CMD_MAX_LEN);
				}
				else
				{
					GM_memcpy(p_rsp, set_fail_rsp(from), CMD_MAX_LEN);
				}
			}
			else
			{
				GM_memcpy(p_rsp, set_fail_rsp(from), CMD_MAX_LEN);
			}
		}
		break;
        
        case CMD_L3_SLEEP:
        {
            para_num = command_scan((char*)p_cmd_content, "s", cmd_name);
			if (para_num == 1)
			{
                if(expmu_service_is_working())
                {
                    GM_memcpy(p_rsp, set_success_rsp(from), CMD_MAX_LEN);
                    
                    smsp_comm_pmu_into_sleep();
                }
                else
                {
                    GM_memcpy(p_rsp, set_fail_rsp(from), CMD_MAX_LEN);
                }
                
            }
        }
        break;
        
        case CMD_POWERON_SEND_COUNT:
        {
            U16 pwr_on_rep_cnt = 0;
            
            para_num = command_scan((char *)p_cmd_content,"s;i",cmd_name,&pwr_on_rep_cnt);

            if(para_num == 2)
            {
                config_service_set(CFG_PWR_REP_CNT,TYPE_SHORT,&pwr_on_rep_cnt,sizeof(u16));
            }

            config_service_get(CFG_PWR_REP_CNT,TYPE_SHORT,&pwr_on_rep_cnt,sizeof(u16));
            
            GM_sprintf(p_rsp,"%s,%d",cmd_name,pwr_on_rep_cnt);
        }
        break;
        
        case CMD_CHPWD:
        {
            //TODO
        }
        break;
        
        case CMD_REPORT_GAP:
        {
            U16 pwr_on_rep_gap = 0;
            
            para_num = command_scan((char *)p_cmd_content,"s;i",cmd_name,&pwr_on_rep_gap);

            if(para_num == 2)
            {
                config_service_set(CFG_REP_GAP,TYPE_SHORT,&pwr_on_rep_gap,sizeof(u16));
            }

            config_service_get(CFG_REP_GAP,TYPE_SHORT,&pwr_on_rep_gap,sizeof(u16));

            GM_sprintf(p_rsp,"%s,%d",cmd_name,pwr_on_rep_gap);
            
        }
        break;
		
		case CMD_PMTK:
		{
			char pmtk_cmd[20] = {0};
			char pmtk_para1[20] = {0};
			char pmtk_para2[20] = {0};
			char pmtk_para3[20] = {0};
			char pmtk_para4[20] = {0};
			char pmtk_para5[20] = {0};
			para_num = command_scan((char*)p_cmd_content, "s;ssssss", cmd_name,pmtk_cmd,pmtk_para1,pmtk_para2,pmtk_para3,pmtk_para4,pmtk_para5);
			
			if(para_num == 7)
			{
				char pmtk_cmd_full[100] = {0};
				GM_snprintf(pmtk_cmd_full, 50, "%s,%s,%s,%s,%s,%s", pmtk_cmd,pmtk_para1,pmtk_para2,pmtk_para3,pmtk_para4,pmtk_para5);
				gps_write_mtk_cmd(pmtk_cmd_full);
			}
			else if(para_num == 6)
			{
				char pmtk_cmd_full[100] = {0};
				GM_snprintf(pmtk_cmd_full, 50, "%s,%s,%s,%s,%s", pmtk_cmd,pmtk_para1,pmtk_para2,pmtk_para3,pmtk_para4);
				gps_write_mtk_cmd(pmtk_cmd_full);
			}
			else if(para_num == 5)
			{
				char pmtk_cmd_full[100] = {0};
				GM_snprintf(pmtk_cmd_full, 50, "%s,%s,%s,%s", pmtk_cmd,pmtk_para1,pmtk_para2,pmtk_para3);
				gps_write_mtk_cmd(pmtk_cmd_full);
			}
			else if(para_num == 4)
			{
				char pmtk_cmd_full[50] = {0};
				GM_snprintf(pmtk_cmd_full, 50, "%s,%s,%s", pmtk_cmd,pmtk_para1,pmtk_para2);
				gps_write_mtk_cmd(pmtk_cmd_full);
			}
			else if(para_num == 3)
			{
				char pmtk_cmd_full[50] = {0};
				GM_snprintf(pmtk_cmd_full, 50, "%s,%s", pmtk_cmd,pmtk_para1);
				gps_write_mtk_cmd(pmtk_cmd_full);
			}
			else if(para_num == 2)
			{
				gps_write_mtk_cmd(pmtk_cmd);
			}
			else
			{
				GM_memcpy(p_rsp, set_fail_rsp(from), CMD_MAX_LEN);
			}
		}
		break;

        case CMD_TEST:
		{	
			if (COMMAND_SMS == from)
			{
				GM_memcpy(p_rsp, set_fail_rsp(from), CMD_MAX_LEN);
			}
			else
			{
				auto_test_create(false);
				GM_memcpy(p_rsp, set_success_rsp(from), CMD_MAX_LEN);
			}
		}
		break;

		case CMD_EXIT:
		{
			auto_test_destroy();
			GM_memcpy(p_rsp, set_success_rsp(from), CMD_MAX_LEN);
		}
		break;

		case CMD_FACTORY_ALL:
		case CMD_FACTORY:
		{
			para_num = command_scan((char*)p_cmd_content, "s", cmd_name);
			if (para_num == 1)
			{
				//恢复出厂设置
				config_service_restore_factory_config(cmd_id == CMD_FACTORY_ALL);
				agps_service_delele_file();
				system_state_reset();
				if (cmd_id == CMD_FACTORY_ALL)
				{
                    expmu_service_destory();
					auto_test_reset();
                    smsp_set_fram_default_value();
				}
				
				//延时重启
				hard_ware_reboot(GM_REBOOT_CMD,5);
				GM_memcpy(p_rsp, "System is recovering and restarting.", CMD_MAX_LEN);
				
			}
			else
			{
				//错误
				GM_memcpy(p_rsp, set_fail_rsp(from), CMD_MAX_LEN);
			}
		}
		break;

        case CMD_VER:
		{
			para_num = command_scan((char*)p_cmd_content, "s", cmd_name);
			if (para_num == 1)
			{	
				char kernel_version[80];
                char expmu_version[100] = {0};
				
			    GM_memset(kernel_version, 0, sizeof(kernel_version));
			    GM_ReleaseVerno((U8*)kernel_version);
				GM_sprintf(kernel_version + GM_strlen(kernel_version), "(%s)",GM_BuildDateTime());
                expmu_get_version_string(expmu_version);
				GM_snprintf((char*)p_rsp, CMD_MAX_LEN, "Version:%s(%s),kernel:%s,check sum:%4X,mcu ver %s",VERSION_NUMBER,SW_APP_BUILD_DATE_TIME,kernel_version,system_state_get_bin_checksum(),expmu_version);
			}
			else
			{
				GM_memcpy(p_rsp, set_fail_rsp(from), CMD_MAX_LEN);
			}
		}
        break;

        case CMD_DEVICE_TYPE:
        {
			char dev_type_str[MAX_DEVICE_STRING] = {0};
			para_num = command_scan((char*)p_cmd_content, "s;s", cmd_name,&dev_type_str);
			if (para_num == 1)
			{
                config_service_get(CFG_DEV_TYPE, TYPE_STRING,  dev_type_str, sizeof(dev_type_str));
                
				GM_snprintf((char*)p_rsp, CMD_MAX_LEN, "Device type:%s",dev_type_str);
			}
			else if(para_num == 2)
			{

                config_service_set(CFG_DEV_TYPE, TYPE_STRING,  dev_type_str, sizeof(dev_type_str));

                GM_memcpy(p_rsp, set_success_rsp(from), CMD_MAX_LEN);

                config_service_save_to_local();
                #if 0
				//设置设备类型
				if (GM_memcmp(dev_type_str,"GW01H",5) != 0)
				{
					GM_memcpy(p_rsp, set_fail_rsp(from), CMD_MAX_LEN);
				}
				else
				{
					GM_memcpy(p_rsp, set_success_rsp(from), CMD_MAX_LEN);
				}
                #endif
			}
			else
			{
				GM_memcpy(p_rsp, set_fail_rsp(from), CMD_MAX_LEN);
			}
		}
        break;

        case CMD_RECORD:
		{
			auto_test_get_result(true,p_rsp);
		}
		break;

		case CMD_RESULT:
		{
			auto_test_get_result(false,p_rsp);
		}
		break;

        case CMD_DEVNO:
        {
            char dev_num[17] = {0};
            
            para_num = command_scan((char*)p_cmd_content, "s;s", cmd_name,&dev_num);
            
			if (para_num == 1)
			{
				GM_snprintf((char*)p_rsp, CMD_MAX_LEN, "Device No:%s",config_service_get_pointer(CFG_DEVICE_NUMBER));
			}
			else if(para_num == 2)
			{
          	    if(util_string_number_counter((const u8 *)dev_num) == 11)
                {
                    if((GM_memcmp(dev_num, "13000000000", 11) >= 0) &&\
                       (GM_memcmp(dev_num, "14599999999", 11) <= 0))
                   {
                        config_service_set(CFG_DEVICE_NUMBER,TYPE_STRING,(U8 *)dev_num,GM_strlen(dev_num));

                        LOG(INFO,"get cell num %s",dev_num);

                        config_service_save_to_local();

                        GM_memcpy(p_rsp, set_success_rsp(from), CMD_MAX_LEN);
                   }
                }
			}
			else
			{
				GM_memcpy(p_rsp, set_fail_rsp(from), CMD_MAX_LEN);
			}
        }
        break;
		
		default:
		{
			char wrong_command_rsp_english[CMD_MAX_LEN] = "Do not support the command!";
		    GM_memcpy(p_rsp, wrong_command_rsp_english, GM_strlen(wrong_command_rsp_english));
		}
	}
	
	do
	{
		JsonObject* p_log = json_create();
		json_add_string(p_log, "event", "command");
		json_add_int(p_log, "from", from);
		json_add_string(p_log, "request", p_cmd_content);
		json_add_string(p_log, "response", p_rsp);
		log_service_upload(INFO, p_log);
	}while (0);
	
	GM_MemoryFree(p_cmd_content);
	p_cmd_content = NULL;
	return GM_SUCCESS;
}

static void cmd_reboot(void)
{
	hard_ware_reboot(GM_REBOOT_CMD, 1);
}

//返回参数个数,-1——失败；0——无参数
static char command_scan(const char* p_command, const char* p_format, ...)
{
    bool para_num = false;
    bool optional = false;
	const char* p_field = p_command;
	char type = 0;
	S32 value_32 = 0;
	char* p_buf = NULL;
    char switch_text[8] = {0};
	
	U8 index = 0;
	
	
    va_list ap;
    va_start(ap, p_format);

    while (*p_format) 
	{
        type = *p_format++;

        if (type == ';') 
		{
            // 后面所有的域都是可选的
            optional = true;
            continue;
        }

        if (!p_field && !optional) 
		{
            goto parse_error;
        }

        switch (type) 
		{
			case 'c': 
			{ 
				char value = 0;
                if (p_field && util_isprint((U8)*p_field) && *p_field != ',' && *p_field != '#')
                {
                    value = *p_field;
					para_num++;
                }
                *va_arg(ap, char*) = value;
            } 
			break;
			
            case 'w': 
			{ 
                index = 0;
                if (p_field) 
				{
                    while (util_isprint((U8)*p_field) && *p_field != ',' && *p_field != '#')
                    {
                        switch_text[index++] = *p_field++;
                    }
                }
				switch_text[index] = 0;

				util_string_upper((U8*)switch_text,GM_strlen(switch_text));

				if (!GM_strcmp(switch_text, "ON") || !GM_strcmp(switch_text, "1"))
				{
					*va_arg(ap, char*) = true;
					para_num++;
				}
				else if (!GM_strcmp(switch_text, "OFF") || !GM_strcmp(switch_text, "0"))
				{
					*va_arg(ap, char*) = false;
					para_num++;
				}
				else
				{
				}
				
            } 
			break;
			
			// Integer value, default 0 (S32).
            case 'i': 
			{ 
                value_32 = 0;

                if (p_field && util_isdigit(*p_field)) 
				{	
                    char *endptr;
                    value_32 = util_strtol(p_field, &endptr);
                    if (util_isprint((U8)*endptr) && *endptr != ',' && *endptr != '#')
                    {
                    	
                        goto parse_error;
                    }
					para_num++;
                }
                *va_arg(ap, S32*) = value_32;
				
            } 
			break;

			// String value (char *).
            case 's': 
			{ 
                p_buf = va_arg(ap, char*);

                if (p_field && util_isprint((U8)*p_field) && *p_field != ',' && *p_field != '#' && *p_field != '*') 
				{
                    while (util_isprint((U8)*p_field) && *p_field != ',' && *p_field != '#' && *p_field != '*')
                    {
                        *p_buf++ = *p_field++;
                    }
					para_num++;
                }
                *p_buf = '\0';
            } 
			break;

            default:
			{ 
                goto parse_error;
            }
        }

        /* Progress to the next p_field. */
    	while (util_isprint((U8)*p_command) && *p_command != ',' && *p_command != '#')
    	{
        	p_command++;
    	}
    	/* Make sure there is a p_field there. */ 
    	if (*p_command == ',') 
		{ 
        	p_command++;
        	p_field = p_command;
    	} 
		else 
		{
        	p_field = NULL;
    	}
    }

parse_error:
    va_end(ap);
    return para_num;
}

