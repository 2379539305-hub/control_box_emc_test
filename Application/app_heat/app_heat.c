#include "app_heat.h"
#include "app_config.h"
#include "app_comm.h"

#include "driver_periph.h"

#include "modul_fan.h"

#include "app_linbus.h"

static unsigned char  heat_ctr_cmd = 0 , old_heat_ctr_cmd = 0;

HEAT_STRUCT  Heat_Stu = {0};

unsigned long heat_para_set_event = 0x00000000;

void APP_HeatInit(void)
{
	Heat_Stu.heat_time_min_set = system_config.flags.heat_default_time_min;
	
	Heat_Stu.heat_state = 0;
	Heat_Stu.old_heat_state = 1;
}
void Heat_Clear_TimeCount(HEAT_STRUCT  *Heat_Stu_Temp)
{
	Heat_Stu_Temp->heat_time_min_set = system_config.flags.heat_default_time_min;
	Heat_Stu_Temp->heat_ms_count = 0;
	Heat_Stu_Temp->heat_min_count = 0;
}

void Heat_Control(void)
{
	unsigned char key_value_temp = Get_Key_Value();
		/*--------------------------获取有用的键值信息---------------------------*/
	if((0 == key_value_temp) || (KEY_HEAT_SW == key_value_temp) || (KEY_HEAT_ON == key_value_temp)
		|| (KEY_HEAT_OFF == key_value_temp) || (KEY_HEAT_LOW == key_value_temp) || (KEY_HEAT_MID == key_value_temp))
	{
		
	}
	else  //无用信息
	{
		return;
	}
	old_heat_ctr_cmd = heat_ctr_cmd;
	
	heat_ctr_cmd = key_value_temp;
	if((heat_ctr_cmd != 0 && heat_ctr_cmd != old_heat_ctr_cmd))
	{
		if(heat_ctr_cmd == KEY_HEAT_SW)
		{
			if(Heat_Stu.heat_state != 0)
			{
				Heat_Stu.heat_state = 0;
				heat_para_set_event |= HEAT_SWITCH_OFF_EVENT;
			}
			else
			{
				Heat_Stu.heat_state = 2;
				heat_para_set_event |= HEAT_SWITCH_ON_EVENT;
			}
		}
		if(heat_ctr_cmd == KEY_HEAT_ON)
		{
			Heat_Stu.heat_state = 2;
			heat_para_set_event |= HEAT_SWITCH_ON_EVENT;
		}
		if(heat_ctr_cmd == KEY_HEAT_OFF)
		{
			Heat_Stu.heat_state = 0;
			heat_para_set_event |= HEAT_SWITCH_OFF_EVENT;
		}
		if(heat_ctr_cmd == KEY_HEAT_LOW)
		{
			Heat_Stu.heat_state = 1;
			heat_para_set_event |= HEAT_SWITCH_OFF_EVENT;
		}
		if(heat_ctr_cmd == KEY_HEAT_MID)
		{
			Heat_Stu.heat_state = 2;
			heat_para_set_event |= HEAT_SWITCH_OFF_EVENT;
		}		
		Heat_Clear_TimeCount(&Heat_Stu);
	}
	//按摩器定时器关闭
	if(Heat_Stu.heat_min_count >= Heat_Stu.heat_time_min_set && Heat_Stu.heat_time_min_set > 0)
	{
		Heat_Stu.heat_state = 0;
		Heat_Clear_TimeCount(&Heat_Stu);
		heat_para_set_event |= HEAT_SWITCH_OFF_EVENT;				
	}
	if(heat_para_set_event != 0x00000000)
	{
		if((heat_para_set_event & HEAT_SWITCH_ON_EVENT) == HEAT_SWITCH_ON_EVENT)
		{
			//LIN从机控制盒
			LIN_Master_Send_WriteHeatSwitch(Heat_Stu.heat_state);
			//本机设备
			if(1 == TTL_Master_Send_WtireHeatSwitch_Cmd(0X00,Heat_Stu.heat_state))
			{
				heat_para_set_event &= ~HEAT_SWITCH_ON_EVENT;
			}
		}
		if((heat_para_set_event & HEAT_SWITCH_OFF_EVENT) == HEAT_SWITCH_OFF_EVENT)
		{
			//LIN从机
			LIN_Master_Send_WriteHeatSwitch(Heat_Stu.heat_state);
			//本机设备
			if(1 == TTL_Master_Send_WtireHeatSwitch_Cmd(0X00,Heat_Stu.heat_state))
			{
				heat_para_set_event &= ~HEAT_SWITCH_OFF_EVENT;
			}
		}
	}
}

void Heat_TimeManagerTask(void)
{
	if(0 == Heat_Stu.heat_state)
	{
		Heat_Set_Pwm(HEAT_LEVEL_0_PWM);
	}
	else if(1 == Heat_Stu.heat_state)
	{
		Heat_Set_Pwm(HEAT_LEVEL_1_PWM);
	}
	else if(2 == Heat_Stu.heat_state)
	{
		Heat_Set_Pwm(HEAT_LEVEL_MAX_PWM);
	}	
	//单色灯时间控制
	if(Heat_Stu.heat_state!= 0)
	{
		if(Heat_Stu.heat_time_min_set > 0)  //设置时间大于0
		{
			Heat_Stu.heat_ms_count	++;
			if(Heat_Stu.heat_ms_count >= 60000/SYS_TIME_BASE) //1分钟
			{
				Heat_Stu.heat_ms_count = 0;
				
				Heat_Stu.heat_min_count ++;
				
				if(Heat_Stu.heat_min_count >= Heat_Stu.heat_time_min_set)
				{
					Heat_Stu.heat_min_count = Heat_Stu.heat_time_min_set;
				}
			}
		}
		else
		{
			Heat_Stu.heat_min_count = 0;
		}
	}
	else
	{
		Heat_Clear_TimeCount(&Heat_Stu);
	}	
}
