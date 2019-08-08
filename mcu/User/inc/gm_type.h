/**
* Copyright (C), Goome Technologies Co., Ltd. 2009-2019. All rights reserved.
* File name : gm_type.h
* Author:  Version:	Date:
* Description: gm_type.c ��ͷ�ļ�
* Function List:
* History:
  1.Date: 2019/4/9
    Author: Chris.Lu
    Modification: Created file
*/
#ifndef __GM_TYPE_H__
#define __GM_TYPE_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */
#include "stm8l15x.h"
#if 0
typedef unsigned long  u32;
typedef unsigned short u16;
typedef unsigned char  u8;

typedef signed long  s32;
typedef signed short s16;
typedef signed char  s8;
#endif

typedef enum 
{
	GM_SUCCESS = 0,             //成功
	GM_UNKNOWN = -1,            //未知错误
	GM_NOT_INIT = -2,           //未初始化
	GM_MEM_NOT_ENOUGH = -3,     //内存不足
	GM_PARAM_ERROR = -4,        //参数错误
	GM_EMPTY_BUF = -5,          //内存为空
    GM_ERROR_STATUS = -6,       //错误状态
    GM_HARD_WARE_ERROR = -7,    //硬件错误
    GM_SYSTEM_ERROR = -8,       //系统错误
    GM_WILL_DELAY_EXEC = -9,    //延时执行
}GM_ERRCODE;

#define _DEBUG_

#ifndef NULL
#define NULL 0
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __GM_TYPE_H__ */
