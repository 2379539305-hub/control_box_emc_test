#include "app_linbus.h"

#include "delay.h"
#include "driver_uart.h"
#include "driver_beep.h"
#include "driver_rtc.h"

#include "modul_ttlbus.h"

#include "modul_light.h"
#include "modul_musicmsgr.h"

#include "app_light.h"
#include "app_motor.h"
#include "app_msgr.h"
#include "app_music.h"
#include "app_light.h"
#include "app_vita.h"
#include "app_rtc.h"
#include "app_heat.h"
#include "app_comm.h"
#include "app_fan.h"
#include "app_backhual.h"
#include "app_config.h"

//lin接收缓冲
unsigned char LIN_BUS_RXBuffer[LIN_BUF_LENGTH]; 
//lin发送缓冲
unsigned char LIN_BUS_TXBuffer[LIN_BUF_LENGTH];

//串口底层解析
static unsigned char lin_recv_length = 0;    //主机接收数据长度
static unsigned char lin_recv_succeed_flag = 0; //主机接收成功标志位
static unsigned short lin_recv_long_time = LIN_FREE_LONG_TIME;

static unsigned char lin_recv_enable = 1; //LIN串口允许接收标志位
static unsigned char lin_send_enable = 1; //LIN串口同步发送标志位
//lin接收键值
unsigned short lin_key_value = 0;

static unsigned char  lin_key_state = 0;

void LIN_RecvDataAnalyTask(void);
//lin串口发送数据
void LinBus_SendString(unsigned char *send_str,unsigned char length)
{
	if(1 == lin_send_enable)
	{
		lin_recv_enable = 0;
		if(system_config.flags.ble_mesh_sync_config == BLE_MESH_SYNC_DISABLE)
		{
			Uart4_SendString(send_str,length);
		}
		else
		{
			Uart5_SendString(send_str,length);
		}
		Delay_Ms(5);
		lin_recv_enable = 1;
	}
}
//获取lin键值
unsigned short Lin_Analy_KeyValue(void)
{
	return lin_key_value;
}
//lin串口接收中断协议解析
void LIN_Bus_RxServer(unsigned char uart_recv_value)
{
	unsigned char temp_i = 0;
	unsigned short recv_check_sum = 0; 

	if(lin_recv_enable != 1)  return;
	
	lin_recv_long_time = 0;
	//
	if(lin_recv_length < LIN_BUF_LENGTH)
	{
		LIN_BUS_RXBuffer[lin_recv_length ++] = uart_recv_value;
	}
	else
	{
		lin_recv_succeed_flag = 2;
		lin_recv_length = 0;
	}
	
	if(1 == lin_recv_length && LIN_BUS_RXBuffer[0] != 0XFA)
	{
		lin_recv_length = 0;
		lin_recv_succeed_flag = 0;
	}
	if(2 == lin_recv_length && LIN_BUS_RXBuffer[1] != 0X5A)
	{
		lin_recv_length = 0;
		lin_recv_succeed_flag = 0;		
	}
	
	if(lin_recv_length >= 3)
	{
		if(lin_recv_length >= LIN_BUS_RXBuffer[2])
		{
			for(temp_i = 2;temp_i < LIN_BUS_RXBuffer[2]-1;temp_i ++)
			{
				recv_check_sum += LIN_BUS_RXBuffer[temp_i];
			}
			if(recv_check_sum%256 == LIN_BUS_RXBuffer[lin_recv_length - 1])
			{
				lin_recv_succeed_flag = 1;
			}
			else
			{
				lin_recv_succeed_flag = 2;
			}
			
			lin_recv_length = 0;
		}	
	}
	//
	if(1 == lin_recv_succeed_flag)
	{
		lin_send_enable = 0;
		lin_recv_succeed_flag = 0;
		//解析指令
		LIN_RecvDataAnalyTask();
	}
}
//lin总线忙检测
unsigned char LIN_Check_Busy_Free(void)
{
  if(lin_recv_long_time>=LIN_FREE_LONG_TIME)
  {
    return 1;
  }
  return 0;
}

//lin总线定时器中断函数
void LIN_Bus_TimeManager(void)
{
  lin_recv_long_time ++;
	
  if(lin_recv_long_time >= LIN_FREE_LONG_TIME)
  {
    lin_recv_long_time = LIN_FREE_LONG_TIME;
		
		motor_para_set_event &= ~MOTOR_LIN_RUN_EVENT;
  }
	
	if(lin_recv_long_time >= LIN_BROKEN_FRAME_TIME)
	{
		lin_recv_length = 0;
	}
	//防止LIN接收之后 接着再返回控制
	if(0 == lin_send_enable)
	{
		if(msgr_para_set_event == 0x0000 && music_para_set_event == 0x0000 && light_para_set_event == 0x0000 && motor_para_set_event == 0X0000 && key_para_set_event == 0x0000)
		{
			lin_send_enable = 1;
		}
	}
}
//lin接收协议解析
void LIN_RecvDataAnalyTask(void)
{
	unsigned char temp_para = 0;
	
	unsigned char temp_i = 0;
	
	if(LIN_BUS_RXBuffer[LIN_SLAVE_TYPE_BIT] == 0x01) //控制盒
	{
		switch(LIN_BUS_RXBuffer[LIN_CODE_BIT])
		{
			//指令
			case 0x21:
			{
				if(LIN_BUS_RXBuffer[LIN_CMD_BIT] == 0x00) //问询
				{
					
				}
				if(LIN_BUS_RXBuffer[LIN_CMD_BIT] == 0x01) //控制
				{
					lin_key_state = 1;
					lin_key_value = ((unsigned short)LIN_BUS_RXBuffer[10] << 8) | LIN_BUS_RXBuffer[11];
					if(lin_key_value == KEY_MUSIC_MEM_HELP_SLEEP)
					{
						music_para_set_event |= MUSIC_DEMO_SAVE_EVENT;
					}
					else
					{
						motor_para_set_event |= MOTOR_LIN_RUN_EVENT;
					}
				}				
			}break;
			//推杆
			case 0x22:
			{
				if(LIN_BUS_RXBuffer[LIN_CMD_BIT] == 0x00) //问询
				{
					
				}
				if(LIN_BUS_RXBuffer[LIN_CMD_BIT] == 0x01) //控制
				{
					if(((LIN_BUS_RXBuffer[2] - 11) % 3) == 0) //数据格式正确
					{
						temp_para = (LIN_BUS_RXBuffer[2] - 11) / 3;
						
						if(temp_para >= 1)
						{
							if(LIN_BUS_RXBuffer[10] <= 6) //防止数组溢出
							{
								Motor_SyncTarget_HallArr[LIN_BUS_RXBuffer[10]] = ((short)LIN_BUS_RXBuffer[11] << 8) | LIN_BUS_RXBuffer[12];
							}
						}
						if(temp_para >= 2)
						{
							if(LIN_BUS_RXBuffer[13] <= 6) //防止数组溢出
							{
									Motor_SyncTarget_HallArr[LIN_BUS_RXBuffer[13]] = ((short)LIN_BUS_RXBuffer[14] << 8) | LIN_BUS_RXBuffer[15];						
							}
						}
						if(temp_para >= 3)
						{
							if(LIN_BUS_RXBuffer[16] <= 6) //防止数组溢出
							{
									Motor_SyncTarget_HallArr[LIN_BUS_RXBuffer[16]] = ((short)LIN_BUS_RXBuffer[17] << 8) | LIN_BUS_RXBuffer[18];									
							}
						}
						if(temp_para >= 4)
						{
							if(LIN_BUS_RXBuffer[19] <= 6) //防止数组溢出
							{
									Motor_SyncTarget_HallArr[LIN_BUS_RXBuffer[19]] = ((short)LIN_BUS_RXBuffer[20] << 8) | LIN_BUS_RXBuffer[21];
							}
						}
						if(temp_para >= 5)
						{
							if(LIN_BUS_RXBuffer[22] <= 6) //防止数组溢出
							{
									Motor_SyncTarget_HallArr[LIN_BUS_RXBuffer[22]] = ((short)LIN_BUS_RXBuffer[23] << 8) | LIN_BUS_RXBuffer[24];									
							}
						}
						if(temp_para >= 6)
						{
							if(LIN_BUS_RXBuffer[25] <= 6) //防止数组溢出
							{
									Motor_SyncTarget_HallArr[LIN_BUS_RXBuffer[25]] = ((short)LIN_BUS_RXBuffer[26] << 8) | LIN_BUS_RXBuffer[27];							
							}
						}						
					}
				}
			}break;
			//按摩器
			case 0x23:
			{
				if(LIN_BUS_RXBuffer[LIN_CMD_BIT] == 0x00) //问询
				{
					
				}
				if(LIN_BUS_RXBuffer[LIN_CMD_BIT] == 0x01) //控制
				{
					Msgr_Clear_TimeCount();
					if(Motor_DemoMode_RunState() == 1)
					{
						Motor_DemoMode_ClearPara();

						GetSet_Motor_Ctr_Cmd(KEY_FLAT);

					}
					else if(Motor_DemoMode_RunState() == 2)
					{
						Motor_DemoMode_ClearPara();

						GetSet_Motor_Ctr_Cmd(0);					
					}
					
					switch(LIN_BUS_RXBuffer[9])
					{
						case 0x21: //模式+时间
						{
							msgr_mode_set = LIN_BUS_RXBuffer[11];
							msgr_min_time_set = LIN_BUS_RXBuffer[12];
							
							msgr_para_set_event |= MSGR_MODE_TIME_EVENT;						
						}break;
						case 0x22: //随振强度
						{
							if(LIN_BUS_RXBuffer[11] >= MSGR_INTS_MAX_LEVEL)
							{
								Msgr_Ints_FlagArr[0] = MSGR_INTS_ZERO_LEVEL;
							}
							else
							{
								Msgr_Ints_FlagArr[0] = LIN_BUS_RXBuffer[11];
							}
							msgr_para_set_event |= MSGR_FOLLOW_INTS_EVENT;
						}break;
						case 0x23: //典型强度
						{				
							msgr_para_set_event |= MSGR_ALL_INTS_EVENT;				
							
							if(LIN_BUS_RXBuffer[10] == 0x00)
							{
								if(LIN_BUS_RXBuffer[11] >= MSGR_INTS_MAX_LEVEL)
								{
									memset(Msgr_Ints_FlagArr,MSGR_INTS_ZERO_LEVEL,sizeof(Msgr_Ints_FlagArr)/sizeof(Msgr_Ints_FlagArr[0]));
								}
								else
								{
									memset(Msgr_Ints_FlagArr,LIN_BUS_RXBuffer[11],sizeof(Msgr_Ints_FlagArr)/sizeof(Msgr_Ints_FlagArr[0]));
								}
							}
							else
							{
								if(((LIN_BUS_RXBuffer[2] - 11) % 2) == 0) //格式接收正确
								{
									temp_para = (LIN_BUS_RXBuffer[2] - 11) / 2;
									
									for(temp_i = 0;temp_i < temp_para;temp_i ++)
									{
										if(LIN_BUS_RXBuffer[10 + 2*temp_i] <= SYS_MSGR_NUM) //防止数组溢出
										{
											Msgr_Ints_FlagArr[LIN_BUS_RXBuffer[10 + 2*temp_i]] = LIN_BUS_RXBuffer[11 + 2*temp_i];
										}										
									}
								}
							}						
						}break;
						default:break;
					}
				}
			}break;
			//RGB灯
			case 0x24:
			{
				if(LIN_BUS_RXBuffer[LIN_CMD_BIT] == 0x00) //问询
				{
					
				}
				if(LIN_BUS_RXBuffer[LIN_CMD_BIT] == 0x01) //控制
				{
					switch(LIN_BUS_RXBuffer[9])
					{
						case 0x01: //灯牌
						{
							led_board_state = LIN_BUS_RXBuffer[11];
							
							light_para_set_event |= LIGHT_BOARD_STATE_EVENT;
						}break;						
						case 0x02: //单色灯开关
						{
							Light_OneColour_Stu.led_colour_state = LIN_BUS_RXBuffer[11];
							
							light_para_set_event |= LIGHT_UBL_SW_EVENT;
						}break;						
						case 0x22: //时间设置
						{
							Light_Time_Set(&Light_OneColour_Stu,((u16)LIN_BUS_RXBuffer[11] << 8) | LIN_BUS_RXBuffer[12]);
							Light_Time_Set(&Light_RgbColour_Stu,((u16)LIN_BUS_RXBuffer[11] << 8) | LIN_BUS_RXBuffer[12]);
							
							light_para_set_event |= LIGHT_UBL_TIME_EVENT;
						}break;
						case 0x23: //RGB灯颜色切换
						{
							Light_RgbColour_Stu.light_colour[RGB_R_BIT] = LIN_BUS_RXBuffer[11];
							Light_RgbColour_Stu.light_colour[RGB_G_BIT] = LIN_BUS_RXBuffer[12];
							Light_RgbColour_Stu.light_colour[RGB_B_BIT] = LIN_BUS_RXBuffer[13];
	
							light_para_set_event |= LIGHT_UBL_COLOUR_EVENT;
							
						}break;
						//床底灯LIN 解析
						case 0x24: //RGB灯特殊模式
						{
							Light_RgbColour_Stu.light_colour_mode[MODE_TYPE_BIT] = LIN_BUS_RXBuffer[11];
							Light_RgbColour_Stu.light_colour_mode[MODE_ORDER_BIT] = LIN_BUS_RXBuffer[12];
							
							light_para_set_event |= LIGHT_UBL_MODE_EVENT;
						}break;
						case 0x25: //RGB呼吸模式
						{
							Light_RgbColour_Stu.light_breath_time[0] = LIN_BUS_RXBuffer[11];
							Light_RgbColour_Stu.light_breath_time[1] = LIN_BUS_RXBuffer[12];
							Light_RgbColour_Stu.light_colour[RGB_R_BIT] = LIN_BUS_RXBuffer[13];
							Light_RgbColour_Stu.light_colour[RGB_G_BIT] = LIN_BUS_RXBuffer[14];
							Light_RgbColour_Stu.light_colour[RGB_B_BIT] = LIN_BUS_RXBuffer[15];
							light_para_set_event |= LIGHT_UBL_BREATH_MODE_EVENT;
						}break;			
						case 0x27: //RGB灯亮度
						{
							Light_RgbColour_Stu.light_brightness = LIN_BUS_RXBuffer[11];
							light_para_set_event |= LIGHT_UBL_BRIGHTNESS_EVENT;
						}break;								
						default:break;
					}
				}			
			}break;	
			case 0x25: //时间同步
			{
				switch(LIN_BUS_RXBuffer[9])
				{
					case 0x01: //常规设置
					{
						RTC_Time_Stu.hour = LIN_BUS_RXBuffer[10];
						RTC_Time_Stu.min =  LIN_BUS_RXBuffer[11];
						RTC_Time_Stu.sec =  LIN_BUS_RXBuffer[12];
						
						rtc_para_set_event |= RTC_CTS_TIME_UPDATA_EVENT;
					}break;
					case 0x02: //UTC时间
					{
					
					}break;
					case 0x03://运行闹钟
					{
						alarm_mode_value[0] = LIN_BUS_RXBuffer[10];
						alarm_mode_value[1] = LIN_BUS_RXBuffer[11];
						alarm_mode_value[2] = LIN_BUS_RXBuffer[12];
						alarm_mode_value[3] = LIN_BUS_RXBuffer[13];                      
						Motor_OneClickCmd_Set(KEY_ALARM_MODE);						
					}break;
					default:break;
				}
			}break;
			//同步锁
			case 0x26:
			{
				if(LIN_BUS_RXBuffer[LIN_CMD_BIT] == 0x00) //问询
				{
					
				}
				if(LIN_BUS_RXBuffer[LIN_CMD_BIT] == 0x01) //控制
				{
					lin_key_state = 1;
					sys_lock_state = ((unsigned short)LIN_BUS_RXBuffer[10] << 8) | LIN_BUS_RXBuffer[11];
//					motor_para_set_event |= MOTOR_LIN_RUN_EVENT;
				}				
			}break;
			case 0x27:
			{
				switch(LIN_BUS_RXBuffer[9])
				{
					case 0x01:
					{//运行状态

					}break;
					case 0x02:
					{//定时时间
						demo_run_time = LIN_BUS_RXBuffer[12];
						motor_para_set_event |= MOTOR_DEMO_SLEEP_RUN_EVENT;
					}break;
					case 0x03:
					{//哄睡步骤
						if(LIN_BUS_RXBuffer[10])
						{
							Set_Motor_Demo_Step(LIN_BUS_RXBuffer[11]);
						}
					}break;
					default:break;
				}
			}break;
			//音乐阵子
			case 0x30:
			{
				if(LIN_BUS_RXBuffer[LIN_CMD_BIT] == 0x00) //问询
				{
					
				}
				if(LIN_BUS_RXBuffer[LIN_CMD_BIT] == 0x01) //控制
				{
					switch(LIN_BUS_RXBuffer[9]) 
					{
						case 0x25: //音量控制
						{
							if(LIN_BUS_RXBuffer[10] == 0x00)
							{
								User_SetMusic_Volume(LIN_BUS_RXBuffer[11]);
							}
							if(LIN_BUS_RXBuffer[10] == 0x01)
							{
								music_para_set_event |= MUSIC_VOL_DCR_EVENT;
							}
							if(LIN_BUS_RXBuffer[10] == 0x02)
							{
								music_para_set_event |= MUSIC_VOL_ADD_EVENT;
							}							
						}break;
						case 0x26:  //曲目控制
						{
							if(LIN_BUS_RXBuffer[10] == 0x01)
							{
								music_para_set_event |= MUSIC_PRE_EVENT;
							}
							if(LIN_BUS_RXBuffer[10] == 0x02)
							{
								music_para_set_event |= MUSIC_NEXT_EVENT;
							}							
						}break; 
						case 0x27: //播放控制
						{								
							if(LIN_BUS_RXBuffer[10] == 0x00)
							{
								music_para_set_event |= MUSIC_PAUSE_EVENT;
							}
							if(LIN_BUS_RXBuffer[10] == 0x01)
							{
								music_para_set_event |= MUSIC_PLAY_EVENT;	
							}		
						}break;
						case 0x29: //蓝牙状态控制
						{
							if(LIN_BUS_RXBuffer[10] == 0x01) //蓝牙开关
							{
								if(LIN_BUS_RXBuffer[11] == 0x00)
								{
									music_para_set_event |= MUSIC_BLE_OFF_EVENT;
								}
								if(LIN_BUS_RXBuffer[11] == 0x01)
								{
									music_para_set_event |= MUSIC_BLE_ON_EVENT;
								}									
							}
							if(LIN_BUS_RXBuffer[10] == 0x02) //TWS开关
							{
								if(LIN_BUS_RXBuffer[11] == 0x00)
								{
									music_para_set_event |= MUSIC_TWS_OFF_EVENT;
								}
								if(LIN_BUS_RXBuffer[11] == 0x01)
								{
									music_para_set_event |= MUSIC_TWS_ON_EVENT;
								}															
							}
							if(LIN_BUS_RXBuffer[10] == 0x03) //列表操作
							{
				
							}							
						}break;
						case 0x2A: break; //将功率模式
						case 0x2B:  //工作模式切换
						{
							if(LIN_BUS_RXBuffer[10] == 0x00) //演示模式关
							{
								music_para_set_event |= MUSIC_DEMO_OFF_EVENT;
							}
							if(LIN_BUS_RXBuffer[10] == 0x01) //演示模式开
							{
								music_para_set_event |= MUSIC_DEMO_ON_EVENT;
							}		
							if(LIN_BUS_RXBuffer[10] == 0x04) //
							{
								if(LIN_BUS_RXBuffer[11] == 0x00)//蓝牙
								{
									music_para_set_event |= MUSIC_DEMO_OFF_EVENT;
								}						
								if(LIN_BUS_RXBuffer[11] == 0x01)//白噪音
								{
									if(LIN_BUS_RXBuffer[12] == 0x01)
									{
										MusicalOsc_Stu.DemoMode_Source = MUSIC_DEMO_SOURCE_WHITE_NOISE;
										MusicalOsc_Stu.DemoMode_TrackState[0] = MUSIC_TRACK_1;
										music_para_set_event |= MUSIC_DEMO_SET_TRACK_EVENT;
									}
									if(LIN_BUS_RXBuffer[12] == 0x02)
									{
										MusicalOsc_Stu.DemoMode_Source = MUSIC_DEMO_SOURCE_WHITE_NOISE;
										MusicalOsc_Stu.DemoMode_TrackState[0] = MUSIC_TRACK_2;										
										music_para_set_event |= MUSIC_DEMO_SET_TRACK_EVENT;
									}
									if(LIN_BUS_RXBuffer[12] == 0x03)
									{
										MusicalOsc_Stu.DemoMode_Source = MUSIC_DEMO_SOURCE_WHITE_NOISE;
										MusicalOsc_Stu.DemoMode_TrackState[0] = MUSIC_TRACK_3;											
										music_para_set_event |= MUSIC_DEMO_SET_TRACK_EVENT;
									}									
								}
								if(LIN_BUS_RXBuffer[11] == 0x02)
								{
									if(LIN_BUS_RXBuffer[12] == 0x01)
									{
										MusicalOsc_Stu.DemoMode_Source = MUSIC_DEMO_SOURCE_MUSIC;
										MusicalOsc_Stu.DemoMode_TrackState[1] = MUSIC_TRACK_1;										
										music_para_set_event |= MUSIC_DEMO_SET_TRACK_EVENT;
									}
									if(LIN_BUS_RXBuffer[12] == 0x02)
									{
										MusicalOsc_Stu.DemoMode_Source = MUSIC_DEMO_SOURCE_MUSIC;
										MusicalOsc_Stu.DemoMode_TrackState[1] = MUSIC_TRACK_1;											
										music_para_set_event |= MUSIC_DEMO_SET_TRACK_EVENT;
									}
									if(LIN_BUS_RXBuffer[12] == 0x03)
									{
										MusicalOsc_Stu.DemoMode_Source = MUSIC_DEMO_SOURCE_MUSIC;
										MusicalOsc_Stu.DemoMode_TrackState[1] = MUSIC_TRACK_1;											
										music_para_set_event |= MUSIC_DEMO_SET_TRACK_EVENT;
									}						
								}							
							}							
						}break;
						default:break;
					}
				}			
			}break;
			//风扇同步
			case 0x31:
			{
				if(LIN_BUS_RXBuffer[LIN_CMD_BIT] == 0x00) //问询
				{
					
				}
				if(LIN_BUS_RXBuffer[LIN_CMD_BIT] == 0x01) //控制
				{
					Fan_Clear_TimeCount();
					switch(LIN_BUS_RXBuffer[9])
					{
						case 0x21: //模式+时间
						{
							Fan_Stu.fan_time_min_set = ((unsigned short)LIN_BUS_RXBuffer[11] << 8) | LIN_BUS_RXBuffer[12];
							Fan_Stu.fan_mode_set = LIN_BUS_RXBuffer[13];
							fan_para_set_event |= FAN_MODE_TIME_EVENT;
						}break;
						case 0x22: //典型强度
						{
							if(((LIN_BUS_RXBuffer[2] - 11) % 2) == 0) //格式接收正确
							{
								temp_para = (LIN_BUS_RXBuffer[2] - 11) / 2;
								
								Fan_Stu.Fan_Dir_FlagArr[1] = LIN_BUS_RXBuffer[11];
								Fan_Stu.Fan_Ints_FlagArr[1] = LIN_BUS_RXBuffer[12];
								Fan_Stu.Fan_Dir_FlagArr[2] = LIN_BUS_RXBuffer[14];
								Fan_Stu.Fan_Ints_FlagArr[2] = LIN_BUS_RXBuffer[15];
								Fan_Stu.Fan_Dir_FlagArr[3] = LIN_BUS_RXBuffer[17];
								Fan_Stu.Fan_Ints_FlagArr[3] = LIN_BUS_RXBuffer[18];
								Fan_Stu.Fan_Dir_FlagArr[4] = LIN_BUS_RXBuffer[20];
								Fan_Stu.Fan_Ints_FlagArr[4] = LIN_BUS_RXBuffer[21];
							}
							fan_para_set_event |= FAN_ALL_INTS_EVENT;							
						}break;						
						case 0x30://fan
						{
							Heat_Stu.heat_state = LIN_BUS_RXBuffer[11];
							if(Heat_Stu.heat_state != 0)
							{
								heat_para_set_event |= HEAT_SWITCH_ON_EVENT;
							}
							else
							{
								heat_para_set_event |= HEAT_SWITCH_OFF_EVENT;
							}
						}break;
						default:break;
					}
				}
			}break;			
			default:break;
		}		
	}
}

unsigned char Lin_Get_KeyState(void)
{
	if(1 == lin_key_state)
	{
		lin_key_state = 0;
		return 1;
	}	
	
	if(1 == LIN_Check_Busy_Free())
	{
		lin_key_value = 0;
		return 2;
	}
	
	return 0;
}

/*-------------------------------------------------------------------------------------------------------------------------------------*/
//lin组包发送函数
unsigned char LIN_Write_Cmd(unsigned char device_funccode_temp,unsigned char extend_data_length)   
{
	unsigned char temp_i = 0;
	unsigned short sum_check = 0;
	unsigned char bus_send_length = 10 + extend_data_length;
	
	
	LIN_BUS_TXBuffer[0] = LIN_HEADER_ONE;LIN_BUS_TXBuffer[1] = LIN_HEADER_TWO;
	LIN_BUS_TXBuffer[2] = bus_send_length; LIN_BUS_TXBuffer[3] = 0X00;
	
	LIN_BUS_TXBuffer[4] = 0X01; //设备型号
	LIN_BUS_TXBuffer[5] = 0XFF; //
	
	LIN_BUS_TXBuffer[6] = 0x00;  //密钥
	LIN_BUS_TXBuffer[7] = device_funccode_temp; //功能码
	LIN_BUS_TXBuffer[8] = 0x01; //控制命令字
	
	
	for(temp_i = 2;temp_i < bus_send_length - 1;temp_i++)
	{
		sum_check += LIN_BUS_TXBuffer[temp_i];
	}

	LIN_BUS_TXBuffer[bus_send_length - 1] = sum_check%256;	
	
	return bus_send_length;
}
void LIN_Master_Send_lockstate_Cmd(unsigned short key_value_temp) //同步锁
{
	LIN_BUS_TXBuffer[LIN_PARA_START] = 0x88;
	
	LIN_BUS_TXBuffer[LIN_PARA_START + 1] = key_value_temp / 256;
	LIN_BUS_TXBuffer[LIN_PARA_START + 2] = key_value_temp % 256;

	LinBus_SendString(LIN_BUS_TXBuffer,LIN_Write_Cmd(0x26, 3));	
}
/*-------------------------------------------------指令相关------------------------------------------------------------*/
//发送指令
void LIN_Master_Send_WriteKeyValue_Cmd(unsigned short key_value_temp) 
{
	LIN_BUS_TXBuffer[LIN_PARA_START] = 0x88;
	
	LIN_BUS_TXBuffer[LIN_PARA_START + 1] = key_value_temp / 256;
	LIN_BUS_TXBuffer[LIN_PARA_START + 2] = key_value_temp % 256;

	LinBus_SendString(LIN_BUS_TXBuffer,LIN_Write_Cmd(0x21, 3));	
}
/*-------------------------------------------------推杆相关------------------------------------------------------------*/
//发送霍尔数
void LIN_Master_Send_WriteMotorHall_Cmd(unsigned char motor_num,unsigned char* motor_hall)
{
	unsigned char temp_i = 0;
	
	LIN_BUS_TXBuffer[LIN_PARA_START] = 0x88;
	
	for(temp_i = 0;temp_i < 3*motor_num;temp_i ++)
	{
		LIN_BUS_TXBuffer[LIN_PARA_START + 1 + temp_i] = motor_hall[temp_i];
	}
	
	LinBus_SendString(LIN_BUS_TXBuffer,LIN_Write_Cmd(0x22, (3*motor_num)+ 1));	
}
/*-------------------------------------------------按摩相关------------------------------------------------------------*/
/*
//按摩器模式设置
//序号现在默认写0X00
//模式:随动  持续35HZ  脉冲35HZ  波浪35HZ
*/
void LIN_Master_Send_WriteMsgrModeTimer_Cmd(unsigned char msgr_order,unsigned char msgr_mode_temp,unsigned char msgr_time_temp)   
{
	LIN_BUS_TXBuffer[LIN_PARA_START] = 0x21;  //模式+时间  设置
	LIN_BUS_TXBuffer[LIN_PARA_START + 1] = msgr_order;  //序号
	LIN_BUS_TXBuffer[LIN_PARA_START + 2] = msgr_mode_temp;  //按摩器模式
	LIN_BUS_TXBuffer[LIN_PARA_START + 3] = msgr_time_temp;  //按摩器定时时间
	
	LinBus_SendString(LIN_BUS_TXBuffer,LIN_Write_Cmd(0x23, 4));
}
/*
//随振模式强度设置
//序号现在默认写0XFF
//强度
*/
void LIN_Master_Send_WriteMsgrFollowInts_Cmd(unsigned char msgr_order,unsigned char msgr_ints_temp)   
{
	LIN_BUS_TXBuffer[LIN_PARA_START] = 0X22;  //典型按摩器强度
	LIN_BUS_TXBuffer[LIN_PARA_START + 1] = msgr_order;  //按摩器序号
	LIN_BUS_TXBuffer[LIN_PARA_START + 2] = msgr_ints_temp;  //按摩器强度
	
	LinBus_SendString(LIN_BUS_TXBuffer,LIN_Write_Cmd(0x23, 3));
}
/*
//典型模式强度
//序号 1头部 2脚部   00全部
//强度 0 1 2 3
*/
void LIN_Master_Send_WriteMsgrTypicalInts_Cmd(unsigned char msgr1_ints,unsigned char msgr2_ints,unsigned char msgr3_ints)   
{
	LIN_BUS_TXBuffer[LIN_PARA_START] = 0X23;  //典型按摩器强度
	LIN_BUS_TXBuffer[LIN_PARA_START + 1] = 1;  //按摩器序号
	LIN_BUS_TXBuffer[LIN_PARA_START + 2] = msgr1_ints;  //按摩器强度
	LIN_BUS_TXBuffer[LIN_PARA_START + 3] = 2;  //按摩器序号
	LIN_BUS_TXBuffer[LIN_PARA_START + 4] = msgr2_ints;  //按摩器强度	
	LIN_BUS_TXBuffer[LIN_PARA_START + 5] = 3;  //按摩器序号
	LIN_BUS_TXBuffer[LIN_PARA_START + 6] = msgr3_ints;  //按摩器强度	
	
	LinBus_SendString(LIN_BUS_TXBuffer,LIN_Write_Cmd(0x23, 7));
}
/*-------------------------------------------------灯控相关------------------------------------------------------------*/
void LIN_Master_Send_WriteColour_Cmd(unsigned char device_code,unsigned char led_order,unsigned char* rgb_colour)   
{
	LIN_BUS_TXBuffer[LIN_PARA_START + 1] = led_order;  //
	if(device_code == LIGHT_BOARD_DEVICE_TYPE)
	{
		LIN_BUS_TXBuffer[LIN_PARA_START] = 0x01;  //灯牌状态
		if(*rgb_colour != 0)
		{
			LIN_BUS_TXBuffer[LIN_PARA_START + 2] = 255;
			LIN_BUS_TXBuffer[LIN_PARA_START + 3] = 255;
			LIN_BUS_TXBuffer[LIN_PARA_START + 4] = 255;					
		}
		else
		{
			LIN_BUS_TXBuffer[LIN_PARA_START + 2] = 0;
			LIN_BUS_TXBuffer[LIN_PARA_START + 3] = 0;
			LIN_BUS_TXBuffer[LIN_PARA_START + 4] = 0;			
		}
	}
	//单色灯
	if(device_code == LIGHT_ONE_DEVICE_TYPE)
	{
		LIN_BUS_TXBuffer[LIN_PARA_START] = 0x02;  //单色床底灯状态
		if(*rgb_colour != 0)
		{
			LIN_BUS_TXBuffer[LIN_PARA_START + 2] = 255;
			LIN_BUS_TXBuffer[LIN_PARA_START + 3] = 255;
			LIN_BUS_TXBuffer[LIN_PARA_START + 4] = 255;					
		}
		else
		{
			LIN_BUS_TXBuffer[LIN_PARA_START + 2] = 0;
			LIN_BUS_TXBuffer[LIN_PARA_START + 3] = 0;
			LIN_BUS_TXBuffer[LIN_PARA_START + 4] = 0;			
		}
	}
	//RGB灯
	if(device_code == LIGHT_RGB_DEVICE_TYPE)
	{
		LIN_BUS_TXBuffer[LIN_PARA_START] = 0x23;  //RGB床底灯状态
		LIN_BUS_TXBuffer[LIN_PARA_START + 2] = *rgb_colour;
		LIN_BUS_TXBuffer[LIN_PARA_START + 3] = *(rgb_colour+1);
		LIN_BUS_TXBuffer[LIN_PARA_START + 4] = *(rgb_colour+2);
	}
	//
	LinBus_SendString(LIN_BUS_TXBuffer,LIN_Write_Cmd(0x24, 5));
}
//发送外设-灯-时间
void LIN_Master_Send_WriteUBLTime_Cmd(unsigned char led_order,unsigned short time_sec_temp) 
{
	LIN_BUS_TXBuffer[LIN_PARA_START] = 0x22;  //时间
	LIN_BUS_TXBuffer[LIN_PARA_START + 1] = led_order;  //
	LIN_BUS_TXBuffer[LIN_PARA_START + 2] = time_sec_temp/256;  //
	LIN_BUS_TXBuffer[LIN_PARA_START + 3] = time_sec_temp%256;  //
	
	LinBus_SendString(LIN_BUS_TXBuffer,LIN_Write_Cmd(0x24, 4));
}
/*-------------------------------------------------哄睡控制相关------------------------------------------------------------*/
//哄睡定时时间
void LIN_Master_Send_DemoRunTime_Cmd(unsigned short time_min_temp)
{
	LIN_BUS_TXBuffer[LIN_PARA_START] = 0x02;  //时间
	LIN_BUS_TXBuffer[LIN_PARA_START + 1] = 0;  //
	LIN_BUS_TXBuffer[LIN_PARA_START + 2] = time_min_temp/256;  
	LIN_BUS_TXBuffer[LIN_PARA_START + 3] = time_min_temp%256;  

	LinBus_SendString(LIN_BUS_TXBuffer,LIN_Write_Cmd(0x27, 4));	
}
//哄睡步骤
void LIN_Master_Send_DemoStep_Cmd(unsigned char demo_num, char step_temp)
{
	LIN_BUS_TXBuffer[LIN_PARA_START] = 0x03;  //步骤
	LIN_BUS_TXBuffer[LIN_PARA_START + 1] = demo_num;  //编号
	LIN_BUS_TXBuffer[LIN_PARA_START + 2] = step_temp;  //步骤值

	LinBus_SendString(LIN_BUS_TXBuffer,LIN_Write_Cmd(0x27, 3));	
}
//闹钟特殊模式
void LIN_Master_Send_AlarmRun_Cmd(unsigned char motor_run_temp,unsigned char msgr_run_temp, unsigned char ubl_run_temp, unsigned char music_temp)
{
	LIN_BUS_TXBuffer[LIN_PARA_START] = 0x03;  //时间
	LIN_BUS_TXBuffer[LIN_PARA_START + 1] = motor_run_temp;  
	LIN_BUS_TXBuffer[LIN_PARA_START + 2] = msgr_run_temp;  
	LIN_BUS_TXBuffer[LIN_PARA_START + 3] = ubl_run_temp;  
	LIN_BUS_TXBuffer[LIN_PARA_START + 4] = music_temp;  

	LinBus_SendString(LIN_BUS_TXBuffer,LIN_Write_Cmd(0x25, 5));	
}
/*-------------------------------------------------音乐控制相关------------------------------------------------------------*/
//音量
void LIN_Master_Send_WriteMusicVolume_Cmd(MUSIC_VOLUME_CMD volume_cmd_temp,unsigned char volume_dat_temp)   
{
	LIN_BUS_TXBuffer[LIN_PARA_START] = 0x25; //音量
	
	LIN_BUS_TXBuffer[LIN_PARA_START + 1] = volume_cmd_temp;  //设置命令
	LIN_BUS_TXBuffer[LIN_PARA_START + 2] = volume_dat_temp;  //音量大小  只有设置指令是0时起作用
	
	
	LinBus_SendString(LIN_BUS_TXBuffer,LIN_Write_Cmd(0x30, 3));
}
//曲目
void LIN_Master_Send_WriteMusicTrack_Cmd(MUSIC_TRACK_CMD track_cmd_temp)   
{
	LIN_BUS_TXBuffer[LIN_PARA_START] = 0x26; //曲目
	
	LIN_BUS_TXBuffer[LIN_PARA_START + 1] = track_cmd_temp;  //设置命令
	
	LinBus_SendString(LIN_BUS_TXBuffer,LIN_Write_Cmd(0x30, 2));
}
//播放
void LIN_Master_Send_WriteMusicPlay_Cmd(SWITCH_CMD play_cmd_temp)   
{
	LIN_BUS_TXBuffer[LIN_PARA_START] = 0x27;  //播放
	
	LIN_BUS_TXBuffer[LIN_PARA_START + 1] = play_cmd_temp;  //设置命令
	
	LinBus_SendString(LIN_BUS_TXBuffer,LIN_Write_Cmd(0x30, 2));
}
//蓝牙
void LIN_Master_Send_WriteBuleTooth_Cmd(MUSIC_BLUE_CMD ble_func_temp,SWITCH_CMD ble_dat_temp)
{
	LIN_BUS_TXBuffer[LIN_PARA_START] = 0x29; //蓝牙控制
	
	LIN_BUS_TXBuffer[LIN_PARA_START + 1] = ble_func_temp;  //
	LIN_BUS_TXBuffer[LIN_PARA_START + 2] = ble_dat_temp;  //
	
	LinBus_SendString(LIN_BUS_TXBuffer,LIN_Write_Cmd(0x30, 3));
}
//工作模式
void LIN_Master_Send_WriteMusicSysMode_Cmd(unsigned char sys_mode_temp)   
{
	LIN_BUS_TXBuffer[LIN_PARA_START] = 0x2B; //蓝牙控制
	
	LIN_BUS_TXBuffer[LIN_PARA_START + 1] = sys_mode_temp;  //
	
	LinBus_SendString(LIN_BUS_TXBuffer,LIN_Write_Cmd(0x30, 2));
}
//demo播放源
void LIN_Master_Send_WriteMusicDemo_Source(unsigned char source,unsigned char track)   
{
	LIN_BUS_TXBuffer[LIN_PARA_START] = 0x2B; //蓝牙控制
	
	LIN_BUS_TXBuffer[LIN_PARA_START + 1] = 0x04;  //
	LIN_BUS_TXBuffer[LIN_PARA_START + 2] = source;  //
	LIN_BUS_TXBuffer[LIN_PARA_START + 3] = track;  //
	
	LinBus_SendString(LIN_BUS_TXBuffer,LIN_Write_Cmd(0x30, 4));
}
void LIN_Master_Send_WriteHeatSwitch(unsigned char hear_switch)
{
	LIN_BUS_TXBuffer[LIN_PARA_START] = 0X30;  //加热垫开关控制
	LIN_BUS_TXBuffer[LIN_PARA_START + 1] = 0x00;
	LIN_BUS_TXBuffer[LIN_PARA_START + 2] = hear_switch;
	
	LinBus_SendString(LIN_BUS_TXBuffer,LIN_Write_Cmd(0x31, 3));	
}
/*-------------------------------------风扇相关-----------------------------------------*/

void LIN_Master_Send_WriteFanInts_Cmd(unsigned char *fan_ints,unsigned char *fan_dir)   
{
	LIN_BUS_TXBuffer[LIN_PARA_START] = 0X22;  //风扇强度方向控制
	LIN_BUS_TXBuffer[LIN_PARA_START + 1] = 1;  //风扇序号
	LIN_BUS_TXBuffer[LIN_PARA_START + 2] = fan_dir[1];  //风扇方向
	LIN_BUS_TXBuffer[LIN_PARA_START + 3] = fan_ints[1];  //风扇强度
	LIN_BUS_TXBuffer[LIN_PARA_START + 4] = 2;  
	LIN_BUS_TXBuffer[LIN_PARA_START + 5] = fan_dir[2];  
	LIN_BUS_TXBuffer[LIN_PARA_START + 6] = fan_ints[2];  
	LIN_BUS_TXBuffer[LIN_PARA_START + 7] = 3;  
	LIN_BUS_TXBuffer[LIN_PARA_START + 8] = fan_dir[3];  
	LIN_BUS_TXBuffer[LIN_PARA_START + 9] = fan_ints[3];  
	LIN_BUS_TXBuffer[LIN_PARA_START + 10] = 4;
	LIN_BUS_TXBuffer[LIN_PARA_START + 11] = fan_dir[4];
	LIN_BUS_TXBuffer[LIN_PARA_START + 12] = fan_ints[4];
	
	LinBus_SendString(LIN_BUS_TXBuffer,LIN_Write_Cmd(0x31, 13));
}

void LIN_Master_Send_WriteFanModeTime_Cmd(unsigned char fan_order,unsigned char fan_mode_temp,unsigned short fan_time_temp)   
{
	fan_order = 0;//暂时没用，做预留
	LIN_BUS_TXBuffer[LIN_PARA_START] = 0X21;  //风扇模式时间控制
	LIN_BUS_TXBuffer[LIN_PARA_START + 1] = 0x00;  //全部风扇
	LIN_BUS_TXBuffer[LIN_PARA_START + 2] = fan_time_temp/256;
	LIN_BUS_TXBuffer[LIN_PARA_START + 3] = fan_time_temp%256; 
	LIN_BUS_TXBuffer[LIN_PARA_START + 4] = fan_mode_temp;
	
	LinBus_SendString(LIN_BUS_TXBuffer,LIN_Write_Cmd(0x31, 5));
}
//app_linbus.c
void LIN_Master_Send_WriteRgbMode_Cmd(unsigned char led_order,unsigned char mode_type,unsigned char mode_order) 
{
	LIN_BUS_TXBuffer[LIN_PARA_START] = 0x24;  //模式
	LIN_BUS_TXBuffer[LIN_PARA_START + 1] = led_order;  //
	LIN_BUS_TXBuffer[LIN_PARA_START + 2] = mode_type;  //
	LIN_BUS_TXBuffer[LIN_PARA_START + 3] = mode_order;  //
	
	LinBus_SendString(LIN_BUS_TXBuffer,LIN_Write_Cmd(0x24, 4));
}
void LIN_Master_Send_WriteRgbBreathMode_Cmd(unsigned char led_order, unsigned char *breath_freq, unsigned char r_color,unsigned char g_color,unsigned char b_color)  
{
	LIN_BUS_TXBuffer[LIN_PARA_START] = 0x25;  //模式
	LIN_BUS_TXBuffer[LIN_PARA_START + 1] = led_order;  //
	LIN_BUS_TXBuffer[LIN_PARA_START + 2] = breath_freq[0];  //
	LIN_BUS_TXBuffer[LIN_PARA_START + 3] = breath_freq[1];  //
	LIN_BUS_TXBuffer[LIN_PARA_START + 4] = r_color;  //
	LIN_BUS_TXBuffer[LIN_PARA_START + 5] = g_color;  //
	LIN_BUS_TXBuffer[LIN_PARA_START + 6] = b_color;  //	
	LinBus_SendString(LIN_BUS_TXBuffer,LIN_Write_Cmd(0x24, 7));
}

//void LIN_Master_Send_WriteSleepMode_Cmd(unsigned char led_order, unsigned char *sleep_time, unsigned char r_color,unsigned char g_color,unsigned char b_color)   
//{
//	LIN_BUS_TXBuffer[LIN_PARA_START] = 0x26;  //模式
//	LIN_BUS_TXBuffer[LIN_PARA_START + 1] = led_order;  //
//	LIN_BUS_TXBuffer[LIN_PARA_START + 2] = sleep_time[0];  //
//	LIN_BUS_TXBuffer[LIN_PARA_START + 3] = sleep_time[1];  //
//	LIN_BUS_TXBuffer[LIN_PARA_START + 4] = r_color;  //
//	LIN_BUS_TXBuffer[LIN_PARA_START + 5] = g_color;  //
//	LIN_BUS_TXBuffer[LIN_PARA_START + 6] = b_color;  //	
//	LinBus_SendString(LIN_BUS_TXBuffer,LIN_Write_Cmd(0x26, 7));
//}
void LIN_Master_Send_WriteRgbBrightness_Cmd(unsigned char led_order, unsigned char brightness)  
{
	LIN_BUS_TXBuffer[LIN_PARA_START] = 0x27;  //模式
	LIN_BUS_TXBuffer[LIN_PARA_START + 1] = led_order;  //
	LIN_BUS_TXBuffer[LIN_PARA_START + 2] = brightness;  //	
	LinBus_SendString(LIN_BUS_TXBuffer,LIN_Write_Cmd(0x24, 3));
}

