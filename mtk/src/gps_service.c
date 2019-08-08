
#include "gm_type.h"
#include "stdio.h"
#include "gm_stdlib.h"
#include "gm_gprs.h"
#include "gps_service.h"
#include "fifo.h"
#include "gps.h"
#include "protocol.h"
#include "log_service.h"
#include "gm_memory.h"
#include "utility.h"
#include "gsm.h"
#include "config_service.h"
#include "gprs.h"
#include "system_state.h"
#include "led.h"
#include "update_service.h"
#include "hard_ware.h"
#include "applied_math.h"
#include "system_state.h"
#include "protocol_2929.h"
#include "gm_timer.h"


static void gps_service_close(void);
GM_ERRCODE gps_service_destroy(void);
void gps_service_destroy_gprs(void);
static u8 does_gps_service_need_upload(void);



#define GPS_PING_TIME 60



typedef struct
{
    u32 data_finish_time;

    u32 saved_socket_ack;  //发消息前，ack的值

    u32 wait_send_count;

    u32 start_time;

    u8  start_send_flg;

    u8 test_mode_report_ok;

    
}SocketTypeExtend;



static SocketType s_gps_socket = {-1,"",SOCKET_STATUS_ERROR,};
static SocketTypeExtend s_gps_socket_extend;


static GM_ERRCODE gps_service_transfer_status(u8 new_status)
{
    u8 old_status = (u8)s_gps_socket.status;
    GM_ERRCODE ret = GM_PARAM_ERROR;
    switch(s_gps_socket.status)
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
                    ret = GM_SUCCESS;
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
        s_gps_socket.status = new_status;
        s_gps_socket.status_fail_count = 0;
        LOG(INFO,"clock(%d) gps_service_transfer_status from %s to %s success", util_clock(), 
            gm_socket_status_string((SocketStatus)old_status), gm_socket_status_string((SocketStatus)new_status));
    }
    else
    {
        LOG(WARN,"clock(%d) gps_service_transfer_status assert(from %s to %s) failed", util_clock(), 
            gm_socket_status_string((SocketStatus)old_status), gm_socket_status_string((SocketStatus)new_status));
    }

    return ret;

}

u8 gps_service_socket_is_same(void)
{
    u8 addr[CONFIG_STRING_MAX_LEN+1];
    u8 IP[4];
    u32 port = 0;
    u8 idx = 0;

    GM_memset(addr, 0x00, sizeof(addr));
    idx = GM_sscanf((const char *)config_service_get_pointer(CFG_SERVERADDR), "%[^:]:%d", addr, &port);
    if (idx != 2)
    {
        LOG(WARN,"clock(%d) gps_service_socket_is_same assert(idx ==2) failed.",util_clock());
        return 1; // not change.
    }

    if(s_gps_socket.port != port)
    {
        return 0;
    }
    
    if(GM_SUCCESS == GM_ConvertIpAddr(addr, IP))
    {
        if(s_gps_socket.ip[0] == IP[0] && s_gps_socket.ip[1] == IP[1] && 
            s_gps_socket.ip[2] == IP[2] && s_gps_socket.ip[3] == IP[3])
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        if(0 == GM_strcmp(s_gps_socket.addr, (const char *)addr))
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    // return 1;
}


void gps_service_connection_ok(void)
{
    gps_service_transfer_status(SOCKET_STATUS_WORK);
    
}

void gps_service_connection_failed(void)
{
    gps_service_close();
    
    if(s_gps_socket.status_fail_count >= MAX_CONNECT_REPEAT)
    {
        // if excuted get_host reinit gprs, else get_host.
        if(s_gps_socket.excuted_get_host || (s_gps_socket.addr[0] == 0))
        {
            //  reinit gprs
            LOG(DEBUG,"clock(%d) gps_service_connection_failed excuted_get_host(%d)",util_clock(),s_gps_socket.excuted_get_host);
            gps_service_destroy_gprs();
        }
        else
        {
            gps_service_transfer_status(SOCKET_STATUS_INIT);
        }
    }
    //else do nothing. wait gps_service_connecting_proc to deal
}



static void gps_service_connecting_proc(void)
{
    u32 current_time = util_clock();

    if((current_time - s_gps_socket.send_time) > CONNECT_TIME_OUT)
    {
        s_gps_socket.status_fail_count ++;
        gps_service_connection_failed();

        if(s_gps_socket.status == SOCKET_STATUS_CONNECTING && 
            s_gps_socket.status_fail_count < MAX_CONNECT_REPEAT)
        {
            if(GM_SUCCESS == gm_socket_connect(&s_gps_socket))
            {
                //do nothing. wait callback
            }
        }
        
    }
}



static void gps_service_data_finish_proc(void)
{
    u32 current_time = util_clock();
    if((current_time - s_gps_socket_extend.data_finish_time) > GPS_PING_TIME)
    {
        LOG(DEBUG,"clock(%d) gps_service_data_finish_proc cur(%d) - fin(%d) > GPS_PING_TIME(%d).",
            util_clock(), current_time, s_gps_socket_extend.data_finish_time,GPS_PING_TIME);

        // 可以重建连接
        gps_service_transfer_status(SOCKET_STATUS_INIT);
    }
}


GM_ERRCODE gps_service_send_one_locate(void)
{
    u8 buff[500];
    u16 idx = 0;
    statistical_info_struct *sta = system_state_get_statis_pointer();
   

    LOG(DEBUG,"clock(%d) gps_service_send_one_locate mode()",util_clock());
    if(s_gps_socket.status != SOCKET_STATUS_LOGIN && s_gps_socket.status != SOCKET_STATUS_WORK)
    {
        LOG(WARN,"clock(%d) gps_service_send_one_locate socket->status(%s) error.", util_clock(), gm_socket_status_string((SocketStatus)s_gps_socket.status));
        return GM_PARAM_ERROR;
    }
    
    idx = peek_one_location_protocol_data(buff);
    
    
    if(SOCKET_STATUS_WORK == s_gps_socket.status)
    {
        LOG(INFO,"clock(%d) gps_service_send_one_locate  msglen(%d)", util_clock(), idx);
        if(GM_SUCCESS == gm_socket_send(&s_gps_socket, buff, idx))
        {
            sta->total_send_gps_count++;
            s_gps_socket.send_time = util_clock();
            return GM_SUCCESS;
        }
        else
        {
            sta->send_gps_error_count++;
            gps_service_destroy_gprs();
        }
    }

    return GM_UNKNOWN;
}



/*
    get msg from fifo
    if have no gps msg, sending heartbeat, else sending gpsmsg
*/
static void gps_service_work_proc(void)
{
    u32 cur_tim = util_clock();

    if(does_gps_service_need_upload())
    {
        if(s_gps_socket_extend.start_send_flg == 0)
        {
			LOG(INFO,"s_gps_socket_extend.start_send_flg == 0");
            if(gps_service_send_one_locate() == GM_SUCCESS)
            {
                s_gps_socket_extend.start_send_flg = 1;
            }
        }
        else
        {
            if(cur_tim - s_gps_socket.send_time >= 10)
            {
                s_gps_socket.status_fail_count++;

                if(s_gps_socket.status_fail_count < 5)
                {
                    if(gps_service_send_one_locate() != GM_SUCCESS)
                    {
                        s_gps_socket_extend.start_send_flg = 0;
                    }
                }
                else
                {
                    s_gps_socket.status_fail_count = 0;

                    s_gps_socket_extend.start_send_flg = 0;

                    gps_service_transfer_status(SOCKET_STATUS_DATA_FINISH);

                    gps_service_destroy_gprs();
                    
                }
            }
        }
            
    
    }
    
    protocol_msg_receive(&s_gps_socket);
}

/*
// have ip, use ip to connect
// no ip, call gm_socket_get_host_by_name 
*/
static void gps_service_init_proc(void)
{
    u8 IP[4];
    gm_socket_get_host_by_name_trigger(&s_gps_socket);
    system_state_get_ip_cache(SOCKET_INDEX_MAIN, IP);
    if(GM_SUCCESS == gm_is_valid_ip(IP))
    {
        GM_memcpy( s_gps_socket.ip , IP, sizeof(IP));
        gps_service_transfer_status(SOCKET_STATUS_CONNECTING);
        if(GM_SUCCESS == gm_socket_connect(&s_gps_socket))
        {
        }
        // else do nothing .   connecting_proc will deal.
    }
    else if((!s_gps_socket.excuted_get_host) && (GM_SUCCESS == gm_is_valid_ip(s_gps_socket.ip)))
    {
        gps_service_transfer_status(SOCKET_STATUS_CONNECTING);
        if(GM_SUCCESS == gm_socket_connect(&s_gps_socket))
        {
        }
        // else do nothing .   connecting_proc will deal.
    }
}


GM_ERRCODE gps_service_create(bool first_create)
{
    u8 addr[2*GOOME_DNS_MAX_LENTH+1];
    u8 IP[4];
    u32 port = 0;
    u8 idx = 0;

    /*because s_gps_socket.fifo.base_addr is not initialized, here depend on first_create
    to check base_addr is null or not*/
    if(!first_create)
    {
        if(s_gps_socket.fifo.base_addr)
        {
            return GM_SUCCESS;
        }
    }
	
    gm_socket_init(&s_gps_socket, SOCKET_INDEX_MAIN);

    GM_memset(addr, 0x00, sizeof(addr));

    if(config_service_is_test_mode() || config_service_is_default_imei())
    {
        idx = GM_sscanf((const char *)config_service_get_pointer(CFG_TEST_SERVERADDR), "%[^:]:%d", addr, &port);
        if (idx != 2)
        {
            LOG(WARN,"clock(%d) gps_service_create assert(idx ==2) failed.", util_clock());
            return GM_PARAM_ERROR;
        }
    }
    else
    {
        idx = GM_sscanf((const char *)config_service_get_pointer(CFG_SERVERADDR), "%[^:]:%d", addr, &port);
        if (idx != 2)
        {
            LOG(WARN,"clock(%d) gps_service_create assert(idx ==2) failed.", util_clock());
            return GM_PARAM_ERROR;
        }
    }
    
    if(GM_SUCCESS != GM_ConvertIpAddr(addr, IP))
    {
        if(util_is_valid_dns(addr, GM_strlen((const char *)addr)))
        {
            gm_socket_set_addr(&s_gps_socket, addr, GM_strlen((const char *)addr), port, STREAM_TYPE_DGRAM);
			system_state_get_ip_cache(SOCKET_INDEX_MAIN, IP);
			gm_socket_set_ip_port(&s_gps_socket, IP, port, STREAM_TYPE_DGRAM);
        }
        else
        {
            LOG(WARN,"clock(%d) gps_service_create assert(dns(%s)) failed.", util_clock(), addr);
            return GM_PARAM_ERROR;
        }
    }
    else
    {
        gm_socket_set_ip_port(&s_gps_socket, IP, port, STREAM_TYPE_DGRAM);
        system_state_set_ip_cache(SOCKET_INDEX_MAIN, IP);
    }



    s_gps_socket_extend.data_finish_time = 0;


    LOG(INFO,"clock(%d) gps_service_create access_id(%d) fifo(%p).", util_clock(), s_gps_socket.access_id, &s_gps_socket.fifo);

	return GM_SUCCESS;
}

static void gps_service_close(void)
{
    if(s_gps_socket.id >=0)
    {
        GM_SocketClose(s_gps_socket.id);
        s_gps_socket.id=-1;
        s_gps_socket_extend.saved_socket_ack = s_gps_socket.last_ack_seq = 0;
    }
}


GM_ERRCODE gps_service_destroy(void)
{
    if(SOCKET_STATUS_ERROR == s_gps_socket.status)
    {
        return GM_SUCCESS;
    }
    
    gps_service_close();
    led_set_gsm_state(GM_LED_FLASH);

    fifo_delete(&s_gps_socket.fifo);
    
    s_gps_socket_extend.data_finish_time = util_clock();
    gps_service_transfer_status(SOCKET_STATUS_ERROR);
    
	return GM_SUCCESS;
}
#if 0
void gps_service_exit_fly_mode(void)
{
    char on_off = 0;
    
    GM_GetSetFlightMode(1,&on_off);
}
#endif
void gps_service_destroy_gprs(void)
{
    //char on_off = 1;
    
    if(SOCKET_STATUS_DATA_FINISH == update_service_get_status() || 
        SOCKET_STATUS_ERROR == update_service_get_status())
    {
        LOG(DEBUG,"clock(%d) gps_service_destroy_gprs update_service_status(%d)",util_clock(),update_service_get_status());
        gprs_destroy();
        #if 0
        GM_GetSetFlightMode(1,&on_off);
        GM_StartTimer(GM_TIMER_DELAY_EXIT_FLYMOD, TIM_GEN_1SECOND*5, gps_service_exit_fly_mode);
        #endif
    }
    else
    {
        //wait gprs reinit
        gps_service_destroy();
    }
}



GM_ERRCODE gps_service_timer_proc(void)
{
    if(!s_gps_socket.fifo.base_addr)
    {
        return GM_SUCCESS;
    }

    switch(s_gps_socket.status)
    {
    case SOCKET_STATUS_INIT:
        gps_service_init_proc();
        break;
    case SOCKET_STATUS_CONNECTING:
        gps_service_connecting_proc();
        break;
    case SOCKET_STATUS_WORK:
        gps_service_work_proc();
        break;
    case SOCKET_STATUS_DATA_FINISH:
        gps_service_data_finish_proc();
        break;
    case SOCKET_STATUS_ERROR:
        gps_service_data_finish_proc();
        break;
    default:
        LOG(WARN,"clock(%d) gps_service_timer_proc assert(s_gps_socket.status(%d)) unknown.",util_clock(), s_gps_socket.status);

        //  reinit gprs
        gps_service_destroy_gprs();
        return GM_ERROR_STATUS;
    }

    return GM_SUCCESS;
}


U8* gps_service_get_current_ip(void)
{
	return s_gps_socket.ip;
}

void gps_service_close_for_reconnect(void)
{
    gps_service_close();
    s_gps_socket_extend.data_finish_time = util_clock();
    gps_service_transfer_status(SOCKET_STATUS_DATA_FINISH);
}

void gps_service_confirm_gps_cache(void)
{
    
}


GM_ERRCODE gps_service_change_config(void)
{
    if (! gps_service_socket_is_same())
    {
        gps_service_destroy();
        
        // clear previous connection info.
        s_gps_socket.ip[0] = s_gps_socket.ip[1] = s_gps_socket.ip[2] = s_gps_socket.ip[3] = 0;
        s_gps_socket.addr[0] = 0;
        system_state_set_ip_cache(SOCKET_INDEX_MAIN, s_gps_socket.ip);
        gps_service_create(false);
    }
    return GM_SUCCESS;
}

GM_ERRCODE gps_service_restart(void)
{
    /*service重启目前不需处理其他信息，直接重启*/
    
    gps_service_destroy();
        
    // clear previous connection info.
    s_gps_socket.ip[0] = s_gps_socket.ip[1] = s_gps_socket.ip[2] = s_gps_socket.ip[3] = 0;
    s_gps_socket.addr[0] = 0;
    system_state_set_ip_cache(SOCKET_INDEX_MAIN, s_gps_socket.ip);
    gps_service_create(false);

    return GM_SUCCESS;
}


u8 gps_service_position_send_ok(void)
{
    return s_gps_socket_extend.test_mode_report_ok;
}


/*发送一条数据,断网之后需要补发*/
void gps_service_push_one_gps(const GPSData *gps)
{
     protocol_2929_install_one_location_packet(gps);

    s_gps_socket_extend.wait_send_count = 1;
   
}

void gps_service_push_lbs(void)
{
    gm_cell_info_struct lbs;
    
    if(gsm_get_cell_info(&lbs) < 0)
    {
        GM_StopTimer(GM_TIMER_DELAY_GET_LBS_INFO);
        /*1s一次*/
        GM_StartTimer(GM_TIMER_DELAY_GET_LBS_INFO,TIM_GEN_1SECOND, gps_service_push_lbs);
        
        return;
    }
    
    protocol_2929_install_one_location_packet(NULL);

    s_gps_socket_extend.wait_send_count = 1;


    return ;
}


void gps_service_clear_all_wait_gps(void)
{
    s_gps_socket_extend.wait_send_count = 0;
}


void gps_service_clear_one_gps(void)
{
    statistical_info_struct* sta = NULL;
    
    s_gps_socket_extend.wait_send_count = 0;

    LOG(INFO,"clock(%d) gps service send position success!",util_clock());

    sta = system_state_get_statis_pointer();

    sta->send_gps_success_count++;

    if(config_service_is_test_mode())
    {
        s_gps_socket_extend.test_mode_report_ok = 1;
    }


}

static u8 does_gps_service_need_upload(void)
{
    return s_gps_socket_extend.wait_send_count > 0 ? 1: 0;
}

void gps_service_send_one_device_ack(u8 *buff , u16 len)
{
    u16 idx = 0;
   

    LOG(DEBUG,"clock(%d) gps_service_send_one_device_ack",util_clock());
    if(s_gps_socket.status != SOCKET_STATUS_LOGIN && s_gps_socket.status != SOCKET_STATUS_WORK)
    {
        LOG(WARN,"clock(%d) gps_service_send_one_device_ack socket->status(%s) error.", util_clock(), gm_socket_status_string((SocketStatus)s_gps_socket.status));
        return;
    }
    
    idx = len;
    
    
    if(SOCKET_STATUS_WORK == s_gps_socket.status)
    {
        LOG(INFO,"clock(%d) gps_service_send_one_device_ack  msglen(%d)", util_clock(), idx);
        if(GM_SUCCESS == gm_socket_send(&s_gps_socket, buff, idx))
        {
            return;
        }
        else
        {
            gps_service_destroy_gprs();
        }
    }

    return;
}

u32 gps_service_get_location_counts(void)
{
    statistical_info_struct *sta = NULL;

    sta = system_state_get_statis_pointer();

    return sta->send_gps_success_count;
}

