/**
* Copyright (C), Goome Technologies Co., Ltd. 2009-2019. All rights reserved.
* File name : bsp_io_manger.c
* Author:  Version:	Date:
* Description:gpio管理
* Function List:
* History:
  1.Date: 2019/4/11
    Author: Chris.Lu
    Modification: Created file
*/

#include "stm8l15x_gpio.h"
#include "feature_config.h"
#include "stm8l15x.h"
#include "gm_type.h"

typedef struct
{
	GPIO_TypeDef* gpio_base;

	GPIO_Pin_TypeDef undefine_pin;
	
}Undefine_Pin_Struct;

#ifdef __STM8L151G6__
static Undefine_Pin_Struct s_undefine_map[] = 
{
	{
		GPIOA,
		(GPIO_Pin_TypeDef)(GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5)
	},

	{
		GPIOB,
		GPIO_Pin_All
	},

	{
		GPIOC,
		(GPIO_Pin_TypeDef)(GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6)
	},

	{
		GPIOD,
		(GPIO_Pin_TypeDef)(GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4)
	},

	{
		NULL,
		GPIO_Pin_All
	}
};
#else
static Undefine_Pin_Struct s_undefine_map[] = 
{
	{
		GPIOA,
		GPIO_Pin_All
	},

	{
		GPIOB,
		GPIO_Pin_All
	},

	{
		GPIOC,
		GPIO_Pin_All
	},

	{
		GPIOD,
		GPIO_Pin_All
	},

    {
		GPIOE,
	    GPIO_Pin_All
	},

    {
        GPIOF,
        GPIO_Pin_0
    },

	{
		NULL,
		GPIO_Pin_All
	}
};

#endif

GM_ERRCODE bsp_gpio_manger_reject_defined_pin(GPIO_TypeDef* gpiox , GPIO_Pin_TypeDef pin)
{
	u8 i = 0;

	u8 pre_pin = 0;

	if(gpiox == NULL) return GM_PARAM_ERROR;

	for(i = 0;s_undefine_map[i].gpio_base != NULL; i++)
	{
		if(gpiox == s_undefine_map[i].gpio_base)
		{
			break;
		}
	}

	if(s_undefine_map[i].gpio_base == NULL) return GM_PARAM_ERROR;

	pre_pin = s_undefine_map[i].undefine_pin;

	pre_pin &= (~(u8)pin);

	s_undefine_map[i].undefine_pin = (GPIO_Pin_TypeDef)pre_pin;

	return GM_SUCCESS;
}


GM_ERRCODE bsp_gpio_manger_register_undefined_pin(GPIO_TypeDef* gpiox , GPIO_Pin_TypeDef pin)
{
	u8 i = 0;

	u8 pre_pin = 0;


	if(gpiox == NULL) return GM_PARAM_ERROR;

	for(i = 0;s_undefine_map[i].gpio_base != NULL ; i++)
	{
		if(gpiox == s_undefine_map[i].gpio_base)
		{
			break;
		}
	}

	if(s_undefine_map[i].gpio_base == NULL) return GM_PARAM_ERROR;

	pre_pin = s_undefine_map[i].undefine_pin;

	pre_pin |= (u8)pin;

	s_undefine_map[i].undefine_pin = (GPIO_Pin_TypeDef)pre_pin;

	return GM_SUCCESS;
}

void bsp_gpio_manger_power_up_init(void)
{
	/*rtc osc pin*/
	bsp_gpio_manger_reject_defined_pin(GPIOC,(GPIO_Pin_TypeDef)(GPIO_Pin_5|GPIO_Pin_6));
    /*SWIM port*/
	bsp_gpio_manger_reject_defined_pin(GPIOA,(GPIO_Pin_TypeDef)(GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3));

}

void bsp_gpio_manger_enter_lpm_init(void)
{
	u8 i = 0;

	for(i = 0 ; s_undefine_map[i].gpio_base != NULL ; i++)
	{
		GPIO_Init(s_undefine_map[i].gpio_base,(GPIO_Pin_TypeDef)s_undefine_map[i].undefine_pin,GPIO_Mode_In_PU_No_IT);
	}

    /*此处的IO处理是为了解决贴了GSENSOR的设备功耗过高的问题*/
    GPIO_Init(GPIOD, GPIO_Pin_5, GPIO_Mode_Out_PP_High_Slow);
    
	GPIO_WriteBit(GPIOD, GPIO_Pin_5, SET);
}

void bsp_gpio_manger_exit_lpm_init(void)
{
	
}

