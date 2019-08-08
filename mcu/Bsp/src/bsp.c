/**
* Copyright (C), Goome Technologies Co., Ltd. 2009-2019. All rights reserved.
* File name : bsp.c
* Author:  Version:	Date:
* Description:板级支持
* Function List:
* History:
  1.Date: 2019/4/9
    Author: Chris.Lu
    Modification: Created file
*/
#include "bsp_system_clk.h"
#include "bsp_watch_dog.h"
#include "bsp_model_power_contrl.h"
#include "bsp_ticks_timer.h"
#include "bsp_light_sensor.h"
#include "bsp_comm_uart.h"
#include "bsp_rtc_alarm.h"
#include "bsp_gpio_manger.h"
#include "stm8l15x_pwr.h"
#include "bsp.h"
#include "time.h"
#include "debug_port.h"

NEAR u8 s_bsp_lpm_flg = 0;

NEAR u8 s_enter_ultra_lpm_flg = 0;

void bsp_power_up_init(void)
{
    watch_dog_power_up_init();
    
	bsp_sys_clk_power_up_init();

    bsp_model_power_power_up_init();
    
	disableInterrupts();
    
    bsp_light_sensor_power_up_init();

	bsp_gpio_manger_power_up_init();

	bsp_debug_uart_power_up_init();

	bsp_rtc_power_up_init();

	enableInterrupts();

    bsp_comm_uart_power_up_init();

	bsp_ticks_timer_power_up_init();

	feed_dog();

	s_bsp_lpm_flg = 0;

	

}


void bsp_enter_lpm_init(BSP_LPM_LEVEL_ENUM level)
{
	u8 i = 0;

	for(i = 0 ; i <= level; i++)
	{
		if((s_bsp_lpm_flg & (1 << i)) == 0)
		{
			s_bsp_lpm_flg |= (1<<i);
			
			switch(i)
			{
				case BSP_LPM_L1:
					{
						bsp_comm_uart_enter_lpm_init();
	
						bsp_model_power_enter_lpm_init();

						feed_dog();
						
						break;
					}
				case BSP_LPM_L2:
					{
						watch_dog_enter_lpm_init();

                        disableInterrupts();

						bsp_light_sensor_enter_lpm_init();

                        enableInterrupts();

						bsp_ticks_timer_enter_lpm_init();

						bsp_rtc_enter_lpm_init();

						delayms((u32)2000);
                        #ifdef __STM8L151C8__
                        bsp_debug_uart_enter_lpm_init();
                        #endif
                        bsp_gpio_manger_enter_lpm_init();
                        bsp_sys_clk_enter_lpm_init();

						break;
					}
				case BSP_LPM_L3:
					{
                        feed_dog();
						CLK_HaltConfig(CLK_Halt_FastWakeup, ENABLE);
						PWR_FastWakeUpCmd(ENABLE);
						PWR_UltraLowPowerCmd(ENABLE);
                        s_enter_ultra_lpm_flg = 1;
						//CLK_HSICmd(DISABLE);
						break;
					}
				default:
					break;
			}
		}
	}
	
}

void bsp_exit_lpm_init(BSP_LPM_LEVEL_ENUM level)
{
	u8 i = 0;

	for(i = level ; i <= BSP_LPM_L3; i++)
	{
		if(s_bsp_lpm_flg & (1 << i))
		{
			s_bsp_lpm_flg &= (~(1<<i));
			
			switch(i)
			{
				case BSP_LPM_L1:
					{
						bsp_model_power_exit_lpm_init();
						
						bsp_comm_uart_exit_lpm_init();

						feed_dog();
	
						s_bsp_lpm_flg = 0;
						
						break;
					}
				case BSP_LPM_L2:
					{
                        s_enter_ultra_lpm_flg = 0;
                        
                        disableInterrupts();
                        
                        bsp_light_sensor_exit_lpm_init();
                        
						bsp_sys_clk_exit_lpm_init();

						bsp_gpio_manger_exit_lpm_init();

						watch_dog_exit_lpm_init();

						bsp_ticks_timer_exit_lpm_init();

						bsp_rtc_exit_lpm_init();

                        enableInterrupts();

                        bsp_debug_uart_exit_lpm_init();

						feed_dog();

						break;
					}
				case BSP_LPM_L3:
					{
						
						break;
					}
				default:
					break;
			}
		}
	}
	
}

u8 get_bsp_lpm_state(void)
{
	return s_bsp_lpm_flg;
}

u8 does_system_in_ultra_lpm(void)
{
    return s_enter_ultra_lpm_flg;
}

