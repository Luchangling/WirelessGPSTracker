/**
    WIFI服务模块
*/
#include "gm_memory.h"
#include "gm_stdlib.h"
#include "utility.h"
#include "log_service.h"
#include "config_service.h"
#include "wifi_service.h"
#include "gm_timer.h"
#include "led.h"

/*去初始化回调*/
static wifi_scan_result_struct s_wifi;


static void wlan_deinit_cb(void *user_data,wlan_deinit_cnf_struct *cnf)
{
    if(cnf->status == SCANONLY_SUCCESS)
    {
        LOG(INFO,"Wlan deinit sucess!");
    }
    else
    {
        LOG(ERROR,"wlan deinit fail code %d",cnf->status);

        if(s_wifi.fail_cnt < 3)
        {
            s_wifi.fail_cnt++;
            
            wlan_deinit(NULL,wlan_deinit_cb,NULL);
        }        

    }
}

static void save_cnf_to_buf(wlan_scan_cnf_struct *cnf , wlan_scan_cnf_struct *save)
{
    *save = *cnf;
}

static void wlan_scan_cb(void *user_data,wlan_scan_cnf_struct *cnf)
{
    if(cnf->status==0)
    {
        s_wifi.record_time = util_clock();

        save_cnf_to_buf(cnf,&s_wifi.info);

        LOG(INFO,"clock(%d),status %d,%d num %d,%d",util_clock(),cnf->status,s_wifi.info.status,cnf->scan_ap_num,s_wifi.info.scan_ap_num);

        LOG(INFO,"clock(%d) wlan deint..",util_clock());
        
        wlan_deinit(NULL,wlan_deinit_cb,NULL); 
    }
    else
    {
        LOG(ERROR,"Wlan scan fail code %d",cnf->status);

        if(s_wifi.fail_cnt < 3)
        {
            s_wifi.fail_cnt++;
            
            wlan_scan(NULL,wlan_scan_cb,NULL);
        }          

    }
}


/*初始化回调*/
static void wlan_init_cb(void *user_data,wlan_init_cnf_struct *cnf)
{
    if(cnf->status == SCANONLY_SUCCESS)
    {
        led_set_wifi_state(GM_LED_FLASH);
        
        LOG(INFO,"Wlan init success, begin scan AP!");       

        s_wifi.fail_cnt = 0;

        wlan_scan(NULL,wlan_scan_cb,NULL);
    }
    else
    {
        LOG(ERROR,"wlan init fail code %d ,reinit!",cnf->status);

        if(s_wifi.fail_cnt < 3)
        {
            s_wifi.fail_cnt++;
            
            wlan_init(NULL,wlan_init_cb,NULL);
        }
        else
        {
            led_set_wifi_state(GM_LED_OFF);
        }

        
    }
}

void wifi_service_period_scan_wifi(void)
{
    LOG(INFO,"Wlan INIT!!");
        
    wlan_init(NULL,wlan_init_cb,NULL);
    
    GM_StartTimer(GM_TIMER_SCAN_WIFI_AP,s_wifi.scan_rate*TIM_GEN_1SECOND,wifi_service_period_scan_wifi);
}

GM_ERRCODE wifi_power_on(u8 scan_rate)
{ 
    u8 state = 0;

    led_set_wifi_state(GM_LED_OFF);

    config_service_get(CFG_WIFI_STATE,TYPE_BYTE,&state,sizeof(u8));

    if(state)
    {
        s_wifi.fail_cnt = 0;

        LOG(INFO,"clock(%d) wifi power on!",util_clock());

        GM_StopTimer(GM_TIMER_SCAN_WIFI_AP);

        if(scan_rate >= 10)
        {
            s_wifi.scan_rate = scan_rate;
            
            GM_StartTimer(GM_TIMER_SCAN_WIFI_AP,s_wifi.scan_rate*TIM_GEN_1SECOND,wifi_service_period_scan_wifi);
        }

        LOG(INFO,"Wlan INIT!!");
        
        wlan_init(NULL,wlan_init_cb,NULL);
    }

    return GM_SUCCESS;
}


GM_ERRCODE wifi_power_off(void)
{ 

    s_wifi.fail_cnt = 0;

    led_set_wifi_state(GM_LED_OFF);
    
    wlan_deinit(NULL,wlan_deinit_cb,NULL);

    GM_StopTimer(GM_TIMER_SCAN_WIFI_AP);

    return GM_SUCCESS;
}

GM_ERRCODE wifi_service_creat(void)
{
    GM_memset((u8 *)&s_wifi,0,sizeof(s_wifi));

    return GM_SUCCESS;
}


GM_ERRCODE wifi_service_timer_proc(void)
{
    u32 cur_time = util_clock();

    u8 state = 0;

    config_service_get(CFG_WIFI_STATE,TYPE_BYTE,&state,sizeof(u8));

    if(cur_time - s_wifi.start_time > 20)
    {
        s_wifi.start_time = cur_time;

        if(state)
        {
            
        }
    }
    
    return GM_SUCCESS;
}

wifi_scan_result_struct *wifi_service_get_wifi_scan_result(void)
{
    return &s_wifi;
}


GM_ERRCODE wifi_service_destory(void)
{
    return GM_SUCCESS;
}

