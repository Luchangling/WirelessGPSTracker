/**
* Copyright (C), Goome Technologies Co., Ltd. 2009-2019. All rights reserved.
* File name : main.c
* Author:  Version:	Date:
* Description:stm8l151g6 project main file
			  1,MCU主循环
* Function List:
* History:
  1.Date: 2019/4/9
    Author: Chris.Lu
    Modification: Created file
*/

#include "bsp.h"
#include "app.h"
#include "main.h"
#include "string.h"
#include "system_ticks.h"
#include "app_rtc_alarm.h"
#include "app_fram_manger.h"
#include "debug_port.h"

void main()
{
	send_event_to_main_hadle(EVENT_POWER_UP);

	while(1)
	{
		switch(get_system_event())
		{
			case EVENT_POWER_UP:

				/*BSP Init*/
				bsp_power_up_init();
				/*App Init*/
				app_power_up_init();

				send_event_to_main_hadle(EVENT_WAIT_LPM);

				set_wkp_reason(EVENT_POWER_UP);

                #ifdef _DEBUG_
                print("\r\nclock(%x) power up!\r\n",clock());
                #endif
                
				break;
			case EVENT_WAIT_LPM:

				app_wait_lpm_process();
				
				if(is_need_change_event_to_lpm())
				{
					send_event_to_main_hadle(EVENT_PREPARE_ENTER_LPM);
				}
				
				break;
			case EVENT_PREPARE_ENTER_LPM:

				app_enter_lpm_init();

                send_event_to_main_hadle(EVENT_ENTER_LPM);
				
				break;
			case EVENT_ENTER_LPM:
            
                #ifdef _DEBUG_
                print("\r\nclock(%d) system enter LPM!",clock());
                #endif

                app_para_write_to_fram();

				bsp_enter_lpm_init(BSP_LPM_L3);

				halt();

				bsp_exit_lpm_init(BSP_LPM_L2);

				app_fram_para_exit_lpm_init();

                #ifdef _DEBUG_
                print("\r\nclock(%d) System wake up!..\r\n",clock());
				#endif
                
				break;
				
			case EVENT_EXTI1_WKP:
				
				exit_lpm_from_exti1_process();

				break;
			case EVENT_EXTI2_WKP:

				exit_lpm_from_exti2_process();
				
				break;
			case EVENT_RTC_WKP:
				
				exit_lpm_from_rtc_alarm_process();
				
				break;
			case EVENT_EXIT_LPM:

				bsp_exit_lpm_init(BSP_LPM_L0);

				app_exit_lpm_init();

				send_event_to_main_hadle(EVENT_WAIT_LPM);

				break;
			default:
				/*异常事件，等待看门欧复位*/
				break;
		}

	}
}
