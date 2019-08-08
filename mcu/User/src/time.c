/**
* Copyright (C), Goome Technologies Co., Ltd. 2009-2019. All rights reserved.
* File name : time.c
* Author:  Version:	Date:
* Description:时间格式转换
* Function List:
* History:
  1.Date: 2019/4/10
    Author: Chris.Lu
    Modification: Created file
*/
#include "gm_type.h"
#include "time.h"
#include "bsp_watch_dog.h"
#include "system_ticks.h"

#define WITHIN_RANGE(value,max,min) (value <= max && value >= min)

#define IS_BCD_FORMAT(value) (((value>>4)/10) == 0 && 0 == ((value&0x0f)/10))

#define BCD2HEX(n)              ((((n)>>4)*10) + ((n)&0x0f))  // 0x13 = 10+3
#define HEX2BCD(n)              (((((n)/10)%10)<<4)  +  ((n)%10))


const char Days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

static time_t mon_yday[2][12] =
{
    {0,31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334},
    {0,31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335},
};
 
u32 isleap(u32 year)
{
    return (year) % 4 == 0 && ((year) % 100 != 0 || (year) % 400 == 0);
}
 
time_t mktime(struct tm dt)
{
    time_t result;
    u32 i =0;
    // 以平年时间计算的秒数

    result = (dt.tm_year - (u32)1970ul) * (u32)365ul * (u32)24ul * (u32)3600ul +
    ((u32)mon_yday[isleap(dt.tm_year)][dt.tm_mon-1] + dt.tm_mday - 1) * (u32)24ul * (u32)3600ul +
    dt.tm_hour * (u32)3600ul+ dt.tm_min * (u32)60ul + dt.tm_sec;
    // 加上闰年的秒数
    for(i=1970; i < dt.tm_year; i++)
    {
        if(isleap(i))
        {
    		result += (u32)24ul * (u32)3600ul;
        }
    }
    return(result);
}

u8 const WeekTab[] = {0x00,0x01,0x04,0x04,0x00,0x02,0x05,0x00,0x03,0x06,0x01,0x04,0x06};

u8 date_trans_to_week_day(u8 date,u8 month,u16 year)
{
    u8 d, m, w;

	u16 y;
	
    d = date;
	
    m = month;

	y = year;

	if((m <= 2) && (!(y & 0x03))) d--;

	w = (d + y + y / 4 + y / 400 - y / 100 + WeekTab[ m ] -2 ) % 7;
    //   4+2009+502+5-20+1-2
    return( w );
}


/*******************************************************************************
* Function Name  : Time_Regulate
* Description    : Returns the time entered by user, using Hyperterminal.
* Input          : None
* Output         : None
* Return         : Current time RTC counter value
*******************************************************************************/
//u32 Month_Days[13] =     {0,31,28,31,30, 31, 30, 31, 31, 30, 31, 30, 31};
#define BASIC_YEAR             2000ul
#define SEC_PER_COM_YEAR   3153600ul //(365*3600*24)
#define SEC_PER_LEAP_YEAR  31622400ul //(366*3600*24)
#define SEC_PER_4_YEAR  126230400ul //((365*3600*24)*3+(366*3600*24))
#define SEC_PER_DAY      (u32)(3600ul*24ul)
#define SEC_UTC_TO_RTC 946684800ul //(2000-01-01 00:00:00 - 1970-01-01 00:00:00)

const u32 YEAR_SEC_ACCU[5] = {0, 31622400ul, 63158400ul, 94694400ul, 126230400ul};
u32 Month_Days_Accu_C[13] = {0,31,59,90,120,151,181,212,243,273,304,334,365};
u32 Month_Days_Accu_L[13] = {0,31,60,91,121,152,182,213,244,274,305,335,366};
const u32 MONTH_SEC_ACCU_C[13] = {0, 2678400ul, 5097600ul, 7776000ul, 10368000ul, 13046400ul, 15638400ul, 18316800ul, 20995200ul, 23587200ul, 26265600ul, 28857600ul, 31536000ul};
const u32 MONTH_SEC_ACCU_L[13] = {0, 2678400ul, 5184000ul, 7862400ul, 10454400ul, 13132800ul, 15724800ul, 18403200ul, 21081600ul, 23673600ul, 26352000ul, 28944000ul, 31622400ul};


#define SecsPerDay (3600ul*24ul)

//setRtcTime = 2011 - 12 -30 11:29:00
u32 Time_Regulate(u32 *setRtcTime)
{
    u32 Tmp_Year=0xFFFF, Tmp_Month=0xFF, Tmp_Date=0xFF;
    u32 Tmp_HH = 0xFF, Tmp_MM = 0xFF, Tmp_SS = 0xFF;
    u32 LeapY, ComY, TotSeconds, TotDays;
    
    
    Tmp_Year= setRtcTime[0];
    Tmp_Month=setRtcTime[1];
    Tmp_Date= setRtcTime[2];
    
    Tmp_HH= setRtcTime[3];
    Tmp_MM= setRtcTime[4];
    Tmp_SS= setRtcTime[5];
    
    /* change Year-Month-Data-Hour-Minute-Seconds into X(Second) to set RTC->CNTR */
    if(Tmp_Year==2000)
    {
        LeapY = 0;
    }
    else
    {
        LeapY = (Tmp_Year - 2000ul -1)/4 +1;
    }
    
    ComY = (Tmp_Year - 2000ul)-(LeapY);
    
    if (Tmp_Year%4)
    {
        //common year 
        TotDays = LeapY*366ul + ComY*365ul + Month_Days_Accu_C[Tmp_Month-1] + (Tmp_Date-1);
    }
    else
    {
        //leap year
        TotDays = LeapY*366ul + ComY*365ul + Month_Days_Accu_L[Tmp_Month-1] + (Tmp_Date-1);
    }
    
    TotSeconds = TotDays*SecsPerDay + (Tmp_HH*3600ul + Tmp_MM*60ul + Tmp_SS);
    
    
    /* Return the value to store in RTC counter register */
    //return((Tmp_HH*3600 + Tmp_MM*60 + Tmp_SS));
    return TotSeconds;
}

u32 datetosecond_base2000(const u8 *bcd_data)
{
    u32 WRTC[6];
    u32 TotalSecond;
    
    WRTC[0]=((bcd_data[0]>>4)*10)+(bcd_data[0]&0x0f)+(u32)2000ul;
    WRTC[1]=((bcd_data[1]>>4)*10)+(bcd_data[1]&0x0f);
    WRTC[2]=((bcd_data[2]>>4)*10)+(bcd_data[2]&0x0f);
    WRTC[3]=((bcd_data[3]>>4)*10)+(bcd_data[3]&0x0f);
    WRTC[4]=((bcd_data[4]>>4)*10)+(bcd_data[4]&0x0f);
    WRTC[5]=((bcd_data[5]>>4)*10)+(bcd_data[5]&0x0f);
    
    TotalSecond = Time_Regulate(&WRTC[0]);
    
    return TotalSecond;
}

s32 goome_timer_sec_to_bcd_base2000(u32 sec_time, u8 *rtc_bcd)
{ 
    u16 tmp_year = 0;
    u8 tmp_mon = 1;
    u8 tmp_day = 0;
    u8 num_4year = 0;
    u8 num_year = 0;
    u32 sec_off_4year = 0;
    u8 year_off_4year = 0;
    u8 index;
    u16 num_day = 0;
    u32 num_hour = 0;
    u8 num_min = 0;
    u8 num_sec = 0;

    if (NULL == rtc_bcd)
    {
        return -1;
    }
    
    num_4year = sec_time/SEC_PER_4_YEAR;
    sec_off_4year = sec_time%SEC_PER_4_YEAR;
    
    index=1;
    while(sec_off_4year >= YEAR_SEC_ACCU[index++])
    {
        year_off_4year++;
    }
    
    /* Numer of Complete Year */
    num_year = num_4year*4 + year_off_4year;
    /* 2000,2001,...~2000+NumY-1 complete year before, so this year is 2000+NumY*/
    tmp_year = BASIC_YEAR+num_year;
    
    sec_off_4year = sec_off_4year - YEAR_SEC_ACCU[index-2];
    
    /* Month (TBD with OffSec)*/
    index = 0;
    
    if (((tmp_year%400)==0) || (((tmp_year%4)==0)&&((tmp_year%400)!=0)))   // leap year
    {
        while(sec_off_4year >= MONTH_SEC_ACCU_L[index++]);
        tmp_mon = index-1;
        sec_off_4year = sec_off_4year - MONTH_SEC_ACCU_L[index-2];
    }
    else  // common year
    {
        while(sec_off_4year >= MONTH_SEC_ACCU_C[index++]);
        tmp_mon = index-1;
        sec_off_4year = sec_off_4year - MONTH_SEC_ACCU_C[index-2];
    }
    
    /* Date (TBD with OffSec) */
    num_day = sec_off_4year/SEC_PER_DAY;
    sec_off_4year = sec_off_4year%SEC_PER_DAY;
    tmp_day = num_day+1;
    
    /* Compute  hours */
    num_hour = sec_off_4year/3600;
    /* Compute minutes */
    num_min = (sec_off_4year % 3600)/60;
    /* Compute seconds */
    num_sec = (sec_off_4year % 3600)% 60;

    rtc_bcd[0] = HEX2BCD(tmp_year - BASIC_YEAR);
    rtc_bcd[1] = HEX2BCD(tmp_mon);
    rtc_bcd[2] = HEX2BCD(tmp_day);
    rtc_bcd[3] = HEX2BCD(num_hour);
    rtc_bcd[4] = HEX2BCD(num_min);
    rtc_bcd[5] = HEX2BCD(num_sec);

    return 0;
}





u32 goome_utc_timer_sec_to_bcd_base2000(u32 utc_sec, u8* rtc_bcd)
{
    u8 idx = 0;
    if(utc_sec == 0)
    {
        for(idx = 0; idx < 6; ++idx)
        {
            rtc_bcd[idx] = 0;
        }
    }
    else
    {
        goome_timer_sec_to_bcd_base2000(utc_sec - SEC_UTC_TO_RTC, rtc_bcd);
    }

    return 0;
}


u32 s_sys_time = 0;

void delayms(__IO u32 ms)
{
	s_sys_time = 0;
	while((++s_sys_time) < (u32)ms*(u32)40ul)
	{
		feed_dog();
	}
}


