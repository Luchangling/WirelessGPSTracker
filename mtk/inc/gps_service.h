#ifndef __GPS_SERVICE_H__
#define __GPS_SERVICE_H__

#include "gm_type.h"
#include "error_code.h"
#include "socket.h"
#include "gps.h"

typedef enum
{
    GPS_MODE_NONE = -1,           // 不上传
    GPS_MODE_FIX_TIME = 0,        // 定时上传 
    GPS_MODE_FIX_DISTANCE = 1,    // 定距上传（终端未做） 
    GPS_MODE_TURN_POINT = 2,      // 拐点上传 
    GPS_MODE_STATUS_CHANGE = 3,   // ACC状态改变上传 
    GPS_MODE_STATIC_POSITION = 4, // 静止最后位置上传 
    GPS_MODE_POWER_UP = 5         // 上电登录成功后直接上传
}GpsDataModeEnum;


/**
 * Function:   创建gps_service模块
 * Description:创建gps_service模块
 * Input:	   无
 * Output:	   无
 * Return:	   GM_SUCCESS——成功；其它错误码——失败
 * Others:	   使用前必须调用,否则调用其它接口返回失败错误码
 */
GM_ERRCODE gps_service_create(bool first_create);


/**
 * Function:   销毁gps_service模块
 * Description:销毁gps_service模块
 * Input:	   无
 * Output:	   无
 * Return:	   GM_SUCCESS——成功；其它错误码——失败
 * Others:	   
 */
GM_ERRCODE gps_service_destroy(void);


/**
 * Function:   gps_service模块定时处理入口
 * Description:gps_service模块定时处理入口
 * Input:	   无
 * Output:	   无
 * Return:	   GM_SUCCESS——成功；其它错误码——失败
 * Others:	   1秒钟调用1次
 */
GM_ERRCODE gps_service_timer_proc(void);


/**
 * Function:   获取当前正在使用的IP地址
 * Description:
 * Input:	   无
 * Output:	   无
 * Return:	   IP地址首地址
 * Others:	   
 */
U8* gps_service_get_current_ip(void);


GM_ERRCODE gps_service_change_config(void);

u8 gps_service_position_send_ok(void);

extern void gps_service_connection_ok(void);

extern void gps_service_close_for_reconnect(void);

extern void gps_service_connection_failed(void);

extern void gps_service_confirm_gps_cache(void);

extern void gps_service_push_one_gps(const GPSData *gps);

extern void gps_service_push_lbs(void);

extern void gps_service_clear_one_gps(void);

extern void gps_service_send_one_device_ack(u8 *buff , u16 len);

extern void gps_service_clear_all_wait_gps(void);

extern u32 gps_service_get_location_counts(void);

extern GM_ERRCODE gps_service_restart(void);

extern void gps_service_destroy_gprs(void);

#endif


