#include "app_ttlbus.h"
#include "delay.h"
#include "app_backhual.h"
#include "driver_beep.h"
#include "driver_rtc.h"
#include "app_ble.h"
#include "modul_ttlbus.h"
#include "driver_uart.h"

#include "app_comm.h"
#include "app_motor.h"
#include "app_msgr.h"
#include "app_music.h"
#include "app_light.h"
#include "app_vita.h"
#include "app_rtc.h"
#include "app_heat.h"
#include "app_config.h"
#include "app_backhual.h"

static unsigned char TTL_RADIO_TXBuffer[40] = {0};
static unsigned char ttl_radio_time = 0;  //广播时间间隔
//TTL控制键值接收
unsigned short ttl_key_value = 0;  //key值

unsigned char back_cmd = 0;
unsigned char leg_cmd = 0;
//电机运行状态
unsigned char ttl_motor_runmode_radioenable = 0; 
//加热垫开关广播使能标志位
unsigned char tll_heatsw_radioenable = 0;
//TTL总线发送驱动函数
void TTL_Bus_SendData(unsigned char data_temp)
{
	 Uart0_SendData_IT(data_temp);
}
//TTL总线接收协议处理
void TTL_Bus_UartDataAnaly(unsigned char device_type)
{
	uint8_t temp_i ;
	switch(device_type) 
	{
		case GESTURE_DEVICE_TYPE: //从控制盒
		{
				if(TTL_BUS_RXBuffer[TTL_CODE_BIT] == 0x21 && TTL_BUS_RXBuffer[8] != 0x00) //手势信号 //没有断联
				{
					key_para_set_event |= KEY_TTL_CMD_EVENT;
					
					switch(TTL_BUS_RXBuffer[10])
					{
						case 1:
						{
							ttl_key_value = KEY_BACK_UP;
						}break;
						case 3:
						{
							ttl_key_value = KEY_LEG_UP;
						}break;
						case 5:
						{
							ttl_key_value = KEY_BACKLEG_UP;
						}break;
						case 7://  如果直接跑 需要加逻辑
						{	
							if(Get_Motor_AllReset()==1)
							{
								ttl_key_value = KEY_GO_ZEROG;
							}
							else
							{
								ttl_key_value = KEY_FLAT;
							}
						}break;
						default:
						{
								ttl_key_value = KEY_NO;
						}break;	
					}						
				}
		}break;
			
		case 0x02:  //无显示屏手控器
		{
			switch(TTL_BUS_RXBuffer[TTL_CODE_BIT])
			{
				case 0x21:  //指令
				{
					if((TTL_BUS_RXBuffer[9] & 0x80) == 0x00)
					{
						if(TTL_BUS_RXBuffer[9] == 0x01)
						{
//							if(Motor_Run_Cmd() != 0)  //加上此处  固定位置之间会直接跑  如果不加就是打断  一般连接米家等云平台需要加
//							{
//								GetSet_Motor_Ctr_Cmd(0);	
//							}
							key_para_set_event |= KEY_TTL_CMD_EVENT;
							ttl_key_value = ((unsigned short)TTL_BUS_RXBuffer[10] << 8) | TTL_BUS_RXBuffer[11];						
						}						
					}
					else
					{

					}
				}break;
				case 0x22: //推杆信息
				{
					if((TTL_BUS_RXBuffer[9] & 0x80) == 0x00)
					{
					
					}
					else //上报
					{
						if((TTL_BUS_RXBuffer[9] & 0x7F) == 0x03)
						{
							ttl_motor_runmode_radioenable = 1;
						}						
					}
				}break;			
				case 0x27: //加热垫
				{
					if((TTL_BUS_RXBuffer[9] & 0x80) == 0x00)
					{
					
					}
					else //上报
					{
						if((TTL_BUS_RXBuffer[9] & 0x7F) == 0x03)
						{
							tll_heatsw_radioenable = 1;
						}						
					}
				}break;						
				default:break;				
			}
		}break;
		case 0x03: //带显示屏遥控器
		{
		
		}break;
		case LIGHT_RGB_DEVICE_TYPE: //RGB感应灯设备
		{
			if(TTL_BUS_RXBuffer[TTL_CODE_BIT] == 0x21 && TTL_BUS_RXBuffer[8] != 0x00) //感应状态功能码 并且从机是连接状态
			{
				if(TTL_BUS_RXBuffer[10] != 0) //有感应信号
				{
					Light_RgbColour_Stu.light_sensor_signal = TTL_BUS_RXBuffer[10];
				}
			}						
		}break;
		case LIGHT_ONE_DEVICE_TYPE: //单色感应灯设备
		{
			if(TTL_BUS_RXBuffer[TTL_CODE_BIT] == 0x21 && TTL_BUS_RXBuffer[8] != 0x00) //感应状态功能码 并且从机是连接状态
			{
				if(TTL_BUS_RXBuffer[10] != 0) //有感应信号
				{
					Light_OneColour_Stu.light_sensor_signal = TTL_BUS_RXBuffer[10];
				}
			}						
		}break;		
		case MUSIC_DEVICE_TYPE: //音乐阵子
		{
			if(TTL_BUS_RXBuffer[TTL_CODE_BIT] == 0x21) //按摩器模式
			{
				msgr_mode_set = TTL_BUS_RXBuffer[10];
				
				msgr_state_updata_event |= MSGR_MODE_TIME_EVENT;
			}
			if(TTL_BUS_RXBuffer[TTL_CODE_BIT] == 0x22) //随振强度
			{
				MusicalOsc_Stu.MsgrFollowMode_IntState = TTL_BUS_RXBuffer[10];
				
				Msgr_Ints_FlagArr[0] = MusicalOsc_Stu.MsgrFollowMode_IntState;
				
				msgr_state_updata_event |= MSGR_FOLLOW_INTS_EVENT;
			}
			
			if(TTL_BUS_RXBuffer[TTL_CODE_BIT] == 0x25) //音乐音量
			{
				MusicalOsc_Stu.MusicVolume_Level = TTL_BUS_RXBuffer[9];
				
				//上报
				music_state_updata_event |= MUSIC_VOL_SET_EVENT;		
			}			
			if(TTL_BUS_RXBuffer[TTL_CODE_BIT] == 0x27) //播放状态功能码
			{
				MusicalOsc_Stu.Music_PlayState = TTL_BUS_RXBuffer[9];
				
				if(TTL_BUS_RXBuffer[8] != 0x00)  //主动上报的需要同步事件  控制问询的不需要触发同步事件要不然就死循环了
				{
//					if(MusicalOsc_Stu.Music_PlayState == SWITCH_STATE_OFF)  //手机音乐软件暂停需要上报同步状态
//					{
//						music_para_set_event |= MUSIC_PAUSE_EVENT;							
//					}
//					else
//					{
//						music_para_set_event |= MUSIC_PLAY_EVENT;								
//					}
				}
				//上报
				if(MusicalOsc_Stu.Music_PlayState == SWITCH_STATE_OFF)  //手机音乐软件暂停需要上报同步状态
				{
					music_state_updata_event |= MUSIC_PAUSE_EVENT;				
				}
				else
				{
					music_state_updata_event |= MUSIC_PLAY_EVENT;	
				}
			}
			if(TTL_BUS_RXBuffer[TTL_CODE_BIT] == 0x28) //音源通道
			{
				MusicalOsc_Stu.Music_SrcChannelSelet = TTL_BUS_RXBuffer[9];	
				
//				if(TTL_BUS_RXBuffer[8] != 0x00)
//				{
//					if(MusicalOsc_Stu.Music_SrcChannelSelet == 0)
//					{
//						music_state_updata_event |= MUSIC_DEMO_OFF_EVENT;		
//						
//						music_para_set_event |= MUSIC_DEMO_OFF_EVENT;
//					}
//					else if(MusicalOsc_Stu.Music_SrcChannelSelet == 2)
//					{
////						music_state_updata_event |= MUSIC_DEMO_ON_EVENT;	
//					}
//				}
				
			}
			
			if(TTL_BUS_RXBuffer[TTL_CODE_BIT] == 0x29) //蓝牙状态功能码
			{
				if(TTL_BUS_RXBuffer[9] == BLUETOOTH_SW_STATE) //蓝牙开关状态
				{
					MusicalOsc_Stu.BlueTooth_State[BLUETOOTH_SW_STATE] = TTL_BUS_RXBuffer[10];
					//上报
					if(MusicalOsc_Stu.BlueTooth_State[BLUETOOTH_SW_STATE] == SWITCH_STATE_OFF)
					{
						music_state_updata_event |= MUSIC_BLE_OFF_EVENT;
					}
					else
					{
						music_state_updata_event |= MUSIC_BLE_ON_EVENT;
					}
				}
				if(TTL_BUS_RXBuffer[9] == BLUETOOTH_CONNECT_STATE) //蓝牙连接状态
				{
					MusicalOsc_Stu.BlueTooth_State[BLUETOOTH_CONNECT_STATE] = TTL_BUS_RXBuffer[10];
					
					if(TTL_BUS_RXBuffer[8] != 0x00)
					{
//					if(MusicalOsc_Stu.BlueTooth_State[BLUETOOTH_CONNECT_STATE] == SWITCH_STATE_OFF)
//					{
//						music_state_updata_event |= MUSIC_BLE_CONNECT_EVENT;
//					}
//					else
//					{
//						music_state_updata_event |= MUSIC_BLE_DISCONNECT_EVENT;
//					}						
					}
				
				}	
				if(TTL_BUS_RXBuffer[9] == BLUETOOTH_TWS_STATE) //蓝牙TWS状态
				{
					if(TTL_BUS_RXBuffer[8] != 0x00)
					{
						
					}
					MusicalOsc_Stu.BlueTooth_State[BLUETOOTH_TWS_STATE] = TTL_BUS_RXBuffer[10];
				}							
			}				
			if(TTL_BUS_RXBuffer[TTL_CODE_BIT] == 0x2B) //演示模式
			{
				if(TTL_BUS_RXBuffer[9] <= 2)
				{
					MusicalOsc_Stu.SysMode_State = TTL_BUS_RXBuffer[9];
					
					if(TTL_BUS_RXBuffer[8] != 0x00)  //主动上报的需要同步事件  控制问询的不需要触发同步事件要不然就死循环了
					{
						if(MusicalOsc_Stu.SysMode_State == MUSIC_SOURCE_BLUE)  //
						{
							MusicalOsc_Stu.SysMode_State = MUSIC_SOURCE_BLUE;

							//上报
							music_state_updata_event |= MUSIC_DEMO_OFF_EVENT;					
						}
						else
						{
							MusicalOsc_Stu.SysMode_State = MUSIC_SOURCE_U;

							//上报
							music_state_updata_event |= MUSIC_DEMO_ON_EVENT;								
						}
					}
					//上报
					if(MusicalOsc_Stu.SysMode_State == MUSIC_SOURCE_BLUE)  //
					{
						music_state_updata_event |= MUSIC_DEMO_OFF_EVENT;				
					}
					else
					{
						music_state_updata_event |= MUSIC_DEMO_ON_EVENT;	
					}		
				}	
				else if(TTL_BUS_RXBuffer[9] == 4)
				{
					if(TTL_BUS_RXBuffer[10] == 1)
					{
						MusicalOsc_Stu.DemoMode_Source = MUSIC_DEMO_SOURCE_WHITE_NOISE;
						MusicalOsc_Stu.DemoMode_TrackState[0] = TTL_BUS_RXBuffer[11];
						music_state_updata_event |= MUSIC_DEMO_SET_TRACK_EVENT;				
					}
					else if(TTL_BUS_RXBuffer[10] == 2)
					{
						MusicalOsc_Stu.DemoMode_Source = MUSIC_DEMO_SOURCE_MUSIC;
						MusicalOsc_Stu.DemoMode_TrackState[0] = TTL_BUS_RXBuffer[11];
						music_state_updata_event |= MUSIC_DEMO_SET_TRACK_EVENT;				
					}						
				}
			}
			if(TTL_BUS_RXBuffer[TTL_CODE_BIT] == 0x2D) //演示模式电机控制
			{
				if(TTL_BUS_RXBuffer[9] == 2)//电机随动模式
				{
					back_cmd = TTL_BUS_RXBuffer[10];
					leg_cmd = TTL_BUS_RXBuffer[12];
				}
			}			
		}break;
		case MUSIC_WHITE_NOISE_DEVICE_TYPE:
		{
			if(TTL_BUS_RXBuffer[TTL_CODE_BIT] == 0x25)
			{
				MusicalOsc_Stu.MusicVolume_Level = TTL_BUS_RXBuffer[9];
				
				//上报
				music_state_updata_event |= MUSIC_VOL_SET_EVENT;					
			}
		}break;
		case VITA_DEVICE_TYPE: //睡眠检测
		{
			if(TTL_BUS_RXBuffer[TTL_CODE_BIT] == 0x21 && TTL_BUS_RXBuffer[8] != 0x00) //
			{
				if(TTL_BUS_RXBuffer[10] != 0x00 && ((sys_lock_state&SYS_SNORE_CHECK_STATE) == SYS_SNORE_CHECK_STATE)) // 有感应信号且打鼾干预开关开启状态
				{
					Vita_EMM_Stu.snore_value = 1;
				}
			}	
			if(TTL_BUS_RXBuffer[TTL_CODE_BIT] == 0x22) //体征信号
			{
				//
				if(((TTL_BUS_RXBuffer[2] - 10) % 6) == 0) //格式接收正确
				{
					unsigned char temp_para = 0;
					temp_para = (TTL_BUS_RXBuffer[2] - 10) / 6;
					
					//解析实时数据
					for (temp_i = 0;temp_i < temp_para;temp_i ++)
					{
						if(TTL_BUS_RXBuffer[9 + temp_i*6 + 0] == 0x01)
						{
						//		Vita_Left_Stu.onbed_flag = TTL_BUS_RXBuffer[9 + temp_i*6 + 1];
//							Vita_Left_Stu.sleep_flag = TTL_BUS_RXBuffer[9 + temp_i*6 + 2];
//							Vita_Left_Stu.heart_value = TTL_BUS_RXBuffer[9 + temp_i*6 + 3];
//							Vita_Left_Stu.breathe_value = TTL_BUS_RXBuffer[9 + temp_i*6 + 4];	
								Vita_Left_Stu.snore_value = TTL_BUS_RXBuffer[9 + temp_i*6 + 5];
						}
						else if(TTL_BUS_RXBuffer[9 + temp_i*6 + 0] == 0x02)
						{
//							Vita_Right_Stu.onbed_flag = TTL_BUS_RXBuffer[9 + temp_i*6 + 1];
//							Vita_Right_Stu.sleep_flag = TTL_BUS_RXBuffer[9 + temp_i*6 + 2];
//							Vita_Right_Stu.heart_value = TTL_BUS_RXBuffer[9 + temp_i*6 + 3];
//							Vita_Right_Stu.breathe_value = TTL_BUS_RXBuffer[9 + temp_i*6 + 4];	
								Vita_Right_Stu.snore_value = TTL_BUS_RXBuffer[9 + temp_i*6 + 5];
						}
						if(Vita_Left_Stu.snore_value  || Vita_Right_Stu.snore_value )//左右检测到睡眠信号 给snore赋值
						{
							Vita_EMM_Stu.snore_value = 1;
						}
					}
				}
			}
			if(TTL_BUS_RXBuffer[TTL_CODE_BIT] == 0x25) //蓝牙广播ID
			{		
					for( temp_i = 0; temp_i < 9;temp_i ++)
					{
						Vita_Ble_RadioName[temp_i] = TTL_BUS_RXBuffer[9 + temp_i];
					}
					//上报蓝牙
					ble_report_set_event(BLE_ACK_EVENT, ACK_REPORT_VITA_ADDR);
			}		
		}break;

		default:break;
	}		
}

//获取TTL总线键值
unsigned char TTL_GetClear_KeyValue(unsigned char set_reset)
{
	if(0 == set_reset)
	{
		ttl_key_value = 0;
	}
	
	return ttl_key_value;
}
//TTL主机广播写缓冲
unsigned char TTL_Radio_Write_Cmd(unsigned char device_funccode_temp , unsigned char extend_data_length)   
{
	unsigned char temp_i = 0;
	unsigned short sum_check = 0;
	unsigned char bus_send_length = 10 + extend_data_length;
	
	if(RingBuffer.ring_write_data_lock != 0) return 0;
	RingBuffer.ring_write_data_lock = 1;		

	if(TTL_RingBuffer_CheckEnough(bus_send_length) == 0)
	{
		RingBuffer.ring_write_data_lock = 0;
		return 0;
	}
	
	TTL_RADIO_TXBuffer[0] = TTL_HEADER_ONE;TTL_RADIO_TXBuffer[1] = TTL_HEADER_TWO;
	TTL_RADIO_TXBuffer[2] = bus_send_length; TTL_RADIO_TXBuffer[3] = 0X00;
	
	TTL_RADIO_TXBuffer[4] = 0XFF; //设备型号
	TTL_RADIO_TXBuffer[5] = 0XFF; 
	
	TTL_RADIO_TXBuffer[6] = 0x00;  //密钥
	TTL_RADIO_TXBuffer[7] = device_funccode_temp; //功能码
	TTL_RADIO_TXBuffer[8] = 0x03; //控制命令字
	
	
	for(temp_i = 2;temp_i < bus_send_length - 1;temp_i++)
	{
		sum_check += TTL_RADIO_TXBuffer[temp_i];
	}

	TTL_RADIO_TXBuffer[bus_send_length - 1] = sum_check%256;	
		

	for(temp_i = 0;temp_i < bus_send_length;temp_i ++)
	{
		if(Write_Ring_Data(TTL_RADIO_TXBuffer[temp_i]) == 1) //错误
		{
			break;
		}
	}	
		
	RingBuffer.ring_write_data_lock = 0;
	
	if(temp_i == bus_send_length)  return 1;

	return 0;
}
unsigned char TTL_Radio_Motor_RunMode(unsigned char motor_run_mode_temp)  //上报电机运行状态
{
	static unsigned char motor_run_mode = 0;
	
	if(motor_run_mode != motor_run_mode_temp)
	{
		motor_run_mode = motor_run_mode_temp;
		ttl_motor_runmode_radioenable = 1;
	}
	
	if(ttl_motor_runmode_radioenable != 0)
	{
		TTL_RADIO_TXBuffer[9] = 0x03;	 
		TTL_RADIO_TXBuffer[10] = motor_run_mode_temp;	 //电机运行状态

		if(1 == TTL_Radio_Write_Cmd(0X1D,2))
		{
			ttl_motor_runmode_radioenable = 0;
			return 2;
		}
		return 0;
	}
	
	return 1;		
}
//TTL主机广播控制盒状态
unsigned char TTL_Radio_Heat_SwState(unsigned char heat_sw_temp) //
{
	static unsigned char heat_sw_state = 0;
	
	if(heat_sw_state != heat_sw_temp)
	{
		heat_sw_state = heat_sw_temp;
		tll_heatsw_radioenable = 1;
	}	


	if(tll_heatsw_radioenable != 0)
	{
		TTL_RADIO_TXBuffer[9] = 0x03;
		TTL_RADIO_TXBuffer[10] = 0x00;
		TTL_RADIO_TXBuffer[11] = heat_sw_temp; //

		if(1 == TTL_Radio_Write_Cmd(0X18,3))
		{
			tll_heatsw_radioenable = 0;
			return 2;
		}
		return 0;
	}
	
	return 1;	
}

void TTL_Radio_VitaBleID(void)  //控制盒获取蓝牙广播ID
{
	if((ttl_vita_set_radioenable & TTL_VITA_BLE_ID_READ_EVENT) == TTL_VITA_BLE_ID_READ_EVENT)
	{
		ttl_vita_set_radioenable &= ~TTL_VITA_BLE_ID_READ_EVENT;
		TTL_User_AskDevice_Status(0X13,0X25,0X00,0X00);
	}
}
//TTL主机广播任务
void TTL_Master_RadioTask(void)
{
	//马达运行模式
	if(2 == TTL_Radio_Motor_RunMode(Get_Motor_Run_CmdState())) return; 
	//加热垫开关状态
	if(2 == TTL_Radio_Heat_SwState(Heat_Stu.heat_state)) return;	
	
	TTL_Radio_VitaBleID();
}
//TTL主机广播任务间隔
void TTL_MasterRadio_TimerManagerTask(void)
{
	ttl_radio_time ++;
	if(ttl_radio_time >= 200)
	{
		ttl_radio_time = 200;
	}
	
	TTL_Master_RadioTask();
}

