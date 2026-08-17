#include "app_ble.h"
#include "delay.h"

#include "driver_ble.h"
#include "driver_beep.h"
#include "driver_rtc.h"

#include "modul_ttlbus.h"
#include "modul_a7105.h"

#include "app_light.h"
#include "app_msgr.h"
#include "app_music.h"
#include "app_rtc.h"
#include "app_light.h"
#include "app_config.h"
#include "app_backhual.h"
#include "app_comm.h"
#include "app_fan.h"
#include "app_ota.h"
#include "app_motor.h"

unsigned char alarm_clock_min_h = 0;
unsigned char alarm_clock_min_l = 0;
unsigned char alarm_clock_days = 0;

unsigned char Ble_Set_Name[] = "AT+NAMEQRRM000000\r\n";
unsigned char Ble_StartRadio_Cmd[] = "AT+ADVSTARPAIR\r\n";
unsigned char Ble_StopRadio_Cmd[] = "AT+ADVSTOPPAIR\r\n";
unsigned char Ble_Disconnect[] = "AT+DISCONNECT\r\n";
unsigned char BLE_Advertising_Mode[] = "AT+SWITCHMODE1\r\n";  //广播模式
unsigned char BLE_RF_Mode[] = "AT+SWITCHMODE2\r\n";  //射频模式	
unsigned char BLE_Reset[] = "AT+RESET\r\n";

unsigned char ble_study_mode = 1; //是否在对码模式

unsigned short ble_key_value = 0;

unsigned long control_ack_event = 0;


unsigned char ttl_vita_set_radioenable = 0; //睡眠传感器带信息设置

#define BLE_BUF_LENGTH    50
 unsigned char Ble_RXBuffer[BLE_BUF_LENGTH];
static unsigned char Ble_TXBuffer[BLE_BUF_LENGTH];
static unsigned char ble_recv_length = 0;    //主机接收数据长度
static unsigned char ble_recv_succeed_flag = 0;
static unsigned short  ble_recv_long_time = BLE_FREE_LONG_TIME;

unsigned char ble_key_state = 0;
unsigned char rf_key_state = 0;

void BleBlueTooth_SetName(unsigned char name_temp , unsigned short mcu_id_temp);  //上电时候需要调用,对码时候也要调用
void bluetooth_variable_length_protocol(const unsigned char *udata, unsigned char data_length);
unsigned char ble_online_flag;

void BleBlueTooth_Init(void)
{
	if(system_config.flags.bt_enable)
	{
		ble_online_flag = 1;
	}
	else
	{
		ble_online_flag = 0;
	}
	Delay_Ms(1200);//8258C测试1200ms的上电初始化时间
	BleBlueTooth_Normal_Mode();
}


void BleBlueTooth_SetName(unsigned char name_temp , unsigned short mcu_id_temp)  //上电时候需要调用,对码时候也要调用
{
	unsigned char mcu_id_arr[5] = {0};
	unsigned char i = 0;
	
	mcu_id_arr[0] = mcu_id_temp/10000 + '0';
	mcu_id_arr[1] = mcu_id_temp%10000/1000 + '0';
	mcu_id_arr[2] = mcu_id_temp%10000%1000/100 + '0';
	mcu_id_arr[3] = mcu_id_temp%10000%1000%100/10 + '0';
	mcu_id_arr[4] = mcu_id_temp%10000%1000%100%10 + '0';	
	
	for(i = 0; i < 5; i ++)
	{
		Ble_Set_Name[12+i] = mcu_id_arr[i];
	}
	
	Ble_Set_Name[11] = name_temp;

	BleBlueTooth_SendString(Ble_Set_Name,strlen((const char *)Ble_Set_Name));
}

void BleBlueTooth_SetDefault(unsigned short mcu_id_temp)  //上电时候需要调用,对码时候也要调用
{
	unsigned char name[] = "AT+NAMEHJ825x_00000\r\n";
	unsigned char mcu_id_arr[5] = {0};
	unsigned char i = 0;
	
	mcu_id_arr[0] = mcu_id_temp/10000 + '0';
	mcu_id_arr[1] = mcu_id_temp%10000/1000 + '0';
	mcu_id_arr[2] = mcu_id_temp%10000%1000/100 + '0';
	mcu_id_arr[3] = mcu_id_temp%10000%1000%100/10 + '0';
	mcu_id_arr[4] = mcu_id_temp%10000%1000%100%10 + '0';	
	
	for(i = 0; i < 5; i ++)
	{
		name[14+i] = mcu_id_arr[i];
	}
	
	BleBlueTooth_SendString(name,strlen((const char *)name));
}
unsigned char BleBlueTooth_Study_Mode(void)
{
	if(ble_online_flag == 0)
	{
		return 0;
	}
	if(ble_study_mode != 1)
	{
		ble_study_mode = 1;
		
		BleBlueTooth_SendString(Ble_Disconnect,strlen((const char *)Ble_Disconnect));
		Delay_Ms(100);
		BleBlueTooth_SetName('0',GetMcu_CrcID());
		Delay_Ms(100);
		BleBlueTooth_SendString(Ble_StartRadio_Cmd,strlen((const char *)Ble_StartRadio_Cmd));
		Delay_Ms(100);
	}
	if(ble_study_mode == 1)
	{
		if(1 == BleBlueTooth_Connect_State())
		{
			return 1;
		}
	}	
	
	return 0;
}

void BleBlueTooth_Offline_Mode(void)
{
	if(0 == BleBlueTooth_Connect_State())
	{
		if(ble_study_mode != 0) //蓝牙断开连接 才能发送AT指令,否则蓝牙模块不执行AT指令
		{
			ble_study_mode = 0;
			BleBlueTooth_SetDefault(GetMcu_CrcID());
		}
	}
}
void BleBlueTooth_Normal_Mode(void)
{
	if(ble_online_flag == 0)
	{
		BleBlueTooth_Offline_Mode();
		return;
	}
	if(0 == BleBlueTooth_Connect_State())
	{
		if(ble_study_mode != 0) //蓝牙断开连接 才能发送AT指令,否则蓝牙模块不执行AT指令
		{
			ble_study_mode = 0;
			BleBlueTooth_SetName('1',GetMcu_CrcID());
			Delay_Ms(100);
			BleBlueTooth_SendString(Ble_StopRadio_Cmd,strlen((const char *)Ble_StopRadio_Cmd));
			Delay_Ms(100);
			BleBlueTooth_SendString(Ble_StopRadio_Cmd,strlen((const char *)Ble_StopRadio_Cmd));
		}
	}
}

// 校验和函数
unsigned char calculate_checksum(const unsigned char *udata, unsigned char length)
{
  unsigned char i = 0;
  unsigned char sum = 0;
  for(i = 0; i < length; i++)
  {
    sum += udata[i];
  }
  return sum;
}
/**
 * @brief 清除串口数据
 * 
 */
void Ble_ClearRxBuffer(void)
{
	ble_recv_length = 0;
}

/**
 * @brief 获取当前串口接收数据
 * 
 * @param recv_data_ptr 串口数据指针
 * @param len 数据长度
 */
void Ble_GetRxBuffer(uint8_t **recv_data_ptr, uint16_t* len)
{
	*recv_data_ptr= Ble_RXBuffer;
	*len = ble_recv_length;
}

/**
 * @brief 判断串口接收数据是否超时
 * 
 * @return bool 串口接收数据是否超时
 */
bool Ble_IsReceiveTimeOut(void)
{
	return ble_recv_long_time >= BLE_FREE_LONG_TIME;
}



void BleBlueTooth_RxServer(unsigned char uart_recv_value)//需要用户放在串口接收中断  并将每次接收的字节传参
{
	unsigned char temp_i = 0;
	unsigned short recv_check_sum = 0; 
	if(ble_online_flag == 0)
	{
		return;
	}
	ble_recv_long_time = 0;	
	if(ble_recv_length < BLE_BUF_LENGTH)
	{
		Ble_RXBuffer[ble_recv_length ++] = uart_recv_value;
	}
	else
	{
		ble_recv_succeed_flag = 2;
		ble_recv_length = 0;
	}
	
	if(1 == ble_recv_length && Ble_RXBuffer[0] != 0X6E)
	{
		ble_recv_length = 0;
		ble_recv_succeed_flag = 0;
	}
  // 处理固定长度协议
  if((Ble_RXBuffer[1] != 0x20) && (Ble_RXBuffer[1] != 0x30) && (Ble_RXBuffer[1] != 0x40) && (ble_recv_length == 5))
  {
		
    recv_check_sum = calculate_checksum(Ble_RXBuffer,4);
    //recv_check_sum = Ble_RXBuffer[0]+Ble_RXBuffer[1]+Ble_RXBuffer[2]+Ble_RXBuffer[3];
		if(recv_check_sum%256 == Ble_RXBuffer[4])
		{
			ble_recv_succeed_flag = 1;		
		}
		else
		{
			ble_recv_succeed_flag = 0;
		}
    if(1 == ble_recv_succeed_flag)
		{
			ble_recv_succeed_flag = 0;
			/*---------------------电机控制指令----------------------------*/
			if(Ble_RXBuffer[1] == 0x01)
			{
				ble_key_state = 1;
				ble_key_value = ((unsigned short)Ble_RXBuffer[2] << 8) | Ble_RXBuffer[3];
				//夫妻床用下面的解析
				//ble_key_value = Ble_RXBuffer[3] & 0xff;
				//Set_Sync_Key_State(Ble_RXBuffer[2]);
			}
			/*--------------------RGB灯带参数设置----------------------------*/
			if(Ble_RXBuffer[1] == 0x0A) //APP 问询 控制盒  是否有彩灯设备
			{
				if(Ble_RXBuffer[2] == 0x00 && Ble_RXBuffer[3] == 0x01) //
				{
					if(Master_SearchIdleAdd(LIGHT_RGB_DEVICE_TYPE) != 0) //检测RGB等待是否存在
					{
						ble_report_set_event(BLE_ACK_EVENT, ACK_RGB_HAVE_EVENT); 		//回复有彩灯设备
					}
				}
			}
			//时间设置
			if(Ble_RXBuffer[1] == 0x0B) //时间
			{
				Light_Time_Set(&Light_RgbColour_Stu,((unsigned short)Ble_RXBuffer[2] << 8) | Ble_RXBuffer[3]);
				Light_Time_Set(&Light_OneColour_Stu,((unsigned short)Ble_RXBuffer[2] << 8) | Ble_RXBuffer[3]);
				
				light_para_set_event |= LIGHT_UBL_TIME_EVENT;
			}
			//颜色控制
			if(Ble_RXBuffer[1] == 0x0C) //红色
			{
				Light_RgbColour_Stu.light_colour[RGB_R_BIT] = Ble_RXBuffer[3];
			}
			if(Ble_RXBuffer[1] == 0x0D) //绿色 蓝色
			{
				Light_RgbColour_Stu.light_colour[RGB_G_BIT] = Ble_RXBuffer[2];
				Light_RgbColour_Stu.light_colour[RGB_B_BIT] = Ble_RXBuffer[3];
					
				light_para_set_event |= LIGHT_UBL_COLOUR_EVENT;
												//test-----
//								light_para_set_event |= LIGHT_UBL_MODE_EVENT;	 ///test
			
			}
			
			/*--------------------闹钟参数设置指令---------------------------*/
			if(Ble_RXBuffer[1] == 0x08) //APP  问询  控制盒  是否有闹钟
			{
				if((Ble_RXBuffer[2] == 0x01) && (Ble_RXBuffer[3] == 0x01))
				{
					ble_report_set_event(BLE_ACK_EVENT, ACK_ALARM_HAVE_EVENT);	 //回复有闹钟
				}
			}
			
			if(Ble_RXBuffer[1] == 0x05) //倒计时分钟数低八位
			{
				alarm_clock_min_l = Ble_RXBuffer[2];
				alarm_clock_days  = Ble_RXBuffer[3];
			}
			if(Ble_RXBuffer[1] == 0x06) //倒计时分钟数高八位+运行指令
			{
				RTC_Alarm_Stu.alarm_cmd = Ble_RXBuffer[3];
				
				alarm_clock_min_h = Ble_RXBuffer[2];
				RTC_Alarm_Stu.alarm_down_minutes = ((unsigned short)alarm_clock_min_h << 8) | (alarm_clock_min_l);
				RTC_Alarm_Stu.alarm_down_minutes = RTC_Alarm_Stu.alarm_down_minutes * (alarm_clock_days + 1);
				
				
				if(RTC_Alarm_Stu.alarm_down_minutes > 0) 
				{
					ble_report_set_event(BLE_ACK_EVENT, ACK_ALARM_SET_EVENT);
					
					rtc_para_set_event |= RTC_ALARM_TIME_SET_EVENT;
				}
				else
				{
					ble_report_set_event(BLE_ACK_EVENT, ACK_ALARM_CLEAR_EVENT);

					rtc_para_set_event |= RTC_ALARM_CANCLE_EVENT;
				}
				
				alarm_clock_min_h = 0;
				alarm_clock_min_l = 0;
				alarm_clock_days = 0;
			}
				
			if(Ble_RXBuffer[1] == 0x13)  //同步当前实时时间 时，分
			{
				RTC_Time_Stu.hour = Ble_RXBuffer[2];
				RTC_Time_Stu.min = Ble_RXBuffer[3];
			}
			if(Ble_RXBuffer[1] == 0x14)
			{
				if(Ble_RXBuffer[3] == 0xFF)  // //同步当前实时时间 秒
				{
					RTC_Time_Stu.sec = Ble_RXBuffer[2];
					rtc_para_set_event |= RTC_CTS_TIME_UPDATA_EVENT;
				}
				else if(Ble_RXBuffer[3] == 0xFE) //闹钟是单次还是重复
				{
					RTC_Alarm_Stu.alarm_mode = Ble_RXBuffer[2]; //
					rtc_para_set_event |= RTC_ALARM_MODE_SET_EVENT;
				}
			}			
      if(Ble_RXBuffer[1] == 0x15) //APP 问是否有音乐阵子
      {
        if((Ble_RXBuffer[2] == 0x01) && (Ble_RXBuffer[3] == 0x01))
        {
					if(Master_SearchIdleAdd(MUSIC_DEVICE_TYPE) != 0) //检测音乐阵子是否存在
					{
						ble_report_set_event(BLE_ACK_EVENT, ACK_MUSIC_HAVE_EVENT);
					}
        }
      }
      if(Ble_RXBuffer[1] == 0x88) //APP
      {
        if(Ble_RXBuffer[2] == 0xD1) //音量设置
        {
          User_SetMusic_Volume(Ble_RXBuffer[3]);
        }
        if(Ble_RXBuffer[2] == 0xD2) //随振强度
        {
          User_SetFollowInts_Level(Ble_RXBuffer[3]);
        }
      }
    }
		ble_recv_length = 0;
		/*--------------------APP下发音乐阵子控制参数设置----------------------------*/
  }
  // 处理可变长度协议
  if((Ble_RXBuffer[1] == 0x20) && (ble_recv_length >= 3))
  {
		ble_recv_long_time = 0;
    // 如果协议长度小于3个字节，忽略当前数据包，准备接收下一个数据包
    if(Ble_RXBuffer[2] < 3)
    {
      ble_recv_length = 0;
    }
    if(ble_recv_length >= Ble_RXBuffer[2])
    {
      recv_check_sum = calculate_checksum(Ble_RXBuffer, ble_recv_length - 1);
      if(recv_check_sum == Ble_RXBuffer[ble_recv_length - 1])
      {
        ble_recv_succeed_flag = 1;
      }
      else
      {
        ble_recv_succeed_flag = 0;
      }
      if(1 == ble_recv_succeed_flag)
      {
        ble_recv_succeed_flag = 0;
        bluetooth_variable_length_protocol(Ble_RXBuffer,ble_recv_length);
      }
				ble_recv_length = 0;
    }
  }
	//	处理射频数据	
	if((Ble_RXBuffer[1] == 0x30) && (ble_recv_length == 7))
	{
		ble_recv_long_time = 0;
    recv_check_sum = calculate_checksum(Ble_RXBuffer,6);
    //recv_check_sum = Ble_RXBuffer[0]+Ble_RXBuffer[1]+Ble_RXBuffer[2]+Ble_RXBuffer[3];
		if(recv_check_sum%256 == Ble_RXBuffer[6])	
		{
			if((Ble_RXBuffer[2] == 0) && (Ble_RXBuffer[3] == 0) && (Ble_RXBuffer[4] == 0) && (Ble_RXBuffer[5] == 0))
			{
				rf_key_state = 0;
			}
			else
			{
				rf_key_state = 1;
				HJ_A7105_CODE[0] = Ble_RXBuffer[2];
				HJ_A7105_CODE[1] = Ble_RXBuffer[3];
				HJ_A7105_CODE[2] = Ble_RXBuffer[4];
				HJ_A7105_CODE[3] = Ble_RXBuffer[5];
			}
		}
		ble_recv_length = 0;		
	}
}
// 解析变长协议
void bluetooth_variable_length_protocol(const unsigned char *udata, unsigned char data_length)
{
  // 检查校验和
//  unsigned char command = 0;
  // 提取数据负载（如果存在）
  const unsigned char *payload = NULL;
  unsigned char payload_length = 0;
  // 提取命令
//  command = udata[1];

  if(data_length > 4)
  {
    unsigned char i = 0;
    payload = udata + 3;
    payload_length = data_length - 4; // 减去头部和校验位
  }

  switch(payload[0])
  {
    //motor
    case 0x01:
    {
      switch(payload[1])
      {
				case 0x00://所有
				{
					switch(payload[2])
					{
						case 0x00: //读取
						{
							ble_report_set_event(BLE_REPORT_EVENT, REPORT_MOTOR_CMD_EVENT);
						}break;
					}
				}break;				
        //state
        case 0x01:
        {
          switch(payload[2])
          {
            //read
            case 0x00:
            {

            } break;
            //write
            case 0x01:
            {
              unsigned char motor_no = payload[3];
              unsigned char motor_func = payload[4];
              switch(motor_no)
              {
                case 0x01:
                {

                }
                break;
                case 0x02:
                {

                }
                break;
              }
            }
            break;
          }
        }
        break;
        //position
        case 0x04:
        {
          switch(payload[2])
          {
            //read
            case 0x00:
            {

            } break;
            //write
            case 0x01:
            {

            }
            break;
          }
        }
        break;
      }
    }break;		
		//massage
		case 0x02:
		{
			unsigned char mass_ints[4] = {0xff, 0xff, 0xff, 0xff};
			switch(payload[1])
			{
				case 0x00://所有
				{
					switch(payload[2])
					{
						case 0x00: //读取
						{
							ble_report_set_event(BLE_ACK_EVENT, ACK_MASSAGE_MODE_EVENT);
							ble_report_set_event(BLE_ACK_EVENT, ACK_MASSAGE_INTS_EVENT);
							ble_report_set_event(BLE_ACK_EVENT, ACK_MASSAGE_TIME_EVENT);
							ble_report_set_event(BLE_ACK_EVENT, ACK_MUSIC_FOLLOW_INITS_EVENT);
						}break;
					}
				}break;
				case 0x01://强度
				{
					switch(payload[2])
					{
						case 0x00: //读取
							ble_report_set_event(BLE_ACK_EVENT, ACK_MASSAGE_INTS_EVENT);
							break;
						case 0x01://写
						{
							unsigned char mass_no = payload[3];
							unsigned char mass_class = payload[4];	
							switch(mass_no)		
							{	
								case 1:	//背部								
								{
									switch(mass_class)
									{
										case 0:{mass_ints[1] = MSGR_INTS_ZERO_LEVEL;}break;
										case 1:{mass_ints[1] = MSGR_INTS_ONE_LEVEL;}break;
										case 2:{mass_ints[1] = MSGR_INTS_TWO_LEVEL;}break;
										case 3:{mass_ints[1] = MSGR_INTS_THREE_LEVEL;}break;
									}
									User_SetMassage_Mode(mass_ints, 0xff, 0xff);	
								}break;
								case 2:	//腿部								
								{
									switch(mass_class)
									{
										case 0:{mass_ints[2] = MSGR_INTS_ZERO_LEVEL;}break;
										case 1:{mass_ints[2] = MSGR_INTS_ONE_LEVEL;}break;
										case 2:{mass_ints[2] = MSGR_INTS_TWO_LEVEL;}break;
										case 3:{mass_ints[2] = MSGR_INTS_THREE_LEVEL;}break;		
									}
									User_SetMassage_Mode(mass_ints, 0xff, 0xff);
								}break;											
							}
						}break;
					}				
				}
				break;
				case 0x02://模式
				{
					switch(payload[2])
					{
						case 0x00: //读取
							ble_report_set_event(BLE_ACK_EVENT, ACK_MASSAGE_MODE_EVENT);
							break;
						case 0x01://写
						{
							switch(payload[4])
							{
								case 0:{
									mass_ints[1] = MSGR_INTS_ZERO_LEVEL;
									mass_ints[2] = MSGR_INTS_ZERO_LEVEL;
									mass_ints[3] = MSGR_INTS_ZERO_LEVEL;
									User_SetMassage_Mode(mass_ints, MSGR_FOLLOW_MODE, 0xff);									
								}break;
								case 1:
								{
									User_SetMassage_Mode(mass_ints, MSGR_CONSTANT_MODE, 0xff);
								}break;
								case 2:
								{
									User_SetMassage_Mode(mass_ints, MSGR_PULSE_MODE, 0xff);
								}break;
								case 3:
								{
									User_SetMassage_Mode(mass_ints, MSGR_WAVE_MODE, 0xff);
								}break;
							}
						}break;
					}				
				}break;
				case 0x03://时间
				{
					switch(payload[2])
					{
						case 0x00: //读取
							ble_report_set_event(BLE_ACK_EVENT, ACK_MASSAGE_TIME_EVENT);
							break;
						case 0x01://写
						{
							unsigned char mass_time = payload[4];
							User_SetMassage_Mode(mass_ints, 0xff, mass_time);
						}
							break;
					}				
				}break;				
				case 0x05://随震强度
				{
					switch(payload[2])
					{
						case 0x00: //读取
							
							break;
						case 0x01://写
						{
							switch(payload[4])
							{
								case 0:
								{
									User_SetFollowInts_Level(0);	
								}break;
								case 1:
								{
									User_SetFollowInts_Level(1);	
								}break;
								case 2:
								{
									User_SetFollowInts_Level(2);	
								}break;
								case 3:
								{
									User_SetFollowInts_Level(3);	
								}break;
							}
						}break;
					}				
				}break;
				case 0x06://开关
				{
					switch(payload[2])
					{
						case 0x00: //读取
							break;
						case 0x01://写
						{
						}break;
					}				
				}break;				
			}
		}break;	
		//ubl
		case 0x03:
		{
			switch(payload[1])
			{
				case 0x00://所有
				{
					switch(payload[2])
						{
							case 0x00: //读取
							{		
								ble_report_set_event(BLE_ACK_EVENT, ACK_BORAD_STATE_EVENT);
							}break;
						}				
				}break;
				case 0x01://开关
				{
					switch(payload[2])
					{
						case 0x00: //读取

							break;
						case 0x01://写
						{
						}break;
					}				
				}break;
				case 0x02://时间
				{
					switch(payload[2])
					{
						case 0x00: //读取

							break;
						case 0x01://写
						{
						}break;
					}
				}break;
				case 0x03://灯牌
				{
					switch(payload[2])
					{
						case 0x00: //读取
							ble_report_set_event(BLE_ACK_EVENT, ACK_BORAD_STATE_EVENT);
							break;
						case 0x01://写
						{
							switch (payload[4])
							{
								case 0x00:
								{
									led_board_state = 1;	
									light_para_set_event |= LIGHT_BOARD_STATE_EVENT;	
								}break;
								case 0x01:
								{
									led_board_state = 0;	
									light_para_set_event |= LIGHT_BOARD_STATE_EVENT;
								}break;
							}
						}break;
					}
				}break;
			}

		}break;	
		//Rgb
		case 0x04:
		{
			switch(payload[1])
			{
				case 0x00://所有
				{
					switch(payload[2])
					{
						case 0x00: //读取
						{
							ble_report_set_event(BLE_ACK_EVENT, ACK_RGB_STATE_CHANGE_EVENT);
							ble_report_set_event(BLE_ACK_EVENT, ACK_RGB_COLOR_CHANGE_EVENT);
							ble_report_set_event(BLE_ACK_EVENT, ACK_RGB_TIME_CHANGE_EVENT);
							ble_report_set_event(BLE_ACK_EVENT, ACK_RGB_MODE_CHANGE_EVENT);
							ble_report_set_event(BLE_ACK_EVENT, ACK_RGB_BREATH_MODE_EVENT);
							ble_report_set_event(BLE_ACK_EVENT, ACK_RGB_BRIGHTNESS_EVENT);
						}break;
					}
				}break;
				case 0x01:
				{
					switch(payload[2])
					{
						case 0x00: //读取
						{
							ble_report_set_event(BLE_ACK_EVENT, ACK_RGB_COLOR_CHANGE_EVENT);
						}break;
						case 0x01://写
						{
							  Led_RgbColour_Hex_Set(payload[4],payload[5],payload[6]);
							//test
//							Light_RgbColour_Stu.light_breath_time[0] = 0;
//							Light_RgbColour_Stu.light_breath_time[1] = 4;
//							Light_RgbColour_Stu.light_colour[RGB_R_BIT] = payload[4];
//							Light_RgbColour_Stu.light_colour[RGB_G_BIT] = payload[5];
//							Light_RgbColour_Stu.light_colour[RGB_B_BIT] = payload[6];							
//							//触发模式事件
//							light_para_set_event |= LIGHT_UBL_BREATH_MODE_EVENT;	
						}break;
					}
				}break;
				case 0x02:
				{
					unsigned long time_min;	
					unsigned long time_sec;					
					switch(payload[2])
					{
						case 0x00: //读取
						{
							ble_report_set_event(BLE_ACK_EVENT, ACK_RGB_TIME_CHANGE_EVENT);
						}break;
						case 0x01://写
						{
							time_min = payload[4]*256 + payload[5];	
							time_sec = time_min *60;		
							Light_Time_Set(&Light_RgbColour_Stu,time_sec);
							light_para_set_event |= LIGHT_UBL_TIME_EVENT;
						}break;
					}			
				}break;
				case 0x04:
				{
					switch(payload[2])
					{
						case 0x00: //读取
						{
							ble_report_set_event(BLE_ACK_EVENT, ACK_RGB_STATE_CHANGE_EVENT);
						}break;
						case 0x01://写
						{
							  if(payload[4] == 0)
								{
									light_para_set_event |= LIGHT_UBL_OFF_EVENT;
								}
								else
								{
									light_para_set_event |= LIGHT_UBL_ON_EVENT;
								}
						}break;
					}
				}break;
				case 0x05://rgb模式
				{
					switch(payload[2])
					{
						case 0x00: //读取
						{
							ble_report_set_event(BLE_ACK_EVENT, ACK_RGB_MODE_CHANGE_EVENT);
						}break;
						case 0x01://写
						{
							Light_RgbColour_Stu.light_colour_mode[MODE_TYPE_BIT] = payload[4];
							Light_RgbColour_Stu.light_colour_mode[MODE_ORDER_BIT] = payload[5];
							//触发模式事件
							light_para_set_event |= LIGHT_UBL_MODE_EVENT;	    
						}break;
					}
				}break;
				case 0x06://呼吸
				{
					switch(payload[2])
					{
						case 0x00: //读取
						{
							ble_report_set_event(BLE_ACK_EVENT, ACK_RGB_BREATH_MODE_EVENT);
						}break;
						case 0x01://写
						{
							Light_RgbColour_Stu.light_breath_time[0] = 0;
							Light_RgbColour_Stu.light_breath_time[1] = 4;
							//触发模式事件
							light_para_set_event |= LIGHT_UBL_BREATH_MODE_EVENT;	    
						}break;
					}
				}break;		
				case 0x08://亮度
				{
					switch(payload[2])
					{
						case 0x00: //读取
						{
							ble_report_set_event(BLE_ACK_EVENT, ACK_RGB_BRIGHTNESS_EVENT);
						}break;
						case 0x01://写
						{
							Light_RgbColour_Stu.light_brightness = payload[4];
							//触发模式事件
							light_para_set_event |= LIGHT_UBL_BRIGHTNESS_EVENT;	    
						}break;
					}
				}break;					
			} 			
		}break;		
    // 闹钟协议处理
    case 0x05:
    {
      Alarm_Protocol(&Alarm_List_BLE,(unsigned char *)payload, payload_length);
    }break;
		//睡眠检测-离床感应开关
		case 0x06:
		{
			switch(payload[1])
			{
				case 0x03:
				{
					switch(payload[2])
					{
						case 0x00: //读取
						{
							ttl_vita_set_radioenable |= TTL_VITA_BLE_ID_READ_EVENT;
						}break;
					}
				}break;
			}
		}break;
    //lock
    case 0x07:
    {
      //state
      switch(payload[2])
      {
        //read
        case 0x00:
        {
        }
        break;
        //write
        case 0x01:
        {
//          Beep_SingSetPara(500,1);
          if(payload[3] == 1)//没加同步，没有提示音
          {
            sys_lock_state |= SYS_CHILD_LOCK_STATE;
          }
          else if(payload[3] == 0)
          {
            sys_lock_state &= ~SYS_CHILD_LOCK_STATE;
          }
          else
          {
            sys_lock_state = (~sys_lock_state)&SYS_CHILD_LOCK_STATE;
          }
        }break;
      }
    }break;		
		//额外设置-时间设置
		case 0x08:
    {
      switch(payload[1])
      {
        //init
        case 0x01:
        {
          
        }break;
				case 0x03:
				{
					RTC_Time_Stu.year =	payload[3];
					RTC_Time_Stu.month = 	payload[4];
					RTC_Time_Stu.day = 	payload[5];
					RTC_Time_Stu.hour = 	payload[6];
					RTC_Time_Stu.min = payload[7];
					RTC_Time_Stu.sec = 	payload[8];
					rtc_para_set_event |= RTC_CTS_TIME_UPDATA_EVENT;
				}break;
      }
    }break;
		//音乐阵子
		case 0x09:
		{
			switch(payload[1])
			{
				case 0x00://所有
				{
					switch(payload[2])
					{
						case 0x00: //读取
						{
							ble_report_set_event(BLE_ACK_EVENT, ACK_MUSIC_DEMO_EVENT);
							ble_report_set_event(BLE_ACK_EVENT, ACK_MUSIC_BLE_EVENT);
							ble_report_set_event(BLE_ACK_EVENT, ACK_MUSIC_PLAY_EVENT);
							ble_report_set_event(BLE_ACK_EVENT, ACK_MUSIC_FOLLOW_INITS_EVENT);
							ble_report_set_event(BLE_ACK_EVENT, ACK_MUSIC_VOL_SET_EVENT);
						}break;
					}
				}break;	
				case 0x01://播放暂停
				{
					switch(payload[2])
					{
						case 0x00: //读取
							ble_report_set_event(BLE_ACK_EVENT, ACK_MUSIC_PLAY_EVENT);
							break;
						case 0x01://写
						{
							switch(payload[4])
							{
								case 0: music_para_set_event |= MUSIC_PLAY_EVENT;break;
								case 1: music_para_set_event |= MUSIC_PAUSE_EVENT;break;
							}		
						}break;
					}				
				}break;				
				case 0x02://音量
				{
					switch(payload[2])
					{
						case 0x00: //读取
						{
							ble_report_set_event(BLE_ACK_EVENT, ACK_MUSIC_VOL_SET_EVENT);
						}break;
						case 0x01:
						{
							if(payload[3] == 0x01)
							{
								switch(payload[4])
								{
									case 0:music_para_set_event |= MUSIC_PRE_EVENT;break;
									case 1:music_para_set_event |= MUSIC_NEXT_EVENT;break;
								}
							}							
							else if(payload[3] == 0x02)
							{	
								User_SetMusic_Volume(payload[4]);
							}					
						}break;
					}
				}break;
				case 0x03://蓝牙关闭打开
				{
					switch(payload[2])
					{
						case 0x00: //读取
							ble_report_set_event(BLE_ACK_EVENT, ACK_MUSIC_BLE_EVENT);
							break;
						case 0x01://写
						{
							switch(payload[4])
							{
								case 0:
								{
									music_para_set_event |= MUSIC_BLE_OFF_EVENT;
								}break;
								case 1:
								{
									music_para_set_event |= MUSIC_BLE_ON_EVENT;
								}break;					
							}	
						}break;
					}				
				}break;	
				case 0x04://音乐播放源
				{
					switch(payload[2])
					{
						case 0x00: //读取
						{
							ble_report_set_event(BLE_ACK_EVENT, ACK_MUSIC_DEMO_EVENT);	
						}break;
						case 0x01://写
						{
							switch(payload[4])
							{
								case 0x00: music_para_set_event |= MUSIC_MUTE_OFF_EVENT;break;//蓝牙
								case 0x01: music_para_set_event |= MUSIC_DEMO_ON_EVENT;break;//USB模式
								case 0x02://白噪音
								{
									if(payload[5] == 0x01)
									{
										MusicalOsc_Stu.DemoMode_Source = MUSIC_DEMO_SOURCE_WHITE_NOISE;
										MusicalOsc_Stu.DemoMode_TrackState[0] = MUSIC_TRACK_1;
										music_para_set_event |= MUSIC_DEMO_SET_TRACK_EVENT;
									}
									if(payload[5] == 0x02)
									{
										MusicalOsc_Stu.DemoMode_Source = MUSIC_DEMO_SOURCE_WHITE_NOISE;
										MusicalOsc_Stu.DemoMode_TrackState[0] = MUSIC_TRACK_2;										
										music_para_set_event |= MUSIC_DEMO_SET_TRACK_EVENT;
									}
									if(payload[5] == 0x03)
									{
										MusicalOsc_Stu.DemoMode_Source = MUSIC_DEMO_SOURCE_WHITE_NOISE;
										MusicalOsc_Stu.DemoMode_TrackState[0] = MUSIC_TRACK_3;											
										music_para_set_event |= MUSIC_DEMO_SET_TRACK_EVENT;
									}									
								}break;
								case 0x03://demo
								{
									if(payload[5] == 0x01)
									{
										MusicalOsc_Stu.DemoMode_Source = MUSIC_DEMO_SOURCE_MUSIC;
										MusicalOsc_Stu.DemoMode_TrackState[1] = MUSIC_TRACK_1;										
										music_para_set_event |= MUSIC_DEMO_SET_TRACK_EVENT;
									}
									if(payload[5] == 0x02)
									{
										MusicalOsc_Stu.DemoMode_Source = MUSIC_DEMO_SOURCE_MUSIC;
										MusicalOsc_Stu.DemoMode_TrackState[1] = MUSIC_TRACK_1;											
										music_para_set_event |= MUSIC_DEMO_SET_TRACK_EVENT;
									}
									if(payload[5] == 0x03)
									{
										MusicalOsc_Stu.DemoMode_Source = MUSIC_DEMO_SOURCE_MUSIC;
										MusicalOsc_Stu.DemoMode_TrackState[1] = MUSIC_TRACK_1;											
										music_para_set_event |= MUSIC_DEMO_SET_TRACK_EVENT;
									}						
								}break;								
							}
						}break;
					}				
				}break;						
				case 0x05://上一曲下一区
				{
					switch(payload[2])
					{
						case 0x00: //读取
							break;
						case 0x01://写
						{
							switch(payload[4])
							{
								case 0:music_para_set_event |= MUSIC_PRE_EVENT;break;
								case 1:music_para_set_event |= MUSIC_NEXT_EVENT;break;
							}
						}break;
					}				
				}break;					
			}			
		}break;	
		case 0x0b:	
		{
			switch(payload[1])
			{
				case 0x00://读取
				{
					switch(payload[2])
					{
						case 0x00: //读取
						{
							ble_report_set_event(BLE_ACK_EVENT, ACK_SYNC_MODE_EVENT);
						}break;
					}
				}break;
				case 0x02://写
				{
					switch(payload[2])
					{
						case 0x00: //读取
						{
							ble_report_set_event(BLE_ACK_EVENT, ACK_SYNC_MODE_EVENT);
						}break;
						case 0x01://写
						{
							switch(payload[3])
							{
								case 0:
								{
								
								}break;
								case 1:
								{
									switch(payload[4])
									{
										case 0:
										{
											Set_Sync_Run_Mode(1);			
											ble_report_set_event(BLE_REPORT_EVENT, REPORT_SYNC_MODE_EVENT);							
										}break;
										case 1:
										{
											Set_Sync_Run_Mode(0);										
											ble_report_set_event(BLE_REPORT_EVENT, REPORT_SYNC_MODE_EVENT);							
										}break;
									}
								}break;					
							}	
						}break;
					}
				}break;
			}
		}break;		
		//风扇
		case 0x0e:
		{
			unsigned char fan_ints[3] = {0xff, 0xff, 0xff};
			unsigned char fan_dir[3] = {0xff, 0xff, 0xff};
			switch(payload[1])
			{
				case 0x00://所有
				{
					switch(payload[2])
					{
						case 0x00: //读取
						{
							ble_report_set_event(BLE_ACK_EVENT, ACK_FAN_MODE_EVENT);
							ble_report_set_event(BLE_ACK_EVENT, ACK_FAN_INTS_EVENT);
							ble_report_set_event(BLE_ACK_EVENT, ACK_FAN_TIME_EVENT);
							ble_report_set_event(BLE_ACK_EVENT, ACK_FAN_DIR_EVENT);
						}break;
					}
				}break;
				case 0x01://强度
				{
					switch(payload[2])
					{
						case 0x00: //读取
							ble_report_set_event(BLE_ACK_EVENT, ACK_FAN_INTS_EVENT);
							break;
						case 0x01://写
						{
							unsigned char fan_no = payload[3];
							unsigned char fan_class = payload[4];	
							switch(fan_no)		
							{	
								case 1:	//背部								
								{
									switch(fan_class)
									{
										case 0:{fan_ints[1] = FAN_INTS_ZERO_LEVEL;}break;
										case 1:{fan_ints[1] = FAN_INTS_ONE_LEVEL;}break;
										case 2:{fan_ints[1] = FAN_INTS_TWO_LEVEL;}break;
										case 3:{fan_ints[1] = FAN_INTS_THREE_LEVEL;}break;
									}
									User_SetFan_Mode(fan_ints, 0xff, fan_dir, 0xff);	
								}break;
								case 2:	//腿部								
								{
									switch(fan_class)
									{
										case 0:{fan_ints[2] = FAN_INTS_ZERO_LEVEL;}break;
										case 1:{fan_ints[2] = FAN_INTS_ONE_LEVEL;}break;
										case 2:{fan_ints[2] = FAN_INTS_TWO_LEVEL;}break;
										case 3:{fan_ints[2] = FAN_INTS_THREE_LEVEL;}break;
									}
									User_SetFan_Mode(fan_ints, 0xff, fan_dir, 0xff);	
								}break;											
							}
						}break;
					}				
				}
				break;
				case 0x02://模式
				{
					switch(payload[2])
					{
						case 0x00: //读取
							ble_report_set_event(BLE_ACK_EVENT, ACK_FAN_MODE_EVENT);
							break;
						case 0x01://写
						{
							switch(payload[4])
							{
								case 0:{
									fan_ints[1] = FAN_INTS_ZERO_LEVEL;
									fan_ints[2] = FAN_INTS_ZERO_LEVEL;
									User_SetFan_Mode(fan_ints, FAN_NONE_MODE, fan_dir, 0xff);									
								}break;
								case 1:
								{
									User_SetFan_Mode(fan_ints, FAN_CONSTANT_MODE, fan_dir, 0xff);
								}break;
								case 2:
								{
									User_SetFan_Mode(fan_ints, FAN_PULSE_MODE, fan_dir, 0xff);
								}break;
								case 3:
								{
									User_SetFan_Mode(fan_ints, FAN_WAVE_MODE, fan_dir, 0xff);
								}break;
							}
						}break;
					}				
				}break;
				case 0x03://时间
				{
					switch(payload[2])
					{
						case 0x00: //读取
							ble_report_set_event(BLE_ACK_EVENT, ACK_FAN_TIME_EVENT);
							break;
						case 0x01://写
						{
							unsigned char fan_time = payload[4];
							User_SetFan_Mode(fan_ints, 0xff, fan_dir, fan_time);							
						}break;
					}				
				}break;		
				case 0x04://方向
				{
					switch(payload[2])
					{
						case 0x00: //读取
							ble_report_set_event(BLE_ACK_EVENT, ACK_FAN_DIR_EVENT);
							break;
						case 0x01://写
						{
							unsigned char fan_no = payload[3];
							switch(fan_no)		
							{
								case 0:
								{
									if(payload[4] == 1)
									{
										fan_dir[1] = FAN_FORWARD;	
										fan_dir[2] = fan_dir[1];	
									}
									else if(payload[4] == 2)
									{
										fan_dir[1] = FAN_BACKWARD;	
										fan_dir[2] = fan_dir[1];									
									}
									User_SetFan_Mode(fan_ints, 0xff, fan_dir, 0xff);									
								}break;
								case 1:	//背部								
								{
									fan_dir[1] = payload[4];	
									User_SetFan_Mode(fan_ints, 0xff, fan_dir, 0xff);	
								}break;
								case 2:	//腿部								
								{
									fan_dir[2] = payload[4];	
									User_SetFan_Mode(fan_ints, 0xff, fan_dir, 0xff);	
								}break;											
							}							
						}break;
					}				
				}break;					
				case 0x05://开关
				{
					switch(payload[2])
					{
						case 0x00: //读取
							break;
						case 0x01://写
						{
						}break;
					}				
				}break;				
			}
		}break;	
		case 0x0f://哄睡设置
		{
			switch(payload[1])
			{
				case 0x00://所有
				{
					switch(payload[2])
					{
						case 0x00: //读取
						{
							ble_report_set_event(BLE_ACK_EVENT, ACK_DEMO_SLEEP_TIME_EVENT);
							ble_report_set_event(BLE_ACK_EVENT, ACK_DEMO_RUN_EVENT);
						}break;
					}
				}break;
				case 0x01://运行哄睡
				{
					switch(payload[2])
					{
						case 0x00: //读取
						{
							ble_report_set_event(BLE_ACK_EVENT, ACK_DEMO_RUN_EVENT);
						}break;
						case 0x01://写
						{
							if(payload[3] == 0x01)
							{							
								if((Motor_DemoMode_RunState() == 1)  || (Motor_DemoMode_RunState() == 3)) 
								{
									Motor_DemoMode_ClearPara();
									GetSet_Motor_Ctr_Cmd(KEY_FLAT);
								}		
								else if(Motor_DemoMode_RunState() == 2)
								{
									Motor_DemoMode_ClearPara();
									GetSet_Motor_Ctr_Cmd(0);
								}
								else
								{									
									if(payload[4] != 0)
									{
										demo_run_time = demo_run_time_save;
										Motor_OneClickCmd_Set(KEY_DEMO1_MODE);
									}
									else
									{
										Motor_OneClickCmd_Set(KEY_MOTOR_STOP);
									}
								}
								motor_para_set_event |= MOTOR_CMD_RUN_EVENT;
							}
							if(payload[3] == 0x02)
							{								
								if((Motor_DemoMode_RunState() == 1)  || (Motor_DemoMode_RunState() == 3)) 
								{
									Motor_DemoMode_ClearPara();
									GetSet_Motor_Ctr_Cmd(KEY_FLAT);
								}		
								else if(Motor_DemoMode_RunState() == 2)
								{
									Motor_DemoMode_ClearPara();
									GetSet_Motor_Ctr_Cmd(0);
								}
								else
								{						
									if(payload[4] != 0)
									{
										Motor_OneClickCmd_Set(KEY_DEMO2_MODE);
									}
									else
									{
										Motor_OneClickCmd_Set(KEY_MOTOR_STOP);
									}
								}
								motor_para_set_event |= MOTOR_CMD_RUN_EVENT;
							}							
							ble_report_set_event(BLE_REPORT_EVENT, REPORT_DEMO_RUN_EVENT);	
							ble_report_set_event(BLE_REPORT_EVENT, REPORT_DEMO_SLEEP_TIME_EVENT);							
						}break;
					}
				}break;
				case 0x02://时间
				{
					switch(payload[2])
					{
						case 0x00: //读取
						{
							ble_report_set_event(BLE_ACK_EVENT, ACK_DEMO_SLEEP_TIME_EVENT);
						}break;
						case 0x01://写
						{
							if(payload[5] != 0)
							{
								Motor_Demo_ClearTime();
								Light_Clear_TimeCount(&Light_RgbColour_Stu);
								Music_Time_ClearCount();
								demo_run_time = payload[5];						
								motor_para_set_event |= MOTOR_DEMO_SLEEP_RUN_EVENT;
							}					
						}break;
					}
				}break;
			}
		}break;			  
	}
}

void BleBlueTooth_TimerManager(void)
{
	if(ble_online_flag == 0)
	{
		return;
	}
	//超时无数据
	ble_recv_long_time ++;
	if(ble_recv_long_time >= BLE_FREE_LONG_TIME)
	{
		ble_recv_long_time = 60000;
	}
	//断帧
	if(ble_recv_long_time >= BLE_BROKEN_FRAME_TIME)
	{
		ble_recv_length = 0;
	}
}
unsigned char Ble_Comm_Free(void)
{
  if(ble_recv_long_time >= BLE_FREE_LONG_TIME || ble_online_flag == 0)
  {
    return 1;
  }
  return 0;
}
	unsigned char key_state = 0;	
unsigned char BleBlueTooth_Get_KeyState(void)
{
	key_state = 0;
	if(ble_online_flag == 0)
	{
		return 0;
	}
	if(1 == ble_key_state)
	{
		ble_key_state = 0;
		key_state = 1;
	}
	if(1 == rf_key_state)
	{
		rf_key_state = 0;
		key_state = 2;
	}
	if(1 == Ble_Comm_Free())
	{
		ble_key_value = 0;
		rf_key_state = 0;
		HJ_A7105_CODE[0] = 0;
		HJ_A7105_CODE[1] = 0;
		HJ_A7105_CODE[2] = 0;
		HJ_A7105_CODE[3] = 0;	
		key_state = 3;
	}
	
	return key_state;
}


