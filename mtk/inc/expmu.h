#ifndef __EXPMU_H__
#define __EXPMU_H__

#include "gm_type.h"

typedef enum
{
	EVENT_POWER_UP = 0,//上电初始化

	EVENT_WAIT_LPM,    //等待进入低功耗

    EVENT_PREPARE_ENTER_LPM, //准备进入低功耗

	EVENT_ENTER_LPM,   //进入低功耗

	EVENT_RTC_WKP,     //RTC中断事件

	EVENT_EXTI1_WKP,   //外部中断1 事件

	EVENT_EXTI2_WKP,   //外部中断2 事件

	EVENT_EXIT_LPM,    //退出低功耗

    EVENT_INVALID,
	
}SYS_EVENT_ENUM;


typedef enum
{
	EXPMU_EVT_REG,

	EXPMU_VER_REG,

	EXPMU_DATE_REG,

	EXPMU_TIME_REG,

	EXPMU_PARA_REG,

	EXPMU_GPIOA_REG,

	EXPMU_GPIOB_REG,

	EXPMU_GPIOC_REG,

	EXPMU_GPIOD_REG,

	EXPMU_END_REG
	
}EXPMU_Support_Reg_Enum;

typedef enum
{
	EXPMU_GPIOA,
	EXPMU_GPIOB,
	EXPMU_GPIOC,
	EXPMU_GPIOD
	
}EXPMU_GPIO_Register_Struct;

typedef struct GPIO_struct
{
  u8 ODR; /*!< Output Data Register */
  u8 IDR; /*!< Input Data Register */
  u8 DDR; /*!< Data Direction Register */
  u8 CR1; /*!< Configuration Register 1 */
  u8 CR2; /*!< Configuration Register 2 */
}Expmu_Gpio_TypeDef;


typedef enum
{
  GPIO_Pin_0    = ((u8)0x01),   /*!< Pin 0 selected */
  GPIO_Pin_1    = ((u8)0x02),   /*!< Pin 1 selected */
  GPIO_Pin_2    = ((u8)0x04),   /*!< Pin 2 selected */
  GPIO_Pin_3    = ((u8)0x08),   /*!< Pin 3 selected */
  GPIO_Pin_4    = ((u8)0x10),   /*!< Pin 4 selected */
  GPIO_Pin_5    = ((u8)0x20),   /*!< Pin 5 selected */
  GPIO_Pin_6    = ((u8)0x40),   /*!< Pin 6 selected */
  GPIO_Pin_7    = ((u8)0x80),   /*!< Pin 7 selected */
  GPIO_Pin_LNib = ((u8)0x0F),   /*!< Low nibble pins selected */
  GPIO_Pin_HNib = ((u8)0xF0),   /*!< High nibble pins selected */
  GPIO_Pin_All  = ((u8)0xFF)    /*!< All pins selected */
}GPIO_Pin_TypeDef;


#pragma pack(1)
typedef struct
{
	u8 factory_mark;


	u8 next_time[7];

	
	u8 exti1_trigger;

	u8 exti2_trigger;

    u8 workmode;

    u8 mod_param[24];

    u8 zone;

    u8 pwr_up[2];

    u8 rtc_wkp[2];

    u8 exit1_wkp[2];

    u8 exit2_wkp[2];

    u8 wkp_time[7];
	
}Expmu_Fram_Para_Struct;

//TODO 注释 LCL
typedef struct
{
	u8 event; 

	u8 wkp_reason;

	u8 exti1_trigger;

	u8 exti2_trigger;

	u8 calibration_tim_flg;

	u8 server_tim[6]; /**/

	u8 rtc_tim[7]; /**/

	u8 next_tim[7];

	u8 force_work;    /**/

	u8 wait_to_sleep; /**/

	u8 flash_commit;

	u8 set_fram_default;

	u8 evt_hold_tim[4];

	u8 evt_hold_max_time[4];

}Expmu_Control_Reg_Struct;
#pragma pack()

typedef struct
{
	u32 pmu_addr;  /*pmu：power management unit，在读写消息中由这个地址去匹配mpu_addr, */

	u32 mpu_addr;  /*微处理器单元(mcu) 寄存器信息 的地址*/

	u16 reg_size;
	
}Expmu_Reg_Mapping_Struct;

extern Expmu_Reg_Mapping_Struct g_smsp_reg_map[];

extern char g_mcu_version[];

extern Expmu_Control_Reg_Struct g_mcu_evt;

/*将外部MCU当做 PMU寄存器处理，当前模块中需要实时查询MCU中涉及的寄存器状态。*/
extern void expmu_create(void);

extern void expmu_timer_proc(void);

extern void smsp_set_pmu_force_work(void);

extern void smsp_clear_pmu_force_work(void);

extern void smsp_comm_pmu_workmode_set(u8 *para , u8 len);

extern u8 is_expmu_power_on(void);

extern u8 *get_expmu_next_wkp_time_register(void);

extern u8 does_expmu_in_sleeping(void);

extern void *get_cur_gw_work_mode(u8 *mode);

extern void parse_expmu_dat_fun(u8 *dat , u16 ilen);

extern void smsp_set_next_wakeup_time(u8 *bcdtime,u8 weekday);

extern void smsp_comm_pmu_into_sleep(void);

extern u8 get_expmu_wkp_reason(void);

extern void smsp_set_exit_triger_mode(u8 mode);

extern void smsp_set_fram_default_value(void);

extern void expmu_get_version_string(char *ver);

extern void expmu_clear_smap_stack(void);

extern u32 expmu_get_smsp_comm_err_count(void);

extern u16 expmu_get_powr_on_cnt(void);

extern u16 expmu_get_rtc_wkp_cnt(void);

extern u16 expmu_get_exti1_wkp_cnt(void);

extern u16 expmu_get_exti2_wkp_cnt(void);

extern void smsp_set_pmu_reset(void);

extern u32 expmu_get_rtc_time_value(void);

extern void auto_test_period_read_expmu_reg(u8 reg);

extern u8 auto_test_get_expmu_light_io_state(void);

extern void smsp_set_save_fram_command(void);

extern void smsp_set_pmu_time_zone(u8 zone);

extern void expmu_exit_sleep(void);

extern void expmu_enter_sleep(void);

#define offsetof(TYPE, MEMBER) ((u32) &((TYPE *)0)->MEMBER)

#define WR_EXPMU_REG(REG,TYPE,MEMBER,DATA,LEN) if((offsetof(TYPE, MEMBER) + LEN) > sizeof(TYPE)) \
												{\
													LOG(INFO,"MCU_DBG write mcu reg len err fun %s line %s", __FUNCTION__, __LINE__);\
												}\
												else\
												{\
													if(g_smsp_reg_map[REG].pmu_addr == 0)\
													{\
														LOG(INFO,"MCU_DBG pmu addr err!! fun %s line %s", __FUNCTION__, __LINE__);\
													}\
													else\
													{\
														write_expmu_reg(REG,offsetof(TYPE, MEMBER),DATA,LEN);\
													}\
												}

#endif



