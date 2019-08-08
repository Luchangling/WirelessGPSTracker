/**
* Copyright (C), Goome Technologies Co., Ltd. 2009-2019. All rights reserved.
* File name : bsp_gpio_manger.h
* Author:  Version:	Date:
* Description: bsp_gpio_manger.c 的头文件
* Function List:
* History:
  1.Date: 2019/4/11
    Author: Chris.Lu
    Modification: Created file
*/
#ifndef __BSP_GPIO_MANGER_H__
#define __BSP_GPIO_MANGER_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#include "gm_type.h"

enum
{
	TRG_FALL_LOW = EXTI_Trigger_Falling_Low,
	TRG_RISING   = EXTI_Trigger_Rising,
	TRG_FALLING  = EXTI_Trigger_Falling,
	TRG_RIS_FALL = EXTI_Trigger_Rising_Falling,
	TRG_NO_ONE
};

extern void bsp_gpio_manger_enter_lpm_init(void);
extern void bsp_gpio_manger_exit_lpm_init(void);
extern void bsp_gpio_manger_power_up_init(void);
extern GM_ERRCODE bsp_gpio_manger_register_undefined_pin(GPIO_TypeDef* gpiox , GPIO_Pin_TypeDef pin);
extern GM_ERRCODE bsp_gpio_manger_reject_defined_pin(GPIO_TypeDef* gpiox , GPIO_Pin_TypeDef pin);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __BSP_GPIO_MANGER_H__ */
