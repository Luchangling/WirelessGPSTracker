/**
* Copyright (C), Goome Technologies Co., Ltd. 2009-2019. All rights reserved.
* File name : watch_dog.c
* Author:  Version:	Date:
* Description:watch dog
* Function List:
* History:
  1.Date: 2019/4/9
    Author: Chris.Lu
    Modification: Created file
*/
#include "stm8l15x_wwdg.h"
#include "bsp_watch_dog.h"

#define _ENABLE_SW_IWDG_
#define RELOAD_VALUE   254

void watch_dog_power_up_init(void)
{

#ifndef _ENABLE_SW_IWDG_
	WWDG_Init(0x7F,0x7D);
#else
    /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
 	IWDG_Enable(); 
  
    /* IWDG timeout equal to 214 ms (the timeout may varies due to LSI frequency dispersion) */
    /* Enable write access to IWDG_PR and IWDG_RLR registers */
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
  
  	/* IWDG configuration: IWDG is clocked by LSI = 38KHz */
  	IWDG_SetPrescaler(IWDG_Prescaler_256);
  
  	/* IWDG timeout equal to 214.7 ms (the timeout may varies due to LSI frequency dispersion) */
  	/* IWDG timeout = (RELOAD_VALUE + 1) * Prescaler / LSI 
                  = (254 + 1) * 32 / 38 000 
                  = 214.7 ms */
  	IWDG_SetReload((uint8_t)RELOAD_VALUE);
  
  	/* Reload IWDG counter */
  	IWDG_ReloadCounter();
#endif
}

void watch_dog_enter_lpm_init(void)
{
	feed_dog();
}

void watch_dog_exit_lpm_init(void)
{
	feed_dog();
}

void feed_dog(void)
{
#ifndef _ENABLE_SW_IWDG_
	unsigned char CounterValue = 0;

	CounterValue = (unsigned char)(WWDG_GetCounter()&0x7F);
    //IWDG_ReloadCounter();
	if(CounterValue < 0x7D)
	{
	 	WWDG_SetCounter(0x7F);  
	}
#else
	IWDG_ReloadCounter();
#endif
}


