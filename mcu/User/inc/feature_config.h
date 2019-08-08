/**
* Copyright (C), Goome Technologies Co., Ltd. 2009-2019. All rights reserved.
* File name : feature_config.h
* Author:  Version:	Date:
* Description: MCU chip define
* Function List:
* History:
  1.Date: 2019/6/6
    Author: Chris.Lu
    Modification: Created file
*/

#ifndef __FEATURE_CONFIG_H__
#define __FEATURE_CONFIG_H__

#include "stm8l15x_gpio.h"
#include "stm8l15x_exti.h"


#ifdef __STM8L151G6__

#define MODEL_POWER_CTRL_BASE GPIOB
#define MODEL_POWER_CTRL_PIN  GPIO_Pin_0

#define LIGHT_SENSATION_BASE  GPIOB
#define LIGHT_SENSATION_PIN   GPIO_Pin_3
#define LIGHT_SENSATION_EXIT  EXTI_Pin_3

#else

#define MODEL_POWER_CTRL_BASE GPIOB
#define MODEL_POWER_CTRL_PIN  GPIO_Pin_0

#define LIGHT_SENSATION_BASE  GPIOC
#define LIGHT_SENSATION_PIN   GPIO_Pin_7
#define LIGHT_SENSATION_EXIT  EXTI_Pin_7


#endif

#endif

