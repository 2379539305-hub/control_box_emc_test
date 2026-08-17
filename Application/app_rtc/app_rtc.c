#include "app_rtc.h"
#include "delay.h"
#include "app_ble.h"
#include "driver_rtc.h"
#include "driver_beep.h"
#include "driver_ble.h"

#include "app_save.h"
#include "app_ttlbus.h"
#include "app_comm.h"
#include "app_motor.h"
#include "modul_motor.h"
#include "app_msgr.h"

#include "app_config.h"

//用于接受蓝牙指令
RTC_ALARM_STRUCT  RTC_Alarm_Stu = {0};
ALARM_LIST Alarm_List_BLE = {0};

FL_RTC_InitTypeDef Rtc_RealTimer = {0}; //RTC实时时间
unsigned char alarm_key_state = 0; //闹钟不分控 全同控
unsigned char alarm_mode_value[4] = {0}; //闹钟特殊模式值
unsigned char alarm_msgr_running = 0; //闹钟按摩器运行标志

void Alarm_Report(void); //轮询判断上报
void Alarm_Process(void);

void RTC_Control(void)
{
	//获取实时时间
	RTC_GetRTC(&Rtc_RealTimer);	
	Rtc_RealTimer.year = Bcd_To_Decimal(Rtc_RealTimer.year);
	Rtc_RealTimer.month = Bcd_To_Decimal(Rtc_RealTimer.month);
	Rtc_RealTimer.day = Bcd_To_Decimal(Rtc_RealTimer.day);
	Rtc_RealTimer.hour = Bcd_To_Decimal(Rtc_RealTimer.hour);
	Rtc_RealTimer.minute = Bcd_To_Decimal(Rtc_RealTimer.minute);
	Rtc_RealTimer.second = Bcd_To_Decimal(Rtc_RealTimer.second);
	//更新时间
	if((rtc_para_set_event & RTC_CTS_TIME_UPDATA_EVENT) == RTC_CTS_TIME_UPDATA_EVENT)
	{
		power_on_time = 0;
		
		RTC_Time_Stu.rtc_enbale_flag = 1;
		
		RTC_Time_ParaSet(RTC_Time_Stu.year,RTC_Time_Stu.month,RTC_Time_Stu.day,RTC_Time_Stu.hour,RTC_Time_Stu.min,RTC_Time_Stu.sec);
		Delay_Ms(10); //等待RTC寄存器写完
		//
		rtc_para_set_event &= ~RTC_CTS_TIME_UPDATA_EVENT;
	}
	if((rtc_para_set_event & RTC_UTC_TIME_UPDATA_EVENT) == RTC_UTC_TIME_UPDATA_EVENT)
	{
		power_on_time = 0;
		
		RTC_Time_Stu.rtc_enbale_flag = 1;
		
		Timer_UtcToCtsTime(Utc_Time_t.Utc_Tamp_Union.system_tamp,Utc_Time_t.zone);	
		
		RTC_Time_ParaSet(RTC_Time_Stu.year,RTC_Time_Stu.month,RTC_Time_Stu.day,RTC_Time_Stu.hour,RTC_Time_Stu.min,RTC_Time_Stu.sec);
		Delay_Ms(10); //等待RTC寄存器写完
		//
		rtc_para_set_event &= ~RTC_UTC_TIME_UPDATA_EVENT;
	}
	//超时没同步时间 一个月
	if(1 == RTC_Time_Stu.rtc_enbale_flag)
	{
		if(power_on_time >= POWER_ON_TIME_MAX)
		{
			RTC_Time_Stu.rtc_enbale_flag = 0;
		}
	}
	//设置闹钟
	if((rtc_para_set_event & RTC_ALARM_TIME_SET_EVENT) == RTC_ALARM_TIME_SET_EVENT) 
	{
		unsigned char temp_alarm_buff[8] = {0};
		unsigned short temp_minutes = 0;
		temp_minutes = Rtc_RealTimer.minute + Rtc_RealTimer.hour*60;							//计算当前已经过了多少分钟
		temp_minutes += RTC_Alarm_Stu.alarm_down_minutes;//计算闹钟要增加多少分钟
		temp_minutes = temp_minutes%1440; //如果超过了一天，则取余
		
		temp_alarm_buff[0] = 0x00;
		temp_alarm_buff[1] = 0x00;
		temp_alarm_buff[2] = temp_minutes/60;
		temp_alarm_buff[3] = temp_minutes%60;
		temp_alarm_buff[4] = RTC_Alarm_Stu.alarm_cmd;
		temp_alarm_buff[5] =  RTC_Alarm_Stu.alarm_mode ? 0xff : 0x01;
		temp_alarm_buff[6] = 0x00;
		temp_alarm_buff[7] = 0x00;
		
		Alarm_List_Add(&Alarm_List_BLE,temp_alarm_buff,8);
		
		rtc_para_set_event &= ~RTC_ALARM_TIME_SET_EVENT;
	}
		//设置闹钟模式	
	if((rtc_para_set_event & RTC_ALARM_MODE_SET_EVENT) == RTC_ALARM_MODE_SET_EVENT) 
	{
		unsigned char temp_alarm_buff[8] = {0};
		unsigned short temp_minutes = 0;
		
		temp_minutes = Rtc_RealTimer.minute + Rtc_RealTimer.hour*60;							//计算当前已经过了多少分钟
		
		temp_minutes += RTC_Alarm_Stu.alarm_down_minutes;//计算闹钟要增加多少分钟
		temp_minutes = temp_minutes%1440; //如果超过了一天，则取余
		
		temp_alarm_buff[0] = 0x00;
		temp_alarm_buff[1] = 0x00;
		temp_alarm_buff[2] = temp_minutes/60;
		temp_alarm_buff[3] = temp_minutes%60;
		temp_alarm_buff[4] = RTC_Alarm_Stu.alarm_cmd;
		temp_alarm_buff[5] =  RTC_Alarm_Stu.alarm_mode ? 0xff : 0x01;
		temp_alarm_buff[6] = 0x00;
		temp_alarm_buff[7] = 0x00;
		
		Alarm_List_Add(&Alarm_List_BLE,temp_alarm_buff,8);
		
		rtc_para_set_event &= ~RTC_ALARM_MODE_SET_EVENT;
	}
	//取消闹钟
	if((rtc_para_set_event & RTC_ALARM_CANCLE_EVENT) == RTC_ALARM_CANCLE_EVENT) 
	{
		unsigned char temp_alarm_buff[8] = {0};
		
		temp_alarm_buff[0] = 0x00;
		temp_alarm_buff[1] = 0x00;
		temp_alarm_buff[2] = 0x00;
		temp_alarm_buff[3] = 0x00;
		temp_alarm_buff[4] = 0x00;
		temp_alarm_buff[5] = 0x00;
		temp_alarm_buff[6] = 0x00;
		temp_alarm_buff[7] = 0x00;
		
		Alarm_List_Add(&Alarm_List_BLE,temp_alarm_buff,8);
		rtc_para_set_event &= ~RTC_ALARM_CANCLE_EVENT;
	}
	/*-----------------------------新闹钟协议-------------------------------*/
	Alarm_Process();
	Alarm_Report();
}

/*--------------------------------------------多个闹钟设置实现方法------------------------------------------------*/
//闹钟添加、查询、删除  在协议接收调用
void Alarm_Protocol(ALARM_LIST *alarm_list,unsigned char *payload, unsigned char length)
{
	unsigned char i = 0;
  //功能位 01 状态 02 删除
  unsigned char command = payload[1];
  //命令字 读写读返回上报
  unsigned char operation = payload[2];
  //Payload: 0x05 0x01 0x01 0x14 0x02 0x07 0x30 0x01 0xFF
  //05 01 00 00
  switch(operation)
  {
    // 读
    case 0x00:
    {
      switch(command)
      {
        // 状态
        case 0x01:  //问询 
        {
          if(payload[3] == 0) //问询所有 
          {
						for(i = 1;i <= alarm_list->alarm_count;i ++)
						{
							alarm_list->Alarm_Stu[i].report_flag = 2;
						}
          }
          else
          {
            alarm_list->Alarm_Stu[payload[3]].report_flag = 2;
          }
        }
        break;
      }
    }
    break;
    // 写
    case 0x01:
    {
      switch(command)
      {
        // 状态
        case 0x01:  //添加
        {
          Alarm_List_Add(alarm_list,payload + 3, length - 3);
        }
        break;
        // 删除
        case 0x02:
        {
          Alarm_List_Delete(alarm_list,payload + 3, length - 3);
        }
        break;
      }
    }
    break;
  }
}
void Alarm_Report_Buff(unsigned char command, ALARM_STRUCT *alarm_stu) //协议打包
{
  unsigned char report_buff[17] = {0};
	unsigned char i = 0;
	unsigned char payload[10];
	
	//BLE上报
  report_buff[0] = 0x6e;
  report_buff[1] = 0x20;
  report_buff[2] = 0x0d;
  report_buff[3] = 0x05;
  report_buff[4] = 0x01;
  report_buff[5] = command;
  report_buff[6] = alarm_stu->number;
  report_buff[7] = alarm_stu->bed_type;
  report_buff[8] = alarm_stu->hour;
  report_buff[9] = alarm_stu->minute;
  report_buff[10] = alarm_stu->mode;
  report_buff[11] = alarm_stu->repeat_flags;

  if(alarm_stu->mode != 0xff)
  {
    report_buff[12] = 0;
    for(i = 0;i < 12 ;i ++)
    {
      report_buff[12] += report_buff[i];
    }
    BleBlueTooth_SendString(report_buff,13);
  }
  else
  {
    report_buff[2] = 0x0f;
    report_buff[12] = alarm_stu->motor_data[0];
    report_buff[13] = alarm_stu->motor_data[1];
    report_buff[14] = alarm_stu->motor_data[2];
    report_buff[15] = alarm_stu->motor_data[3];
    report_buff[16] = 0;
    for(i = 0;i < 16 ;i ++)
    {
      report_buff[16] += report_buff[i];
    }
    BleBlueTooth_SendString(report_buff,17);
  }
	Delay_Ms(70);

}
void Alarm_Report(void) //轮询判断上报
{
	unsigned char j = 0;
	static uint8_t report_count = 0;
	if(report_count ++ < 10)
	{
		return;
	}
	report_count = 0;
	if(control_ack_event) return;
	
  
  for(j = 1 ; j <= Alarm_List_BLE.alarm_count; j ++)
  {
    if(Alarm_List_BLE.Alarm_Stu[j].report_flag == 1) //状态变化上报(目前只有单次闹钟执行完毕上报闹钟关闭)  闹钟改变
    {
      Alarm_List_BLE.Alarm_Stu[j].report_flag = 0;
      Alarm_Report_Buff(0x03, &Alarm_List_BLE.Alarm_Stu[j]);
			
			//alarm_trig_number = Alarm_List_BLE.Alarm_Stu[j].number;
    }
    else if(Alarm_List_BLE.Alarm_Stu[j].report_flag == 2) //设备问询时候
    {
      Alarm_List_BLE.Alarm_Stu[j].report_flag = 0;
      Alarm_Report_Buff(0x02, &Alarm_List_BLE.Alarm_Stu[j]);
    }
  }
}
RTC_TIME_STRUCT  RTC_Time_Stu_Temp = {0};
void Alarm_Process(void)
{
  static unsigned char sec = 0;
	unsigned char i = 0 ;
	//可加一个定时判断
	if(1 == RTC_Time_Stu.rtc_enbale_flag || CLOCK_SINGLE_CONFIG == system_config.flags.alarm_enable)
  {
		if(sec != Rtc_RealTimer.second)
    {
      sec = Rtc_RealTimer.second;
      RTC_Time_Stu_Temp.year = Rtc_RealTimer.year;
      RTC_Time_Stu_Temp.month = Rtc_RealTimer.month;
      RTC_Time_Stu_Temp.day = Rtc_RealTimer.day;
      RTC_Time_Stu_Temp.hour = Rtc_RealTimer.hour;
      RTC_Time_Stu_Temp.min = Rtc_RealTimer.minute;
      RTC_Time_Stu_Temp.sec = Rtc_RealTimer.second;
			
      RTC_Time_Stu_Temp.weekday = Get_DayOfWeek(RTC_Time_Stu_Temp.year  + 1970, RTC_Time_Stu_Temp.month, RTC_Time_Stu_Temp.day);
			
      for(i = 0 ; i <= Alarm_List_BLE.alarm_count; i ++)
      {
        if((Alarm_List_BLE.Alarm_Stu[i].repeat_flags & 0x01) == 0x01)  // 如果闹钟开着
        {
          if(Alarm_List_BLE.Alarm_Stu[i].repeat_flags == 0x01) //如果是单次闹钟
          {
            if(Alarm_List_BLE.Alarm_Stu[i].hour == RTC_Time_Stu_Temp.hour && Alarm_List_BLE.Alarm_Stu[i].minute == RTC_Time_Stu_Temp.min)
            {
              Alarm_List_BLE.Alarm_Stu[i].report_flag = 1; //主动上报
              
              Alarm_List_BLE.Alarm_Stu[i].repeat_flags = 0;
              
              rtc_para_set_event |= RTC_ALARM_CHANGE_EVENT;
              /*---------------------执行控制动作---------------------*/
              if(Alarm_List_BLE.Alarm_Stu[i].mode != 0)
              {
                alarm_key_state = 1;
                if(Alarm_List_BLE.Alarm_Stu[i].mode == 0xff)
                {
                  alarm_mode_value[0] = Alarm_List_BLE.Alarm_Stu[i].motor_data[0];
                  alarm_mode_value[1] = Alarm_List_BLE.Alarm_Stu[i].motor_data[1];
                  alarm_mode_value[2] = Alarm_List_BLE.Alarm_Stu[i].motor_data[2];
                  alarm_mode_value[3] = Alarm_List_BLE.Alarm_Stu[i].motor_data[3];
                  motor_para_set_event |= MOTOR_ALARM_RUN_EVENT;
                }
                else if(Alarm_List_BLE.Alarm_Stu[i].mode == 0x4C) // BLE_tmpbuf_MSGR1_INTS_ADD: 本地按摩1档,不同步
                {
                  motor_para_set_event |= MOTOR_ALARM_MSGR_RUN_EVENT;
                }
                else
                {
                  if(system_config.flags.alarm_action_mode == ALARM_ACTION_SINGLE)
                  {
                    Motor_OneClickCmd_Set(Ble_Analy_KeyValue(Alarm_List_BLE.Alarm_Stu[i].mode));
                  }
                  else if(system_config.flags.alarm_action_mode == ALARM_ACTION_THREE)
                  {
                    alarm_three_target_mode = Ble_Analy_KeyValue(Alarm_List_BLE.Alarm_Stu[i].mode);
                    motor_para_set_event |= MOTOR_ALARM_THREE_RUN_EVENT;
                  }
                }
              }
              Motor_Sync_EnableSet(0);  //闹钟指令结束马达位置不同步
              /*------------------------------------------------------*/
            }
          }
          else //不是单次
          {
            if(((Alarm_List_BLE.Alarm_Stu[i].repeat_flags >> (7 - RTC_Time_Stu_Temp.weekday)) & 0x01) == 0x01)
            {
              if(Alarm_List_BLE.Alarm_Stu[i].hour == RTC_Time_Stu_Temp.hour && Alarm_List_BLE.Alarm_Stu[i].minute == RTC_Time_Stu_Temp.min)
              {
                if(Alarm_List_BLE.Alarm_Stu[i].run_flag == 0)//一分钟之内只执行一次
                {
                  Alarm_List_BLE.Alarm_Stu[i].run_flag = 1;
                  
                  /*---------------------执行控制动作---------------------*/
                  if(Alarm_List_BLE.Alarm_Stu[i].mode != 0)
                  {
                    alarm_key_state = 1;
                    if(Alarm_List_BLE.Alarm_Stu[i].mode == 0xff)
                    {
                      alarm_mode_value[0] = Alarm_List_BLE.Alarm_Stu[i].motor_data[0];
                      alarm_mode_value[1] = Alarm_List_BLE.Alarm_Stu[i].motor_data[1];
                      alarm_mode_value[2] = Alarm_List_BLE.Alarm_Stu[i].motor_data[2];
                      alarm_mode_value[3] = Alarm_List_BLE.Alarm_Stu[i].motor_data[3];                      
                     
                      motor_para_set_event |= MOTOR_ALARM_RUN_EVENT;
                    }
                    else if(Alarm_List_BLE.Alarm_Stu[i].mode == 0x4C) // BLE_tmpbuf_MSGR1_INTS_ADD: 本地按摩1档,不同步
                    {
                      motor_para_set_event |= MOTOR_ALARM_MSGR_RUN_EVENT;
                    }
                    else
                    {
                      if(system_config.flags.alarm_action_mode == ALARM_ACTION_SINGLE)
                      {
                        Motor_OneClickCmd_Set(Ble_Analy_KeyValue(Alarm_List_BLE.Alarm_Stu[i].mode));
                      }
                      else if(system_config.flags.alarm_action_mode == ALARM_ACTION_THREE)
                      {
                        alarm_three_target_mode = Ble_Analy_KeyValue(Alarm_List_BLE.Alarm_Stu[i].mode);
                        motor_para_set_event |= MOTOR_ALARM_THREE_RUN_EVENT;
                      }
                    }
                  }
                  Motor_Sync_EnableSet(0);  //闹钟指令结束马达位置不同步
                  /*------------------------------------------------------*/
                }
              }
              else
              {
								if(0 == GetSet_Motor_Ctr_Cmd(0xff))
								{
                 Alarm_List_BLE.Alarm_Stu[i].run_flag = 0;
								}
              }
            }				
          }
        }
        else
        {
           Alarm_List_BLE.Alarm_Stu[i].run_flag = 0;
        }
      }
    }
  }
}

//解析闹钟数据并添加到列表
void Alarm_List_Add(ALARM_LIST *alarm_list,const unsigned char *udata_buf, unsigned char buf_length)
{
	unsigned char i = 0;
	unsigned char add_or_change = 0;	
	ALARM_STRUCT new_alarm = {0};
  // 传入参数不对
  if(buf_length != 6 && buf_length != 8 && buf_length != 10)
  {
    return;
  }

	// 增加新闹钟到列表中
	new_alarm.number = udata_buf[0],
	new_alarm.bed_type = udata_buf[1],
	new_alarm.hour = udata_buf[2],
	new_alarm.minute = udata_buf[3],
	new_alarm.mode = udata_buf[4],
	new_alarm.repeat_flags = udata_buf[5];
	
	new_alarm.motor_data[0] = 0;new_alarm.motor_data[1] = 0;
	if(buf_length == 8)
	{
		new_alarm.motor_data[0] = udata_buf[6];
		new_alarm.motor_data[1] = udata_buf[7];
	}
	else if((buf_length == 10) && (new_alarm.mode == 0xff))
	{
		new_alarm.motor_data[0] = udata_buf[6];
		new_alarm.motor_data[1] = udata_buf[7];
		new_alarm.motor_data[2] = udata_buf[8];
		new_alarm.motor_data[3] = udata_buf[9];
	}
	
	new_alarm.report_flag = 1;
	
	if(new_alarm.number == 0) //如果编号是0，那么就是普通闹钟
	{
		alarm_list->Alarm_Stu[0] = new_alarm;
		return;
	}
	
	for(i = 1; i <= alarm_list->alarm_count; i++)  //搜索是否有重复编号
	{
		if(alarm_list->Alarm_Stu[i].number == new_alarm.number)
		{
			add_or_change = i;
		}
	}
	// 闹钟已满
	if(alarm_list->alarm_count >= MAX_ALARM_COUNT && add_or_change == 0) //数量没满 且是新增的编号
	{
		return;
	}
	
	if(add_or_change != 0) //有重复编号  更新
	{
		alarm_list->Alarm_Stu[add_or_change] = new_alarm;
	}
	else //没有重复编号  新增
	{
		alarm_list->Alarm_Stu[++(alarm_list->alarm_count)] = new_alarm;
	}
}

//从列表中删除指定闹钟
void Alarm_List_Delete(ALARM_LIST *alarm_list,const unsigned char *udata_buf, unsigned char buf_length)
{
  unsigned char i = 0 , j = 0;
	unsigned char id = 0;
  // 传入参数不对
  if(buf_length < 1)
  {
    return;
  }
  // 闹钟为空
  if(alarm_list->alarm_count == 0)
  {
    return;
  }
  if(buf_length == 1 && udata_buf[0] == 0x00)
  {
    memset(alarm_list, 0, sizeof(ALARM_LIST)); // 清除所有闹钟
		
    alarm_list->alarm_count = 0;
    return;
  }
  for(i = 0; i < buf_length; i ++)
  {
    id = udata_buf[i];
    for(j = 1; j <= alarm_list->alarm_count; j ++)
    {
      if(alarm_list->Alarm_Stu[j].number == id)
      {
        // 找到匹配的闹钟，将其后的所有闹钟向前移动一位
        memmove(&alarm_list->Alarm_Stu[j], &alarm_list->Alarm_Stu[j + 1], (alarm_list->alarm_count - j) * sizeof(ALARM_STRUCT));
        memset(&alarm_list->Alarm_Stu[alarm_list->alarm_count], 0, sizeof(ALARM_STRUCT));
        alarm_list->alarm_count--; // 更新闹钟总数
        break; // 退出内层循环，继续下一个ID的删除
      }
    }
  }
}


