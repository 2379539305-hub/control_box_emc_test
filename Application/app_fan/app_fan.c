#include "app_fan.h"
#include "app_config.h"
#include "app_comm.h"
#include "app_msgr.h"
#include "app_linbus.h"
#include "app_backhual.h"

#include "modul_fan.h"

FAN_STRUCT  Fan_Stu = {0};

static unsigned char  fan_ctr_cmd = 0 , old_fan_ctr_cmd = 0;


unsigned long fan_para_set_event = 0x00000000;

void APP_FanInit(void)
{
	Fan_Stu.fan_time_min_set = system_config.flags.fan_default_time_min;
	
	Fan_Stu.fan_state = 0;
	Fan_Stu.old_fan_state = 1;
}


void Fan_Clear_TimeCount(void)
{
	Fan_Stu.fan_ms_count = 0;
	Fan_Stu.fan_sec_count = 0;
	Fan_Stu.fan_min_count = 0;
}
void Fan_Control(void)
{
	unsigned char key_value_temp = Get_Key_Value();
	/*--------------------------获取有用的键值信息---------------------------*/
	//if((0 == key_value_temp) || (1 == Msgr_AcceptCmd_KeyInfo(key_value_temp)))
	if((0 == key_value_temp) || (key_value_temp >= KEY_FAN_START && key_value_temp <= KEY_FAN_END))
	{
		
	}
	else  //无用信息
	{
		return;
	}
	old_fan_ctr_cmd = fan_ctr_cmd;
	
	fan_ctr_cmd = key_value_temp;
	
	
	if(fan_ctr_cmd != 0 && fan_ctr_cmd != old_fan_ctr_cmd)
	{
		Fan_Clear_TimeCount();
		switch(fan_ctr_cmd)
		{
			case KEY_FAN_ON:
			{
				if(Fan_Stu.Fan_Ints_FlagArr[1] == 0 || Fan_Stu.Fan_Ints_FlagArr[2] == 0 || Fan_Stu.Fan_Ints_FlagArr[3] == 0 || Fan_Stu.Fan_Ints_FlagArr[4] == 0)
				{
					Fan_Stu.fan_mode_set = FAN_CONSTANT_MODE;
					Fan_Stu.Fan_Ints_FlagArr[1] = FAN_INTS_ONE_LEVEL;
					Fan_Stu.Fan_Dir_FlagArr[1] = FAN_FORWARD;
					Fan_Stu.Fan_Ints_FlagArr[2] = FAN_INTS_ONE_LEVEL;
					Fan_Stu.Fan_Dir_FlagArr[2] = FAN_FORWARD;
					Fan_Stu.Fan_Ints_FlagArr[3] = FAN_INTS_ONE_LEVEL;
					Fan_Stu.Fan_Dir_FlagArr[3] = FAN_FORWARD;
					Fan_Stu.Fan_Ints_FlagArr[4] = FAN_INTS_ONE_LEVEL;
					Fan_Stu.Fan_Dir_FlagArr[4] = FAN_FORWARD;
					fan_para_set_event |= FAN_ALL_INTS_EVENT;
					fan_para_set_event |= FAN_MODE_TIME_EVENT;
				}
			}break;
			case KEY_FAN_OFF:
			{
				Fan_Stu.fan_mode_set = FAN_CONSTANT_MODE;
				Fan_Stu.Fan_Ints_FlagArr[1] = FAN_INTS_ZERO_LEVEL;
				Fan_Stu.Fan_Dir_FlagArr[1] = FAN_FORWARD;
				Fan_Stu.Fan_Ints_FlagArr[2] = FAN_INTS_ZERO_LEVEL;
				Fan_Stu.Fan_Dir_FlagArr[2] = FAN_FORWARD;
				Fan_Stu.Fan_Ints_FlagArr[3] = FAN_INTS_ZERO_LEVEL;
				Fan_Stu.Fan_Dir_FlagArr[3] = FAN_FORWARD;
				Fan_Stu.Fan_Ints_FlagArr[4] = FAN_INTS_ZERO_LEVEL;
				Fan_Stu.Fan_Dir_FlagArr[4] = FAN_FORWARD;
				fan_para_set_event |= FAN_ALL_INTS_EVENT;
				fan_para_set_event |= FAN_MODE_TIME_EVENT;
			}break;
			case KEY_FAN_MODE:  //模式切换后,档位从1开始
			{
				if(Fan_Stu.Fan_Ints_FlagArr[1] != FAN_INTS_ZERO_LEVEL ||\
						Fan_Stu.Fan_Ints_FlagArr[2] != FAN_INTS_ZERO_LEVEL||\
						Fan_Stu.Fan_Ints_FlagArr[3] != FAN_INTS_ZERO_LEVEL||\
						Fan_Stu.Fan_Ints_FlagArr[4] != FAN_INTS_ZERO_LEVEL)
				{
					fan_para_set_event |= FAN_MODE_TIME_EVENT;
					if(Fan_Stu.fan_mode_set == FAN_CONSTANT_MODE)
					{
						Fan_Stu.fan_mode_set = FAN_PULSE_MODE;
					}
					else if(Fan_Stu.fan_mode_set == FAN_PULSE_MODE)
					{
						Fan_Stu.fan_mode_set = FAN_WAVE_MODE;
					}
					else
					{
						Fan_Stu.fan_mode_set = FAN_CONSTANT_MODE;
					}
					if(Fan_Stu.Fan_Ints_FlagArr[1] != FAN_INTS_ZERO_LEVEL)
					{
						fan_para_set_event |= FAN_ALL_INTS_EVENT;
						Fan_Stu.Fan_Ints_FlagArr[1] = FAN_INTS_ONE_LEVEL;
					}
					if(Fan_Stu.Fan_Ints_FlagArr[2] != FAN_INTS_ZERO_LEVEL)
					{
						fan_para_set_event |= FAN_ALL_INTS_EVENT;
						Fan_Stu.Fan_Ints_FlagArr[2] = FAN_INTS_ONE_LEVEL;
					}
					if(Fan_Stu.Fan_Ints_FlagArr[3] != FAN_INTS_ZERO_LEVEL)
					{
						fan_para_set_event |= FAN_ALL_INTS_EVENT;
						Fan_Stu.Fan_Ints_FlagArr[3] = FAN_INTS_ONE_LEVEL;
					}
					if(Fan_Stu.Fan_Ints_FlagArr[4] != FAN_INTS_ZERO_LEVEL)
					{
						fan_para_set_event |= FAN_ALL_INTS_EVENT;
						Fan_Stu.Fan_Ints_FlagArr[4] = FAN_INTS_ONE_LEVEL;
					}
					
				}
			}break;
			case KEY_FAN_CONSTANT:
			{
				if(Fan_Stu.Fan_Ints_FlagArr[1] != FAN_INTS_ZERO_LEVEL||\
						Fan_Stu.Fan_Ints_FlagArr[2] != FAN_INTS_ZERO_LEVEL||\
						Fan_Stu.Fan_Ints_FlagArr[3] != FAN_INTS_ZERO_LEVEL||\
						Fan_Stu.Fan_Ints_FlagArr[4] != FAN_INTS_ZERO_LEVEL
				)
				{
					fan_para_set_event |= FAN_MODE_TIME_EVENT;
					Fan_Stu.fan_mode_set = FAN_CONSTANT_MODE;
					if(Fan_Stu.Fan_Ints_FlagArr[1] != FAN_INTS_ZERO_LEVEL)
					{
						fan_para_set_event |= FAN_ALL_INTS_EVENT;
						Fan_Stu.Fan_Ints_FlagArr[1] = FAN_INTS_ONE_LEVEL;
					}
					if(Fan_Stu.Fan_Ints_FlagArr[2] != FAN_INTS_ZERO_LEVEL)
					{
						fan_para_set_event |= FAN_ALL_INTS_EVENT;
						Fan_Stu.Fan_Ints_FlagArr[2] = FAN_INTS_ONE_LEVEL;
					}
					if(Fan_Stu.Fan_Ints_FlagArr[3] != FAN_INTS_ZERO_LEVEL)
					{
						fan_para_set_event |= FAN_ALL_INTS_EVENT;
						Fan_Stu.Fan_Ints_FlagArr[3] = FAN_INTS_ONE_LEVEL;
					}
					if(Fan_Stu.Fan_Ints_FlagArr[4] != FAN_INTS_ZERO_LEVEL)
					{
						fan_para_set_event |= FAN_ALL_INTS_EVENT;
						Fan_Stu.Fan_Ints_FlagArr[4] = FAN_INTS_ONE_LEVEL;
					}
				}
			}break;
			case KEY_FAN_PULSE:
			{
				if(Fan_Stu.Fan_Ints_FlagArr[1] != FAN_INTS_ZERO_LEVEL||\
						Fan_Stu.Fan_Ints_FlagArr[2] != FAN_INTS_ZERO_LEVEL||\
						Fan_Stu.Fan_Ints_FlagArr[3] != FAN_INTS_ZERO_LEVEL||\
						Fan_Stu.Fan_Ints_FlagArr[4] != FAN_INTS_ZERO_LEVEL
				)
				{
					fan_para_set_event |= FAN_MODE_TIME_EVENT;
					Fan_Stu.fan_mode_set = FAN_PULSE_MODE;
					if(Fan_Stu.Fan_Ints_FlagArr[1] != FAN_INTS_ZERO_LEVEL)
					{
						fan_para_set_event |= FAN_ALL_INTS_EVENT;
						Fan_Stu.Fan_Ints_FlagArr[1] = FAN_INTS_ONE_LEVEL;
					}
					if(Fan_Stu.Fan_Ints_FlagArr[2] != FAN_INTS_ZERO_LEVEL)
					{
						fan_para_set_event |= FAN_ALL_INTS_EVENT;
						Fan_Stu.Fan_Ints_FlagArr[2] = FAN_INTS_ONE_LEVEL;
					}
					if(Fan_Stu.Fan_Ints_FlagArr[3] != FAN_INTS_ZERO_LEVEL)
					{
						fan_para_set_event |= FAN_ALL_INTS_EVENT;
						Fan_Stu.Fan_Ints_FlagArr[3] = FAN_INTS_ONE_LEVEL;
					}
					if(Fan_Stu.Fan_Ints_FlagArr[4] != FAN_INTS_ZERO_LEVEL)
					{
						fan_para_set_event |= FAN_ALL_INTS_EVENT;
						Fan_Stu.Fan_Ints_FlagArr[4] = FAN_INTS_ONE_LEVEL;
					}
				}
			}break;
			case KEY_FAN1_INC:
			{
				if(Fan_Stu.fan_mode_set == FAN_NONE_MODE)
				{
					Fan_Stu.fan_mode_set  = FAN_CONSTANT_MODE;
					fan_para_set_event |= FAN_MODE_TIME_EVENT;
				}
				if (Fan_Stu.Fan_Ints_FlagArr[1] != FAN_INTS_ZERO_LEVEL||\
						Fan_Stu.Fan_Ints_FlagArr[2] != FAN_INTS_ZERO_LEVEL)//如果头部风扇动起来
				{
					Fan_Stu.Fan_Ints_FlagArr[1] ++;
//					Fan_Stu.Fan_Dir_FlagArr[1] = FAN_FORWARD;
					Fan_Stu.Fan_Ints_FlagArr[2]  ++;
//					Fan_Stu.Fan_Dir_FlagArr[2] = FAN_FORWARD;
					if((Fan_Stu.Fan_Ints_FlagArr[1] >= FAN_INTS_MAX_LEVEL) || (Fan_Stu.Fan_Ints_FlagArr[2] >= FAN_INTS_MAX_LEVEL))
					{
						Fan_Stu.Fan_Ints_FlagArr[1] = FAN_INTS_ZERO_LEVEL;
						Fan_Stu.Fan_Ints_FlagArr[2] = FAN_INTS_ZERO_LEVEL;
						fan_para_set_event |= FAN_MODE_TIME_EVENT;
					}
				}
				else
				{
					Fan_Stu.Fan_Ints_FlagArr[1] = FAN_INTS_ONE_LEVEL;
					Fan_Stu.Fan_Dir_FlagArr[1] = FAN_FORWARD;
					Fan_Stu.Fan_Ints_FlagArr[2] = FAN_INTS_ONE_LEVEL;
					Fan_Stu.Fan_Dir_FlagArr[2] = FAN_FORWARD;
				}
				fan_para_set_event |= FAN_ALL_INTS_EVENT;
			}break;
			case KEY_FAN2_INC:
			{
				if(Fan_Stu.fan_mode_set == FAN_NONE_MODE)
				{
					Fan_Stu.fan_mode_set  = FAN_CONSTANT_MODE;
					fan_para_set_event |= FAN_MODE_TIME_EVENT;
				}
				if (Fan_Stu.Fan_Ints_FlagArr[3] != FAN_INTS_ZERO_LEVEL||\
						Fan_Stu.Fan_Ints_FlagArr[4] != FAN_INTS_ZERO_LEVEL)//如脚部风扇动起来
				{
					Fan_Stu.Fan_Ints_FlagArr[3] ++;
//					Fan_Stu.Fan_Dir_FlagArr[3] = FAN_FORWARD;
					Fan_Stu.Fan_Ints_FlagArr[4]  ++;
//				Fan_Stu.Fan_Dir_FlagArr[4] = FAN_FORWARD;
					if((Fan_Stu.Fan_Ints_FlagArr[3] >= FAN_INTS_MAX_LEVEL) || (Fan_Stu.Fan_Ints_FlagArr[4] >= FAN_INTS_MAX_LEVEL))
					{
						Fan_Stu.Fan_Ints_FlagArr[3] = FAN_INTS_ZERO_LEVEL;
						Fan_Stu.Fan_Ints_FlagArr[4] = FAN_INTS_ZERO_LEVEL;
						fan_para_set_event |= FAN_MODE_TIME_EVENT;
					}
				}
				else
				{
					Fan_Stu.Fan_Ints_FlagArr[3] = FAN_INTS_ONE_LEVEL;
					Fan_Stu.Fan_Dir_FlagArr[3] = FAN_FORWARD;
					Fan_Stu.Fan_Ints_FlagArr[4] = FAN_INTS_ONE_LEVEL;
					Fan_Stu.Fan_Dir_FlagArr[4] = FAN_FORWARD;
				}
				fan_para_set_event |= FAN_ALL_INTS_EVENT;
			}break;
			case KEY_FAN_WAVE:
			{
				if(Fan_Stu.Fan_Ints_FlagArr[1] != FAN_INTS_ZERO_LEVEL||\
						Fan_Stu.Fan_Ints_FlagArr[2] != FAN_INTS_ZERO_LEVEL||\
						Fan_Stu.Fan_Ints_FlagArr[3] != FAN_INTS_ZERO_LEVEL||\
						Fan_Stu.Fan_Ints_FlagArr[4] != FAN_INTS_ZERO_LEVEL
				)
				{
					fan_para_set_event |= FAN_MODE_TIME_EVENT;
					Fan_Stu.fan_mode_set = FAN_WAVE_MODE;
					if(Fan_Stu.Fan_Ints_FlagArr[1] != FAN_INTS_ZERO_LEVEL)
					{
						fan_para_set_event |= FAN_ALL_INTS_EVENT;
						Fan_Stu.Fan_Ints_FlagArr[1] = FAN_INTS_ONE_LEVEL;
					}
					if(Fan_Stu.Fan_Ints_FlagArr[2] != FAN_INTS_ZERO_LEVEL)
					{
						fan_para_set_event |= FAN_ALL_INTS_EVENT;
						Fan_Stu.Fan_Ints_FlagArr[2] = FAN_INTS_ONE_LEVEL;
					}
					if(Fan_Stu.Fan_Ints_FlagArr[3] != FAN_INTS_ZERO_LEVEL)
					{
						fan_para_set_event |= FAN_ALL_INTS_EVENT;
						Fan_Stu.Fan_Ints_FlagArr[3] = FAN_INTS_ONE_LEVEL;
					}
					if(Fan_Stu.Fan_Ints_FlagArr[4] != FAN_INTS_ZERO_LEVEL)
					{
						fan_para_set_event |= FAN_ALL_INTS_EVENT;
						Fan_Stu.Fan_Ints_FlagArr[4] = FAN_INTS_ONE_LEVEL;
					}
				}
			}break;
			case KEY_FAN_2H:
			{
				Fan_Stu.fan_time_min_set = FAN_TIME_SHORT;
				//给外设发送模式、时间
				fan_para_set_event |= FAN_MODE_TIME_EVENT;
			}break;
			case KEY_FAN_3H:
			{
				Fan_Stu.fan_time_min_set = FAN_TIME_MID;
				//给外设发送模式、时间
				fan_para_set_event |= FAN_MODE_TIME_EVENT;
			}break;
			case KEY_FAN_5H:
			{
				Fan_Stu.fan_time_min_set = FAN_TIME_LONG;
				//给外设发送模式、时间
				fan_para_set_event |= FAN_MODE_TIME_EVENT;
			}break;
			case KEY_FAN1_OFF:
			{
				Fan_Stu.Fan_Ints_FlagArr[1] = FAN_INTS_ZERO_LEVEL;
				Fan_Stu.Fan_Dir_FlagArr[1] = FAN_FORWARD;
				Fan_Stu.Fan_Ints_FlagArr[2] = FAN_INTS_ZERO_LEVEL;
				Fan_Stu.Fan_Dir_FlagArr[2] = FAN_FORWARD;
				if(Fan_Stu.Fan_Ints_FlagArr[1] == FAN_INTS_ZERO_LEVEL&&\
						Fan_Stu.Fan_Ints_FlagArr[2] == FAN_INTS_ZERO_LEVEL&&\
						Fan_Stu.Fan_Ints_FlagArr[3] == FAN_INTS_ZERO_LEVEL&&\
						Fan_Stu.Fan_Ints_FlagArr[4] == FAN_INTS_ZERO_LEVEL
				)
				{
					Fan_Stu.fan_mode_set = FAN_NONE_MODE;
				}
				fan_para_set_event |= FAN_ALL_INTS_EVENT;
				fan_para_set_event |= FAN_MODE_TIME_EVENT;
			}break;
			case KEY_FAN2_OFF:
			{
				if(Fan_Stu.Fan_Ints_FlagArr[1] == FAN_INTS_ZERO_LEVEL&&\
						Fan_Stu.Fan_Ints_FlagArr[2] == FAN_INTS_ZERO_LEVEL&&\
						Fan_Stu.Fan_Ints_FlagArr[3] == FAN_INTS_ZERO_LEVEL&&\
						Fan_Stu.Fan_Ints_FlagArr[4] == FAN_INTS_ZERO_LEVEL
				)
				{
					Fan_Stu.fan_mode_set = FAN_NONE_MODE;
				}
				Fan_Stu.Fan_Ints_FlagArr[3] = FAN_INTS_ZERO_LEVEL;
				Fan_Stu.Fan_Dir_FlagArr[3] = FAN_FORWARD;
				Fan_Stu.Fan_Ints_FlagArr[4] = FAN_INTS_ZERO_LEVEL;
				Fan_Stu.Fan_Dir_FlagArr[4] = FAN_FORWARD;
				fan_para_set_event |= FAN_ALL_INTS_EVENT;
				fan_para_set_event |= FAN_MODE_TIME_EVENT;
			}break;
			case KEY_FAN_DIR_SW:
			{
				if(Fan_Stu.Fan_Ints_FlagArr[1] != FAN_INTS_ZERO_LEVEL||\
						Fan_Stu.Fan_Ints_FlagArr[2] != FAN_INTS_ZERO_LEVEL||\
						Fan_Stu.Fan_Ints_FlagArr[3] != FAN_INTS_ZERO_LEVEL||\
						Fan_Stu.Fan_Ints_FlagArr[4] != FAN_INTS_ZERO_LEVEL
				)
				{
					if((Fan_Stu.Fan_Dir_FlagArr[1] == FAN_FORWARD) && (Fan_Stu.Fan_Dir_FlagArr[2] == FAN_FORWARD) 
						&& (Fan_Stu.Fan_Dir_FlagArr[3] == FAN_FORWARD) && (Fan_Stu.Fan_Dir_FlagArr[4] == FAN_FORWARD) )
					{
						Fan_Stu.Fan_Dir_FlagArr[1] = FAN_BACKWARD;
						Fan_Stu.Fan_Dir_FlagArr[2] = FAN_BACKWARD;
						Fan_Stu.Fan_Dir_FlagArr[3] = FAN_BACKWARD;
						Fan_Stu.Fan_Dir_FlagArr[4] = FAN_BACKWARD;
					}
					else
					{
						Fan_Stu.Fan_Dir_FlagArr[1] = FAN_FORWARD;
						Fan_Stu.Fan_Dir_FlagArr[2] = FAN_FORWARD;
						Fan_Stu.Fan_Dir_FlagArr[3] = FAN_FORWARD;
						Fan_Stu.Fan_Dir_FlagArr[4] = FAN_FORWARD;					
					}
					
					if(Fan_Stu.Fan_Ints_FlagArr[1] != FAN_INTS_ZERO_LEVEL)
					{
						Fan_Stu.Fan_Ints_FlagArr[1] = FAN_INTS_ONE_LEVEL;
					}
					if(Fan_Stu.Fan_Ints_FlagArr[2] != FAN_INTS_ZERO_LEVEL)
					{
						Fan_Stu.Fan_Ints_FlagArr[2] = FAN_INTS_ONE_LEVEL;
					}
					if(Fan_Stu.Fan_Ints_FlagArr[3] != FAN_INTS_ZERO_LEVEL)
					{
						Fan_Stu.Fan_Ints_FlagArr[3] = FAN_INTS_ONE_LEVEL;
					}
					if(Fan_Stu.Fan_Ints_FlagArr[4] != FAN_INTS_ZERO_LEVEL)
					{
						Fan_Stu.Fan_Ints_FlagArr[4] = FAN_INTS_ONE_LEVEL;
					}					
					fan_para_set_event |= FAN_ALL_INTS_EVENT;
				}				
			}break;
			case KEY_FAN1_FORWARD_ONE:
			{
				if(Fan_Stu.fan_mode_set == FAN_NONE_MODE)
				{
					Fan_Stu.fan_mode_set = FAN_CONSTANT_MODE;
					fan_para_set_event |= FAN_MODE_TIME_EVENT;
				}
				Fan_Stu.Fan_Ints_FlagArr[1] = FAN_INTS_ONE_LEVEL;
				Fan_Stu.Fan_Dir_FlagArr[1] = FAN_FORWARD;
				Fan_Stu.Fan_Ints_FlagArr[2] = FAN_INTS_ONE_LEVEL;
				Fan_Stu.Fan_Dir_FlagArr[2] = FAN_FORWARD;
				fan_para_set_event |= FAN_ALL_INTS_EVENT;
			}break;
			case KEY_FAN1_FORWARD_TWO:
			{
				if(Fan_Stu.fan_mode_set == FAN_NONE_MODE)
				{
					Fan_Stu.fan_mode_set = FAN_CONSTANT_MODE;
					fan_para_set_event |= FAN_MODE_TIME_EVENT;
				}
				Fan_Stu.Fan_Ints_FlagArr[1] = FAN_INTS_TWO_LEVEL;
				Fan_Stu.Fan_Dir_FlagArr[1] = FAN_FORWARD;
				Fan_Stu.Fan_Ints_FlagArr[2] = FAN_INTS_TWO_LEVEL;
				Fan_Stu.Fan_Dir_FlagArr[2] = FAN_FORWARD;
				fan_para_set_event |= FAN_ALL_INTS_EVENT;
			}break;
			case KEY_FAN1_FORWARD_THREE:
			{
				if(Fan_Stu.fan_mode_set == FAN_NONE_MODE)
				{
					Fan_Stu.fan_mode_set = FAN_CONSTANT_MODE;
					fan_para_set_event |= FAN_MODE_TIME_EVENT;
				}
				Fan_Stu.Fan_Ints_FlagArr[1] = FAN_INTS_THREE_LEVEL;
				Fan_Stu.Fan_Dir_FlagArr[1] = FAN_FORWARD;
				Fan_Stu.Fan_Ints_FlagArr[2] = FAN_INTS_THREE_LEVEL;
				Fan_Stu.Fan_Dir_FlagArr[2] = FAN_FORWARD;
				fan_para_set_event |= FAN_ALL_INTS_EVENT;
			}break;
			// case KEY_FAN1_BACKWARD_ONE://超内存 注释
			// {
			// 	if(Fan_Stu.fan_mode_set == FAN_NONE_MODE)
			// 	{
			// 		Fan_Stu.fan_mode_set = FAN_CONSTANT_MODE;
			// 		fan_para_set_event |= FAN_MODE_TIME_EVENT;
			// 	}
			// 	Fan_Stu.Fan_Ints_FlagArr[1] = FAN_INTS_ONE_LEVEL;
			// 	Fan_Stu.Fan_Dir_FlagArr[1] = FAN_BACKWARD;
			// 	Fan_Stu.Fan_Ints_FlagArr[2] = FAN_INTS_ONE_LEVEL;
			// 	Fan_Stu.Fan_Dir_FlagArr[2] = FAN_FORWARD;
			// 	fan_para_set_event |= FAN_ALL_INTS_EVENT;
			// }break;
			// case KEY_FAN1_BACKWARD_TWO:
			// {
			// 	if(Fan_Stu.fan_mode_set == FAN_NONE_MODE)
			// 	{
			// 		Fan_Stu.fan_mode_set = FAN_CONSTANT_MODE;
			// 		fan_para_set_event |= FAN_MODE_TIME_EVENT;
			// 	}
			// 	Fan_Stu.Fan_Ints_FlagArr[1] = FAN_INTS_TWO_LEVEL;
			// 	Fan_Stu.Fan_Dir_FlagArr[1] = FAN_BACKWARD;
			// 	Fan_Stu.Fan_Ints_FlagArr[2] = FAN_INTS_TWO_LEVEL;
			// 	Fan_Stu.Fan_Dir_FlagArr[2] = FAN_FORWARD;
			// 	fan_para_set_event |= FAN_ALL_INTS_EVENT;
			// }break;
			// case KEY_FAN1_BACKWARD_THREE:
			// {
			// 	if(Fan_Stu.fan_mode_set == FAN_NONE_MODE)
			// 	{
			// 		Fan_Stu.fan_mode_set = FAN_CONSTANT_MODE;
			// 		fan_para_set_event |= FAN_MODE_TIME_EVENT;
			// 	}
			// 	Fan_Stu.Fan_Ints_FlagArr[1] = FAN_INTS_THREE_LEVEL;
			// 	Fan_Stu.Fan_Dir_FlagArr[1] = FAN_BACKWARD;
			// 	Fan_Stu.Fan_Ints_FlagArr[2] = FAN_INTS_THREE_LEVEL;
			// 	Fan_Stu.Fan_Dir_FlagArr[2] = FAN_FORWARD;
			// 	fan_para_set_event |= FAN_ALL_INTS_EVENT;
			// }break;
			case KEY_FAN2_FORWARD_ONE:
			{
				if(Fan_Stu.fan_mode_set == FAN_NONE_MODE)
				{
					Fan_Stu.fan_mode_set = FAN_CONSTANT_MODE;
					fan_para_set_event |= FAN_MODE_TIME_EVENT;
				}
				Fan_Stu.Fan_Ints_FlagArr[3] = FAN_INTS_ONE_LEVEL;
				Fan_Stu.Fan_Dir_FlagArr[3] = FAN_FORWARD;
				Fan_Stu.Fan_Ints_FlagArr[4] = FAN_INTS_ONE_LEVEL;
				Fan_Stu.Fan_Dir_FlagArr[4] = FAN_FORWARD;
				fan_para_set_event |= FAN_ALL_INTS_EVENT;
			}break;
			case KEY_FAN2_FORWARD_TWO:
			{
				if(Fan_Stu.fan_mode_set == FAN_NONE_MODE)
				{
					Fan_Stu.fan_mode_set = FAN_CONSTANT_MODE;
					fan_para_set_event |= FAN_MODE_TIME_EVENT;
				}
				Fan_Stu.Fan_Ints_FlagArr[3] = FAN_INTS_TWO_LEVEL;
				Fan_Stu.Fan_Dir_FlagArr[3] = FAN_FORWARD;
				Fan_Stu.Fan_Ints_FlagArr[4] = FAN_INTS_TWO_LEVEL;
				Fan_Stu.Fan_Dir_FlagArr[4] = FAN_FORWARD;
				fan_para_set_event |= FAN_ALL_INTS_EVENT;
			}break;
			case KEY_FAN2_FORWARD_THREE:
			{
				if(Fan_Stu.fan_mode_set == FAN_NONE_MODE)
				{
					Fan_Stu.fan_mode_set = FAN_CONSTANT_MODE;
					fan_para_set_event |= FAN_MODE_TIME_EVENT;
				}
				Fan_Stu.Fan_Ints_FlagArr[3] = FAN_INTS_THREE_LEVEL;
				Fan_Stu.Fan_Dir_FlagArr[3] = FAN_FORWARD;
				Fan_Stu.Fan_Ints_FlagArr[4] = FAN_INTS_THREE_LEVEL;
				Fan_Stu.Fan_Dir_FlagArr[4] = FAN_FORWARD;
				fan_para_set_event |= FAN_ALL_INTS_EVENT;
			}break;
			// case KEY_FAN2_BACKWARD_ONE:
			// {
			// 	if(Fan_Stu.fan_mode_set == FAN_NONE_MODE)
			// 	{
			// 		Fan_Stu.fan_mode_set = FAN_CONSTANT_MODE;
			// 		fan_para_set_event |= FAN_MODE_TIME_EVENT;
			// 	}
			// 	Fan_Stu.Fan_Ints_FlagArr[3] = FAN_INTS_ONE_LEVEL;
			// 	Fan_Stu.Fan_Dir_FlagArr[3] = FAN_BACKWARD;
			// 	Fan_Stu.Fan_Ints_FlagArr[4] = FAN_INTS_ONE_LEVEL;
			// 	Fan_Stu.Fan_Dir_FlagArr[4] = FAN_BACKWARD;
			// 	fan_para_set_event |= FAN_ALL_INTS_EVENT;
			// }break;
			// case KEY_FAN2_BACKWARD_TWO:
			// {
			// 	if(Fan_Stu.fan_mode_set == FAN_NONE_MODE)
			// 	{
			// 		Fan_Stu.fan_mode_set = FAN_CONSTANT_MODE;
			// 		fan_para_set_event |= FAN_MODE_TIME_EVENT;
			// 	}
			// 	Fan_Stu.Fan_Ints_FlagArr[3] = FAN_INTS_TWO_LEVEL;
			// 	Fan_Stu.Fan_Dir_FlagArr[3] = FAN_BACKWARD;
			// 	Fan_Stu.Fan_Ints_FlagArr[4] = FAN_INTS_TWO_LEVEL;
			// 	Fan_Stu.Fan_Dir_FlagArr[4] = FAN_BACKWARD;
			// 	fan_para_set_event |= FAN_ALL_INTS_EVENT;
			// }break;
			// case KEY_FAN2_BACKWARD_THREE:
			// {
			// 	if(Fan_Stu.fan_mode_set == FAN_NONE_MODE)
			// 	{
			// 		Fan_Stu.fan_mode_set = FAN_CONSTANT_MODE;
			// 		fan_para_set_event |= FAN_MODE_TIME_EVENT;
			// 	}
			// 	Fan_Stu.Fan_Ints_FlagArr[3] = FAN_INTS_THREE_LEVEL;
			// 	Fan_Stu.Fan_Dir_FlagArr[3] = FAN_BACKWARD;
			// 	Fan_Stu.Fan_Ints_FlagArr[4] = FAN_INTS_THREE_LEVEL;
			// 	Fan_Stu.Fan_Dir_FlagArr[4] = FAN_BACKWARD;
			// 	fan_para_set_event |= FAN_ALL_INTS_EVENT;
			// }break;
			case KEY_NO:
			default:
			{
			
			}break;
		}
	}
	//按摩器定时器关闭
	if(Fan_Stu.fan_min_count >= Fan_Stu.fan_time_min_set && Fan_Stu.fan_time_min_set != 0)
	{
		Fan_Stu.Fan_Ints_FlagArr[1] = FAN_INTS_ZERO_LEVEL;
		Fan_Stu.Fan_Dir_FlagArr[1] = FAN_FORWARD;
		Fan_Stu.Fan_Ints_FlagArr[2] = FAN_INTS_ZERO_LEVEL;
		Fan_Stu.Fan_Dir_FlagArr[2] = FAN_FORWARD;
		Fan_Stu.Fan_Ints_FlagArr[3] = FAN_INTS_ZERO_LEVEL;
		Fan_Stu.Fan_Dir_FlagArr[3] = FAN_FORWARD;
		Fan_Stu.Fan_Ints_FlagArr[4] = FAN_INTS_ZERO_LEVEL;
		Fan_Stu.Fan_Dir_FlagArr[4] = FAN_FORWARD;
		Fan_Stu.fan_mode_set = FAN_NONE_MODE;
		Fan_Clear_TimeCount();
		fan_para_set_event |= FAN_ALL_INTS_EVENT;
		fan_para_set_event |= FAN_MODE_TIME_EVENT;
	}
	if(fan_para_set_event != 0x00000000)
	{
		//模式时间
		if((fan_para_set_event & FAN_MODE_TIME_EVENT) == FAN_MODE_TIME_EVENT)
		{
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_FAN_MODE_EVENT);
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_FAN_TIME_EVENT);
			//LIN从机控制盒模式
			LIN_Master_Send_WriteFanModeTime_Cmd(0x00,Fan_Stu.fan_mode_set,Fan_Stu.fan_time_min_set);				
			//本机设备控制模式
			if(1 == TTL_Master_Send_WriteFanModeTime_Cmd(0X00,Fan_Stu.fan_mode_set,Fan_Stu.fan_time_min_set))
			{
				fan_para_set_event &= ~FAN_MODE_TIME_EVENT;
			}
		}
		if((fan_para_set_event & FAN_ALL_INTS_EVENT) == FAN_ALL_INTS_EVENT)
		{
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_FAN_INTS_EVENT);
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_FAN_DIR_EVENT);
			//LIN从机控制盒模式
			LIN_Master_Send_WriteFanInts_Cmd(Fan_Stu.Fan_Ints_FlagArr,Fan_Stu.Fan_Dir_FlagArr);				
			//本机设备控制模式
			if(1 == TTL_Master_Send_WriteFanInts_Cmd(Fan_Stu.Fan_Ints_FlagArr,Fan_Stu.Fan_Dir_FlagArr))
			{
				fan_para_set_event &= ~FAN_ALL_INTS_EVENT;
			}
		}
	}
}


void Fan_TimeManagerTask(void)
{
	if(Fan_Stu.Fan_Ints_FlagArr[1] != FAN_INTS_ZERO_LEVEL || Fan_Stu.Fan_Ints_FlagArr[2] != FAN_INTS_ZERO_LEVEL ||\
		 Fan_Stu.Fan_Ints_FlagArr[3] != FAN_INTS_ZERO_LEVEL || Fan_Stu.Fan_Ints_FlagArr[4] != FAN_INTS_ZERO_LEVEL)
	{
		Fan_Stu.fan_ms_count ++;
		if(Fan_Stu.fan_ms_count >= 200)
		{
			Fan_Stu.fan_ms_count = 0;
			Fan_Stu.fan_sec_count ++;
			if(Fan_Stu.fan_sec_count >= 60)
			{
				Fan_Stu.fan_sec_count = 0;
				Fan_Stu.fan_min_count ++;
				if(Fan_Stu.fan_min_count >= Fan_Stu.fan_time_min_set)
				{
					Fan_Stu.fan_min_count = Fan_Stu.fan_time_min_set;
				}
			}
		}
	}
	else
	{
		Fan_Clear_TimeCount();
	}
}
//0-ff,当参数为ff时，忽略此参数
//ints[]:0-3; 1,背部，2腿部， dir[]:0-3 1背部，2腿部   / 1：正转，2 反转
void User_SetFan_Mode(unsigned char *ints, unsigned char mode, unsigned char *dir, unsigned char set_time)
{
	Fan_Clear_TimeCount();
	if(mode == 0xff)
	{
		if((ints[1] != 0xff) || (ints[2] != 0xff) )
		{
			if(Fan_Stu.fan_mode_set == FAN_NONE_MODE)
			{
				Fan_Stu.fan_mode_set = FAN_CONSTANT_MODE;
			}				
			if(ints[1] != 0xff)
			{
				Fan_Stu.Fan_Ints_FlagArr[1] = ints[1];
				Fan_Stu.Fan_Ints_FlagArr[2] = ints[1];
			}
			if(ints[2] != 0xff)
			{
				Fan_Stu.Fan_Ints_FlagArr[3] = ints[2];
				Fan_Stu.Fan_Ints_FlagArr[4] = ints[2];
			}
		}
	}
	else
	{
		if(mode == 0x01)
		{
			Fan_Stu.fan_mode_set = FAN_CONSTANT_MODE;
		}
		else if(mode == 0x02)
		{
			Fan_Stu.fan_mode_set = FAN_WAVE_MODE;
		}
		else if(mode == 0x03)
		{
			Fan_Stu.fan_mode_set = FAN_PULSE_MODE;
		}
		
		if(ints[1] != 0xff)
		{
			Fan_Stu.Fan_Ints_FlagArr[1] = ints[1];
			Fan_Stu.Fan_Ints_FlagArr[2] = ints[1];
		}
		if(ints[2] != 0xff)
		{
			Fan_Stu.Fan_Ints_FlagArr[3] = ints[2];
			Fan_Stu.Fan_Ints_FlagArr[4] = ints[2];
		}
	}
	if(dir[1] != 0xff)
	{
		Fan_Stu.Fan_Dir_FlagArr[1] = dir[1]; 
		Fan_Stu.Fan_Dir_FlagArr[2] = dir[1]; 
	}
	
	if(dir[2] != 0xff)
	{
		Fan_Stu.Fan_Dir_FlagArr[3] = dir[2]; 
		Fan_Stu.Fan_Dir_FlagArr[4] = dir[2]; 
	}
	
	if(set_time != 0xff)
	{
		Fan_Stu.fan_time_min_set = set_time;
	}
	//同步外设发送模式、时间、强度
	fan_para_set_event |= FAN_MODE_TIME_EVENT;				
	fan_para_set_event |= FAN_ALL_INTS_EVENT;
	
}
