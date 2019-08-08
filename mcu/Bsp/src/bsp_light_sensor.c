/**
* Copyright (C), Goome Technologies Co., Ltd. 2009-2019. All rights reserved.
* File name : bsp_light_sensor.c
* Author:  Version:	Date:
* Description:light sensor相关
* Function List:
* History:
  1.Date: 2019/4/9
    Author: Chris.Lu
    Modification: Created file
*/
#include "gm_type.h"
#include "stm8l15x_exti.h"
#include "bsp_gpio_manger.h"
#include "app_fram_manger.h"
#include "feature_config.h"
#ifdef _DEBUG_
#include "debug_port.h"
#include "system_ticks.h"
#endif

typedef struct
{
    u8 period;
    u8 timeout;
    u8 pre_state;
}Check_State_Change_Struct;

static Check_State_Change_Struct s_chk_sta;
static u8 s_pre_exti1_io_state = 0;
u8 static s_pre_enter_lpm_state = 0;


void bsp_light_sensor_power_up_init(void)
{
	bsp_gpio_manger_reject_defined_pin(LIGHT_SENSATION_BASE,(GPIO_Pin_TypeDef)(LIGHT_SENSATION_PIN));

	GPIO_Init(LIGHT_SENSATION_BASE,LIGHT_SENSATION_PIN,GPIO_Mode_In_FL_No_IT);//init no interrput

	EXTI_DeInit();

	EXTI_SetPinSensitivity(LIGHT_SENSATION_EXIT,EXTI_Trigger_Rising);//rasing trigger

    s_pre_enter_lpm_state = GPIO_ReadInputDataBit(LIGHT_SENSATION_BASE, LIGHT_SENSATION_PIN)>0?1:0;

    s_chk_sta.period = 10;

    s_chk_sta.timeout = 30;

    s_chk_sta.pre_state = s_pre_enter_lpm_state;
}

void bsp_light_sensor_enter_lpm_init(void)
{
    u8 ret = 1;

    //u8 io_state;
    
    //io_state = s_pre_enter_lpm_state;//GPIO_ReadInputDataBit(LIGHT_SENSATION_BASE, LIGHT_SENSATION_PIN) >0?1:0;

	switch(g_para.exti1_trigger)
	{
		case TRG_FALL_LOW:
            //if(io_state == 1)ret=1;
            break;
		case TRG_RISING:
            //if(io_state == 0)ret=1;
            break;
		case TRG_FALLING:
            //if(io_state == 1)ret=1;
		case TRG_RIS_FALL:
			//ret = 1;
			break;
			
		case TRG_NO_ONE:
		default:

            ret = 0;

			break;
			
	}


    if(ret == 1)
    {
        
        #ifdef _DEBUG_
        print("\r\nclock(%d) SET EXTI TRIGGER pre io state %d ,traigger %d\r\n",clock(),s_pre_enter_lpm_state,g_para.exti1_trigger);
        #endif
        
        EXTI_SetPinSensitivity(LIGHT_SENSATION_EXIT,(EXTI_Trigger_TypeDef)g_para.exti1_trigger);
			
		GPIO_Init(LIGHT_SENSATION_BASE, LIGHT_SENSATION_PIN, GPIO_Mode_In_FL_IT);
    }
    else
    {
        #ifdef _DEBUG_
        print("\r\nclock(%d) NO EXTI TRIGGER pre io state %d ,traigger %d\r\n",clock(),s_pre_enter_lpm_state,g_para.exti1_trigger);
        #endif
        
        GPIO_Init(LIGHT_SENSATION_BASE, (GPIO_Pin_TypeDef)(LIGHT_SENSATION_PIN), GPIO_Mode_In_FL_No_IT);
        
	    //GPIO_ResetBits(LIGHT_SENSATION_BASE, LIGHT_SENSATION_PIN);
    }

}

void bsp_light_sensor_exit_lpm_init(void)
{    
	GPIO_Init(LIGHT_SENSATION_BASE, LIGHT_SENSATION_PIN, GPIO_Mode_In_FL_No_IT);

	s_pre_exti1_io_state = GPIO_ReadInputDataBit(LIGHT_SENSATION_BASE, LIGHT_SENSATION_PIN)>0?1:0;

    s_chk_sta.period = 10;

    s_chk_sta.timeout = 30;

    //s_chk_sta.pre_state = s_pre_exti1_io_state;
}

void light_sensor_period_check_pin_10ms(void)
{
    if(s_chk_sta.period == 0)
    {
        s_chk_sta.period = 10;

        s_chk_sta.pre_state = GPIO_ReadInputDataBit(LIGHT_SENSATION_BASE, LIGHT_SENSATION_PIN)>0?1:0;

        if(s_chk_sta.pre_state !=  s_pre_enter_lpm_state)
        {
            s_chk_sta.timeout = 30;
        }
        else
        {
            s_chk_sta.timeout--;

            if(s_chk_sta.timeout == 0)
            {
                s_chk_sta.timeout = 30;
                
                s_pre_enter_lpm_state = s_chk_sta.pre_state;
            }
        }
    }

    s_chk_sta.period--;
}

u8 get_exit_lpm_exti1_io_state(void)
{
	return s_pre_exti1_io_state;
}

