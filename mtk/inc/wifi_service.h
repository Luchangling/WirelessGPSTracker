
#ifndef __WIFI_SERVICE_H__
#define __WIFI_SERVICE_H__

#include "gm_type.h"


#define ENABLE_WIFI_LOCATION				1
#define	DISABLE_WIFI_LOCATION				0

#define SCANONLY_MAX_SCAN_AP_NUM			30
#define SCANONLY_MAC_ADDRESS_LEN	    	6
#define SCANONLY_SSID_MAX_LEN				32


/* Result enum */
typedef enum
{
    WLAN_RESULT_SUCCESS,
    WLAN_RESULT_WOULDBLOCK,
    WLAN_RESULT_FAIL,
    
    WLAN_RESULT_TOTAL
} wlan_result_enum;

/* Struct */
typedef struct
{
    // Reserved struct
    void *para;
} wlan_init_req_struct;


typedef enum
{  
   SCANONLY_SUCCESS = 0,
   
   SCANONLY_INIT_BUSY_IS_INITING= 1,
   SCANONLY_INIT_FAIL__ALREAD_INITED,
   SCANONLY_INIT_FAIL__DRIVER_REASON,
   SCANONLY_INIT_FAIL__UNKOWN,

   SCANONLY_DEINIT_BUSY__IS_DEINITING = 10,
   SCANONLY_DEINIT_FAIL__ALREAD_DEINITED,
   SCANONLY_DEINIT_FAIL__DRIVER_REASON,
   SCANONLY_DEINIT_FAIL__UNKOWN,

   SCANONLY_SCAN_BUSY__IS_SCANNING = 20,
   SCANONLY_SCAN_FAIL__NOT_INITED,
   SCANONLY_SCAN_FAIL__DRIVER_REASON,
   SCANONLY_SCAN_FAIL__UNKOWN,
   
   SCANONLY_STATUS_END
} SCANONLY_STATUS_ENUM;

typedef struct
{
    SCANONLY_STATUS_ENUM status;
} wlan_init_cnf_struct;

typedef struct
{
    SCANONLY_STATUS_ENUM status;
} wlan_deinit_cnf_struct;

typedef struct
{
    // Reserved struct
    void *para;
} wlan_deinit_req_struct;

typedef struct scanonly_scan_ap_info_struct
{    
  kal_uint8        bssid[ SCANONLY_MAC_ADDRESS_LEN ];
  kal_uint8        ssid_len;
  kal_uint8        ssid [ SCANONLY_SSID_MAX_LEN ];    
  kal_int8         rssi;                           
  kal_uint8        channel_number;    
} scanonly_scan_ap_info_struct;

typedef struct
{
    SCANONLY_STATUS_ENUM                 status;
    kal_uint8                            scan_ap_num;
    scanonly_scan_ap_info_struct         scan_ap[SCANONLY_MAX_SCAN_AP_NUM];
} wlan_scan_cnf_struct;

typedef struct
{
    u32 start_time;
    
    u32 record_time;    

    u8 fail_cnt;

    u8 scan_rate;

    wlan_scan_cnf_struct info;
}wifi_scan_result_struct;

typedef struct
{
    void *para;
    
    kal_uint8 scan_type;
    
} wlan_scan_req_struct;

typedef void (*wlan_init_cb_func_ptr) (void *user_data, wlan_init_cnf_struct *cnf);
typedef void (*wlan_deinit_cb_func_ptr) (void *user_data, wlan_deinit_cnf_struct *cnf);
typedef void (*wlan_scan_cb_func_ptr) (void *user_data, wlan_scan_cnf_struct *cnf);


extern wlan_result_enum wlan_init(wlan_init_req_struct *req, wlan_init_cb_func_ptr callback, void *user_data);
extern wlan_result_enum wlan_deinit(wlan_deinit_req_struct *req, wlan_deinit_cb_func_ptr callback, void *user_data);
extern wlan_result_enum wlan_scan(wlan_scan_req_struct *req, wlan_scan_cb_func_ptr callback, void *user_data);

GM_ERRCODE wifi_service_creat(void);


GM_ERRCODE wifi_power_off(void);

GM_ERRCODE wifi_power_on(u8 scan_rate);


wifi_scan_result_struct *wifi_service_get_wifi_scan_result(void);



#endif
