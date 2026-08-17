#include "app_vita.h"
#include "driver_beep.h"
#include "app_comm.h"
#include "app_motor.h"
#include "modul_motor.h"
#include "app_config.h"
#include "app_linbus.h"

#define EMM_LOOP_NUM  3
#define EMM_STOP_CHECK_TIME  60

#define HJ_SNORE_STOP_TIME   300
#define HJ_WAIT_STOP_TIME   60

VITA_STRUCT  Vita_EMM_Stu = {255,0,0,0};
VITA_STRUCT  Vita_Left_Stu	= {255,0,0,0};
VITA_STRUCT  Vita_Right_Stu	= {255,0,0,0};
static unsigned short snore_autorun_enable_min_time = 0;
static unsigned short snore_autorun_enable_ms_time = 0;
static unsigned char snore_autorun_enable = 0;  //

static unsigned char snore_loop_num = 0; //循环周期

static unsigned char snore_run_ms_time = 0;
static unsigned long snore_run_sec_time = 0;

static unsigned short snore_start_check_time = 0;
unsigned char vita_key_state = 0;

unsigned char Vita_Ble_RadioName[9]= {0};
	
unsigned char Get_Auto_SnoreState(void)
{
	if(0 == Vita_EMM_Stu.snore_step)
	{
		if(Vita_EMM_Stu.snore_value != 0)
		{
			Vita_EMM_Stu.snore_value = 0;
			
			//判断是否电机是否在动作
			if(0 == Get_Motor_PortState())
			{
				if(3 != snore_autorun_enable)
				{
					Vita_EMM_Stu.snore_step = 1;
					snore_autorun_enable = 1;
					if(Motor_DemoMode_RunState())
					{
						Motor_DemoMode_ClearPara();
						GetSet_Motor_Ctr_Cmd(0);
					}					
				}
			}
			else
			{
				return 0;
			}
		}
	}
	if(1 == snore_autorun_enable)
	{
		return 1;
	}
	
	return 0;	
}

void EMM_Snore_Control(void)
{

	if(snore_start_check_time < 3000)  return;
	Get_Auto_SnoreState();
	if(1 == snore_autorun_enable)
	{
		if(1 == Vita_EMM_Stu.snore_step)
		{
			Vita_EMM_Stu.snore_step = 2;
			vita_key_state = 1;
			Motor_OneClickCmd_Set(KEY_GO_SNORE);
			LIN_Master_Send_WriteKeyValue_Cmd(KEY_GO_SNORE);
		}
		else if(Vita_EMM_Stu.snore_step == 2) //等到位
		{
			if(0 == Get_Motor_PortState())
			{
				snore_run_sec_time = 0;
				Vita_EMM_Stu.snore_step = 3;
			}
		}
		else if(Vita_EMM_Stu.snore_step == 3) //等超时
		{
			//如果再次检测到清除时间，如果时间超过10分钟
			if(Vita_EMM_Stu.snore_value != 0)
			{
				Vita_EMM_Stu.snore_value = 0;
				snore_run_sec_time = 0;
			}
			if(snore_run_sec_time >= HJ_SNORE_STOP_TIME)
			{
				Vita_EMM_Stu.snore_step = 4;
				snore_run_sec_time = 0;
			}
		}
		else if(Vita_EMM_Stu.snore_step == 4) //复位
		{
			vita_key_state = 1;
			Motor_OneClickCmd_Set(KEY_FLAT);
			LIN_Master_Send_WriteKeyValue_Cmd(KEY_FLAT);
			Vita_EMM_Stu.snore_step = 5;
		}
		else if(Vita_EMM_Stu.snore_step == 5) //等复位完成
		{
			if(0 == Get_Motor_PortState())
			{
				Vita_EMM_Stu.snore_step = 6;
				snore_run_sec_time = 0;
				snore_loop_num ++;
				if(snore_loop_num >= EMM_LOOP_NUM)
				{
					snore_loop_num = 0;
					snore_autorun_enable = 3;
				}
			}		
		}
		else if(Vita_EMM_Stu.snore_step == 6) //等超时 30S
		{
			if(snore_run_sec_time >= HJ_WAIT_STOP_TIME)
			{
				Vita_EMM_Stu.snore_value = 0;
				Vita_EMM_Stu.snore_step = 0;
				snore_run_sec_time = 0;
			}		
		}
	}
	else
	{
		Vita_EMM_Stu.snore_step = 0;
		snore_loop_num = 0;
	}
}

void Snore_TimeManagerTask(void)
{
	//复位完成15S后才可以使用鼾声检测功能
	if(Vita_EMM_Stu.snore_step == 255)
	{
		if(1 == Get_Motor_AllReset())
		{
			Vita_EMM_Stu.snore_step = 254;
		}
		else
		{
			return;
		}
	}
	if(Vita_EMM_Stu.snore_step == 254)
	{
		snore_start_check_time ++;
		
		Vita_EMM_Stu.snore_value = 0;
		
		if(snore_start_check_time >= 3000)
		{
			Vita_EMM_Stu.snore_step = 0;
			snore_start_check_time = 3000;
		}
		else
		{
			return;
		}
	}	
	
	//任意按键退出止鼾模式
	if(Get_Key_Value() != 0)
	{
		Vita_EMM_Stu.snore_step = 0;
		snore_autorun_enable = 0;		
	}
	
	snore_run_ms_time ++;
	
	if(snore_run_ms_time >= 200)
	{
		snore_run_ms_time = 0;
		snore_run_sec_time ++;
		if(snore_run_sec_time >= HJ_SNORE_STOP_TIME)
		{
			snore_run_sec_time = HJ_SNORE_STOP_TIME;
		}
	}
	if(snore_autorun_enable == 3)
	{
		snore_autorun_enable_ms_time ++;
		if(snore_autorun_enable_ms_time >= 12000)
		{
			snore_autorun_enable_ms_time = 0;
			snore_autorun_enable_min_time ++;
			if(snore_autorun_enable_min_time >= 480)
			{
				snore_autorun_enable_min_time = 0;
				snore_autorun_enable = 0;
			}
		}
	}
	else
	{
		snore_autorun_enable_ms_time = 0;
		snore_autorun_enable_min_time = 0;
	}
}



