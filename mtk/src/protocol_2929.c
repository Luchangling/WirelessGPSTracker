
#include "gps.h"
#include <stdio.h>
#include "applied_math.h"
#include "utility.h"
#include "gm_stdlib.h"
#include "gm_memory.h"
#include "gm_type.h"
#include "gsm.h"
#include "log_service.h"
#include "config_service.h"
#include "hard_ware.h"
#include "protocol.h"
#include "protocol_2929.h"
#include "gps.h"
#include "gps_service.h"
#include "sms.h"
#include "gm_timer.h"
#include "system_state.h"
#include "command.h"
#include "expmu.h"
#include "gm_fs.h"
#include "wifi_service.h"
#include "stdlib.h"
#include "math.h"
#include "expmu_service.h"
#include "calc_wkp_time.h"
#include "auto_test.h"

#define PROTOCOL_DATA_SAVE L"Z:\\goome\\ProtocolDat\0"

#define DATA_VALID_FLAG 0xA55A


SaveProtocolDataStruct s_protocol;


typedef enum
{
    PROTOCOL_2929_CMD_CENTER_CONFIR = 0x21,        //中心确认包(下行)
    PROTOCOL_2929_CMD_INSTRUCTION_QUERY = 0x22,    //80携带0xF002平台指令查询的应答包(下行)
    PROTOCOL_2929_CMD_Name = 0x30,                 //点名查车(下行)
    PROTOCOL_2929_CMD_CHECK_STATUS = 0x31,         //查状态(下行)
    PROTOCOL_2929_CMD_RESET = 0x32,                //复位重启(下行)
    PROTOCOL_2929_CMD_SET_ACC_INTERVER = 0x34,     //设置ACC开间隔(下行)
    PROTOCOL_2929_CMD_CANCEL_ALARM = 0x37,         //取消报警位(下行)
    PROTOCOL_2929_CMD_SCHEDYLE_TEXT = 0x3A,        //下发调度文本信息,设置主机输出控制方式(下行)
    PROTOCOL_2929_CMD_VIEW_VERSION = 0x3D,         //查看版本号(下行)
    PROTOCOL_2929_CMD_VIEW_MONITOR = 0x3E,         //监听(下行)
    PROTOCOL_2929_CMD_SET_OVERAPEED_ALARM = 0x3F,  //设置超速报警(下行)
    PROTOCOL_2929_CMD_MODIFY_IP= 0x69,             //修改UDP IP
    PROTOCOL_2929_CMD_TIMING = 0x7B,               //定时定次回传(下行)
    PROTOCOL_2929_CMD_LOCATE = 0x80,               //位置信息汇报(上行)
    PROTOCOL_2929_CMD_CONFIRM = 0x85,              //车载终端确认包(上行)
    PROTOCOL_2929_CMD_SCHOOL_TIME = 0xD6,          //上发便携终端校时协议(上行)
    PROTOCOL_2929_CMD_SETTING_PARAMETERS = 0xD8    //上发申请设置参数(上行)
}CmdType;


typedef enum
{
    PROTOCOL_2929_ECMD_REMAINING_NUM = 0x0008,     //剩余条数
    PROTOCOL_2929_ECMD_EALARM_STATUS = 0x0089,     //扩展报警状态
    PROTOCOL_2929_ECMD_DEVICE_MODEL = 0x00A5,      //终端型号
    PROTOCOL_2929_ECMD_CELL_STAT = 0x00A9,         //多基站
    PROTOCOL_2929_ECMD_ICCID = 0x00B2,             //ICCID
    PROTOCOL_2929_ECMD_NEXT_TIME_UPDATE = 0xF001,  //下次唤醒时间
    PROTOCOL_2929_WORK_MODE_UPDTAE = 0xF000,       //更新工作模式
    PROTOCOL_2929_PLATFROM_MODE_FLG = 0xF002,      //平台模式标志
    PROTOCOL_2929_ECMD_WIFI_LOCATION = 0x00B9,     //WIFI信息
}ExpandCmdType;

#define	SYS_DEV_STATE_LIGHT_ALARM				0x00001000


static GM_ERRCODE protocol_2929_get_num_to_pseudo_ip(u8 *pseudo_ip);
static void protocol_2929_center_confirmation_parse(u8 *pdata , u16 len);
static void protocol_2929_schedyle_test_parse(u8 *pdata , u16 len);
static void protocol_2929_instruction_query_parse(u8 *pdata , u16 len);

static void protocol_2929_pack_gps_data(u8 *rData, u16 *idx, u16 len,const GPSData *gps);
static void protocol_2929_pack_additional_lbs(u8 *rData, u16 *idx, u16 len,const GPSData *gps);
static void protocol_2929_pack_remaining_number(u8 *rData, u16 *idx, u16 len);
static void protocol_2929_pack_device_model(u8 *rData, u16 *idx, u16 len);
static void protocol_2929_pack_iccid(u8 *rData, u16 *idx, u16 len);
static void protocol_2929_pack_extended_alarm_status(u8 *rData, u16 *idx, u16 len);


void protocol_2929_center_confirmation_parse(u8 *pdata , u16 len)
{
    UploadWorkModeStruct *up = NULL;

    if(auto_test_is_working() == false)
    {
        config_service_set_reming_gps_count();

        config_service_save_to_local();

        system_state_set_static_info();
    }
    else
    {
        auto_test_set_gps_location_counts();
    }
    

    gps_service_clear_one_gps();

    up = get_work_mode_upload_data();

    if(up->upload_flg)
    {
        GM_memset((u8 *)up, 0 , sizeof(UploadWorkModeStruct));
    }
    
}

void protocol_2929_schedyle_test_parse(u8 *pdata , u16 len)
{
   u8  resp[500] = {0};
   u16 dlen  = 0;
   
   command_on_receive_data(COMMAND_GPRS,(char*)pdata,len,(char*)resp,NULL);

   GM_memset(resp,0,500);

   dlen = protocol_2929_install_vehicle_terminal_confirmation_packet(resp);

   gps_service_send_one_device_ack(resp,dlen);
}

void protocol_2929_instruction_query_parse(u8 *pdata , u16 len)
{
    UploadWorkModeStruct *up = NULL;

    if(auto_test_is_working() == false)
    {
        config_service_set_reming_gps_count();

        config_service_save_to_local();

        system_state_set_static_info();
    }
    else
    {
        auto_test_set_gps_location_counts();
    }

    deal_platform_mode(pdata,len);
    
    gps_service_clear_one_gps();

    up = get_work_mode_upload_data();

    if(up->upload_flg)
    {
        GM_memset((u8 *)up, 0 , sizeof(UploadWorkModeStruct));
    }
}

void protocol_2929_parse_msg(u8 *pdata , u16 len)
{
    u16 j = 0;
    u8 k = 0;
    u8 check_sum=0;
    u16 cmd = 0;
    u16 buf_len;
    
	if(pdata[0]!=PROTOCOL_HEADER_2929||pdata[1]!=PROTOCOL_HEADER_2929) 
	{
	    LOG(WARN,"clock(%d) protocol_2929_parse_msg assert (Starter:(%02x,%02x) failed.", util_clock(), pdata[0],pdata[1]);
	    return;//起始符错误Starter
    }

    j=((u16)pdata[3]<<8)+pdata[4];
    if(pdata[j+4]!=0x0d)
    {
        LOG(WARN,"clock(%d) protocol_2929_parse_msg assert (Terminator:(%02x) failed.", util_clock(), pdata[j+4]);
        return;   //结束符错误Terminator
    }
    
    for(k=0; k<=(j+3); k++)
    {
        check_sum^=pdata[k];
    }

    if(check_sum!=0) 
    {
        LOG(WARN,"clock(%d) protocol_jt_parse_msg assert(checksum(%02x=%02x, %02x, %02x)) failed.", util_clock(), check_sum, pdata[len-3], pdata[len-2], pdata[len-1]);
        return;//校验错误
    }
    
    cmd=pdata[2]; // 指令码
    buf_len = (j-6);
    
    switch(cmd)
    {
        case PROTOCOL_2929_CMD_CENTER_CONFIR:
        protocol_2929_center_confirmation_parse(&pdata[5],buf_len);
        break;
        
        case PROTOCOL_2929_CMD_INSTRUCTION_QUERY:
        protocol_2929_instruction_query_parse(&pdata[5],buf_len);
        break;
        
        case PROTOCOL_2929_CMD_SCHEDYLE_TEXT:
        protocol_2929_schedyle_test_parse(&pdata[9],buf_len);
        break;
        
        case PROTOCOL_2929_CMD_VIEW_VERSION:
        break;
        
        case PROTOCOL_2929_CMD_TIMING:
        break;
        
        default:
        LOG(DEBUG,"clock(%d) protocol_2929_parse_msg default(%02x).", util_clock(),cmd);
        break;
    }
}

GM_ERRCODE protocol_2929_get_num_to_pseudo_ip(u8 *pseudo_ip)
{
    u8 buff[24] = {0};
    u8 len = 0;

	u8 i = 0;
	
    config_service_get(CFG_DEVICE_NUMBER,TYPE_STRING,buff,sizeof(buff));

    len = GM_strlen((char*)buff); 


    if (len == GM_DEV_NUMBER_LEN)
    {
        i=util_chr(buff[0])*100+util_chr(buff[1])*10+util_chr(buff[2]);
        i-=130;
		
        pseudo_ip[0]=util_chr(buff[3])*10+util_chr(buff[4]);
        if((GET_BIT3(i))>0)
        {
			SET_BIT7(pseudo_ip[0]);
        }
		
        pseudo_ip[1]=util_chr(buff[5])*10+util_chr(buff[6]);
        if((GET_BIT2(i))>0)
        {
			SET_BIT7(pseudo_ip[1]);
		}
		
        pseudo_ip[2]=util_chr(buff[7])*10+util_chr(buff[8]);
        if((GET_BIT1(i))>0)
        {
			SET_BIT7(pseudo_ip[2]);
		}
		
        pseudo_ip[3]=util_chr(buff[9])*10+util_chr(buff[10]);
        if((GET_BIT0(i))>0)
        {
            SET_BIT7(pseudo_ip[3]);
		}
		return GM_SUCCESS;
    }
	else
	{
	    LOG(WARN,"clock(%d) dev number len:%d - %s",util_clock(),len,buff);
	    return GM_NOT_INIT;
	}

}

static u16 protocol_2929_send_packet(u8 code, u8 *send, u16 *idx, u16 len, u8 *pdata)
{
    GM_ERRCODE ret = GM_SUCCESS;
    u16 index = 0;
    u8 j = 0;
    u8 xor=0;
    
    pdata[index++] = PROTOCOL_HEADER_2929;
    pdata[index++] = PROTOCOL_HEADER_2929;
    pdata[index++] = code;
    pdata[index++] = ((*idx)+6)>>8;//长度高位
    pdata[index++] = ((*idx)+6)&0xff;//长度低位

    if(GM_SUCCESS != (ret = protocol_2929_get_num_to_pseudo_ip(&pdata[index])))
    {
        LOG(INFO,"clock(%d) protocol_2929_get_num_to_pseudo_ip can not get pseudo ip, ret:%d.", util_clock(), ret);
    }
	index += 4;
	
	for(j=0; j<(*idx); j++) 
	{
	    pdata[index++] = *(send++);
	}
	
    for(j=0; j<index; j++)
    {
        xor ^= pdata[j];
    }
    pdata[index++] = xor;
    pdata[index++] = PROTOCOL_TAIL_2929;
    return index;
}

static bool get_gps_valid_position(GPSData *gps)
{
    time_t sys_time = 0;

    sys_time = util_get_utc_time();

    if(gps_get_last_data(gps) == true)
    {
        if(abs(sys_time - gps->gps_time) < 300)
        {
            return true;
        }
    }

    GM_memset((u8 *)gps,0,sizeof(GPSData));

    return false;
}

static void protocol_2929_pack_gps_data(u8 *rData, u16 *idx, u16 len ,const GPSData *gps)
{
    //GPSData gps;
    u32 latitudev = 0;
    u32 longitudev = 0;
    u16 gps_speed = 0;
    u16 gps_angle = 0;
    u8 bcd_tim[6] = {0};
    u8 zone = 0;
    time_t gps_time = 0;
    u8 i = 0;
    u16 jval;
	int integer = 0;	
	double decimal = 0;  


    
    if((*idx) + 19 > len)
    {
        LOG(WARN,"clock(%d) protocol_2929_pack_gps_data assert(len(%d)) failed.", util_clock(), len);
        return;
    }
    
    GM_memset(&gps_time,0,sizeof(gps_time));
    //GM_memset(&gps,0,sizeof(gps));
	//is_valid_gps = get_gps_valid_position(&gps);
    if(gps == NULL)
    {
        gps_time = util_get_utc_time();
    }
    else
    {
        gps_time = gps->gps_time;
    }

    /*2929协议固定东八区时间*/
    zone = 8;//config_service_get_zone();
    
    util_utc_sec_to_bcdtime_base2000(gps_time, bcd_tim, zone);
    
	for(i = 0; i < sizeof(bcd_tim); ++i)
    {
        rData[(*idx)++] = bcd_tim[i];
    }

    
    if (gps != NULL)//定位状态
    {
        integer = (int)fabs(gps->lat);
        decimal = fabs(gps->lat)-integer;
        latitudev = (((float)integer*100) + (decimal*60))*1000;
        

    	integer = (int)fabs(gps->lng);
        decimal = fabs(gps->lng)-integer;
        longitudev = (((float)integer*100) + (decimal*60))*1000;
        
        jval = (*idx);
        DWord2BCD(latitudev,4,&rData[*idx]);
        rData[jval] |= ((gps->lat < 0)?0x80:0);
    	(*idx) += 4;

        jval = (*idx);
    	DWord2BCD(longitudev,4,&rData[*idx]);
        rData[jval] |= ((gps->lng < 0)?0x80:0);
    	(*idx) += 4;

        
        gps_speed = ((u16)gps->speed > 180) ? 180 : ((u16)gps->speed);
        gps_angle = ((u16)gps->course > 360) ? 0 : ((u16)gps->course);

        
        gps_speed = HEX2BCD(gps_speed);
        rData[(*idx)++] = BHIGH_BYTE(gps_speed);
        rData[(*idx)++] = BLOW_BYTE(gps_speed);

    	gps_angle = HEX2BCD(gps_angle);
        rData[(*idx)++] = BHIGH_BYTE(gps_angle);
        rData[(*idx)++] = BLOW_BYTE(gps_angle);
        
        rData[(*idx)++] = 0x80;
    }
    else
    {
        GM_memset(&rData[*idx],0,12);

        (*idx) += 12;
        
        rData[(*idx)++] = 0;
    }
}

static void protocol_2929_pack_get_state(u8 *rData, u16 *idx, u16 len)
{

    if((*idx) + 15 > len)
    {
        LOG(WARN,"clock(%d) protocol_2929_pack_get_state assert(len(%d)) failed.", util_clock(), len);
        return;
    }
    
    GM_memset(&rData[*idx],0,15);
    (*idx) += 15;
}

static void protocol_2929_pack_additional_lbs(u8 *rData, u16 *idx, u16 len,const GPSData *gps)
{
    u8 i = 0;
    GM_ERRCODE ret;
    gm_cell_info_struct lbs;
    //GPSData gps;
    
    if (gps != NULL)
    {
        return;
    }
    
    GM_memset(&lbs,0, sizeof(lbs));

    if((*idx) + 0x26 > len)
    {
        LOG(WARN,"clock(%d) protocol_2929_pack_additional_lbs assert(len(%d)) failed.", util_clock(), len);
        return;
    }
    ret = gsm_get_cell_info(&lbs);
    LOG(DEBUG,"clock(%d) protocol_2929_pack_additional_lbs ret(%d) lbs(%d).", util_clock(), ret, lbs.nbr_cell_num);

    // LEN
    rData[(*idx)++] = 0;
    rData[(*idx)++] = 0x24;
    // CMD
    rData[(*idx)++] = (PROTOCOL_2929_ECMD_CELL_STAT>>8) & 0xff;
    rData[(*idx)++] = PROTOCOL_2929_ECMD_CELL_STAT & 0xff;

    rData[(*idx)++] = (u8)(lbs.serv_info.mcc >> 8);
    rData[(*idx)++] = (u8)(lbs.serv_info.mcc);

    rData[(*idx)++] = (lbs.serv_info.mnc);

    if (lbs.nbr_cell_num > 5)
    {
        lbs.nbr_cell_num = 5;
    }
    
    rData[(*idx)++] = lbs.nbr_cell_num + 1;

	rData[(*idx)++] = (u8)(lbs.serv_info.lac >> 8);
    rData[(*idx)++] = (u8)(lbs.serv_info.lac);

    rData[(*idx)++] = (u8)(lbs.serv_info.ci >> 8);
    rData[(*idx)++] = (u8)(lbs.serv_info.ci);

    rData[(*idx)++] = lbs.serv_info.rxlev;
	
    for (i = 0; i < 5; i++)
    {
        if (lbs.nbr_cell_info[i].mcc == 0x01CC)
        {
            rData[(*idx)++] = (u8)(lbs.nbr_cell_info[i].lac >> 8);
            rData[(*idx)++] = (u8)(lbs.nbr_cell_info[i].lac);

            rData[(*idx)++] = (u8)(lbs.nbr_cell_info[i].ci >> 8);
            rData[(*idx)++] = (u8)(lbs.nbr_cell_info[i].ci);

            rData[(*idx)++] = lbs.nbr_cell_info[i].rxlev;
        }
        else
        {
            rData[(*idx)++] = 0;
            rData[(*idx)++] = 0;
            rData[(*idx)++] = 0;
            rData[(*idx)++] = 0;
            rData[(*idx)++] = 0;
        }
    }
}

u16 protocol_2929_get_remaining_number(void)
{
   u16 i = 0;

   config_service_get(CFG_TIMING_NUMBER,TYPE_SHORT,&i,sizeof(u16));
   
   return i;
}

static void protocol_2929_pack_remaining_number(u8 *rData, u16 *idx, u16 len)
{
    u16 i = 0;
    
    if((*idx) + 0x06 > len)
    {
        LOG(WARN,"clock(%d) protocol_2929_pack_remaining_number assert(len(%d)) failed.", util_clock(), len);
        return;
    }
    
    // LEN
    rData[(*idx)++] = 0;
    rData[(*idx)++] = 0x04;
    // CMD
    i = protocol_2929_get_remaining_number();
    rData[(*idx)++] = (PROTOCOL_2929_ECMD_REMAINING_NUM>>8) & 0xff;
    rData[(*idx)++] = PROTOCOL_2929_ECMD_REMAINING_NUM & 0xff;
    rData[(*idx)++] = (i >> 8) & 0xff;
    rData[(*idx)++] = i & 0xff;
}

static void protocol_2929_pack_device_model(u8 *rData, u16 *idx, u16 len)
{
    if((*idx) + 0x08 > len)
    {
        LOG(WARN,"clock(%d) protocol_2929_pack_device_model assert(len(%d)) failed.", util_clock(), len);
        return;
    }
    
    rData[(*idx)++] = 0;
    rData[(*idx)++] = 0x06;
    rData[(*idx)++] = (PROTOCOL_2929_ECMD_DEVICE_MODEL >>8) & 0xff;
    rData[(*idx)++] = PROTOCOL_2929_ECMD_DEVICE_MODEL & 0xff;
    rData[(*idx)++] = 0x00;
    rData[(*idx)++] = 0x00;
    rData[(*idx)++] = 0x00;
    rData[(*idx)++] = 0x06;
}

static void protocol_2929_pack_iccid(u8 *rData, u16 *idx, u16 len)
{
    GM_ERRCODE ret = GM_SUCCESS;
    u8 iccid[GM_ICCID_LEN + 1] = {0};
    u16 content_len = 0;
    
    if((*idx) + 0x0E > len)
    {
        LOG(WARN,"clock(%d) protocol_2929_pack_device_model assert(len(%d)) failed.", util_clock(), len);
        return;
    }

    if(GM_SUCCESS != (ret = gsm_get_iccid(iccid)))
    {
        LOG(INFO,"clock(%d) protocol_2929_pack_iccid can not get iccid, ret:%d.", util_clock(), ret);
    }
    
    content_len = GM_strlen((const char *)iccid);
    if (0 == content_len)
    {
        /*如果读不到正确的ICCID退出!*/
        return;
    }

    
    rData[(*idx)++] = 0;
    rData[(*idx)++] = 0x0C;
    rData[(*idx)++] = (PROTOCOL_2929_ECMD_ICCID >>8) & 0xff;
    rData[(*idx)++] = PROTOCOL_2929_ECMD_ICCID & 0xff;

    rData[(*idx)++] = MERGEBCD(util_chr(iccid[0]), util_chr(iccid[1]));
    rData[(*idx)++] = MERGEBCD(util_chr(iccid[2]), util_chr(iccid[3]));
    rData[(*idx)++] = MERGEBCD(util_chr(iccid[4]), util_chr(iccid[5]));
    rData[(*idx)++] = MERGEBCD(util_chr(iccid[6]), util_chr(iccid[7]));
    rData[(*idx)++] = MERGEBCD(util_chr(iccid[8]), util_chr(iccid[9]));
    rData[(*idx)++] = MERGEBCD(util_chr(iccid[10]), util_chr(iccid[11]));
    rData[(*idx)++] = MERGEBCD(util_chr(iccid[12]), util_chr(iccid[13]));
    rData[(*idx)++] = MERGEBCD(util_chr(iccid[14]), util_chr(iccid[15]));
    rData[(*idx)++] = MERGEBCD(util_chr(iccid[16]), util_chr(iccid[17]));
    rData[(*idx)++] = MERGEBCD(util_chr(iccid[18]), util_chr(iccid[19]));



}

static void protocol_2929_pack_extended_alarm_status(u8 *rData, u16 *idx, u16 len)
{
    u32 exstate = 0xFFFFFFFF;
    
    if((*idx) + 0x08 > len)
    {
        LOG(WARN,"clock(%d) protocol_2929_pack_extended_alarm_status assert(len(%d)) failed.", util_clock(), len);
        return;
    }
    
    rData[(*idx)++] = 0;
    rData[(*idx)++] = 0x06;
    rData[(*idx)++] = (PROTOCOL_2929_ECMD_EALARM_STATUS >>8) & 0xff;
    rData[(*idx)++] = PROTOCOL_2929_ECMD_EALARM_STATUS & 0xff;

    if(get_expmu_wkp_reason() == EVENT_EXTI1_WKP)
    {
        exstate &= (~SYS_DEV_STATE_LIGHT_ALARM);
    }
    
    rData[(*idx)++] = (exstate >> 24) & 0xff;
    rData[(*idx)++] = (exstate >> 16) & 0xff;
    rData[(*idx)++] = (exstate >> 8) & 0xff;
    rData[(*idx)++] = exstate & 0xff;
}

void protocol_2929_get_next_time_update(u8 *rData, u16 *idx)
{
    u32 next_sec ,cur_sec; 
    u8 next_bcd_time[7] = {0};
    u8 zone ,day = 0;

    cur_sec = util_get_utc_time();

    next_sec = get_next_wake_up_time_point(&day,cur_sec);

    if(next_sec == 0)
    {
        next_sec = cur_sec + expmu_service_get_report_interval();
    }

    config_service_get(CFG_TIME_ZONE,TYPE_BYTE,&zone,sizeof(u8)); 

    util_utc_sec_to_bcdtime_base2000(next_sec,next_bcd_time,zone);

    LOG(INFO,"clock(%d) protocol next wkp time %x-%x-%x %x:%x:%x",util_clock(),next_bcd_time[0],next_bcd_time[1],next_bcd_time[2],\
        next_bcd_time[3],next_bcd_time[4],next_bcd_time[5]);
    
    GM_memcpy(&rData[(*idx)], next_bcd_time, 6);
    
    (*idx) += 6;
}

static void protocol_2929_pack_next_time_and_environment(u8 *rData, u16 *idx, u16 len)
{
    //u8 next_time[6] = {0};
    u8 gps_cnt = 0;
    if((*idx) + 0x0B > len)
    {
        LOG(WARN,"clock(%d) protocol_2929_pack_next_time_update assert(len(%d)) failed.", util_clock(), len);
        return;
    }
    rData[(*idx)++] = 0;
    rData[(*idx)++] = 0x09;
    rData[(*idx)++] = (PROTOCOL_2929_ECMD_NEXT_TIME_UPDATE >>8) & 0xff;
    rData[(*idx)++] = PROTOCOL_2929_ECMD_NEXT_TIME_UPDATE & 0xff;
    
    protocol_2929_get_next_time_update(rData,idx);

    gps_cnt = gps_get_satellites_good();

    if (gps_cnt < 3) 
    {
        rData[(*idx)++] = 2;
    }
    else if (gps_cnt >= 3 && gps_cnt <= 5) //半开放
    {
        rData[(*idx)++] = 1;
    }
    else if (gps_cnt > 5)  //全开放
    {
        rData[(*idx)++] = 0;
    }
}

static void protocol_2929_pack_platfrom_mode_flg(u8 *rData, u16 *idx, u16 len)
{
    u8 mode;

    get_cur_gw_work_mode(&mode);

    if(mode == PLATFORM_CYCLIC_MODE || mode == PLATFORM_TIMEPOINT_MODE)
    {
        rData[(*idx)++] = 0;
        rData[(*idx)++] = 0x03;
        rData[(*idx)++] = (PROTOCOL_2929_PLATFROM_MODE_FLG >>8) & 0xff;
        rData[(*idx)++] = PROTOCOL_2929_PLATFROM_MODE_FLG & 0xff;
        rData[(*idx)++] = 0;
    }
}

static void protocol_2929_pack_work_mode_update(u8 *rData, u16 *idx, u16 len)
{
    UploadWorkModeStruct *up = NULL;

    up = get_work_mode_upload_data();
    /*在收到回复时清除标志*/
    if(up->datalen >0)
    {
        up->upload_flg = 1;
        rData[(*idx)++] = 0;
        rData[(*idx)++] = up->datalen + 2;
        rData[(*idx)++] = (PROTOCOL_2929_WORK_MODE_UPDTAE >>8) & 0xff;
        rData[(*idx)++] = PROTOCOL_2929_WORK_MODE_UPDTAE & 0xff;

        GM_memcpy(&rData[(*idx)], up->data, up->datalen);

        (*idx) += up->datalen;
    }
}

u8 is_saved_protocol_data_valid(void)
{
    u16 i = 0;

    u8 xor = 0;

    LOG(INFO,"save hisdata len was %d",s_protocol.len);
    
    if(s_protocol.valid_flg == DATA_VALID_FLAG)
    {
       if(s_protocol.len > 44)
       {
         for(i = 0; i < (s_protocol.len - 1); i++)
         {
            xor ^= s_protocol.save_data[i];
         }

         if(xor == 0)
         {
            LOG(INFO,"clock(%d) saved hisdata xor valid!!",util_clock());
            
            return 1;
         }
       }

    }

    return 0;
}

void clear_saved_protocol_data(void)
{
    GM_memset((u8 *)&s_protocol,0,sizeof(SaveProtocolDataStruct));
}

/*从文件中读取保存的协议数据*/
u8 read_saved_protocol_data_from_file(void)
{
    s32 handle;

    u32 fs_len;

    int ret;
    
    handle = GM_FS_Open(PROTOCOL_DATA_SAVE, GM_FS_READ_ONLY | GM_FS_ATTR_ARCHIVE | GM_FS_CREATE);
    
    if (handle < 0)
    {
        LOG(INFO,"read_saved_protocol_data_from_file: open fail [%d]", handle);

        return 0;
    }
    
    
    ret = GM_FS_Read(handle, (void *)&s_protocol, sizeof(SaveProtocolDataStruct), &fs_len);
    if (ret < 0 )
    {
        LOG(INFO,"read_saved_protocol_data_from_file: read fail len:%d ret:%d.",fs_len, ret);
        
        GM_memset((u8 *)&s_protocol,0,sizeof(SaveProtocolDataStruct));

        GM_FS_Close(handle);

        return 0;
    }
    
    GM_FS_Close(handle);

    return 1;
}

/*写数据到文件中*/
u8 save_protocol_data_to_file(u8 clear)
{
    u32 fs_len;
    int handle, ret;
    
    if(clear)
    {
        GM_memset((u8 *)&s_protocol,0,sizeof(SaveProtocolDataStruct));
    }
    
    handle = GM_FS_Open(PROTOCOL_DATA_SAVE, GM_FS_READ_WRITE | GM_FS_ATTR_ARCHIVE | GM_FS_CREATE);
    if (handle < 0)
    {
        LOG(INFO,"save_protocol_data_to_file: open fail [%d]", handle);
        return 0;
    }    
    
    
    ret = GM_FS_Write(handle, (void *)&s_protocol, sizeof(SaveProtocolDataStruct), &fs_len);
    if (ret < 0)
    {
        LOG(INFO,"save_protocol_data_to_file: write fail [%d]", ret);
        
        GM_FS_Close(handle);
        return 0;
    }
    
    GM_FS_Close(handle);
    
    LOG(INFO,"save_protocol_data_to_file, len:%d", fs_len);

    return 1;

}

/*获取保存GPS数据的时间*/
u32 get_saved_protocol_data_gps_time(void)
{
    ProtocolStruct *p = NULL;

    time_t sec = 0;

    u8 zone = 0;

    u32 adj_sec  = 0;

    p = (ProtocolStruct *)s_protocol.save_data;

    LOG(INFO,"his data date %x-%02x-%02x %02x:%02x:%02x",p->time[0],p->time[1],p->time[2],\
    p->time[3],p->time[4],p->time[5]);


    sec = util_bcd_to_sec(p->time);

    if(sec > 0)
    {
        LOG(INFO,"his data gps tiem %d",sec);
        
        config_service_get(CFG_TIME_ZONE,TYPE_BYTE,&zone,sizeof(u8));

        adj_sec = ((u32)(zone & 0x7F)*3600);

        if(zone & 0x80)
        {
            sec += adj_sec;
        }
        else
        {
            sec -= adj_sec;
        }
    }
    else
    {
        sec = 0;
    }

    

    return (u32)sec;
}

static void protocol_2929_pack_wifi_data(u8 *rData, u16 *idx, u16 len)
{
    u8 wifi_sta = 0;
    u16 jval,ap_addr_len = 0;
    u8 j = 0 ,i = 0,ap_count = 0;
    wifi_scan_result_struct *ap = NULL;
    //wifi data len(2) + ap_count(1) + ap_info(5 *22) + cmd(2) = 115 

    if((*idx) + 115 > len)
    {
        LOG(WARN,"clock(%d) protocol_2929_pack_wifi_data assert(len(%d)) failed.", util_clock(), len);
        return;
    }

    ap = wifi_service_get_wifi_scan_result();

    config_service_get(CFG_WIFI_STATE,TYPE_BYTE,&wifi_sta,sizeof(u8));

    /*wifi字段只有在设备打开wifi开关时才打开*/
    if(wifi_sta)
    {
        /*40s 之前的数据丢掉*/
        if(util_clock() - ap->record_time < 30)
        {
            /*扫描成功*/
            if(ap->info.status == 0)
            {
                if(ap->info.scan_ap_num > 0)
                {
                    ap_count = ap->info.scan_ap_num;
                    
                    if(ap->info.scan_ap_num)ap_count = 5;

                    rData[(*idx)++]= 0;

                    jval = (*idx)++;

                    rData[jval]= 0;

                    rData[(*idx)++]= (PROTOCOL_2929_ECMD_WIFI_LOCATION >> 8) & 0xff;

                    rData[(*idx)++]= PROTOCOL_2929_ECMD_WIFI_LOCATION & 0xff;

                    rData[(*idx)++]= ap_count; 

                    for(i = 0; i < ap_count ; i++)
                    {
                        j = GM_sprintf((char *)&rData[(*idx)],"%02x:%02x:%02x:%02x:%02x:%02x",ap->info.scan_ap[i].bssid[0],ap->info.scan_ap[i].bssid[1],\
                            ap->info.scan_ap[i].bssid[2],ap->info.scan_ap[i].bssid[3],ap->info.scan_ap[i].bssid[4],ap->info.scan_ap[i].bssid[5]);

                        *idx = (*idx) + j;

                        ap_addr_len += j;

                        j = GM_sprintf((char *)&rData[*idx],",%02d",(int)(ap->info.scan_ap[i].rssi));

                        *idx = (*idx) + j;

                        ap_addr_len += j;

                        if(i < (ap_count - 1))
                        {
                            rData[(*idx)++] = ',';

                            ap_addr_len++;
                        }
                    }

                    rData[jval] = ap_addr_len + 3;

                    return;
                }
            }
        }
        #if 0
        rData[(*idx)++] = 0;

        rData[(*idx)++] = 3;

        rData[(*idx)++] = (PROTOCOL_2929_ECMD_WIFI_LOCATION >> 8) & 0xff;

        rData[(*idx)++] = PROTOCOL_2929_ECMD_WIFI_LOCATION & 0xff;

        rData[(*idx)++] = 0; 
        #endif
   }

    
}

u16 peek_one_location_protocol_data(u8 *pdata)
{
    if(s_protocol.valid_flg == DATA_VALID_FLAG)
    {
        GM_memcpy(pdata,s_protocol.save_data, s_protocol.len);

        return s_protocol.len;
    }

    return 0;
}

u16 protocol_2929_install_one_location_packet(const GPSData *gps)
{
    u8 *send;
    u16 len = 400;
    u16 idx = 0;
    u16 buf_len = 0;
    
    send = (u8 *) GM_MemoryAlloc(len);
	if (send == NULL)
	{
        LOG(INFO,"clock(%d) protocol_2929_send_one_gprs assert(GM_MemoryAlloc(%d)) failed.", util_clock(), len);
		return (u16)GM_UNKNOWN;
	}
   
    protocol_2929_pack_gps_data(send, &idx, len, gps);

	protocol_2929_pack_get_state(send, &idx, len);

	protocol_2929_pack_additional_lbs(send, &idx, len,gps);

	protocol_2929_pack_remaining_number(send, &idx, len);

	protocol_2929_pack_device_model(send, &idx, len);

	protocol_2929_pack_iccid(send, &idx, len);

	protocol_2929_pack_extended_alarm_status(send, &idx, len);

    protocol_2929_pack_wifi_data(send, &idx, len);

    protocol_2929_pack_work_mode_update(send, &idx, len);

	protocol_2929_pack_next_time_and_environment(send, &idx, len);
    
    protocol_2929_pack_platfrom_mode_flg(send, &idx, len);

	buf_len = protocol_2929_send_packet(0x80,send,&idx,len,s_protocol.save_data);

    s_protocol.valid_flg = DATA_VALID_FLAG;

    s_protocol.len = buf_len;

	GM_MemoryFree(send);
    
    return buf_len;

}

u16 protocol_2929_install_vehicle_terminal_confirmation_packet(u8 *pdata)
{
    u8 *send;
    u16 len = 400;
    u16 idx = 0;
    u16 buf_len = 0;
    GPSData gps;
    
    send = (u8 *) GM_MemoryAlloc(len);
	if (send == NULL)
	{
        LOG(INFO,"clock(%d) protocol_2929_install_Vehicle_terminal_confirmation_packet assert(GM_MemoryAlloc(%d)) failed.", util_clock(), len);
		return (u16)GM_UNKNOWN;
	}

    if(get_gps_valid_position(&gps) == false)
    {
        GM_memset(&gps,0,sizeof(gps));
    }
    
	protocol_2929_pack_gps_data(send, &idx, len, (const GPSData*)&gps);
	
	protocol_2929_pack_get_state(send, &idx, len);
	
	buf_len = protocol_2929_send_packet(0x85,send,&idx,len,pdata);
    
	GM_MemoryFree(send);
    return buf_len;

}


