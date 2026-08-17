#include "app_msgr.h"
#include "delay.h"
#include "app_motor.h"

#include "driver_periph.h"
#include "driver_beep.h"
#include "app_ttlbus.h"
#include "modul_musicmsgr.h"
#include "modul_ttlbus.h"
#include "app_comm.h"
#include "app_linbus.h"
#include "app_config.h"
#include "app_backhual.h"

//振子处于随动模式,按下头部按摩开关等指令,应该只开头部按摩器,脚部按摩器直接关闭

#define MSGE_CHANGE_STOP_1S   (1)   //模式切换停止1S
#define MSGR_MODE_ON_DEVICE   (0)  //模式是否打开按摩器

unsigned long msgr_para_set_event = 0; //按摩状态同步事件

unsigned long msgr_state_updata_event = 0;

MSGR_BASE_PARA  Msgr1_ParaStu = {0};
MSGR_BASE_PARA  Msgr2_ParaStu = {0};
MSGR_BASE_PARA  Msgr3_ParaStu = {0};

static unsigned char msgr_ctr_cmd = 0,old_msgr_ctr_cmd = 0;

//按摩器模式  随振  持续  脉冲  波浪
unsigned char msgr_mode_set = 0;
//按摩器记忆模式  随振  持续  脉冲  波浪
static unsigned char msgr_old_mode_set = 0;
//按摩器定时
unsigned char msgr_min_time_set = MSGR_TIME_LONG;

static unsigned short msgr_time_ms_count = 0;
static unsigned char msgr_time_sec_count = 0;
static unsigned char msgr_time_min_count = 0;

unsigned char Msgr_Ints_FlagArr[SYS_MSGR_NUM + 1] = {0}; //0随振  1头部  2脚部 3腰部
unsigned char Msgr_Old_Ints_FlagArr[SYS_MSGR_NUM + 1] = {0}; //记忆强度 0随振  1头部  2脚部 3腰部


#define MSGR_STOP_TIME_MAX (1000/SYS_TIME_BASE)
unsigned char msgr_mode_stop_flag = 0; //模式切换  需要停止1S在启动
unsigned char msgr_stop_time = 0;

void Msgr_Wave_Mode(MSGR_BASE_PARA *Msgr_ParaStu);

unsigned char Msgr_AcceptCmd_KeyInfo(unsigned char key_temp)
{
	if(key_temp >= KEY_MSGR_START && key_temp <= KEY_MSGR_END)
	{
		return 1;
	}
	return 0;
}

void Msgr_Analy_Info(void)  //获取按摩器状态信息
{
	unsigned char key_value_temp = Get_Key_Value();
	/*--------------------------获取有用的键值信息---------------------------*/
	if((0 == key_value_temp) || (1 == Msgr_AcceptCmd_KeyInfo(key_value_temp)))
	{
		
	}
	else  //无用信息
	{
		return;
	}
	old_msgr_ctr_cmd = msgr_ctr_cmd;
	
	msgr_ctr_cmd = key_value_temp;
	
//	TTL_GetClear_KeyValue(0); //此处是防止重复进入
	
	/*------------------------执行控制指令--------------------------*/
	//按摩器都是点动指令
	if(msgr_ctr_cmd != 0 && msgr_ctr_cmd != old_msgr_ctr_cmd)
	{
		if(Motor_DemoMode_RunState() == 1)
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
			switch(msgr_ctr_cmd)
			{
				case KEY_NO:
				{
				
				}break;
				/*--------------------------------随振模式强度设置--------------------------------------*/
				//单码值遥控器
				case KEY_MUSIC_FOLLOW_INTS_ADD: //随振模式强度增强  重新进入档位为1
				{
					if(msgr_mode_set != MSGR_FOLLOW_MODE)
					{
						msgr_mode_set = MSGR_FOLLOW_MODE;
						Msgr_Ints_FlagArr[0] = MSGR_INTS_ONE_LEVEL;
						
						//进入随震后关闭普通按摩器
						Msgr_Ints_FlagArr[1] = 0;
						Msgr_Ints_FlagArr[2] = 0;
						Msgr_Ints_FlagArr[3] = 0;
					}
					else
					{
						if(1)  //如果处于播放状态 发送增强  目前没有加检测 所以就一直循环即可
						{
							Msgr_Ints_FlagArr[0] ++;
							
							if(Msgr_Ints_FlagArr[0] >= MSGR_INTS_MAX_LEVEL)
							{
								Msgr_Ints_FlagArr[0] = MSGR_INTS_ZERO_LEVEL;
							}
						}
					}
					//设置音乐阵子模式、时间		
					msgr_para_set_event |= MSGR_MODE_TIME_EVENT;				
					//设置音乐阵子随振强度
					msgr_para_set_event |= MSGR_FOLLOW_INTS_EVENT;
				}break;
				//遥控器带指示灯的  需要遥控器发送四个不同的码值
				case KEY_MUSIC_FOLLOW_INTS_ZERO:
				{
					Msgr_Ints_FlagArr[0] = MSGR_INTS_ZERO_LEVEL;
					Msgr_Ints_FlagArr[1] = MSGR_INTS_ZERO_LEVEL;
					Msgr_Ints_FlagArr[2] = MSGR_INTS_ZERO_LEVEL;
					Msgr_Ints_FlagArr[3] = MSGR_INTS_ZERO_LEVEL;
					
					//设置音乐阵子随振强度			
					msgr_para_set_event |= MSGR_ALL_INTS_EVENT;
					msgr_para_set_event |= MSGR_FOLLOW_INTS_EVENT;
					
					
				}break;
				case KEY_MUSIC_FOLLOW_INTS_ONE:
				{
					Msgr_Ints_FlagArr[1] = 0;
					Msgr_Ints_FlagArr[2] = 0;
					Msgr_Ints_FlagArr[3] = 0;
					//设置音乐阵子模式、时间
					msgr_mode_set = MSGR_FOLLOW_MODE;
					msgr_para_set_event |= MSGR_MODE_TIME_EVENT;		
					//设置音乐阵子随振强度
					Msgr_Ints_FlagArr[0] = MSGR_INTS_ONE_LEVEL;
					msgr_para_set_event |= MSGR_FOLLOW_INTS_EVENT;
				}break;
				case KEY_MUSIC_FOLLOW_INTS_TWO:
				{
					Msgr_Ints_FlagArr[1] = 0;
					Msgr_Ints_FlagArr[2] = 0;
					Msgr_Ints_FlagArr[3] = 0;
					//设置音乐阵子模式、时间
					msgr_mode_set = MSGR_FOLLOW_MODE;
					msgr_para_set_event |= MSGR_MODE_TIME_EVENT;	
					//设置音乐阵子随振强度
					Msgr_Ints_FlagArr[0] = MSGR_INTS_TWO_LEVEL;
					msgr_para_set_event |= MSGR_FOLLOW_INTS_EVENT;
				}break;
				case KEY_MUSIC_FOLLOW_INTS_THREE:
				{
					Msgr_Ints_FlagArr[1] = 0;
					Msgr_Ints_FlagArr[2] = 0;
					Msgr_Ints_FlagArr[3] = 0;
					//设置音乐阵子模式、时间
					msgr_para_set_event |= MSGR_MODE_TIME_EVENT;	
					msgr_mode_set = MSGR_FOLLOW_MODE;			
					//设置音乐阵子随振强度				
					Msgr_Ints_FlagArr[0] = MSGR_INTS_THREE_LEVEL;
					msgr_para_set_event |= MSGR_FOLLOW_INTS_EVENT;
				}break;			
				/*-----------------------------------------------典型按摩器-------------------------------------------------------*/
				case KEY_MSGR_MODE:  //模式切换后,档位从1开始
				{
					if(Msgr_Ints_FlagArr[1] != MSGR_INTS_ZERO_LEVEL || Msgr_Ints_FlagArr[2] != MSGR_INTS_ZERO_LEVEL || Msgr_Ints_FlagArr[3] != MSGR_INTS_ZERO_LEVEL)  //按摩器开启的时候 才能切换模式
					{
						Msgr_Ints_FlagArr[0] = 0;
						//模式、时间
						msgr_para_set_event |= MSGR_MODE_TIME_EVENT;
						//
						if(msgr_mode_set != MSGR_CONSTANT_MODE && msgr_mode_set != MSGR_PULSE_MODE && msgr_mode_set != MSGR_WAVE_MODE)
						{
							msgr_mode_set = MSGR_CONSTANT_MODE;
						}
						else
						{
							msgr_stop_time = 0;
							
							if(msgr_mode_set == MSGR_CONSTANT_MODE)
							{
								msgr_mode_set = MSGR_PULSE_MODE;
							}
							else if(msgr_mode_set == MSGR_PULSE_MODE)
							{
								msgr_mode_set = MSGR_WAVE_MODE;
							}
							else //if(msgr_mode_set == MSGR_WAVE_MODE)
							{
								msgr_mode_set = MSGR_CONSTANT_MODE;							
							}
						}
						//1挡
						if(Msgr_Ints_FlagArr[1] != MSGR_INTS_ZERO_LEVEL && Msgr_Ints_FlagArr[2] != MSGR_INTS_ZERO_LEVEL && Msgr_Ints_FlagArr[3] != MSGR_INTS_ZERO_LEVEL)
						{
							memset(Msgr_Ints_FlagArr + 1 , MSGR_INTS_ONE_LEVEL ,SYS_MSGR_NUM);
							#if MSGE_CHANGE_STOP_1S	
							msgr_mode_stop_flag = 3;
							#else
							msgr_para_set_event |= MSGR_ALL_INTS_EVENT;
							#endif
							
						}
						if(Msgr_Ints_FlagArr[1] != MSGR_INTS_ZERO_LEVEL)
						{
							Msgr_Ints_FlagArr[1] = MSGR_INTS_ONE_LEVEL;
							
							#if MSGE_CHANGE_STOP_1S	
							msgr_mode_stop_flag = 1;
							#else
							msgr_para_set_event |= MSGR_ALL_INTS_EVENT;
							#endif
						}
						if(Msgr_Ints_FlagArr[2] != MSGR_INTS_ZERO_LEVEL)
						{
							
							Msgr_Ints_FlagArr[2] = MSGR_INTS_ONE_LEVEL;
							
							#if MSGE_CHANGE_STOP_1S							
							msgr_mode_stop_flag = 2;
							#else
							msgr_para_set_event |= MSGR_ALL_INTS_EVENT;
							#endif
						}
						if(Msgr_Ints_FlagArr[3] != MSGR_INTS_ZERO_LEVEL)
						{
							
							Msgr_Ints_FlagArr[3] = MSGR_INTS_ONE_LEVEL;
							
							#if MSGE_CHANGE_STOP_1S							
							msgr_mode_stop_flag = 2;
							#else
							msgr_para_set_event |= MSGR_ALL_INTS_EVENT;
							#endif
						}
					}
					else
					{
						#if MSGR_MODE_ON_DEVICE
						msgr_mode_set = MSGR_CONSTANT_MODE;
						Msgr_Ints_FlagArr[1] = MSGR_INTS_ONE_LEVEL;Msgr_Ints_FlagArr[2] = MSGR_INTS_ONE_LEVEL;Msgr_Ints_FlagArr[3] = MSGR_INTS_ONE_LEVEL;			
						msgr_para_set_event |= MSGR_ALL_INTS_EVENT;
						#endif
					}
				}break;
				case KEY_MSGR_CONSTANT:
				{
					if(Msgr_Ints_FlagArr[1] != MSGR_INTS_ZERO_LEVEL || Msgr_Ints_FlagArr[2] != MSGR_INTS_ZERO_LEVEL  || Msgr_Ints_FlagArr[3] != MSGR_INTS_ZERO_LEVEL)  //按摩器开启的时候 才能切换模式
					{
						Msgr_Ints_FlagArr[0] = 0;
						//模式、时间
						msgr_para_set_event |= MSGR_MODE_TIME_EVENT;
						msgr_mode_set = MSGR_CONSTANT_MODE;
											//1挡
						if(Msgr_Ints_FlagArr[1] != MSGR_INTS_ZERO_LEVEL && Msgr_Ints_FlagArr[2] != MSGR_INTS_ZERO_LEVEL && Msgr_Ints_FlagArr[3] != MSGR_INTS_ZERO_LEVEL)
						{
							memset(Msgr_Ints_FlagArr + 1 , MSGR_INTS_ONE_LEVEL ,SYS_MSGR_NUM);
							#if MSGE_CHANGE_STOP_1S	
							msgr_mode_stop_flag = 3;
							#else
							msgr_para_set_event |= MSGR_ALL_INTS_EVENT;
							#endif
						}
						if(Msgr_Ints_FlagArr[1] != MSGR_INTS_ZERO_LEVEL)
						{
							Msgr_Ints_FlagArr[1] = MSGR_INTS_ONE_LEVEL;
							
							#if MSGE_CHANGE_STOP_1S	
							msgr_mode_stop_flag = 1;
							#else
							msgr_para_set_event |= MSGR_ALL_INTS_EVENT;
							#endif
						}
						if(Msgr_Ints_FlagArr[2] != MSGR_INTS_ZERO_LEVEL)
						{
							Msgr_Ints_FlagArr[2] = MSGR_INTS_ONE_LEVEL;
							
							#if MSGE_CHANGE_STOP_1S							
							msgr_mode_stop_flag = 2;
							#else
							msgr_para_set_event |= MSGR_ALL_INTS_EVENT;
							#endif
						}
						if(Msgr_Ints_FlagArr[3] != MSGR_INTS_ZERO_LEVEL)
						{
							Msgr_Ints_FlagArr[3] = MSGR_INTS_ONE_LEVEL;
							
							#if MSGE_CHANGE_STOP_1S							
							msgr_mode_stop_flag = 2;
							#else
							msgr_para_set_event |= MSGR_ALL_INTS_EVENT;
							#endif
						}
					}
					else
					{
						#if MSGR_MODE_ON_DEVICE
						msgr_mode_set = MSGR_CONSTANT_MODE;
						Msgr_Ints_FlagArr[1] = MSGR_INTS_ONE_LEVEL;Msgr_Ints_FlagArr[2] = MSGR_INTS_ONE_LEVEL;Msgr_Ints_FlagArr[3] = MSGR_INTS_ONE_LEVEL;			
						msgr_para_set_event |= MSGR_ALL_INTS_EVENT;
						#endif
					}
				}break;
				case KEY_MSGR_PULSE:
				{
					if(Msgr_Ints_FlagArr[1] != MSGR_INTS_ZERO_LEVEL || Msgr_Ints_FlagArr[2] != MSGR_INTS_ZERO_LEVEL || Msgr_Ints_FlagArr[3] != MSGR_INTS_ZERO_LEVEL)  //按摩器开启的时候 才能切换模式
					{
						Msgr_Ints_FlagArr[0] = 0;
						//模式、时间
						msgr_para_set_event |= MSGR_MODE_TIME_EVENT;
						msgr_mode_set = MSGR_PULSE_MODE;
											//1挡
						if(Msgr_Ints_FlagArr[1] != MSGR_INTS_ZERO_LEVEL && Msgr_Ints_FlagArr[2] != MSGR_INTS_ZERO_LEVEL&& Msgr_Ints_FlagArr[3] != MSGR_INTS_ZERO_LEVEL)
						{
							memset(Msgr_Ints_FlagArr + 1 , MSGR_INTS_ONE_LEVEL ,SYS_MSGR_NUM);
							#if MSGE_CHANGE_STOP_1S	
							msgr_mode_stop_flag = 3;
							#else
							msgr_para_set_event |= MSGR_ALL_INTS_EVENT;
							#endif
						}
						if(Msgr_Ints_FlagArr[1] != MSGR_INTS_ZERO_LEVEL)
						{
							Msgr_Ints_FlagArr[1] = MSGR_INTS_ONE_LEVEL;
							
							#if MSGE_CHANGE_STOP_1S	
							msgr_mode_stop_flag = 1;
							#else
							msgr_para_set_event |= MSGR_ALL_INTS_EVENT;
							#endif
						}
						if(Msgr_Ints_FlagArr[2] != MSGR_INTS_ZERO_LEVEL)
						{
							Msgr_Ints_FlagArr[2] = MSGR_INTS_ONE_LEVEL;
							
							#if MSGE_CHANGE_STOP_1S							
							msgr_mode_stop_flag = 2;
							#else
							msgr_para_set_event |= MSGR_ALL_INTS_EVENT;
							#endif
						}
						if(Msgr_Ints_FlagArr[3] != MSGR_INTS_ZERO_LEVEL)
						{
							Msgr_Ints_FlagArr[3] = MSGR_INTS_ONE_LEVEL;
							
							#if MSGE_CHANGE_STOP_1S							
							msgr_mode_stop_flag = 2;
							#else
							msgr_para_set_event |= MSGR_ALL_INTS_EVENT;
							#endif
						}
					}
					else
					{
						#if MSGR_MODE_ON_DEVICE
						msgr_mode_set = MSGR_PULSE_MODE;
						Msgr_Ints_FlagArr[1] = MSGR_INTS_ONE_LEVEL;Msgr_Ints_FlagArr[2] = MSGR_INTS_ONE_LEVEL;Msgr_Ints_FlagArr[3] = MSGR_INTS_ONE_LEVEL;		
						msgr_para_set_event |= MSGR_ALL_INTS_EVENT;
						#endif
					}
				}break;
				case KEY_MSGR_WAVE:
				{
					if(Msgr_Ints_FlagArr[1] != MSGR_INTS_ZERO_LEVEL || Msgr_Ints_FlagArr[2] != MSGR_INTS_ZERO_LEVEL || Msgr_Ints_FlagArr[3] != MSGR_INTS_ZERO_LEVEL)  //按摩器开启的时候 才能切换模式
					{
						Msgr_Ints_FlagArr[0] = 0;
						//模式、时间
						msgr_para_set_event |= MSGR_MODE_TIME_EVENT;
						msgr_mode_set = MSGR_WAVE_MODE;
											//1挡
						if(Msgr_Ints_FlagArr[1] != MSGR_INTS_ZERO_LEVEL && Msgr_Ints_FlagArr[2] != MSGR_INTS_ZERO_LEVEL && Msgr_Ints_FlagArr[3] != MSGR_INTS_ZERO_LEVEL)
						{
							memset(Msgr_Ints_FlagArr + 1 , MSGR_INTS_ONE_LEVEL ,SYS_MSGR_NUM);
							#if MSGE_CHANGE_STOP_1S	
							msgr_mode_stop_flag = 3;
							#else
							msgr_para_set_event |= MSGR_ALL_INTS_EVENT;
							#endif
						}
						if(Msgr_Ints_FlagArr[1] != MSGR_INTS_ZERO_LEVEL)
						{
							Msgr_Ints_FlagArr[1] = MSGR_INTS_ONE_LEVEL;
							
							#if MSGE_CHANGE_STOP_1S	
							msgr_mode_stop_flag = 1;
							#else
							msgr_para_set_event |= MSGR_ALL_INTS_EVENT;
							#endif
						}
						if(Msgr_Ints_FlagArr[2] != MSGR_INTS_ZERO_LEVEL)
						{
							Msgr_Ints_FlagArr[2] = MSGR_INTS_ONE_LEVEL;
							
							#if MSGE_CHANGE_STOP_1S							
							msgr_mode_stop_flag = 2;
							#else
							msgr_para_set_event |= MSGR_ALL_INTS_EVENT;
							#endif
						}
						if(Msgr_Ints_FlagArr[3] != MSGR_INTS_ZERO_LEVEL)
						{
							Msgr_Ints_FlagArr[3] = MSGR_INTS_ONE_LEVEL;
							
							#if MSGE_CHANGE_STOP_1S							
							msgr_mode_stop_flag = 2;
							#else
							msgr_para_set_event |= MSGR_ALL_INTS_EVENT;
							#endif
						}
					}
					else
					{
						#if MSGR_MODE_ON_DEVICE
						msgr_mode_set = MSGR_WAVE_MODE;
						Msgr_Ints_FlagArr[1] = MSGR_INTS_ONE_LEVEL;Msgr_Ints_FlagArr[2] = MSGR_INTS_ONE_LEVEL;Msgr_Ints_FlagArr[3] = MSGR_INTS_ONE_LEVEL;		
						msgr_para_set_event |= MSGR_ALL_INTS_EVENT;
						#endif
					}
				}break;
				case KEY_MSGR_ALL_ON: //开启所有按摩器
				{
					msgr_mode_set = MSGR_CONSTANT_MODE;
					Msgr_Ints_FlagArr[0] = 0;
						//同步外设发送模式、时间、强度
					msgr_para_set_event |= MSGR_MODE_TIME_EVENT;
					msgr_para_set_event |= MSGR_ALL_INTS_EVENT;
					if(system_config.flags.msgr_allon_switch_massage == MSGR_ALL_ON_DISABLE)
					{
						if(Msgr_Ints_FlagArr[1] == MSGR_INTS_ZERO_LEVEL)
						{
							Msgr_Ints_FlagArr[1] = MSGR_INTS_ONE_LEVEL;
						}
						if(Msgr_Ints_FlagArr[2] == MSGR_INTS_ZERO_LEVEL)
						{
							Msgr_Ints_FlagArr[2] = MSGR_INTS_ONE_LEVEL;
						}
						if(Msgr_Ints_FlagArr[3] == MSGR_INTS_ZERO_LEVEL)
						{
							Msgr_Ints_FlagArr[3] = MSGR_INTS_ONE_LEVEL;
						}
					}
					else
					{
						if(system_config.flags.pb4_config == LUMBAR_MASSAGE || system_config.flags.pb5_config == LUMBAR_MASSAGE || \
							 system_config.flags.pb6_config == LUMBAR_MASSAGE || system_config.flags.pb7_config == LUMBAR_MASSAGE)
						{
							if(Msgr_Ints_FlagArr[1] == MSGR_INTS_ZERO_LEVEL && Msgr_Ints_FlagArr[2] == MSGR_INTS_ZERO_LEVEL && Msgr_Ints_FlagArr[3] == MSGR_INTS_ZERO_LEVEL)
							{
								Msgr_Ints_FlagArr[1] = MSGR_INTS_ONE_LEVEL;
								Msgr_Ints_FlagArr[2] = MSGR_INTS_ONE_LEVEL;
								Msgr_Ints_FlagArr[3] = MSGR_INTS_ONE_LEVEL;
							}
							else
							{
								Msgr_Ints_FlagArr[1] = MSGR_INTS_ZERO_LEVEL;
								Msgr_Ints_FlagArr[2] = MSGR_INTS_ZERO_LEVEL;
								Msgr_Ints_FlagArr[3] = MSGR_INTS_ZERO_LEVEL;
							}
						}
						else
						{
							if(Msgr_Ints_FlagArr[1] == MSGR_INTS_ZERO_LEVEL && Msgr_Ints_FlagArr[2] == MSGR_INTS_ZERO_LEVEL )
							{

								Msgr_Ints_FlagArr[1] = MSGR_INTS_ONE_LEVEL;
								Msgr_Ints_FlagArr[2] = MSGR_INTS_ONE_LEVEL;
								Msgr_Ints_FlagArr[3] = MSGR_INTS_ONE_LEVEL;
							}
							else// 如果全部关闭 则全部打开  
							{
								Msgr_Ints_FlagArr[1] = MSGR_INTS_ZERO_LEVEL;
								Msgr_Ints_FlagArr[2] = MSGR_INTS_ZERO_LEVEL;
								Msgr_Ints_FlagArr[3] = MSGR_INTS_ZERO_LEVEL;
							}						
						}

					}
				}break;
				case KEY_MSGR_ALL_OFF: //关闭所有按摩器
				{
					//如果配置关闭的情况下 按摩器全关将数组清零
					if(system_config.flags.msgr_all_close_memory_order == MSGR_ALL_CLOSE_NO_MEMORY)
					{
						memset(Msgr_Ints_FlagArr, MSGR_INTS_ZERO_LEVEL ,sizeof(Msgr_Ints_FlagArr)/sizeof(Msgr_Ints_FlagArr[0]));
						//同步外设发送典型按摩关闭
						msgr_para_set_event |= MSGR_ALL_INTS_EVENT;
						//同步外设随振按摩关闭
						msgr_para_set_event |= MSGR_FOLLOW_INTS_EVENT;
					}
					else if(system_config.flags.msgr_all_close_memory_order == MSGR_ALL_CLOSE_MEMORY)	//如果配置全部关闭记忆时。
					{
						if(system_config.flags.pb4_config == LUMBAR_MASSAGE || system_config.flags.pb5_config == LUMBAR_MASSAGE || \
							system_config.flags.pb6_config == LUMBAR_MASSAGE || system_config.flags.pb7_config == LUMBAR_MASSAGE )
							{
								if((Msgr_Ints_FlagArr[1] == MSGR_INTS_ZERO_LEVEL) && (Msgr_Ints_FlagArr[2] == MSGR_INTS_ZERO_LEVEL) && (Msgr_Ints_FlagArr[2] == MSGR_INTS_ZERO_LEVEL))//全关就开启
								{
									if(Msgr_Old_Ints_FlagArr[1] == MSGR_INTS_ZERO_LEVEL && Msgr_Old_Ints_FlagArr[2] == MSGR_INTS_ZERO_LEVEL  && Msgr_Old_Ints_FlagArr[3] == MSGR_INTS_ZERO_LEVEL)
									{

									}
									else
									{
										Msgr_Ints_FlagArr[1]  = Msgr_Old_Ints_FlagArr[1];
										Msgr_Ints_FlagArr[2]  = Msgr_Old_Ints_FlagArr[2];
										Msgr_Ints_FlagArr[3]  = Msgr_Old_Ints_FlagArr[3];
										msgr_mode_set  = msgr_old_mode_set;
									}
								}
								else
								{
									Msgr_Old_Ints_FlagArr[1] = Msgr_Ints_FlagArr[1];
									Msgr_Old_Ints_FlagArr[2] = Msgr_Ints_FlagArr[2];
									Msgr_Old_Ints_FlagArr[3] = Msgr_Ints_FlagArr[3];
									msgr_old_mode_set = msgr_mode_set;
									Msgr_Ints_FlagArr[0] = MSGR_INTS_ZERO_LEVEL;
									Msgr_Ints_FlagArr[1] = MSGR_INTS_ZERO_LEVEL;
									Msgr_Ints_FlagArr[2] = MSGR_INTS_ZERO_LEVEL;
									Msgr_Ints_FlagArr[3] = MSGR_INTS_ZERO_LEVEL;
								}
							}
							else
							{
								if((Msgr_Ints_FlagArr[1] == MSGR_INTS_ZERO_LEVEL) && (Msgr_Ints_FlagArr[2] == MSGR_INTS_ZERO_LEVEL))//全关就开启
								{
									if(Msgr_Old_Ints_FlagArr[1] == MSGR_INTS_ZERO_LEVEL && Msgr_Old_Ints_FlagArr[2] == MSGR_INTS_ZERO_LEVEL)
									{

									}
									else
									{
										Msgr_Ints_FlagArr[1]  = Msgr_Old_Ints_FlagArr[1];
										Msgr_Ints_FlagArr[2]  = Msgr_Old_Ints_FlagArr[2];
										msgr_mode_set  = msgr_old_mode_set;
									}
								}
								else
								{
									Msgr_Old_Ints_FlagArr[1] = Msgr_Ints_FlagArr[1];
									Msgr_Old_Ints_FlagArr[2] = Msgr_Ints_FlagArr[2];
									msgr_old_mode_set = msgr_mode_set;
									Msgr_Ints_FlagArr[0] = MSGR_INTS_ZERO_LEVEL;
									Msgr_Ints_FlagArr[1] = MSGR_INTS_ZERO_LEVEL;
									Msgr_Ints_FlagArr[2] = MSGR_INTS_ZERO_LEVEL;
								}
							}
						}
							//同步外设发送模式、时间、强度
						msgr_para_set_event |= MSGR_MODE_TIME_EVENT;
						msgr_para_set_event |= MSGR_ALL_INTS_EVENT;
				}break;
				case KEY_MSGR1_SW:
				{
					if(Msgr_Ints_FlagArr[1] != MSGR_INTS_ZERO_LEVEL)
					{
						Msgr_Ints_FlagArr[1] = MSGR_INTS_ZERO_LEVEL;
					}
					else
					{
						if(msgr_mode_set != MSGR_CONSTANT_MODE && msgr_mode_set != MSGR_PULSE_MODE && msgr_mode_set != MSGR_WAVE_MODE)
						{
							msgr_mode_set = MSGR_CONSTANT_MODE;
							Msgr_Ints_FlagArr[0] = 0;
							Msgr_Ints_FlagArr[2] = MSGR_INTS_ZERO_LEVEL;
							Msgr_Ints_FlagArr[3] = MSGR_INTS_ZERO_LEVEL;
						}
						Msgr_Ints_FlagArr[1] = MSGR_INTS_ONE_LEVEL;
					}
					//同步外设发送模式、时间、强度
					msgr_para_set_event |= MSGR_MODE_TIME_EVENT;				
					msgr_para_set_event |= MSGR_ALL_INTS_EVENT;
				}break;		
				case KEY_MSGR1_ON:
				{
					if(Msgr_Ints_FlagArr[1] == MSGR_INTS_ZERO_LEVEL)
					{
						if(msgr_mode_set != MSGR_CONSTANT_MODE && msgr_mode_set != MSGR_PULSE_MODE && msgr_mode_set != MSGR_WAVE_MODE)
						{
							msgr_mode_set = MSGR_CONSTANT_MODE;
							Msgr_Ints_FlagArr[0] = 0;
							Msgr_Ints_FlagArr[2] = MSGR_INTS_ZERO_LEVEL;
							Msgr_Ints_FlagArr[3] = MSGR_INTS_ZERO_LEVEL;
						}
						Msgr_Ints_FlagArr[1] = MSGR_INTS_ONE_LEVEL;
					}
					//同步外设发送模式、时间、强度
					msgr_para_set_event |= MSGR_MODE_TIME_EVENT;				
					msgr_para_set_event |= MSGR_ALL_INTS_EVENT;					
				}break;
				case KEY_MSGR1_INTS_OFF:
				{
					Msgr_Ints_FlagArr[1] = MSGR_INTS_ZERO_LEVEL;
					//同步外设发送模式、时间、强度
					msgr_para_set_event |= MSGR_MODE_TIME_EVENT;				
					msgr_para_set_event |= MSGR_ALL_INTS_EVENT;
				}break;
				case KEY_MSGR1_INTS_ONE:
				{
					if(msgr_mode_set != MSGR_CONSTANT_MODE && msgr_mode_set != MSGR_PULSE_MODE && msgr_mode_set != MSGR_WAVE_MODE)
					{
						msgr_mode_set = MSGR_CONSTANT_MODE;
						Msgr_Ints_FlagArr[2] = MSGR_INTS_ZERO_LEVEL;
						Msgr_Ints_FlagArr[3] = MSGR_INTS_ZERO_LEVEL;
						Msgr_Ints_FlagArr[0] = 0;
					}
					Msgr_Ints_FlagArr[1] = MSGR_INTS_ONE_LEVEL;
					//同步外设发送模式、时间、强度
					msgr_para_set_event |= MSGR_MODE_TIME_EVENT;				
					msgr_para_set_event |= MSGR_ALL_INTS_EVENT;
					
				}break;
				case KEY_MSGR1_INTS_TWO:
				{
					if(msgr_mode_set != MSGR_CONSTANT_MODE && msgr_mode_set != MSGR_PULSE_MODE && msgr_mode_set != MSGR_WAVE_MODE)
					{
						msgr_mode_set = MSGR_CONSTANT_MODE;
						Msgr_Ints_FlagArr[2] = MSGR_INTS_ZERO_LEVEL;	
						Msgr_Ints_FlagArr[3] = MSGR_INTS_ZERO_LEVEL;
						Msgr_Ints_FlagArr[0] = 0;					
					}	
					Msgr_Ints_FlagArr[1] = MSGR_INTS_TWO_LEVEL;
					//同步外设发送模式、时间、强度
					msgr_para_set_event |= MSGR_MODE_TIME_EVENT;				
					msgr_para_set_event |= MSGR_ALL_INTS_EVENT;			
				}break;			
				case KEY_MSGR1_INTS_THREE:
				{
					if(msgr_mode_set != MSGR_CONSTANT_MODE && msgr_mode_set != MSGR_PULSE_MODE && msgr_mode_set != MSGR_WAVE_MODE)
					{
						msgr_mode_set = MSGR_CONSTANT_MODE;
						Msgr_Ints_FlagArr[2] = MSGR_INTS_ZERO_LEVEL;
						Msgr_Ints_FlagArr[0] = 0;
						Msgr_Ints_FlagArr[3] = MSGR_INTS_ZERO_LEVEL;
					}	
					Msgr_Ints_FlagArr[1] = MSGR_INTS_THREE_LEVEL;
					//同步外设发送模式、时间、强度
					msgr_para_set_event |= MSGR_MODE_TIME_EVENT;				
					msgr_para_set_event |= MSGR_ALL_INTS_EVENT;			
				}break;			
				case KEY_MSGR1_INTS_ADD:
				{
					if(msgr_mode_set != MSGR_CONSTANT_MODE && msgr_mode_set != MSGR_PULSE_MODE && msgr_mode_set != MSGR_WAVE_MODE)
					{
						msgr_mode_set = MSGR_CONSTANT_MODE;
						Msgr_Ints_FlagArr[0] = 0;
						Msgr_Ints_FlagArr[1] = MSGR_INTS_ONE_LEVEL;
						Msgr_Ints_FlagArr[2] = MSGR_INTS_ZERO_LEVEL;
						Msgr_Ints_FlagArr[3] = MSGR_INTS_ZERO_LEVEL;			
					}
					else
					{
						Msgr_Ints_FlagArr[1] ++;
						
						if(Msgr_Ints_FlagArr[1] >= MSGR_INTS_MAX_LEVEL)
						{
							Msgr_Ints_FlagArr[1] = MSGR_INTS_ZERO_LEVEL;
						}
					}
					//同步外设发送档位
					msgr_para_set_event |= MSGR_ALL_INTS_EVENT;
					//同步外设发送模式、时间
					msgr_para_set_event |= MSGR_MODE_TIME_EVENT;
				}break;
				case KEY_MSGR1_INTS_DCR:
				{
					if(msgr_mode_set != MSGR_CONSTANT_MODE && msgr_mode_set != MSGR_PULSE_MODE && msgr_mode_set != MSGR_WAVE_MODE)
					{
						msgr_mode_set = MSGR_CONSTANT_MODE;
					}
					if(Msgr_Ints_FlagArr[1] > MSGR_INTS_ZERO_LEVEL)
					{
						Msgr_Ints_FlagArr[0] = 0;
						Msgr_Ints_FlagArr[1] --;
					}
					else if(system_config.flags.msgr_dcr_cycle == MSGR_DCR_CYCLE)
					{
						Msgr_Ints_FlagArr[0] = 0;
						Msgr_Ints_FlagArr[1] = MSGR_INTS_THREE_LEVEL;
					}
					//同步外设发送档位
					msgr_para_set_event |= MSGR_ALL_INTS_EVENT;
					//同步外设发送模式、时间
					msgr_para_set_event |= MSGR_MODE_TIME_EVENT;
				}break;
				

				case KEY_MSGR2_ON:
				{
					if(Msgr_Ints_FlagArr[2] == MSGR_INTS_ZERO_LEVEL)
					{
						if(msgr_mode_set != MSGR_CONSTANT_MODE && msgr_mode_set != MSGR_PULSE_MODE && msgr_mode_set != MSGR_WAVE_MODE)
						{
							msgr_mode_set = MSGR_CONSTANT_MODE;
							Msgr_Ints_FlagArr[0] = 0;
							Msgr_Ints_FlagArr[1] = MSGR_INTS_ZERO_LEVEL;
							Msgr_Ints_FlagArr[3] = MSGR_INTS_ZERO_LEVEL;
						}
						Msgr_Ints_FlagArr[2] = MSGR_INTS_ONE_LEVEL;
					}
					//同步外设发送模式、时间、强度
					msgr_para_set_event |= MSGR_MODE_TIME_EVENT;				
					msgr_para_set_event |= MSGR_ALL_INTS_EVENT;					
				}break;
				case KEY_MSGR2_SW:
				{
					if(Msgr_Ints_FlagArr[2] != MSGR_INTS_ZERO_LEVEL)
					{
						Msgr_Ints_FlagArr[2] = MSGR_INTS_ZERO_LEVEL;
					}
					else
					{
						if(msgr_mode_set != MSGR_CONSTANT_MODE && msgr_mode_set != MSGR_PULSE_MODE && msgr_mode_set != MSGR_WAVE_MODE)
						{
							Msgr_Ints_FlagArr[0] = 0;
							msgr_mode_set = MSGR_CONSTANT_MODE;
							Msgr_Ints_FlagArr[1] = MSGR_INTS_ZERO_LEVEL;	
							Msgr_Ints_FlagArr[3] = MSGR_INTS_ZERO_LEVEL;
						}
						Msgr_Ints_FlagArr[2] = MSGR_INTS_ONE_LEVEL;
					}
					//同步外设发送模式、时间、强度
					msgr_para_set_event |= MSGR_MODE_TIME_EVENT;				
					msgr_para_set_event |= MSGR_ALL_INTS_EVENT;
				}break;			
				case KEY_MSGR2_INTS_OFF:
				{
					Msgr_Ints_FlagArr[2] = MSGR_INTS_ZERO_LEVEL;
					//同步外设发送模式、时间、强度
					msgr_para_set_event |= MSGR_MODE_TIME_EVENT;				
					msgr_para_set_event |= MSGR_ALL_INTS_EVENT;
				}break;
				case KEY_MSGR2_INTS_ONE:
				{
					if(msgr_mode_set != MSGR_CONSTANT_MODE && msgr_mode_set != MSGR_PULSE_MODE && msgr_mode_set != MSGR_WAVE_MODE)
					{
						Msgr_Ints_FlagArr[0] = 0;
						msgr_mode_set = MSGR_CONSTANT_MODE;
						Msgr_Ints_FlagArr[1] = MSGR_INTS_ZERO_LEVEL;
						Msgr_Ints_FlagArr[3] = MSGR_INTS_ZERO_LEVEL;
					}	
					Msgr_Ints_FlagArr[2] = MSGR_INTS_ONE_LEVEL;
					//同步外设发送模式、时间、强度
					msgr_para_set_event |= MSGR_MODE_TIME_EVENT;				
					msgr_para_set_event |= MSGR_ALL_INTS_EVENT;
					
				}break;
				case KEY_MSGR2_INTS_TWO:
				{
					if(msgr_mode_set != MSGR_CONSTANT_MODE && msgr_mode_set != MSGR_PULSE_MODE && msgr_mode_set != MSGR_WAVE_MODE)
					{
						Msgr_Ints_FlagArr[0] = 0;
						msgr_mode_set = MSGR_CONSTANT_MODE;
						Msgr_Ints_FlagArr[1] = MSGR_INTS_ZERO_LEVEL;
						Msgr_Ints_FlagArr[3] = MSGR_INTS_ZERO_LEVEL;
					}	
					Msgr_Ints_FlagArr[2] = MSGR_INTS_TWO_LEVEL;
					//同步外设发送模式、时间、强度
					msgr_para_set_event |= MSGR_MODE_TIME_EVENT;				
					msgr_para_set_event |= MSGR_ALL_INTS_EVENT;			
				}break;			
				case KEY_MSGR2_INTS_THREE:
				{
					if(msgr_mode_set != MSGR_CONSTANT_MODE && msgr_mode_set != MSGR_PULSE_MODE && msgr_mode_set != MSGR_WAVE_MODE)
					{
						Msgr_Ints_FlagArr[0] = 0;
						msgr_mode_set = MSGR_CONSTANT_MODE;
						Msgr_Ints_FlagArr[1] = MSGR_INTS_ZERO_LEVEL;
						Msgr_Ints_FlagArr[3] = MSGR_INTS_ZERO_LEVEL;
					}	
					Msgr_Ints_FlagArr[2] = MSGR_INTS_THREE_LEVEL;
					//同步外设发送模式、时间、强度
					msgr_para_set_event |= MSGR_MODE_TIME_EVENT;				
					msgr_para_set_event |= MSGR_ALL_INTS_EVENT;			
				}break;						
				case KEY_MSGR2_INTS_ADD:
				{
					if(msgr_mode_set != MSGR_CONSTANT_MODE && msgr_mode_set != MSGR_PULSE_MODE && msgr_mode_set != MSGR_WAVE_MODE)
					{
						Msgr_Ints_FlagArr[0] = 0;
						msgr_mode_set = MSGR_CONSTANT_MODE;
						Msgr_Ints_FlagArr[2] = MSGR_INTS_ONE_LEVEL;	
						Msgr_Ints_FlagArr[1] = MSGR_INTS_ZERO_LEVEL;	
						Msgr_Ints_FlagArr[3] = MSGR_INTS_ZERO_LEVEL;
					}
					else
					{
						Msgr_Ints_FlagArr[2] ++;
						
						if(Msgr_Ints_FlagArr[2] >= MSGR_INTS_MAX_LEVEL)
						{
							Msgr_Ints_FlagArr[2] = MSGR_INTS_ZERO_LEVEL;
						}
					}				
					//给外设发送档位
					msgr_para_set_event |= MSGR_ALL_INTS_EVENT;		
					//给外设发送模式、时间
					msgr_para_set_event |= MSGR_MODE_TIME_EVENT;	
				}break;
				case KEY_MSGR2_INTS_DCR:
				{
					if(msgr_mode_set != MSGR_CONSTANT_MODE && msgr_mode_set != MSGR_PULSE_MODE && msgr_mode_set != MSGR_WAVE_MODE)
					{
						msgr_mode_set = MSGR_CONSTANT_MODE;
					}
					if(Msgr_Ints_FlagArr[2] > MSGR_INTS_ZERO_LEVEL)
					{
						Msgr_Ints_FlagArr[0] = 0;
						Msgr_Ints_FlagArr[2] --;
					}
					else if(system_config.flags.msgr_dcr_cycle == MSGR_DCR_CYCLE)
					{
						Msgr_Ints_FlagArr[0] = 0;
						Msgr_Ints_FlagArr[2] = MSGR_INTS_THREE_LEVEL;
					}
					//给外设发送档位
					msgr_para_set_event |= MSGR_ALL_INTS_EVENT;
					//同步外设发送模式、时间
					msgr_para_set_event |= MSGR_MODE_TIME_EVENT;
				}break;	
				case KEY_MSGR3_INTS_DCR:
				{
					if(msgr_mode_set != MSGR_CONSTANT_MODE && msgr_mode_set != MSGR_PULSE_MODE && msgr_mode_set != MSGR_WAVE_MODE)
					{
						msgr_mode_set = MSGR_CONSTANT_MODE;
					}
					if(Msgr_Ints_FlagArr[3] > MSGR_INTS_ZERO_LEVEL)
					{
						Msgr_Ints_FlagArr[0] = 0;
						Msgr_Ints_FlagArr[3] --;
					}
					else if(system_config.flags.msgr_dcr_cycle == MSGR_DCR_CYCLE)
					{
						Msgr_Ints_FlagArr[0] = 0;
						Msgr_Ints_FlagArr[3] = MSGR_INTS_THREE_LEVEL;
					}
					//同步外设发送档位
					msgr_para_set_event |= MSGR_ALL_INTS_EVENT;
					//同步外设发送模式、时间
					msgr_para_set_event |= MSGR_MODE_TIME_EVENT;
				}break;	
				case KEY_MSGR3_INTS_OFF:
				{
					Msgr_Ints_FlagArr[3] = MSGR_INTS_ZERO_LEVEL;
					//同步外设发送模式、时间、强度
					msgr_para_set_event |= MSGR_MODE_TIME_EVENT;				
					msgr_para_set_event |= MSGR_ALL_INTS_EVENT;
				}break;
				case KEY_MSGR3_INTS_ONE:
				{
					if(msgr_mode_set != MSGR_CONSTANT_MODE && msgr_mode_set != MSGR_PULSE_MODE && msgr_mode_set != MSGR_WAVE_MODE)
					{
						Msgr_Ints_FlagArr[0] = 0;
						msgr_mode_set = MSGR_CONSTANT_MODE;
						Msgr_Ints_FlagArr[1] = MSGR_INTS_ZERO_LEVEL;
						Msgr_Ints_FlagArr[2] = MSGR_INTS_ZERO_LEVEL;					
					}	
					Msgr_Ints_FlagArr[3] = MSGR_INTS_ONE_LEVEL;
					//同步外设发送模式、时间、强度
					msgr_para_set_event |= MSGR_MODE_TIME_EVENT;				
					msgr_para_set_event |= MSGR_ALL_INTS_EVENT;
					
				}break;
				case KEY_MSGR3_INTS_TWO:
				{
					if(msgr_mode_set != MSGR_CONSTANT_MODE && msgr_mode_set != MSGR_PULSE_MODE && msgr_mode_set != MSGR_WAVE_MODE)
					{
						Msgr_Ints_FlagArr[0] = 0;
						msgr_mode_set = MSGR_CONSTANT_MODE;
						Msgr_Ints_FlagArr[1] = MSGR_INTS_ZERO_LEVEL;
						Msgr_Ints_FlagArr[2] = MSGR_INTS_ZERO_LEVEL;						
					}	
					Msgr_Ints_FlagArr[3] = MSGR_INTS_TWO_LEVEL;
					//同步外设发送模式、时间、强度
					msgr_para_set_event |= MSGR_MODE_TIME_EVENT;				
					msgr_para_set_event |= MSGR_ALL_INTS_EVENT;			
				}break;			
				case KEY_MSGR3_INTS_THREE:
				{
					if(msgr_mode_set != MSGR_CONSTANT_MODE && msgr_mode_set != MSGR_PULSE_MODE && msgr_mode_set != MSGR_WAVE_MODE)
					{
						Msgr_Ints_FlagArr[0] = 0;
						msgr_mode_set = MSGR_CONSTANT_MODE;
						Msgr_Ints_FlagArr[1] = MSGR_INTS_ZERO_LEVEL;
						Msgr_Ints_FlagArr[2] = MSGR_INTS_ZERO_LEVEL;						
					}	
					Msgr_Ints_FlagArr[3] = MSGR_INTS_THREE_LEVEL;
					//同步外设发送模式、时间、强度
					msgr_para_set_event |= MSGR_MODE_TIME_EVENT;				
					msgr_para_set_event |= MSGR_ALL_INTS_EVENT;			
				}break;	
				case KEY_MSGR3_INTS_ADD:
				{
						if(msgr_mode_set != MSGR_CONSTANT_MODE && msgr_mode_set != MSGR_PULSE_MODE && msgr_mode_set != MSGR_WAVE_MODE)
						{
								Msgr_Ints_FlagArr[0] = 0;
								msgr_mode_set = MSGR_CONSTANT_MODE;
								Msgr_Ints_FlagArr[3] = MSGR_INTS_ONE_LEVEL;
								Msgr_Ints_FlagArr[2] = MSGR_INTS_ZERO_LEVEL;
								Msgr_Ints_FlagArr[1] = MSGR_INTS_ZERO_LEVEL;
						}
						else
						{
								Msgr_Ints_FlagArr[3]++;

								if (Msgr_Ints_FlagArr[3] >= MSGR_INTS_MAX_LEVEL)
								{
										Msgr_Ints_FlagArr[3] = MSGR_INTS_ZERO_LEVEL;
								}
						}

						// 给外设发送档位
						msgr_para_set_event |= MSGR_ALL_INTS_EVENT;
						// 给外设发送模式、时间
						msgr_para_set_event |= MSGR_MODE_TIME_EVENT;
				}
				break;						
				case KEY_MSGR_10MIN:
				{
					msgr_min_time_set = MSGR_TIME_SHORT;
					//给外设发送模式、时间
					msgr_para_set_event |= MSGR_MODE_TIME_EVENT;
					if(system_config.flags.timer_close_massage == TIMER_CLOSE_MASSAGE_ENABLE)
					{
						memset(Msgr_Ints_FlagArr, MSGR_INTS_ZERO_LEVEL ,sizeof(Msgr_Ints_FlagArr)/sizeof(Msgr_Ints_FlagArr[0]));
						//同步外设发送典型按摩关闭
						msgr_para_set_event |= MSGR_ALL_INTS_EVENT;
						//同步外设随振按摩关闭
						msgr_para_set_event |= MSGR_FOLLOW_INTS_EVENT;
					}
				}break;
				case KEY_MSGR_20MIN:
				{
					msgr_min_time_set = MSGR_TIME_MID;
					//给外设发送模式、时间
					msgr_para_set_event |= MSGR_MODE_TIME_EVENT;
					if(system_config.flags.timer_close_massage == TIMER_CLOSE_MASSAGE_ENABLE)
					{
						memset(Msgr_Ints_FlagArr, MSGR_INTS_ZERO_LEVEL ,sizeof(Msgr_Ints_FlagArr)/sizeof(Msgr_Ints_FlagArr[0]));
						//同步外设发送典型按摩关闭
						msgr_para_set_event |= MSGR_ALL_INTS_EVENT;
						//同步外设随振按摩关闭
						msgr_para_set_event |= MSGR_FOLLOW_INTS_EVENT;
					}
				}break;
				case KEY_MSGR_30MIN:
				{
					msgr_min_time_set = MSGR_TIME_LONG;
					//给外设发送模式、时间
					msgr_para_set_event |= MSGR_MODE_TIME_EVENT;	
					if(system_config.flags.timer_close_massage == TIMER_CLOSE_MASSAGE_ENABLE)
					{
						memset(Msgr_Ints_FlagArr, MSGR_INTS_ZERO_LEVEL ,sizeof(Msgr_Ints_FlagArr)/sizeof(Msgr_Ints_FlagArr[0]));
						//同步外设发送典型按摩关闭
						msgr_para_set_event |= MSGR_ALL_INTS_EVENT;
						//同步外设随振按摩关闭
						msgr_para_set_event |= MSGR_FOLLOW_INTS_EVENT;
					}
				}break;
				case KEY_MSGR_TIMER_SET:
				{
					if(msgr_min_time_set == MSGR_TIME_SHORT)
					{
						msgr_min_time_set = MSGR_TIME_MID;
					}
					else if(msgr_min_time_set == MSGR_TIME_MID)
					{
						msgr_min_time_set = MSGR_TIME_LONG;
					}
					else
					{
						msgr_min_time_set = MSGR_TIME_SHORT;
					}
					//给外设发送模式、时间
					msgr_para_set_event |= MSGR_MODE_TIME_EVENT;
				}break;
				default:break;
			}	
		}	
		//清零计时
		Msgr_Clear_TimeCount();			
	}
	/*---------------------------------按摩器全部关闭后,模式自动变为CONTINUE---------------------------------------------*/
	if(msgr_mode_set == MSGR_CONSTANT_MODE || msgr_mode_set == MSGR_PULSE_MODE || msgr_mode_set == MSGR_WAVE_MODE)
	{
		if(Msgr_Ints_FlagArr[1] == MSGR_INTS_ZERO_LEVEL && Msgr_Ints_FlagArr[2] == MSGR_INTS_ZERO_LEVEL && Msgr_Ints_FlagArr[3] == MSGR_INTS_ZERO_LEVEL)
		{
			if(msgr_mode_set != MSGR_CONSTANT_MODE)
			{
				msgr_mode_set = MSGR_CONSTANT_MODE; //屏蔽此句 关闭按摩器可保持关闭前模式
				msgr_para_set_event |= MSGR_MODE_TIME_EVENT;
			}
		}
	}
	//按摩器定时器关闭
	if(msgr_time_min_count >= msgr_min_time_set && msgr_min_time_set > 0) //分钟
	{
		Msgr_Clear_TimeCount();
		memset(Msgr_Ints_FlagArr + 1 , MSGR_INTS_ZERO_LEVEL ,SYS_MSGR_NUM);
		alarm_msgr_running = 0;
		//给音乐阵子发送关闭
		msgr_para_set_event |= MSGR_ALL_INTS_EVENT;				
	}
	//
	/*---------------------------------------按摩器相关状态同步-------------------------------------------------*/
	if(msgr_para_set_event != 0x0000)
	{
		if(Motor_DemoMode_RunState() == 1)
		{
			Motor_DemoMode_ClearPara();
			GetSet_Motor_Ctr_Cmd(KEY_FLAT);
		}
		else if(Motor_DemoMode_RunState() == 2)
		{
			Motor_DemoMode_ClearPara();
			GetSet_Motor_Ctr_Cmd(KEY_NO);
		}	
		//模式时间
		if((msgr_para_set_event & MSGR_MODE_TIME_EVENT) == MSGR_MODE_TIME_EVENT)
		{
			//上报按摩器模式
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_MASSAGE_MODE_EVENT);
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_MASSAGE_TIME_EVENT);
			msgr_state_updata_event |= MSGR_FOLLOW_INTS_EVENT;
			if(msgr_mode_stop_flag != 0) //判断是否停1S
			{
				//LIN
				LIN_Master_Send_WriteMsgrTypicalInts_Cmd(0x00,0x00,0x00); 
				//TTL
				if(1 == TTL_Master_Send_WriteMsgrTypicalInts_Cmd(0x00,0x00))
				{
					msgr_para_set_event &= ~MSGR_MODE_TIME_EVENT;
				}
			}
			//LIN从机控制盒模式
			LIN_Master_Send_WriteMsgrModeTimer_Cmd(0x00,msgr_mode_set,msgr_min_time_set);				
			//本机设备控制模式
			if(1 == TTL_Master_Send_WriteMsgrModeTimer_Cmd(0X00,msgr_mode_set,msgr_min_time_set))
			{
				msgr_para_set_event &= ~MSGR_MODE_TIME_EVENT;
			}	
		}
		//随振强度
		if((msgr_para_set_event & MSGR_FOLLOW_INTS_EVENT) == MSGR_FOLLOW_INTS_EVENT)
		{
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_MASSAGE_INTS_EVENT);
			msgr_state_updata_event |= MSGR_FOLLOW_INTS_EVENT;
			
			memset(Msgr_Ints_FlagArr + 1 , MSGR_INTS_ZERO_LEVEL ,SYS_MSGR_NUM);
			//LIN从机控制盒
			LIN_Master_Send_WriteMsgrFollowInts_Cmd(0x00,Msgr_Ints_FlagArr[0]);
			//本机设备控制
			if(1 == TTL_Master_Send_WriteMsgrFollowInts_Cmd(0x00,Msgr_Ints_FlagArr[0]))
			{
				msgr_para_set_event &= ~MSGR_FOLLOW_INTS_EVENT;
			}				
		}
		//典型强度
		if((msgr_para_set_event & MSGR_ALL_INTS_EVENT) == MSGR_ALL_INTS_EVENT) //
		{
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_MASSAGE_INTS_EVENT);
			msgr_state_updata_event |= MSGR_FOLLOW_INTS_EVENT;
			//LIN从机控制盒
			LIN_Master_Send_WriteMsgrTypicalInts_Cmd(Msgr_Ints_FlagArr[1],Msgr_Ints_FlagArr[2],Msgr_Ints_FlagArr[3]); 
			LIN_Master_Send_WriteMsgrModeTimer_Cmd(0x00,msgr_mode_set,msgr_min_time_set);
			//本机设备控制
			if(1 == TTL_Master_Send_WriteMsgrTypicalInts_Cmd(Msgr_Ints_FlagArr[1],Msgr_Ints_FlagArr[2])) //强度
			{
				msgr_para_set_event &= ~MSGR_ALL_INTS_EVENT;
			}
		}		
	}
	/*------------------------------------------------------------------------------------------------*/
}


void Msgr_Control(void)
{
	Msgr_Analy_Info();
}


void Msgr_TimeManagerTask(void)
{
	static unsigned short msgr_pulse_cycle_time = 0;
	
#if MSGE_CHANGE_STOP_1S		
	
	if(msgr_mode_stop_flag != 0)  //按摩器切换模式 停止1S
	{
		if(msgr_mode_stop_flag != 0x80)
		{
			msgr_stop_time ++;
			if(msgr_stop_time >= MSGR_STOP_TIME_MAX)
			{
				msgr_stop_time = MSGR_STOP_TIME_MAX;
				
				if(1 == msgr_mode_stop_flag || 2 == msgr_mode_stop_flag || 3 == msgr_mode_stop_flag)
				{
					msgr_para_set_event |= MSGR_ALL_INTS_EVENT;
				}

				msgr_mode_stop_flag = 0x80;
			}
			msgr_pulse_cycle_time = 0;
			Msgr1_ParaStu.wave_dir = 0;
			Msgr2_ParaStu.wave_dir = 0;
			Msgr3_ParaStu.wave_dir = 0;
			Massager_M1_Speed(0);Massager_M2_Speed(0);Massager_M3_Speed(0);	
		}
		else
		{
			if((msgr_para_set_event & MSGR_ALL_INTS_EVENT) == 0x00)
			{
				msgr_mode_stop_flag = 0;
			}
		}

		return;
	}
	else
	{
		msgr_stop_time = 0;
	}
#endif	
	
	//只要执行了按摩器指令,计时重置
	//头部按摩器定时20分钟，运行了15分钟后，此时打开脚部按摩器，那么计时重新开始20分钟
	//模式切换后  按摩器计时重新开始
	//切换强度    按摩器计时重新开始
	if(Msgr_Ints_FlagArr[1] != MSGR_INTS_ZERO_LEVEL || Msgr_Ints_FlagArr[2] != MSGR_INTS_ZERO_LEVEL || Msgr_Ints_FlagArr[3] != MSGR_INTS_ZERO_LEVEL)
	{
		//运行计时
		msgr_time_ms_count	++;
		if(msgr_time_ms_count >= 1000/SYS_TIME_BASE) //毫秒
		{
			msgr_time_ms_count = 0;
			
			msgr_time_sec_count ++;
			if(msgr_time_sec_count >= 60) //秒
			{
				msgr_time_sec_count = 0;
				msgr_time_min_count ++ ;
				if(msgr_time_min_count >= msgr_min_time_set) //分钟
				{
					msgr_time_min_count = msgr_min_time_set;				
				}
			}
		}
		//模式动作
		switch(msgr_mode_set)
		{
			case MSGR_CONSTANT_MODE: //持续模式
			{
				//按摩器1
				if(Msgr_Ints_FlagArr[1] == MSGR_INTS_ONE_LEVEL)
				{
					Massager_M1_Speed(MSGR_CONSTANT_ONE_PWM);
				}
				else if(Msgr_Ints_FlagArr[1] == MSGR_INTS_TWO_LEVEL)
				{
					Massager_M1_Speed(MSGR_CONSTANT_TWO_PWM);
				}
				else if(Msgr_Ints_FlagArr[1] == MSGR_INTS_THREE_LEVEL)
				{
					Massager_M1_Speed(MSGR_CONSTANT_THREE_PWM);
				}
				else
				{
					Massager_M1_Speed(0);
				}
				//按摩器2
				if(Msgr_Ints_FlagArr[2] == MSGR_INTS_ONE_LEVEL)
				{
					Massager_M2_Speed(MSGR_CONSTANT_ONE_PWM);
				}
				else if(Msgr_Ints_FlagArr[2] == MSGR_INTS_TWO_LEVEL)
				{
					Massager_M2_Speed(MSGR_CONSTANT_TWO_PWM);
				}
				else if(Msgr_Ints_FlagArr[2] == MSGR_INTS_THREE_LEVEL)
				{
					Massager_M2_Speed(MSGR_CONSTANT_THREE_PWM);
				}
				else
				{
					Massager_M2_Speed(0);
				}	
					// 按摩器3
				if (Msgr_Ints_FlagArr[3] == MSGR_INTS_ONE_LEVEL)
				{
						Massager_M3_Speed(MSGR3_CONSTANT_ONE_PWM);
				}
				else if (Msgr_Ints_FlagArr[3] == MSGR_INTS_TWO_LEVEL)
				{
						Massager_M3_Speed(MSGR3_CONSTANT_TWO_PWM);
				}
				else if (Msgr_Ints_FlagArr[3] == MSGR_INTS_THREE_LEVEL)
				{
						Massager_M3_Speed(MSGR3_CONSTANT_THREE_PWM);
				}
				else
				{
						Massager_M3_Speed(0);
				}
			}break;
			case MSGR_PULSE_MODE:  //脉冲模式
			{
				msgr_pulse_cycle_time ++;
				if(msgr_pulse_cycle_time <= MSGR_PULSE_CYCLE_TIME)
				{
					//按摩器1
					if(Msgr_Ints_FlagArr[1] == MSGR_INTS_ONE_LEVEL)
					{
						Massager_M1_Speed(MSGR_PULSE_ONE_PWM);
					}
					else if(Msgr_Ints_FlagArr[1] == MSGR_INTS_TWO_LEVEL)
					{
						Massager_M1_Speed(MSGR_PULSE_TWO_PWM);
					}
					else if(Msgr_Ints_FlagArr[1] == MSGR_INTS_THREE_LEVEL)
					{
						Massager_M1_Speed(MSGR_PULSE_THREE_PWM);
					}
					else
					{
						Massager_M1_Speed(0);
					}	
					//按摩器2
					if(Msgr_Ints_FlagArr[2] == MSGR_INTS_ONE_LEVEL)
					{
						Massager_M2_Speed(MSGR_PULSE_ONE_PWM);
					}
					else if(Msgr_Ints_FlagArr[2] == MSGR_INTS_TWO_LEVEL)
					{
						Massager_M2_Speed(MSGR_PULSE_TWO_PWM);
					}
					else if(Msgr_Ints_FlagArr[2] == MSGR_INTS_THREE_LEVEL)
					{
						Massager_M2_Speed(MSGR_PULSE_THREE_PWM);
					}
					else
					{
						Massager_M2_Speed(0);
					}
					// 按摩器3
					if(Msgr_Ints_FlagArr[3] == MSGR_INTS_ONE_LEVEL)
					{
						Massager_M3_Speed(MSGR3_PULSE_ONE_PWM);
					}
					else if(Msgr_Ints_FlagArr[3] == MSGR_INTS_TWO_LEVEL)
					{
						Massager_M3_Speed(MSGR3_PULSE_TWO_PWM);
					}
					else if(Msgr_Ints_FlagArr[3] == MSGR_INTS_THREE_LEVEL)
					{
						Massager_M3_Speed(MSGR3_PULSE_THREE_PWM);
					}
					else
					{
						Massager_M3_Speed(0);
					}
				}
				else if(msgr_pulse_cycle_time <= 2 * MSGR_PULSE_CYCLE_TIME)
				{
					Massager_M1_Speed(0);Massager_M2_Speed(0);Massager_M3_Speed(0);
				}
				else
				{
					msgr_pulse_cycle_time = 0;
				}
			}break;
			case MSGR_WAVE_MODE:  //波浪模式
			{
				//按摩器1
				Msgr1_ParaStu.cycle_time_count ++;

				if(Msgr_Ints_FlagArr[1] == MSGR_INTS_ONE_LEVEL) 
				{
					Msgr1_ParaStu.msgr_pwm_min = MSGR_WAVE_LOW_PWM;
					Msgr1_ParaStu.msgr_pwm_max = MSGR_WAVE_ONE_PWM;
					Msgr1_ParaStu.cycle_time_set = MSGR_WAVE_ONE_CYCLE_TIME;
					Msgr1_ParaStu.wave_time_slope = 4;
				}
				else if(Msgr_Ints_FlagArr[1] == MSGR_INTS_TWO_LEVEL) 
				{
					Msgr1_ParaStu.msgr_pwm_min = MSGR_WAVE_LOW_PWM;
					Msgr1_ParaStu.msgr_pwm_max = MSGR_WAVE_TWO_PWM;
					Msgr1_ParaStu.cycle_time_set = MSGR_WAVE_TWO_CYCLE_TIME;
					Msgr1_ParaStu.wave_time_slope = 4.5;
				}
				else if(Msgr_Ints_FlagArr[1] == MSGR_INTS_THREE_LEVEL) 
				{
					Msgr1_ParaStu.msgr_pwm_min = MSGR_WAVE_LOW_PWM;
					Msgr1_ParaStu.msgr_pwm_max = MSGR_WAVE_THREE_PWM;
					Msgr1_ParaStu.cycle_time_set = MSGR_WAVE_THREE_CYCLE_TIME;
					Msgr1_ParaStu.wave_time_slope = 4.2;
				}
				else
				{
					Msgr1_ParaStu.msgr_pwm_min = 0;
					Msgr1_ParaStu.msgr_pwm_max = 0;					
					Msgr1_ParaStu.msgr_pwm = 0;
				}
				if(Msgr_Ints_FlagArr[1] != MSGR_INTS_ZERO_LEVEL)
				{
					Msgr_Wave_Mode(&Msgr1_ParaStu);
					Massager_M1_Speed(Msgr1_ParaStu.msgr_pwm);
				}
				else
				{
					Msgr1_ParaStu.msgr_pwm = 0;
					Massager_M1_Speed(Msgr1_ParaStu.msgr_pwm);
				}
				//按摩器2		
				Msgr2_ParaStu.cycle_time_count ++;

				if(Msgr_Ints_FlagArr[2] == MSGR_INTS_ONE_LEVEL) 
				{
					Msgr2_ParaStu.msgr_pwm_min = MSGR_WAVE_LOW_PWM;
					Msgr2_ParaStu.msgr_pwm_max = MSGR_WAVE_ONE_PWM;
					Msgr2_ParaStu.cycle_time_set = MSGR_WAVE_ONE_CYCLE_TIME;
					Msgr2_ParaStu.wave_time_slope = 4;
				}
				else if(Msgr_Ints_FlagArr[2] == MSGR_INTS_TWO_LEVEL) 
				{
					Msgr2_ParaStu.msgr_pwm_min = MSGR_WAVE_LOW_PWM;
					Msgr2_ParaStu.msgr_pwm_max = MSGR_WAVE_TWO_PWM;
					Msgr2_ParaStu.cycle_time_set = MSGR_WAVE_TWO_CYCLE_TIME;
					Msgr2_ParaStu.wave_time_slope = 4.5;
				}
				else if(Msgr_Ints_FlagArr[2] == MSGR_INTS_THREE_LEVEL) 
				{
					Msgr2_ParaStu.msgr_pwm_min = MSGR_WAVE_LOW_PWM;
					Msgr2_ParaStu.msgr_pwm_max = MSGR_WAVE_THREE_PWM;
					Msgr2_ParaStu.cycle_time_set = MSGR_WAVE_THREE_CYCLE_TIME;
					Msgr2_ParaStu.wave_time_slope = 4.2;
				}
				else
				{
					Msgr2_ParaStu.msgr_pwm_min = 0;
					Msgr2_ParaStu.msgr_pwm_max = 0;
					Msgr2_ParaStu.msgr_pwm = 0;
				}
				if(Msgr_Ints_FlagArr[2] != MSGR_INTS_ZERO_LEVEL)
				{
					Msgr_Wave_Mode(&Msgr2_ParaStu);
					Massager_M2_Speed(Msgr2_ParaStu.msgr_pwm);
				}
				else
				{
					Msgr2_ParaStu.msgr_pwm = 0;
					Massager_M2_Speed(Msgr2_ParaStu.msgr_pwm);
				}
				// 按摩器3
				Msgr3_ParaStu.cycle_time_count++;
				
				if (Msgr_Ints_FlagArr[3] == MSGR_INTS_ONE_LEVEL)
				{
					Msgr3_ParaStu.msgr_pwm_min = MSGR3_WAVE_LOW_PWM;
					Msgr3_ParaStu.msgr_pwm_max = MSGR3_WAVE_ONE_PWM;
					Msgr3_ParaStu.cycle_time_set = MSGR_WAVE_ONE_CYCLE_TIME;
					Msgr3_ParaStu.wave_time_slope = 5;
				}
				else if (Msgr_Ints_FlagArr[3] == MSGR_INTS_TWO_LEVEL)
				{
					Msgr3_ParaStu.msgr_pwm_min = MSGR3_WAVE_LOW_PWM;
					Msgr3_ParaStu.msgr_pwm_max = MSGR3_WAVE_TWO_PWM;
					Msgr3_ParaStu.cycle_time_set = MSGR_WAVE_TWO_CYCLE_TIME;
					Msgr3_ParaStu.wave_time_slope = 2.5;
				}
				else if (Msgr_Ints_FlagArr[3] == MSGR_INTS_THREE_LEVEL)
				{
					Msgr3_ParaStu.msgr_pwm_min = MSGR3_WAVE_LOW_PWM;
					Msgr3_ParaStu.msgr_pwm_max = MSGR3_WAVE_THREE_PWM;
					Msgr3_ParaStu.cycle_time_set = MSGR_WAVE_THREE_CYCLE_TIME;
					Msgr3_ParaStu.wave_time_slope = 2.15;
				}
				else
				{
					Msgr3_ParaStu.msgr_pwm_min = 0;
					Msgr3_ParaStu.msgr_pwm_max = 0;
					Msgr3_ParaStu.msgr_pwm = 0;
				}
				if (Msgr_Ints_FlagArr[3] != MSGR_INTS_ZERO_LEVEL)
				{
					Msgr_Wave_Mode(&Msgr3_ParaStu);
					Massager_M3_Speed(Msgr3_ParaStu.msgr_pwm);
				}
				else
				{
					Msgr3_ParaStu.msgr_pwm = 0;
					Massager_M3_Speed(Msgr3_ParaStu.msgr_pwm);	
				}
			}break;
			default:break;
		}
	}
	else
	{

		msgr_pulse_cycle_time = 0;
		
		Msgr_Clear_TimeCount();
		
		Massager_M1_Speed(0);Massager_M2_Speed(0);Massager_M3_Speed(0);
	}
	
	if(Msgr_Ints_FlagArr[1] == MSGR_INTS_ZERO_LEVEL)
	{
		Msgr1_ParaStu.wave_dir = 0;Msgr1_ParaStu.cycle_time_count = 0;
	}
	if(Msgr_Ints_FlagArr[2] == MSGR_INTS_ZERO_LEVEL)
	{
		Msgr2_ParaStu.wave_dir = 0;Msgr2_ParaStu.cycle_time_count = 0;
	}
	if(Msgr_Ints_FlagArr[3] == MSGR_INTS_ZERO_LEVEL)
	{
		Msgr3_ParaStu.wave_dir = 0;Msgr3_ParaStu.cycle_time_count = 0;
	}
	
	if(msgr_mode_set != MSGR_PULSE_MODE)
	{
		msgr_pulse_cycle_time = 0;
	}
	if(msgr_mode_set != MSGR_WAVE_MODE)
	{
		Msgr1_ParaStu.wave_dir = 0;
		Msgr2_ParaStu.wave_dir = 0;
		Msgr3_ParaStu.wave_dir = 0;
	}	
}

void Msgr_Clear_TimeCount(void)
{
	msgr_time_ms_count = 0;
	msgr_time_sec_count = 0;			
	msgr_time_min_count = 0;	
}

void Msgr_Wave_Mode(MSGR_BASE_PARA *Msgr_ParaStu)
{
	if(0 == Msgr_ParaStu->wave_dir)
	{
//						Msgr_ParaStu->msgr_pwm = MSGR_WAVE_LOW_PWM + Msgr_ParaStu->wave_cycle_time * ((MSGR_WAVE_ONE_PWM - MSGR_WAVE_LOW_PWM) / (MSGR_WAVE_ONE_CYCLE_TIME));
		if(Msgr_ParaStu->cycle_time_count >= Msgr_ParaStu->cycle_time_set)		
		{
			Msgr_ParaStu->wave_dir = 1;
			Msgr_ParaStu->cycle_time_count = 0;
			Msgr_ParaStu->msgr_pwm = Msgr_ParaStu->msgr_pwm_max;
		}
		else
		{
			Msgr_ParaStu->msgr_pwm = MSGR_WAVE_LOW_PWM + Msgr_ParaStu->cycle_time_count * Msgr_ParaStu->wave_time_slope;			
		}
	}
	else
	{
//						Msgr_ParaStu->msgr_pwm = MSGR_WAVE_LOW_PWM + (MSGR_WAVE_ONE_PWM - Msgr_ParaStu->wave_cycle_time * (((MSGR_WAVE_ONE_PWM - MSGR_WAVE_LOW_PWM)) / (MSGR_WAVE_ONE_CYCLE_TIME)));
		if(Msgr_ParaStu->cycle_time_count >= Msgr_ParaStu->cycle_time_set)		
		{
			Msgr_ParaStu->wave_dir = 0;
			Msgr_ParaStu->cycle_time_count = 0;
			Msgr_ParaStu->msgr_pwm = Msgr_ParaStu->msgr_pwm_min;
		}
		else
		{
			Msgr_ParaStu->msgr_pwm = (Msgr_ParaStu->msgr_pwm_max - Msgr_ParaStu->cycle_time_count * Msgr_ParaStu->wave_time_slope);	
		}
	}

}


void User_SetFollowInts_Level(unsigned char ints_temp)
{
	Msgr_Ints_FlagArr[0] = ints_temp;
	msgr_mode_set = MSGR_FOLLOW_MODE;
	//设置音乐阵子随振强度
	msgr_para_set_event |= MSGR_FOLLOW_INTS_EVENT;		
	msgr_para_set_event |= MSGR_MODE_TIME_EVENT;
}
//0-ff,当参数为ff时，忽略此参数
//ints[]:0-3; 1,背部，2腿部，3腰部
void User_SetMassage_Mode(unsigned char *ints, unsigned char mode, unsigned char set_time)
{
	if(mode == 0xff)
	{
		if((ints[1] != 0xff) || (ints[2] != 0xff) || ints[3] != 0xff)
		{
			if(msgr_mode_set == MSGR_FOLLOW_MODE)
			{
				msgr_mode_set = MSGR_CONSTANT_MODE;
				if(ints[1] != 0xff)
				{
					Msgr_Ints_FlagArr[1] = ints[1];
				}
				if(ints[2] != 0xff)
				{
					Msgr_Ints_FlagArr[2] = ints[2];
				}
				if(ints[3] != 0xff)
				{
					Msgr_Ints_FlagArr[3] = ints[3];
				}								
			}
		}
	}
	else
	{
		msgr_mode_set = mode;
		if(ints[1] != 0xff)
		{
			Msgr_Ints_FlagArr[1] = ints[1];
		}
		if(ints[2] != 0xff)
		{
			Msgr_Ints_FlagArr[2] = ints[2];
		}
		if(ints[3] != 0xff)
		{
			Msgr_Ints_FlagArr[3] = ints[3];
		}	
	}
	if(set_time != 0xff)
	{
		msgr_min_time_set = set_time;
		//给外设发送模式、时间
		msgr_para_set_event |= MSGR_MODE_TIME_EVENT;		
	}
	//同步外设发送模式、时间、强度
	msgr_para_set_event |= MSGR_MODE_TIME_EVENT;				
	msgr_para_set_event |= MSGR_ALL_INTS_EVENT;
	
}

unsigned char User_Msgr_Demo(unsigned char step)
{
	unsigned char result = 0;
	switch(step)
	{
		case 0://关闭按摩器
		{
			msgr_mode_set = MSGR_FOLLOW_MODE;
			Msgr_Ints_FlagArr[0] = 0x00;
			Msgr_Ints_FlagArr[1] = 0x00;
			Msgr_Ints_FlagArr[2] = 0x00;
			Msgr_Ints_FlagArr[3] = 0x00;
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_MASSAGE_INTS_EVENT);
			//同步外设发送模式、时间、强度
			if(1 == TTL_Master_Send_WriteMsgrTypicalInts_Cmd(Msgr_Ints_FlagArr[1],Msgr_Ints_FlagArr[2])) //强度
			{
				if(1 == TTL_Master_Send_WriteMsgrFollowInts_Cmd(0x00,Msgr_Ints_FlagArr[0]))
				{
					result = 1;
				}					
			}	
		}
		break;
		case 1://开启所有按摩器 强度1档 模式持续模式
		{
			if(alarm_msgr_running == 0)
			{
				alarm_msgr_running = 1;
				Msgr_Clear_TimeCount();
				Msgr_Ints_FlagArr[0] = MSGR_INTS_ZERO_LEVEL;
				memset(Msgr_Ints_FlagArr + 1, MSGR_INTS_ONE_LEVEL, SYS_MSGR_NUM);
				msgr_mode_set = MSGR_CONSTANT_MODE;
			}
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_MASSAGE_INTS_EVENT);
			//同步外设发送模式、时间、强度
			if(1 == TTL_Master_Send_WriteMsgrTypicalInts_Cmd(Msgr_Ints_FlagArr[1],Msgr_Ints_FlagArr[2])) //强度
			{
				if(1 == TTL_Master_Send_WriteMsgrFollowInts_Cmd(0x00,Msgr_Ints_FlagArr[0]))
				{
					result = 1;
				}					
			}	
		}break;
		case 2:

			break;
		default:

			break;
	}
	return result;
}
