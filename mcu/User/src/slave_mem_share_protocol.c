/**
* Copyright (C), Goome Technologies Co., Ltd. 2009-2019. All rights reserved.
* File name : slave_mem_share_protocol.c
* Author:  Version:	Date:
* Description:从机内存共享协议
* Function List:
* History:
  1.Date: 2019/4/15
    Author: Chris.Lu
    Modification: Created file
*/


#include "app.h"
#include "gm_type.h"
#include "bsp_comm_uart.h"
#include "stm8l15x_gpio.h"
#include "app_fram_manger.h"
#include "string.h"
#include "system_ticks.h"
#include "debug_port.h"
#define MAX_SEND_BUF_LEN 200

char const g_sw_version[] = {"GW.C8.SMSP1.V1.0.6"};

char const g_build_date[] = {__DATE__};

char const g_build_time[] = {__TIME__};

u8 s_send_buf[MAX_SEND_BUF_LEN] = {0};

enum
{
	RD_PACKET,  
	WR_PACKET,  
	QE_PACKET, 
};

//#pragma pack(1)
typedef struct
{
	u32 addr;

	u16 len;
	
}Slave_Mem_Block_struct;

typedef struct
{
	u16 len;

	u8  mark;

	u8  type;

	u8  block[1];
	
}Smsp_Header_Struct; /*SMSP mean Slave Memory Share Protocol*/


typedef struct
{
	u16 len;

    //u16 addrh;

	u32 addr;

	u8  data[1];
	
}Smsp_Block_Struct;
//#pragma pack()

u8 check_arry_data_xor(u8 *data , u16 len)
{
	u16 i = 0;
	
	u8 xor = 0;

	for(i = 0 ; i < len ; i++)
	{
		xor ^= data[i];
	}

	return xor;
}


typedef enum
{
	EXPMU_EVT_REG,

	EXPMU_VER_REG,

	EXPMU_DATE_REG,

	EXPMU_TIME_REG,

	EXPMU_PARA_REG,

	EXPMU_GPIOA_REG,

	EXPMU_GPIOB_REG,

	EXPMU_GPIOC_REG,

	EXPMU_GPIOD_REG,

	EXPMU_END_REG
	
}EXPMU_Support_Reg_Enum;

Slave_Mem_Block_struct g_slave_mem_block[EXPMU_END_REG] = 
{
	{
		(u32)&g_evt_ctrl,
		sizeof(system_event_control_struct)
	},
	{
		(u32)g_sw_version,
		(sizeof(g_sw_version) - 1)
	},
	{
		(u32)g_build_date,
		(sizeof(g_build_date) - 1)
	},
	{
		(u32)g_build_time,
		(sizeof(g_build_time) - 1)
	},
	{
		(u32)&g_para,
		(sizeof(g_para) - 1)
	},
	{
		(u32)0x5000,  /*GPIOA*/
		sizeof(GPIO_TypeDef)
	},
	{
		(u32)0x5005,  /*GPIOB*/
		sizeof(GPIO_TypeDef)
	},
	{
		(u32)0x500A,  /*GPIOC*/
		sizeof(GPIO_TypeDef)
	},
	{
		(u32)0x500F,  /*GPIOD*/
		sizeof(GPIO_TypeDef)
	}
};

u8 match_the_register_addr(u32 addr)
{
    u8 i = 0;


    for(i = 0; i < EXPMU_END_REG ; i++)
    {
        if((g_slave_mem_block[i].addr <= addr) && ((g_slave_mem_block[i].addr + g_slave_mem_block[i].len) > addr))
        {
            return i;
        }
    }


    return EXPMU_END_REG;
}


u8 check_the_same_register_addr(u32 addr)
{
    u8 i = 0;

    for(i = 0; i < EXPMU_END_REG ; i++)
    {
        if(g_slave_mem_block[i].addr == addr)
        {
            return i;
        }
    }

    return EXPMU_END_REG;
}

u16 prase_read_packet_block(u8 *block , u8 mark , u16 max_len)
{
	Smsp_Block_Struct *bl = NULL;

	u16 i = 0 , j = 0;

	u8 k,xor = 0;

	s_send_buf[j++] = 0;

	s_send_buf[j++] = 0;

	s_send_buf[j++] = mark;

	s_send_buf[j++] = RD_PACKET;

	while(i < max_len)
	{
		bl = (Smsp_Block_Struct *)&block[i];

        k = check_the_same_register_addr(bl->addr);

        if( k < EXPMU_END_REG)
        {
            if((bl->addr)&&(bl->len == 7))
    		{
    			s_send_buf[j++] = ((u16)(bl->data[0] + 6) >> 8)&0x00ff;

    			s_send_buf[j++] = ((u16)(bl->data[0] + 6))&0x00ff;

    			s_send_buf[j++] = (bl->addr >> 24)&0xff;

    			s_send_buf[j++] = (bl->addr >> 16)&0xff;

    			s_send_buf[j++] = (bl->addr >>  8)&0xff;

    			s_send_buf[j++] = (bl->addr      )&0xff;

    			memcpy(&s_send_buf[j] ,(u8 *)bl->addr,bl->data[0]);

    			j += bl->data[0];
          	}
            else
            {
              #ifdef _DEBUG_
              print("\r\nclock(%d) read packet reg invalid!\r\n",clock());
              #endif
              break;
            }
        }
        else
        {
          #ifdef _DEBUG_
          print("\r\nclock(%d) read packet reg invalid,addr %x mark %x\r\n",clock(),(u16)bl->addr,mark);
          #endif
          break;
        }
        i += bl->len;
        break;
	}

	s_send_buf[0] = ((j+1) >> 8)&0xff;

	s_send_buf[1] = (j+1       )&0xff;

	xor = check_arry_data_xor(s_send_buf,j);

	s_send_buf[j++] = xor;

	return j;
}


u16 prase_write_pracket_block(u8 *block , u8 mark , u16 max_len)
{
	Smsp_Block_Struct *bl = NULL;

	u16 i = 0 , j = 0 , k = 0;

	u8 xor = 0;

	s_send_buf[j++] = 0;

	s_send_buf[j++] = 0;

	s_send_buf[j++] = mark;

	s_send_buf[j++] = WR_PACKET;

	while(i < max_len)
	{
		bl = (Smsp_Block_Struct *)&block[i];
		
		if((bl->addr)&&(bl->len > 6))
		{   
            
            #ifdef _DEBUG_
            print("clock(%d) addr %04x\r\n",(u16)bl->addr);
            #endif
            
            k = match_the_register_addr(bl->addr);

            if(k < EXPMU_END_REG)
            {
                if((bl->addr + bl->len -6) < (g_slave_mem_block[k].addr + g_slave_mem_block[k].len))
                {
                    memcpy((u8 *)bl->addr,bl->data,(bl->len - 6));
                }
                else
                {
                    #ifdef _DEBUG_
                    print("clock(%d) smsp write addr oversize!!!",clock());
                    #endif
                    break;
                }
                
                s_send_buf[j++] = ((u16)(g_slave_mem_block[k].len + 6) >> 8)&0x00ff;

    			s_send_buf[j++] = ((u16)(g_slave_mem_block[k].len + 6))&0x00ff;

    			s_send_buf[j++] = (g_slave_mem_block[k].addr >> 24)&0xff;

    			s_send_buf[j++] = (g_slave_mem_block[k].addr >> 16)&0xff;

    			s_send_buf[j++] = (g_slave_mem_block[k].addr >>  8)&0xff;

    			s_send_buf[j++] = (g_slave_mem_block[k].addr      )&0xff;

    			memcpy(&s_send_buf[j] ,(u8 *)g_slave_mem_block[k].addr,g_slave_mem_block[k].len);

    			j += g_slave_mem_block[k].len;

                break;
            }
            else
            {
                #ifdef _DEBUG_
                print("clock(%d) smsp write packet fail!",clock());
                #endif
                
                s_send_buf[j++] = 0;

    			s_send_buf[j++] = 6;

    			s_send_buf[j++] = (bl->addr >> 24)&0xff;

    			s_send_buf[j++] = (bl->addr >> 16)&0xff;

    			s_send_buf[j++] = (bl->addr >>  8)&0xff;

    			s_send_buf[j++] = (bl->addr      )&0xff;

            }

			
		}
        else
        {
          break;
        }
        
        i += bl->len;
	}

	s_send_buf[0] = ((j+1) >> 8)&0xff;

	s_send_buf[1] = (j+1       )&0xff;

	xor = check_arry_data_xor(s_send_buf,j);

	s_send_buf[j++] = xor;

	return j;
}


u16 install_query_packet(u8 mark)
{
	u16 i,j = 0;

	u8 xor = 0;

	
	s_send_buf[j++] = 0;

	s_send_buf[j++] = 0;

	s_send_buf[j++] = mark;

	s_send_buf[j++] = QE_PACKET;

	for(i = 0; i < EXPMU_END_REG; i++)
	{
		s_send_buf[j++] = 0;

		s_send_buf[j++] = 8;

		s_send_buf[j++] = (g_slave_mem_block[i].addr >> 24)&0xff;

		s_send_buf[j++] = (g_slave_mem_block[i].addr >> 16)&0xff;

		s_send_buf[j++] = (g_slave_mem_block[i].addr >>  8)&0xff;

		s_send_buf[j++] = (g_slave_mem_block[i].addr      )&0xff;

		s_send_buf[j++] = (g_slave_mem_block[i].len  >>  8)&0xff;

		s_send_buf[j++] = (g_slave_mem_block[i].len       )&0xff;
	}

	s_send_buf[0] = ((j+1) >> 8)&0xff;

	s_send_buf[1] = (j+1       )&0xff;

	xor = check_arry_data_xor(s_send_buf,j);

	s_send_buf[j++] = xor;
        
    return j;
	
}

void send_smsp_packet_to_uart(u8 *pdata , u16 len)
{
	u16  j = 0;
	
	if(len > 0)
	{
		uart_send_byte(USART1,'$');
		
		uart_send_byte(USART1,'$');

		for(j = 0 ; j < len ; j++)
		{
			uart_send_byte(USART1,pdata[j]);
		}

		uart_send_byte(USART1,'@');
		
	}
}

u8 slave_mem_share_protocol_prase(void)
{
	Rec_Uart_Data_Struct *rec = NULL;

	Smsp_Header_Struct *head = NULL;

	u16 len = 0;
	
	rec = get_comm_dat_from_uart();

	if(rec != NULL)
	{
		head = (Smsp_Header_Struct *)rec->data;
		
		if(check_arry_data_xor(rec->data,rec->datalen) == 0) /*Xor correct!*/
		{
            #ifdef _DEBUG_
            print("\r\nclock(%d) get packet %02x%02x %02x %02x %02x%02x %02x%02x%02x%02x\r\n",clock(),rec->data[0],rec->data[1],rec->data[2],\
                rec->data[3],rec->data[4],rec->data[5],rec->data[6],rec->data[7],rec->data[8],rec->data[9]);
            #endif
			switch(head->type)
			{
				case RD_PACKET:
				{
					len  = prase_read_packet_block(head->block,head->mark,rec->datalen - 4);
					break;
				}
				case WR_PACKET:
				{
					len  = prase_write_pracket_block(head->block,head->mark,rec->datalen - 4);
					break;
				}
				case QE_PACKET:
				{
					len = install_query_packet(head->mark);
					break;
				}
				default:
					break;
			}
			
			send_smsp_packet_to_uart(s_send_buf,len);
			
		}

		rec->datalen = 0;
	}
        
        return 0;
	
}

