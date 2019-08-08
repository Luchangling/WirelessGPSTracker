/*

*/
#include "expmu.h"
#include "gm_memory.h"
#include "gm_stdlib.h"
#include "utility.h"
#include "log_service.h"
#include "config_service.h"
#include "stdlib.h"
#include "uart.h"
#include "expmu_service.h"
#include "system_state.h"

#define MAX_SEND_BUF_LEN 200
#define SLAVE_SHARE_MEM_COMM_MAX_LEN 120
#define MAX_SEND_QUE_LEN 20
#define MAX_FAULT_TOLERANCE 6


typedef struct
{
	u16 len;

	u8 retry;

	u8 buf[SLAVE_SHARE_MEM_COMM_MAX_LEN];
	
}Smsp_Comm_Que_Struct;

typedef struct
{
	u8 mark;
	
	Smsp_Comm_Que_Struct que[MAX_SEND_QUE_LEN];

	u8 in;

	u8 out;

    u32 count;
	
}Send_Smsp_Control_Struct;

enum
{
	RD_PACKET,  
	WR_PACKET, 
	QE_PACKET, 
};

#pragma pack(1)


Expmu_Control_Reg_Struct g_mcu_evt;

char g_mcu_version[30] = {0};

char g_mcu_build_date[30] = {0};

char g_mcu_build_time[30] = {0};

Expmu_Gpio_TypeDef g_gpio[EXPMU_GPIOD] = {0};

Expmu_Fram_Para_Struct g_mcu_para;

Expmu_Reg_Mapping_Struct g_smsp_reg_map[EXPMU_END_REG];

typedef enum 
{
    EXPMU_STATE_INIT,

    EXPMU_STATE_QUREY,

    EXPMU_STATE_WORK_PROC,

    EXPMU_STATE_FINISH,

    EXPMU_STATE_ERROR,
    
}Expmu_State;

static char s_wkp_reason_string[EVENT_INVALID + 1][30] = {
                "MCU重新上电",
                "未知1",
                "未知2",
                "未知3",
                "RTC唤醒",
                "光感触发",
                "外部中断2唤醒",
                "未知3",
                "WatchDogReset",
};

static char s_work_mode_str[WORK_MODE_INVALID][11] = {
                "Unknow0",
                "AlarmClock",
                "Unknow1",
                "TrackerMod",
                "WeekAlarm",
                "PlatLoop",
                "PlatTime"
};

typedef struct
{
    Expmu_State status;

    u8 opration;

    u8 fail_cnt;

    u32 read_bit_map;
    
    u32 start_time;

    u8 read_reg_rate;

    u32 smsp_comm_err;

    u32 expmu_rtc_time;

    u8 expmu_comm_err;

    u8 is_sleep;
}Expmu_Struct;


static Expmu_Struct s_expmu;



typedef struct
{
	u8 lenH;

	u8 lenL;

	u8  mark;

	u8  type;

	u8  block[1];
	
}Smsp_Header_Struct; /*SMSP mean Slave Memory Share Protocol*/


typedef struct
{
	u16 len;

	u32 addr;

	u8  data[1];
	
}Smsp_Block_Struct;

#pragma pack()

typedef struct
{
	u32 addr;
	
	u16 len;
	
}Slave_Register_Map_Struct;

Send_Smsp_Control_Struct g_mcu_send;

static void expmu_init_proc(void);
static void expmu_read_register(void);
static void expmu_query_proc(void);
static void expmu_work_proc(void);
static void delete_smsp_stack_que(u8 mark);
static void push_smsp_packet_to_uart(void);
static void send_query_packet_to_expmu(void);
static u8 smsp_query_packet_ack_result(void);
static void send_read_reg_packet_to_uart(u8 *data , u16 len);
static u16 creat_write_pmu_register_block(u32 reg_addr , u8 offset , u8 *data , u8  len, u8* out);
static void show_smsp_time(u8 *head,u8 *bcdtime ,u8 *week);
static u8 sync_expmu_mpu_time_process(void);



u8 is_expmu_power_on(void)
{
	return (g_mcu_evt.wkp_reason == EVENT_POWER_UP?1:0);
}

u8 get_expmu_wkp_reason(void)
{
    return g_mcu_evt.wkp_reason;
}

#define MAX_EXPMU_QUERY_TIME 2

static expmu_transfer_status(Expmu_State new_state)
{
    u8 ret = 0;

    switch(s_expmu.status)
    {
       case EXPMU_STATE_INIT:

           switch(new_state)
            {
               case EXPMU_STATE_INIT:
               ret = 1;
               break;

               case EXPMU_STATE_QUREY:
               ret = 1;
               break;

               case EXPMU_STATE_WORK_PROC:
               
               break;

               case EXPMU_STATE_FINISH:

               break;

               case EXPMU_STATE_ERROR:
               ret = 1;
               break;

               default:
               break;
            }
       
       break;

       case EXPMU_STATE_QUREY:

           switch(new_state)
            {
               case EXPMU_STATE_INIT:

               break;

               case EXPMU_STATE_QUREY:
               
               break;

               case EXPMU_STATE_WORK_PROC:
               ret = 1;
               break;

               case EXPMU_STATE_FINISH:

               break;

               case EXPMU_STATE_ERROR:
               ret = 1;
               break;
               default:
               break;
            }
       
       break;

       case EXPMU_STATE_WORK_PROC:
            switch(new_state)
            {
               case EXPMU_STATE_INIT:
               
               break;

               case EXPMU_STATE_QUREY:

               break;

               case EXPMU_STATE_WORK_PROC:

               break;

               case EXPMU_STATE_FINISH:
               ret = 1;
               break;

               case EXPMU_STATE_ERROR:
               ret = 1;
               break;
               default:
               break;
            }

       break;

       case EXPMU_STATE_FINISH:

           switch(new_state)
            {
               case EXPMU_STATE_INIT:
               ret = 1;
               break;

               case EXPMU_STATE_QUREY:
              
               break;

               case EXPMU_STATE_WORK_PROC:
              
               break;

               case EXPMU_STATE_FINISH:
               
               break;
               case EXPMU_STATE_ERROR:
               ret = 1;
               break;
               default:
               break;
            }

       break;
    }
    
    if(ret == 1)
    {
        s_expmu.status = new_state;
    }
}

void printf_hex_string(char *head , u8 *hex , u16 len)
{
	char* temp = NULL;

	u16 i,j = 0;

	temp = GM_MemoryAlloc(1500);

	if(temp != NULL)
	{
		j = GM_sprintf(temp,"%s,%d->",head,util_clock());

		for(i = 0 ; i < len ; i++)
		{
			j += GM_sprintf(&temp[j],"%02x",hex[i]);

			if(j >= 1498)break;
		}

		LOG(INFO,temp);
	}
	
	GM_MemoryFree(temp);
}

u8 check_arry_data_xor(u8 *data , u16 len)
{
	u16 i = 0;
	
	u8 xor = 0;

	for(i = 0 ; i < len ; i++)
	{
		xor ^= data[i];
	}

	return xor;
}

u8 *get_expmu_next_wkp_time_register(void)
{
    return g_mcu_evt.next_tim;
}

u8 does_expmu_in_sleeping(void)
{
    return g_mcu_evt.wait_to_sleep;
}

void expmu_create(void)
{
	//u8 i = 0;
	
	GM_memset((u8 *)g_smsp_reg_map,0,sizeof(g_smsp_reg_map));

	//g_mcu_evt = (system_event_control_struct *)GM_MemoryAlloc(sizeof(system_event_control_struct));

	g_smsp_reg_map[EXPMU_EVT_REG].mpu_addr = (u32)&g_mcu_evt;

    g_smsp_reg_map[EXPMU_EVT_REG].reg_size = sizeof(g_mcu_evt);

	//g_mcu_version = (u8 *)GM_MemoryAlloc(30);

	//GM_memset((u8 *)g_mcu_version, 0, 30);

	g_smsp_reg_map[EXPMU_VER_REG].mpu_addr = (u32)g_mcu_version;

    g_smsp_reg_map[EXPMU_VER_REG].reg_size = sizeof(g_mcu_version);

	//g_mcu_build_date = (u8 *)GM_MemoryAlloc(30);

	//GM_memset((u8 *)g_mcu_build_date, 0, 30);

	g_smsp_reg_map[EXPMU_DATE_REG].mpu_addr = (u32)g_mcu_build_date;

    g_smsp_reg_map[EXPMU_DATE_REG].reg_size = sizeof(g_mcu_build_date);

	//g_mcu_build_time = (u8 *)GM_MemoryAlloc(30);

	//GM_memset((u8 *)g_mcu_build_time, 0, 30);

	g_smsp_reg_map[EXPMU_TIME_REG].mpu_addr = (u32)g_mcu_build_time;

    g_smsp_reg_map[EXPMU_TIME_REG].reg_size = sizeof(g_mcu_build_time);

	//g_mcu_para = (Fram_Para_Struct *)GM_MemoryAlloc(sizeof(Fram_Para_Struct));

	g_smsp_reg_map[EXPMU_PARA_REG].mpu_addr = (u32)&g_mcu_para;

    g_smsp_reg_map[EXPMU_PARA_REG].reg_size = sizeof(g_mcu_para);

	//g_gpio[EXPMU_GPIOA] = (GPIO_TypeDef *)GM_MemoryAlloc(sizeof(GPIO_TypeDef));

	g_smsp_reg_map[EXPMU_GPIOA_REG].mpu_addr = (u32)&g_gpio[EXPMU_GPIOA];

    g_smsp_reg_map[EXPMU_GPIOA_REG].reg_size = sizeof(Expmu_Gpio_TypeDef);

	//g_gpio[EXPMU_GPIOB] = (GPIO_TypeDef *)GM_MemoryAlloc(sizeof(GPIO_TypeDef));

	g_smsp_reg_map[EXPMU_GPIOB_REG].mpu_addr = (u32)&g_gpio[EXPMU_GPIOB];

    g_smsp_reg_map[EXPMU_GPIOB_REG].reg_size = sizeof(Expmu_Gpio_TypeDef);

	//g_gpio[EXPMU_GPIOC] = (GPIO_TypeDef *)GM_MemoryAlloc(sizeof(GPIO_TypeDef));

	g_smsp_reg_map[EXPMU_GPIOC_REG].mpu_addr = (u32)&g_gpio[EXPMU_GPIOC];

    g_smsp_reg_map[EXPMU_GPIOC_REG].reg_size = sizeof(Expmu_Gpio_TypeDef);

	//g_gpio[EXPMU_GPIOD] = (GPIO_TypeDef *)GM_MemoryAlloc(sizeof(GPIO_TypeDef));

	g_smsp_reg_map[EXPMU_GPIOD_REG].mpu_addr = (u32)&g_gpio[EXPMU_GPIOD];

    g_smsp_reg_map[EXPMU_GPIOD_REG].reg_size = sizeof(Expmu_Gpio_TypeDef);

    expmu_transfer_status(EXPMU_STATE_INIT);

    GM_memset((u8 *)&s_expmu,0,sizeof(s_expmu));

    //s_expmu.push_data_rate = 30; //300ms

    s_expmu.read_reg_rate = 3;  //3s

    s_expmu.is_sleep = 0;

}

void expmu_enter_sleep(void)
{
    expmu_clear_smap_stack();
    s_expmu.is_sleep = 1;
}

void expmu_exit_sleep(void)
{
    s_expmu.is_sleep = 0;
}


#define READ_PMU_REG_RATE 50
#define PUSH_PACKET_RATE  50
#define CALIBRA_TIM_RATE  10

void expmu_timer_proc(void)
{
    static u16 push_packet_rate = 0;

    if(s_expmu.is_sleep)return;
    
    switch(s_expmu.status)
    {
        case EXPMU_STATE_INIT:
        {
            expmu_init_proc();
        }
        break;

        case EXPMU_STATE_QUREY:
        {
            expmu_query_proc();
        }
        break;

        case EXPMU_STATE_WORK_PROC:
        {
            expmu_work_proc();
        }
        break;

        case EXPMU_STATE_FINISH:
        {
            
        }
        break;
        case EXPMU_STATE_ERROR:
        {
            //report logs 
            static bool reported = false;
            if(!reported)
            {
                JsonObject* p_json_log = json_create();
                json_add_string(p_json_log,"event","start failed.");
                log_service_upload(INFO, p_json_log);
                reported = true;
            }
        }
        break;
        default:
        break;
    }

    if(push_packet_rate == 0)
    {
        push_packet_rate = PUSH_PACKET_RATE;
        
        push_smsp_packet_to_uart();
    }

    push_packet_rate--;

}

static void expmu_init_proc(void)
{
    u8 i= 0;
    u32 cur_time = util_clock();


    if(cur_time - s_expmu.start_time >= MAX_EXPMU_QUERY_TIME)
    {
        s_expmu.fail_cnt++;

        if(s_expmu.fail_cnt <= MAX_FAULT_TOLERANCE)
        {
            s_expmu.start_time = util_clock();
        
            send_query_packet_to_expmu();
        }
        else
        {
            
            system_state_set_expmu_smsp_comm_cannot_establish();
            
            expmu_transfer_status(EXPMU_STATE_ERROR);
        }

    }
    else
    {
        if(smsp_query_packet_ack_result())
        {
            expmu_transfer_status(EXPMU_STATE_QUREY);

            s_expmu.start_time = util_clock();

            s_expmu.read_bit_map = 0;

            s_expmu.fail_cnt = 0;

            for(i = 0; i < EXPMU_END_REG ; i++)
            {
                s_expmu.read_bit_map |= (1 << i);
            }

            expmu_read_register();
        }
    }
    
}

void auto_test_period_read_expmu_reg(u8 reg)
{
   u8 block[40] = {0};

   u8 i = reg,len  = 0 ,j = 0;

   if(s_expmu.status != EXPMU_STATE_WORK_PROC)return;

   len = g_smsp_reg_map[i].reg_size;

   j = 0;

   j = creat_write_pmu_register_block(g_smsp_reg_map[i].pmu_addr,0,&len,1,&block[j]);

   send_read_reg_packet_to_uart(block,j);
}

u8 auto_test_get_expmu_light_io_state(void)
{
    if(s_expmu.status != EXPMU_STATE_WORK_PROC)return 0xFF;

    return ((g_gpio[EXPMU_GPIOC].IDR&GPIO_Pin_7)>0?1:0);
}

static void expmu_read_register(void)
{
     u8 block[40] = {0};

  	 u8 i = 0,len  = 0 ,j = 0;

    for(i = 0; i < EXPMU_END_REG ; i++)
    {
        if(s_expmu.read_bit_map & (1 << i))
        {
            len = g_smsp_reg_map[i].reg_size;

			j = 0;
				
			j = creat_write_pmu_register_block(g_smsp_reg_map[i].pmu_addr,0,&len,1,&block[j]);

			send_read_reg_packet_to_uart(block,j);
        }
    }
}

void get_bcd_time_string(u8 *out , u8 *bcd , u8 cnt)
{
    u8 j = 0;

    j = GM_sprintf((char *)out,"%02x-%02x-%02x %02x:%02x:%02x",bcd[0],bcd[1],bcd[2],bcd[3],bcd[4],bcd[5]);

    if(cnt > 6)
    {
        j += GM_sprintf((char *)&out[j]," %x",bcd[6]);
    }

    out[j] = 0;
}

void expmu_get_version_string(char *ver)
{
    if((GM_strlen(g_mcu_build_time) > 0)&&(GM_strlen(g_mcu_build_date) > 0)&&\
       (GM_strlen(g_mcu_version) > 0))
    {
        GM_sprintf(ver,"%s(%s %s)",g_mcu_version,g_mcu_build_date,g_mcu_build_time);
    }
    
}

u16 expmu_get_powr_on_cnt(void)
{
    return MKWORD(g_mcu_para.pwr_up[0], g_mcu_para.pwr_up[1]);
}

u16 expmu_get_rtc_wkp_cnt(void)
{
    return MKWORD(g_mcu_para.rtc_wkp[0], g_mcu_para.rtc_wkp[1]);
}

u16 expmu_get_exti1_wkp_cnt(void)
{
    return MKWORD(g_mcu_para.exit1_wkp[0], g_mcu_para.exit1_wkp[1]);
}

u16 expmu_get_exti2_wkp_cnt(void)
{
    return MKWORD(g_mcu_para.exit2_wkp[0], g_mcu_para.exit2_wkp[1]);
}


static void expmu_query_proc(void)
{
    u32 cur_time =  util_clock();
    char date[100] = {0};


    if((GM_strlen(g_mcu_build_time) > 0)&&(GM_strlen(g_mcu_build_date) > 0)&&\
       (GM_strlen(g_mcu_version) > 0)&& (g_mcu_para.factory_mark == 0xA5)) 
    {
        JsonObject* p_log_root = json_create();
        s_expmu.start_time = util_clock();
        
        expmu_transfer_status(EXPMU_STATE_WORK_PROC);

        json_add_string(p_log_root, "expmu", "comm establish");
        json_add_string(p_log_root, "wkp reason", s_wkp_reason_string[g_mcu_evt.wkp_reason]);
	    json_add_int(p_log_root, "power on", MKWORD(g_mcu_para.pwr_up[0], g_mcu_para.pwr_up[1]));
	    json_add_int(p_log_root, "rtc wkp", MKWORD(g_mcu_para.rtc_wkp[0], g_mcu_para.rtc_wkp[1]));
	    json_add_int(p_log_root, "exit1 wkp", MKWORD(g_mcu_para.exit1_wkp[0], g_mcu_para.exit1_wkp[1]));
        json_add_int(p_log_root, "exit2 wkp", MKWORD(g_mcu_para.exit2_wkp[0], g_mcu_para.exit2_wkp[1]));
        json_add_string(p_log_root, "work mode",s_work_mode_str[g_mcu_para.workmode]);
        
        get_bcd_time_string((u8 *)date,g_mcu_para.next_time,7);
        json_add_string(p_log_root,"pre wkp time", date);
        get_bcd_time_string((u8 *)date,g_mcu_evt.rtc_tim,7);
        json_add_string(p_log_root,"expmu time", date);

        expmu_get_version_string(date);
        json_add_string(p_log_root,"ver",date);
        
	    log_service_upload(INFO,p_log_root);

        expmu_service_creat();

        s_expmu.read_bit_map = 0;

        s_expmu.read_bit_map |= (1 << EXPMU_EVT_REG);

        s_expmu.read_bit_map |= (1 << EXPMU_PARA_REG);

        LOG(INFO,"MCU_DBG mcu version %s %s %s",g_mcu_version,g_mcu_build_date,g_mcu_build_time);

	    LOG(INFO,"MCU_DBG mcu event %d wkp reason %d",g_mcu_evt.event,g_mcu_evt.wkp_reason);

    }
    else
    {
        if(cur_time - s_expmu.start_time > 10)
        {
            if(s_expmu.fail_cnt < MAX_FAULT_TOLERANCE)
            {
                s_expmu.start_time = util_clock();

                expmu_read_register();
            }
            else
            {

                system_state_set_expmu_smsp_comm_cannot_establish();
                
                expmu_transfer_status(EXPMU_STATE_ERROR);
            }
        }
    }

    
}

static void expmu_work_proc(void)
{
    u32 cur_time = util_clock();

    u8 sta;

    static u8 show_info_rate = 0;

    static u8 read_reg_rate  = 0;

    if(cur_time - s_expmu.start_time >= 1)
    {
        s_expmu.start_time = util_clock();

        if(read_reg_rate == 0)
        {
            expmu_read_register();

            sta = sync_expmu_mpu_time_process();

            if(sta == 3)
            {
                s_expmu.read_reg_rate = 30;

                //s_expmu.push_data_rate = 100;
            }
            else
            {
                s_expmu.read_reg_rate = 3;
            }

            read_reg_rate = s_expmu.read_reg_rate;
            
        }


        if(show_info_rate == 0)
        {
            show_info_rate = 20;

		    show_smsp_time("serv time",g_mcu_evt.server_tim,NULL);

		    show_smsp_time("rtc  time",g_mcu_evt.rtc_tim, &g_mcu_evt.rtc_tim[6]);

		    show_smsp_time("next time",g_mcu_evt.next_tim, &g_mcu_evt.next_tim[6]);
        }

        show_info_rate--;
        read_reg_rate--;
    }
}

static void parse_smsp_query_packet(u8 *data , u16 len)
{
	u16 i = 0 , j = 0 , ilen = 0 ;

    u16 reg_size = 0;

	while(i < len)
	{
		ilen = ((u16)data[i] << 8);

		ilen |= data[i+1];

		if(ilen == 8)
		{
			g_smsp_reg_map[j].pmu_addr = ((u32)data[i+2] << 24);

			g_smsp_reg_map[j].pmu_addr |= ((u32)data[i+3] << 16);

			g_smsp_reg_map[j].pmu_addr |= ((u32)data[i+4] <<   8);

			g_smsp_reg_map[j].pmu_addr |= ((u32)data[i+5]);

            reg_size = (((u16)data[i+6] << 8)|(data[i+7]));

			if(reg_size > g_smsp_reg_map[j].reg_size)
			{
                LOG(ERROR,"MCU_DBG mpu reg[%d].size(%d) != %d",j,g_smsp_reg_map[j].reg_size,reg_size);

                break;
            }
            else
            {
                /*要以MCU的寄存器长度为准*/
                g_smsp_reg_map[j].reg_size = reg_size;
                
                LOG(INFO,"MCU_DBG query packet response map(%d) 0x%x , 0x%x , size %d",j,g_smsp_reg_map[j].pmu_addr,g_smsp_reg_map[j].mpu_addr,\
				g_smsp_reg_map[j].reg_size);
            }

			j++;

			i += 8;

			if(j >= EXPMU_END_REG)break;

		}
		else
		{
			i += 2;
		}
	}
}

u8 match_the_mpu_register_addr(u32 pmu_reg_addr)
{
	u8 i = 0;

	for(i = 0 ; i< EXPMU_END_REG ; i++)
	{
		if(g_smsp_reg_map[i].pmu_addr == pmu_reg_addr)
		{
			return i;//g_smsp_reg_map[i].mpu_addr;
		}
	}

	return 0xFF;
}

void prase_smsp_read_packet(u8 *data , u16 len)
{
	u8 reg = 0;
	
	u16 i = 0;

	u16 nlen = 0 , dlen  = 0;
	
	u32 pmu_addr = 0;
	
	while(i < len)
	{
		dlen = MKWORD(data[i], data[i+1]);

		pmu_addr = MKDWORD(data[i+2], data[i+3], data[i+4], data[i+5]);

		reg = match_the_mpu_register_addr(pmu_addr);

		if(reg < EXPMU_END_REG)
		{
			
			nlen  = g_smsp_reg_map[reg].reg_size < (dlen - 6)?g_smsp_reg_map[reg].reg_size:(dlen - 6);

			printf_hex_string("MCU_DBG read pmu reg",&data[i+6],nlen);

			GM_memcpy((u8 *)g_smsp_reg_map[reg].mpu_addr, &data[i+6], nlen);
		}

		i += dlen;
	}
}

void prase_smsp_write_response_packet(u8 *data , u16 len)
{
	u8 reg = 0;
	
	u16 i = 0;

	u16 nlen = 0 , dlen  = 0;
	
	u32 pmu_addr = 0;
	
	while(i < len)
	{
		dlen = MKWORD(data[i], data[i+1]);

		pmu_addr = MKDWORD(data[i+2], data[i+3], data[i+4], data[i+5]);

		reg = match_the_mpu_register_addr(pmu_addr);

		if(reg < EXPMU_END_REG)
		{
			
			nlen  = g_smsp_reg_map[reg].reg_size < (dlen - 6)?g_smsp_reg_map[reg].reg_size:(dlen - 6);

			printf_hex_string("MCU_DBG write pmu reg",&data[i],nlen + 6);

			GM_memcpy((u8 *)g_smsp_reg_map[reg].mpu_addr, &data[i+6], nlen);
		}

		i += dlen;
	}
}


void slave_mem_share_protocol_prase(u8 *data)
{

	Smsp_Header_Struct *head = NULL;

	//u16 len = 0;
	
	head = (Smsp_Header_Struct *)data;

	if(head != NULL)
	{
		if(check_arry_data_xor(data,MKWORD(head->lenH, head->lenL)) == 0) /*Xor correct!*/
		{
			delete_smsp_stack_que(head->mark);
			
			switch(head->type)
			{
				case RD_PACKET:
				{
					prase_smsp_read_packet(head->block,MKWORD(head->lenH, head->lenL)-5);
					
					break;
				}
				case WR_PACKET:
				{
					prase_smsp_write_response_packet(head->block,MKWORD(head->lenH, head->lenL)-5);
					
					break;
				}
				case QE_PACKET:
				{
					parse_smsp_query_packet(head->block,MKWORD(head->lenH, head->lenL)-5);
					
					break;
				}
				default:
					break;
			}
			
		}
		else
		{
			LOG(WARN,"MCU_DBG REC MCU Data XOR err!!\r\n");
		}
	}
	
}

void parse_expmu_dat_fun(u8 *dat , u16 ilen)
{
	u16 i = 0;

	u16 len = 0;

	//ptmsg("MCU_DBG Rec len %d",ilen);
	
	while(i < ilen)
	{
		if((dat[i++] == '$')&&(dat[i++] == '$'))
		{
			len  = MKWORD(dat[i], dat[i+1]);
			
			if(dat[i + len] == '@')
			{
				//printf_hex_string("MCU_DBG  Rec",dat,len+2);
				slave_mem_share_protocol_prase(&dat[i]);

				i += (len + 1);
			}
		}
	}
}

u16 insert_smsp_stack_que(u8 type,u8* data,u16 len)
{
	u16 j = 0;

	u8 xor = 0;

	u8  in = g_mcu_send.in;

	g_mcu_send.que[in].buf[j++] = '$';

	g_mcu_send.que[in].buf[j++] = '$';
	
	g_mcu_send.que[in].buf[j++] = ((len + 5) >> 8);
	
	g_mcu_send.que[in].buf[j++] = ((len + 5)&0xff);

	g_mcu_send.que[in].buf[j++] =  ++g_mcu_send.mark;
	
	g_mcu_send.que[in].buf[j++] = type;

	if(data)
	{
		GM_memcpy(&g_mcu_send.que[in].buf[j],data,len);
	}
	
	j += len;

	xor = check_arry_data_xor(&g_mcu_send.que[in].buf[2],j - 2);

	g_mcu_send.que[in].buf[j++] = xor;

	g_mcu_send.que[in].buf[j++] = '@';

	g_mcu_send.que[in].len = j;

	g_mcu_send.que[in].retry = 0;

	g_mcu_send.in++;

	g_mcu_send.in = g_mcu_send.in%MAX_SEND_QUE_LEN;

	return j;
}

static void delete_smsp_stack_que(u8 mark)
{
	u8 out = g_mcu_send.out;

	//ptmsg("MCU_DBG del mark %x",mark);

	if(g_mcu_send.que[out].buf[4] == mark)
	{
		g_mcu_send.out++;

		g_mcu_send.out = g_mcu_send.out%MAX_SEND_QUE_LEN;
	}
}

void expmu_clear_smap_stack(void)
{
   g_mcu_send.out = g_mcu_send.in;
}

u32 expmu_get_smsp_comm_err_count(void)
{
    return s_expmu.smsp_comm_err;
}

static void push_smsp_packet_to_uart(void)
{

	if(g_mcu_send.in != g_mcu_send.out)
	{
		if(g_mcu_send.que[g_mcu_send.out].len > 0)
		{
			g_mcu_send.que[g_mcu_send.out].retry++;

			if(g_mcu_send.que[g_mcu_send.out].retry >= 10)
			{
                g_mcu_send.que[g_mcu_send.out].retry = 0;
                
				LOG(ERROR,"MCU_DBG send smsp cannot get reponse");

				g_mcu_send.out++;

                s_expmu.smsp_comm_err++;

                system_state_set_expmu_comm_err_count();

				g_mcu_send.out = g_mcu_send.out%MAX_SEND_QUE_LEN;

			}
			else
			{
				
				if(GM_SUCCESS == uart_write(GM_UART_EXPMU, g_mcu_send.que[g_mcu_send.out].buf, g_mcu_send.que[g_mcu_send.out].len))
				{
                    g_mcu_send.count += g_mcu_send.que[g_mcu_send.out].len;

                    LOG(INFO,"MTK HAVE SEND %d byte",g_mcu_send.count);

                    printf_hex_string("MCU_DBG Send",g_mcu_send.que[g_mcu_send.out].buf, g_mcu_send.que[g_mcu_send.out].len);
                }
                else
                {
                    LOG(ERROR,"Send data Error!");
                }
			}
			
		}
	}
}

static void send_query_packet_to_expmu(void)
{	
	insert_smsp_stack_que(QE_PACKET,NULL,0);
}

/*SMSP协议查询包回复检测 1:成功 0:失败*/
static u8 smsp_query_packet_ack_result(void)
{
    u8 i = 0;

    for(i = 0 ; i < EXPMU_END_REG ; i++)
    {
        if(g_smsp_reg_map[i].pmu_addr == 0)
        {
            return 0;
        }
    }

    return 1;
}
static void send_read_reg_packet_to_uart(u8 *data , u16 len)
{
	insert_smsp_stack_que(RD_PACKET,data,len);
}

void write_expmu_reg(u8 reg , u8 offset , u8 *data , u8 len)
{
	u8 pdata[40] = {0};

	u8 j = 0;

	j += creat_write_pmu_register_block(g_smsp_reg_map[reg].pmu_addr,offset,data,len,pdata);

	insert_smsp_stack_que(WR_PACKET,pdata,j);
}



static u16 creat_write_pmu_register_block(u32 reg_addr , u8 offset , u8 *data , u8  len, u8* out)
{
	u16 j = 0;

	out[j++] = 0;

	out[j++] = 0;

	out[j++] = ((reg_addr + offset)>> 24)&0xFF;

	out[j++] = ((reg_addr + offset) >> 16)&0xFF;

	out[j++] = ((reg_addr + offset) >>  8)&0xFF;

	out[j++] = ((reg_addr + offset) 	  )&0xFF;

	if(data != NULL)
	{
		GM_memcpy(&out[j],data,len);

		j += len;
	}

	out[0] = ((len + 6) >> 8)&0xFF;

	out[1] = ((len + 6)     )&0xFF;

	return j;
}



void smsp_calibration_pmu_time(u8 *bcdtime)
{

	u8 serv_time[7] = {0};

	serv_time[0] = 1; 

	GM_memcpy(&serv_time[1],bcdtime,6);

	WR_EXPMU_REG(EXPMU_EVT_REG,Expmu_Control_Reg_Struct,calibration_tim_flg,serv_time,7);
	
}

void smsp_set_pmu_force_work(void)
{
	u8 force = 1;

	if(g_mcu_evt.force_work == 0)
	{
		WR_EXPMU_REG(EXPMU_EVT_REG,Expmu_Control_Reg_Struct,force_work,&force,1);
	}
}

void smsp_set_save_fram_command(void)
{
    u8 commit = 1;

    WR_EXPMU_REG(EXPMU_EVT_REG,Expmu_Control_Reg_Struct,flash_commit,&commit,1);
}

void smsp_set_exit_triger_mode(u8 mode)
{
    u8 trig = mode;

    u8 commit = 1;

    WR_EXPMU_REG(EXPMU_PARA_REG, Expmu_Fram_Para_Struct,exti1_trigger, &trig, 1);

    WR_EXPMU_REG(EXPMU_EVT_REG,Expmu_Control_Reg_Struct,flash_commit,&commit,1);
}

void smsp_set_fram_default_value(void)
{

    u8 commit = 0xA5;

    WR_EXPMU_REG(EXPMU_EVT_REG,Expmu_Control_Reg_Struct,set_fram_default,&commit,1);
}

void smsp_clear_pmu_force_work(void)
{
	u8 force_work;

	force_work = 0;
		
	WR_EXPMU_REG(EXPMU_EVT_REG,Expmu_Control_Reg_Struct,force_work,&force_work,1);

}

void smsp_set_pmu_reset(void)
{
    u8 event;

    event = EVENT_INVALID;
        
    WR_EXPMU_REG(EXPMU_EVT_REG,Expmu_Control_Reg_Struct,event,&event,1);

}

void smsp_set_pmu_time_zone(u8 zone)
{
    u8 value_u8 = zone;

    u8 commit = 1;

    WR_EXPMU_REG(EXPMU_PARA_REG, Expmu_Fram_Para_Struct, zone, &value_u8, 1);

    WR_EXPMU_REG(EXPMU_EVT_REG,Expmu_Control_Reg_Struct,flash_commit,&commit,1);

    s_expmu.read_reg_rate = 3;
}

void smsp_set_next_wakeup_time(u8 *bcdtime,u8 weekday)
{
	u8 wkp_time[7] = {0};

	u8 len = 0;

	GM_memcpy(wkp_time,bcdtime,6);

	if(weekday)
	{
		wkp_time[6] = weekday;
		
		len  = 7;
	}
    else
    {
        len = 6;
    }


	WR_EXPMU_REG(EXPMU_EVT_REG,Expmu_Control_Reg_Struct,next_tim,wkp_time,len);

	//commit = 1;

	//WR_EXPMU_REG(EXPMU_EVT_REG,system_event_control_struct,flash_commit,&commit,1);
	
}

void smsp_comm_pmu_into_sleep(void)
{
	u8 wait_time;

	wait_time = 50;
		
	WR_EXPMU_REG(EXPMU_EVT_REG,Expmu_Control_Reg_Struct,wait_to_sleep,&wait_time,1)

}


static void show_smsp_time(u8 *head,u8 *bcdtime ,u8 *week)
{
	if(week == NULL)
	{
		LOG(INFO,"MCU_DBG %s %02x-%02x-%02x %02x:%02x:%02x",head,bcdtime[0],bcdtime[1],bcdtime[2],bcdtime[3],bcdtime[4],bcdtime[5]);
	}
	else
	{
		LOG(INFO,"MCU_DBG %s %02x-%02x-%02x %02x:%02x:%02x week_day %d",head,bcdtime[0],bcdtime[1],bcdtime[2],bcdtime[3],bcdtime[4],bcdtime[5],(*week));
	}
	
}

void set_expmu_time_to_mpu(u8 *bcd)
{
    ST_Time mt;

    struct tm tm_t;

    u8 zone = 0;

    u32 t = 0,adj_sec = 0;
    
    mt.year = 2000 + BCD2HEX(bcd[0]);
    mt.month = BCD2HEX(bcd[1]);
    mt.day = BCD2HEX(bcd[2]);
    
    mt.hour = BCD2HEX(bcd[3]);
    mt.minute = BCD2HEX(bcd[4]);
    mt.second= BCD2HEX(bcd[5]);
	mt.timezone = 0;
    LOG(DEBUG,"clock(%d) expmu time GM_SetLocalTime(%d-%02d-%02d %02d:%02d:%02d)).",
        util_clock(), mt.year,mt.month,mt.day,mt.hour,mt.minute,mt.second);

    util_mtktime_to_tm(&mt, &tm_t);
    t = util_mktime(&tm_t);
    if(t == (time_t)-1)
    {
        LOG(INFO,"clock(%d) expmu time GM_SetLocalTime(%d-%02d-%02d %02d:%02d:%02d)) failed.",
            util_clock(), mt.year,mt.month,mt.day,mt.hour,mt.minute,mt.second);
        return;
    }

    config_service_get(CFG_TIME_ZONE,TYPE_BYTE,&zone,sizeof(u8));

    adj_sec = (U32)(zone & 0x7F) * 3600;

    if (zone & 0x80)
    {
        t += adj_sec;
    }
    else
    {
        t -= adj_sec;
    }

    tm_t = *util_localtime(&t);
    util_tm_to_mtktime(&tm_t, &mt);

    GM_SetLocalTime(&mt);
}

u32 expmu_get_rtc_time_value(void)
{
    return s_expmu.expmu_rtc_time;
}

static u8 sync_expmu_mpu_time_process(void)
{
    enum
    {
        SYS_TIME_EXPMU_VALID = 0x01,

        SYS_TIME_MPU_VALID   = 0x02,

        SYS_TIME_SYNC_CHECK  = 0x03,
    };
    
	u32 adj_sec = 0,rtc_time  = 0,sys_time = 0;

	u8 bcdtime[6] = {0};

	u8 zone= 0,calib = 0,time_state = 0;

	//ST_Time tim;

	rtc_time = util_bcd_to_sec(g_mcu_evt.rtc_tim);

	sys_time = util_get_utc_time();

    config_service_get(CFG_TIME_ZONE,TYPE_BYTE,&zone,sizeof(u8));

    adj_sec = ((u32)(zone & 0x7F)*3600);

    if(zone & 0x80)
    {
        rtc_time += adj_sec;
    }
    else
    {
        rtc_time -= adj_sec;
    }

    s_expmu.expmu_rtc_time = rtc_time;

    /*比较特征值,以代码编辑时间为基准*/
	if(rtc_time > 1563456865)
    {
        time_state |= SYS_TIME_EXPMU_VALID;
    }
    else
    {
        if(sys_time < rtc_time)
        {
            time_state = 1;
        }
    }

	if(sys_time > 1563456865)time_state |= SYS_TIME_MPU_VALID;


	switch(time_state)
	{
		case SYS_TIME_SYNC_CHECK:

			if(abs(rtc_time - sys_time) > 20)
			{
                system_state_set_rtc_time_not_match_count();
                
				calib = 1;

                time_state = 0;
			}
			else
			{
				//时间同步
			}

			break;
			
		case SYS_TIME_MPU_VALID:

			calib = 1;

			break;

		case SYS_TIME_EXPMU_VALID:

			set_expmu_time_to_mpu(g_mcu_evt.rtc_tim);

			break;
		//case 0:
		default:
			break;
	}

	if(calib)
	{

		util_utc_sec_to_bcdtime_base2000(sys_time,bcdtime,zone);

        LOG(INFO,"smsp_calibration_expmu_time %x-%x-%x %x:%x:%x",bcdtime[0],bcdtime[1],bcdtime[2],\
            bcdtime[3],bcdtime[4],bcdtime[5]);

		smsp_calibration_pmu_time(bcdtime);
	}

    return time_state;
}


void smsp_comm_pmu_workmode_set(u8 *para , u8 len)
{
    u8 commit = 1;
    
    WR_EXPMU_REG(EXPMU_PARA_REG,Expmu_Fram_Para_Struct,workmode,para,len);


	WR_EXPMU_REG(EXPMU_EVT_REG,Expmu_Control_Reg_Struct,flash_commit,&commit,1);
}


/*****************************************************************************
 函 数 名  : get_cur_gw_work_mode
 功能描述  : 获取当前的工作模式，如果非法，会给出默认的模式
 输入参数  : u8 *mode  
 输出参数  : 无
 返 回 值  : void
 日    期  : 2019年3月19日
 作    者  : Chris.Lu
*****************************************************************************/
void *get_cur_gw_work_mode(u8 *mode)
{
    u8 wkm[WORK_MODE_PARAM_LEN] = {0};
    u8 work_mode = 0;
    config_service_get(CFG_WORK_MODE,TYPE_BYTE,&work_mode,sizeof(work_mode));
	if((!work_mode)||( work_mode >= WORK_MODE_INVALID))
	{
		work_mode = PLATFORM_TIMEPOINT_MODE;
		
		GM_memset((u8 *)&wkm,0,WORK_MODE_PARAM_LEN);

        config_service_set(CFG_WORK_MODE_PARAM,TYPE_ARRY,(u8 *)wkm,WORK_MODE_PARAM_LEN);
	}

    if(mode)
    {
        *mode = work_mode;
    }
	
	return (void *)config_service_get_pointer(CFG_WORK_MODE_PARAM);
}



