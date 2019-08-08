/**
* Copyright (C), Goome Technologies Co., Ltd. 2009-2019. All rights reserved.
* File name : watch_dog.h
* Author:  Version:	Date:
* Description: watch_dog.c 的头文件
* Function List:
* History:
  1.Date: 2019/4/9
    Author: Chris.Lu
    Modification: Created file
*/
#ifndef __WATCH_DOG_H__
#define __WATCH_DOG_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

extern void feed_dog(void);
extern void watch_dog_enter_lpm_init(void);
extern void watch_dog_exit_lpm_init(void);
extern void watch_dog_power_up_init(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __WATCH_DOG_H__ */
