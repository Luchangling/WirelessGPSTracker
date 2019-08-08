/**
 * Copyright @ Goome Technologies Co., Ltd. 2009-2019. All rights reserved.
 * File name:        protocol_2929.h
 * Author:           李飞祥      
 * Version:          1.0
 * Date:             2019-06-17
 * Description:      超长待机2929协议实现《goome2929超长待机通讯协议.pdf》
 * Others:           
 * Function List:    
    1. 
    
 * History: 
    1. Date:         2019-06-17
       Author:       李飞祥
       Modification: 创建初始版本
    2. Date: 		 
	   Author:		 
	   Modification: 

 */


#ifndef __PROTOCOL_2929_H__
#define __PROTOCOL_2929_H__
#include "gps.h"
typedef struct
{
    u8 hdh;
    u8 hdl;
    u8 cmd;
    u8 lenh;
    u8 lenl;
    u8 ip[4];
    u8 time[1];
}ProtocolStruct;

typedef struct
{
    u16 valid_flg;
    
    u16 len;

    u8 save_data[400];
}SaveProtocolDataStruct;


#define GM_DEV_NUMBER_LEN 11
#define GOOME_DEV_NUMBER_DEFAULT "13000000000"

u16 protocol_2929_install_one_location_packet(const GPSData *gps);//80位置数据组包
u16 protocol_2929_install_vehicle_terminal_confirmation_packet(u8 *pdata);//85应答包
void protocol_2929_parse_msg(u8 *pdata , u16 len);//接收数据解析
void protocol_2929_get_next_time_update(u8 *rData, u16 *idx);//获取下次唤醒时间
void protocol_2929_get_device_environment(u8 *rData, u16 *idx);//获取定位环境,是全天空还是半天空
u16 protocol_2929_get_remaining_number(void);//获取剩余条数
u8 read_saved_protocol_data_from_file(void);
u8 save_protocol_data_to_file(u8 clear);
u8 is_saved_protocol_data_valid(void);
u32 get_saved_protocol_data_gps_time(void);
void clear_saved_protocol_data(void);
u16 peek_one_location_protocol_data(u8 *pdata);

#endif

