

#include "gm_type.h"
#include "stm8l15x_usart.h"
#include "stm8l15x_gpio.h"
#include "bsp_gpio_manger.h"
#include "bsp_comm_uart.h"
#include "bsp_watch_dog.h"
#include "feature_config.h"

#include <stdarg.h>
#include <stdio.h>



void bsp_debug_uart_power_up_init(void)
{

    CLK_PeripheralClockConfig(CLK_Peripheral_USART3,ENABLE);

	GPIO_Init(GPIOE,GPIO_Pin_7,GPIO_Mode_In_FL_No_IT); 
	
	// PE6:Tx  PE7:Rx
    /* Configure USART Tx as alternate function push-pull  (software pull up)*/
    GPIO_ExternalPullUpConfig(GPIOE, GPIO_Pin_6, ENABLE);
    /* Configure USART Rx as alternate function push-pull  (software pull up)*/
    GPIO_ExternalPullUpConfig(GPIOE, GPIO_Pin_7, ENABLE);

    USART_DeInit(USART3);
	
    USART_Init(USART3, (uint32_t)115200, USART_WordLength_8b, USART_StopBits_1, USART_Parity_No,  (USART_Mode_TypeDef)(USART_Mode_Tx));

    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
    
    USART_ClearITPendingBit(USART3,USART_IT_RXNE);
	
    //USART_ITConfig(USART1,USART_IT_RXNE, ENABLE);

    USART_ITConfig(USART3,USART_IT_TXE, DISABLE);
    
    USART_Cmd(USART3, ENABLE);
	
    bsp_gpio_manger_reject_defined_pin(GPIOE,(GPIO_Pin_TypeDef)(GPIO_Pin_6|GPIO_Pin_7));

	
}

void bsp_debug_uart_enter_lpm_init(void)
{
    USART_Cmd(USART2, DISABLE);

	USART_Cmd(USART3, DISABLE);

	CLK_PeripheralClockConfig(CLK_Peripheral_USART1,DISABLE);

	GPIO_Init(GPIOE, GPIO_Pin_6|GPIO_Pin_7, GPIO_Mode_Out_PP_Low_Slow);
	
    GPIO_SetBits(GPIOE, GPIO_Pin_6|GPIO_Pin_7);	

}

void bsp_debug_uart_exit_lpm_init(void)
{

    CLK_PeripheralClockConfig(CLK_Peripheral_USART3,ENABLE);

    GPIO_Init(GPIOE,GPIO_Pin_7,GPIO_Mode_In_FL_No_IT); 
    
    // PE6:Tx  PE7:Rx
    /* Configure USART Tx as alternate function push-pull  (software pull up)*/
    GPIO_ExternalPullUpConfig(GPIOE, GPIO_Pin_6, ENABLE);
    /* Configure USART Rx as alternate function push-pull  (software pull up)*/
    GPIO_ExternalPullUpConfig(GPIOE, GPIO_Pin_7, ENABLE);

    USART_DeInit(USART3);
    
    USART_Init(USART3, (uint32_t)115200, USART_WordLength_8b, USART_StopBits_1, USART_Parity_No,  (USART_Mode_TypeDef)(USART_Mode_Tx));

    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
    
    USART_ClearITPendingBit(USART3,USART_IT_RXNE);
    
    //USART_ITConfig(USART1,USART_IT_RXNE, ENABLE);

    USART_ITConfig(USART3,USART_IT_TXE, DISABLE);
    
    USART_Cmd(USART3, ENABLE);
    
    bsp_gpio_manger_reject_defined_pin(GPIOE,(GPIO_Pin_TypeDef)(GPIO_Pin_6|GPIO_Pin_7));

    
}

static u8 sprint_buf[200] = {0};

void print(const char* fmt ,...)
{
	// = 0;
	va_list args;
	u16 i = 0 ,len = 0;
    

    va_start(args, fmt);
    len = vsprintf((char *)sprint_buf, fmt, args);
    va_end(args);

    for(i = 0; i < len ; i++)
    {
        uart_send_byte(USART3,sprint_buf[i]);
    }

}



