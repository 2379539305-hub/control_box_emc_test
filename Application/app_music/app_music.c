#include "app_music.h"
#include "delay.h"

#include "driver_beep.h"
#include "driver_ble.h"
#include "driver_periph.h"

#include "modul_ttlbus.h"
#include "modul_musicmsgr.h"


#include "app_comm.h"
#include "app_linbus.h"
#include "app_msgr.h"
#include "app_backhual.h"
#include "app_light.h"
#include "app_motor.h"
#include "app_rtc.h"

unsigned long music_para_set_event = 0;  //音乐同步状态事件

unsigned long music_state_updata_event = 0; //回传状态显示

unsigned char musicmsgr_device_online = 0;
unsigned short musicmsgr_status_check = 0; //状态回传事件
unsigned char music_device_ack_flag = 0;  //是否回传

MUSIC_OSC_STRUCT  MusicalOsc_Stu = {0}; 

unsigned char demo_music_save[2] = {MUSIC_DEMO_SOURCE_WHITE_NOISE, MUSIC_TRACK_1};
unsigned char demo_light_save[3] = {255,220,120};
unsigned char demo_volume_save = 10;
unsigned char demo_run_time = 30;
unsigned char demo_run_time_save = 30;
unsigned int music_time_ms_count = 0;
unsigned int music_time_sec_count = 0;
static unsigned char  music_ctr_cmd = 0 , old_music_ctr_cmd = 0;

unsigned char Music_AcceptCmd_KeyInfo(unsigned char key_temp)
{
	if((key_temp >= KEY_MUSIC_START && key_temp <= KEY_MUSIC_END)|| \
		(key_temp >= KEY_MUSIC_FOLLOW_INTS_ADD && key_temp <= KEY_MUSIC_FOLLOW_INTS_THREE))
	{
		return 1;
	}
	return 0;
}

void Music_Control(void)
{
	unsigned char key_value_temp = Get_Key_Value();
	/*--------------------------获取有用的键值信息---------------------------*/
	if((0 == key_value_temp) || (1 == Music_AcceptCmd_KeyInfo(key_value_temp)))
	{
		
	}
	else  //无用信息
	{
		return;
	}
	
	old_music_ctr_cmd = music_ctr_cmd;
	
	music_ctr_cmd = key_value_temp;
	
	
	if(music_ctr_cmd != 0 && music_ctr_cmd != old_music_ctr_cmd)
	{
		switch(music_ctr_cmd)
		{
			case KEY_NO:
			{
			
			}break;
			//开关控制是否需要获取从机的状态再发对应指令   还是由从机自己判断即可？？？  间隔多长时间问询
			//蓝牙控制部分
			case KEY_MUSIC_BLE_SW:
			{
				if((Motor_DemoMode_RunState() == 1) || (Motor_DemoMode_RunState() == 3))
				{
					Motor_DemoMode_ClearPara();
					GetSet_Motor_Ctr_Cmd(KEY_FLAT);				
				}
				else if(Motor_DemoMode_RunState() == 2)
				{
					Motor_DemoMode_ClearPara();
					GetSet_Motor_Ctr_Cmd(KEY_NO);					
				}
				else
				{				
					if(MusicalOsc_Stu.BlueTooth_State[BLUETOOTH_SW_STATE] != SWITCH_STATE_OFF)
					{
						music_para_set_event |= MUSIC_BLE_OFF_EVENT;
						if(MusicalOsc_Stu.MotorFollowMode_State != SWITCH_STATE_OFF) 
						{
							MusicalOsc_Stu.MotorFollowMode_State = SWITCH_STATE_OFF;
							music_para_set_event |= MUSIC_FOLLOW_MODE_SW_EVENT;
						}
					}
					else
					{
						music_para_set_event |= MUSIC_BLE_ON_EVENT;
					}
					if(Master_SearchIdleAdd(MUSIC_DEVICE_TYPE) != 0) 
					{
						msgr_mode_set = MSGR_FOLLOW_MODE;
						Msgr_Ints_FlagArr[0] = MSGR_INTS_ONE_LEVEL;
						Msgr_Ints_FlagArr[1] = MSGR_INTS_ZERO_LEVEL;
						
						Msgr_Ints_FlagArr[2] = MSGR_INTS_ZERO_LEVEL;
						//同步外设发送模式、时间、强度
						msgr_para_set_event |= MSGR_MODE_TIME_EVENT;				
						msgr_para_set_event |= MSGR_ALL_INTS_EVENT;
					}
					//增加打断逻辑
						
				}
			}break;
			case KEY_MOTOR_FOLLOW_MODE_SW:
			{
				if(MusicalOsc_Stu.MotorFollowMode_State != SWITCH_STATE_OFF) 
				{					
					MusicalOsc_Stu.MotorFollowMode_State = SWITCH_STATE_OFF;
					music_para_set_event |= MUSIC_FOLLOW_MODE_SW_EVENT;
					Beep_SingSetPara(1000,1);
				}
				else				
				{
					MusicalOsc_Stu.MotorFollowMode_State = SWITCH_STATE_ON;
					music_para_set_event |= MUSIC_FOLLOW_MODE_SW_EVENT;
					Beep_SingSetPara(200,2);
				}
			}break;
			//TWS组队
			case KEY_MUSIC_TWS_SW:
			{
				if(system_config.flags.ble_mesh_sync_config == BLE_MESH_SYNC_DISABLE)
				{
					if(MusicalOsc_Stu.BlueTooth_State[BLUETOOTH_TWS_STATE] != SWITCH_STATE_OFF)
					{
						music_para_set_event |= MUSIC_TWS_OFF_EVENT;
					}
					else
					{
						music_para_set_event |= MUSIC_TWS_ON_EVENT;
					}
				}
			}break;
			//系统模式控制
			case KEY_MUSIC_DEVICE_MODE:
			{
				if(MusicalOsc_Stu.SysMode_State != MUSIC_SOURCE_BLUE)
				{
					music_para_set_event |= MUSIC_DEMO_OFF_EVENT;
				}
				else
				{
					music_para_set_event |= MUSIC_DEMO_ON_EVENT;
				}
			}break;
		
			
			//音乐控制部分
			case KEY_MUSIC_SW:
			{
				if(MusicalOsc_Stu.Music_PlayState != SWITCH_STATE_OFF)
				{
					music_para_set_event |= MUSIC_PAUSE_EVENT;
				}
				else
				{
					music_para_set_event |= MUSIC_PLAY_EVENT;
				}
				
			}break;	
			case KEY_MUSIC_PRE: //上一曲
			{
				music_para_set_event |= MUSIC_PRE_EVENT;
			}break;
			case KEY_MUSIC_NEXT: //下一曲
			{
				music_para_set_event |= MUSIC_NEXT_EVENT;
			}break;
			case KEY_MUSIC_VOLUME_ADD: //音量+
			{
				music_para_set_event |= MUSIC_VOL_ADD_EVENT;
			}break;
			case KEY_MUSIC_VOLUME_DCR: //音量-
			{
				music_para_set_event |= MUSIC_VOL_DCR_EVENT;
			}break;
			case KEY_MUSIC_MEM_HELP_SLEEP://哄睡保存
			{
				music_para_set_event |= MUSIC_DEMO_SAVE_EVENT;

			}break;
			case KEY_MUSIC_WHITE_NOISE: //白噪音开关
			{
				if(MusicalOsc_Stu.SysMode_State != MUSIC_SOURCE_BLUE)
				{
					if(MusicalOsc_Stu.DemoMode_TrackState[0] == MUSIC_TRACK_1)
					{
						MusicalOsc_Stu.DemoMode_TrackState[0] = MUSIC_TRACK_2;
						MusicalOsc_Stu.DemoMode_Source = MUSIC_DEMO_SOURCE_WHITE_NOISE;
					}
					else if(MusicalOsc_Stu.DemoMode_TrackState[0] == MUSIC_TRACK_2)
					{
						MusicalOsc_Stu.DemoMode_TrackState[0] = MUSIC_TRACK_3;
						MusicalOsc_Stu.DemoMode_Source = MUSIC_DEMO_SOURCE_WHITE_NOISE;
					}
					else if(MusicalOsc_Stu.DemoMode_TrackState[0] == MUSIC_TRACK_3)
					{
						MusicalOsc_Stu.DemoMode_TrackState[0] = MUSIC_TRACK_NOMAL;
					}

					if(MusicalOsc_Stu.DemoMode_TrackState[0] == MUSIC_TRACK_NOMAL)
					{
						music_para_set_event |= MUSIC_MUTE_OFF_EVENT;
					}
					else
					{
						music_para_set_event |= MUSIC_DEMO_SET_TRACK_EVENT;
					}			
				}
				else
				{
					MusicalOsc_Stu.DemoMode_TrackState[0] = MUSIC_TRACK_1;
					MusicalOsc_Stu.DemoMode_Source = MUSIC_DEMO_SOURCE_WHITE_NOISE;
					music_para_set_event |= MUSIC_DEMO_SET_TRACK_EVENT;
				}
			}break;
			default:break;
		}		
	}
	/*---------------------------------------音乐相关状态同步-------------------------------------------------*/
	if(music_para_set_event != 0x00000000)
	{
		if((music_para_set_event & MUSIC_DEMO_SAVE_EVENT) == MUSIC_DEMO_SAVE_EVENT)
		{
			if((KEY_DEMO2_MODE == GetSet_Motor_Ctr_Cmd(0xff)) || (KEY_DEMO1_MODE == GetSet_Motor_Ctr_Cmd(0xff)))
			{
				Beep_SingSetPara(200,3);
				//Light_RgbColour_Stu.led_colour_state = Led_RgbColour_GetState(Light_RgbColour_Stu);
				demo_light_save[0] = Light_RgbColour_Stu.light_colour[RGB_R_BIT];
				demo_light_save[1] = Light_RgbColour_Stu.light_colour[RGB_G_BIT];
				demo_light_save[2] = Light_RgbColour_Stu.light_colour[RGB_B_BIT];
				if(MusicalOsc_Stu.SysMode_State == MUSIC_SOURCE_BLUE)
				{
					demo_music_save[0] = 0;
					demo_music_save[1] = 0;
				}
				else
				{
					if(MusicalOsc_Stu.DemoMode_Source == MUSIC_DEMO_SOURCE_WHITE_NOISE)
					{
						demo_music_save[0] = MUSIC_DEMO_SOURCE_WHITE_NOISE;
						demo_music_save[1] = MusicalOsc_Stu.DemoMode_TrackState[0];					
					}
					else if(MusicalOsc_Stu.DemoMode_Source == MUSIC_DEMO_SOURCE_MUSIC)
					{
						demo_music_save[0] = MUSIC_DEMO_SOURCE_MUSIC;
						demo_music_save[1] = MusicalOsc_Stu.DemoMode_TrackState[1];							
					}
					else
					{
						demo_music_save[0] = 0;
						demo_music_save[1] = 0;					
					}
				}
				demo_volume_save = MusicalOsc_Stu.MusicVolume_Level;
				demo_run_time_save = demo_run_time;
				LIN_Master_Send_WriteKeyValue_Cmd(KEY_MUSIC_MEM_HELP_SLEEP);
			}
			music_para_set_event &= ~MUSIC_DEMO_SAVE_EVENT;		
		}
		//蓝牙开
		if((music_para_set_event & MUSIC_BLE_ON_EVENT) == MUSIC_BLE_ON_EVENT)
		{
			Music_SW(SWITCH_CTR_ON);//音响打开
			MusicalOsc_Stu.BlueTooth_State[BLUETOOTH_SW_STATE] = SWITCH_STATE_ON;
			//上报
			music_state_updata_event |= MUSIC_BLE_ON_EVENT;
			//查询音量
			musicmsgr_status_check |= MUSIC_VOLUME_CHECK;		
			//LIN从机控制盒
			LIN_Master_Send_WriteBuleTooth_Cmd(MUSIC_BLUE_SW,SWITCH_CTR_ON);		
			//本机设备控制	
			if(Master_SearchIdleAdd(MUSIC_DEVICE_TYPE) == 0) //音乐阵子是否存在 
			{
				music_para_set_event &= ~MUSIC_BLE_ON_EVENT;
			}
			else
			{
				if(1 == TTL_Master_Send_WriteBuleTooth_Cmd(MUSIC_BLUE_SW,SWITCH_CTR_ON))		
				{
					Msgr_Ints_FlagArr[0] = MSGR_INTS_ONE_LEVEL;
					Msgr_Ints_FlagArr[1] = MSGR_INTS_ZERO_LEVEL;
					Msgr_Ints_FlagArr[2] = MSGR_INTS_ZERO_LEVEL;
					msgr_mode_set = MSGR_FOLLOW_MODE;
					if(1 == TTL_Master_Send_WriteMsgrFollowInts_Cmd(0x00,Msgr_Ints_FlagArr[0]))
					{
						//查询音量
						musicmsgr_status_check |= MUSIC_VOLUME_CHECK;							
						music_para_set_event &= ~MUSIC_BLE_ON_EVENT;
					}
				}	
			}
		}
		//蓝牙关
		if((music_para_set_event & MUSIC_BLE_OFF_EVENT) == MUSIC_BLE_OFF_EVENT)	
		{
			Music_SW(SWITCH_CTR_OFF);//音响关闭
			
			MusicalOsc_Stu.BlueTooth_State[BLUETOOTH_SW_STATE] = SWITCH_STATE_OFF;
			//上报
			music_state_updata_event |= MUSIC_BLE_OFF_EVENT;	
			//LIN从机控制盒
			LIN_Master_Send_WriteBuleTooth_Cmd(MUSIC_BLUE_SW,SWITCH_CTR_OFF);		
			if(Master_SearchIdleAdd(MUSIC_DEVICE_TYPE) == 0) //音乐阵子是否存在 
			{
				Beep_SingSetPara(200,3);
				music_para_set_event &= ~MUSIC_BLE_OFF_EVENT;
			}
			else
			{
				//本机设备控制				
				if(1 == TTL_Master_Send_WriteBuleTooth_Cmd(MUSIC_BLUE_SW,SWITCH_CTR_OFF))
				{
					Msgr_Ints_FlagArr[0] = MSGR_INTS_ZERO_LEVEL;
					Msgr_Ints_FlagArr[1] = MSGR_INTS_ZERO_LEVEL;
					Msgr_Ints_FlagArr[2] = MSGR_INTS_ZERO_LEVEL;
					msgr_mode_set = MSGR_FOLLOW_MODE;
					if(1 == TTL_Master_Send_WriteMsgrFollowInts_Cmd(0x00,Msgr_Ints_FlagArr[0]))
					{
						music_para_set_event &= ~MUSIC_BLE_OFF_EVENT;
					}
				}
			}
		}	
		//TWS组队
		if((music_para_set_event & MUSIC_TWS_ON_EVENT) == MUSIC_TWS_ON_EVENT)
		{
			MusicalOsc_Stu.BlueTooth_State[BLUETOOTH_TWS_STATE] = SWITCH_STATE_ON;
			//LIN从机控制盒
			LIN_Master_Send_WriteBuleTooth_Cmd(MUSIC_BLUE_SW,SWITCH_CTR_ON);
			LIN_Master_Send_WriteBuleTooth_Cmd(MUSIC_TWS_SW,SWITCH_CTR_ON);			
			//本机设备控制	
			if(1 == TTL_Master_Send_WriteBuleTooth_Cmd(MUSIC_TWS_SW,SWITCH_CTR_ON))
			{
				music_para_set_event &= ~MUSIC_TWS_ON_EVENT;
			}
		}
		//TWS退出组队
		if((music_para_set_event & MUSIC_TWS_OFF_EVENT) == MUSIC_TWS_OFF_EVENT)
		{
			MusicalOsc_Stu.BlueTooth_State[BLUETOOTH_TWS_STATE] = SWITCH_STATE_OFF;
			//LIN从机控制盒
			LIN_Master_Send_WriteBuleTooth_Cmd(MUSIC_TWS_SW,SWITCH_CTR_OFF);
//			LIN_Master_Send_WriteBuleTooth_Cmd(MUSIC_BLUE_SW,SWITCH_CTR_OFF);
			//本机设备控制	
			if(1 == TTL_Master_Send_WriteBuleTooth_Cmd(MUSIC_TWS_SW,SWITCH_CTR_OFF))
			{
				music_para_set_event &= ~MUSIC_TWS_OFF_EVENT;
			}
		}

		//演示模式开
		if((music_para_set_event & MUSIC_DEMO_ON_EVENT) == MUSIC_DEMO_ON_EVENT)
		{
			MusicalOsc_Stu.SysMode_State = MUSIC_SOURCE_U;
			MusicalOsc_Stu.DemoMode_Source = MUSIC_DEMO_SOURCE_USB;
			//上报
			music_state_updata_event |= MUSIC_DEMO_ON_EVENT;	
			//LIN从机控制盒
			LIN_Master_Send_WriteMusicSysMode_Cmd(SWITCH_CTR_ON);
			//本机设备控制	
			if(1 == TTL_Master_Send_WriteMusicSysMode_Cmd(SWITCH_CTR_ON))
			{
				music_para_set_event &= ~MUSIC_DEMO_ON_EVENT;
				//查询音量
				musicmsgr_status_check |= MUSIC_VOLUME_CHECK;			
				musicmsgr_status_check |= MUSIC_FOLLOW_INTS_CHECK;				
			}
		}
		//演示模式关
		if((music_para_set_event & MUSIC_DEMO_OFF_EVENT) == MUSIC_DEMO_OFF_EVENT)
		{
			MusicalOsc_Stu.SysMode_State = MUSIC_SOURCE_BLUE;
			MusicalOsc_Stu.BlueTooth_State[BLUETOOTH_SW_STATE] = SWITCH_STATE_ON;	
			//上报
			music_state_updata_event |= MUSIC_BLE_ON_EVENT;			
			//上报
			music_state_updata_event |= MUSIC_DEMO_OFF_EVENT;		
			//LIN从机控制盒
			LIN_Master_Send_WriteMusicSysMode_Cmd(SWITCH_CTR_OFF);		
			//本机设备控制				
			if(1 == TTL_Master_Send_WriteMusicSysMode_Cmd(SWITCH_CTR_OFF))
			{
				if(1 == TTL_Master_Send_WriteBuleTooth_Cmd(MUSIC_BLUE_SW,SWITCH_CTR_ON))	
				{
					if(1 == TTL_Master_WhiteNoiseSend_SetMusicTrack_Cmd(0))
					{
						musicmsgr_status_check |= MUSIC_FOLLOW_INTS_CHECK;	
						musicmsgr_status_check |= MUSIC_MSGR_MODE_CHECK;	
						//查询音量
						musicmsgr_status_check |= MUSIC_VOLUME_CHECK;						
						music_para_set_event &= ~MUSIC_DEMO_OFF_EVENT;
					}
				}	
			}				
		}
		//静音关闭
		if((music_para_set_event & MUSIC_MUTE_OFF_EVENT) == MUSIC_MUTE_OFF_EVENT)
		{
			MusicalOsc_Stu.BlueTooth_State[BLUETOOTH_SW_STATE] = SWITCH_STATE_OFF;
			MusicalOsc_Stu.SysMode_State = MUSIC_SOURCE_BLUE;
			MusicalOsc_Stu.DemoMode_Source = MUSIC_DEMO_SOURCE_USB;
			//上报
			music_state_updata_event |= MUSIC_BLE_OFF_EVENT;			
			//上报
			music_state_updata_event |= MUSIC_DEMO_OFF_EVENT;		
			
			if(1== TTL_Master_Send_WriteBuleTooth_Cmd(MUSIC_BLUE_SW, SWITCH_MUTE_OFF))
			{
				musicmsgr_status_check |= MUSIC_FOLLOW_INTS_CHECK;	
				musicmsgr_status_check |= MUSIC_MSGR_MODE_CHECK;	
				musicmsgr_status_check |= MUSIC_BLE_SW_CHECK;				
				music_para_set_event &= ~MUSIC_MUTE_OFF_EVENT;
			}		
		}		
		//电机随动模式开关、、没加lin相关指令，等随动模式完善后再加
		if((music_para_set_event & MUSIC_FOLLOW_MODE_SW_EVENT) == MUSIC_FOLLOW_MODE_SW_EVENT)
		{	
			if(MusicalOsc_Stu.MotorFollowMode_State == SWITCH_STATE_ON)//要打开电机随动
			{
				//打开蓝牙
				if(MusicalOsc_Stu.BlueTooth_State[BLUETOOTH_SW_STATE] == SWITCH_STATE_OFF)//如果蓝牙是关闭的，打开蓝牙
				{
					MusicalOsc_Stu.BlueTooth_State[BLUETOOTH_SW_STATE] = SWITCH_STATE_ON;
					//查询音量
					musicmsgr_status_check |= MUSIC_VOLUME_CHECK;		
					if(1 == TTL_Master_Send_WriteBuleTooth_Cmd(MUSIC_BLUE_SW,SWITCH_CTR_ON))	
					{
						Msgr_Ints_FlagArr[1] = MSGR_INTS_ZERO_LEVEL;
						Msgr_Ints_FlagArr[2] = MSGR_INTS_ZERO_LEVEL;
						msgr_mode_set = MSGR_FOLLOW_MODE;
						if(1 == TTL_Master_Send_WriteMotorSysMode_Cmd(MusicalOsc_Stu.MotorFollowMode_State))
						{
							motor_para_set_event |= KEY_MOTOR_FOLLOW_MODE_SWITCH_EVENT;		//电机动作逻辑					
							music_para_set_event &= ~MUSIC_FOLLOW_MODE_SW_EVENT;
						}
					}
				}
				else
				{
					if(1 == TTL_Master_Send_WriteMotorSysMode_Cmd(MusicalOsc_Stu.MotorFollowMode_State))
					{					
						motor_para_set_event |= KEY_MOTOR_FOLLOW_MODE_SWITCH_EVENT;		//电机动作逻辑		
						music_para_set_event &= ~MUSIC_FOLLOW_MODE_SW_EVENT;
					}
				}
			}
			else //关闭电机随动
			{
				if(1 == TTL_Master_Send_WriteMotorSysMode_Cmd(MusicalOsc_Stu.MotorFollowMode_State))
				{
					GetSet_Motor_Ctr_Cmd(0);		
					music_para_set_event &= ~MUSIC_FOLLOW_MODE_SW_EVENT;
				}
			}
		}
		//演示模式曲目设置
		if((music_para_set_event & MUSIC_DEMO_SET_TRACK_EVENT) == MUSIC_DEMO_SET_TRACK_EVENT)
		{
			MusicalOsc_Stu.SysMode_State = MUSIC_SOURCE_U;
			
			music_state_updata_event |= MUSIC_DEMO_SET_TRACK_EVENT;
			
			if(MusicalOsc_Stu.DemoMode_Source == MUSIC_DEMO_SOURCE_WHITE_NOISE)
			{
				LIN_Master_Send_WriteMusicDemo_Source(MUSIC_DEMO_SOURCE_WHITE_NOISE, MusicalOsc_Stu.DemoMode_TrackState[0]);
				if(1 == TTL_Master_Send_WriteMusicDemo_Source(MUSIC_DEMO_SOURCE_WHITE_NOISE, MusicalOsc_Stu.DemoMode_TrackState[0]))
				{
					if(1 == TTL_Master_WhiteNoiseSend_SetMusicTrack_Cmd(MusicalOsc_Stu.DemoMode_TrackState[0]))
					{
						//查询音量
						musicmsgr_status_check |= MUSIC_VOLUME_CHECK;			
						musicmsgr_status_check |= MUSIC_FOLLOW_INTS_CHECK;							
						music_para_set_event &= ~MUSIC_DEMO_SET_TRACK_EVENT;
					}
				}				
			}
			else if (MusicalOsc_Stu.DemoMode_Source == MUSIC_DEMO_SOURCE_MUSIC)
			{
				LIN_Master_Send_WriteMusicDemo_Source(MUSIC_DEMO_SOURCE_MUSIC, MusicalOsc_Stu.DemoMode_TrackState[1]);
				if(1 == TTL_Master_Send_WriteMusicDemo_Source(MUSIC_DEMO_SOURCE_MUSIC, MusicalOsc_Stu.DemoMode_TrackState[1]))
				{
					//查询音量
					musicmsgr_status_check |= MUSIC_VOLUME_CHECK;			
					musicmsgr_status_check |= MUSIC_FOLLOW_INTS_CHECK;							
					music_para_set_event &= ~MUSIC_DEMO_SET_TRACK_EVENT;
				}				
			}
			else
			{
				LIN_Master_Send_WriteMusicDemo_Source(MUSIC_DEMO_SOURCE_USB, 0);
				if(1 == TTL_Master_Send_WriteMusicDemo_Source(MUSIC_DEMO_SOURCE_USB, 0))
				{
					if(1 == TTL_Master_WhiteNoiseSend_SetMusicTrack_Cmd(0))
					{
						//查询音量
						musicmsgr_status_check |= MUSIC_VOLUME_CHECK;			
						musicmsgr_status_check |= MUSIC_FOLLOW_INTS_CHECK;							
						music_para_set_event &= ~MUSIC_DEMO_SET_TRACK_EVENT;
					}
				}				
			}			
		}		
		//音乐播放
		if((music_para_set_event & MUSIC_PLAY_EVENT) == MUSIC_PLAY_EVENT)
		{
			MusicalOsc_Stu.Music_PlayState = SWITCH_STATE_ON;
			
			//上报
			music_state_updata_event |= MUSIC_PLAY_EVENT;
			//查询阵子模式
			musicmsgr_status_check |= MUSIC_FOLLOW_INTS_CHECK;	
			
			//LIN从机控制盒
//			if(MusicalOsc_Stu.BlueTooth_State[BLUETOOTH_TWS_STATE] == SWITCH_STATE_ON)
			{
				LIN_Master_Send_WriteMusicPlay_Cmd(SWITCH_CTR_ON);
			}				
			//本机设备控制
			if(1 == TTL_Master_Send_WriteMusicPlay_Cmd(SWITCH_CTR_ON))
			{
				if(1 == TTL_Master_WhiteNoiseSend_WriteMusicPlay_Cmd(SWITCH_CTR_ON))
				{
					music_para_set_event &= ~MUSIC_PLAY_EVENT;
				}
			}
		}
		//音乐暂停
		if((music_para_set_event & MUSIC_PAUSE_EVENT) == MUSIC_PAUSE_EVENT)
		{
			//上报
			music_state_updata_event |= MUSIC_PAUSE_EVENT;			
			MusicalOsc_Stu.Music_PlayState = SWITCH_STATE_OFF;
			//LIN从机控制盒
//			if(MusicalOsc_Stu.BlueTooth_State[BLUETOOTH_TWS_STATE] == SWITCH_STATE_ON)
			{
				LIN_Master_Send_WriteMusicPlay_Cmd(SWITCH_CTR_OFF);		
			}				
			//本机设备控制				
			if(1 == TTL_Master_Send_WriteMusicPlay_Cmd(SWITCH_CTR_OFF))
			{
				if(1 == TTL_Master_WhiteNoiseSend_WriteMusicPlay_Cmd(SWITCH_CTR_OFF))
				{
					music_para_set_event &= ~MUSIC_PAUSE_EVENT;
				}
			}				
		}
		//上一曲
		if((music_para_set_event & MUSIC_PRE_EVENT) == MUSIC_PRE_EVENT)
		{
			//LIN从机控制盒
//			if(MusicalOsc_Stu.BlueTooth_State[BLUETOOTH_TWS_STATE] == SWITCH_STATE_ON)
			{
				LIN_Master_Send_WriteMusicTrack_Cmd(MUSIC_TRACK_PRE);
			}
			//本机设备控制	
			if(1 == TTL_Master_Send_WriteMusicTrack_Cmd(MUSIC_TRACK_PRE))
			{
				if(1 == TTL_Master_WhiteNoiseSend_WriteMusicTrack_Cmd(MUSIC_TRACK_PRE))
				{
					music_para_set_event &= ~MUSIC_PRE_EVENT;
				}
			}
		}			
		//下一曲
		if((music_para_set_event & MUSIC_NEXT_EVENT) == MUSIC_NEXT_EVENT)
		{
			//LIN从机控制盒
//			if(MusicalOsc_Stu.BlueTooth_State[BLUETOOTH_TWS_STATE] == SWITCH_STATE_ON)
			{
				LIN_Master_Send_WriteMusicTrack_Cmd(MUSIC_TRACK_NEXT);
			}
			//本机设备控制
			if(1 == TTL_Master_Send_WriteMusicTrack_Cmd(MUSIC_TRACK_NEXT))
			{
				if(1 == TTL_Master_WhiteNoiseSend_WriteMusicTrack_Cmd(MUSIC_TRACK_NEXT))
				{
					music_para_set_event &= ~MUSIC_NEXT_EVENT;
				}
			}
		}
		//音量+
		if((music_para_set_event & MUSIC_VOL_ADD_EVENT) == MUSIC_VOL_ADD_EVENT)
		{	
			//上报
			music_state_updata_event |= MUSIC_VOL_SET_EVENT;
//			MusicalOsc_Stu.MusicVolume_Level ++;
//			if(MusicalOsc_Stu.MusicVolume_Level > 15) 	MusicalOsc_Stu.MusicVolume_Level = 15;
//			music_state_updata_event |= MUSIC_VOL_SET_EVENT;			
			//LIN从机控制盒
//			if(MusicalOsc_Stu.BlueTooth_State[BLUETOOTH_TWS_STATE] == SWITCH_STATE_ON)
			{
				LIN_Master_Send_WriteMusicVolume_Cmd(MUSIC_VOLUME_ADD, 0);
			}
			//本机设备控制
			if(1 == TTL_Master_Send_WriteMusicVolume_Cmd(MUSIC_VOLUME_ADD, 0))
			{
				if(1 == TTL_Master_WhiteNoiseSend_WriteMusicVolume_Cmd(MUSIC_VOLUME_ADD, 0))
				{
					//查询音量
					musicmsgr_status_check |= MUSIC_VOLUME_CHECK;					
					music_para_set_event &= ~MUSIC_VOL_ADD_EVENT;
				}
			}
		}			
		//音量-
		if((music_para_set_event & MUSIC_VOL_DCR_EVENT) == MUSIC_VOL_DCR_EVENT)
		{			
			//上报
			music_state_updata_event |= MUSIC_VOL_SET_EVENT;
//			if(MusicalOsc_Stu.MusicVolume_Level > 0) MusicalOsc_Stu.MusicVolume_Level --;
//			music_state_updata_event |= MUSIC_VOL_SET_EVENT;				
			//LIN从机控制盒
//			if(MusicalOsc_Stu.BlueTooth_State[BLUETOOTH_TWS_STATE] == SWITCH_STATE_ON)
			{
				LIN_Master_Send_WriteMusicVolume_Cmd(MUSIC_VOLUME_DCR,0);
			}
			//本机设备控制		
			if(1 == TTL_Master_Send_WriteMusicVolume_Cmd(MUSIC_VOLUME_DCR,0))
			{
				if(1 == TTL_Master_WhiteNoiseSend_WriteMusicVolume_Cmd(MUSIC_VOLUME_DCR,0))
				{
					//查询音量
					musicmsgr_status_check |= MUSIC_VOLUME_CHECK;						
					music_para_set_event &= ~MUSIC_VOL_DCR_EVENT;
				}
			}
		}
		//音量设置
		if((music_para_set_event & MUSIC_VOL_SET_EVENT) == MUSIC_VOL_SET_EVENT)
		{
			//上报
			music_state_updata_event |= MUSIC_VOL_SET_EVENT;		
			//LIN从机控制盒
//			if(MusicalOsc_Stu.BlueTooth_State[BLUETOOTH_TWS_STATE] == SWITCH_STATE_ON)
			{
				LIN_Master_Send_WriteMusicVolume_Cmd(MUSIC_VOLUME_SET,MusicalOsc_Stu.MusicVolume_Level);
			}
			//本机设备控制		
			if(1 == TTL_Master_Send_WriteMusicVolume_Cmd(MUSIC_VOLUME_SET,MusicalOsc_Stu.MusicVolume_Level))
			{				
				if(1 == TTL_Master_WhiteNoiseSend_WriteMusicVolume_Cmd(MUSIC_VOLUME_SET,MusicalOsc_Stu.MusicVolume_Level))
				{
					//查询音量
					musicmsgr_status_check |= MUSIC_VOLUME_CHECK;
					music_para_set_event &= ~MUSIC_VOL_SET_EVENT;
				}
			}				
		}		
		/*---------------------------------------------------------------------------------------------------------*/
	}
}
void Music_Time_ClearCount(void)
{
	music_time_ms_count = 0;
	music_time_sec_count = 0;
}
void Music_TimeManagerTask(void)
{
	//定时器中断调用,每5ms调用一次
	// if(((MusicalOsc_Stu.DemoMode_Source == MUSIC_DEMO_SOURCE_WHITE_NOISE) || 
	// 	 (MusicalOsc_Stu.DemoMode_Source == MUSIC_DEMO_SOURCE_MUSIC))
	// 	 && (MusicalOsc_Stu.SysMode_State == MUSIC_SOURCE_U))
	// {
	// 	Light_Clear_TimeCount(&Light_RgbColour_Stu);
	// 	music_time_ms_count++;
	// 	if(music_time_ms_count >= 200) //200*5ms = 1s
	// 	{
	// 		music_time_ms_count = 0;
	// 		music_time_sec_count++;
	// 		if(music_time_sec_count >= demo_run_time * 60) //每30分钟停止
	// 		{
	// 			music_time_sec_count = 0;
	// 			light_para_set_event |= LIGHT_UBL_OFF_EVENT;//白噪音结束后关灯
	// 			music_state_updata_event |= MUSIC_DEMO_OFF_EVENT;
	// 		}
	// 	}
	// }
	// else
	// {
	// 	music_time_ms_count = 0;
	// 	music_time_sec_count = 0;		
	// }

}

void User_SetMusic_Volume(unsigned char vol_temp)
{
	MusicalOsc_Stu.MusicVolume_Level = vol_temp;
	
	if(MusicalOsc_Stu.MusicVolume_Level > 15) 	MusicalOsc_Stu.MusicVolume_Level = 15;
	
	music_para_set_event |= MUSIC_VOL_SET_EVENT;
}

unsigned char User_MusicDemo(unsigned char step)
{
	unsigned char run_state = 0;
	unsigned char music_run_complete = 0;
	switch(step)
	{
		case 0://关闭演示
		{	
			MusicalOsc_Stu.SysMode_State = MUSIC_SOURCE_BLUE;
			MusicalOsc_Stu.BlueTooth_State[BLUETOOTH_SW_STATE] = SWITCH_CTR_OFF;	
			MusicalOsc_Stu.MotorFollowMode_State = SWITCH_STATE_OFF;
			//上报
			music_state_updata_event |= MUSIC_BLE_OFF_EVENT;
			//查询音量
			musicmsgr_status_check |= MUSIC_VOLUME_CHECK;					
			//上报
			music_state_updata_event |= MUSIC_DEMO_OFF_EVENT;			
			//本机设备控制				
			if(1 == TTL_Master_Send_WriteMusicSysMode_Cmd(SWITCH_CTR_OFF))
			{
				if(1 == TTL_Master_Send_WriteBuleTooth_Cmd(MUSIC_BLUE_SW,SWITCH_CTR_OFF))	
				{
					if(1 == TTL_Master_WhiteNoiseSend_SetMusicTrack_Cmd(0))
					{
						if(1 == TTL_Master_Send_WriteMotorSysMode_Cmd(MusicalOsc_Stu.MotorFollowMode_State))
						{
							musicmsgr_status_check |= MUSIC_FOLLOW_INTS_CHECK;	
							musicmsgr_status_check |= MUSIC_MSGR_MODE_CHECK;						
							run_state = 1;
						}
					}
				}	
			}				
		}
		break;	
		case 1://哄睡打开
		{
			if(demo_music_save[0] == 0)
			{
				MusicalOsc_Stu.SysMode_State = MUSIC_SOURCE_BLUE;
		
				music_state_updata_event |= MUSIC_DEMO_OFF_EVENT;	
				//查询音量
				musicmsgr_status_check |= MUSIC_VOLUME_CHECK;						
				if(MusicalOsc_Stu.Music_PlayState == SWITCH_STATE_OFF)
				{
					//本机设备控制				
					if(1 == TTL_Master_Send_WriteMusicSysMode_Cmd(SWITCH_CTR_OFF))
					{
						run_state = 1;
					}	
				}
				else
				{
					//暂停播放
					//上报
					music_state_updata_event |= MUSIC_PAUSE_EVENT;			
					MusicalOsc_Stu.Music_PlayState = SWITCH_STATE_OFF;				
					//本机设备控制				
					if(1 == TTL_Master_Send_WriteMusicPlay_Cmd(SWITCH_CTR_OFF))
					{
						run_state = 1;
					}						
				}
			}
			else 
			{
				MusicalOsc_Stu.DemoMode_Source = demo_music_save[0];
				
				if(MusicalOsc_Stu.DemoMode_Source == MUSIC_DEMO_SOURCE_WHITE_NOISE)
				{
					MusicalOsc_Stu.SysMode_State = MUSIC_SOURCE_U;
					MusicalOsc_Stu.DemoMode_Source = MUSIC_DEMO_SOURCE_WHITE_NOISE;
					MusicalOsc_Stu.DemoMode_TrackState[0] = demo_music_save[1];
					music_state_updata_event |= MUSIC_DEMO_SET_TRACK_EVENT;
					if(1 == TTL_Master_Send_WriteMusicDemo_Source(MUSIC_DEMO_SOURCE_WHITE_NOISE, demo_music_save[1]))
					{
						if(1 == TTL_Master_WhiteNoiseSend_SetMusicTrack_Cmd(MusicalOsc_Stu.DemoMode_TrackState[0]))
						{
							run_state = 1;
						}
					}				
				}
				else if (MusicalOsc_Stu.DemoMode_Source == MUSIC_DEMO_SOURCE_MUSIC)
				{
					MusicalOsc_Stu.SysMode_State = MUSIC_SOURCE_U;
					MusicalOsc_Stu.DemoMode_Source = MUSIC_DEMO_SOURCE_MUSIC;
					MusicalOsc_Stu.DemoMode_TrackState[1] = demo_music_save[1];					
					music_state_updata_event |= MUSIC_DEMO_SET_TRACK_EVENT;
					if(1 == TTL_Master_Send_WriteMusicDemo_Source(MUSIC_DEMO_SOURCE_MUSIC, demo_music_save[1]))
					{
						run_state = 1;
					}				
				}
				else
				{		
					MusicalOsc_Stu.SysMode_State = MUSIC_SOURCE_BLUE;
			
					music_state_updata_event |= MUSIC_DEMO_OFF_EVENT;	
					
					//本机设备控制				
					if(1 == TTL_Master_Send_WriteMusicSysMode_Cmd(SWITCH_CTR_OFF))
					{
						run_state = 1;
					}				
				}
			}			
		}break;
		case 2:	//	静音关闭
		{
			MusicalOsc_Stu.BlueTooth_State[BLUETOOTH_SW_STATE] = SWITCH_STATE_OFF;
			MusicalOsc_Stu.SysMode_State = MUSIC_SOURCE_BLUE;
			MusicalOsc_Stu.DemoMode_Source = MUSIC_DEMO_SOURCE_USB;
			//music_state_updata_event |= MUSIC_BLE_OFF_EVENT;	
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_MUSIC_BLE_EVENT);
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_MUSIC_DEMO_EVENT);
			MusicalOsc_Stu.MotorFollowMode_State = SWITCH_STATE_OFF;
			if(1== TTL_Master_Send_WriteBuleTooth_Cmd(MUSIC_BLUE_SW, SWITCH_MUTE_OFF))
			{
				if(1 == TTL_Master_WhiteNoiseSend_SetMusicTrack_Cmd(0))
				{
					musicmsgr_status_check |= MUSIC_FOLLOW_INTS_CHECK;	
					musicmsgr_status_check |= MUSIC_MSGR_MODE_CHECK;	
					musicmsgr_status_check |= MUSIC_BLE_SW_CHECK;				
					
					if(1 == TTL_Master_Send_WriteMotorSysMode_Cmd(MusicalOsc_Stu.MotorFollowMode_State))
					{
						run_state = 1;
					}	

				}
			}
		}
		break;
		case 3:	//	恢复音量
		{
			//上报
			music_state_updata_event |= MUSIC_VOL_SET_EVENT;		
			MusicalOsc_Stu.MusicVolume_Level = demo_volume_save;
			//本机设备控制		
			if(1 == TTL_Master_Send_WriteMusicVolume_Cmd(MUSIC_VOLUME_SET,MusicalOsc_Stu.MusicVolume_Level))
			{
				if(1 == TTL_Master_WhiteNoiseSend_WriteMusicVolume_Cmd(MUSIC_VOLUME_SET,MusicalOsc_Stu.MusicVolume_Level))
				{
					run_state = 1;
				}
			}	
		}
		break;
		case 4://闹钟模式
		{
			MusicalOsc_Stu.SysMode_State = MUSIC_SOURCE_U;
			MusicalOsc_Stu.DemoMode_Source = MUSIC_DEMO_SOURCE_CLOCK;
			MusicalOsc_Stu.DemoMode_TrackState[1] = alarm_mode_value[3];					
			music_state_updata_event |= MUSIC_DEMO_SET_TRACK_EVENT;
			if(1 == TTL_Master_Send_WriteMusicDemo_Source(MUSIC_DEMO_SOURCE_CLOCK, alarm_mode_value[3]))
			{
				run_state = 1;
			}				
		}break;	
	}
	
	return run_state;
}
