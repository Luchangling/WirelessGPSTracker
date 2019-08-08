/**
* Copyright (C), Goome Technologies Co., Ltd. 2009-2019. All rights reserved.
* File name : bsp.h
* Author:  Version:	Date:
* Description: bsp.c 的头文件
* Function List:
* History:
  1.Date: 2019/4/9
    Author: Chris.Lu
    Modification: Created file
*/
#ifndef __BSP_H__
#define __BSP_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */
#include "stm8l15x.h"

typedef enum
{
	BSP_LPM_L0, /*L0 等级正常运行*/
	
	BSP_LPM_L1, /*L1 等级休眠关闭模块电源和相关通信串口*/

	BSP_LPM_L2, /*L2 等级休眠关闭时钟和所有外设*/

	BSP_LPM_L3  /*L3 等级休眠芯片进入halt 极致省电*/
	
}BSP_LPM_LEVEL_ENUM;

extern void bsp_enter_lpm_init(BSP_LPM_LEVEL_ENUM level);
extern void bsp_exit_lpm_init(BSP_LPM_LEVEL_ENUM level);
extern void bsp_power_up_init(void);
extern u8 get_bsp_lpm_state(void);
extern u8 does_system_in_ultra_lpm(void);



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __BSP_H__ */
