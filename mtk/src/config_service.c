#include "gm_type.h"
#include "stdio.h"
#include "gm_stdlib.h"
#include "gm_gprs.h"
#include "fifo.h"
#include "utility.h"
#include "gsm.h"
#include "gm_memory.h"
#include "config_service.h"
#include "gps_service.h"
#include "log_service.h"
#include "agps_service.h"
#include "update_service.h"
#include "g_sensor.h"
#include "auto_test.h"
#include "gprs.h"
#include "expmu.h"
#include "command.h"
#include "stdlib.h"


#define CONFIG_PING_TIME  43200

typedef struct
{
    u32 last_ok_time;   //上一次执行配置检测的时间
    u32 wait;           //执行配置检测的间隔
}ConfigServiceExtend;

static SocketType s_config_socket = {-1,"",SOCKET_STATUS_ERROR,};
static ConfigServiceExtend s_config_socket_extend;

static ConfigParamItems s_configs[CFG_PARAM_MAX];


typedef struct
{
    char cmd_string[CONFIG_CMD_MAX_LEN+1];
    u16 index;
}ConfigParamStruct;

static const ConfigParamStruct s_config_param[] = 
{
    {"SERVERADDR",                      CFG_SERVERADDR},

    {"WORKMODE",                        CFG_WORK_MODE},

    {"TIMEZONE",                        CFG_TIME_ZONE},

    {"WIFISTATE",                       CFG_WIFI_STATE},

    {"CHANGPWD",                        CFG_CHANG_PWD},

    {"PWRREPCNT",                       CFG_PWR_REP_CNT},

    {"REPGAP",                          CFG_REP_GAP},

};


typedef struct
{
    char device_str[10];
    u16 idx;
}ConfigDeviceTypeStruct;

static GM_ERRCODE config_service_transfer_status(u8 new_status);
static void config_service_init_proc(void);
static void config_service_connecting_proc(void);
static void config_service_work_proc(void);
static void config_service_finish_proc(void);
static void config_service_send_request_failed(void);
static void config_srevice_create_other_services(void);
static void config_service_close(void);
static GM_ERRCODE config_msg_upload(u8 cmd, const u8 *msg, u16 len);
static void config_msg_request(void);
static void config_msg_response(void);
static void config_msg_receive(SocketType *socket);
static void config_msg_content_parse(u8 *pdata, u16 len);
static u16 config_msg_get_item(const ConfigParamDataType data_type, u8 *pOut, const u8 *pSrc, u8 *len);
static void config_msg_param_set(u16 index, u8 *pMsg, u8 len);
static void config_service_finish(u32 wait);
static void config_service_retry_config(void);

static GM_ERRCODE config_service_transfer_status(u8 new_status)
{
    u8 old_status = (u8)s_config_socket.status;
    GM_ERRCODE ret = GM_PARAM_ERROR;
    switch(s_config_socket.status)
    {
        case SOCKET_STATUS_INIT:
            switch(new_status)
            {
                case SOCKET_STATUS_INIT:
                    ret = GM_SUCCESS;
                    break;
                case SOCKET_STATUS_GET_HOST:
                    ret = GM_SUCCESS;
                    break;
                case SOCKET_STATUS_CONNECTING:
                    ret = GM_SUCCESS;
                    break;
                case SOCKET_STATUS_WORK:
                    break;
                case SOCKET_STATUS_DATA_FINISH:
                    ret = GM_SUCCESS;
                    break;
                case SOCKET_STATUS_ERROR:
                    ret = GM_SUCCESS;
                    break;
                default:
                    break;
            }
            break;
        case SOCKET_STATUS_GET_HOST:
            switch(new_status)
            {
                case SOCKET_STATUS_INIT:
                    ret = GM_SUCCESS;
                    break;
                case SOCKET_STATUS_GET_HOST:
                    break;
                case SOCKET_STATUS_CONNECTING:
                    ret = GM_SUCCESS;
                    break;
                case SOCKET_STATUS_WORK:
                    break;
                case SOCKET_STATUS_DATA_FINISH:
                    ret = GM_SUCCESS;
                    break;
                case SOCKET_STATUS_ERROR:
                    ret = GM_SUCCESS;
                    break;
                default:
                    break;
            }
            break;
        case SOCKET_STATUS_CONNECTING:
            switch(new_status)
            {
                case SOCKET_STATUS_INIT:
                    ret = GM_SUCCESS;
                    break;
                case SOCKET_STATUS_GET_HOST:
                    ret = GM_SUCCESS;
                    break;
                case SOCKET_STATUS_CONNECTING:
                    break;
                case SOCKET_STATUS_WORK:
                    ret = GM_SUCCESS;
                    break;
                case SOCKET_STATUS_DATA_FINISH:
                    ret = GM_SUCCESS;
                    break;
                case SOCKET_STATUS_ERROR:
                    ret = GM_SUCCESS;
                    break;
                default:
                    break;
            }
            break;
        case SOCKET_STATUS_WORK:
            switch(new_status)
            {
                case SOCKET_STATUS_INIT:
                    ret = GM_SUCCESS;
                    break;
                case SOCKET_STATUS_GET_HOST:
                    ret = GM_SUCCESS;
                    break;
                case SOCKET_STATUS_CONNECTING:
                    break;
                case SOCKET_STATUS_WORK:
                    break;
                case SOCKET_STATUS_DATA_FINISH:
                    ret = GM_SUCCESS;
                    break;
                case SOCKET_STATUS_ERROR:
                    ret = GM_SUCCESS;
                    break;
                default:
                    break;
            }
            break;
        case SOCKET_STATUS_DATA_FINISH:
            switch(new_status)
            {
                case SOCKET_STATUS_INIT:
                    ret = GM_SUCCESS;
                    break;
                case SOCKET_STATUS_GET_HOST:
                    ret = GM_SUCCESS;
                    break;
                case SOCKET_STATUS_CONNECTING:
                    break;
                case SOCKET_STATUS_WORK:
                    break;
                case SOCKET_STATUS_DATA_FINISH:
                    break;
                case SOCKET_STATUS_ERROR:
                    ret = GM_SUCCESS;
                    break;
                default:
                    break;
            }
            break;
        case SOCKET_STATUS_ERROR:
            switch(new_status)
            {
                case SOCKET_STATUS_INIT:
                    break;
                case SOCKET_STATUS_GET_HOST:
                    break;
                case SOCKET_STATUS_CONNECTING:
                    break;
                case SOCKET_STATUS_WORK:
                    break;
                case SOCKET_STATUS_DATA_FINISH:
                    break;
                case SOCKET_STATUS_ERROR:
                    ret = GM_SUCCESS;
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }


    if(GM_SUCCESS == ret)
    {
        s_config_socket.status = new_status;
        s_config_socket.status_fail_count = 0;
        LOG(INFO,"clock(%d) config_service_transfer_status from %s to %s success", util_clock(), 
            gm_socket_status_string((SocketStatus)old_status), gm_socket_status_string((SocketStatus)new_status));
    }
    else
    {
        LOG(INFO,"clock(%d) config_service_transfer_status from %s to %s failed", util_clock(), 
            gm_socket_status_string((SocketStatus)old_status), gm_socket_status_string((SocketStatus)new_status));
    }

    return ret;

}

ConfigProtocolEnum config_service_get_app_protocol(void)
{
    return PROTOCOL_2929;
}


GPSChipType config_service_get_gps_type(void)
{
    return GM_GPS_TYPE_MTK_EPO;
}


static void config_service_set_addr(u16 index, u8 *pMsg, u8 len)
{
    u8 addr[2*GOOME_DNS_MAX_LENTH];
    u8 IP[4];
    u32 port = 0;
    u8 idx = 0;

    GM_memset(addr, 0x00, sizeof(addr));
    idx = GM_sscanf((const char*)pMsg, "%[^:]:%d", addr, &port);
    if (idx != 2)
    {
        LOG(INFO,"clock(%d) config_service_set_addr assert(2 fields(%s)) failed.", util_clock(), pMsg);
        return;
    }
    
    if(GM_SUCCESS != GM_ConvertIpAddr(addr, IP))
    {
        if(util_is_valid_dns(addr, GM_strlen((const char *)addr)))
        {
        	LOG(INFO,"clock(%d) config_service_set_addr(%s) success.", util_clock(), pMsg);
            config_service_set((ConfigParamEnum)index, TYPE_STRING, pMsg, len);
        }
        else
        {
            LOG(WARN,"clock(%d) config_service_set_addr assert(dns(%s)) failed.", util_clock(), addr);
            return;
        }
    }
    else
    {
        LOG(INFO,"clock(%d) config_service_set_addr(%s) success.", util_clock(), pMsg);
        config_service_set((ConfigParamEnum)index, TYPE_STRING, pMsg, len);
    }
}


static void config_service_set_byte(u16 index, u8 *pMsg, u8 len)
{
	if (NULL == pMsg || len < sizeof(U8))
	{
		LOG(INFO,"clock(%d) config_service_set_range_word index(%d) len %d failed.", util_clock(),index,len);
		return;
	}
    config_service_set((ConfigParamEnum)index, TYPE_BYTE, pMsg, sizeof(U8));
    LOG(INFO,"clock(%d) config_service_set_byte index(%d) value %d.", util_clock(),index,pMsg[0]);
}

static void config_service_set_zone(u16 index, u8 *pMsg, u8 len)
{   
    s32 data = 0;

    u8 zone = 0;
    
	if (NULL == pMsg || len < sizeof(U8))
	{
		LOG(INFO,"clock(%d) config_service_set_range_word index(%d) len %d failed.", util_clock(),index,len);
		return;
	}

    data = MKDWORD(pMsg[0],pMsg[1],pMsg[2], pMsg[3]);

    zone  = abs(data);
    
    if(data < 0)
    {
        zone |= 0x80;
    }
    
    config_service_set((ConfigParamEnum)index, TYPE_BYTE, &zone, sizeof(U8));
    
    LOG(INFO,"clock(%d) config_service_set_byte index(%d) value %d.", util_clock(),index,pMsg[0]);
}


static void config_service_set_short(u16 index, u8 *pMsg, u8 len)
{
    u16 interval = 0;
    
	if (NULL == pMsg || len < sizeof(U16))
	{
		LOG(INFO,"clock(%d) config_service_set_range_word index(%d) len %d failed.", util_clock(),index,len);
		return;
	}

    interval = MKWORD(pMsg[2], pMsg[3]);
    
    config_service_set((ConfigParamEnum)index, TYPE_SHORT, &interval, sizeof(U8));
    LOG(INFO,"clock(%d) config_service_set_byte index(%d) value %d.", util_clock(),index,pMsg[0]);
}


static void config_service_set_pwd(u16 index, u8 *pMsg, u8 len)
{
    if ((NULL == pMsg || len == 0))
	{
		LOG(INFO,"clock(%d) config_service_set_range_word index(%d) len %d failed.", util_clock(),index,len);
		return;
	}

    config_service_set((ConfigParamEnum)index, TYPE_STRING, pMsg, len);
    LOG(INFO,"clock(%d) config_service_set_pwd index(%d) string %s.", util_clock(),index,pMsg);

}

/*
// have ip, use ip to connect
// no ip, call gm_socket_get_host_by_name 
*/
static void config_service_init_proc(void)
{
    u8 IP[4];
    gm_socket_get_host_by_name_trigger(&s_config_socket);
    system_state_get_ip_cache(SOCKET_INDEX_CONFIG, IP);
    if(GM_SUCCESS == gm_is_valid_ip(IP))
    {
        GM_memcpy( s_config_socket.ip , IP, sizeof(IP));
        config_service_transfer_status(SOCKET_STATUS_CONNECTING);
        if(GM_SUCCESS == gm_socket_connect(&s_config_socket))
        {
        }
        // else do nothing .   connecting_proc will deal.
    }
    else if((!s_config_socket.excuted_get_host) && (GM_SUCCESS == gm_is_valid_ip(s_config_socket.ip)))
    {
        config_service_transfer_status(SOCKET_STATUS_CONNECTING);
        if(GM_SUCCESS == gm_socket_connect(&s_config_socket))
        {
        }
        // else do nothing .   connecting_proc will deal.
    }
}


static void config_service_connecting_proc(void)
{
    u32 current_time = util_clock();

    if((current_time - s_config_socket.send_time) > CONNECT_TIME_OUT)
    {
        s_config_socket.status_fail_count ++;
        config_service_connection_failed();

        if(s_config_socket.status == SOCKET_STATUS_CONNECTING &&
            s_config_socket.status_fail_count < MAX_CONNECT_REPEAT)
        {
            if(GM_SUCCESS == gm_socket_connect(&s_config_socket))
            {
                //do nothing. wait callback
            }
        }
    }
}


static GM_ERRCODE config_msg_upload(u8 cmd, const u8 *msg, u16 len)
{
    u16 k, msglen;
    u8 csm=0;
    u8 buff[50];
    u8 imei[GM_IMEI_LEN + 1];
    GM_ERRCODE ret;

    GM_memset(buff, 0x00, sizeof(buff));
    
    buff[0] = 0x66; 
    buff[1] = 0x66;
    buff[2] = cmd;
    buff[3] = 0x00;
    buff[4] = 0x08;
    
    msglen = 5;

    if(GM_SUCCESS != (ret = gsm_get_imei(imei)))
    {
        LOG(INFO,"clock(%d) config_msg_upload can not get imei, ret:%d.", util_clock(), ret);
        return ret;
    }
    
    buff[msglen++] = util_chr(imei[0]);
    buff[msglen++] = MERGEBCD(util_chr(imei[1]), util_chr(imei[2]));
    buff[msglen++] = MERGEBCD(util_chr(imei[3]), util_chr(imei[4]));
    buff[msglen++] = MERGEBCD(util_chr(imei[5]), util_chr(imei[6]));
    buff[msglen++] = MERGEBCD(util_chr(imei[7]), util_chr(imei[8]));
    buff[msglen++] = MERGEBCD(util_chr(imei[9]), util_chr(imei[10]));
    buff[msglen++] = MERGEBCD(util_chr(imei[11]), util_chr(imei[12]));
    buff[msglen++] = MERGEBCD(util_chr(imei[13]), util_chr(imei[14]));
    
    if ((len > 0) && (NULL != msg))
    {
        GM_memcpy(&buff[msglen], msg, len);
        msglen += len;
    }
    
    buff[3] = UPPER_BYTE(len + 8);
    buff[4] = LOWER_BYTE(len + 8);
    
    for (k=0; k<msglen; k++)
    {
        csm ^= buff[k];
    }
    
    buff[msglen++] = csm;
    buff[msglen++] = 0x0D;

    LOG(DEBUG,"clock(%d) config_msg_upload msglen(%d)", util_clock(), msglen);
    ret = gm_socket_send(&s_config_socket,buff, msglen);
    return ret;
}



static void config_msg_request(void)
{
    U8 msg[1];
    u32 current_time = util_clock();
	
    msg[0] = CFG_CMD_REQ_ALL;  // req all 

    s_config_socket.send_time = current_time; 
    if(GM_SUCCESS != config_msg_upload(CFG_CMD_REQ, msg,1))
    {
        config_service_finish(CONFIG_PING_TIME);
    }
}

static void config_msg_response(void)
{
    U8 msg[1];
	
    msg[0] = CFG_CMD_REQ_ALL;  // req all 

    config_msg_upload(CFG_CMD_RESULT, msg,1); //ignore send fail
}


static void config_msg_receive(SocketType *socket)
{
    // parse buf msg
    // if OK, after creating other socket, transfer to finish
    // not ok, ignore msg.
    u8 checksum = 0;
    u8 cmd = 0;
    u8 idx = 0;
    u8 jval = 0;
    u8 resp_imei[GM_IMEI_LEN + 1];
    u16 index = 0;

    #define CONFIG_MSG_HEAD_LEN 5
    #define CONFIG_MSG_TAIL_LEN 2
    u8 head[CONFIG_MSG_HEAD_LEN];
    u8 *pdata;
    u32 msg_len = sizeof(head);
    //u8 *buf;

    GM_ERRCODE ret;
    u8 imei[GM_IMEI_LEN + 1];
    static u32 packet_error_start = 0;

    GM_memset(resp_imei, 0, sizeof(resp_imei));
    
    if(SOCKET_STATUS_WORK != socket->status)
    {
        return;
    }
    
    //get head then body
    if(GM_SUCCESS != fifo_peek(&socket->fifo, head, msg_len))
    {
        return;
    }
    
    msg_len = MKWORD(head[3], head[4]);
    msg_len = msg_len + CONFIG_MSG_HEAD_LEN + CONFIG_MSG_TAIL_LEN;

    if(msg_len > MAX_GPRS_MESSAGE_LEN)
    {
        LOG(WARN,"clock(%d) config_msg_receive assert(msg_len(%d)) failed.",util_clock(), msg_len);
        //clear fifo and restart socket.
        fifo_reset(&socket->fifo);
        gm_socket_close_for_reconnect(socket);
        return;
    }

    pdata = GM_MemoryAlloc(msg_len);
    if(pdata == NULL)
    {
        LOG(WARN,"clock(%d) config_msg_receive assert(GM_MemoryAlloc(%d)) failed.",util_clock(), msg_len);

        //clear fifo and restart socket.
        fifo_reset(&socket->fifo);
        gm_socket_close_for_reconnect(socket);
        return;
    }
    
    if(GM_SUCCESS != fifo_peek(&socket->fifo, pdata, msg_len))
    {
        GM_MemoryFree(pdata);
        
        if(packet_error_start == 0)
        {
            LOG(DEBUG,"clock(%d) config_msg_receive get msg failed. len:%d", util_clock(), msg_len);
            log_service_print_hex((const char*)head, sizeof(head));
            packet_error_start = util_clock();
        }
        else
        {
            if((util_clock() - packet_error_start) > MAX_GPRS_PART_MESSAGE_TIMEOUT)
            {
                LOG(WARN,"clock(%d) config_msg_receive MAX_GPRS_PART_MESSAGE_TIMEOUT.",util_clock());
                //clear fifo and restart socket.
                fifo_reset(&socket->fifo);
                gm_socket_close_for_reconnect(socket);
                packet_error_start = 0;
            }
        }
        return;
    }

    fifo_pop_len(&socket->fifo, msg_len);

	LOG(DEBUG,"clock(%d) config_msg_receive msg fifo(%p) len(%d)", 
		util_clock(), &socket->fifo, msg_len);

    if(GM_SUCCESS != (ret = gsm_get_imei(imei)))
    {
        LOG(INFO,"clock(%d) config_msg_receive can not get imei, ret:%d.", util_clock(), ret);
        GM_MemoryFree(pdata);
        return;
    }

    for (index=0; index<msg_len-2; index++)
    {
        checksum = checksum^pdata[index];
    }
    
    if (checksum != pdata[msg_len-2])
    {
        LOG(INFO,"clock(%d) config_msg_receive checksum err. %d != %d", util_clock(), pdata[msg_len-2], checksum);
        GM_MemoryFree(pdata);
        return;
    }

    GM_memset(resp_imei, 0x00, sizeof(resp_imei));
    
    cmd = pdata[2];
    switch(cmd)
    {
        case CFG_CMD_ACK:
            index = 3;
            index += 2;

            resp_imei[0] = util_asc(pdata[index], 'x');
            for (idx=1,jval=1; jval<8; idx+=2,jval++)
            {
                resp_imei[idx] = util_asc((pdata[index+jval] >> 4) & 0x0F, 'x');
                resp_imei[idx+1] = util_asc(pdata[index+jval] & 0x0F, 'x');
            }

            if (0 != GM_memcmp(resp_imei, imei, GM_strlen((const char *)imei)))
            {
                LOG(INFO,"clock(%d) config_msg_receive imei err. %s != %s", util_clock(), resp_imei, imei);
                break;
            }
            
            index += 8;
            if (0x00 != pdata[index])
            {
	            config_msg_content_parse(&pdata[index+1], msg_len - index-2);
	            config_service_save_to_local();
            }

            config_msg_response();
            config_service_finish(CONFIG_PING_TIME);
            break;
                        
        default:
            break;
    }

    GM_MemoryFree(pdata);
}


/*
    get msg from fifo
*/
static void config_service_work_proc(void)
{
    u32 current_time = util_clock();

    if((current_time - s_config_socket.send_time) > MESSAGE_TIME_OUT)
    {
        s_config_socket.status_fail_count ++;
        config_service_send_request_failed();

        if(s_config_socket.status == SOCKET_STATUS_WORK &&
            s_config_socket.status_fail_count < MAX_CONNECT_REPEAT)
        {
            config_msg_request();
        }
    }

    config_msg_receive(&s_config_socket);
}


static void config_service_retry_config(void)
{
    u32 current_time = util_clock();

    if((current_time - s_config_socket_extend.last_ok_time) > s_config_socket_extend.wait)
    {
        LOG(DEBUG,"clock(%d) config_service_finish_proc cur(%d) - lastok(%d) > wait(%d).",
            util_clock(), current_time, s_config_socket_extend.last_ok_time,s_config_socket_extend.wait);
        s_config_socket_extend.last_ok_time = 0;
        s_config_socket_extend.wait = CONFIG_PING_TIME;
        config_service_transfer_status(SOCKET_STATUS_INIT);
    }

}



static void config_service_finish_proc(void)
{
    if(!gprs_is_ok())
    {
        return;
    }

    config_srevice_create_other_services();

    config_service_retry_config();
}



void config_service_connection_ok(void)
{
    config_service_transfer_status(SOCKET_STATUS_WORK);
    config_msg_request();
}

void config_service_close_for_reconnect(void)
{
    config_service_destroy();
}


void config_service_connection_failed(void)
{
    config_service_close();
    
    if(s_config_socket.status_fail_count >= MAX_CONNECT_REPEAT)
    {
        // if excuted get_host transfer to error statu, else get_host.
        if(s_config_socket.excuted_get_host || (s_config_socket.addr[0] == 0))
        {
            config_service_finish(CONFIG_PING_TIME);
        }
        else
        {
            config_service_transfer_status(SOCKET_STATUS_INIT);
        }
    }
    // else do nothing . wait connecting proc to deal.
}


static void config_service_send_request_failed(void)
{
    if(s_config_socket.status_fail_count >= MAX_CONNECT_REPEAT)
    {
        LOG(INFO,"clock(%d) config_service_send_request_failed s_config_socket.status_fail_count(%d) > %d.", 
			util_clock(), s_config_socket.status_fail_count,MAX_CONNECT_REPEAT);
        config_service_finish(CONFIG_PING_TIME);
		GM_memset(s_config_socket.ip,0,sizeof(s_config_socket.ip));
    }
    // else do nothing .
}

GM_ERRCODE config_service_create(void)
{
    u8 addr[2*GOOME_DNS_MAX_LENTH+1];
    u8 IP[4];
    u32 port = 0;
    u8 idx = 0;

    gm_socket_init(&s_config_socket, SOCKET_INDEX_CONFIG);

    GM_memset(addr, 0x00, sizeof(addr));
    idx = GM_sscanf((const char*)config_service_get_pointer(CFG_CFGSERVERADDR), "%[^:]:%d", addr, &port);
    if (idx != 2)
    {
        LOG(WARN,"clock(%d) config_service_create assert(idx ==2) failed.", util_clock());
        return GM_PARAM_ERROR;
    }
    
    if(GM_SUCCESS != GM_ConvertIpAddr(addr, IP))
    {
        if(util_is_valid_dns(addr, GM_strlen((const char *)addr)))
        {
            gm_socket_set_addr(&s_config_socket, addr, GM_strlen((const char*)addr), port, STREAM_TYPE_DGRAM);		
			system_state_get_ip_cache(SOCKET_INDEX_CONFIG, IP);
			gm_socket_set_ip_port(&s_config_socket, IP, port, STREAM_TYPE_DGRAM);
        }
        else
        {
            LOG(WARN,"clock(%d) config_service_create assert(dns(%s)) failed.", util_clock(), addr);
            return GM_PARAM_ERROR;
        }
    }
    else
    {
        gm_socket_set_ip_port(&s_config_socket, IP, port, STREAM_TYPE_DGRAM);
        system_state_set_ip_cache(SOCKET_INDEX_CONFIG, IP);
    }

    s_config_socket_extend.last_ok_time = 0;
    s_config_socket_extend.wait = CONFIG_PING_TIME;
    
    LOG(WARN,"clock(%d) config_service_create access_id(%d) fifo(%p).", util_clock(), s_config_socket.access_id, &s_config_socket.fifo);
	return GM_SUCCESS;
}


static void config_service_finish(u32 wait)
{
    config_service_close();

    s_config_socket_extend.last_ok_time=util_clock();
    s_config_socket_extend.wait=wait;
    
    config_service_transfer_status(SOCKET_STATUS_DATA_FINISH);
}

GM_ERRCODE config_service_destroy(void)
{
    if(SOCKET_STATUS_ERROR == s_config_socket.status)
    {
        return GM_SUCCESS;
    }
    
    config_service_close();

    if(s_config_socket.status < SOCKET_STATUS_DATA_FINISH)
    {
        // 配置正在获取过程中断了网, 重建连接
        config_service_transfer_status(SOCKET_STATUS_INIT);
        return GM_SUCCESS;
    }

    //config_service是在gprs_create中创建, 后面 重建连接, 所以保留fifo
    
    s_config_socket_extend.last_ok_time=util_clock();
    s_config_socket_extend.wait=CONFIG_PING_TIME;
    
    config_service_transfer_status(SOCKET_STATUS_ERROR);
	return GM_SUCCESS;
}

static void config_service_close(void)
{
    if(s_config_socket.id >=0)
    {
        GM_SocketClose(s_config_socket.id);
        s_config_socket.id=-1;
    }
}



GM_ERRCODE config_service_timer_proc(void)
{
    if(!s_config_socket.fifo.base_addr)
    {
        return GM_SUCCESS;
    }
    switch(s_config_socket.status)
    {
    case SOCKET_STATUS_INIT:
        if(is_expmu_power_on())
        {
            config_service_init_proc();
        }
        else
        {
            /*非EXPMU重新上电的情况下,不去查询*/
            config_service_transfer_status(SOCKET_STATUS_DATA_FINISH);
        }
        break;
    case SOCKET_STATUS_CONNECTING:
        config_service_connecting_proc();
        break;
    case SOCKET_STATUS_WORK:
        config_service_work_proc();
        break;
    case SOCKET_STATUS_DATA_FINISH:
        config_service_finish_proc();
        break;
    case SOCKET_STATUS_ERROR:
        config_service_finish_proc();
        break;
    default:
        LOG(WARN,"clock(%d) config_service_timer_proc assert(s_config_socket.status(%d)) unknown.", util_clock(), s_config_socket.status);
        return GM_ERROR_STATUS;
    }

    return GM_SUCCESS;
}

void config_service_set(ConfigParamEnum id, ConfigParamDataType type, const void* buf, u16 len)
{
	
    s_configs[id].type = type;
    
    switch(s_configs[id].type)
    {
        case TYPE_STRING:
            s_configs[id].len = len;
			GM_MemoryFree(s_configs[id].data.str);
            s_configs[id].data.str = GM_MemoryAlloc(len + 1);
            GM_memcpy(s_configs[id].data.str, buf, len);
            s_configs[id].data.str[len] = 0;
			break;
        case TYPE_INT:
            s_configs[id].len = 4;
            GM_memcpy(&s_configs[id].data.i, buf, s_configs[id].len);
			break;
        case TYPE_SHORT:
            s_configs[id].len = 2;
            GM_memcpy(&s_configs[id].data.s, buf, s_configs[id].len);
			break;
        case TYPE_BYTE:
            s_configs[id].len = 1;
            s_configs[id].data.b = *((u8 *)buf);
            break;
        case TYPE_BOOL:
            s_configs[id].len = 1;
            s_configs[id].data.b = *((u8 *)buf);
			break;
        case TYPE_FLOAT:
            s_configs[id].len = 4;
            GM_memcpy(&s_configs[id].data.s, buf, s_configs[id].len);
			break;
        case TYPE_ARRY:
            s_configs[id].len = len;
			GM_MemoryFree(s_configs[id].data.str);
            s_configs[id].data.str = GM_MemoryAlloc(len);
            GM_memcpy(s_configs[id].data.str, buf, len);
            break;
        default:
            LOG(WARN,"clock(%d) config_service_set assert(s_configs[%d].type(%d)) unknown.", util_clock(),id,s_configs[id].type);
            return;
    }

    if(len != s_configs[id].len)
    {
        LOG(WARN,"clock(%d) config_service_set assert(len == s_configs[id].len) failed,s_configs[%d].len=%d", util_clock(),id,s_configs[id].len);
    }
}




/*获取指定id对应的配置        */
GM_ERRCODE config_service_get(ConfigParamEnum id, ConfigParamDataType type, void* buf, u16 len)
{	
	if(type != s_configs[id].type)
	{
		LOG(WARN,"clock(%d) config_service_get assert(s_configs[%d].type(%d)== %d) failed.", util_clock(),id,s_configs[id].type, type);
		return GM_PARAM_ERROR;
	}
	
    switch(s_configs[id].type)
    {
        case TYPE_STRING:
            if(len < s_configs[id].len)
            {
                LOG(WARN,"clock(%d) config_service_get assert(s_configs[%d].type(%d)) len:%d < %d.", util_clock(),id,s_configs[id].type,len,s_configs[id].len);
                return GM_PARAM_ERROR;
            }
            if(s_configs[id].data.str)
            {
                GM_memcpy(buf, s_configs[id].data.str, s_configs[id].len);
            }
            else
            {
                GM_memcpy(buf, "", 1);
            }
            if(len > s_configs[id].len)
            {
                ((u8 *)buf)[s_configs[id].len] = 0;
            }
            else
            {
                ((u8 *)buf)[len -1] = 0;
            }
            break;
        case TYPE_INT:
            if(s_configs[id].len != len)
            {
                LOG(WARN,"clock(%d) config_service_get assert(s_configs[%d].type(%d)) len:%d != %d.", util_clock(),id,s_configs[id].type,len,s_configs[id].len);
                return GM_PARAM_ERROR;
            }
            GM_memcpy(buf, &s_configs[id].data.i, s_configs[id].len);
            break;
        case TYPE_SHORT:
            if(s_configs[id].len != len)
            {
                LOG(WARN,"clock(%d) config_service_get assert(s_configs[%d].type(%d)) len:%d != %d.", util_clock(),id,s_configs[id].type,len,s_configs[id].len);
                return GM_PARAM_ERROR;
            }
            GM_memcpy(buf, &s_configs[id].data.s, s_configs[id].len);
            break;
        case TYPE_BYTE:
            if(s_configs[id].len != len)
            {
                LOG(WARN,"clock(%d) config_service_get assert(s_configs[%d].type(%d)) len:%d != %d.", util_clock(),id,s_configs[id].type,len,s_configs[id].len);
                return GM_PARAM_ERROR;
            }
            *((u8 *)buf) = s_configs[id].data.b;
            break;
        case TYPE_BOOL:
            if(s_configs[id].len != len)
            {
                LOG(WARN,"clock(%d) config_service_get assert(s_configs[%d].type(%d)) len:%d != %d.", util_clock(),id,s_configs[id].type,len,s_configs[id].len);
                return GM_PARAM_ERROR;
            }
            *((u8 *)buf) = s_configs[id].data.b;
            break;
        case TYPE_FLOAT:
            if(s_configs[id].len != len)
            {
                LOG(WARN,"clock(%d) config_service_get assert(s_configs[%d].type(%d)) len:%d != %d.", util_clock(),id,s_configs[id].type,len,s_configs[id].len);
                return GM_PARAM_ERROR;
            }
            GM_memcpy(buf, &s_configs[id].data.f, s_configs[id].len);
            break;
        case TYPE_ARRY:
            if(s_configs[id].len != len)
            {
                LOG(WARN,"clock(%d) config_service_get assert(s_configs[%d].type(%d)) len:%d != %d.", util_clock(),id,s_configs[id].type,len,s_configs[id].len);
                return GM_PARAM_ERROR;
            }
            GM_memcpy(buf, s_configs[id].data.str, s_configs[id].len);
            break;
        default:
            LOG(WARN,"clock(%d) config_service_get assert(s_configs[%d].type(%d)) unknown.", util_clock(),id,s_configs[id].type);
            return GM_PARAM_ERROR;
    }
	return GM_SUCCESS;
}


u16 config_service_get_length(ConfigParamEnum id, ConfigParamDataType type)
{
    if(id >= CFG_PARAM_MAX )
    {
        LOG(WARN,"clock(%d) config_service_get assert(id(%d)) failed.", util_clock(),id);
        return 0;
    }

	return s_configs[id].len;
}


void* config_service_get_pointer(ConfigParamEnum id)
{
    static u8 empty_string[1] = "";
    
    switch(s_configs[id].type)
    {
        case TYPE_STRING:
            if(s_configs[id].data.str)
            {
                return s_configs[id].data.str;
            }
            else
            {
                return empty_string;
            }
        case TYPE_INT:
            return &s_configs[id].data.i;
        case TYPE_SHORT:
            return &s_configs[id].data.s;
        case TYPE_BYTE:
            return &s_configs[id].data.b;
        case TYPE_BOOL:
            return &s_configs[id].data.b;
        case TYPE_FLOAT:
            return &s_configs[id].data.f;
        case TYPE_ARRY:
            if(s_configs[id].data.str)
            {
                return s_configs[id].data.str;
            }
            else
            {
                return empty_string;
            }
        default:
            LOG(WARN,"clock(%d) config_service_get_pointer assert(s_configs[%d].type(%d)) unknown.", util_clock(),id,s_configs[id].type);
            return NULL;
    }
}


void config_msg_content_parse(u8 *pdata, u16 len)
{
    ConfigParamDataType data_type = TYPE_NONE;
    u8 *psrc = pdata;
    u8 index = 0;
    u16 parse_len = 0;
    u8 cmd_string[CONFIG_CMD_MAX_LEN+1] = {0};
    u8 msg_buff[CONFIG_STRING_MAX_LEN+1] = {0};
    u8 msg_len = 0;
	bool has_config = false;

	JsonObject* p_json_log = json_create();
	json_add_string(p_json_log, "event", "config");
	
    while(parse_len <= (len-12))
    {
        data_type = (ConfigParamDataType)(psrc[parse_len]);
        parse_len++;
		
		
        GM_memcpy(cmd_string, &psrc[parse_len], CONFIG_CMD_MAX_LEN);
        cmd_string[CONFIG_CMD_MAX_LEN] = 0;
        parse_len += CONFIG_CMD_MAX_LEN;

		GM_memset(msg_buff,0,sizeof(msg_buff));
		parse_len += config_msg_get_item(data_type, msg_buff, &psrc[parse_len], &msg_len);

		switch (data_type)
		{
			case 1:
				json_add_int(p_json_log, (const char*)cmd_string, MKDWORD(msg_buff[0], msg_buff[1], msg_buff[2], msg_buff[3]));
				break;
			case 2:
				json_add_int(p_json_log, (const char*)cmd_string, MKDWORD(msg_buff[0], msg_buff[1], msg_buff[2], msg_buff[3])/1000.0);
				break;
			case 3:
				json_add_string(p_json_log, (const char*)cmd_string, (const char*)msg_buff);
				break;
			case 4:
				json_add_int(p_json_log, (const char*)cmd_string, msg_buff[0]);
				break;
			default:
				json_add_string(p_json_log, (const char*)cmd_string, "unknown config name");
				break;	
		}

		
        util_remove_char(cmd_string, GM_strlen((const char*)cmd_string),' ');
        util_string_upper(cmd_string, GM_strlen((const char*)cmd_string));

		
		//特殊配置，进行校准修复
		if (0 == GM_strcmp((const char*)cmd_string, "REPAIRRAMP") && msg_buff[0])
		{
			auto_test_repair_ramp();
			has_config = true;
		}
		
        for (index=0; index< (sizeof(s_config_param)/sizeof(ConfigParamStruct)); index++)
        {
            if (0 == GM_strcmp((const char*)cmd_string, (const char*)s_config_param[index].cmd_string))
            {
                config_msg_param_set(s_config_param[index].index, msg_buff, msg_len);
				has_config = true;
                break;
            }
        }
    };

	if (has_config)
	{
		config_service_save_to_local();
    	gps_service_change_config();
		log_service_upload(INFO, p_json_log);
	}
	else
	{
		json_destroy(p_json_log);
		p_json_log = NULL;
	}
  
    return;
}

static void config_service_work_mode_set(u8 *input)
{
    u8 *resp= NULL;
    char old_Set_cmd[100] = {0};

    u16 len  = 0;

    resp = (u8 *)GM_MemoryAlloc(400);

    if(resp != NULL)
    {
        len = GM_sprintf(old_Set_cmd,"<SPGM*P:GMGPS*%s>",input);

        command_on_receive_data(COMMAND_GPRS,old_Set_cmd,len,(char *)resp,NULL);
    }

    GM_MemoryFree(resp);
}


static void config_msg_param_set(u16 index, u8 *pMsg, u8 len)
{
    //u8 value_u8;

    u8 temp[100] = {0};

    GM_memcpy(temp, pMsg, len);

    switch(index)
    {
        case CFG_SERVERADDR:
           config_service_set_addr(index,temp,len);
           LOG(INFO,"Config from server ,SERVER ADDR %s",temp);
        break;
        case CFG_WORK_MODE:
            config_service_work_mode_set(temp);
            LOG(INFO,"Config from server ,WORK MODE %s",temp);
        break;
        case CFG_TIME_ZONE:
            config_service_set_zone(index,temp,len);
            LOG(INFO,"Config from server ,TIME ZONE %d",MKDWORD(temp[0], temp[1], temp[2], temp[3]));
        break;
        case CFG_WIFI_STATE:
            config_service_set_byte(index,temp,len);
            LOG(INFO,"Config from server ,WIFI STATE %d",MKDWORD(temp[0], temp[1], temp[2], temp[3]));
        break;
        case CFG_CHANG_PWD:
            config_service_set_pwd(index,temp,len);
            LOG(INFO,"Config from server ,SMS PWD %s",temp);
        break;
        case CFG_PWR_REP_CNT:
            config_service_set_short(index,temp,len);
            LOG(INFO,"Config from server ,PWR REP %d",MKDWORD(temp[0], temp[1], temp[2], temp[3]));
        break;
        case CFG_REP_GAP:
            config_service_set_short(index,temp,len);
            LOG(INFO,"Config from server ,REP GAP %d",MKDWORD(temp[0], temp[1], temp[2], temp[3]));
        break;
            
    }
}



static u16 config_msg_get_item(const ConfigParamDataType data_type, u8 *pOut, const u8 *pSrc, u8 *len)
{
    u16 msg_len = 4;
    u8 seek = 0;

    *len = msg_len;
    if ((NULL == pOut) || (NULL == pSrc))
    {
        return 0;
    }

    if (TYPE_STRING == data_type)
    {
        //第一字节是长度
        msg_len = *pSrc;
        *len = (msg_len > CONFIG_STRING_MAX_LEN) ? CONFIG_STRING_MAX_LEN : msg_len;
        seek = 1;
        pSrc++;
        if(*len)
        {
            GM_memcpy(pOut, pSrc, *len);
            pOut[*len] = 0;
        }
        else
        {
            pOut[0] = 0;
        }
    }
    else if (TYPE_BOOL == data_type)
    {
        *len = msg_len = 1;
        GM_memcpy(pOut, pSrc, *len);
    }
    else
    {
        //*len = msg_len = 4;
        GM_memcpy(pOut, pSrc, *len);
    }
    return msg_len + seek;
}

static void config_srevice_create_other_services(void)
{
    static bool first_create = false;  // already created at gprs_init
    gps_service_create(first_create);

    if(first_create)
    {
        // after first create, all later create is not first create.
        first_create = false;
    }
}

S8 config_service_get_zone(void)
{
    GM_ERRCODE ret;
    s8 time_zone = 0;
    
	//获取本地时间
	ret = config_service_get(CFG_TIME_ZONE, TYPE_BYTE, &time_zone, sizeof(time_zone));
	if (GM_SUCCESS != ret)
	{
		LOG(INFO,"clock(%d) config_service_get_zone failed,ret=%d", util_clock(),ret);
	}
    return time_zone;
}

void config_service_set_reming_gps_count(void)
{
    u16 count;

    config_service_get(CFG_TIMING_NUMBER,TYPE_SHORT,&count,sizeof(u16));

    if(count > 0)count--;

    config_service_set(CFG_TIMING_NUMBER,TYPE_SHORT,&count,sizeof(u16));
}


StreamType config_service_update_socket_type(void)
{
    return STREAM_TYPE_DGRAM;
    //return STREAM_TYPE_STREAM;
}

StreamType config_service_agps_socket_type(void)
{
    //return STREAM_TYPE_DGRAM;
    return STREAM_TYPE_STREAM;
}

