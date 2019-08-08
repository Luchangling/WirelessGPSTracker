/**
    电源管理服务
*/

#include "expmu_service.h"
#include "hard_ware.h"
#include "expmu.h"
#include "wifi_service.h"
#include "gps.h"
#include "gsm.h"
#include "gm_network.h"
#include "gps_service.h"
#include "config_service.h"
#include "config_save.h"
#include "utility.h"
#include "log_service.h"
#include "gm_timer.h"
#include "json.h"
#include "stdlib.h"
#include "gm_stdlib.h"
#include "calc_wkp_time.h"
#include "protocol_2929.h"
#include "command.h"
#include "update_service.h"
#include "gprs.h"
#include "auto_test.h"
#include "gm_gprs.h"

GM_ERRCODE expmu_service_close(void);


/*设备强制工作时间,计时起点：收到EX PMU的所有信息*/
#define DEVICE_FORCE_WORK_TIME 90

/*唤醒时等待GPS定位时间*/
#define WKP_WAIT_GPS_POSITION_TIM 90

/*上电时等待GPS定位时间为4分钟*/
#define PWR_ON_WAIT_GPS_POSITION_TIM 240

/*盲点数据上传的强制工作最长时间 100s*/
#define HISDAT_UPLOAD_DEVICE_FORCE_WORK_TIME 100

/*非EXPMU上电，设备最长工作时间*/
#define DEVICE_MAX_WORK_TIME 300

/*EXPMU上电，设备最长工作时间*/
#define DEVICE_POWER_ON_MAX_WORK_TIME 300

/*注网限制时间*/
#define CANNOT_REGISER_NET_TIME_LIMIT 70

#define IN_LIMIT_REPORT_STAGE_GPS_WORK_TIME 40

#define CURRENT_COMPILTE_TIME 1560945514ul

typedef enum
{
    /*上电初始化*/
    EXPMU_SRV_INIT,
    /*上报位置*/
    EXPMU_SRV_SEND,
    /*等待上报位置完成*/
    EXPMU_SRV_WAIT_UPLOAD_FINISH,
    /*准备进入低功耗计算下次唤醒时间点*/
    EXPMU_SRV_PRAPER_INTO_LPM,
    /*设置EX PMU下次唤醒时间点*/
    EXPMU_SRV_NEXT_WKP_TIME_SET_PROC,
    /*使能EX PMU低功耗*/
    EXPMU_SRV_INTO_LPM,
    /*关闭电源管理服务*/
    EXPMU_SRV_CLOSE,
    
}Expmu_Srv_Stage_Enum;

typedef enum
{
    TRACK_MODE_OPEN_GPS_PWR,
    TRACK_MODE_REPORT_GPS
}Expmu_Srv_Track_Mode_Report_Enum;
enum
{
    LIMIT_REPORT_WAIT_OPEN_GPS_POWER,

    LIMIT_REPORT_WAIT_REPORT_LOCATION,

    LIMIT_REPORT_WAIT_REPORT_SUCCESS

};

typedef enum
{
   ERROR_NONE = 0,
    
   GSM_SIM_INVALID = -1,

   GSM_CREG_INVALID = -2,

   GSM_CGREG_INVALID = -3,

   GSM_CSQ_TOO_LOW = -4,
    
   GPS_DATA_CANNOT_SEND_COMPLETE = -5,

   SET_NEXT_WKP_TIME_FAIL = -6,

   SET_FORCE_SLEEP_FAIL = -7,
   
}ExpmuSrvErrorCode;


typedef struct
{
    u8 inited;

    u8 start_report_flg;

    Expmu_Srv_Stage_Enum stage;

    u8 is_expmu_power_on;

    u8 report_gps_data_cnt;

    u8 positioned;

    u8 limit_report_stage;

    u8 expmu_next_wkp_time[7];

    u8 fail_cnt;

    u8 track_mode_report_stage;

    s16 gps_service_error_code;

    s16 error_code;

    u16 report_gps_data_interval;

    u32 rep_gps_data_success_cnt;

    u32 work_timeout;

    u32 reg_set_start_time;

    u32 start_report_time;

    u32 gps_power_on_time;

}Expmu_Srv_Proc_Struct;

Expmu_Srv_Proc_Struct s_expmu_srv;

typedef struct
{
    u8 next_wkp_time[7];
    u8 force_work;
    u8 wait_tim;
}Expmu_Register_Map_Struct;

Expmu_Register_Map_Struct s_expmu_reg;


void expmu_srv_trans_stage(Expmu_Srv_Stage_Enum stage)
{

    s_expmu_srv.stage = stage;
}

u32 get_unlimit_report_stop_time(void)
{
    //void *ptr = NULL;

    u8 mode = 0;

    PlatLoopModeStruct *loop = NULL;

    loop = get_cur_gw_work_mode(&mode);

    if(mode == TRACK_MODE)
    {
        return 0x3FFFFFFF;
    }

    return loop->loop_stop_tim;
}

u16 expmu_service_get_report_interval(void)
{
    return s_expmu_srv.report_gps_data_interval;
}
/*判断当前的工作模式是否需要低功耗        0:不需要             */
u8 does_the_device_need_into_lpm(void)
{
    void *ptr = NULL;

    u8 ret = 0,mode = 0;

    static u32 s_cur_time = 0;

    static u32 s_start_calc_time = 0;

    TrackeModeTimeValueStruct *track = NULL;

    PlatLoopModeStruct *loop = NULL;

    /*防止频繁读取RTC时间 5s读取一次*/
    if(util_clock() - s_start_calc_time >= 5)
    {
        s_cur_time = util_get_utc_time();

        s_start_calc_time = util_clock();
    }

    ptr = get_cur_gw_work_mode(&mode);


    if(mode == TRACK_MODE)
    {
        track = (TrackeModeTimeValueStruct *)ptr;

        if(track->interval > 0 && track->interval < TRACK_MODE_NEED_INTO_SLEEP_INTERVAL)
        {
            //smsp_set_pmu_force_work();

            s_expmu_srv.report_gps_data_interval = track->interval*60;
            
            return 0;
        }
    }
    else if(mode == PLATFORM_CYCLIC_MODE)
    {
        loop = (PlatLoopModeStruct *)ptr;

        if(loop->loop_stop_tim == 0)
        {
            if(loop->loop_start_tim < s_cur_time)
            {
                ret = 1; /*在设置的时间范围内*/
            }
        }
        else
        {
            if(loop->loop_stop_tim > s_cur_time)
            {
                ret = 1;
            }
        }

        if(loop->loop_interval > 0 && loop->loop_interval < TRACK_MODE_NEED_INTO_SLEEP_INTERVAL && ret == 1)
        {
            //smsp_set_pmu_force_work();

            s_expmu_srv.report_gps_data_interval = loop->loop_interval*60;
            
            return 0;
        }
    }

    return 1;
}

static u32 get_device_rep_gps_data_success_cnt(void)
{
    statistical_info_struct *sta = system_state_get_statis_pointer();

    return sta->send_gps_success_count;
}

/*不需要休眠返回0*/
static u32 get_device_max_work_time(void)
{
    u16 power_on_report_cnt = 0;

    u32 work_timeout = 0;
    
    //config_service_get(CFG_PWR_REP_CNT,TYPE_SHORT,&power_on_report_cnt,sizeof(u16));
    
    /*不需要休眠*/
    if(!does_the_device_need_into_lpm())
    {
       work_timeout = 0;
    }
    else
    {
        /*EXPMU重新上电*/
        if(is_expmu_power_on())
        {
            /*最大工作时间判断的基准为当前的时间，所以要加上当前时间*/
            work_timeout = DEVICE_POWER_ON_MAX_WORK_TIME + util_clock();
        }
        else
        {
            work_timeout = DEVICE_MAX_WORK_TIME + util_clock();
        }
    }

    LOG(INFO,"get dev max work time %d, poweron report cnt %d",work_timeout,power_on_report_cnt);

    return work_timeout;
}

static void expmu_service_init_proc(void) 
{
    //u8  ret;
    u16  wait_gps_time = 0;
    u16  report_interval = 0;

    if(!does_the_device_need_into_lpm())
    {
    
        wait_gps_time = 0xFFFF;
        report_interval = 0xFFFF;
        s_expmu_srv.gps_power_on_time = util_clock();
        gps_power_on(wait_gps_time,report_interval);
        
    }

    /*记录初始发送GPS成功次数*/
    s_expmu_srv.rep_gps_data_success_cnt = get_device_rep_gps_data_success_cnt();

    expmu_srv_trans_stage(EXPMU_SRV_SEND);

    LOG(INFO,"clock(%d) expmu srv init OK",util_clock());
}

ExpmuSrvErrorCode get_gsm_error_code(void)
{
    if(get_sim_card_sate() == false)
    {
        LOG(WARN,"clock(%d) sim card invalid!",util_clock());
        
        return GSM_SIM_INVALID;
    }
    else
    {
        if(gsm_get_creg_state() != GSM_CREG_REGISTER_LOCAL && gsm_get_creg_state() != GSM_CREG_REGISTER_ROAM)
        {
            LOG(WARN,"clock(%d) creg(%d) register fail!",util_clock(),gsm_get_creg_state());
            
            return GSM_CREG_INVALID;
        }
        else
        {
            if(!gprs_is_ok())
            {
                LOG(WARN,"clcok(%d) gprs cannot regoster ok!",util_clock());
                
                return GSM_CGREG_INVALID;
            }
            else
            {
                if(gsm_get_csq() < 5)
                {
                    LOG(WARN,"clock(%d) gsm csq(%d) too low!!",util_clock(),gsm_get_csq());

                    return GSM_CSQ_TOO_LOW;
                }
                    
            }
        }
    }

    return GSM_CGREG_INVALID;
    
}
#define LBS_RX_LEV_CHANGE_THRELOD 5
/*判断设备注册的LBS信息是否变化 1:有变化*/
static u8 device_register_lbs_changed(gm_cell_info_struct *now ,gm_cell_info_struct *pre)
{
    u8 ret = 0,i = 0,change = 1;
    
    if(now->serv_info.lac == pre->serv_info.lac && \
       now->serv_info.ci  == pre->serv_info.ci)
    {
        LOG(INFO,"serv_lbs_info no change!!");

        LOG(INFO,"serv_lbs_info rxlev change %d",abs(now->serv_info.rxlev - pre->serv_info.rxlev));
        if(abs(now->serv_info.rxlev - pre->serv_info.rxlev) < LBS_RX_LEV_CHANGE_THRELOD)
        {
            LOG(INFO,"nbr_cell_num %d,%d",now->nbr_cell_num,pre->nbr_cell_num);
            if(now->nbr_cell_num == pre->nbr_cell_num)
            {
                for(i = 0; i < now->nbr_cell_num ; i++)
                {
                    if((now->nbr_cell_info[i].lac != pre->nbr_cell_info[i].lac) || \
                       (now->nbr_cell_info[i].ci  != pre->nbr_cell_info[i].ci)  || \
                       (abs(now->nbr_cell_info[i].rxlev - pre->nbr_cell_info[i].rxlev) > LBS_RX_LEV_CHANGE_THRELOD))
                    {
                        ret = 1;
                        
                        break;
                    }
                }

                if((ret == 0)&&(now->nbr_cell_num > 0))
                {
                    change = 0;
                }
            }
        }
    }

    *pre = *now;
       
    return change;
}

static u8 does_need_open_gps_power(void)
{
    u32 cur_time = util_clock();

    gm_cell_info_struct now_lbs;
    
    static gm_cell_info_struct pre_lbs;
    /*1,通过基站的变化决定是否打开GPS
      基站有变化打开GPS定位
      基站无变化
      (1,距离上次定位未超过5分钟不需开启GPS电源)
    */
    if(gsm_get_cell_info(&now_lbs) == GM_SUCCESS)
    {
        if(device_register_lbs_changed(&now_lbs,&pre_lbs) == 1)
        {
            LOG(INFO,"clock(%d) expmu srv LBS info change open gps!",util_clock());
            return 1;
        }
        else
        {
            if(cur_time - (u32)get_last_fixed_time() > 300)
            {
                LOG(INFO,"clock(%d) expmu srv LBS info no change but position timeout!",util_clock());
                return 1;
            }
            /*无变化,且上次定位时间离现在未超过5分钟,不需打开GPS*/
            LOG(INFO,"clock(%d) expmu srv LBS no change and position valid , close gps",util_clock());
            return 0;
        }
    }

    LOG(WARN,"clock(%d) expmu srv LBS info invalid!",util_clock());

    return 1;
}

static bool does_the_last_gps_data_valid(GPSData *gps)
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


static void expmu_srv_track_mode_report_proc(void)
{
    if(gps_is_fixed())
    {
        if(gps_get_state() != GM_GPS_OFF)
        {
            gps_power_off();
        }
    }
    
    switch(s_expmu_srv.track_mode_report_stage)
    {
        case TRACK_MODE_OPEN_GPS_PWR:
            if(util_clock() - s_expmu_srv.start_report_time >= (s_expmu_srv.report_gps_data_interval - 35))
            {

                if(gps_get_state() == GM_GPS_OFF)
                {
                    if(does_need_open_gps_power())
                    {
                        //s_expmu_srv.gps_power_on_time = util_clock();
                        
                        gps_power_on(0xFFFF,0xFFFF);//
                    }

                    hard_ware_awake();

                    expmu_exit_sleep();
                }

                s_expmu_srv.track_mode_report_stage = TRACK_MODE_REPORT_GPS;
            }
            break;
        case TRACK_MODE_REPORT_GPS:
            
            if(util_clock() - s_expmu_srv.start_report_time  >= s_expmu_srv.report_gps_data_interval)
            {
                GPSData gps = {0};
                
                expmu_srv_trans_stage(EXPMU_SRV_WAIT_UPLOAD_FINISH);

                s_expmu_srv.track_mode_report_stage = TRACK_MODE_OPEN_GPS_PWR;

                s_expmu_srv.rep_gps_data_success_cnt = get_device_rep_gps_data_success_cnt();

                if(does_the_last_gps_data_valid(&gps))
                {
                    gps_power_off();

                    gps.gps_time = util_get_utc_time();

                    gps_service_push_one_gps(&gps);
                }
                else
                {
                    gps_service_push_lbs();

                    if(util_clock() - s_expmu_srv.gps_power_on_time >= 300)
                    {
                        gps_power_off();
                    }
                }
                //Send gps
            }
            break;
            default:
               break;
    }
}

static void expmu_srv_upload_location_proc(void)
{  
    if(does_the_device_need_into_lpm())
    {
        /*发送成功或者超时进入休眠*/
        if((util_clock() > s_expmu_srv.work_timeout)||(get_device_rep_gps_data_success_cnt() - s_expmu_srv.rep_gps_data_success_cnt >= 1))
        {
            expmu_srv_trans_stage(EXPMU_SRV_PRAPER_INTO_LPM);
        }
    }
    else
    {
        expmu_srv_track_mode_report_proc();
    }
  
}

static void expmu_srv_wait_location_report_result(void)
{
    if(get_device_rep_gps_data_success_cnt() - s_expmu_srv.rep_gps_data_success_cnt >= 1)
    {
        GPSData gps = {0};

        s_expmu_srv.start_report_time = util_clock();
        
        expmu_srv_trans_stage(EXPMU_SRV_SEND);

        if(gps_get_last_data(&gps))
        {
            if(gps_get_state() != GM_GPS_OFF)
            {
                gps_power_off();
            }
            
            hard_ware_sleep();

            expmu_enter_sleep();
        }
    }
}

GM_ERRCODE expmu_service_creat(void)
{
    u16  wait_gps_time = 0;
    u16  report_interval = 0;
    
    LOG(INFO,"clock(%d) expmu service normal work!",util_clock());
    
    GM_memset((u8 *)&s_expmu_srv.inited , 0 , sizeof(Expmu_Srv_Proc_Struct));

    s_expmu_srv.start_report_time = util_clock();

    s_expmu_srv.stage = EXPMU_SRV_INIT;

    s_expmu_srv.error_code = 0;

    s_expmu_srv.positioned = 0;

    s_expmu_srv.limit_report_stage = LIMIT_REPORT_WAIT_OPEN_GPS_POWER;

    s_expmu_srv.inited = 1;

    s_expmu_srv.track_mode_report_stage = TRACK_MODE_OPEN_GPS_PWR;

    if(auto_test_is_working() == false)
    {
        /*最大工作时间*/
        s_expmu_srv.work_timeout = get_device_max_work_time();

        if(!does_the_device_need_into_lpm())
        {
            /*在发送前的35s打开 wifi电源*/
            wifi_power_on(20);
        }
        else
        {
            /*判断当前EXPMU启动原因*/
            if(get_expmu_wkp_reason() == EVENT_EXTI1_WKP)
            {

                wait_gps_time = 30;

                report_interval = 0;
            }
            else
            {

                wait_gps_time = WKP_WAIT_GPS_POSITION_TIM;

                report_interval = 0;
            }

            wifi_power_on(20);

            s_expmu_srv.gps_power_on_time = util_clock();

            gps_power_on(wait_gps_time,report_interval);
        }
    }
    else
    {
        s_expmu_srv.work_timeout = 0x7FFFFFFF;

        //wait_gps_time = WKP_WAIT_GPS_POSITION_TIM;
        /*上报间隔 在does_the_device_need_into_lpm中计算得到*/
        report_interval = 10;

        wait_gps_time = 10;
        
        wifi_power_on(10);

        s_expmu_srv.gps_power_on_time = util_clock();

        gps_power_on(wait_gps_time,report_interval);
    }

    smsp_set_pmu_force_work();

    return GM_SUCCESS;
}




/*准备进入低功耗时要把下次唤醒时间点计算出来*/
static void expmu_srv_praper_into_lpm_proc(void)
{
    u32 cur_time = 0,next_time = 0 , recalc_next_time = 0;

    u16 rep_gap = 0;
    
    u8 zone = 0,week_day = 0;

    u8 next_bcd_time[7] = {0};

    //char on_off = 0;

    statistical_info_struct *sta = NULL;
    
    /*update service是工作状态或者在等待重启，无须进入休眠*/
    if((update_service_get_status() != SOCKET_STATUS_DATA_FINISH 
        && update_service_get_status() != SOCKET_STATUS_ERROR)
        ||(update_service_is_waiting_reboot() == true))
    {
        return;
    }

    /*测试或老化模式不进入休眠*/
    if(auto_test_is_working() == true) return;
    #if 0
    GM_StopTimer(GM_TIMER_DELAY_EXIT_FLYMOD);

    GM_GetSetFlightMode(1,&on_off);
    #endif
    sta = system_state_get_statis_pointer();

    cur_time = util_get_utc_time();

    config_service_get(CFG_TIME_ZONE,TYPE_BYTE,&zone,sizeof(u8)); 

    config_service_get(CFG_REP_GAP,TYPE_SHORT,&rep_gap,sizeof(u16));
    //计算下个唤醒时间点
    next_time = get_next_wake_up_time_point(&week_day,cur_time);

    LOG(INFO,"Normal next wkp time point:%d,w:%d",next_time,week_day);

    /*如果发送成功记录没变则说明没有发送成功*/
    if(get_device_rep_gps_data_success_cnt() == s_expmu_srv.rep_gps_data_success_cnt)
    {
        s_expmu_srv.gps_service_error_code = GPS_DATA_CANNOT_SEND_COMPLETE;
    }
    
    /*需要重新计算下个唤醒时间点*/
    if((s_expmu_srv.gps_service_error_code < 0)&&(rep_gap > 0)&&(next_time > 0))
    {
        /*如果上报失败,设备工作时间大于2分钟，剩余条数减1*/
        if(util_clock() > 120)config_service_set_reming_gps_count();
        
        recalc_next_time = recalc_next_wkp_time(cur_time, next_time);

        LOG(INFO,"Error process(%d) next wkp time point:%d",recalc_next_time,next_time);

        if(recalc_next_time > 0)
        {
            next_time = recalc_next_time;

            week_day = 0;

            //clear = 0; /*不需要清空数据*/
        }
    }

    util_utc_sec_to_bcdtime_base2000(next_time,next_bcd_time,zone);


    if(next_time > 0)
    {
        //正常时间点
        GM_memcpy(s_expmu_srv.expmu_next_wkp_time,next_bcd_time, 6);

        s_expmu_srv.expmu_next_wkp_time[6] = week_day;

        LOG(INFO,"Expmu next wkp time %x-%x-%x %x:%x:%x w: %d",next_bcd_time[0],next_bcd_time[1],next_bcd_time[2],\
        next_bcd_time[3],next_bcd_time[4],next_bcd_time[5],next_bcd_time[6]);

        /*下个上报时间设置*/
        smsp_set_next_wakeup_time(next_bcd_time,week_day);

        s_expmu_srv.reg_set_start_time = util_clock();

        s_expmu_srv.fail_cnt = 0;

        sta->pre_second_wkp_time = sta->pre_first_wkp_time;

        sta->pre_first_wkp_time = next_time;

        sta->pre_report_fail_reason = s_expmu_srv.gps_service_error_code;

        sta->pre_csq = gsm_get_csq();

        sta->pre_sim_sta = get_sim_card_sate();

        sta->pre_creg = gsm_get_creg_state();

        sta->pre_cgreg = gprs_is_ok();

        expmu_srv_trans_stage(EXPMU_SRV_NEXT_WKP_TIME_SET_PROC);

        system_state_set_boot_reason(GM_REBOOT_POWER_ON);

        config_service_save_to_local();

        system_state_set_static_info();
        
    }
    else
    {
        /*不考虑多线程的影响*/
        /*下个唤醒时间点为0,可能在串口回调中完成了工作模式切换*/
        //TODO LCL
        
    }
  
}

static void expmu_srv_next_wkp_time_set_proc(void)
{
    u32 cur_time = util_clock();

    /*EXPMU寄存器未设置成功*/
    if(GM_memcmp((u8 *)s_expmu_srv.expmu_next_wkp_time,get_expmu_next_wkp_time_register(),7) != 0)
    {
        /*超过3s未设置成功*/
        if(cur_time - s_expmu_srv.reg_set_start_time > 3)
        {   
            s_expmu_srv.reg_set_start_time = util_clock();
            
            if(s_expmu_srv.fail_cnt < 3)
            {
                s_expmu_srv.fail_cnt++;

                LOG(WARN,"expmu srv set next wakeup time retry!");
            
                smsp_set_next_wakeup_time(s_expmu_srv.expmu_next_wkp_time,s_expmu_srv.expmu_next_wkp_time[6]);
            }
            else
            {
                //设置不成功，进入睡眠，不在从睡眠中唤醒，除非重启。
                /*异常处理*/
                s_expmu_srv.fail_cnt = 0;

                s_expmu_srv.reg_set_start_time = util_clock();

                s_expmu_srv.error_code = SET_NEXT_WKP_TIME_FAIL;

                //smsp_clear_pmu_force_work();

                smsp_comm_pmu_into_sleep();

                expmu_srv_trans_stage(EXPMU_SRV_INTO_LPM);

                LOG(ERROR,"expmu srv set next wakeup time fail!");
            }
        }
    }
    else
    {
        LOG(INFO,"clock(%d) expmu srv set next wakeup time ok!",util_clock());
        
        s_expmu_srv.reg_set_start_time = util_clock();

        //smsp_clear_pmu_force_work();

        //smsp_comm_pmu_into_sleep();
        
        expmu_srv_trans_stage(EXPMU_SRV_INTO_LPM);
    }
}

static void expmu_srv_into_lpm_proc(void)
{
   u32 cur_time = util_clock();

   if(does_expmu_in_sleeping() == 0)
   {
       if(cur_time  - s_expmu_srv.reg_set_start_time > 5)
       {
            s_expmu_srv.reg_set_start_time = cur_time;
        
            if(s_expmu_srv.fail_cnt < 4)  
            {
                s_expmu_srv.fail_cnt++;

                LOG(WARN,"clock(%d) expmu enter sleep retry!",util_clock());

                //smsp_clear_pmu_force_work();

                smsp_comm_pmu_into_sleep();
            }
            else
            {
                LOG(ERROR,"clock(%d) expmu enter sleep fail!",util_clock());
                
                s_expmu_srv.error_code = SET_FORCE_SLEEP_FAIL;

                expmu_srv_trans_stage(EXPMU_SRV_CLOSE);
            }
       }
   }
   else
   {
        LOG(INFO,"clock(%d) expmu in sleeping OK!",util_clock());
   
        expmu_srv_trans_stage(EXPMU_SRV_CLOSE);
   }

}

/*just run step by step*/
GM_ERRCODE expmu_service_timer_proc(void)
{
    if(!s_expmu_srv.inited) return GM_NOT_INIT;

    if(auto_test_is_working()) return GM_SUCCESS;

    switch(s_expmu_srv.stage)
    {
        case EXPMU_SRV_INIT:
            expmu_service_init_proc();
            break;
       case EXPMU_SRV_SEND:
            expmu_srv_upload_location_proc();
            break;
        case EXPMU_SRV_WAIT_UPLOAD_FINISH:
            expmu_srv_wait_location_report_result();
            break;
        case EXPMU_SRV_PRAPER_INTO_LPM:
            expmu_srv_praper_into_lpm_proc();
            break;
        case EXPMU_SRV_NEXT_WKP_TIME_SET_PROC:
            expmu_srv_next_wkp_time_set_proc();
            break;
        case EXPMU_SRV_INTO_LPM:
            expmu_srv_into_lpm_proc();
            break;
        case EXPMU_SRV_CLOSE:
            expmu_service_close();
            break;
        default:
        break;
    }

    return GM_SUCCESS;
}

GM_ERRCODE expmu_service_close(void)
{
    gps_power_off();
    
    hard_ware_sleep();
    
    GM_StopTimer(GM_TIMER_10MS_MAIN);

    return GM_SUCCESS;
}

GM_ERRCODE expmu_service_destory(void)
{
    LOG(INFO,"clock(%d) expmu service destory !",util_clock());

    if(s_expmu_srv.inited == 0) return GM_SUCCESS;
    
    s_expmu_srv.inited = 0;

    expmu_clear_smap_stack();

    gps_power_off();
    
    expmu_service_creat();

    gps_service_clear_all_wait_gps();

    return GM_SUCCESS;
}


bool expmu_service_is_working(void)
{
    return s_expmu_srv.inited > 0?true:false;
}

