/**
* Copyright (C), Goome Technologies Co., Ltd. 2009-2019. All rights reserved.
* File name : slave_mem_share_protocol.h
* Author:  Version:	Date:
* Description: slave_mem_share_protocol.c 的头文件
* Function List:
* History:
  1.Date: 2019/4/17
    Author: Chris.Lu
    Modification: Created file
*/
#ifndef __SLAVE_MEM_SHARE_PROTOCOL_H__
#define __SLAVE_MEM_SHARE_PROTOCOL_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

extern u8 check_arry_data_xor(u8 *data , u16 len);
extern u16 install_query_packet(u8 mark);
extern u16 prase_read_packet_block(u8 *block , u8 mark , u16 max_len);
extern u16 prase_write_pracket_block(u8 *block , u8 mark , u16 max_len);
extern void send_smsp_packet_to_uart(u8 *pdata , u16 len);
extern u8 slave_mem_share_protocol_prase(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __SLAVE_MEM_SHARE_PROTOCOL_H__ */
