/**
* Copyright (C), Goome Technologies Co., Ltd. 2009-2019. All rights reserved.
* File name : bsp_fram_manger.c
* Author:  Version:	Date:
* Description: FRAM管理
* Function List:
* History:
  1.Date: 2019/4/19
    Author: Chris.Lu
    Modification: Created file
*/

#include "gm_type.h"
#include "string.h"
#include "bsp_watch_dog.h"
#include "bsp_gpio_manger.h"
#include "app_fram_manger.h"
#include "app_calc_next_wkp_time.h"
#include "stm8l15x_flash.h"

Fram_Para_Struct g_para;

Fram_Para_Struct g_para_bkp;

#define WAIT_ACT_TIMEOUT 0xFFFD

u8 app_fram_write(u16 offset, const u8* pdata, u16 len)
{
    u16 idx;
	u16 timeout = 0;
    
    if (pdata == NULL)
    {
        return 0xFF;
    }

    //disableInterrupts();
    
    /* Unlock flash data eeprom memory */
    FLASH_Unlock(FLASH_MemType_Data);
	
    /* Wait until Data EEPROM area unlocked flag is set*/
    while (FLASH_GetFlagStatus(FLASH_FLAG_DUL) == RESET)
    {
		timeout ++;

		if(timeout >= WAIT_ACT_TIMEOUT)break;
		feed_dog();
	}
    for(idx = 0; idx < len; idx++)
    {
        if ((offset+idx) > 4096)
        {
            FLASH_Lock(FLASH_MemType_Data);

            //enableInterrupts();
            
            return idx+1;
        }
        
        FLASH_ProgramByte(FLASH_DATA_EEPROM_START_PHYSICAL_ADDRESS
           + offset + idx, *(pdata + idx));

		timeout = 0;
		
        while (FLASH_GetFlagStatus(FLASH_FLAG_EOP) == RESET)
        {
			timeout ++;

			 /* Wait until Data EEPROM area end of pragramming */
			if(timeout >= WAIT_ACT_TIMEOUT)break;
            feed_dog();
        }
    }
    FLASH_Lock(FLASH_MemType_Data);

	timeout = 0;

    //enableInterrupts();
    
    while (FLASH_GetFlagStatus(FLASH_FLAG_DUL) == SET)
    {
		timeout ++;

		if(timeout >= WAIT_ACT_TIMEOUT)break;
		
		feed_dog();
	}
    
    return 0x00;
}

u8 app_fram_read(u16 offset, u8* pdata, u16 len)
{
    u16 idx;
    
    if (pdata == NULL)
    {
        return 0xFF;
    }
    
    for(idx = 0; idx < len; idx++)
    {
        if ((offset+idx) > 4096)
        {
            return idx+1;
        }        
        pdata[idx] = FLASH_ReadByte(FLASH_DATA_EEPROM_START_PHYSICAL_ADDRESS + offset + idx);
    }    
    return 0x00;
}

void app_para_write_to_fram(void)
{
    if(g_para.factory_mark != MAGIC_NUM)
    {
        set_para_to_default_value();
    }
    else
    {
        g_para.sum = get_para_sum_vlue(&g_para);
	
	    memcpy((u8 *)&g_para_bkp,(u8 *)&g_para,sizeof(Fram_Para_Struct));

	    app_fram_write(USER_PARA_START_ADDR,(u8 *)&g_para,sizeof(Fram_Para_Struct));

	    app_fram_write(USER_BKP_SATRT_ADDR,(u8 *)&g_para_bkp,sizeof(Fram_Para_Struct));
    }

}

u8 get_para_sum_vlue(Fram_Para_Struct *para)
{
	u16 i = 0;
	
	u8 *data = (u8 *)para;

	u8 sum = 0;

	for(i = 0 ; i < (sizeof(Fram_Para_Struct)- 1); i++)
	{
		sum += data[i];
	}

	return sum;
}

u8 check_para_file_is_vaild(Fram_Para_Struct *para)
{
	
	if(para->factory_mark == MAGIC_NUM)
	{
		if(para->sum ==  get_para_sum_vlue(para))
		{
			return 1;
		}
	}

	return 0;
}

void set_para_to_default_value(void)
{
    ClockModeTimePointStruct *clock = NULL;
    
	memset((u8 *)&g_para    ,0,sizeof(Fram_Para_Struct));

	memset((u8 *)&g_para_bkp,0,sizeof(Fram_Para_Struct));

	g_para.factory_mark = MAGIC_NUM;

	g_para.exti1_trigger = TRG_RISING;

	g_para.next_year = 0x19;

	g_para.next_mon  = 0x04;

	g_para.next_day  = 0x22;
	
	g_para.next_hour = 0x12;

	g_para.next_min  = 0x30;

	g_para.next_week_day = 0;

    g_para.zone = 8;

    g_para.workmode = ALARM_CLOCK_MODE;

    clock = (ClockModeTimePointStruct *)g_para.mod_param;

    memset(g_para.mod_param,0xFF,16);

    clock->time_point[0] = 1230;

	g_para.exti1_trigger = TRG_RISING;

	g_para.exti2_trigger = TRG_NO_ONE;

	g_para.sum = get_para_sum_vlue(&g_para);

	memcpy((u8 *)&g_para_bkp,(u8 *)&g_para,sizeof(Fram_Para_Struct));

	app_fram_write(USER_PARA_START_ADDR,(u8 *)&g_para,sizeof(Fram_Para_Struct));

	app_fram_write(USER_BKP_SATRT_ADDR,(u8 *)&g_para_bkp,sizeof(Fram_Para_Struct));

}

void app_fram_para_powerup_init(void)
{
	memset((u8 *)&g_para    ,0,sizeof(Fram_Para_Struct));

	memset((u8 *)&g_para_bkp,0,sizeof(Fram_Para_Struct));

	FLASH_SetProgrammingTime((FLASH_ProgramTime_TypeDef)FLASH_ProgramMode_Standard);
	
	app_fram_read(USER_PARA_START_ADDR,(u8 *)&g_para,sizeof(Fram_Para_Struct));

	app_fram_read(USER_BKP_SATRT_ADDR,(u8 *)&g_para_bkp,sizeof(Fram_Para_Struct));

	if(!check_para_file_is_vaild(&g_para))
	{
		if(check_para_file_is_vaild(&g_para_bkp))
		{
			memcpy((u8 *)&g_para,(u8 *)&g_para_bkp,sizeof(Fram_Para_Struct));

			app_fram_write(USER_PARA_START_ADDR,(u8 *)&g_para,sizeof(Fram_Para_Struct));

			app_fram_write(USER_BKP_SATRT_ADDR,(u8 *)&g_para_bkp,sizeof(Fram_Para_Struct));
		}
		else
		{
			set_para_to_default_value();
		}

		
	}
}

void app_fram_para_wait_lpm_commit(void)
{
	g_para.sum = get_para_sum_vlue(&g_para);

	memcpy((u8 *)&g_para_bkp,(u8 *)&g_para,sizeof(Fram_Para_Struct));

	app_fram_write(USER_PARA_START_ADDR,(u8 *)&g_para,sizeof(Fram_Para_Struct));

	app_fram_write(USER_BKP_SATRT_ADDR,(u8 *)&g_para_bkp,sizeof(Fram_Para_Struct));
}

void app_fram_para_enter_lpm_init(void)
{
	
}

void app_fram_para_exit_lpm_init(void)
{
	memset((u8 *)&g_para    ,0,sizeof(Fram_Para_Struct));

	memset((u8 *)&g_para_bkp,0,sizeof(Fram_Para_Struct));

	FLASH_SetProgrammingTime((FLASH_ProgramTime_TypeDef)FLASH_ProgramMode_Standard);
	
	app_fram_read(USER_PARA_START_ADDR,(u8 *)&g_para,sizeof(Fram_Para_Struct));

	app_fram_read(USER_BKP_SATRT_ADDR,(u8 *)&g_para_bkp,sizeof(Fram_Para_Struct));

	if(!check_para_file_is_vaild(&g_para))
	{
		if(check_para_file_is_vaild(&g_para_bkp))
		{
			memcpy((u8 *)&g_para,(u8 *)&g_para_bkp,sizeof(Fram_Para_Struct));

			app_fram_write(USER_PARA_START_ADDR,(u8 *)&g_para,sizeof(Fram_Para_Struct));

			app_fram_write(USER_BKP_SATRT_ADDR,(u8 *)&g_para_bkp,sizeof(Fram_Para_Struct));
		}
		else
		{
			set_para_to_default_value();
		}
	}
}

