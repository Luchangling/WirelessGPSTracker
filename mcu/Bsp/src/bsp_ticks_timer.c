/**
* Copyright (C), Goome Technologies Co., Ltd. 2009-2019. All rights reserved.
* File name : bsp_ticks_timer.c
* Author:  Version:	Date:
* Description:计时定时器
* Function List:
* History:
  1.Date: 2019/4/9
    Author: Chris.Lu
    Modification: Created file
*/

#include "stm8l15x_tim4.h"

void bsp_ticks_timer_power_up_init(void)
{
	CLK_PeripheralClockConfig(CLK_Peripheral_TIM4, ENABLE);   
  
    TIM4_TimeBaseInit(TIM4_Prescaler_1024, 156);  // 32:500us  128:1ms  1024:10ms  
    
    /* Clear TIM4 update flag */
    TIM4_ClearFlag(TIM4_FLAG_Update);
    /* Enable update interrupt */
    TIM4_ITConfig(TIM4_IT_Update, ENABLE);
    /* Enable TIM4 */
    TIM4_Cmd(ENABLE); 
}

void bsp_ticks_timer_enter_lpm_init(void)
{
	CLK_PeripheralClockConfig(CLK_Peripheral_TIM4, DISABLE);  

	TIM4_Cmd(DISABLE);
}

void bsp_ticks_timer_exit_lpm_init(void)
{
	CLK_PeripheralClockConfig(CLK_Peripheral_TIM4, ENABLE);   
  
    TIM4_TimeBaseInit(TIM4_Prescaler_1024, 156);  // 32:500us  128:1ms  1024:10ms  
    
    /* Clear TIM4 update flag */
    TIM4_ClearFlag(TIM4_FLAG_Update);
    /* Enable update interrupt */
    TIM4_ITConfig(TIM4_IT_Update, ENABLE);
    /* Enable TIM4 */
    TIM4_Cmd(ENABLE); 
}

