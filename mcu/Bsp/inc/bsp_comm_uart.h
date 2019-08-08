/**
* Copyright (C), Goome Technologies Co., Ltd. 2009-2019. All rights reserved.
* File name : bsp_comm_uart.h
* Author:  Version:	Date:
* Description: bsp_comm_uart.c 的头文件
* Function List:
* History:
  1.Date: 2019/4/9
    Author: Chris.Lu
    Modification: Created file
*/
#ifndef __BSP_COMM_UART_H__
#define __BSP_COMM_UART_H__

#include "gm_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

extern void bsp_comm_uart_enter_lpm_init(void);
extern void bsp_comm_uart_exit_lpm_init(void);
extern void bsp_comm_uart_power_up_init(void);
extern void instert_comm_dat_to_queue(u8 byte);
extern void uart_send_byte(USART_TypeDef* USARTx,u8 byte);

enum
{
	REC_HEAD0,
	REC_HEAD1,
	REC_LENH,
	REC_LENL,
	REC_DAT,
	REC_TAIL
};

#define MAX_RECV_BUFF_CNT 2
#define SLAVE_SHARE_MEM_COMM_MAX_LEN 100
#define REC_SLAVE_SHARE_MEM_LEN 1

typedef struct
{	
	u8 data[SLAVE_SHARE_MEM_COMM_MAX_LEN];

	u8 datalen;
}Rec_Uart_Data_Struct;

typedef struct
{
	Rec_Uart_Data_Struct rec[MAX_RECV_BUFF_CNT];
	u8 rec_sta;
	u8 in;
	u8 out;
}Uart_Data_Queue_Struct;

typedef struct
{
    u8 data[REC_SLAVE_SHARE_MEM_LEN];

    u8 in;

    u8 out;
}RecDataCriStruct;

extern Rec_Uart_Data_Struct *get_comm_dat_from_uart(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __BSP_COMM_UART_H__ */
