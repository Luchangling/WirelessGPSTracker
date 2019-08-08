/**
* Copyright (C), Goome Technologies Co., Ltd. 2009-2019. All rights reserved.
* File name : bsp_model_power_contrl.h
* Author:  Version:	Date:
* Description: bsp_model_power_contrl.c 的头文件
* Function List:
* History:
  1.Date: 2019/4/9
    Author: Chris.Lu
    Modification: Created file
*/
#ifndef __BSP_MODEL_POWER_CONTRL_H__
#define __BSP_MODEL_POWER_CONTRL_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

extern void bsp_model_power_enter_lpm_init(void);
extern void bsp_model_power_exit_lpm_init(void);
extern void bsp_model_power_power_up_init(void);
extern void bsp_model_prapre_power_power_up(void);
extern void bsp_model_restart(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __BSP_MODEL_POWER_CONTRL_H__ */
