/**
* Copyright (C), Goome Technologies Co., Ltd. 2009-2019. All rights reserved.
* File name : bsp_model_power_contrl.c
* Author:  Version:	Date:
* Description: 
* Function List:
* History:
  1.Date: 2019/4/9
    Author: Chris.Lu
    Modification: Created file
*/

#include "gm_type.h"
#include "time.h"
#include "stm8l15x_gpio.h"
#include "bsp_gpio_manger.h"
#include "feature_config.h"

void bsp_model_prapre_power_power_up(void)
{
	GPIO_Init(MODEL_POWER_CTRL_BASE, MODEL_POWER_CTRL_PIN,GPIO_Mode_Out_PP_Low_Slow); 

	GPIO_ResetBits(MODEL_POWER_CTRL_BASE, MODEL_POWER_CTRL_PIN);

}


void bsp_model_power_power_up_init(void)
{

    GPIO_Init(MODEL_POWER_CTRL_BASE, MODEL_POWER_CTRL_PIN,GPIO_Mode_Out_PP_Low_Slow); 

	GPIO_SetBits(MODEL_POWER_CTRL_BASE, MODEL_POWER_CTRL_PIN);

	bsp_gpio_manger_reject_defined_pin(MODEL_POWER_CTRL_BASE, MODEL_POWER_CTRL_PIN);
}

void bsp_model_restart(void)
{
    GPIO_ResetBits(MODEL_POWER_CTRL_BASE, MODEL_POWER_CTRL_PIN);

    delayms(1000);

    GPIO_SetBits(MODEL_POWER_CTRL_BASE, MODEL_POWER_CTRL_PIN);
}

void bsp_model_power_enter_lpm_init(void)
{
    GPIO_Init(MODEL_POWER_CTRL_BASE, MODEL_POWER_CTRL_PIN,GPIO_Mode_Out_OD_Low_Slow); 
    
	GPIO_ResetBits(MODEL_POWER_CTRL_BASE, MODEL_POWER_CTRL_PIN);
}

void bsp_model_power_exit_lpm_init(void)
{
     GPIO_Init(MODEL_POWER_CTRL_BASE, MODEL_POWER_CTRL_PIN,GPIO_Mode_Out_PP_Low_Slow); 
    
	GPIO_SetBits(MODEL_POWER_CTRL_BASE, MODEL_POWER_CTRL_PIN);
}
