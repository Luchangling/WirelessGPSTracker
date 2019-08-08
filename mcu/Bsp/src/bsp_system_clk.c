/**
* Copyright (C), Goome Technologies Co., Ltd. 2009-2019. All rights reserved.
* File name : bsp_system_clkc
* Author:  Version:	Date:
* Description:BSP模块系统时钟相关
* Function List:
* History:
  1.Date: 2019/4/9
    Author: Chris.Lu
    Modification: Created file
*/
#include "stm8l15x_clk.h"

void bsp_sys_clk_power_up_init(void)
{
	CLK_HSICmd(ENABLE);
	CLK_SYSCLKSourceConfig(CLK_SYSCLKSource_HSI);
	CLK_SYSCLKDivConfig(CLK_SYSCLKDiv_1);

}

void bsp_sys_clk_enter_lpm_init(void)
{
	/*进入休眠时,所有外设的时钟关闭.PS:独立模块的时钟，由模块控制*/
	CLK_PeripheralClockConfig(CLK_Peripheral_TIM2, DISABLE);
	CLK_PeripheralClockConfig(CLK_Peripheral_TIM3, DISABLE);
	CLK_PeripheralClockConfig(CLK_Peripheral_I2C1, DISABLE);
	CLK_PeripheralClockConfig(CLK_Peripheral_SPI1, DISABLE);
	CLK_PeripheralClockConfig(CLK_Peripheral_BEEP, DISABLE);
	CLK_PeripheralClockConfig(CLK_Peripheral_DAC, DISABLE);
	CLK_PeripheralClockConfig(CLK_Peripheral_ADC1, DISABLE);
	CLK_PeripheralClockConfig(CLK_Peripheral_TIM1, DISABLE);

	CLK_PeripheralClockConfig(CLK_Peripheral_LCD, DISABLE);
	CLK_PeripheralClockConfig(CLK_Peripheral_DMA1, DISABLE);
	CLK_PeripheralClockConfig(CLK_Peripheral_COMP, DISABLE);
	CLK_PeripheralClockConfig(CLK_Peripheral_BOOTROM, DISABLE);
    CLK_PeripheralClockConfig(CLK_Peripheral_AES, DISABLE);
	CLK_PeripheralClockConfig(CLK_Peripheral_TIM5, DISABLE);
    CLK_PeripheralClockConfig(CLK_Peripheral_SPI2, DISABLE); 
	CLK_PeripheralClockConfig(CLK_Peripheral_USART2, DISABLE);
	CLK_PeripheralClockConfig(CLK_Peripheral_USART3, DISABLE);
    
    CLK_PeripheralClockConfig(CLK_Peripheral_CSSLSE, DISABLE);
	

}

void bsp_sys_clk_exit_lpm_init(void)
{
    CLK_HSICmd(ENABLE);
	CLK_SYSCLKSourceConfig(CLK_SYSCLKSource_HSI);
	CLK_SYSCLKDivConfig(CLK_SYSCLKDiv_1);
}
