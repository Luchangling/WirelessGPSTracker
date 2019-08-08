/**
 * Copyright @ Goome Technologies Co., Ltd. 2009-2019. All rights reserved.
 * File name:        config_service.h
 * Author:           梁震       
 * Version:          1.0
 * Date:             2019-03-01
 * Description:      
 * Others:      
 * Function List:    
    1. 创建config_service模块
    2. 销毁config_service模块
    3. config_service模块定时处理入口
 * History: 
    1. Date:         2019-03-01
       Author:       梁震
       Modification: 创建初始版本
    2. Date: 		 
	   Author:		 
	   Modification: 

 */

#ifndef __CONFIG_SERVICE_H__
#define __CONFIG_SERVICE_H__

#include "gm_type.h"
#include "error_code.h"
#include "socket.h"
#include "gps.h"

//发布版修改这个数字
#define VERSION_NUMBER  "1.0.26"

#define DEFAULT_DEVICE_TYPE "GW01H"
#define DEFAULT_SMS_PWD "GMGPS"
#define APN_DEFAULT "CMNET"
#define APN_USER_DEFAULT ""
#define APN_PWD_DEFAULT ""
#define GOOME_IMEI_DEFAULT "668613000000000"
#define GOOME_DEV_NUMBER_DEFAULT "13000000000"
#define UPDATE_OEM_CODE  "GOOME"
#define UPDATE_DEVICE_CODE "DEVICEID00001"
#define UPDATE_BOOT_CODE "FFFFFFFF"
//#define CONFIG_LITE_SERVER_ADDERSS     "litedev.gmiot.net:10021"
#define CONFIG_GOOCAR_SERVER_ADDERSS   "gw01.szdatasource.com:8841"
#define CONFIG_LITE_SERVER_IP          "119.23.109.222"
#define CONFIG_GOOCAR_SERVER_IP        "54.222.183.200"
#define CONFIG_FACTORY_SERVER_ADDERSS  "factorytest.gpsorg.net:8841"
#define CONFIG_SERVER_ADDERSS          "config.gpsorg.net:39996"
#define CONFIG_SERVER_IP               "47.106.251.151"
#define CONFIG_AGPS_SERVER_ADDERSS     "agps.srv.gpsorg.net:8866"
#define CONFIG_AGPS_SERVER_IP          "119.23.109.222"
#define CONFIG_LOG_SERVER_ADDERSS      "firmwarelog.gpsorg.net:39998"
#define CONFIG_LOG_SERVER_IP           "47.106.251.151"
#define GOOME_UPDATE_SERVER_DNS        "firmware.gpsorg.net:39999"
#define GOOME_UPDATE_SERVER_IP         "47.106.251.151"

#define CONFIG_CMD_MAX_LEN         10
#define CONFIG_STRING_MAX_LEN      100

#define CONFIG_UPLOAD_TIME_MAX     10800ul
#define CONFIG_UPLOAD_TIME_DEFAULT 10 
#define CONFIG_UPLOAD_TIME_MIN     3 
#define CONFIG_SPEEDTHR_MAX        180
#define CONFIG_SPEEDTHR_MIN        5

#define GOOME_LITEDEV_DNS          ".gmiot.net"
#define GOOME_GPSOO_DNS            ".szdatasource.com"


#define GOOME_MAPS_URL_DEFAULT "http://ditu.google.cn/maps?q="

typedef enum
{
  EXTI_Trigger_Falling_Low    = (u8)0x00, /*!< Interrupt on Falling edge and Low level */
  EXTI_Trigger_Rising         = (u8)0x01, /*!< Interrupt on Rising edge only */
  EXTI_Trigger_Falling        = (u8)0x02, /*!< Interrupt on Falling edge only */
  EXTI_Trigger_Rising_Falling = (u8)0x03  /*!< Interrupt on Rising and Falling edges */
} EXTI_Trigger_TypeDef;


enum
{
	TRG_FALL_LOW = EXTI_Trigger_Falling_Low,
	TRG_RISING   = EXTI_Trigger_Rising,
	TRG_FALLING  = EXTI_Trigger_Falling,
	TRG_RIS_FALL = EXTI_Trigger_Rising_Falling,
	TRG_NO_ONE
};


typedef enum
{
    CFG_SERVERADDR,           // 主服务器地址,xxx.xxx.xxx.xxx:port      = 0
    
    CFG_UPLOADTIME,           // 上传gps间隔

    CFG_APN_NAME,

    CFG_APN_USER,

    CFG_APN_PWD,

    CFG_DEVICE_NUMBER,

    //1 agps  0 epo  由于有CFG_GPS_TYPE,该配置已无用
    CFG_OPEN_APGS, 
    
	//GPS关闭:0——不关闭,1——关闭
	CFG_GPS_CLOSE,

	//GPS模块型号[TYPE_INT]:0——未知；1——内置GPS模块（串口速率115200）；2——泰斗（串口速率9600）；3-中科微（串口速率9600）,added by bruce
	CFG_GPS_TYPE,

    //GPS定位检测时间,默认5秒
    CFG_GPS_UPDATE_TIME,

    /*配置服务器地址,xxx.xxx.xxx.xxx:port*/
    CFG_CFGSERVERADDR,     //=70
    
    /*测试服务器地址,xxx.xxx.xxx.xxx:port*/
    CFG_TEST_SERVERADDR,
    
    /*AGPS服务器地址,xxx.xxx.xxx.xxx:port*/
    CFG_AGPSSERVERADDR,
    
    /*日志服务器地址,xxx.xxx.xxx.xxx:port*/
    CFG_LOGSERVERADDR,
    
    /*更新服务器地址,xxx.xxx.xxx.xxx:port*/
    CFG_UPDATESERVERADDR,
    
    /*文件服务器地址,xxx.xxx.xxx.xxx:port*/
    CFG_UPDATEFILESERVER,

    CFG_MIN_SNR,

	//打开GPS多久后AGPS准备好要重新打开GPS
	CFG_REOPEN_GSP_TIME,

    CFG_SMOOTH_TRACK,

    CFG_CUSTOM_CODE,      //客户代码
    CFG_TERM_MODEL,       //终端型号ID
    CFG_TERM_VERSION,     //终端当前版本号     //100
    CFG_TERM_BOOT_CHECK,  //终端当前版本BOOT校验码
    
	
    //时区[TYPE_BYTE]
    CFG_TIME_ZONE,

    CFG_EXPMU_PWRON_CNT,

    CFG_TIMING_NUMBER,

    CFG_MTK_PWRON_CNT,

    CFG_WORK_MODE,

    CFG_POSITION_SOURCE,

    CFG_WORK_MODE_PARAM,

    CFG_APN_CHECK,

    CFG_WIFI_STATE,

    CFG_CHANG_PWD,

    CFG_PWR_REP_CNT,

    CFG_REP_GAP,

    CFG_SMS_PWD,

    CFG_LIGHT_STATE,

    CFG_STATIS_INFO,

    //当前是否测试模式
    CFG_IS_TEST_MODE,

    CFG_DEV_TYPE,

    CFG_PARAM_MAX
}ConfigParamEnum;

typedef enum
{
    TYPE_NONE = 0x00,
    TYPE_INT = 0x01,
    TYPE_SHORT = 0x02,
    TYPE_STRING = 0x03,
    TYPE_BOOL = 0X04,
    TYPE_FLOAT = 0X05,
    TYPE_BYTE = 0x06,
    TYPE_ARRY = 0x07,
    TYPE_MAX
}ConfigParamDataType;

typedef enum
{
    CFG_CMD_REQ = 0x13,
    CFG_CMD_ACK = 0x92,
    CFG_CMD_RESULT = 0x14,
    CFG_CMD_MAX
}ConfigCmdEnum;

typedef enum
{
    CFG_CMD_REQ_ALL = 0x00,
    CFG_CMD_REQ_ONCE = 0x01,
    CFG_CMD_REQ_PERMANENT= 0x02,
}ConfigCmdReqType;

typedef enum
{
    HEART_SMART = 0x00,  //0x03 or 0x07
    HEART_NORMAL = 0x01,  //0x03
    HEART_EXPAND = 0x02,  //0x07
    HEART_MAX
}ConfigHearUploadEnum;


typedef struct
{
    ConfigParamDataType type;
    union
    {
        u8*  str;
        u32  i;
        u16  s;
        u8   b;
        float f;
    }data;
    u16 len;
} ConfigParamItems;


typedef enum
{
    PROTOCOL_NONE = 0x00,
    PROTOCOL_GOOME = 0x01,
    PROTOCOL_CONCOX = 0x02,
    PROTOCOL_JT808 = 0x03,
    PROTOCOL_2929 = 0x04,
    PROTOCOL_MAX
}ConfigProtocolEnum;


enum
{
    PLATFORM_LOC_MODE_WIFI = 0,   //WiFi定位

	PLATFORM_LOC_MODE_GPS = 1,   //GPS定位
    
    PLATFORM_LOC_MODE_LBS = 2,   //非GPS定位

	PLATFORM_LOC_MODE_INVAILD,
};


enum
{
	ALARM_CLOCK_MODE = 1,       // 闹钟模式 [一天之中哪个时间点起来,最多可以设置4个]

    FIX_FRQ_MODE = 2,           // 

    TRACK_MODE = 3,             //定时模式     AppStu.WorkMode  [每隔多长时间起来.比如5分钟]

    WEEK_ALARM_MODE = 4,        //星期模式 [每个星期的星期几起来]
	
	PLATFORM_CYCLIC_MODE = 5,   //平台模式中的循环模式
	
	PLATFORM_TIMEPOINT_MODE = 6,//平台模式中的时间点模式

	WORK_MODE_INVALID,
};



//断电报警方式 00: 只GPRS方式 01:SMS+GPRS  10: GPRS+SMS+CALL 11:GPRS+CALL 默认:01
typedef enum
{
    PWRALM_GPRS = 0,
    PWRALM_GPRS_SMS = 1,
    PWRALM_GPRS_SMS_CALL = 2,
    PWRALM_GPRS_CALL = 3,
}PowerAlarmMode;


typedef struct
{
	u32 day;
	u32 time_point;

}WeekkModeTimePointStruct;

typedef struct
{
	u32 time_point[4];
}ClockModeTimePointStruct;

typedef struct
{
	u32 interval;
}TrackeModeTimeValueStruct;


typedef struct
{
	u32 pos_mode;
	u32 loop_start_tim;
	u32 loop_interval; /*单位分钟*/
	u32 loop_stop_tim;
}PlatLoopModeStruct;

typedef struct
{
	u32 pos_mode;
	u32 utc_time_point;
}PlatTimePiontModeStruct;


typedef union
{
	PlatTimePiontModeStruct plat_tp[3];

	PlatLoopModeStruct plat_loop;

	TrackeModeTimeValueStruct track;

	ClockModeTimePointStruct clock;

	WeekkModeTimePointStruct week;
	
}WorkModeParamUnion;



#define WORK_MODE_PARAM_LEN 24

#define MAX_SMS_PWD_LEN     11

#define MAX_DEVICE_STRING   10


/**
 * Function:   创建config_service模块
 * Description:创建config_service模块
 * Input:	   无
 * Output:	   无
 * Return:	   GM_SUCCESS——成功；其它错误码——失败
 * Others:	   使用前必须调用,否则调用其它接口返回失败错误码
 */
GM_ERRCODE config_service_create(void);

/**
 * Function:   销毁config_service模块
 * Description:销毁config_service模块
 * Input:	   无
 * Output:	   无
 * Return:	   GM_SUCCESS——成功；其它错误码——失败
 * Others:	   
 */
GM_ERRCODE config_service_destroy(void);

/**
 * Function:   config_service模块定时处理入口
 * Description:config_service模块定时处理入口
 * Input:	   无
 * Output:	   无
 * Return:	   GM_SUCCESS——成功；其它错误码——失败
 * Others:	   1秒钟调用1次
 */
GM_ERRCODE config_service_timer_proc(void);


/**
 * Function:   
 * Description: 配置socket连接正常
 * Input:	   无
 * Output:	   无
 * Return:	   无
 * Others:	   无
 */
void config_service_connection_ok(void);

/**
 * Function:   
 * Description: 配置socket连接失败
 * Input:	   无
 * Output:	   无
 * Return:	   无
 * Others:	   无
 */
void config_service_connection_failed(void);
void config_service_close_for_reconnect(void);
   


/**
 * Function:   
 * Description: 获取指定id对应的配置
 * Input:	   id:配置id    type:数据类型
 * Output:	   无
 * Return:	   数据长度
 * Others:	   该函数只有字符串类型的配置, 不知道长度里, 需要先调用以获知长度
 */
u16 config_service_get_length(ConfigParamEnum id, ConfigParamDataType type);

/**
 * Function:   
 * Description: 根据id与type 获取配置到buf中.
 * Input:	   id:配置id    type:数据类型      buf:输出,用来存储配置数据            len:buf长度
 * Output:	   无
 * Return:	   无
 * Others:	   无
 */
GM_ERRCODE config_service_get(ConfigParamEnum id, ConfigParamDataType type, void* buf, u16 len);

/**
 * Function:   
 * Description: 根据id与type 把 buf中的内容 设置到config_service模块
 * Input:	   id:配置id    type:数据类型      buf:用来存储配置数据            len:buf长度
 * Output:	   无
 * Return:	   无
 * Others:	   无
 */
void config_service_set(ConfigParamEnum id, ConfigParamDataType type, const void* buf, u16 len);

/**
 * Function:   
 * Description: 此函数可代替config_service_get, 特别是string类型, 会少掉字符串拷贝的时间
 * Input:	   id:配置id 
 * Output:	   无
 * Return:	   指向id对应配置的指针
 * Others:	   无
 */
void* config_service_get_pointer(ConfigParamEnum id);

    
/**
 * Function:   
 * Description: 获取当前协议配置
 * Input:	   无
 * Output:	   无
 * Return:	   当前协议
 * Others:	   无
 */
ConfigProtocolEnum config_service_get_app_protocol(void);

/**
 * Function:   
 * Description: 获取当前心跳协议配置
 * Input:	   无
 * Output:	   无
 * Return:	   心跳协议
 * Others:	   无
 */
ConfigHearUploadEnum config_service_get_heartbeat_protocol(void);

/**
 * Function:   
 * Description: 获取当前时区配置
 * Input:	   无
 * Output:	   无
 * Return:	   时区
 * Others:	   无
 */
S8 config_service_get_zone(void);


/**
 * Function:   
 * Description: 用何种心跳协议
 * Input:	   无
 * Output:	   无
 * Return:	   心跳协议
 * Others:	   无
 */
bool config_service_is_main_server_goome(void);



/**
 * Function:   
 * Description: 告警只发送给一个号码
 * Input:	   无
 * Output:	   无
 * Return:	   心跳协议
 * Others:	   无
 */
bool config_service_is_alarm_sms_to_one(void);

/**
 * Function:   
 * Description: 获取短信发送号码
 * Input:	   无
 * Output:	   无
 * Return:	   号码长度
 * Others:	   无
 */
u8 config_service_get_sos_number(u8 index, u8 *buf, u16 len);



/**
 * Function:   
 * Description: 是否启用agps, 如果不是, 表示启用epo
 * Input:	   无
 * Output:	   无
 * Return:	   1 apgs    0 epo
 * Others:	   无
 */
bool config_service_is_agps(void);



/**
 * Function:   
 * Description: 获取当前gps类型
 * Input:	   无
 * Output:	   无
 * Return:	   时区
 * Others:	   无
 */
GPSChipType config_service_get_gps_type(void);



/**
 * Function:   
 * Description: 从本地配置文件中获取配置信息
 * Input:	   configs列表指针
 * Output:	   configs列表指针
 * Return:	   GM_SUCCESS 成功, 其它失败
 * Others:	   无
 */
GM_ERRCODE config_service_read_from_local(void);

/**
 * Function:   
 * Description: 保存配置到本地文件中
 * Input:	   configs列表指针
 * Output:	   无
 * Return:	   GM_SUCCESS 成功, 其它失败
 * Others:	   无
 */
GM_ERRCODE config_service_save_to_local(void);


/**
 * Function:   根据ID获取设备型号
 * Description:获取设备型号(字符串)
 * Input:	   设备型号
 * Output:	   
 * Return:	   设备型号(字符串)
 * Others:	   无
 */
const char * config_service_get_device_type(u16 index);


/**
 * Function:   
 * Description: 恢复出厂设置
 * Input:	   configs列表指针     is_all(是不是包括ip地址设置)
 * Output:	   configs列表指针
 * Return:	   GM_SUCCESS 成功, 其它失败
 * Others:	   无
 */
void config_service_restore_factory_config(bool is_all);

/**
 * Function:   
 * Description: 设置测试模式
 * Input:	   state:true——进入测试模式;false——退出测试模式
 * Output:	   无
 * Return:	   无
 * Others:	   无
 */
void config_service_set_test_mode(bool state);


/**
 * Function:   
 * Description: 判断当前是不是测试模式
 * Input:	   无
 * Output:	   无
 * Return:	   true 测试模式 , false 非测试模式
 * Others:	   无
 */
bool config_service_is_test_mode(void);

/**
 * Function:   
 * Description: 判断当前的imei是否写死在代码中的默认imei
 * Input:	   无
 * Output:	   无
 * Return:	   true GOOME_IMEI_DEFAULT , false 非 GOOME_IMEI_DEFAULT
 * Others:	   无
 */
bool config_service_is_default_imei(void);

/**
 * Function:   
 * Description: 设置设备的sim号
 * Input:	   sim
 * Output:	   GM_SUCCESS 成功, 其它失败
 * Return:	   无
 * Others:	   无
 */
GM_ERRCODE config_service_set_device_sim(u8 *pdata);


/**
 * Function:   
 * Description: 改变ip地址
 * Input:	   idx:配置id,其值是(xxx.xxx.xxx.xxx:port)形式;   buf: dns  ;len: dns长度
 * Output:	   无
 * Return:	   无
 * Others:	   无
 */
void config_service_change_ip(ConfigParamEnum idx, u8 *buf, u16 len);

/**
 * Function:   
 * Description: 改变port
 * Input:	   idx:配置id,其值是(xxx.xxx.xxx.xxx:port)形式;   port:端 口
 * Output:	   无
 * Return:	   无
 * Others:	   无
 */
void config_service_change_port(ConfigParamEnum idx, u16 port);


/*
    
*/
/**
 * Function:   
 * Description: 升级使用的socket类型
 * Input:      无
 * Output:     无
 * Return:     升级使用的socket类型
 * Others:     无
 */
StreamType config_service_update_socket_type(void);



S8 config_service_get_zone(void);

void config_service_set_reming_gps_count(void);
StreamType config_service_agps_socket_type(void);

#endif


