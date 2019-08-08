/**
* Copyright (C), Goome Technologies Co., Ltd. 2009-2019. All rights reserved.
* File name : bsp_light_sensor.h
* Author:  Version:	Date:
* Description: bsp_light_sensor.c 的头文件
* Function List:
* History:
  1.Date: 2019/4/9
    Author: Chris.Lu
    Modification: Created file
*/
#ifndef __BSP_LIGHT_SENSOR_H__
#define __BSP_LIGHT_SENSOR_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#include "gm_type.h"

extern void bsp_light_sensor_enter_lpm_init(void);
extern void bsp_light_sensor_exit_lpm_init(void);
extern void bsp_light_sensor_power_up_init(void);
extern u8 get_exit_lpm_exti1_io_state(void);
extern void bsp_light_sensor_set_wkp_result(void);
extern void light_sensor_period_check_pin_10ms(void);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __BSP_LIGHT_SENSOR_H__ */
