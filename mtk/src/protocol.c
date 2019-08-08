#include <stdio.h>
#include "utility.h"
#include "gm_stdlib.h"
#include "gm_memory.h"
#include "gm_type.h"
#include "gsm.h"
#include "protocol.h"
#include "protocol_2929.h"
#include "log_service.h"
#include "config_service.h"
#include "gprs.h"
#include "gm_gprs.h"

void protocol_msg_receive(SocketType *socket)
{
    u8 head[7];
    u32 len = sizeof(head);
    u16 msg_len = 0;
    u8 *pdata;
    static u32 packet_error_start = 0;


    if(GM_SUCCESS != fifo_peek(&socket->fifo, head, len))
    {
        // no msg
        return;
    }

    if(head[0]!=PROTOCOL_HEADER_2929||head[1]!=PROTOCOL_HEADER_2929)
    {
        LOG(WARN,"clock(%d) protocol_msg_receive head assert(head(%x%x)) failed.",util_clock(), head[0],head[1]);
        fifo_reset(&socket->fifo);
        gm_socket_close_for_reconnect(socket);
    }

    msg_len = MKWORD(head[3], head[4]);

    msg_len += 5;

    if(msg_len > MAX_GPRS_MESSAGE_LEN)
    {
        LOG(WARN,"clock(%d) protocol_msg_receive assert(msg_len(%d)) failed.",util_clock(), msg_len);
        //clear fifo and restart socket.
        fifo_reset(&socket->fifo);
        gm_socket_close_for_reconnect(socket);
        return;
    }

    pdata = GM_MemoryAlloc(msg_len);
    if(pdata == NULL)
    {
        LOG(INFO,"clock(%d) protocol_msg_receive alloc buf failed. len:%d", util_clock(), msg_len);

        //clear fifo and restart socket.
        fifo_reset(&socket->fifo);
        gm_socket_close_for_reconnect(socket);
        return;
    }

    if(GM_SUCCESS != fifo_peek(&socket->fifo, pdata, msg_len))
    {
        // GM_EMPTY_BUF
        GM_MemoryFree(pdata);
        
        if(packet_error_start == 0)
        {
            LOG(DEBUG,"clock(%d) protocol_msg_receive get msg failed. len:%d", util_clock(), msg_len);
            log_service_print_hex((const char*)head, sizeof(head));
            packet_error_start = util_clock();
        }
        else
        {
            if((util_clock() - packet_error_start) > MAX_GPRS_PART_MESSAGE_TIMEOUT)
            {
                LOG(WARN,"clock(%d) protocol_msg_receive MAX_GPRS_PART_MESSAGE_TIMEOUT.",util_clock());
                //clear fifo and restart socket.
                fifo_reset(&socket->fifo);
                gm_socket_close_for_reconnect(socket);
                packet_error_start = 0;
            }
        }
        return;
    }
    fifo_pop_len(&socket->fifo, msg_len);
	LOG(DEBUG,"clock(%d) protocol_msg_receive msg len(%d) head(%02x)", util_clock(), msg_len, head[0]);

    protocol_2929_parse_msg(pdata,  msg_len);

    GM_MemoryFree(pdata);
}




