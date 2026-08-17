#ifndef __DRIVER_RTC_H
#define __DRIVER_RTC_H

#include "system.h"
#include "driver_config.h"


#define RTC_UPDATA_TIME_EVENT  (0X01)

typedef struct
{
	uint8_t  rtc_enbale_flag;
	uint8_t  year;
	uint8_t  month;
	uint8_t  day;
	uint8_t  weekday;
	uint8_t  hour;
	uint8_t  min;
	uint8_t  sec;
}RTC_TIME_STRUCT;
extern RTC_TIME_STRUCT  RTC_Time_Stu;

typedef union
{
	uint32_t system_tamp;
	uint8_t system_tamp_buff[4];
}UTC_TAMP_UNION;

typedef struct
{
	UTC_TAMP_UNION Utc_Tamp_Union;
	char zone;
}UTC_TIME_STRUCT;
extern UTC_TIME_STRUCT Utc_Time_t;


#define RTC_CTS_TIME_UPDATA_EVENT     (0X01)
#define RTC_UTC_TIME_UPDATA_EVENT     (0X02)
#define RTC_ALARM_TIME_SET_EVENT       (0X04)
#define RTC_ALARM_CANCLE_EVENT    (0X08)
#define RTC_ALARM_CHANGE_EVENT    (0X10)
#define RTC_ALARM_MODE_SET_EVENT       (0x20)
extern uint8_t rtc_para_set_event;

void User_Rtc_Init(void);

void RTC_Alarm_Enable(void);
void RTC_Alarm_Disable(void);
uint8_t RTC_GetRTC(FL_RTC_InitTypeDef *InitStructer);
uint8_t RTC_SetRTC(FL_RTC_InitTypeDef *InitStructer);
uint8_t Get_DayOfWeek(uint16_t year, uint8_t month, uint8_t day);
uint8_t Get_LeapYear(uint16_t year); //判断是否是闰年
void Timer_UtcToCtsTime(unsigned long timestampMs,char time_zone);  //UTC转时区时间
void RTC_Time_ParaSet(uint8_t year,uint8_t month,uint8_t day,uint8_t hour, uint8_t min, uint8_t sec);
void RTC_Alarm_ParaSet(uint8_t alarm_hour, uint8_t alarm_min, uint8_t alarm_sec);


#endif






