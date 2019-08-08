/**
* Copyright (C), Goome Technologies Co., Ltd. 2009-2019. All rights reserved.
* File name : 
* Author:  Version:	Date:
* Description: 
* Function List:
* History:
  1.Date: 2019/4/9
    Author: Chris.Lu
    Modification: Created file
*/

#ifndef __DEBUG_PORT_H__
#define __DEBUG_PORT_H__

#include "gm_type.h"

extern void bsp_debug_uart_power_up_init(void);

extern void bsp_debug_uart_enter_lpm_init(void);

extern void bsp_debug_uart_exit_lpm_init(void);

extern void print(const char* fmt ,...);


#endif

