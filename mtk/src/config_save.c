#include "gm_type.h"
#include "stdio.h"
#include "gm_stdlib.h"
#include "gm_gprs.h"
#include "config_save.h"
#include "config_service.h"
#include "log_service.h"
#include "utility.h"
#include "gm_memory.h"
#include "gm_fs.h"
#include "g_sensor.h"
#include "system_state.h"
#include "gm_system.h"
#include "g_sensor.h"
#include "gsm.h"
#include "applied_math.h"

#define goome_para_main L"Z:\\goome\\GmParaMain\0"
#define goome_para_minor L"Z:\\goome\\GmParmMinor\0"
#define MAGIC_NUMBER 0xABCDDCBA

enum
{
    UOP_FAIL = 0,  // 错误
    UOP_OK,         // 正确
    
    UOP_CMD_ERR,  // 命令错误
    UOP_LEN_ERR,   // 长度错误
    UOP_PARA_ERR, // 参数错误
    UOP_CSM_ERR,  // 校验值错误
    UOP_CSM_OK,
    UOP_EXIST,  // 7
    
    UOP_MAGIC_ERR,
    UOP_MAGIC_OK,
    UOP_STATUS_1,
    UOP_STATUS_2,
    UOP_STATUS_3,
    UOP_STATUS_4,
    UOP_STATUS_5,
    UOP_STATUS_6,
    UOP_STATUS_7,
    UOP_STATUS_8,
    UOP_STATUS_9,
    UOP_STATUS_10,
    
    UOP_NONE
};

typedef struct
{
   u8 IP[4];
   u16 PORT;
} UserTypeIPV4; 

typedef struct
{
   u8 dns[GOOME_DNS_MAX_LENTH];
   u16 port;
} UserTypeDns; 

typedef enum
{
    TCP_IP = 0,
    UDP_IP,
    DNS_IP,
    IP_MAX
}UserIPTypeEnum;

typedef struct
{
    u8 sock_type;
    UserTypeIPV4 sock_ip[IP_MAX];
    UserTypeDns  src_dns;
}SocketParamStruct;


// 关键参数结构体
#pragma pack(1)
typedef struct
{
    u32 len;
    u32 crc;
    u32 magic;

    u8 valid_flag; // UOP_OK:正常  UOP_PARA_ERR:参数错误
    u16 para_none;
    u16 para_err;
    u16 para_open_err;
    u16 para_crc_err;
    u16 para_mg_err;
    
    u16 para_from_mem;
    u16 para_from_file;
    u16 para_from_stm8;
    u16 para_from_server;
    u16 para_write_total;
    u16 para_res1;
    u16 para_res2;
    u16 para_res3;
    u16 para_res4;

    u16 para2_none;
    u16 para2_err;
    u16 para2_open_err;
    u16 para2_crc_err;
    u16 para2_mg_err;
    
    u16 para2_from_mem;
    u16 para2_from_file;
    u16 para2_write_total;
    u16 para2_from_stm8;
    u16 para2_from_server;
    u16 para2_res1;
    u16 para2_res2;
    u16 para2_res3;
    u16 device_type;

    SocketParamStruct    sock[6];
    u8 Apn[GOOME_APN_MAX_LENGTH];
    u8 strUser[GOOME_APN_MAX_LENGTH];
    u8 strPwd[GOOME_APN_MAX_LENGTH];   
    u8 strSim[GOOME_DEV_NUMBER_MAX_LEN];

    u8 work_mode;

    u8 position_source;

    u8 mod_param[WORK_MODE_PARAM_LEN];

    u8 gps_type;

    u8 GpsUpdatTime;

    s8  LocalTime;

    u8 auto_set_apn;

    u8 wifi_switch;

    u8 sms_pwd[MAX_SMS_PWD_LEN];

    u8 pos_mode;

    u8 light_switch;

    u16 UploadTim;

    u16 remaining_number;

    u16 pwron_rep_cnt;

    u16 no_signal_next_rep_gap;

    u8 min_snr;  //MTK GPS定位的最小信噪比参数

	u8 reopen_gps_time;

    u8 dvice_type[MAX_DEVICE_STRING];
    
}GprsParaFileType, *T_GprsParam;


static T_GprsParam           p_param_mem = NULL;
static p_nvram_ef_goome_param_struct s_nvram = NULL;

static GprsParaFileType * s_Para = NULL;
static GprsParaFileType * s_Para2 = NULL;


static void convert_cfg_to_para(GprsParaFileType *para);
static void convert_para_to_cfg(const GprsParaFileType *para);

static u32 goome_file_param_main_write(void);
static u32 goome_file_param_main_read(void);
static u32 goome_file_write_minor_param(void);
static u32 goome_file_read_minor_param(void);
static s32 goome_delete_file(u8 *file);
static u8 param_write_back_free_mem(void);
static u8 param_read_back_free_mem(void);
static void config_service_set_deault(void);
static bool check_para_ok(const GprsParaFileType *para);
static bool check_address_para_ok(u8 * param);
static void goome_file_nvram_save(void);




/************************************************************************
    Function :
    Description :
    Parameter : 
    Return : 
    Author: 
    Date:  Aug-29-2018
************************************************************************/
static u32 goome_file_param_main_write(void)
{
    u32 fs_len;
    int handle, ret;

    handle = GM_FS_Open(goome_para_main, GM_FS_READ_WRITE | GM_FS_ATTR_ARCHIVE | GM_FS_CREATE);
    if (handle < 0)
    {
        LOG(INFO,"goome_file_param_main_write param main file: open fail [%d]", handle);
        return 0;
    }
    
    s_Para->magic = MAGIC_NUMBER;
    s_Para->valid_flag = UOP_OK;
    
    s_Para->para_write_total++;
    s_Para2->para_write_total++;
    
    s_Para->len = sizeof(GprsParaFileType);
    s_Para->crc = applied_math_calc_common_crc16((u8*)&s_Para->magic, sizeof(GprsParaFileType)-8);  // 968
    
    
    ret = GM_FS_Write(handle, (void *)s_Para, sizeof(GprsParaFileType), &fs_len);
    if (ret < 0)
    {
        LOG(INFO,"goome_file_param_main_write param main file: write fail [%d]", ret);
        
        GM_FS_Close(handle);
        return 0;
    }
    
    GM_FS_Close(handle);
    
    LOG(INFO,"goome_file_param_main_write param main file, len:%d", fs_len);
    
    
    return 1;
}


/************************************************************************
    Function :
    Description :
    Parameter : 
    Return : 
    Author:  
    Date: Aug-29-2018
************************************************************************/
static u32 goome_file_param_main_read(void)
{
    u32 fs_len, crc;
    int handle, ret;
    
    handle = GM_FS_Open(goome_para_main, GM_FS_READ_ONLY | GM_FS_ATTR_ARCHIVE | GM_FS_CREATE);
    if (handle < 0)
    {
        LOG(INFO,"goome_file_param_main_read param main read: open fail [%d]", handle);
        s_Para->para_open_err++;
        return UOP_FAIL;
    }
    
    
    ret = GM_FS_Read(handle, (void *)s_Para, sizeof(GprsParaFileType), &fs_len);
    if (ret < 0 || fs_len != sizeof(GprsParaFileType))
    {
        LOG(INFO,"goome_file_param_main_read param main read: read fail len:%d ret:%d.",fs_len, ret);
        s_Para->para_open_err++;
        GM_FS_Close(handle);
        return UOP_FAIL;
    }
    
    GM_FS_Close(handle);
    
    crc = applied_math_calc_common_crc16((u8*)&s_Para->magic, sizeof(GprsParaFileType)-8);
    
    if (GM_strlen((const char* )s_Para->sock[SOCKET_INDEX_MAIN].src_dns.dns) >= sizeof(s_Para->sock[SOCKET_INDEX_MAIN].src_dns.dns)-1)
    {        
        GM_memset(&s_Para->sock[SOCKET_INDEX_MAIN].src_dns.dns, 0x00, sizeof(s_Para->sock[SOCKET_INDEX_MAIN].src_dns.dns));
    }
    
    if (s_Para->crc != crc)
    {
        LOG(INFO,"goome_file_param_main_read param main crc err:%x, %x ", s_Para->crc, crc);
        s_Para->para_crc_err++;
        
        return UOP_CSM_ERR;
    }
    
    if (s_Para->magic != MAGIC_NUMBER)
    {
        LOG(INFO,"goome_file_param_main_read param main magic err:%x, ", s_Para->magic);
        s_Para->para_mg_err++;
        return UOP_MAGIC_ERR;
    }

    return UOP_OK;;
}






/************************************************************************
    Function :
    Description :
    Parameter : 
    Return : 
    Author: 
    Date:  Sep-18-2018
************************************************************************/
static u32 goome_file_write_minor_param(void)
{
    u32 fs_len;
    int handle, ret;
        
    handle = GM_FS_Open(goome_para_minor, GM_FS_READ_WRITE | GM_FS_ATTR_ARCHIVE | GM_FS_CREATE);
    if (handle < 0)
    {
        LOG(INFO,"goome_file_write_minor_param open minor file fail [%d] \r\n", handle);
        return 0;
    }
    
    s_Para2->magic = MAGIC_NUMBER;
    s_Para2->valid_flag = UOP_OK;
    
    s_Para->para2_write_total++;
    s_Para2->para2_write_total++;
    
    s_Para2->len = sizeof(GprsParaFileType);
    s_Para2->crc = applied_math_calc_common_crc16((u8*)&s_Para2->magic, sizeof(GprsParaFileType)-8);  // 968
    
    
    ret = GM_FS_Write(handle, (void *)s_Para2, sizeof(GprsParaFileType), &fs_len);
    if (ret < 0)
    {
        LOG(INFO,"goome_file_write_minor_param write minor file fail [%d] \r\n", ret);
        
        GM_FS_Close(handle);
        return 0;
    }
    
    GM_FS_Close(handle);
        
    return 1;
}




/************************************************************************
    Function :
    Description :
    Parameter : 
    Return : 
    Author:  
    Date: Sep-18-2018
************************************************************************/
static u32 goome_file_read_minor_param(void)
{
    u32 fs_len, crc;
    int handle, ret;
    
    handle = GM_FS_Open(goome_para_minor, GM_FS_READ_ONLY | GM_FS_ATTR_ARCHIVE | GM_FS_CREATE);
    if (handle < 0)
    {
        LOG(INFO,"goome_file_read_minor_param open minor file read fail [%d] \r\n", handle);
        s_Para->para2_open_err++;
        s_Para2->para2_open_err++;
        
        return UOP_FAIL;
    }
    
    
    ret = GM_FS_Read(handle, (void *)s_Para2, sizeof(GprsParaFileType), &fs_len);
    if (ret < 0)
    {
        LOG(INFO,"goome_file_read_minor_param read minor file read fail [%d] \r\n", ret);
        s_Para->para2_open_err++;
        s_Para2->para2_open_err++;
        
        GM_FS_Close(handle);
        return UOP_FAIL;
    }
    
    GM_FS_Close(handle);
    
    crc = applied_math_calc_common_crc16((u8*)&s_Para2->magic, sizeof(GprsParaFileType)-8); // -18
    
    
    if (GM_strlen((const char* )s_Para2->sock[SOCKET_INDEX_MAIN].src_dns.dns) >= sizeof(s_Para2->sock[SOCKET_INDEX_MAIN].src_dns.dns)-1)
    {       
        GM_memset(s_Para2->sock[SOCKET_INDEX_MAIN].src_dns.dns, 0x00, sizeof(s_Para2->sock[SOCKET_INDEX_MAIN].src_dns.dns));
    }
    
    if (s_Para2->crc != crc)
    {
        LOG(INFO,"goome_file_read_minor_param read minor file read crc err:%x, %x ", s_Para2->crc, crc);
        s_Para->para2_crc_err++;
        s_Para2->para2_crc_err++;
        
        return UOP_CSM_ERR;
    }
    
    if (s_Para2->magic != MAGIC_NUMBER)
    {
        LOG(INFO,"goome_file_read_minor_param read minor file read magic err:%x, ", s_Para2->magic);
        s_Para->para2_mg_err++;
        s_Para2->para2_mg_err++;
        
        return UOP_MAGIC_ERR;
    }
        
    return UOP_OK;
}




/************************************************************************
    Function :
    Description : 
    Parameter : 
    Return : 
    Author:  
    Date: May-12-2016
************************************************************************/
static s32 goome_delete_file(u8 *file)
{
    s32  ret=0;
    s32  handle; 
    u8 buff[100];
    
    
    // 检查文件是否存在
    handle = GM_FS_CheckFile((u16 *)file);
    if (handle >= 0)
    {
        // 文件存在删掉,重新创建
        ret = GM_FS_Delete((u16 *)file);
        
        #if 1
        util_ucs2_to_ascii((u8*)file, buff, 160);
        
        LOG(INFO,"goome_delete_file deletfile:%.66s,ret:%d \r\n", buff,ret);
        #endif
    }
    
    
    return ret;
}


void goome_del_param_file(void)
{
    goome_delete_file((u8 *)goome_para_minor);
    goome_delete_file((u8 *)goome_para_main);
}

/************************************************************************
    Function :
    Description :  
    Parameter : 
    Return : 
    Author:  
    Date: Sep-18-2018
************************************************************************/
static u8 param_write_back_free_mem(void)
{
    s_Para->magic = MAGIC_NUMBER;
    s_Para->valid_flag = UOP_OK;
    s_Para->len = sizeof(GprsParaFileType);
    s_Para->crc = applied_math_calc_common_crc16((u8*)&s_Para->magic, sizeof(GprsParaFileType)-8);  // 968

    GM_memcpy(p_param_mem, s_Para, sizeof(GprsParaFileType));
    
    LOG(INFO,"param_write_back_free_mem param_write_mem:%x %d", s_Para->crc, sizeof(GprsParaFileType)-8);
    
    return UOP_OK;
}

/************************************************************************
    Function :
    Description :  
    Parameter : 
    Return : 
    Author:  
    Date: Sep-18-2018
************************************************************************/
static u8 param_read_back_free_mem(void)
{
    u32 crc;
    
    if (p_param_mem->magic != MAGIC_NUMBER)
    {
        LOG(INFO,"param_read_back_free_mem goome param file free mem magic error:%x ", p_param_mem->magic);
        return UOP_MAGIC_ERR;
    }
    
    crc = applied_math_calc_common_crc16((u8*)&p_param_mem->magic, sizeof(GprsParaFileType)-8); // -18
    if (crc != p_param_mem->crc)
    {
        LOG(INFO,"param_read_back_free_mem goome param file free mem Crc error:%x,%x %d", crc, p_param_mem->crc, sizeof(GprsParaFileType)-8);
        return UOP_CSM_ERR;
    }
    
    return UOP_OK;
}




/************************************************************************
    Function :
    Description : 读取系统参数,FreeMem,主参数文件,备份参数文件
    Parameter : 
    Return : 
    Author: 
    Date: Oct-10-2018
************************************************************************/
u8 read_param_from_file(void)
{
    u8 file1=0, file2=0, free_mem=0;
    
    GM_memset(s_Para, 0x00, sizeof(GprsParaFileType));
    GM_memset(s_Para2, 0x00, sizeof(GprsParaFileType));
    
    file1=0;
    file2=0;
    
    if (GM_FS_CheckFile((const U16*)goome_para_main) < 0)
    {
        LOG(INFO,"read_param_from_file no find para_main_file");
        
        s_Para->para_none++;
        s_Para2->para_none++;
        file1 = UOP_FAIL;
    }
    else
    {
        file1 = goome_file_param_main_read();
    }
    
    
    if (GM_FS_CheckFile((const U16*)goome_para_minor) < 0)
    {
        LOG(INFO,"read_param_from_file no find para_minor_file");
        
        s_Para->para2_none++;
        s_Para2->para2_none++;
        file2 = UOP_FAIL;
    }
    else
    {
        file2 = goome_file_read_minor_param();
    }
    
    free_mem = param_read_back_free_mem();
    if (free_mem == UOP_OK)
    {        
        if (file1 != UOP_OK)
        {
            GM_memcpy(s_Para, p_param_mem, sizeof(GprsParaFileType));
            
            s_Para->para_from_mem++;
            s_Para2->para_from_mem++;
            
            goome_delete_file((u8 *)goome_para_main);
            goome_file_param_main_write();
        }
        
        if (file2 != UOP_OK)
        {
            GM_memcpy(s_Para2, p_param_mem, sizeof(GprsParaFileType));
            
            s_Para->para2_from_mem++;
            s_Para2->para2_from_mem++;
            
            goome_delete_file((u8 *)goome_para_minor);
            goome_file_write_minor_param();
        }
    }
    else
    {
        if(system_state_get_boot_reason(false) == GM_RREBOOT_UNKNOWN)
        {
    	    system_state_set_boot_reason(GM_REBOOT_POWER_ON);
        }

        if ((file1 == UOP_OK) && (file2 != UOP_OK)) 
        {            
            GM_memcpy(s_Para2, s_Para, sizeof(GprsParaFileType));
            
            s_Para->para2_from_file++;
            s_Para2->para2_from_file++;
            
            goome_delete_file((u8 *)goome_para_minor);
            goome_file_write_minor_param();
            param_write_back_free_mem();
        }
        else if ((file1 != UOP_OK) && (file2 == UOP_OK)) 
        {
            GM_memcpy(s_Para, s_Para2, sizeof(GprsParaFileType));
            
            s_Para->para_from_file++;
            s_Para2->para_from_file++;
            
            goome_delete_file((u8 *)goome_para_main);
            goome_file_param_main_write();
            param_write_back_free_mem();
        }
        else if ((file1 != UOP_OK) && (file2 != UOP_OK))
        {
            s_Para->valid_flag = UOP_PARA_ERR;
        }
    }
    
    return 0;
}

static void goome_file_nvram_save(void)
{
    s32 lib_index = -1;
    s32 result = -1;

    if (NULL == s_nvram)
    {
        s_nvram = (p_nvram_ef_goome_param_struct )GM_MemoryAlloc(sizeof(nvram_ef_goome_param_struct));
        if (NULL == s_nvram)
        {
            LOG(ERROR,"goome_file_nvram_save assert(s_nvram) failed.");
            return;
        }
    }
    if (NULL == s_Para)
    {
        LOG(ERROR,"goome_file_nvram_save assert(s_Para) failed.");
        return;
    }

    if(GM_strlen((const char *)GOOME_DEV_NUMBER_DEFAULT) == config_service_get_length(CFG_DEVICE_NUMBER,TYPE_STRING))
    {
        GM_memcpy(s_nvram->data_u, config_service_get_pointer(CFG_DEVICE_NUMBER), config_service_get_length(CFG_DEVICE_NUMBER,TYPE_STRING));
    }
    else
    {
        config_service_set(CFG_DEVICE_NUMBER,TYPE_STRING,GOOME_DEV_NUMBER_DEFAULT,GM_strlen((const char *)GOOME_DEV_NUMBER_DEFAULT));
        GM_memcpy(s_nvram->data_u, config_service_get_pointer(CFG_DEVICE_NUMBER), config_service_get_length(CFG_DEVICE_NUMBER,TYPE_STRING));
    }
    
    lib_index = GM_ReadNvramLid(NVRAM_EF_GM_CUSTOMER_PARAM_LID);
    GM_ReadWriteNvram(0, lib_index, 1, s_nvram, sizeof(nvram_ef_goome_param_struct), &result);

    //只分配一次,后面一直使用
    //GM_MemoryFree(s_nvram);
    //s_nvram = NULL;
}


static void read_nvram_device_id(void)
{
    s32 lib_index = -1;
    s32 result = -1;

    if (NULL == s_nvram)
    {
        s_nvram = (p_nvram_ef_goome_param_struct)GM_MemoryAlloc(sizeof(nvram_ef_goome_param_struct));
        if (NULL == s_nvram)
        {
            LOG(ERROR,"goome_file_nvram_save assert(s_nvram) failed.");
            return;
        }
    }
    if (NULL == s_Para)
    {
        LOG(ERROR,"goome_file_nvram_save assert(s_Para) failed.");
        return;
    }

    lib_index = GM_ReadNvramLid(NVRAM_EF_GM_CUSTOMER_PARAM_LID);
    GM_ReadWriteNvram(1, lib_index, 1, s_nvram, sizeof(nvram_ef_goome_param_struct), &result);

    //必须是1打头的数字串, 比如13000000000
    if(s_nvram->data_u[0] != '1')
    {
        config_service_set(CFG_DEVICE_NUMBER,TYPE_STRING,GOOME_DEV_NUMBER_DEFAULT,GM_strlen((const char *)GOOME_DEV_NUMBER_DEFAULT));
    }
    else
    {
        config_service_set(CFG_DEVICE_NUMBER,TYPE_STRING,s_nvram->data_u,GM_strlen((const char *)GOOME_DEV_NUMBER_DEFAULT));
    }
    LOG(INFO,"DEV num %s",config_service_get_pointer(CFG_DEVICE_NUMBER));
    
}



/************************************************************************
    Function :
    Description :
    Parameter : 
    Return : 
    Author: 
    Date:  Sep-18-2018
************************************************************************/
static u8 goome_file_param_save(void)
{
    param_write_back_free_mem();
    goome_file_param_main_write();
    
    GM_memcpy(s_Para2, s_Para, sizeof(GprsParaFileType));
    goome_file_write_minor_param();
    goome_file_nvram_save();
    
    return 1;
}

GM_ERRCODE config_service_save_to_local(void)
{
    bool allocate_here1 = false;
    bool allocate_here2 = false;
    
    if(NULL == s_Para)
    {
        s_Para = (GprsParaFileType *)GM_MemoryAlloc(sizeof(GprsParaFileType));
        if(!s_Para)
        {
            LOG(ERROR,"config_service_read_from_local assert(s_Para) failed.");
            return GM_MEM_NOT_ENOUGH;
        }
        allocate_here1 = true;
    }
    
    if(NULL == s_Para2)
    {
        s_Para2 = (GprsParaFileType *)GM_MemoryAlloc(sizeof(GprsParaFileType));
        if(!s_Para2)
        {
            LOG(ERROR,"config_service_read_from_local assert(s_Para) failed.");
            GM_MemoryFree(s_Para);
            s_Para = NULL;
            return GM_MEM_NOT_ENOUGH;
        }
        allocate_here2 = true;
    }

    convert_cfg_to_para(s_Para);
    goome_file_param_save();

    if(allocate_here1)
    {
        GM_MemoryFree(s_Para);
        s_Para = NULL;
    }
    
    if(allocate_here2)
    {
        GM_MemoryFree(s_Para2);
        s_Para2 = NULL;
    }
	return GM_SUCCESS;
}


GM_ERRCODE config_service_read_from_local(void)
{
    if(!p_param_mem)
    {
        p_param_mem = (T_GprsParam)(GM_ImageDummyBase()-2*1024);
    }
    
    if(NULL == s_Para)
    {
        s_Para = (GprsParaFileType *)GM_MemoryAlloc(sizeof(GprsParaFileType));
        if(!s_Para)
        {
            LOG(ERROR,"config_service_read_from_local assert(s_Para) failed.");
            return GM_MEM_NOT_ENOUGH;
        }
    }
    
    if(NULL == s_Para2)
    {
        s_Para2 = (GprsParaFileType *)GM_MemoryAlloc(sizeof(GprsParaFileType));
        if(!s_Para2)
        {
            LOG(ERROR,"config_service_read_from_local assert(s_Para) failed.");
            GM_MemoryFree(s_Para);
            s_Para = NULL;
            return GM_MEM_NOT_ENOUGH;
        }
    }
    
	//有些新参数在旧设备上没有,要先设置为默认参数,从文件读取后再覆盖
    config_service_set_deault();
    read_nvram_device_id();
	read_param_from_file();
    
    if(UOP_PARA_ERR != s_Para->valid_flag && check_para_ok(s_Para))
    {
        convert_para_to_cfg(s_Para);
		LOG(INFO,"convert_para_to_cfg");
    }
    else
    {
        config_service_save_to_local();
		LOG(INFO,"config_service_save_to_local");
    }


    GM_MemoryFree(s_Para);
    s_Para = NULL;
    GM_MemoryFree(s_Para2);
    s_Para2 = NULL;
    return GM_SUCCESS;
}


/*
设置出厂时的默认参数
不包括 sim, socket, heart_mode, apn, protocol配置.
*/
static void config_service_set_factory_deault(void)
{
    u8 value_u8 = 0;
    s8 value_s8 = 0;
    u16 value_u16 = 0;
    //u32 value_u32 = 0;
    u8 arry_u8[CONFIG_STRING_MAX_LEN] = {0};

    ClockModeTimePointStruct *clock = NULL;
    //float value_float = 0;

    value_s8 = 8;
    config_service_set(CFG_TIME_ZONE, TYPE_BYTE, &value_s8, sizeof(value_s8));

    value_u8 = GM_GPS_TYPE_MTK_EPO;
    config_service_set(CFG_GPS_TYPE, TYPE_BYTE, &value_u8, sizeof(value_u8));

    value_u8 = ALARM_CLOCK_MODE;
    config_service_set(CFG_WORK_MODE,TYPE_BYTE,&value_u8 , sizeof(value_u8));

    clock = (ClockModeTimePointStruct *)arry_u8;

    clock->time_point[0] = 1230;
    clock->time_point[1] = (u32)(-1);
    clock->time_point[2] = (u32)(-1);
    clock->time_point[3] = (u32)(-1);

    //GM_memset(arry_u8,0,CONFIG_STRING_MAX_LEN);
    config_service_set(CFG_WORK_MODE_PARAM,TYPE_ARRY,arry_u8,WORK_MODE_PARAM_LEN);

    value_u8 = 5;
    config_service_set(CFG_GPS_UPDATE_TIME, TYPE_BYTE, &value_u8, sizeof(value_u8));

    value_u16 = 1500;
    config_service_set(CFG_TIMING_NUMBER,TYPE_SHORT , &value_u16,sizeof(value_u16));

    value_u16 = 0;
    config_service_set(CFG_MTK_PWRON_CNT,TYPE_SHORT , &value_u16,sizeof(value_u16));

    value_u16 = 0;
    config_service_set(CFG_EXPMU_PWRON_CNT,TYPE_SHORT , &value_u16,sizeof(value_u16));

    value_u8 = true;
    config_service_set(CFG_APN_CHECK, TYPE_BOOL, &value_u8, sizeof(value_u8));

    value_u8 = 1;
    config_service_set(CFG_WIFI_STATE,TYPE_BYTE,&value_u8,sizeof(value_u8));

    value_u8 = PLATFORM_LOC_MODE_INVAILD;
    config_service_set(CFG_POSITION_SOURCE,TYPE_BYTE,&value_u8,sizeof(value_u8));


    value_u16 = 1;
    config_service_set(CFG_PWR_REP_CNT,TYPE_SHORT,&value_u16,sizeof(value_u16));


    value_u16 = 120;
    config_service_set(CFG_REP_GAP,TYPE_SHORT,&value_u16 ,sizeof(value_u16));

    value_u8 = 1;
    config_service_set(CFG_LIGHT_STATE,TYPE_BYTE,&value_u8,sizeof(value_u8));

    value_u8 = 16;
    config_service_set(CFG_MIN_SNR, TYPE_BYTE, &value_u8, sizeof(value_u8));

	
    value_u8 = 30;
    config_service_set(CFG_REOPEN_GSP_TIME, TYPE_BYTE, &value_u8, sizeof(value_u8));

    value_u8 = false;
    config_service_set(CFG_SMOOTH_TRACK, TYPE_BOOL, &value_u8, sizeof(value_u8));

    value_u16 = 30;
    config_service_set(CFG_UPLOADTIME, TYPE_SHORT, &value_u16, sizeof(value_u16));
    

    config_service_set(CFG_TERM_VERSION, TYPE_STRING, VERSION_NUMBER, GM_strlen(VERSION_NUMBER));
    LOG(INFO,"current version(%s).",VERSION_NUMBER);
    
    config_service_set(CFG_CUSTOM_CODE, TYPE_STRING,UPDATE_OEM_CODE, GM_strlen(UPDATE_OEM_CODE));
    config_service_set(CFG_TERM_MODEL, TYPE_STRING, UPDATE_DEVICE_CODE, GM_strlen(UPDATE_DEVICE_CODE));
    config_service_set(CFG_TERM_BOOT_CHECK, TYPE_STRING,UPDATE_BOOT_CODE, GM_strlen(UPDATE_BOOT_CODE));

    config_service_set_test_mode(false);
    
}




/*
设置 默认参数
包括 sim, socket, heart_mode, apn, protocol配置.
*/
static void config_service_set_deault(void)
{
    //u8 value_u8;

    
    //u16 value_u16;
    //u32 value_u32;
    //float value_float;

    config_service_set_factory_deault();
    
    config_service_set(CFG_SERVERADDR, TYPE_STRING, CONFIG_GOOCAR_SERVER_ADDERSS, GM_strlen(CONFIG_GOOCAR_SERVER_ADDERSS));
    config_service_set(CFG_CFGSERVERADDR, TYPE_STRING, CONFIG_SERVER_ADDERSS, GM_strlen(CONFIG_SERVER_ADDERSS));
    config_service_set(CFG_TEST_SERVERADDR, TYPE_STRING, CONFIG_FACTORY_SERVER_ADDERSS, GM_strlen(CONFIG_FACTORY_SERVER_ADDERSS));
    config_service_set(CFG_AGPSSERVERADDR, TYPE_STRING, CONFIG_AGPS_SERVER_ADDERSS, GM_strlen(CONFIG_AGPS_SERVER_ADDERSS));
    config_service_set(CFG_LOGSERVERADDR, TYPE_STRING, CONFIG_LOG_SERVER_ADDERSS, GM_strlen(CONFIG_LOG_SERVER_ADDERSS));
    config_service_set(CFG_UPDATESERVERADDR, TYPE_STRING, GOOME_UPDATE_SERVER_DNS, GM_strlen(GOOME_UPDATE_SERVER_DNS));
    config_service_set(CFG_UPDATEFILESERVER, TYPE_STRING, GOOME_UPDATE_SERVER_DNS, GM_strlen(GOOME_UPDATE_SERVER_DNS));

    
    config_service_set(CFG_APN_NAME, TYPE_STRING, APN_DEFAULT, GM_strlen((const char *)APN_DEFAULT));
    config_service_set(CFG_APN_USER, TYPE_STRING, APN_USER_DEFAULT, GM_strlen((const char *)APN_USER_DEFAULT));
    config_service_set(CFG_APN_PWD, TYPE_STRING, APN_USER_DEFAULT, GM_strlen((const char *)APN_USER_DEFAULT));
    config_service_set(CFG_SMS_PWD,TYPE_STRING,DEFAULT_SMS_PWD,GM_strlen((const char *)DEFAULT_SMS_PWD));
    
    config_service_set(CFG_DEV_TYPE,TYPE_STRING,DEFAULT_DEVICE_TYPE,GM_strlen((const char *)DEFAULT_DEVICE_TYPE));


}






void config_service_restore_factory_config(bool is_all)
{

    config_service_set_deault();


    config_service_set_factory_deault();

 
    config_service_save_to_local();
}




void config_service_set_test_mode(bool state)
{
	config_service_set(CFG_IS_TEST_MODE, TYPE_BOOL, &state, sizeof(state));
}

bool config_service_is_test_mode(void)
{
    bool is_test_mode = false;
	config_service_get(CFG_IS_TEST_MODE, TYPE_BOOL, &is_test_mode, sizeof(is_test_mode));
    return is_test_mode;
}

bool config_service_is_default_imei(void)
{
    GM_ERRCODE ret = GM_SUCCESS;
    u8 imei[GM_IMEI_LEN + 1] = {0};
    if(GM_SUCCESS != (ret = gsm_get_imei(imei)))
    {
        LOG(INFO,"clock(%d) config_service_is_default_imei can not get imei, ret:%d.", util_clock(), ret);
        return false;
    }
    if (0 == GM_strcmp((const char *)imei, (const char *)GOOME_IMEI_DEFAULT))
    {
        LOG(INFO,"clock(%d) config_service_is_default_imei(%s) = true.", util_clock(), imei);
        return true;
    }
	LOG(INFO,"clock(%d) config_service_is_default_imei(%s) = false.", util_clock(), imei);
    return false;
}

static void convert_cfg_to_para(GprsParaFileType *para)
{
    u8 value_u8;
    char value_char = 0;
    u16 value_u16;
    u32 value_u32;
    //float value_float;
    u8 value_str[CONFIG_STRING_MAX_LEN];
    s32 idx;

    GM_memset(para,0, sizeof(GprsParaFileType));
    para->len = sizeof(GprsParaFileType);
    para->magic = MAGIC_NUMBER;
    para->valid_flag = UOP_OK;
    

    para->sock[SOCKET_INDEX_MAIN].sock_type = STREAM_TYPE_DGRAM;
    GM_memset(value_str, 0x00, sizeof(value_str));
    idx = GM_sscanf((const char*)config_service_get_pointer(CFG_SERVERADDR), "%[^:]:%d", value_str, &value_u32);
    if (idx != 2 || GM_strlen((const char*)value_str) >= sizeof(para->sock[SOCKET_INDEX_MAIN].src_dns.dns) || value_u32>65535)
    {
        LOG(WARN,"convert_cfg_to_para assert(idx ==2) of CFG_SERVERADDR failed.");
        return;
    }
    GM_strcpy((char*)para->sock[SOCKET_INDEX_MAIN].src_dns.dns, (const char*)value_str);
    para->sock[SOCKET_INDEX_MAIN].src_dns.port = value_u32;

    para->sock[SOCKET_INDEX_AGPS].sock_type = STREAM_TYPE_DGRAM;
    GM_memset(value_str, 0x00, sizeof(value_str));
    idx = GM_sscanf((const char*)config_service_get_pointer(CFG_AGPSSERVERADDR), "%[^:]:%d", value_str, &value_u32);
    if (idx != 2 || GM_strlen((const char*)value_str) >= sizeof(para->sock[SOCKET_INDEX_AGPS].src_dns.dns) || value_u32>65535)
    {
        LOG(WARN,"convert_cfg_to_para assert(idx ==2) of CFG_AGPSSERVERADDR failed.");
        return;
    }
    GM_strcpy((char*)para->sock[SOCKET_INDEX_AGPS].src_dns.dns, (const char*)value_str);
    para->sock[SOCKET_INDEX_AGPS].src_dns.port = value_u32;
    
    para->sock[SOCKET_INDEX_LOG].sock_type = STREAM_TYPE_DGRAM;
    GM_memset(value_str, 0x00, sizeof(value_str));
    idx = GM_sscanf((const char*)config_service_get_pointer(CFG_LOGSERVERADDR), "%[^:]:%d", value_str, &value_u32);
    if (idx != 2 || GM_strlen((const char*)value_str) >= sizeof(para->sock[SOCKET_INDEX_LOG].src_dns.dns) || value_u32>65535)
    {
        LOG(WARN,"convert_cfg_to_para assert(idx ==2) of CFG_LOGSERVERADDR failed.");
        return;
    }
    GM_strcpy((char*)para->sock[SOCKET_INDEX_LOG].src_dns.dns, (const char*)value_str);
    para->sock[SOCKET_INDEX_LOG].src_dns.port = value_u32;
    
    para->sock[SOCKET_INDEX_UPDATE].sock_type = STREAM_TYPE_DGRAM;
    GM_memset(value_str, 0x00, sizeof(value_str));
    idx = GM_sscanf((const char*)config_service_get_pointer(CFG_UPDATESERVERADDR), "%[^:]:%d", value_str, &value_u32);
    if (idx != 2 || GM_strlen((const char*)value_str) >= sizeof(para->sock[SOCKET_INDEX_UPDATE].src_dns.dns) || value_u32>65535)
    {
        LOG(WARN,"convert_cfg_to_para assert(idx ==2) of CFG_UPDATESERVERADDR failed.");
        return;
    }
    GM_strcpy((char*)para->sock[SOCKET_INDEX_UPDATE].src_dns.dns, (const char*)value_str);
    para->sock[SOCKET_INDEX_UPDATE].src_dns.port = value_u32;
    
    para->sock[SOCKET_INDEX_CONFIG].sock_type = STREAM_TYPE_DGRAM;
    GM_memset(value_str, 0x00, sizeof(value_str));
    idx = GM_sscanf((const char*)config_service_get_pointer(CFG_CFGSERVERADDR), "%[^:]:%d", value_str, &value_u32);
    if (idx != 2 || GM_strlen((const char*)value_str) >= sizeof(para->sock[SOCKET_INDEX_CONFIG].src_dns.dns) || value_u32>65535)
    {
        LOG(WARN,"convert_cfg_to_para assert(idx ==2) of CFG_CFGSERVERADDR failed.");
        return;
    }
    GM_strcpy((char*)para->sock[SOCKET_INDEX_CONFIG].src_dns.dns, (const char*)value_str);
    para->sock[SOCKET_INDEX_CONFIG].src_dns.port = value_u32;
    
    config_service_get(CFG_APN_NAME, TYPE_STRING, value_str, sizeof(value_str));
    GM_strcpy((char *)para->Apn,(const char*)value_str);
    
    config_service_get(CFG_APN_USER, TYPE_STRING, value_str, sizeof(value_str));
    GM_strcpy((char *)para->strUser,(const char*)value_str);

    config_service_get(CFG_APN_PWD, TYPE_STRING, value_str, sizeof(value_str));
    GM_strcpy((char *)para->strPwd,(const char*)value_str);

    config_service_get(CFG_DEVICE_NUMBER, TYPE_STRING,  value_str, sizeof(value_str));
    GM_memset(para->strSim,0,sizeof(para->strSim));
    GM_strcpy((char *)para->strSim,(const char*)value_str);

    config_service_get(CFG_WORK_MODE,TYPE_BYTE,&value_u8,sizeof(value_u8));
    para->work_mode = value_u8;

    config_service_get(CFG_WORK_MODE_PARAM,TYPE_ARRY,(u8 *)para->mod_param,WORK_MODE_PARAM_LEN);

    config_service_get(CFG_TIMING_NUMBER,TYPE_SHORT , &value_u16,sizeof(value_u16));
    para->remaining_number = value_u16;


    config_service_get(CFG_UPLOADTIME, TYPE_SHORT, &value_u16, sizeof(value_u16));
    para->UploadTim = value_u16;


    config_service_get(CFG_GPS_UPDATE_TIME, TYPE_BYTE, &value_u8, sizeof(value_u8));
    para->GpsUpdatTime = value_u8;

    config_service_get(CFG_APN_CHECK, TYPE_BOOL, &value_u8, sizeof(value_u8));
    para->auto_set_apn = value_u8;

    config_service_get(CFG_TIME_ZONE, TYPE_BYTE, &value_char, sizeof(value_char));
    para->LocalTime = value_char;

    config_service_get(CFG_WIFI_STATE,TYPE_BYTE,&value_u8,sizeof(value_u8));
    para->wifi_switch = value_u8;

    config_service_get(CFG_POSITION_SOURCE,TYPE_BYTE,&value_u8,sizeof(value_u8));
    para->position_source = value_u8;

    config_service_get(CFG_PWR_REP_CNT,TYPE_SHORT,&value_u16,sizeof(value_u16));
    para->pwron_rep_cnt = value_u16;

    config_service_get(CFG_REP_GAP,TYPE_SHORT,&value_u16 ,sizeof(value_u16));
    para->no_signal_next_rep_gap = value_u16;

    config_service_get(CFG_LIGHT_STATE,TYPE_BYTE,&value_u8,sizeof(value_u8));
    para->light_switch = value_u8;

	
    config_service_get(CFG_REOPEN_GSP_TIME, TYPE_BYTE, &value_u8, sizeof(value_u8));
    para->reopen_gps_time = value_u8;

    config_service_get(CFG_MIN_SNR, TYPE_BYTE, &value_u8, sizeof(value_u8));
    para->min_snr = value_u8;

    config_service_get(CFG_SMS_PWD, TYPE_STRING,  para->sms_pwd, sizeof(para->sms_pwd));

    config_service_get(CFG_DEV_TYPE, TYPE_STRING,  para->dvice_type, sizeof(para->dvice_type));


}



static void convert_para_to_cfg(const GprsParaFileType *para)
{
    u8 value_u8;
    s8 value_s8 = 0;
    u16 value_u16;
    //u32 value_u32;
    //float value_float;
    u8 value_str[CONFIG_STRING_MAX_LEN];
    

    GM_snprintf((char*)value_str,sizeof(value_str), "%s:%d",(char*)para->sock[SOCKET_INDEX_MAIN].src_dns.dns, para->sock[SOCKET_INDEX_MAIN].src_dns.port);
    config_service_set(CFG_SERVERADDR, TYPE_STRING, &value_str, GM_strlen((char*)value_str));

    GM_snprintf((char*)value_str,sizeof(value_str), "%s:%d",(char*)para->sock[SOCKET_INDEX_AGPS].src_dns.dns, para->sock[SOCKET_INDEX_AGPS].src_dns.port);
    config_service_set(CFG_AGPSSERVERADDR, TYPE_STRING, &value_str, GM_strlen((char*)value_str));

    GM_snprintf((char*)value_str,sizeof(value_str), "%s:%d",(char*)para->sock[SOCKET_INDEX_LOG].src_dns.dns, para->sock[SOCKET_INDEX_LOG].src_dns.port);
    config_service_set(CFG_LOGSERVERADDR, TYPE_STRING, &value_str, GM_strlen((char*)value_str));
    
    GM_snprintf((char*)value_str,sizeof(value_str), "%s:%d",(char*)para->sock[SOCKET_INDEX_UPDATE].src_dns.dns, para->sock[SOCKET_INDEX_UPDATE].src_dns.port);
    config_service_set(CFG_UPDATESERVERADDR, TYPE_STRING, &value_str, GM_strlen((char*)value_str));

    GM_snprintf((char*)value_str,sizeof(value_str), "%s:%d",(char*)para->sock[SOCKET_INDEX_CONFIG].src_dns.dns, para->sock[SOCKET_INDEX_CONFIG].src_dns.port);
    config_service_set(CFG_CFGSERVERADDR, TYPE_STRING, &value_str, GM_strlen((char*)value_str));
    

    config_service_set(CFG_APN_NAME, TYPE_STRING, para->Apn, GM_strlen((const char*)para->Apn));
    config_service_set(CFG_APN_USER, TYPE_STRING, para->strUser, GM_strlen((const char*)para->strUser));
    config_service_set(CFG_APN_PWD, TYPE_STRING,  para->strPwd, GM_strlen((const char*)para->strPwd));

    value_u8 = para->work_mode;
    config_service_set(CFG_WORK_MODE,TYPE_BYTE,&value_u8,sizeof(value_u8));
   
    
    config_service_set(CFG_WORK_MODE_PARAM,TYPE_ARRY,para->mod_param,WORK_MODE_PARAM_LEN);
    LOG(INFO,"MOD_PARAM %x %x %x %x = %x",(*(u32 *)(para->mod_param)),(*(u32 *)(para->mod_param + 4)),(*(u32 *)(para->mod_param + 8)),(*(u32 *)(para->mod_param + 12)),sizeof(WorkModeParamUnion));
    
    value_u16 = para->UploadTim;
    config_service_set(CFG_UPLOADTIME, TYPE_SHORT, &value_u16, sizeof(value_u16));


    value_u8 = para->GpsUpdatTime;
    config_service_set(CFG_GPS_UPDATE_TIME, TYPE_BYTE, &value_u8, sizeof(value_u8));
    

    value_u16 = para->remaining_number;
    config_service_set(CFG_TIMING_NUMBER,TYPE_SHORT , &value_u16,sizeof(value_u16));
    

    value_s8 = para->LocalTime;
    config_service_set(CFG_TIME_ZONE, TYPE_BYTE, &value_s8, sizeof(value_s8));



    value_u8 = para->gps_type;
    config_service_set(CFG_GPS_TYPE, TYPE_BYTE, &value_u8, sizeof(value_u8));

    
    value_u8 = para->auto_set_apn;
    config_service_set(CFG_APN_CHECK, TYPE_BOOL, &value_u8, sizeof(value_u8));


    value_u8 = para->wifi_switch;
    config_service_set(CFG_WIFI_STATE,TYPE_BYTE,&value_u8,sizeof(value_u8));

    value_u8 = para->position_source;
    config_service_set(CFG_POSITION_SOURCE,TYPE_BYTE,&value_u8,sizeof(value_u8));


    value_u16 = para->pwron_rep_cnt;
    config_service_set(CFG_PWR_REP_CNT,TYPE_SHORT,&value_u16,sizeof(value_u16));


    value_u16 = para->no_signal_next_rep_gap;
    config_service_set(CFG_REP_GAP,TYPE_SHORT,&value_u16 ,sizeof(value_u16));

    
    value_u8 = para->light_switch;
    config_service_set(CFG_LIGHT_STATE,TYPE_BYTE,&value_u8,sizeof(value_u8));
    
    config_service_set(CFG_SMS_PWD, TYPE_STRING,  para->sms_pwd, sizeof(para->sms_pwd));

	
    value_u8 = para->reopen_gps_time;
    config_service_set(CFG_REOPEN_GSP_TIME, TYPE_BYTE, &value_u8, sizeof(value_u8));


    config_service_set(CFG_DEV_TYPE, TYPE_STRING,  para->dvice_type, sizeof(para->dvice_type));


}


static bool check_address_para_ok(u8 * param)
{
    u8 addr[2*GOOME_DNS_MAX_LENTH];
    u8 IP[4];
    u32 port = 0;
    u8 idx = 0;

    GM_memset(addr, 0x00, sizeof(addr));
    idx = GM_sscanf((const char*)param, "%[^:]:%d", addr, &port);
    if (idx != 2)
    {
        LOG(INFO,"check_address_para_ok assert(2 fields(%s)) failed.", param);
        return false;
    }
    
    if(GM_SUCCESS != GM_ConvertIpAddr(addr, IP))
    {
        if(! util_is_valid_dns(addr, GM_strlen((const char *)addr)))
        {
            LOG(WARN,"check_address_para_ok assert(dns(%s)) failed.", addr);
            return false;
        }
    }
	return true;
}

static bool check_para_ok(const GprsParaFileType *para)
{
    s8 value_s8 = 0;
    u8 value_str[CONFIG_STRING_MAX_LEN];
   

    GM_snprintf((char*)value_str,sizeof(value_str), "%s:%d",(char*)para->sock[SOCKET_INDEX_MAIN].src_dns.dns, para->sock[SOCKET_INDEX_MAIN].src_dns.port);
    if(! check_address_para_ok(value_str))
    {
        LOG(WARN,"check_para_ok assert(CFG_SERVERADDR(%s)) failed.", value_str);
        return false;
    }
    
    GM_snprintf((char*)value_str,sizeof(value_str), "%s:%d",(char*)para->sock[SOCKET_INDEX_AGPS].src_dns.dns, para->sock[SOCKET_INDEX_AGPS].src_dns.port);
    if(! check_address_para_ok(value_str))
    {
        LOG(WARN,"check_para_ok assert(CFG_AGPSSERVERADDR(%s)) failed.", value_str);
        return false;
    }
    
    GM_snprintf((char*)value_str,sizeof(value_str), "%s:%d",(char*)para->sock[SOCKET_INDEX_LOG].src_dns.dns, para->sock[SOCKET_INDEX_LOG].src_dns.port);
    if(! check_address_para_ok(value_str))
    {
        LOG(WARN,"check_para_ok assert(CFG_LOGSERVERADDR(%s)) failed.", value_str);
        return false;
    }
    
    GM_snprintf((char*)value_str,sizeof(value_str), "%s:%d",(char*)para->sock[SOCKET_INDEX_UPDATE].src_dns.dns, para->sock[SOCKET_INDEX_UPDATE].src_dns.port);
    if(! check_address_para_ok(value_str))
    {
        LOG(WARN,"check_para_ok assert(CFG_UPDATESERVERADDR(%s)) failed.", value_str);
        return false;
    }
    
    GM_snprintf((char*)value_str,sizeof(value_str), "%s:%d",(char*)para->sock[SOCKET_INDEX_CONFIG].src_dns.dns, para->sock[SOCKET_INDEX_CONFIG].src_dns.port);
    if(! check_address_para_ok(value_str))
    {
        LOG(WARN,"check_para_ok assert(CFG_CFGSERVERADDR(%s)) failed.", value_str);
        return false;
    }
    
    if(0 == para->Apn[0])
    {
        LOG(WARN,"check_para_ok assert(CFG_APN(%s)) failed.", para->Apn);
        return false;
    }


    value_s8 = para->LocalTime;
    if(value_s8 >= 12 && value_s8 <= -12)
    {
        LOG(WARN,"check_para_ok assert(CFG_TIME_ZONE(%d)) failed.", value_s8);
        return false;
    }


    return true;
}




