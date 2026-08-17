#include "driver_rtc.h"
#include "delay.h"
/*
	平年/闰年月份对应天数表
*/
const uint8_t monthDays[2][12] =
{
  {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
  {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
};
//时间戳变量结构体，包含共用体时间戳和时区
UTC_TIME_STRUCT Utc_Time_t = {0};
//日历时间结构体，包含日期和时间
RTC_TIME_STRUCT   RTC_Time_Stu = {0};
//rtc参数设置事件
uint8_t rtc_para_set_event = 0;

/*
	rtc初始化
*/
void User_Rtc_Init(void)
{
	FL_RTC_InitTypeDef    defaultInitStruct;
	FL_NVIC_ConfigTypeDef    InterruptConfigStruct;

	defaultInitStruct.year = 0x20;
	defaultInitStruct.month = 0x10;
	defaultInitStruct.day = 0x01;
	defaultInitStruct.week = 0x00;
	defaultInitStruct.hour = 0x00;
	defaultInitStruct.minute = 0x00;
	defaultInitStruct.second = 0x00;

	FL_RTC_Init(RTC, &defaultInitStruct);

	InterruptConfigStruct.preemptPriority = 0x02;
	FL_NVIC_Init(&InterruptConfigStruct, RTC_IRQn);

	FL_RTC_WriteAdjustValue(RTC, 0);  //RTC的时基计数器调校
	
	FL_RTC_DisableIT_Alarm(RTC);  //关闭闹钟中断
	
	FL_RCC_EnableGroup1BusClock(FL_RCC_GROUP1_BUSCLK_RTC);  //RTC总线时钟使能
}
//RTC 闹钟中断设置
void RTC_AlarmTimeSet(void)
{
	FL_NVIC_ConfigTypeDef    InterruptConfigStruct;

	FL_RTC_DisableIT_Alarm(RTC);                              //关闭闹钟中断
	//设置闹钟时间
	FL_RTC_WriteHourAlarm(RTC, 0x00);                         //时
	FL_RTC_WriteMinuteAlarm(RTC, 0x00);                       //分
	FL_RTC_WriteSecondAlarm(RTC, 0x05);                       //秒

	FL_RTC_ClearFlag_Alarm(RTC);                              //清除闹钟中断标志
	FL_RTC_EnableIT_Alarm(RTC);                               //打开闹钟中断
	InterruptConfigStruct.preemptPriority = 0x02;
	FL_NVIC_Init(&InterruptConfigStruct, RTC_IRQn);
}
//获取RTC模块的时间到 ram
uint8_t RTC_GetRTC(FL_RTC_InitTypeDef *InitStructer)
{
	uint8_t n, i;
	uint8_t Result = 1;

	FL_RTC_InitTypeDef TempTime1, TempTime2;

	for(n = 0 ; n < 3; n++)
	{
		FL_RTC_GetTime(RTC, &TempTime1);                      //读一次时间
		FL_RTC_GetTime(RTC, &TempTime2);                      //再读一次时间

		for(i = 0; i < 7; i++)                                //两者一致, 表示读取成功
		{
			if(((uint32_t *)(&TempTime1))[i] != ((uint32_t *)(&TempTime2))[i]) { break; }
		}

		if(i == 7)
		{
			Result = 0;
			memcpy((uint32_t *)(InitStructer), (uint32_t *)(&TempTime1), 7 * sizeof(uint32_t)); //读取正确则更新新的时间
			break;
		}
	}

	return Result;
}
uint8_t RTC_SetRTC(FL_RTC_InitTypeDef *InitStructer)
{
	uint8_t n, i;
	uint8_t Result;
	FL_RTC_InitTypeDef TempTime1;

	for(n = 0 ; n < 3; n++)
	{
		FL_RTC_ConfigTime(RTC, InitStructer);
		Result = RTC_GetRTC(&TempTime1);                    //读取确认设置结果

		if(Result == 0)
		{
			Result = 1;

			for(i = 0; i < 7; i++)                          //两者一致, 表示设置成功
			{
				if(((uint32_t *)(&TempTime1))[i] != ((uint32_t *)(InitStructer))[i])
				{ break; }
			}

			if(i == 7)
			{
				Result = 0;
				break;
			}
		}
	}

	return Result;
}

//UTC转时区时间
void Timer_UtcToCtsTime(unsigned long timestampMs,char time_zone)  
{
  unsigned long timestamp = timestampMs + time_zone*3600; // 8 * 3600? 

  uint8_t seconds = timestamp % 60;
  uint8_t minutes = (timestamp / 60) % 60;
  uint8_t hours = (timestamp / 3600) % 24;
  uint16_t days = timestamp / 86400;
  uint8_t month = 0;
  uint16_t year = 1970;
  while(days >= 365 + Get_LeapYear(year))
  {
    days -= 365 + Get_LeapYear(year);
    year++;
  }

  while(days >= monthDays[Get_LeapYear(year)][month])
  {
    days -= monthDays[Get_LeapYear(year)][month];
    month++;
  }
	
	RTC_Time_Stu.year = year - 1970;
	RTC_Time_Stu.month = month + 1;
	RTC_Time_Stu.day = days + 1;
	RTC_Time_Stu.hour = hours;
	RTC_Time_Stu.min = minutes;
	RTC_Time_Stu.sec = seconds;
}
//单片机根据日期计算出周几
//0周一 1周二  2周三  3周四 4周五 5周六 6周日
uint8_t Get_DayOfWeek(uint16_t year, uint8_t month, uint8_t day)   
{
  uint8_t K = 0;
  uint8_t J = 0;
  uint8_t h = 0;
  if(month < 3)
  {
    month += 12;
    year -= 1;
  }
  K = year % 100;
  J = year / 100;
  h = (day + (13 * (month + 1)) / 5 + K + K / 4 + J / 4 - 2 * J) % 7;
  h = (h + 5) % 7;
	
  return h;
}

//判断是否是闰年 1 闰年 0平年
uint8_t Get_LeapYear(uint16_t year)
{
  return (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
}
//设置单片机RTC寄存器值
void RTC_Time_ParaSet(uint8_t year,uint8_t month,uint8_t day,uint8_t hour, uint8_t min, uint8_t sec)
{
	FL_RTC_InitTypeDef   InitTime;

#if 1
	if(year <= 99)
	{
		InitTime.year = Decimal_To_Bcd(year);
	}
	if(month >= 1 && month <= 12)
	{
		InitTime.month = Decimal_To_Bcd(month);
	}
	if(day >= 1 && day <= 31)
	{
		InitTime.day = Decimal_To_Bcd(day);
	}
	if(hour <= 23)
	{
		InitTime.hour = Decimal_To_Bcd(hour);   
	}
	if(min <= 59)
	{
		InitTime.minute = Decimal_To_Bcd(min);      
	}
	if(sec <= 59)
	{
		InitTime.second = Decimal_To_Bcd(sec);      
	}
	
	InitTime.week = Get_DayOfWeek(year  + 1970, month, day);
	
	RTC_SetRTC(&InitTime);                           
#endif	
}

