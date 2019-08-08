/**
* Copyright (C), Goome Technologies Co., Ltd. 2009-2019. All rights reserved.
* File name : app.c
* Author:  Version:	Date:
* Description:
* Function List:
* History:
  1.Date: 2019/4/9
    Author: Chris.Lu
    Modification: Created file
*/

#include "app.h"
#include "time.h"
#include "string.h"
#include "gm_type.h"
#include "system_ticks.h"

#include "app_rtc_alarm.h"
#include "stm8l15x_conf.h"

#include "app_fram_manger.h"
#include "bsp_light_sensor.h"
#include "bsp_watch_dog.h"
#include "feature_config.h"
#include "debug_port.h"
#include "bsp_model_power_contrl.h"
#include "app_calc_next_wkp_time.h"
#include "slave_mem_share_protocol.h"

#define offsetof(TYPE, MEMBER) ((u32) &((TYPE *)0)->MEMBER)

#define IS_VAILD_WKP_EVENT(ev) ((ev == EVENT_POWER_UP)|| \
								 (ev == EVENT_EXTI1_WKP)||\
								 (ev == EVENT_EXTI2_WKP)|| \
								 (ev == EVENT_RTC_WKP))


#define IS_VALID_BCD_TIM(bcd) ((bcd[0] >= 0x19)&&\
							   (bcd[1] >= 0x01 && bcd[1] <= 0x12)&&\
							   (bcd[2] >= 0x01 && bcd[2] <= 0x31)&&\
							   (bcd[3] <= 0x24)&&\
							   (bcd[4] <= 0x59)&&\
							   (bcd[5] <= 0x59))

FAR system_event_control_struct g_evt_ctrl;

FAR u8 g_sleep_now;

/*发送事件到主循环,并检查事件的递进关系*/
u8 send_event_to_main_hadle(SYS_EVENT_ENUM event)
{
	u8 ret = 0;
	
	switch(event)
	{
		case EVENT_POWER_UP:

			ret = 1;
			
			break;
		case EVENT_WAIT_LPM:

			switch(g_evt_ctrl.event)
			{
				case EVENT_POWER_UP:
					ret = 1;
					break;
				case EVENT_WAIT_LPM:
					
					break;
                case EVENT_PREPARE_ENTER_LPM:
					
					break;
				case EVENT_ENTER_LPM:
					
					break;

				case EVENT_EXTI1_WKP:
					
					break;
				case EVENT_EXTI2_WKP:
				
					break;
				case EVENT_RTC_WKP:
					
					break;
				case EVENT_EXIT_LPM:
					 ret = 1;
					break;
				default:
					
					break;
			}

			break;
		case EVENT_PREPARE_ENTER_LPM:

			switch(g_evt_ctrl.event)
			{
				case EVENT_POWER_UP:
					
					break;
				case EVENT_WAIT_LPM:
					ret = 1;
					break;
                case EVENT_PREPARE_ENTER_LPM:

                    break;
				case EVENT_ENTER_LPM:
					
					break;

				case EVENT_EXTI1_WKP:
					//ret = 1;
					break;
				case EVENT_EXTI2_WKP:
					//ret = 1;
					break;
				case EVENT_RTC_WKP:
					//ret = 1;
					break;
				case EVENT_EXIT_LPM:

					break;
				default:
					
					break;
			}

			break;
        case EVENT_ENTER_LPM:
            
            switch(g_evt_ctrl.event)
			{
				case EVENT_POWER_UP:
					
					break;
				case EVENT_WAIT_LPM:
					
					break;
                case EVENT_PREPARE_ENTER_LPM:
                    ret = 1;
                    break;
				case EVENT_ENTER_LPM:
					
					break;
				case EVENT_EXTI1_WKP:
					ret = 1;
					break;
				case EVENT_EXTI2_WKP:
					ret = 1;
					break;
				case EVENT_RTC_WKP:
					ret = 1;
					break;
				case EVENT_EXIT_LPM:

					break;
				default:
					
					break;
			}
            
            break;
		case EVENT_EXTI1_WKP:
	    case EVENT_EXTI2_WKP:
		case EVENT_RTC_WKP:

			switch(g_evt_ctrl.event)
			{
				case EVENT_POWER_UP:
					
					break;
				case EVENT_WAIT_LPM:
					
					break;
				case EVENT_ENTER_LPM:
					ret = 1;
					break;
				case EVENT_PREPARE_ENTER_LPM:
				
					break;
				case EVENT_EXTI1_WKP:
				
					break;
				case EVENT_EXTI2_WKP:
					
					break;
				case EVENT_RTC_WKP:
					
					break;
				case EVENT_EXIT_LPM:

					break;
				default:
					
					break;
			}

			break;
		case EVENT_EXIT_LPM:
			
			switch(g_evt_ctrl.event)
			{
				case EVENT_POWER_UP:
					
					break;
				case EVENT_WAIT_LPM:
					
					break;
                case EVENT_PREPARE_ENTER_LPM:

                    break;
				case EVENT_ENTER_LPM:
					
					break;
				case EVENT_EXTI1_WKP:
					ret = 1;
					break;
				case EVENT_EXTI2_WKP:
					ret = 1;
					break;
				case EVENT_RTC_WKP:
					ret = 1;
					break;
				case EVENT_EXIT_LPM:

					break;
				default:
					
					break;
			}
			break;
		default:
			
			break;
	}

	if(ret) 
	{
		/*获取启动ticks*/
		g_evt_ctrl.evt_hold_tim = get_system_ticks()/100;
		
		g_evt_ctrl.event = event;
		
		g_evt_ctrl.evt_hold_max_time = 0;
		
		switch(event)
		{
			case EVENT_POWER_UP:
				
				break;
			case EVENT_WAIT_LPM:
				g_evt_ctrl.evt_hold_max_time = 300ul;
				break;
            case EVENT_PREPARE_ENTER_LPM:

                break;
			case EVENT_ENTER_LPM:

				break;
			case EVENT_EXTI1_WKP:
		    case EVENT_EXTI2_WKP:
				g_evt_ctrl.evt_hold_max_time = 3ul;
				break;
			case EVENT_RTC_WKP:

				break;
			case EVENT_EXIT_LPM:
				
				break;
			default:
				
				break;
		}
	}

    return ret;
}

u8 is_event_hold_timeout(void)
{
	if((get_system_ticks()/100 - g_evt_ctrl.evt_hold_tim) > g_evt_ctrl.evt_hold_max_time)
	{
		return 1;
	}
	 return 0;
}

u8 is_need_change_event_to_lpm(void)
{
	if(g_sleep_now == 1)
	{
		g_sleep_now = 0;
		return 1;
	}
	
	/*��ʱ*/
	if(is_event_hold_timeout())
	{
		if(g_evt_ctrl.force_work)
		{
			return 0;
		}

		return 1;
	}

	return 0;
}

void set_wkp_reason(SYS_EVENT_ENUM ev)
{
	assert_param(IS_VAILD_WKP_EVENT(ev));
	
	g_evt_ctrl.wkp_reason = ev;
}

u32 get_evt_control_register_addr(void)
{
	return (u32)&g_evt_ctrl;
}

u16 get_evt_control_register_size(void)
{
	return sizeof(system_event_control_struct);
}

SYS_EVENT_ENUM get_system_event(void)
{
	return (SYS_EVENT_ENUM)g_evt_ctrl.event;
}

void app_power_up_init(void)
{
	system_ticks_statistics_power_up_init();

	app_rtc_alarm_power_up_init(); 

	app_fram_para_powerup_init();

    MCU_WKP_STATISTICS(pwr_up);

	memset((u8 *)&g_evt_ctrl,0,sizeof(g_evt_ctrl));

    delayms(200);

}

void set_evt_100ms_flg(void)
{
    //if(g_evt_ctrl.event_100ms == 0)g_evt_ctrl.event_100ms = 1;
}


void app_wait_lpm_process(void)
{	
	static u8 ticks = 0;

    static u32 start_time = 0;

    u32 time_value = 0;

	/*smsp协议栈解析*/
	slave_mem_share_protocol_prase();
	
    /*校时标志,由Master Set*/
	if(g_evt_ctrl.calibration_tim_flg == 1)
 	{
		if(IS_VALID_BCD_TIM(g_evt_ctrl.server_tim))
		{
			if(app_bcd_to_time_value(g_evt_ctrl.server_tim) > (u32)1554879748ul)
			{
				app_rtc_time_update(g_evt_ctrl.server_tim);
				
				g_evt_ctrl.calibration_tim_flg = 0;
			}
			
		}
	}

    //100ms ticks
	if((get_system_ticks() - start_time) >= 10)
	{
		if(g_evt_ctrl.wait_to_sleep > 0)
		{
			g_evt_ctrl.wait_to_sleep--;

			if(g_evt_ctrl.wait_to_sleep == 0)
			{
                #ifdef _DEBUG_
                app_get_cur_time_value(&time_value);

                if(time_value < (u32)1562122005ul)
                {
                    #ifdef _DEBUG_
                    print("\r\nclock(%d) rtc time not correct!\r\n",clock());
                    #endif
                    g_sleep_now = 0;
                    
                    bsp_model_restart();

                    #ifdef _DEBUG_
                    print("\r\nclock(%d) model restart!!\r\n",clock());
                    #endif
                }
                else
                {
                    #ifdef _DEBUG_
                    print("\r\nclock(%d) go to sleep\r\n",clock());
                    #endif
                    g_sleep_now = 1;
                }
                #endif
			}
		}
		
		if((ticks == 0)||(g_evt_ctrl.rtc_tim[0] == 0))
		{
			/*5s 获取一次RTC时间*/
			app_get_ur_rtc_time(g_evt_ctrl.rtc_tim,&g_evt_ctrl.rtc_tim[6]);

            #ifdef _DEBUG_
            print("\r\nclock(%d) mcu rtc time get %x-%02x-%02x %02x:%02x:%02x\r\n",clock(),g_evt_ctrl.rtc_tim[0],g_evt_ctrl.rtc_tim[1],\
            g_evt_ctrl.rtc_tim[2],g_evt_ctrl.rtc_tim[3],g_evt_ctrl.rtc_tim[4],g_evt_ctrl.rtc_tim[5]);
            #endif


			ticks = 50;
		}
		
		start_time = get_system_ticks();

		ticks--;
	}

	if(g_evt_ctrl.flash_commit)
	{
		app_fram_para_wait_lpm_commit();

		g_evt_ctrl.flash_commit = 0;
	}

    if(g_evt_ctrl.set_fram_default == 0xA5)
    {
        set_para_to_default_value();
        
        g_evt_ctrl.set_fram_default = 0;
    }

	feed_dog();

}

void app_enter_lpm_init(void)
{
	u16 mclock =0;

	u8  week_day = 0;

    u32 next_time = 0;

	feed_dog();
	
	system_ticks_statistics_enter_lpm_init();

	if(IS_VALID_BCD_TIM(g_evt_ctrl.next_tim))
	{
		
		mclock = ((u16)g_evt_ctrl.next_hour << 8)|(g_evt_ctrl.next_min);

		week_day = g_evt_ctrl.next_tim[6];

		memcpy(g_para.next_time,g_evt_ctrl.next_tim,7);

		//app_para_write_to_fram();

        #ifdef _DEBUG_
        print("\r\nclock(%d) event ctrl next time valid %x-%02x-%02x %02x:%02x:%02x\r\n",clock(),g_evt_ctrl.next_tim[0],g_evt_ctrl.next_tim[1],\
        g_evt_ctrl.next_tim[2],g_evt_ctrl.next_tim[3],g_evt_ctrl.next_tim[4],g_evt_ctrl.next_tim[5]);
        #endif
	
	}
	else
	{

        next_time = get_next_sleep_time_point(&week_day);

        if(next_time == 0)
        {
            /*追踪模式*/
            app_get_cur_time_value(&next_time);

            week_day = 0;

            next_time += 300ul; 

            next_time = LOCAL_TO_UTC_SEC(next_time);
        }


        if((week_day > 0)&&(week_day <= 7))
        {
             next_time = week_day_to_utc_time(week_day,next_time);

	         goome_utc_timer_sec_to_bcd_base2000(UTC_TO_LOCAL_SEC(next_time),g_para.next_time);
        }
        else
        {
             goome_utc_timer_sec_to_bcd_base2000(UTC_TO_LOCAL_SEC(next_time),g_para.next_time);
        }

        
        #ifdef _DEBUG_
        print("\r\nclock(%d) cannot get valid next tim %x-%02x-%02x %02x:%02x:%02x\r\n",clock(),g_para.next_time[0],g_para.next_time[1],\
        g_para.next_time[2],g_para.next_time[3],g_para.next_time[4],g_para.next_time[5]);
        #endif

        
        //app_para_write_to_fram();
    
		if(IS_VALID_BCD_TIM(g_para.next_time))
		{
			mclock = ((u16)g_para.next_time[3] << 8)|(g_para.next_time[4]);

			week_day = g_para.next_time[6];
		}
		else
		{
			mclock = 0x1230;
			
			week_day = 0;
		}
		
	}

	app_rtc_alarm_enter_lpm_init(mclock,week_day);
	
	memset((u8 *)&g_evt_ctrl.calibration_tim_flg,0,offsetof(system_event_control_struct,evt_hold_max_time) - offsetof(system_event_control_struct,calibration_tim_flg) + 1);

}

void app_exit_lpm_init(void)
{
	system_ticks_statistics_exit_lpm_init();

	app_rtc_alarm_exit_lpm_init();
}

void app_exit_lpm_l2_init(void)
{
	app_get_rtc_alarm_time(g_evt_ctrl.next_tim);
}

u8 is_cur_time_match_wkp_time(u8 wkp_flg)
{
	u8 cur_time[7] = {0};

    u8 next_tim[7] = {0};

	u32 cur_sec = 0 , wkp_sec = 0;

	u8 result = 1;

    u8 *p1 = NULL , *p2 = NULL;

	//app_get_cur_time_value(&time_rtc);
	app_get_ur_rtc_time(cur_time,NULL); 

    memcpy(g_para.wkp_time,cur_time,7);

    memcpy(next_tim,g_para.next_time,7);
 
    #ifdef _DEBUG_
    print("\r\nclock(%d) cur wakeup time %x-%02x-%02x %02x:%02x:%02x\r\n",clock(),cur_time[0],cur_time[1],\
    cur_time[2],cur_time[3],cur_time[4],cur_time[5]);
    #endif

    //app_get_rtc_alarm_time(next_tim);

    next_tim[5] = 0;

	cur_sec = datetosecond_base2000(cur_time);

	wkp_sec = datetosecond_base2000(next_tim);

    #ifdef _DEBUG_
    p1 = (u8 *)&cur_sec;
    p2 = (u8 *)&wkp_sec;
    print("clock(%d) cur_sec 0x%02x%02x%02x%02x wkp sec 0x%02x%02x%02x%02x",clock(),p1[0],p1[1],p1[2],p1[3],p2[0],p2[1],p2[2],p2[3]);
    #endif

    if(wkp_flg == EVENT_RTC_WKP)
    {
        if(cur_sec > (u32)615048204ul)
        {
            if(cur_sec < wkp_sec)
            {
                result = 0;
            }
        }
    }
    else
    {
        //result = 1;
        if(cur_sec > (u32)615048204ul)
        {
            if(cur_sec <= wkp_sec)
        	{
        		if((wkp_sec - cur_sec) >= 120)
        		{
                    /*RTC唤醒时间与当前的时间相比大于120s需要重新进入休眠*/
        			result = 0;
        		}
                //else RTC唤醒时间与当前时间相比小于120s需要唤醒MTK
        	}
            //else RTC唤醒时间落后与当前时间需要唤醒MTK  
        }
        //else 当前时间未校时 需要重新唤醒MTK
    }

	

	return result;
}

void exit_lpm_from_exti1_process(void)
{
    static u32 start_time_10ms = 0;
    static u32 cur_time_10ms = 0;
    #define LIGHT_ENTER_SENSOR 1
    u8 bitsta = 0;
    
    cur_time_10ms = get_system_ticks();

	if((start_time_10ms > cur_time_10ms)||(cur_time_10ms - start_time_10ms >= 10))
	{
        start_time_10ms = cur_time_10ms;

        feed_dog();

        bitsta = GPIO_ReadInputDataBit(LIGHT_SENSATION_BASE, LIGHT_SENSATION_PIN)>0?1:0;
        
		/* 与退出低功耗时的IO状态判断 */
		if(LIGHT_ENTER_SENSOR != bitsta)
		{
                MCU_WKP_STATISTICS(exit1_wkp);
        
				if(is_cur_time_match_wkp_time(EVENT_EXTI1_WKP))
				{                    
					set_wkp_reason(EVENT_EXTI1_WKP);
					
					send_event_to_main_hadle(EVENT_EXIT_LPM);
				}
				else
				{
                    send_event_to_main_hadle(EVENT_ENTER_LPM);
				}
		}
		else
		{
			if(is_event_hold_timeout())
			{
                MCU_WKP_STATISTICS(exit1_wkp);
                
				send_event_to_main_hadle(EVENT_EXIT_LPM);
				
				set_wkp_reason(EVENT_EXTI1_WKP);
			}
			
		}
	}

}

void exit_lpm_from_exti2_process(void)
{
    MCU_WKP_STATISTICS(exit2_wkp);
    
	set_wkp_reason(EVENT_EXTI2_WKP);

	send_event_to_main_hadle(EVENT_EXIT_LPM);

}

void exit_lpm_from_rtc_alarm_process(void)
{
	//set_wkp_reason(EVENT_RTC_WKP);
	MCU_WKP_STATISTICS(rtc_wkp);
	
	if(!is_cur_time_match_wkp_time(EVENT_RTC_WKP))
	{
        #ifdef _DEBUG_
        print("\r\nclock(%d) rtc wake up time not match!\r\n",clock());
        #endif
        
		send_event_to_main_hadle(EVENT_ENTER_LPM);
	}
	else
	{
        #ifdef _DEBUG_
        print("\r\nclock(%d) rtc wake up time is matched!\r\n",clock());
        #endif
        
		set_wkp_reason(EVENT_RTC_WKP);
		
		send_event_to_main_hadle(EVENT_EXIT_LPM);
	}
}

