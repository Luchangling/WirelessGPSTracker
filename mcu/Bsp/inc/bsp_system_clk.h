/**
* Copyright (C), Goome Technologies Co., Ltd. 2009-2019. All rights reserved.
* File name : bsp_system_clk.h
* Author:  Version:	Date:
* Description: bsp_system_clkc 的头文件
* Function List:
* History:
  1.Date: 2019/4/9
    Author: Chris.Lu
    Modification: Created file
*/
#ifndef __BSP_SYSTEM_CLKC_H__
#define __BSP_SYSTEM_CLKC_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

extern void bsp_sys_clk_enter_lpm_init(void);
extern void bsp_sys_clk_exit_lpm_init(void);
extern void bsp_sys_clk_power_up_init(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __BSP_SYSTEM_CLKC_H__ */
