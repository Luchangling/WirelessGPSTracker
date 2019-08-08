/**
* Copyright (C), Goome Technologies Co., Ltd. 2009-2019. All rights reserved.
* File name : bsp_comm_uart.c
* Author:  Version:	Date:
* Description:与Model通信串口相关
* Function List:
* History:
  1.Date: 2019/4/9
    Author: Chris.Lu
    Modification: Created file
*/
#include "stm8l15x_usart.h"
#include "stm8l15x_gpio.h"
#include "bsp_gpio_manger.h"
#include "bsp_comm_uart.h"
#include "bsp_watch_dog.h"
#include "debug_port.h"
#include "string.h"
#include "system_ticks.h"
Uart_Data_Queue_Struct s_uart_queue;
static RecDataCriStruct s_rec;
volatile u32 cur_time = 0;


void bsp_comm_uart_power_up_init(void)
{

    CLK_PeripheralClockConfig(CLK_Peripheral_USART1,ENABLE);

	GPIO_Init(GPIOC,GPIO_Pin_2,GPIO_Mode_In_FL_No_IT); 
	
	// PC3:Tx  PC2:Rx
    /* Configure USART Tx as alternate function push-pull  (software pull up)*/
    GPIO_ExternalPullUpConfig(GPIOC, GPIO_Pin_3, ENABLE);
    /* Configure USART Rx as alternate function push-pull  (software pull up)*/
    GPIO_ExternalPullUpConfig(GPIOC, GPIO_Pin_2, ENABLE);

    USART_DeInit(USART1);
	
    USART_Init(USART1, (uint32_t)115200ul, USART_WordLength_8b, USART_StopBits_1, USART_Parity_No,  (USART_Mode_TypeDef)(USART_Mode_Rx|USART_Mode_Tx));

    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    
    USART_ClearITPendingBit(USART1,USART_IT_RXNE);
	
    //USART_ITConfig(USART1,USART_IT_RXNE, ENABLE);

    USART_ITConfig(USART1,USART_IT_TXE, DISABLE);
    
    USART_Cmd(USART1, ENABLE);
	
    bsp_gpio_manger_reject_defined_pin(GPIOC,(GPIO_Pin_TypeDef)(GPIO_Pin_2|GPIO_Pin_3));

	memset((u8 *)&s_uart_queue,0,sizeof(Uart_Data_Queue_Struct));
    memset((u8 *)&s_rec,0,sizeof(RecDataCriStruct));
	
}

void bsp_comm_uart_enter_lpm_init(void)
{

	USART_Cmd(USART1, DISABLE);

	CLK_PeripheralClockConfig(CLK_Peripheral_USART1,DISABLE);

	GPIO_Init(GPIOC, GPIO_Pin_2|GPIO_Pin_3, GPIO_Mode_Out_OD_Low_Slow);
	
    GPIO_ResetBits(GPIOC, GPIO_Pin_2|GPIO_Pin_3);	

	memset((u8 *)&s_uart_queue,0,sizeof(Uart_Data_Queue_Struct));
    memset((u8 *)&s_rec,0,sizeof(RecDataCriStruct));
}

void bsp_comm_uart_exit_lpm_init(void)
{
    CLK_PeripheralClockConfig(CLK_Peripheral_USART1,ENABLE);

	GPIO_Init(GPIOC,GPIO_Pin_2,GPIO_Mode_In_FL_No_IT); 
	// PC3:Tx  PC2:Rx
    /* Configure USART Tx as alternate function push-pull  (software pull up)*/
    GPIO_ExternalPullUpConfig(GPIOC, GPIO_Pin_3, ENABLE);
    /* Configure USART Rx as alternate function push-pull  (software pull up)*/
    GPIO_ExternalPullUpConfig(GPIOC, GPIO_Pin_2, ENABLE);

    USART_DeInit(USART1);
	
    USART_Init(USART1, (u32)115200, USART_WordLength_8b, USART_StopBits_1, USART_Parity_No,  (USART_Mode_TypeDef)(USART_Mode_Rx|USART_Mode_Tx));

    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    
    USART_ClearITPendingBit(USART1,USART_IT_RXNE);
	
    //USART_ITConfig(USART1,USART_IT_RXNE, ENABLE);

    USART_ITConfig(USART1,USART_IT_TXE, DISABLE);
    
    USART_Cmd(USART1, ENABLE);

	bsp_gpio_manger_reject_defined_pin(GPIOC,(GPIO_Pin_TypeDef)(GPIO_Pin_2|GPIO_Pin_3));

	memset((u8 *)&s_uart_queue,0,sizeof(Uart_Data_Queue_Struct));
    memset((u8 *)&s_rec,0,sizeof(RecDataCriStruct));
}



Rec_Uart_Data_Struct *get_comm_dat_from_uart(void)
{
	Rec_Uart_Data_Struct *p =NULL;

	if(s_uart_queue.in != s_uart_queue.out)
	{
		p = &s_uart_queue.rec[s_uart_queue.out%MAX_RECV_BUFF_CNT];

		s_uart_queue.out++;

		s_uart_queue.out = s_uart_queue.out%MAX_RECV_BUFF_CNT;
	}

	return p;
}

void prase_smsp_data_to_que(void)
{
    #if 0
    Uart_Data_Queue_Struct *que = &s_uart_queue;
	
	Rec_Uart_Data_Struct *p = &que->rec[que->in%MAX_RECV_BUFF_CNT];

    u8 byte;

    static u16 len = 0;
    
    while(s_rec.in != s_rec.out)
    {
        byte = s_rec.data[s_rec.out];

        s_rec.out++;

        s_rec.out %= REC_SLAVE_SHARE_MEM_LEN;

        switch(que->rec_sta)
    	{
    		case REC_HEAD0:
    		{
    			if(byte == '$')que->rec_sta = REC_HEAD1;

                //que->start_time = cur_time;
                
    			break;
    		}
    		case REC_HEAD1:
    		{
    			if(byte == '$')
    			{
    				que->rec_sta = REC_LENH;
    			}
    			else
    			{
    				que->rec_sta = REC_HEAD0;
    			}
    			break;
    		}
    		case REC_LENH:
    		{
    			p->data[p->datalen++] = byte;
    			len = (u16)(byte << 8);
    			que->rec_sta = REC_LENL;
    			break;
    		}
    		case REC_LENL: 
    		{
    			p->data[p->datalen++] = byte;
    			len |= byte;
    			if(len + 3 < SLAVE_SHARE_MEM_COMM_MAX_LEN)
    			{
    				que->rec_sta = REC_DAT;
    			}
    			else
    			{
    				que->rec_sta = REC_HEAD0;
    			}
    			break;
    		}
    		case REC_DAT:
    		{
    			p->data[p->datalen++] = byte;
    			if(p->datalen  == len)
    			{
    				que->rec_sta = REC_TAIL;
    			}
                else if(p->datalen  > len)
                {
                    que->rec_sta = REC_HEAD0;
                }
                   
    			break;
    		}

    		case REC_TAIL:
    		{
    			if(byte == '@')
    			{
    				/*Complete*/
    				que->in++;
    				que->in = que->in%MAX_RECV_BUFF_CNT;
                    //que->start_time = cur_time;
                    return;
    			}
    			que->rec_sta = REC_HEAD0;
    			break;
    		}
    		default:
    			que->rec_sta = REC_HEAD0;
    			break;
    	}
    }
    #endif
}

#pragma optimize=none 
void instert_comm_dat_to_queue(u8 byte)
{   
    #if 0
    s_rec.data[s_rec.in] = byte;

    s_rec.in++;

    s_rec.in %= REC_SLAVE_SHARE_MEM_LEN;
    #else
    
    Uart_Data_Queue_Struct *que = &s_uart_queue;
	
	Rec_Uart_Data_Struct *p = &que->rec[que->in%MAX_RECV_BUFF_CNT];
    

	static u16 len = 0;

    cur_time = get_system_ticks();


    if(get_system_ticks() - cur_time  > 3)
    {
        que->rec_sta = REC_HEAD0;
    }

    cur_time = get_system_ticks();

	switch(que->rec_sta)
	{
		case REC_HEAD0:
		{
			if(byte == '$')que->rec_sta = REC_HEAD1;

            p->datalen = 0;
            
			break;
		}
		case REC_HEAD1:
		{
			if(byte == '$')
			{
				que->rec_sta = REC_LENH;

                p->datalen = 0;
			}
			else
			{
				que->rec_sta = REC_HEAD0;
			}
			break;
		}
		case REC_LENH:
		{
			p->data[p->datalen++] = byte;
			len = (u16)(byte << 8);
			que->rec_sta = REC_LENL;
			break;
		}
		case REC_LENL: 
		{
			p->data[p->datalen++] = byte;
			len |= byte;
			if(len + 3 < SLAVE_SHARE_MEM_COMM_MAX_LEN)
			{
				que->rec_sta = REC_DAT;
			}
			else
			{
				que->rec_sta = REC_HEAD0;
			}
			break;
		}
		case REC_DAT:
		{
			p->data[p->datalen++] = byte;
			if(p->datalen  == len)
			{
				que->rec_sta = REC_TAIL;
			}
            else if(p->datalen  > len)
            {
                que->rec_sta = REC_HEAD0;
            }
               
			break;
		}

		case REC_TAIL:
		{
			if(byte == '@')
			{
				/*Complete*/
				que->in++;
				que->in = que->in%MAX_RECV_BUFF_CNT;

			}
			que->rec_sta = REC_HEAD0;
			break;
		}
		default:
			que->rec_sta = REC_HEAD0;
			break;
	}
    #endif

}

void uart_send_byte(USART_TypeDef* USARTx,u8 byte)
{
    u16 timeout;
    
    timeout = 0x8000;
    /* Loop until the end of transmission */
    while (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET)
    {
        timeout--;
		
		feed_dog();

		if (timeout == 0)
        {
            break;
        }
    }
    
    /* Write a character to the USART */
    USART_SendData8(USARTx, byte);
}

