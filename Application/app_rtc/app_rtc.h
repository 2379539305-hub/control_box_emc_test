#ifndef __APP_RTC_H
#define __APP_RTC_H

#include "main.h"
#include "system.h"


typedef struct
{
	unsigned char  alarm_mode;       //单次、重复
	unsigned char  alarm_cmd;        //到达闹钟执行命令
	unsigned char  alarm_down_day;   //闹钟倒计时天
	unsigned short alarm_down_minutes;  //闹钟倒计时分钟
}RTC_ALARM_STRUCT;
extern RTC_ALARM_STRUCT  RTC_Alarm_Stu;



#define MAX_ALARM_COUNT 20
#define RTC_CTS_TIME_UPDATA_EVENT     (0X01)
#define RTC_UTC_TIME_UPDATA_EVENT     (0X02)
#define RTC_ALARM_TIME_SET_EVENT      (0X04)
#define RTC_ALARM_CANCLE_EVENT    		(0X08)
#define RTC_ALARM_CHANGE_EVENT    		(0X10)
#define RTC_ALARM_MODE_SET_EVENT      (0x20)
typedef struct
{
  unsigned char number;
  unsigned char bed_type;
  unsigned char hour;
  unsigned char minute;
  unsigned char mode;
  unsigned char repeat_flags;  //重复开关
  unsigned char report_flag;
  unsigned char run_flag;
	unsigned char motor_data[4]; //2个电机
}ALARM_STRUCT;

typedef struct
{
	unsigned char alarm_count;
  ALARM_STRUCT Alarm_Stu[MAX_ALARM_COUNT+1];
}ALARM_LIST;

extern ALARM_LIST Alarm_List_BLE;
extern unsigned char alarm_key_state; //闹钟不分控 全同控
extern unsigned char alarm_mode_value[4];
extern unsigned char alarm_msgr_running; //闹钟按摩器运行标志
void RTC_Control(void);

void Alarm_Protocol(ALARM_LIST *alarm_list,unsigned char *payload, unsigned char length);
void Alarm_List_Add(ALARM_LIST *alarm_list,const unsigned char *udata_buf, unsigned char buf_length);
void Alarm_List_Delete(ALARM_LIST *alarm_list,const unsigned char *udata_buf, unsigned char buf_length);

#endif






