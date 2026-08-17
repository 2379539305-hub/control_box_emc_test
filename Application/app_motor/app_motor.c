#include "app_motor.h"
#include "delay.h"
#include "iwdg.h"

#include "driver_beep.h"
#include "driver_led.h"
#include "driver_periph.h"

#include "modul_a7105.h"
#include "modul_musicmsgr.h"
#include "modul_ttlbus.h"
#include "app_music.h"

#include "app_linbus.h"
#include "app_ttlbus.h"
#include "app_vita.h"
#include "app_ble.h"
#include "app_comm.h"
#include "app_save.h"
#include "app_config.h"
#include "app_light.h"
#include "app_backhual.h"
#include "app_rtc.h"
#include "app_msgr.h"

MOTOR_BASE_PARA *Motor_Back_ParaStu      = NULL;
MOTOR_BASE_PARA *Motor_Leg_ParaStu     	 = NULL; 
MOTOR_BASE_PARA *Motor_Lumbar_ParaStu    = NULL;
MOTOR_BASE_PARA *Motor_Neck_ParaStu      = NULL;
MOTOR_BASE_PARA *Motor_Lumbar2_ParaStu  = NULL;
MOTOR_BASE_PARA *Motor_Neck2_ParaStu      = NULL;
MOTOR_BASE_PARA *Motor_Tilt1_ParaStu   = NULL;
MOTOR_BASE_PARA *Motor_Tilt2_ParaStu   = NULL;

unsigned char motor_oneclick_cmd = 0; //电机在执行一键动作指令

unsigned char alarm_three_target_mode = 0; //闹钟三次动作目标位置键值
unsigned char alarm_three_phase = 0;     //闹钟三次动作当前阶段
unsigned char alarm_three_count = 0;     //闹钟三次动作已完成循环次数

unsigned char motor_ctr_cmd = 0 , old_motor_ctr_cmd = 0; //电机控制指令
//电机霍尔保存标志
static unsigned char motor_save_psition = 0; //

static unsigned char motor_cmd_change_fast = 0; //控制指令快速切换
static unsigned char motor_cmd_delay_time = 0; //控制指令快速切换计时
//lin发送延时
static unsigned char motor_lin_send_time = 0;
//控制指令射频接收信号空闲期内，多次指令屏蔽接收
static unsigned char motor_ctr_offline = 0; 
//电机参数设置事件
unsigned short motor_para_set_event = 0;
//电机固定位置运行完毕标志
unsigned char motor_run_fixed_complate = 0;
//再次按下
unsigned char motor_need_click_again = 0;
//演示模式
unsigned char motor_demo_loop_count = 0;
unsigned char motor_demo_step = 0;
unsigned char motor_demo_ms_time = 0;
unsigned short motor_demo_sec_time = 0;
unsigned short motor_demo_5ms_cont;
unsigned short motor_demo_1sec_cont;
unsigned char motor_demo_slave_flag = 0;
unsigned char lin_received_step = 0;
#define MOTOR_CTR_MODE_SNORE     0X01
#define MOTOR_CTR_MODE_TARGET    0X02
static unsigned char motor_ctr_mode = 0;  //电机控制模式

unsigned char motor_run_cmd_state = 0; //电机运行状态

static unsigned char motor_snore_ms_time = 0; //snore 复位
static unsigned char motor_snore_sec_time = 0;
#define MOTOR_NUM  (6)
unsigned short Motor_SyncTarget_HallArr[7] = {0};  //每个电机目标位置  跑估计位置时候将save的值合并到此数组
static unsigned char Motor_SyncCurrent_HallArr[MOTOR_NUM*3];

static unsigned char motor_sync_enable = 0; //控制是否要发追同步信号

unsigned char motor_ctr_offline_LIN = 0;
//保存位置使能标志
unsigned char Save_Shape_Enable(void);
//运行固定位置
unsigned char Command_GoFixed_Shape(SAVE_MOTOR_INFO_TYPE save_info_type);
//运行记忆位置
unsigned char Command_GoMemory_Shape(SAVE_MOTOR_INFO_TYPE save_info_type);
//lin发送同步
void Motor_Lin_SendSync(void);
//lin发送位置
void Motor_Lin_SendPosition(void);

//演示模式1/2/3
unsigned char Motor_Demo_Help_Sleep_Mode(void);
unsigned char Motor_Demo_Help_Sleep_SSB_Mode(void);
unsigned char Motor_Demo_Help_Sleep_ZhuiMi_Mode(void);
unsigned char Motor_Alarm_Mode(void);
unsigned char Motor_AlarmThree_Mode(void);
unsigned char Motor_Demo2_Mode(void);
unsigned char Motor_Demo_Tilt_Mode(void);
unsigned char Motor_Demo_Help_Sleep_Tilt_SSB_Mode(void);
unsigned char Motor_Sys_Mode(void);
void Motor_Step_ClearPara(void);

MOTOR_BASE_PARA *Get_Motor_ParaStu(uint8_t motor_port)
{
	if(Motor1_ParaStu.motor_port == motor_port)
	{
		return &Motor1_ParaStu;
	}
	if(Motor2_ParaStu.motor_port == motor_port)
	{
		return &Motor2_ParaStu;
	}
	if(Motor3_ParaStu.motor_port == motor_port)
	{
		return &Motor3_ParaStu;
	}
	if(Motor4_ParaStu.motor_port == motor_port)
	{
		return &Motor4_ParaStu;
	}
	if(Motor5_ParaStu.motor_port == motor_port)
	{
		return &Motor5_ParaStu;
	}
	if(Motor6_ParaStu.motor_port == motor_port)
	{
		return &Motor6_ParaStu;
	}

	return NULL;
}

static MOTOR_BASE_PARA *Get_Motor_ParaStu_WithDefault(uint8_t motor_port, MOTOR_BASE_PARA *default_para)
{
	MOTOR_BASE_PARA *para = Get_Motor_ParaStu(motor_port);
	return (para != NULL) ? para : default_para;
}

static MOTOR_BASE_PARA *Get_Motor_ParaStu_WithConfig(uint8_t motor_port, uint8_t motor_type)
{
	if((motor_port == MOTOR_NO) || (motor_type == NO_INPUT_TYPE))
	{
		return &Motor_Invalid_ParaStu;
	}

	return Get_Motor_ParaStu_WithDefault(motor_port, &Motor_Invalid_ParaStu);
}

void Motor_ParaStu_Init(void)
{
	// 背部
	Motor_Back_ParaStu = Get_Motor_ParaStu_WithConfig(system_config.flags.motor_back_port, system_config.flags.motor_back_type);
	if((Motor_Back_ParaStu != NULL) && (Motor_Back_ParaStu != &Motor_Invalid_ParaStu))
	{
		Motor_Back_ParaStu->motor_type = &system_config.flags.motor_back_type;
	}

	// 腿
	Motor_Leg_ParaStu = Get_Motor_ParaStu_WithConfig(system_config.flags.motor_leg_port, system_config.flags.motor_leg_type);
	if((Motor_Leg_ParaStu != NULL) && (Motor_Leg_ParaStu != &Motor_Invalid_ParaStu))
	{
		Motor_Leg_ParaStu->motor_type = &system_config.flags.motor_leg_type;
	}

	// 腰一
	Motor_Lumbar_ParaStu = Get_Motor_ParaStu_WithConfig(system_config.flags.motor_lumbar_port, system_config.flags.motor_lumbar_type);
	if((Motor_Lumbar_ParaStu != NULL) && (Motor_Lumbar_ParaStu != &Motor_Invalid_ParaStu))
	{
		Motor_Lumbar_ParaStu->motor_type = &system_config.flags.motor_lumbar_type;
	}

		// 颈1
	Motor_Neck_ParaStu = Get_Motor_ParaStu_WithConfig(system_config.flags.motor_neck_port, system_config.flags.motor_neck_type);
	if((Motor_Neck_ParaStu != NULL) && (Motor_Neck_ParaStu != &Motor_Invalid_ParaStu))
	{
		Motor_Neck_ParaStu->motor_type = &system_config.flags.motor_neck_type;
	}

	// 腰二
	Motor_Lumbar2_ParaStu = Get_Motor_ParaStu_WithConfig(system_config.flags.motor_lumbar2_port, system_config.flags.motor_lumbar2_type);
	if((Motor_Lumbar2_ParaStu != NULL) && (Motor_Lumbar2_ParaStu != &Motor_Invalid_ParaStu))
	{
		Motor_Lumbar2_ParaStu->motor_type = &system_config.flags.motor_lumbar2_type;
	}

	// 颈2
	Motor_Neck2_ParaStu = Get_Motor_ParaStu_WithConfig(system_config.flags.motor_neck2_port, system_config.flags.motor_neck2_type);
	if((Motor_Neck2_ParaStu != NULL) && (Motor_Neck2_ParaStu != &Motor_Invalid_ParaStu))
	{
		Motor_Neck2_ParaStu->motor_type = &system_config.flags.motor_neck2_type;
	}
	
	//升降1
	Motor_Tilt1_ParaStu = Get_Motor_ParaStu_WithConfig(system_config.flags.motor_tilt1_port, system_config.flags.motor_tilt1_type);
	if((Motor_Tilt1_ParaStu != NULL) && (Motor_Tilt1_ParaStu != &Motor_Invalid_ParaStu))
	{
		Motor_Tilt1_ParaStu->motor_type = &system_config.flags.motor_tilt1_type;
	}
	//升降2
	Motor_Tilt2_ParaStu = Get_Motor_ParaStu_WithConfig(system_config.flags.motor_tilt2_port, system_config.flags.motor_tilt2_type);
	if((Motor_Tilt2_ParaStu != NULL) && (Motor_Tilt2_ParaStu != &Motor_Invalid_ParaStu))
	{
		Motor_Tilt2_ParaStu->motor_type = &system_config.flags.motor_tilt2_type;
	}
}
//获取电机动作信息
unsigned char Motor_Run_KeyInfo(unsigned char key_temp) 
{
	if(((key_temp >= KEY_MOTOR_RUN_START) && (key_temp <= KEY_MOTOR_RUN_END)) ||
	((key_temp >= KEY_ONE_CLICK_RUN_START) && (key_temp <= KEY_ONE_CLICK_RUN_END)))
	{
		return 1;
	}
	return 0;
}
//获取电机控制信息
unsigned char Motor_GoFixed_KeyInfo(unsigned char key_temp)  
{
	if(((key_temp >= KEY_MOTOR_ONE_CLICK_RUN_START) && (key_temp <= KEY_MOTOR_ONE_CLICK_RUN_END))||
	((key_temp >= KEY_ONE_CLICK_RUN_START) && (key_temp <= KEY_ONE_CLICK_RUN_END)))
	{
		return 1;
	}
	
	return 0;
}
//获取电机相关一键动作
unsigned char Motor_OneClickCmd_KeyInfo(unsigned char key_temp)  
{
	if(((key_temp >= KEY_MOTOR_ONE_CLICK_START) && (key_temp <= KEY_MOTOR_ONE_CLICK_END))||
	((key_temp >= KEY_ONE_CLICK_RUN_START) && (key_temp <= KEY_ONE_CLICK_RUN_END)))
	{
		return 1;
	}
	
	return 0;
}
//获取电机指令
unsigned char Motor_AcceptCmd_KeyInfo(unsigned char key_temp)
{
	if(((key_temp >= KEY_MOTOR_START) && (key_temp <= KEY_MOTOR_END))||
	((key_temp >= KEY_ONE_CLICK_RUN_START) && (key_temp <= KEY_ONE_CLICK_RUN_END)))
	{
		return 1;
	}
	return 0;
}
unsigned char Motor_Continue_KeyInfo(unsigned char key_temp)
{
	if(key_temp >= KEY_MOTOR_COUNTINUE_RUN_START && key_temp <= KEY_MOTOR_COUNTINUE_RUN_END)
	{
		return 1;
	}
	return 0;
}
//判断当前是否运行在演示/闹钟模式
//返回值: 1=演示/闹钟模式(需放平), 2=闹钟按摩器模式(不需放平), 0=非演示模式,3=电机随动模式，只有电机相关可以打断放平
unsigned char Motor_DemoMode_RunState(void)
{
	if((KEY_DEMO1_MODE == GetSet_Motor_Ctr_Cmd(0xff)) || (KEY_DEMO2_MODE == GetSet_Motor_Ctr_Cmd(0xff)) || (KEY_DEMO3_MODE == GetSet_Motor_Ctr_Cmd(0xff)) || (KEY_ALARM_MODE == GetSet_Motor_Ctr_Cmd(0xff)) ||
	   (KEY_ALARM_THREE_MODE == GetSet_Motor_Ctr_Cmd(0xff)))
		return 1;
	if(KEY_ALARM_MSGR_MODE == GetSet_Motor_Ctr_Cmd(0xff))
		return 2;
	if(KEY_MOTOR_FOLLOW_UP == GetSet_Motor_Ctr_Cmd(0xff))
		return 3;
	return 0;
}
//判断键值是否为演示/闹钟指令
//返回值: 1=演示/闹钟指令(需放平), 2=闹钟按摩器指令(不需放平), 0=非演示指令，0=非演示模式,3=电机随动模式，只有电机相关可以打断放平
unsigned char Motor_DemoMode_KeyInfo(unsigned char key_temp)
{
	if((key_temp == KEY_DEMO1_MODE) || (key_temp == KEY_DEMO2_MODE) || (key_temp == KEY_DEMO3_MODE) || (key_temp == KEY_ALARM_MODE) ||
	   (key_temp == KEY_ALARM_THREE_MODE))
		return 1;
	if(key_temp == KEY_ALARM_MSGR_MODE)
		return 2;
	if(key_temp == KEY_MOTOR_FOLLOW_UP)
		return 3;
	return 0;
}
//电机参数复位
void Motor_Para_AllReset(void)
{
	Motor_Para_Reset(&Motor1_ParaStu);
	Motor_Para_Reset(&Motor2_ParaStu);
	Motor_Para_Reset(&Motor3_ParaStu);
	Motor_Para_Reset(&Motor4_ParaStu);
	Motor_Para_Reset(&Motor5_ParaStu);
	Motor_Para_Reset(&Motor6_ParaStu);
}
unsigned char Get_Sync_Motor_Side(void)
{
	unsigned char side = 0;
	if(0 == Get_Sync_Run_Mode() || 1 == alarm_key_state || 1 == vita_key_state)
	{
		side = 0;
		alarm_key_state = 0;
		vita_key_state = 0;
	}
	else
	{
		if(0 == Get_Sync_Key_State()) //主遥控器
		{
			side = 1;
		}
		else
		{
			side = 2;
		}
	}
	return side;
}
void Set_Motor_Demo_Step(unsigned char step)
{
	lin_received_step = step;
	motor_demo_ms_time = 0;
	motor_demo_sec_time = 0;
}
//电机控制主循环
void Motor_Control(void)
{
	
	static unsigned char key_value_temp = 0, old_key_value_temp = 0;
	//保存历史键值
	old_key_value_temp = key_value_temp;
	//接收新的键值
	key_value_temp = Get_Key_Value();
	EMM_Snore_Control();
	/*--------------------------获取有用的键值信息---------------------------*/
	if((0 == key_value_temp) || (1 == Motor_AcceptCmd_KeyInfo(key_value_temp)))
	{
		
	}
	else  //无用信息
	{
		return;
	}
	/*----------------------------------控制指令逻辑处理--------------------------------------*/
	//如果没有其他来源的事件，就判断接收到的键值
	if(motor_para_set_event == 0x0000)  //常规控制
	{
		//一键指令
		if(1 == Motor_OneClickCmd_KeyInfo(key_value_temp))
		{
			TTL_GetClear_KeyValue(0); //此处是防止重复进入
			//首次接收
			if(0 == motor_ctr_offline)
			{
				motor_ctr_offline = 1;
				
				motor_sync_enable = 0;   //运行固定位置过程中打断  要追同步
				//如果是动作指令，不是记忆指令
				if(1 == Motor_GoFixed_KeyInfo(key_value_temp))
				{
					//如果当前没有动作
					if(KEY_NO == motor_oneclick_cmd)
					{
						//追同步未完成  如果接收到一键控制信号 打断运行 并发送同步位置
						if(0 == Motor_Sync_Complate()) 
						{
							//使能追同步
							motor_sync_enable = 1;
							//停止动作
							key_value_temp = KEY_NO;
							memset(Motor_SyncTarget_HallArr,0x00,2 * (sizeof(Motor_SyncTarget_HallArr)/sizeof(Motor_SyncTarget_HallArr[0])));							
						}
						else
						{
							//如果当前没动作而且追同步结束了，开始动作新的键值
							motor_oneclick_cmd = key_value_temp;
						}
					}
					else
					{
						if((Motor_DemoMode_RunState() == 1) || (Motor_DemoMode_RunState() == 3)) 
						{
							Motor_DemoMode_ClearPara();
							GetSet_Motor_Ctr_Cmd(KEY_FLAT);
							key_value_temp = KEY_NO;
						}
						else if(Motor_DemoMode_RunState() == 2)
						{
							Motor_DemoMode_ClearPara();
							GetSet_Motor_Ctr_Cmd(KEY_NO);
							key_value_temp = KEY_NO;
						}
						else
						{	
							 //运行固定位置过程中打断  要追同步
							motor_sync_enable = 1;  
							//打断当前动作
							motor_oneclick_cmd = KEY_NO;
	//						motor_ctr_cmd = KEY_NO;
							key_value_temp = KEY_NO;
						}	
					}
				}
				else   //接收到记忆某个位置的指令也要打断马达运行  而且不能去执行记忆的功能
				{
					if((Motor_DemoMode_RunState() == 1) || (Motor_DemoMode_RunState() == 3)) 
					{
						Motor_DemoMode_ClearPara();
						GetSet_Motor_Ctr_Cmd(KEY_FLAT);
						key_value_temp = KEY_NO;
					}
					else if(Motor_DemoMode_RunState() == 2)
					{
						Motor_DemoMode_ClearPara();
						GetSet_Motor_Ctr_Cmd(KEY_NO);
						key_value_temp = KEY_NO;
					}
					else if(motor_oneclick_cmd != KEY_NO)
					{
						motor_oneclick_cmd = KEY_NO;
						key_value_temp = KEY_NO;							
					}
				}
				//通过lin发送接收到的键值
				LIN_Master_Send_WriteKeyValue_Cmd(key_value_temp);				
			}
			else
			{
				//在一定时间内再次接收一键指令不执行动作
				key_value_temp = KEY_NO;
			}
		}
		else
		{
			//如果是持续指令
			if(key_value_temp != KEY_NO)               
			{
				motor_sync_enable = 1; //接收到非LIN的信号  就可以再次发LIN信号
				
				motor_ctr_offline = 0; //非一键指令 就清除
				motor_oneclick_cmd = KEY_NO;
				if(motor_lin_send_time >= 10)
				{
					motor_lin_send_time = 0;
					LIN_Master_Send_WriteKeyValue_Cmd(key_value_temp);
				}
			}
			else
			{
				//如果没有指令了
				
				if(1 == Motor_OneClickCmd_KeyInfo(old_key_value_temp))  //主机的先到达固定位置,从机没到达,保证不会打断从机
				{
					//之前跑的是固定位置，停止后不通过lin发送停止
				}
				else
				{
					//之前运行的不是一键动作，停止后通过lin发送停止，并发送追同步
					if(1 == Motor_AcceptCmd_KeyInfo(old_key_value_temp)) //发停止
					{
						LIN_Master_Send_WriteKeyValue_Cmd(key_value_temp);
					}
				}
			}
		}
		//等待信号释放，信号释放后，才能继续接收下一个一键指令
		if(1 == A7105_Comm_Free() && 1 == Ble_Comm_Free() && 1 == LIN_Check_Busy_Free()) 
		{
			motor_ctr_offline = 0;
			motor_ctr_offline_LIN = 0;
		}
	}
	else 
	{
		if((motor_para_set_event & MOTOR_LIN_RUN_EVENT) == MOTOR_LIN_RUN_EVENT)  //在LIN控制状态
		{
			//如果收到了lin控制事件
			key_value_temp = Lin_Analy_KeyValue();
			//失能追同步
			motor_sync_enable = 0;
			//如果收到了0或者其他电机动作
			if((0 == key_value_temp) || (1 == Motor_AcceptCmd_KeyInfo(key_value_temp))) //保证非马达指令不打断从机马达动作
			{
				//如果是一键动作就执行，如果不是就清零大师保留键值
				if(1 == Motor_OneClickCmd_KeyInfo(key_value_temp))
				{
					if(0 == motor_ctr_offline_LIN)
					{
						motor_ctr_offline_LIN = 1;
						if((Motor_DemoMode_RunState() == 1) || (Motor_DemoMode_KeyInfo(motor_oneclick_cmd) == 1) 
						|| (Motor_DemoMode_RunState() == 3) || (Motor_DemoMode_KeyInfo(motor_oneclick_cmd) == 3)) 
						{
							Motor_DemoMode_ClearPara();
							key_value_temp = KEY_FLAT;	
						}
						else if(Motor_DemoMode_RunState() == 2 || Motor_DemoMode_KeyInfo(motor_oneclick_cmd) == 2)
						{
							Motor_DemoMode_ClearPara();
							key_value_temp = KEY_NO;
						}
						motor_oneclick_cmd = key_value_temp;
					
						motor_demo_slave_flag = 1;
					}
				}
				else
				{
					motor_ctr_offline_LIN = 0;
					//非一键动作就清零
					motor_oneclick_cmd = KEY_NO;
				}
			}
		}
		if((motor_para_set_event & MOTOR_TARGET_POS_EVENT) == MOTOR_TARGET_POS_EVENT)  //设置推杆目标位置指令
		{
			//电机位置直接设置事件==手动发起追同步动作
			motor_para_set_event &= ~MOTOR_TARGET_POS_EVENT;
		
	//		motor_run_fixed_complate = 0; //如果马达之前是到达某个位置了,此处不清除那么不会上报状态清零
		
			motor_oneclick_cmd = 0;
		
			motor_cmd_change_fast = 1;
		
			//给LIN发送数据
			if(1 == LIN_Check_Busy_Free())
			{
				Motor_Lin_SendPosition();
			}
		}
		if((motor_para_set_event & MOTOR_MEM_POS_EVENT) == MOTOR_MEM_POS_EVENT)  //记忆推杆固定位置
		{
			memset(Motor_SyncTarget_HallArr,0x00,2 * (sizeof(Motor_SyncTarget_HallArr)/sizeof(Motor_SyncTarget_HallArr[0])));
			motor_para_set_event &= ~MOTOR_MEM_POS_EVENT;
		}
		if((motor_para_set_event & MOTOR_CMD_RUN_EVENT) == MOTOR_CMD_RUN_EVENT)	//一键动作事件
		{
			motor_para_set_event &= ~MOTOR_CMD_RUN_EVENT;		
			motor_cmd_change_fast = 1;
			//给LIN发送数据
			LIN_Master_Send_WriteKeyValue_Cmd(motor_oneclick_cmd);	
		}
		if((motor_para_set_event & MOTOR_DEMO_SLEEP_RUN_EVENT) == MOTOR_DEMO_SLEEP_RUN_EVENT) //哄睡状态同步
		{
			Light_Time_Set(&Light_RgbColour_Stu,demo_run_time*60);		
			
			light_para_set_event |= LIGHT_UBL_TIME_EVENT;					
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_DEMO_SLEEP_TIME_EVENT);

			LIN_Master_Send_DemoRunTime_Cmd(demo_run_time);
			motor_para_set_event &= ~MOTOR_DEMO_SLEEP_RUN_EVENT;			
		}			
		if((motor_para_set_event & MOTOR_ALARM_RUN_EVENT) == MOTOR_ALARM_RUN_EVENT)//闹钟同步
		{
			Motor_Step_ClearPara();
			Motor_OneClickCmd_Set(KEY_ALARM_MODE);
			motor_cmd_change_fast = 1;
			LIN_Master_Send_AlarmRun_Cmd(alarm_mode_value[0], alarm_mode_value[1], alarm_mode_value[2], alarm_mode_value[3]);
			motor_para_set_event &= ~MOTOR_ALARM_RUN_EVENT;
		}
		if((motor_para_set_event & MOTOR_ALARM_THREE_RUN_EVENT) == MOTOR_ALARM_THREE_RUN_EVENT)//闹钟三次动作
		{
			Motor_OneClickCmd_Set(KEY_ALARM_THREE_MODE);
			motor_cmd_change_fast = 1;
			motor_para_set_event &= ~MOTOR_ALARM_THREE_RUN_EVENT;
		}
		if((motor_para_set_event & MOTOR_ALARM_MSGR_RUN_EVENT) == MOTOR_ALARM_MSGR_RUN_EVENT)//闹钟按摩器
		{
			Motor_OneClickCmd_Set(KEY_ALARM_MSGR_MODE);
			motor_cmd_change_fast = 1;
			motor_para_set_event &= ~MOTOR_ALARM_MSGR_RUN_EVENT;
		}	
		if((motor_para_set_event & KEY_MOTOR_FOLLOW_MODE_SWITCH_EVENT) == KEY_MOTOR_FOLLOW_MODE_SWITCH_EVENT) //主从模式切换事件
		{
			Motor_Step_ClearPara();
			GetSet_Motor_Ctr_Cmd(KEY_MOTOR_FOLLOW_UP);
			motor_para_set_event &= ~KEY_MOTOR_FOLLOW_MODE_SWITCH_EVENT;
		}		
	}
	/*---------------------------------推杆指令赋值---------------------------------*/
	old_motor_ctr_cmd = motor_ctr_cmd;
	if(motor_oneclick_cmd != KEY_NO)
	{
		motor_ctr_cmd = motor_oneclick_cmd;
	}
	else
	{
		motor_ctr_cmd = key_value_temp;	
	}
		//给TTL从机控制盒发送停止  使用这个控制盒就没有缓停了
	if(motor_ctr_cmd == KEY_NO && motor_ctr_cmd != old_motor_ctr_cmd)
	{
		motor_cmd_change_fast = 1;
	}
	if((motor_oneclick_cmd != 0) || (motor_ctr_cmd != 0))
	{
		if((!Motor_DemoMode_KeyInfo(motor_oneclick_cmd) && (motor_oneclick_cmd != KEY_MOTOR_FOLLOW_UP)))//不使用motor_ctr_cmd，因为持续动作命令motor_ctr_cmd不为零
		{
			if(Motor_DemoMode_KeyInfo(old_motor_ctr_cmd) || old_motor_ctr_cmd == KEY_MOTOR_FOLLOW_UP)  //如果之前是演示模式或者主从模式
			{
				if(KEY_ALARM_THREE_MODE == old_motor_ctr_cmd)
				{
					Motor_Step_ClearPara();
					GetSet_Motor_Ctr_Cmd(KEY_FLAT);
				}
				else
				{
					Motor_DemoMode_ClearPara();
					if((Motor_DemoMode_KeyInfo(old_motor_ctr_cmd) == 1) || (old_motor_ctr_cmd == KEY_MOTOR_FOLLOW_UP))
					{
						GetSet_Motor_Ctr_Cmd(KEY_FLAT);
					}
					else
					{
						GetSet_Motor_Ctr_Cmd(KEY_NO);
					}
				}
			}
		}
	}
	//给电机指令运行状态赋值
	if((motor_ctr_cmd != KEY_NO && motor_ctr_cmd != old_motor_ctr_cmd))  //指令切换  	
	{
		motor_run_fixed_complate = 0;
		ble_report_set_event(BLE_REPORT_EVENT, REPORT_MOTOR_CMD_EVENT);
		Motor_Step_ClearPara();
	}
	if(0 == motor_ctr_cmd || 1 == Motor_Run_KeyInfo(motor_ctr_cmd))
	{
		if(motor_ctr_cmd != 0)
		{
			if(motor_ctr_cmd != KEY_MOTOR_STOP)
			{
				if(1 == motor_run_fixed_complate)
				{
					motor_run_cmd_state = motor_ctr_cmd;
				}
				else
				{
					motor_run_cmd_state = 0;
				}
			}
			else
			{
				motor_run_cmd_state = 0;
			}
		}
		else
		{
			if(0 == motor_run_fixed_complate) //如果马达跑完固定位置就不清楚状态
			{
				motor_run_cmd_state = 0;
				if((old_motor_ctr_cmd != 0) && (motor_ctr_cmd == KEY_NO))
				{//动作结束或打断进
					ble_report_set_event(BLE_REPORT_EVENT, REPORT_MOTOR_CMD_EVENT);		
					alarm_key_state = 0;
					vita_key_state = 0;						
				}
			}
			else
			{
				if(motor_run_cmd_state == 0)
				{
					ble_report_set_event(BLE_REPORT_EVENT, REPORT_MOTOR_CMD_EVENT);	
					alarm_key_state = 0;
					vita_key_state = 0;						
				}						
				if(old_motor_ctr_cmd != 0)
				{
					motor_run_cmd_state = old_motor_ctr_cmd;
				}
			}
		}
	}
	
	//如果本机自身有键值,则清除同步位置,防止停止时跑同步位置
	if(motor_ctr_cmd != KEY_NO)
	{
		memset(Motor_SyncTarget_HallArr,0x00,2 * (sizeof(Motor_SyncTarget_HallArr)/sizeof(Motor_SyncTarget_HallArr[0])));
	}
#if 1
	if((motor_ctr_cmd != KEY_NO && motor_ctr_cmd != old_motor_ctr_cmd))  //指令切换  
	{
		//马达停止  && old_motor_ctr_cmd != KEY_NO
		if(KEY_NO == old_motor_ctr_cmd)
		{
			if(Get_Motor_PortState() != 0)  //保证上个指令动作完全完成。
			{
				motor_cmd_change_fast = 1;
			}
		}
		else  
		{
			motor_cmd_change_fast = 1;
		}	
	}
	if(1 == motor_cmd_change_fast)
	{
		if(0 == Get_Motor_PortState()) //所有电机继电器释放
		{
			motor_cmd_change_fast = 2;
		}
	}
#endif		
	/*-----------------------马达动作就要记忆霍尔----------------------------*/
	if(motor_ctr_cmd != KEY_NO || Get_Motor_PortState() != 0)
	{
		if((motor_save_psition & 0x01) == 0)
		{
			motor_save_psition |= 0x01;	
		}
	}	
	//电机停止后保存当前霍尔
  if((KEY_NO == motor_ctr_cmd) && (0 == Get_Motor_PortState())  && (1 == Ble_Comm_Free()) && (1 == LIN_Check_Busy_Free()) && (1 == A7105_Comm_Free()))
  {
		motor_demo_slave_flag = 0;		
    if((motor_save_psition & 0x01) == 0x01)
    {
      motor_save_psition &= 0xFE; 	
			Command_Save_MotorHall();//保存霍尔
            Delay_Ms(10);
			Motor_Para_AllReset();
			//如果使能了追同步，就发送追同步
			if(1 == motor_sync_enable && 1 == LIN_Check_Busy_Free())
			{
				motor_sync_enable = 0;
				Motor_Lin_SendSync();
			}	
    }
		else
		{
		}	
    /*-------------------------------------------------------------------------------------------*/
  }
	/*---------------音乐阵子降功率  电机是否在运行作判断----------------------*/
	if(0 == Get_Motor_PortState()) //电机静止 
	{
		if(MusicalOsc_Stu.MsgrPower_State != SWITCH_STATE_OFF) 
		{
			if(1 == TTL_Master_Send_WriteMsgrPower_Cmd(SWITCH_CTR_OFF))
			{
				MusicalOsc_Stu.MsgrPower_State = SWITCH_STATE_OFF;
			}
		}
	}
	else
	{
		if(MusicalOsc_Stu.MsgrPower_State == SWITCH_STATE_OFF) 
		{
			if(1 == TTL_Master_Send_WriteMsgrPower_Cmd(SWITCH_CTR_ON))
			{
				MusicalOsc_Stu.MsgrPower_State = SWITCH_STATE_ON;
			}
		}
	}
	//记忆功能  保证一次指令只保存一次
	switch(motor_ctr_cmd)
	{
		case KEY_MEM_M1:
		{
			if(1 == Save_Shape_Enable())
			{
				while(!Command_SaveM1_Shape());
				Beep_SingSetPara(200,3);
			}
		}break;
		case KEY_MEM_M2:
		{
			if(1 == Save_Shape_Enable())
			{
				while(!Command_SaveM2_Shape());
				Beep_SingSetPara(200,3);	
			}
		}break;
    case KEY_MEM_M3:
		{
			if(1 == Save_Shape_Enable())
			{
				while(!Command_SaveM3_Shape());
				Beep_SingSetPara(200,3);	
			}
		}break;	
		case KEY_MEM_TV:
		{
			if(1 == Save_Shape_Enable())
			{
				while(!Command_SaveTV_Shape());
				Beep_SingSetPara(200,3);	
			}
		}break;
		case KEY_MEM_ZEROG:
		{
			if(1 == Save_Shape_Enable())
			{
				while(!Command_SaveZeroG_Shape());
				Beep_SingSetPara(200,3);	
			}
		}break;
		case KEY_MEM_LOUNGE:
		{
			if(1 == Save_Shape_Enable())
			{
				while(!Command_SaveLounge_Shape());
				Beep_SingSetPara(200,3);
			}
		}break;
		case KEY_MEM_SNORE:
		{
			if(1 == Save_Shape_Enable())
			{
				while(!Command_SaveSnore_Shape()); 
				Beep_SingSetPara(200,3);	
			}
		}break;
		case KEY_MEM_READ:
		{
			if(1 == Save_Shape_Enable())
			{
				while(!Command_SaveRead_Shape()); 
				Beep_SingSetPara(200,3);	
			}			
		}break;
		case KEY_MEM_YOGA:
		{
			if(1 == Save_Shape_Enable())
			{
				while(!Command_SaveYoga_Shape()); 
				Beep_SingSetPara(200,3);	
			}	
		}break;
		case KEY_MEM_GETUP:
		{
			if(1 == Save_Shape_Enable())
			{
				while(!Command_SaveGetUp_Shape()); 
				Beep_SingSetPara(200,3);	
			}	
		}break;
		case KEY_MEM_NURSING:
		{
			if(1 == Save_Shape_Enable())
			{
				while(!Command_SaveNursing_Shape()); 
				Beep_SingSetPara(200,3);	
			}	
		}break;
		case KEY_DATA_RESET:
		{
			if(1 == Save_Shape_Enable())
			{
				while(!Command_FactoryReset_Shape());  //加超时退出
				Beep_SingSetPara(200,3);	
			}	
		}break;
		default:break;
	}
}
//电机停止，清除电机运行标志位
void Motor_AllStop(void)
{
	motor_ctr_cmd = KEY_NO;
	motor_oneclick_cmd = KEY_NO;
}
//背部运行函数
void Command_Back_Run(unsigned char key_func)
{
	if(key_func == KEY_BACK_UP)
	{
			Motor_ArrivePosition(Motor_Back_ParaStu,HALL_UP_NUM);
	}
	if(key_func == KEY_BACK_DOWN)
	{
			Motor_ArrivePosition(Motor_Back_ParaStu,HALL_MIN_NUM);
	}
}
//腿部运行函数
void Command_Leg_Run(unsigned char key_func)
{
	if(key_func == KEY_LEG_UP)
	{
		Motor_ArrivePosition(Motor_Leg_ParaStu,HALL_UP_NUM);
	}
	if(key_func == KEY_LEG_DOWN)
	{
    Motor_ArrivePosition(Motor_Leg_ParaStu,HALL_MIN_NUM);
	}
}
//背腿同时运行函数
void Command_BackLeg_Run(unsigned char key_func)
{
	if(key_func == KEY_BACKLEG_UP)
	{
		Motor_ArrivePosition(Motor_Back_ParaStu,HALL_UP_NUM);
		Motor_ArrivePosition(Motor_Leg_ParaStu,HALL_UP_NUM);
	}
	if(key_func == KEY_BACKLEG_DOWN)
	{
		Motor_ArrivePosition(Motor_Back_ParaStu,HALL_MIN_NUM);	
		Motor_ArrivePosition(Motor_Leg_ParaStu,HALL_MIN_NUM);
	}
}
//外托同时运行函数
void Command_Tilt_All_Run(unsigned char key_func)
{
	if(key_func == KEY_TILT_ALL_UP)
	{
		if(Motor_Tilt1_ParaStu == &Motor_Invalid_ParaStu && Motor_Tilt2_ParaStu == &Motor_Invalid_ParaStu)
		{
			Motor_ArrivePosition(Motor_Lumbar_ParaStu,HALL_UP_NUM);
			Motor_ArrivePosition(Motor_Neck_ParaStu,HALL_UP_NUM);
		}
		else
		{
			if(motor_need_click_again == 0)
			{
				if(Motor_Tilt1_ParaStu->hall_run_num + 10 < Motor_Tilt2_ParaStu->hall_run_num)
				{
					Motor_ArrivePosition(Motor_Tilt1_ParaStu,HALL_UP_NUM);
				}
				else if (Motor_Tilt2_ParaStu->hall_run_num + 10 < Motor_Tilt1_ParaStu->hall_run_num)
				{
					Motor_ArrivePosition(Motor_Tilt2_ParaStu,HALL_UP_NUM);
				}
				else
				{
					motor_need_click_again = 1;
				}
			}
			else
			{
				Motor_ArrivePosition(Motor_Tilt1_ParaStu,HALL_UP_NUM);	
				Motor_ArrivePosition(Motor_Tilt2_ParaStu,HALL_UP_NUM);	
			}
		}
	}
	if(key_func == KEY_TILT_ALL_DOWN)
	{
		if(Motor_Tilt1_ParaStu == &Motor_Invalid_ParaStu && Motor_Tilt2_ParaStu == &Motor_Invalid_ParaStu)
		{
			Motor_ArrivePosition(Motor_Lumbar_ParaStu,HALL_MIN_NUM);
			Motor_ArrivePosition(Motor_Neck_ParaStu,HALL_MIN_NUM);
		}
		else
		{		
			if(motor_need_click_again == 0)
			{
				if(Motor_Tilt1_ParaStu->hall_run_num > Motor_Tilt2_ParaStu->hall_run_num + 10)
				{
					Motor_ArrivePosition(Motor_Tilt1_ParaStu,HALL_MIN_NUM);
				}
				else if (Motor_Tilt2_ParaStu->hall_run_num > Motor_Tilt1_ParaStu->hall_run_num + 10)
				{
					Motor_ArrivePosition(Motor_Tilt2_ParaStu,HALL_MIN_NUM);
				}
				else
				{
					motor_need_click_again = 1;
				}
			}
			else
			{
				Motor_ArrivePosition(Motor_Tilt1_ParaStu,HALL_MIN_NUM);
				Motor_ArrivePosition(Motor_Tilt2_ParaStu,HALL_MIN_NUM);	
			}
		}
	}
}
//腰部运行函数
void Command_Lumbar_Run(unsigned char key_func)
{
	if(key_func == KEY_LUMBAR_UP)
	{
		Motor_ArrivePosition(Motor_Lumbar_ParaStu,HALL_UP_NUM);		
	}
	if(key_func == KEY_LUMBAR_DOWN)
	{
		Motor_ArrivePosition(Motor_Lumbar_ParaStu,HALL_MIN_NUM);		
	}
}
//腰部2运行函数
void Command_Lumbar2_Run(unsigned char key_func)
{
	if(key_func == KEY_LUMBAR2_UP)
	{
		Motor_ArrivePosition(Motor_Lumbar2_ParaStu,HALL_UP_NUM);		
	}
	if(key_func == KEY_LUMBAR2_DOWN)
	{
		Motor_ArrivePosition(Motor_Lumbar2_ParaStu,HALL_MIN_NUM);		
	}
}
//颈部运行函数
void Command_Neck_Run(unsigned char key_func)
{
	if(key_func == KEY_NECK_UP)
	{
		Motor_ArrivePosition(Motor_Neck_ParaStu,HALL_UP_NUM);		
	}
	if(key_func == KEY_NECK_DOWN)
	{
		Motor_ArrivePosition(Motor_Neck_ParaStu,HALL_MIN_NUM);		
	}
}
//颈部运行函数
void Command_Neck2_Run(unsigned char key_func)
{
	if(key_func == KEY_NECK2_UP)
	{
		Motor_ArrivePosition(Motor_Neck2_ParaStu,HALL_UP_NUM);		
	}
	if(key_func == KEY_NECK2_DOWN)
	{
		Motor_ArrivePosition(Motor_Neck2_ParaStu,HALL_MIN_NUM);		
	}
}
void Command_All_Motor_Run(unsigned char key_func)
{
	if(key_func == KEY_ALL_MOTOR_UP)
	{
		Motor_ArrivePosition(Motor_Back_ParaStu,HALL_UP_NUM);
		Motor_ArrivePosition(Motor_Leg_ParaStu,HALL_UP_NUM);		
		Motor_ArrivePosition(Motor_Lumbar_ParaStu,HALL_UP_NUM);	
		Motor_ArrivePosition(Motor_Lumbar2_ParaStu,HALL_UP_NUM);	
		Motor_ArrivePosition(Motor_Neck_ParaStu,HALL_UP_NUM);	
		Motor_ArrivePosition(Motor_Neck2_ParaStu,HALL_UP_NUM);			
	}
	if(key_func == KEY_ALL_MOTOR_DOWN)
	{
		Motor_ArrivePosition(Motor_Back_ParaStu,HALL_MIN_NUM);	
		Motor_ArrivePosition(Motor_Leg_ParaStu,HALL_MIN_NUM);		
		Motor_ArrivePosition(Motor_Lumbar_ParaStu,HALL_MIN_NUM);	
		Motor_ArrivePosition(Motor_Lumbar2_ParaStu,HALL_MIN_NUM);	
		Motor_ArrivePosition(Motor_Neck_ParaStu,HALL_MIN_NUM);	
		Motor_ArrivePosition(Motor_Neck2_ParaStu,HALL_MIN_NUM);		
	}
}
void Command_Pitch_Run(unsigned char key_func)
{
	if(key_func == KEY_FORWARD)
	{
		Motor_ArrivePosition(Motor_Tilt1_ParaStu,HALL_UP_NUM);
		Motor_ArrivePosition(Motor_Tilt2_ParaStu,HALL_MIN_NUM);
	}
	if(key_func == KEY_BACKWARD)
	{
		Motor_ArrivePosition(Motor_Tilt1_ParaStu,HALL_MIN_NUM);	
		Motor_ArrivePosition(Motor_Tilt2_ParaStu,HALL_UP_NUM);
	}
}
void Command_Tilt1_Run(unsigned char key_func)
{
	if(key_func == KEY_TILT1_UP)
	{
		Motor_ArrivePosition(Motor_Tilt1_ParaStu,HALL_UP_NUM);
	}
	if(key_func == KEY_TILT1_DOWN)
	{
		Motor_ArrivePosition(Motor_Tilt1_ParaStu,HALL_MIN_NUM);	
	}
}
void Command_Tilt2_Run(unsigned char key_func)
{
	if(key_func == KEY_TILT2_UP)
	{
		Motor_ArrivePosition(Motor_Tilt2_ParaStu,HALL_UP_NUM);
	}
	if(key_func == KEY_TILT2_DOWN)
	{
		Motor_ArrivePosition(Motor_Tilt2_ParaStu,HALL_MIN_NUM);	
	}
}

unsigned char Command_OnClick_Back_Up(void)
{
	if(!Motor_ArrivePosition(Motor_Back_ParaStu,HALL_UP_NUM))
	{
		motor_run_fixed_complate = 1;
		motor_oneclick_cmd = 0;
		return 1;
	}
	return 0;
}
unsigned char Command_OnClick_Back_Down(void)
{
	if(!Motor_ArrivePosition(Motor_Back_ParaStu,HALL_MIN_NUM))
	{
		motor_run_fixed_complate = 1;
		motor_oneclick_cmd = 0;
		return 1;
	}
	return 0;
}
unsigned char Command_OnClick_Leg_Up(void)
{
	if(!Motor_ArrivePosition(Motor_Leg_ParaStu,HALL_UP_NUM))
	{
		motor_run_fixed_complate = 1;
		motor_oneclick_cmd = 0;
		return 1;
	}
	return 0;
}
unsigned char Command_OnClick_Leg_Down(void)
{
	if(!Motor_ArrivePosition(Motor_Leg_ParaStu,HALL_MIN_NUM))
	{
		motor_run_fixed_complate = 1;
		motor_oneclick_cmd = 0;
		return 1;
	}
	return 0;
}
unsigned char Command_OnClick_Lumbar_Up(void)
{
	if(!Motor_ArrivePosition(Motor_Lumbar_ParaStu,HALL_UP_NUM))
	{
		motor_run_fixed_complate = 1;
		motor_oneclick_cmd = 0;
		return 1;
	}
	return 0;
}
unsigned char Command_OnClick_Lumbar_Down(void)
{
	if(!Motor_ArrivePosition(Motor_Lumbar_ParaStu,HALL_MIN_NUM))
	{
		motor_run_fixed_complate = 1;
		motor_oneclick_cmd = 0;
		return 1;
	}
	return 0;
}
unsigned char Command_OnClick_Neck_Up(void)
{
	if(!Motor_ArrivePosition(Motor_Neck_ParaStu,HALL_UP_NUM))
	{
		motor_run_fixed_complate = 1;
		motor_oneclick_cmd = 0;
		return 1;
	}
	return 0;
}
unsigned char Command_OnClick_Neck_Down(void)
{
	if(!Motor_ArrivePosition(Motor_Neck_ParaStu,HALL_MIN_NUM))
	{
		motor_run_fixed_complate = 1;
		motor_oneclick_cmd = 0;
		return 1;
	}
	return 0;
}
unsigned char Command_OnClick_Lumbar2_Up(void)
{
	if(!Motor_ArrivePosition(Motor_Lumbar2_ParaStu,HALL_UP_NUM))
	{
		motor_run_fixed_complate = 1;
		motor_oneclick_cmd = 0;
		return 1;
	}
	return 0;
}
unsigned char Command_OnClick_Lumbar2_Down(void)
{
	if(!Motor_ArrivePosition(Motor_Lumbar2_ParaStu,HALL_MIN_NUM))
	{
		motor_run_fixed_complate = 1;
		motor_oneclick_cmd = 0;
		return 1;
	}
	return 0;
}
unsigned char Command_OnClick_Neck2_Up(void)
{
	if(!Motor_ArrivePosition(Motor_Neck2_ParaStu,HALL_UP_NUM))
	{
		motor_run_fixed_complate = 1;
		motor_oneclick_cmd = 0;
		return 1;
	}
	return 0;
}
unsigned char Command_OnClick_Neck2_Down(void)
{
	if(!Motor_ArrivePosition(Motor_Neck2_ParaStu,HALL_MIN_NUM))
	{
		motor_run_fixed_complate = 1;
		motor_oneclick_cmd = 0;
		return 1;
	}
	return 0;
}
unsigned char Command_OnClick_AllMotor_Up(void)
{
	
	return 1;
}
unsigned char Command_OnClick_AllMotor_Down(void)
{

	return  1;
}
unsigned char Command_OnClick_BackLeg_Up(void)
{
	unsigned char motor_complete_temp = 0;
	
	motor_demo_ms_time ++ ;
	if(motor_demo_ms_time >= 200)
	{
		motor_demo_ms_time = 200;
	}
	if(!Motor_ArrivePosition(Motor_Back_ParaStu,HALL_UP_NUM))
	{
		Motor_Para_Reset(Motor_Back_ParaStu);
		motor_complete_temp |= 0x01;
	}
	if(motor_demo_ms_time >= MOTOR_START_RUN_INTRVAL_MS / SYS_TIME_BASE)
	{
		if(!Motor_ArrivePosition(Motor_Leg_ParaStu,HALL_UP_NUM))
		{
			Motor_Para_Reset(Motor_Leg_ParaStu);
			motor_complete_temp |= 0x02;
		}
	}
	//动作完成
	if((motor_complete_temp & 0x03) == 0x03)
	{
		motor_run_fixed_complate = 1;
		motor_demo_ms_time = 0;
		return 1;
	}

	return  0;	
}
unsigned char Command_OnClick_BackLeg_Down(void)
{
	unsigned char motor_complete_temp = 0;
	
	motor_demo_ms_time ++ ;
	if(motor_demo_ms_time >= 200)
	{
		motor_demo_ms_time = 200;
	}
	if(!Motor_ArrivePosition(Motor_Back_ParaStu,HALL_MIN_NUM))
	{
		Motor_Para_Reset(Motor_Back_ParaStu);
		motor_complete_temp |= 0x01;
	}
	if(motor_demo_ms_time >= MOTOR_START_RUN_INTRVAL_MS / SYS_TIME_BASE)
	{
		if(!Motor_ArrivePosition(Motor_Leg_ParaStu,HALL_MIN_NUM))
		{
			Motor_Para_Reset(Motor_Leg_ParaStu);
			motor_complete_temp |= 0x02;
		}
	}
	//动作完成
	if((motor_complete_temp & 0x03) == 0x03)
	{
		motor_run_fixed_complate = 1;
		motor_demo_ms_time = 0;
		return 1;
	}

	return  0;	
}
unsigned char Command_OnClick_LumbarNeck_Up(void)
{
	unsigned char motor_complete_temp = 0;
	
	motor_demo_ms_time ++ ;
	if(motor_demo_ms_time >= 200)
	{
		motor_demo_ms_time = 200;
	}
	if(!Motor_ArrivePosition(Motor_Lumbar_ParaStu,HALL_UP_NUM))
	{
		Motor_Para_Reset(Motor_Lumbar_ParaStu);
		motor_complete_temp |= 0x01;
	}
	if(motor_demo_ms_time >= MOTOR_START_RUN_INTRVAL_MS / SYS_TIME_BASE)
	{
		if(!Motor_ArrivePosition(Motor_Neck_ParaStu,HALL_UP_NUM))
		{
			Motor_Para_Reset(Motor_Neck_ParaStu);
			motor_complete_temp |= 0x02;
		}
	}
	//动作完成
	if((motor_complete_temp & 0x03) == 0x03)
	{
		motor_run_fixed_complate = 1;
		motor_demo_ms_time = 0;
		return 1;
	}

	return  0;	
}
unsigned char Command_OnClick_LumbarNeck_Down(void)
{
	unsigned char motor_complete_temp = 0;
	
	motor_demo_ms_time ++ ;
	if(motor_demo_ms_time >= 200)
	{
		motor_demo_ms_time = 200;
	}
	if(!Motor_ArrivePosition(Motor_Lumbar_ParaStu,HALL_MIN_NUM))
	{
		Motor_Para_Reset(Motor_Lumbar_ParaStu);
		motor_complete_temp |= 0x01;
	}
	if(motor_demo_ms_time >= MOTOR_START_RUN_INTRVAL_MS / SYS_TIME_BASE)
	{
		if(!Motor_ArrivePosition(Motor_Neck_ParaStu,HALL_MIN_NUM))
		{
			Motor_Para_Reset(Motor_Neck_ParaStu);
			motor_complete_temp |= 0x02;
		}
	}
	//动作完成
	if((motor_complete_temp & 0x03) == 0x03)
	{
		motor_run_fixed_complate = 1;
		motor_demo_ms_time = 0;
		return 1;
	}

	return  0;	
}
//运行flat
unsigned char Command_GoFlat_Shape(void)
{
	unsigned char motor_complete_temp = 0;
	
	motor_demo_ms_time ++ ;
	if(motor_demo_ms_time >= 200)
	{
		motor_demo_ms_time = 200;
	}
	if(motor_demo_step == 0)
	{
		if(!Motor_ArrivePosition(Motor_Lumbar_ParaStu,HALL_MIN_NUM))
		{
			motor_demo_step += 2;
		}
		else
		{
			motor_demo_step++;
			motor_demo_ms_time = 0;
		}
	}
	if(motor_demo_step == 1)
	{
		if(motor_demo_ms_time >= MOTOR_START_RUN_INTRVAL_MS / SYS_TIME_BASE)
		{
			motor_demo_step++;
		}
	}
	if(motor_demo_step == 2)
	{
		if(!Motor_ArrivePosition(Motor_Lumbar2_ParaStu,HALL_MIN_NUM))
		{
			motor_demo_step	+= 2;
		}
		else
		{
			motor_demo_step++;
			motor_demo_ms_time = 0;
		}
	}
	if(motor_demo_step == 3)
	{
		if(motor_demo_ms_time >= MOTOR_START_RUN_INTRVAL_MS / SYS_TIME_BASE)
		{
			motor_demo_step++;
		}
	}
	if(motor_demo_step == 4)
	{
		if(!Motor_ArrivePosition(Motor_Neck_ParaStu,HALL_MIN_NUM)) 
		{
			motor_demo_step += 2;
		}
		else
		{
			motor_demo_step++;
			motor_demo_ms_time = 0;
		}
	}
	if(motor_demo_step == 5)
	{
		if(motor_demo_ms_time >= MOTOR_START_RUN_INTRVAL_MS / SYS_TIME_BASE)
		{
			motor_demo_step++;
		}
	}
	if(motor_demo_step == 6)
	{
		if(!Motor_ArrivePosition(Motor_Neck2_ParaStu,HALL_MIN_NUM))
		{
			motor_demo_step += 2;
		}
		else
		{
			motor_demo_step++;
			motor_demo_ms_time = 0;
		}
	}
	if(motor_demo_step == 7)
	{
		if(motor_demo_ms_time >= MOTOR_START_RUN_INTRVAL_MS / SYS_TIME_BASE)
		{
			motor_demo_step++;
		}
	}
	if(motor_demo_step == 8)
	{
		if(system_config.flags.flat_run_tilt_motor == FLAT_RUN_TILT_MOTOR_ENABLE)
		{
			if(!Motor_ArrivePosition(Motor_Tilt1_ParaStu,HALL_MIN_NUM))
			{
				motor_demo_step += 2;
			}
			else
			{
				motor_demo_step++;
				motor_demo_ms_time = 0;
			}
		}
		else
		{
			motor_demo_step += 2;
		}
	}
	if(motor_demo_step == 9)
	{
		if(motor_demo_ms_time >= MOTOR_START_RUN_INTRVAL_MS / SYS_TIME_BASE)
		{
			motor_demo_step++;
		}
	}
	if(motor_demo_step == 10)
	{
		if(system_config.flags.flat_run_tilt_motor == FLAT_RUN_TILT_MOTOR_ENABLE)
		{
			if(!Motor_ArrivePosition(Motor_Tilt2_ParaStu,HALL_MIN_NUM))
			{
				motor_demo_step++;
			}
			else
			{
				motor_demo_step++;
				motor_demo_ms_time = 0;
			}
		}
		else
		{
			motor_demo_step++;
		}
	}
	if(motor_demo_step == 11)
	{
		if(system_config.flags.flat_motor_move_order == FLAT_MOVE_NO_TOGETHER)
		{
			if(!Motor_ArrivePosition(Motor_Lumbar_ParaStu,HALL_MIN_NUM))
			{
				Motor_Para_Reset(Motor_Lumbar_ParaStu);
				motor_complete_temp |= 0x01;
			}
			if(!Motor_ArrivePosition(Motor_Lumbar2_ParaStu,HALL_MIN_NUM))
			{
				Motor_Para_Reset(Motor_Lumbar2_ParaStu);
				motor_complete_temp |= 0x02;
			}
			if(!Motor_ArrivePosition(Motor_Neck_ParaStu,HALL_MIN_NUM))
			{
				Motor_Para_Reset(Motor_Neck_ParaStu);
				motor_complete_temp |= 0x04;
			}
			if(!Motor_ArrivePosition(Motor_Neck2_ParaStu,HALL_MIN_NUM))
			{
				Motor_Para_Reset(Motor_Neck2_ParaStu);
				motor_complete_temp |= 0x08;
			}
			if(system_config.flags.flat_run_tilt_motor == FLAT_RUN_TILT_MOTOR_ENABLE)
			{
				if(!Motor_ArrivePosition(Motor_Tilt1_ParaStu,HALL_MIN_NUM))
				{
					Motor_Para_Reset(Motor_Tilt1_ParaStu);
					motor_complete_temp |= 0x10;
				}
				if(!Motor_ArrivePosition(Motor_Tilt2_ParaStu,HALL_MIN_NUM))
				{
					Motor_Para_Reset(Motor_Tilt2_ParaStu);
					motor_complete_temp |= 0x20;
				}
			}
			else
			{
				motor_complete_temp |= 0x30;
			}
			//动作完成
			if((motor_complete_temp & 0x3F) == 0x3F)
			{
				motor_complete_temp = 0;
				motor_demo_ms_time = 0;
				motor_demo_step++;	
			}				
		}
		else
		{
			motor_demo_step++;
		}
	}
	if(motor_demo_step == 12)
	{
		if(!Motor_ArrivePosition(Motor_Back_ParaStu,HALL_MIN_NUM))
		{
			motor_demo_step += 2;
		}
		else
		{
			motor_demo_step++;
			motor_demo_ms_time = 0;
		}
	}
	if(motor_demo_step == 13)
	{
		if(motor_demo_ms_time >= MOTOR_START_RUN_INTRVAL_MS / SYS_TIME_BASE)
		{
			motor_demo_step++;
		}
	}
	if(motor_demo_step == 14)
	{
		if(!Motor_ArrivePosition(Motor_Leg_ParaStu,HALL_MIN_NUM))
		{
			motor_demo_step++;
		}
		else
		{
			motor_demo_step++;
			motor_demo_ms_time = 0;
		}
	}
	if(motor_demo_step == 15)
	{
		if(!Motor_ArrivePosition(Motor_Lumbar_ParaStu,HALL_MIN_NUM))
		{
			Motor_Para_Reset(Motor_Lumbar_ParaStu);
			motor_complete_temp |= 0x01;
		}
		if(!Motor_ArrivePosition(Motor_Lumbar2_ParaStu,HALL_MIN_NUM))
		{
			Motor_Para_Reset(Motor_Lumbar2_ParaStu);
			motor_complete_temp |= 0x02;
		}
		if(!Motor_ArrivePosition(Motor_Neck_ParaStu,HALL_MIN_NUM))
		{
			Motor_Para_Reset(Motor_Neck_ParaStu);
			motor_complete_temp |= 0x04;
		}
		if(!Motor_ArrivePosition(Motor_Neck2_ParaStu,HALL_MIN_NUM))
		{
			Motor_Para_Reset(Motor_Neck2_ParaStu);
			motor_complete_temp |= 0x08;
		}
		if(!Motor_ArrivePosition(Motor_Back_ParaStu,HALL_MIN_NUM))
		{
			Motor_Para_Reset(Motor_Back_ParaStu);
			motor_complete_temp |= 0x10;
		}			
		if(!Motor_ArrivePosition(Motor_Leg_ParaStu,HALL_MIN_NUM))
		{
			Motor_Para_Reset(Motor_Leg_ParaStu);
			motor_complete_temp |= 0x20;
		}
		if(system_config.flags.flat_run_tilt_motor == FLAT_RUN_TILT_MOTOR_ENABLE)
		{
			if(!Motor_ArrivePosition(Motor_Tilt1_ParaStu,HALL_MIN_NUM))
			{
				Motor_Para_Reset(Motor_Tilt1_ParaStu);
				motor_complete_temp |= 0x40;
			}
			if(!Motor_ArrivePosition(Motor_Tilt2_ParaStu,HALL_MIN_NUM))
			{
				Motor_Para_Reset(Motor_Tilt2_ParaStu);
				motor_complete_temp |= 0x80;
			}
		}
		else
		{
			motor_complete_temp |= 0xC0;
		}
		//动作完成
		if((motor_complete_temp & 0xFF) == 0xFF)
		{
			motor_run_fixed_complate = 1;
			motor_demo_ms_time = 0;
			motor_demo_step = 0;
			return 1;
		}		
	}
	return  0;
}
//运行至M1
unsigned char Command_GoM1_Shape(void)
{
	if(1 == Command_GoMemory_Shape(MOTOR_M1_INFO))
	{
		motor_run_fixed_complate = 1;
		motor_oneclick_cmd = 0;
		return 1;
	}
	
	return 0 ;
}
//运行至M2
unsigned char Command_GoM2_Shape(void)
{
	if(1 == Command_GoMemory_Shape(MOTOR_M2_INFO))
	{
		motor_run_fixed_complate = 1;
		motor_oneclick_cmd = 0;
		return 1;
	}
	
	return 0 ;
}
//运行至M3
unsigned char Command_GoM3_Shape(void)
{
	if(1 == Command_GoMemory_Shape(MOTOR_M3_INFO))
	{
		motor_run_fixed_complate = 1;
		motor_oneclick_cmd = 0;
		return 1;
	}
	
	return 0 ;
}
//运行至TV
unsigned char Command_GoTV_Shape(void)
{
	if(1 == Command_GoFixed_Shape(MOTOR_TV_INFO))
	{
		motor_run_fixed_complate = 1;
		motor_oneclick_cmd = 0;
		return 1;		
	}
	
	return 0;
}
//运行至ZG
unsigned char Command_GoZeroG_Shape(void)
{
	if(1 == Command_GoFixed_Shape(MOTOR_ZEROG_INFO))
	{
		motor_run_fixed_complate = 1;
		motor_oneclick_cmd = 0;
		return 1;		
	}
	
	return 0;
}
//运行至LOUNGE
unsigned char Command_GoLounge_Shape(void)
{
	if(1 == Command_GoFixed_Shape(MOTOR_LOUNGE_INFO))
	{
		motor_run_fixed_complate = 1;
		motor_oneclick_cmd = 0;
		return 1;			
	}
	
	return 0;
}
//运行至SNORE
unsigned char Command_GoSnore_Shape(void)
{
	if(1 == Command_GoFixed_Shape(MOTOR_SNORE_INFO))
	{
		motor_run_fixed_complate = 1;
		motor_oneclick_cmd = 0;
		return 1;			
	}
	
	return 0;
}
//运行至READ
unsigned char Command_GoRead_Shape(void)
{
	if(1 == Command_GoFixed_Shape(MOTOR_READ_INFO))
	{
		motor_run_fixed_complate = 1;
		motor_oneclick_cmd = 0;
		return 1;			
	}
	
	return 0;
}
//运行至YOGA
unsigned char Command_GoYoga_Shape(void)
{
	if(1 == Command_GoFixed_Shape(MOTOR_YOGA_INFO))
	{
		motor_run_fixed_complate = 1;
		motor_oneclick_cmd = 0;
		return 1;			
	}
	
	return 0;
}
//运行GETUP
unsigned char Command_GoGetUp_Shape(void)
{
	if(1 == Command_GoFixed_Shape(MOTOR_GETUP_INFO))
	{
		motor_run_fixed_complate = 1;
		motor_oneclick_cmd = 0;
		return 1;			
	}
	
	return 0;
}

//运行NURSING
unsigned char Command_GoNursing_Shape(void)
{
	if(1 == Command_GoFixed_Shape(MOTOR_NURSING_INFO))
	{
		motor_run_fixed_complate = 1;
		motor_oneclick_cmd = 0;
		return 1;			
	}
	
	return 0;
}

//根据参数跑固定位置
unsigned char Command_GoFixed_Shape(SAVE_MOTOR_INFO_TYPE save_info_type)
{
	unsigned char motor_complete_temp = 0;
	
	motor_demo_ms_time ++ ;
	if(motor_demo_ms_time >= 200)
	{
		motor_demo_ms_time = 200;
	}

	if(motor_demo_step == 0)
	{
		if(!Motor_ArrivePosition(Motor_Back_ParaStu,User_ReadMotor_Info(save_info_type,Motor_Back_ParaStu->motor_port)))
		{
			motor_demo_step = 2;
		}
		else
		{
			motor_demo_step++;
			motor_demo_ms_time = 0;
		}
	}
	if(motor_demo_step == 1)
	{
		if(motor_demo_ms_time >= MOTOR_START_RUN_INTRVAL_MS / SYS_TIME_BASE)
		{
			motor_demo_step++;
		}
	}
	if(motor_demo_step == 2)
	{
		if(!Motor_ArrivePosition(Motor_Leg_ParaStu,User_ReadMotor_Info(save_info_type,Motor_Leg_ParaStu->motor_port)))
		{
			motor_demo_step = 3;
		}
		else
		{
			motor_demo_step++;
			motor_demo_ms_time = 0;
		}
	}
	if(motor_demo_step == 3)
	{
		if(!Motor_ArrivePosition(Motor_Back_ParaStu,User_ReadMotor_Info(save_info_type,Motor_Back_ParaStu->motor_port)))
		{
			motor_complete_temp |= 0x01;
		}
		if(!Motor_ArrivePosition(Motor_Leg_ParaStu,User_ReadMotor_Info(save_info_type,Motor_Leg_ParaStu->motor_port)))
		{
			motor_complete_temp |= 0x02;
		}

		if(system_config.flags.fixed_position_extend_move == OUTSIDE_MOTOR_NO_MEMORY)//外拖不需要动作
		{
			if((motor_complete_temp & 0x03) == 0x03 )
			{
				motor_demo_ms_time = 0; 
				motor_demo_step = 0;
				return 1;// 直接完成
			}
		}
		else
		{
			if(system_config.flags.fixed_position_motor_move_order == FIX_POSITION_MOVE_TOGETHER)//外拖同时动作
			{
				motor_demo_step ++;
				motor_demo_ms_time = 0; 
			}
			else//外拖后动作
			{
				if((motor_complete_temp & 0x03) == 0x03 )
				{
					motor_demo_ms_time = 0; 
					motor_demo_step++;
				}				
			}
		}
	}
	if(motor_demo_step == 4)
	{
		if(!Motor_ArrivePosition(Motor_Lumbar_ParaStu,User_ReadMotor_Info(save_info_type,Motor_Lumbar_ParaStu->motor_port)))
		{
			motor_demo_step = 6;
		}
		else
		{
			motor_demo_step++;
			motor_demo_ms_time = 0;
		}
	}
	if(motor_demo_step == 5)
	{
		if(motor_demo_ms_time >= MOTOR_START_RUN_INTRVAL_MS / SYS_TIME_BASE)
		{
			motor_demo_step++;
		}
	}
	if(motor_demo_step == 6)
	{
		if(!Motor_ArrivePosition(Motor_Neck_ParaStu,User_ReadMotor_Info(save_info_type,Motor_Neck_ParaStu->motor_port)))
		{
			motor_demo_step = 8;
		}
		else
		{
			motor_demo_step++;
			motor_demo_ms_time = 0;
		}
	}
	if(motor_demo_step == 7)
	{
		if(motor_demo_ms_time >= MOTOR_START_RUN_INTRVAL_MS / SYS_TIME_BASE)
		{
			motor_demo_step++;
		}
	}
	if(motor_demo_step == 8)
	{
		if(!Motor_ArrivePosition(Motor_Lumbar2_ParaStu,User_ReadMotor_Info(save_info_type,Motor_Lumbar2_ParaStu->motor_port)))
		{
			motor_demo_step = 10;
		}
		else
		{
			motor_demo_step++;
			motor_demo_ms_time = 0;
		}
	}
	if(motor_demo_step == 9)
	{
		if(motor_demo_ms_time >= MOTOR_START_RUN_INTRVAL_MS / SYS_TIME_BASE)
		{
			motor_demo_step++;
		}
	}
	if(motor_demo_step == 10)
	{
		if(!Motor_ArrivePosition(Motor_Neck2_ParaStu,User_ReadMotor_Info(save_info_type,Motor_Neck2_ParaStu->motor_port)))
		{
			motor_demo_step++;
		}
		else
		{
			motor_demo_step++;
			motor_demo_ms_time = 0;
		}
	}
	if(motor_demo_step == 11)
	{
		if(!Motor_ArrivePosition(Motor_Back_ParaStu,User_ReadMotor_Info(save_info_type,Motor_Back_ParaStu->motor_port)))
		{
			motor_complete_temp |= 0x01;
		}
		if(!Motor_ArrivePosition(Motor_Leg_ParaStu,User_ReadMotor_Info(save_info_type,Motor_Leg_ParaStu->motor_port)))
		{
			motor_complete_temp |= 0x02;
		}
		if(!Motor_ArrivePosition(Motor_Lumbar_ParaStu,User_ReadMotor_Info(save_info_type,Motor_Lumbar_ParaStu->motor_port)))
		{
			motor_complete_temp |= 0x04;
		}
		if(!Motor_ArrivePosition(Motor_Neck_ParaStu,User_ReadMotor_Info(save_info_type,Motor_Neck_ParaStu->motor_port)))
		{
			motor_complete_temp |= 0x08;
		}
		if(!Motor_ArrivePosition(Motor_Lumbar2_ParaStu,User_ReadMotor_Info(save_info_type,Motor_Lumbar2_ParaStu->motor_port)))
		{
			motor_complete_temp |= 0x10;
		}
		if(!Motor_ArrivePosition(Motor_Neck2_ParaStu,User_ReadMotor_Info(save_info_type,Motor_Neck2_ParaStu->motor_port)))
		{
			motor_complete_temp |= 0x20;
		}
			//动作完成
		if(((motor_complete_temp & 0x3F) == 0x3F) && (system_config.flags.fixed_position_extend_move == OUTSIDE_MOTOR_MEMORY))
		{
			motor_demo_ms_time = 0;
			motor_demo_step = 0;
			return 1;
		}		
	}
	
	return 0;
	
}
//根据参数跑记忆位置
unsigned char Command_GoMemory_Shape(SAVE_MOTOR_INFO_TYPE save_info_type)
{
	unsigned char motor_complete_temp = 0;
	
	motor_demo_ms_time ++ ;
	if(motor_demo_ms_time >= 200)
	{
		motor_demo_ms_time = 200;
	}

	if(motor_demo_step == 0)
	{
		if(!Motor_ArrivePosition(Motor_Back_ParaStu,User_ReadMotor_Info(save_info_type,Motor_Back_ParaStu->motor_port)))
		{
			motor_demo_step = 2;
		}
		else
		{
			motor_demo_step++;
			motor_demo_ms_time = 0;
		}
	}
	if(motor_demo_step == 1)
	{
		if(motor_demo_ms_time >= MOTOR_START_RUN_INTRVAL_MS / SYS_TIME_BASE)
		{
			motor_demo_step++;
		}
	}
	if(motor_demo_step == 2)
	{
		if(!Motor_ArrivePosition(Motor_Leg_ParaStu,User_ReadMotor_Info(save_info_type,Motor_Leg_ParaStu->motor_port)))
		{
			motor_demo_step = 3;
		}
		else
		{
			motor_demo_step++;
			motor_demo_ms_time = 0;
		}
	}
	if(motor_demo_step == 3)
	{
		if(!Motor_ArrivePosition(Motor_Back_ParaStu,User_ReadMotor_Info(save_info_type,Motor_Back_ParaStu->motor_port)))
		{
			motor_complete_temp |= 0x01;
		}
		if(!Motor_ArrivePosition(Motor_Leg_ParaStu,User_ReadMotor_Info(save_info_type,Motor_Leg_ParaStu->motor_port)))
		{
			motor_complete_temp |= 0x02;
		}

		if(system_config.flags.fixed_position_extend_move == OUTSIDE_MOTOR_NO_MEMORY)//外拖不需要动作
		{
			if((motor_complete_temp & 0x03) == 0x03 )
			{
				motor_demo_ms_time = 0; 
				motor_demo_step = 0;
				return 1;// 直接完成
			}
		}
		else
		{
			if(system_config.flags.fixed_position_motor_move_order == FIX_POSITION_MOVE_TOGETHER)//外拖同时动作
			{
				motor_demo_step ++;
				motor_demo_ms_time = 0; 
			}
			else//外拖后动作
			{
				if((motor_complete_temp & 0x03) == 0x03 )
				{
					motor_demo_ms_time = 0; 
					motor_demo_step++;
				}				
			}
		}
	}
	if(motor_demo_step == 4)
	{
		if(!Motor_ArrivePosition(Motor_Lumbar_ParaStu,User_ReadMotor_Info(save_info_type,Motor_Lumbar_ParaStu->motor_port)))
		{
			motor_demo_step = 6;
		}
		else
		{
			motor_demo_step++;
			motor_demo_ms_time = 0;
		}
	}
	if(motor_demo_step == 5)
	{
		if(motor_demo_ms_time >= MOTOR_START_RUN_INTRVAL_MS / SYS_TIME_BASE)
		{
			motor_demo_step++;
		}
	}
	if(motor_demo_step == 6)
	{
		if(!Motor_ArrivePosition(Motor_Neck_ParaStu,User_ReadMotor_Info(save_info_type,Motor_Neck_ParaStu->motor_port)))
		{
			motor_demo_step = 8;
		}
		else
		{
			motor_demo_step++;
			motor_demo_ms_time = 0;
		}
	}
	if(motor_demo_step == 7)
	{
		if(motor_demo_ms_time >= MOTOR_START_RUN_INTRVAL_MS / SYS_TIME_BASE)
		{
			motor_demo_step++;
		}
	}
	if(motor_demo_step == 8)
	{
		if(!Motor_ArrivePosition(Motor_Lumbar2_ParaStu,User_ReadMotor_Info(save_info_type,Motor_Lumbar2_ParaStu->motor_port)))
		{
			motor_demo_step = 10;
		}
		else
		{
			motor_demo_step++;
			motor_demo_ms_time = 0;
		}
	}
	if(motor_demo_step == 9)
	{
		if(motor_demo_ms_time >= MOTOR_START_RUN_INTRVAL_MS / SYS_TIME_BASE)
		{
			motor_demo_step++;
		}
	}
	if(motor_demo_step == 10)
	{
		if(!Motor_ArrivePosition(Motor_Neck2_ParaStu,User_ReadMotor_Info(save_info_type,Motor_Neck2_ParaStu->motor_port)))
		{
			motor_demo_step++;
		}
		else
		{
			motor_demo_step++;
			motor_demo_ms_time = 0;
		}
	}
	if(motor_demo_step == 11)
	{
		if(!Motor_ArrivePosition(Motor_Back_ParaStu,User_ReadMotor_Info(save_info_type,Motor_Back_ParaStu->motor_port)))
		{
			motor_complete_temp |= 0x01;
		}
		if(!Motor_ArrivePosition(Motor_Leg_ParaStu,User_ReadMotor_Info(save_info_type,Motor_Leg_ParaStu->motor_port)))
		{
			motor_complete_temp |= 0x02;
		}
		if(!Motor_ArrivePosition(Motor_Lumbar_ParaStu,User_ReadMotor_Info(save_info_type,Motor_Lumbar_ParaStu->motor_port)))
		{
			motor_complete_temp |= 0x04;
		}
		if(!Motor_ArrivePosition(Motor_Neck_ParaStu,User_ReadMotor_Info(save_info_type,Motor_Neck_ParaStu->motor_port)))
		{
			motor_complete_temp |= 0x08;
		}
		if(!Motor_ArrivePosition(Motor_Lumbar2_ParaStu,User_ReadMotor_Info(save_info_type,Motor_Lumbar2_ParaStu->motor_port)))
		{
			motor_complete_temp |= 0x10;
		}
		if(!Motor_ArrivePosition(Motor_Neck2_ParaStu,User_ReadMotor_Info(save_info_type,Motor_Neck2_ParaStu->motor_port)))
		{
			motor_complete_temp |= 0x20;
		}
			//动作完成
		if(((motor_complete_temp & 0x3F) == 0x3F) && (system_config.flags.fixed_position_extend_move == OUTSIDE_MOTOR_MEMORY))
		{
			motor_demo_ms_time = 0;
			motor_demo_step = 0;
			return 1;
		}		
	}
	
	return 0;
}
//保存电机霍尔， 刷新buff
unsigned char Command_Save_MotorHall(void)
{
	//读取原有数据
	User_Read_MotorInfoEeprom(); 

	User_SaveMotor_Info(MOTOR_HALL_INFO,MOTOR1_PORT,Motor1_ParaStu.hall_run_num);
	User_SaveMotor_Info(MOTOR_HALL_INFO,MOTOR2_PORT,Motor2_ParaStu.hall_run_num);
	User_SaveMotor_Info(MOTOR_HALL_INFO,MOTOR3_PORT,Motor3_ParaStu.hall_run_num);
	User_SaveMotor_Info(MOTOR_HALL_INFO,MOTOR4_PORT,Motor4_ParaStu.hall_run_num);
	User_SaveMotor_Info(MOTOR_HALL_INFO,MOTOR5_PORT,Motor5_ParaStu.hall_run_num);
	User_SaveMotor_Info(MOTOR_HALL_INFO,MOTOR6_PORT,Motor6_ParaStu.hall_run_num);
	//擦出扇区
	User_Erase_MotorInfoEeprom(); 	
	//写EEPROM
	return User_Write_MotorInfoEeprom();
}
//保存固定位置
unsigned char Command_Save_Shape(SAVE_MOTOR_INFO_TYPE motor_info)
{
	//读取原有数据
	User_Read_MotorInfoEeprom(); 

	User_SaveMotor_Info(motor_info,Motor_Back_ParaStu->motor_port,Motor_Back_ParaStu->hall_run_num);
	User_SaveMotor_Info(motor_info,Motor_Leg_ParaStu->motor_port,Motor_Leg_ParaStu->hall_run_num);
	User_SaveMotor_Info(motor_info,Motor_Lumbar_ParaStu->motor_port,Motor_Lumbar_ParaStu->hall_run_num);
	User_SaveMotor_Info(motor_info,Motor_Neck_ParaStu->motor_port,Motor_Neck_ParaStu->hall_run_num);
	User_SaveMotor_Info(motor_info,Motor_Lumbar2_ParaStu->motor_port,Motor_Lumbar2_ParaStu->hall_run_num);
	User_SaveMotor_Info(motor_info,Motor_Neck2_ParaStu->motor_port,Motor_Neck2_ParaStu->hall_run_num);
	//擦出扇区
	User_Erase_MotorInfoEeprom(); 	
	//写EEPROM
	return User_Write_MotorInfoEeprom();
}
unsigned char Command_SaveM1_Shape(void)
{
	return Command_Save_Shape(MOTOR_M1_INFO);
}
unsigned char Command_SaveM2_Shape(void)
{
	return Command_Save_Shape(MOTOR_M2_INFO);
}
unsigned char Command_SaveM3_Shape(void)
{
	return Command_Save_Shape(MOTOR_M3_INFO);
}
unsigned char Command_SaveTV_Shape(void)
{
	return Command_Save_Shape(MOTOR_TV_INFO);
}
unsigned char Command_SaveZeroG_Shape(void)
{
	return Command_Save_Shape(MOTOR_ZEROG_INFO);	
}
unsigned char Command_SaveLounge_Shape(void)
{
  return Command_Save_Shape(MOTOR_LOUNGE_INFO);	
}
unsigned char Command_SaveSnore_Shape(void)
{
  return Command_Save_Shape(MOTOR_SNORE_INFO);
}
unsigned char Command_SaveRead_Shape(void)
{
  return Command_Save_Shape(MOTOR_READ_INFO);
}
unsigned char Command_SaveYoga_Shape(void)
{
  return Command_Save_Shape(MOTOR_YOGA_INFO);
}
unsigned char Command_SaveGetUp_Shape(void)
{
  return Command_Save_Shape(MOTOR_GETUP_INFO);
}
unsigned char Command_SaveNursing_Shape(void)
{
  return Command_Save_Shape(MOTOR_NURSING_INFO);
}

unsigned char Command_FactoryReset_Shape(void)
{
	//读取原有数据
	User_Read_MotorInfoEeprom(); 

	User_SaveMotor_Info(MOTOR_TV_INFO,Motor_Back_ParaStu->motor_port,	system_config.flags.tv_hall_back_default);
	User_SaveMotor_Info(MOTOR_TV_INFO,Motor_Leg_ParaStu->motor_port,		system_config.flags.tv_hall_leg_default);
	User_SaveMotor_Info(MOTOR_TV_INFO,Motor_Lumbar_ParaStu->motor_port,system_config.flags.tv_hall_lumbar_default);
	User_SaveMotor_Info(MOTOR_TV_INFO,Motor_Neck_ParaStu->motor_port,	system_config.flags.tv_hall_neck_default);
	User_SaveMotor_Info(MOTOR_TV_INFO,Motor_Lumbar2_ParaStu->motor_port,system_config.flags.tv_hall_lumbar2_default);
	User_SaveMotor_Info(MOTOR_TV_INFO,Motor_Neck2_ParaStu->motor_port,	system_config.flags.tv_hall_neck2_default);
	
	User_SaveMotor_Info(MOTOR_ZEROG_INFO,Motor_Back_ParaStu->motor_port,		system_config.flags.zerog_hall_back_default);
	User_SaveMotor_Info(MOTOR_ZEROG_INFO,Motor_Leg_ParaStu->motor_port,		system_config.flags.zerog_hall_leg_default);
	User_SaveMotor_Info(MOTOR_ZEROG_INFO,Motor_Lumbar_ParaStu->motor_port,	system_config.flags.zerog_hall_lumbar_default);
	User_SaveMotor_Info(MOTOR_ZEROG_INFO,Motor_Neck_ParaStu->motor_port,		system_config.flags.zerog_hall_neck_default);
	User_SaveMotor_Info(MOTOR_ZEROG_INFO,Motor_Lumbar2_ParaStu->motor_port,	system_config.flags.zerog_hall_lumbar2_default);
	User_SaveMotor_Info(MOTOR_ZEROG_INFO,Motor_Neck2_ParaStu->motor_port,		system_config.flags.zerog_hall_neck2_default);
	
	User_SaveMotor_Info(MOTOR_LOUNGE_INFO,Motor_Back_ParaStu->motor_port,	system_config.flags.lounge_hall_back_default);
	User_SaveMotor_Info(MOTOR_LOUNGE_INFO,Motor_Leg_ParaStu->motor_port,		system_config.flags.lounge_hall_leg_default);
	User_SaveMotor_Info(MOTOR_LOUNGE_INFO,Motor_Lumbar_ParaStu->motor_port,system_config.flags.lounge_hall_lumbar_default);
	User_SaveMotor_Info(MOTOR_LOUNGE_INFO,Motor_Neck_ParaStu->motor_port,	system_config.flags.lounge_hall_neck_default);
	User_SaveMotor_Info(MOTOR_LOUNGE_INFO,Motor_Lumbar2_ParaStu->motor_port,system_config.flags.lounge_hall_lumbar2_default);
	User_SaveMotor_Info(MOTOR_LOUNGE_INFO,Motor_Neck2_ParaStu->motor_port,	system_config.flags.lounge_hall_neck2_default);
	
	User_SaveMotor_Info(MOTOR_SNORE_INFO,Motor_Back_ParaStu->motor_port,		system_config.flags.snore_hall_back_default);
	User_SaveMotor_Info(MOTOR_SNORE_INFO,Motor_Leg_ParaStu->motor_port,		system_config.flags.snore_hall_leg_default);
	User_SaveMotor_Info(MOTOR_SNORE_INFO,Motor_Lumbar_ParaStu->motor_port,	system_config.flags.snore_hall_lumbar_default);
	User_SaveMotor_Info(MOTOR_SNORE_INFO,Motor_Neck_ParaStu->motor_port,		system_config.flags.snore_hall_neck_default);
	User_SaveMotor_Info(MOTOR_SNORE_INFO,Motor_Lumbar2_ParaStu->motor_port,	system_config.flags.snore_hall_lumbar2_default);
	User_SaveMotor_Info(MOTOR_SNORE_INFO,Motor_Neck2_ParaStu->motor_port,		system_config.flags.snore_hall_neck2_default);
	
	User_SaveMotor_Info(MOTOR_READ_INFO,Motor_Back_ParaStu->motor_port,	system_config.flags.read_hall_back_default);
	User_SaveMotor_Info(MOTOR_READ_INFO,Motor_Leg_ParaStu->motor_port,		system_config.flags.read_hall_leg_default);
	User_SaveMotor_Info(MOTOR_READ_INFO,Motor_Lumbar_ParaStu->motor_port,system_config.flags.read_hall_lumbar_default);
	User_SaveMotor_Info(MOTOR_READ_INFO,Motor_Neck_ParaStu->motor_port,	system_config.flags.read_hall_neck_default);
	User_SaveMotor_Info(MOTOR_READ_INFO,Motor_Lumbar2_ParaStu->motor_port,system_config.flags.read_hall_lumbar2_default);
	User_SaveMotor_Info(MOTOR_READ_INFO,Motor_Neck2_ParaStu->motor_port,	system_config.flags.read_hall_neck2_default);
	
	User_SaveMotor_Info(MOTOR_YOGA_INFO,Motor_Back_ParaStu->motor_port,	system_config.flags.yoga_hall_back_default);
	User_SaveMotor_Info(MOTOR_YOGA_INFO,Motor_Leg_ParaStu->motor_port,		system_config.flags.yoga_hall_leg_default);
	User_SaveMotor_Info(MOTOR_YOGA_INFO,Motor_Lumbar_ParaStu->motor_port,system_config.flags.yoga_hall_lumbar_default);
	User_SaveMotor_Info(MOTOR_YOGA_INFO,Motor_Neck_ParaStu->motor_port,	system_config.flags.yoga_hall_neck_default);
	User_SaveMotor_Info(MOTOR_YOGA_INFO,Motor_Lumbar2_ParaStu->motor_port,system_config.flags.yoga_hall_lumbar2_default);
	User_SaveMotor_Info(MOTOR_YOGA_INFO,Motor_Neck2_ParaStu->motor_port,	system_config.flags.yoga_hall_neck2_default);
	
	User_SaveMotor_Info(MOTOR_GETUP_INFO,Motor_Back_ParaStu->motor_port,	system_config.flags.getup_hall_back_default);
	User_SaveMotor_Info(MOTOR_GETUP_INFO,Motor_Leg_ParaStu->motor_port,		system_config.flags.getup_hall_leg_default);
	User_SaveMotor_Info(MOTOR_GETUP_INFO,Motor_Lumbar_ParaStu->motor_port,system_config.flags.getup_hall_lumbar_default);
	User_SaveMotor_Info(MOTOR_GETUP_INFO,Motor_Neck_ParaStu->motor_port,	system_config.flags.getup_hall_neck_default);
	User_SaveMotor_Info(MOTOR_GETUP_INFO,Motor_Lumbar2_ParaStu->motor_port,system_config.flags.getup_hall_lumbar2_default);
	User_SaveMotor_Info(MOTOR_GETUP_INFO,Motor_Neck2_ParaStu->motor_port,	system_config.flags.getup_hall_neck2_default);
	
	User_SaveMotor_Info(MOTOR_NURSING_INFO,Motor_Back_ParaStu->motor_port,	system_config.flags.nursing_hall_back_default);
	User_SaveMotor_Info(MOTOR_NURSING_INFO,Motor_Leg_ParaStu->motor_port,		system_config.flags.nursing_hall_leg_default);
	User_SaveMotor_Info(MOTOR_NURSING_INFO,Motor_Lumbar_ParaStu->motor_port,system_config.flags.nursing_hall_lumbar_default);
	User_SaveMotor_Info(MOTOR_NURSING_INFO,Motor_Neck_ParaStu->motor_port,	system_config.flags.nursing_hall_neck_default);
	User_SaveMotor_Info(MOTOR_NURSING_INFO,Motor_Lumbar2_ParaStu->motor_port,system_config.flags.nursing_hall_lumbar2_default);
	User_SaveMotor_Info(MOTOR_NURSING_INFO,Motor_Neck2_ParaStu->motor_port,	system_config.flags.nursing_hall_neck2_default);

	User_SaveMotor_Info(MOTOR_M1_INFO,Motor_Back_ParaStu->motor_port,system_config.flags.mem1_position_hall_back);
	User_SaveMotor_Info(MOTOR_M1_INFO,Motor_Leg_ParaStu->motor_port,system_config.flags.mem1_position_hall_leg);
	User_SaveMotor_Info(MOTOR_M1_INFO,Motor_Lumbar_ParaStu->motor_port,system_config.flags.mem1_position_hall_lumbar);
	User_SaveMotor_Info(MOTOR_M1_INFO,Motor_Neck_ParaStu->motor_port,system_config.flags.mem1_position_hall_neck);
	User_SaveMotor_Info(MOTOR_M1_INFO,Motor_Lumbar2_ParaStu->motor_port,system_config.flags.mem1_position_hall_lumbar2);
	User_SaveMotor_Info(MOTOR_M1_INFO,Motor_Neck2_ParaStu->motor_port,system_config.flags.mem1_position_hall_neck2);
	
	User_SaveMotor_Info(MOTOR_M2_INFO,Motor_Back_ParaStu->motor_port,system_config.flags.mem2_position_hall_back);
	User_SaveMotor_Info(MOTOR_M2_INFO,Motor_Leg_ParaStu->motor_port,system_config.flags.mem2_position_hall_leg);
	User_SaveMotor_Info(MOTOR_M2_INFO,Motor_Lumbar_ParaStu->motor_port,system_config.flags.mem2_position_hall_lumbar);
	User_SaveMotor_Info(MOTOR_M2_INFO,Motor_Neck_ParaStu->motor_port,system_config.flags.mem2_position_hall_neck);
	User_SaveMotor_Info(MOTOR_M2_INFO,Motor_Lumbar2_ParaStu->motor_port,system_config.flags.mem2_position_hall_lumbar2);
	User_SaveMotor_Info(MOTOR_M2_INFO,Motor_Neck2_ParaStu->motor_port,system_config.flags.mem2_position_hall_neck2);
	
	User_SaveMotor_Info(MOTOR_M3_INFO,Motor_Back_ParaStu->motor_port,system_config.flags.mem3_position_hall_back);
	User_SaveMotor_Info(MOTOR_M3_INFO,Motor_Leg_ParaStu->motor_port,system_config.flags.mem3_position_hall_leg);
	User_SaveMotor_Info(MOTOR_M3_INFO,Motor_Lumbar_ParaStu->motor_port,system_config.flags.mem3_position_hall_lumbar);
	User_SaveMotor_Info(MOTOR_M3_INFO,Motor_Neck_ParaStu->motor_port,system_config.flags.mem3_position_hall_neck);
	User_SaveMotor_Info(MOTOR_M3_INFO,Motor_Lumbar2_ParaStu->motor_port,system_config.flags.mem3_position_hall_lumbar2);
	User_SaveMotor_Info(MOTOR_M3_INFO,Motor_Neck2_ParaStu->motor_port,system_config.flags.mem3_position_hall_neck2);
	//擦出扇区
	User_Erase_MotorInfoEeprom(); 	
	//写EEPROM
	return User_Write_MotorInfoEeprom();
}
unsigned char Save_Shape_Enable(void) //
{
	motor_oneclick_cmd = KEY_NO;  //清除 防止一键动作一直有键值
	
	if(motor_ctr_offline != 2)  //等待所有信号消失  不加这个lin发送会持续记忆
	{
		motor_ctr_offline = 2;
	}
	else
	{
		return 0;
	}	
	
	return 1;
}
/*------------------------------------------------------------------------------------------*/
//电机定时器中断函数
void Motor_TimerManagerTask(void)
{
	//LIN发送时间  50ms
	motor_lin_send_time ++;
	if(motor_lin_send_time >= 10)
	{
		motor_lin_send_time = 10;
	}
	//电机快速切换延时
	if(motor_cmd_change_fast != 0)
	{
		motor_cmd_delay_time ++;
		
		Motor_PwmImStop(&Motor1_ParaStu);      
		Motor_PwmImStop(&Motor2_ParaStu);
		Motor_PwmImStop(&Motor3_ParaStu);
		Motor_PwmImStop(&Motor4_ParaStu);
		Motor_PwmImStop(&Motor5_ParaStu);
    Motor_PwmImStop(&Motor6_ParaStu);
		
		Motor_Back_ParaStu->motor_slow_run_flag = 0;
		Motor_Leg_ParaStu->motor_slow_run_flag = 0;		
		Motor_Lumbar_ParaStu->motor_slow_run_flag = 0;
		Motor_Neck_ParaStu->motor_slow_run_flag = 	0;
		Motor_Lumbar2_ParaStu->motor_slow_run_flag = 0;
		Motor_Neck2_ParaStu->motor_slow_run_flag = 	0;	
		Motor_Tilt1_ParaStu->motor_slow_run_flag = 0;
		Motor_Tilt2_ParaStu->motor_slow_run_flag = 0;
		if(2 == motor_cmd_change_fast)
		{
			if(motor_cmd_delay_time >= 60) //300ms
			{
				Motor_Para_AllReset();	
				motor_cmd_change_fast = 0;
			}
		}
	}
	else
	{
		motor_cmd_delay_time = 0;
		//根据指令控制电机	
		if(0 == motor_ctr_mode)
		{
			/*---------------------------------------正常控制动作-------------------------------------*/
			switch(motor_ctr_cmd)
			{
				case KEY_BACK_UP:
				{
					Command_Back_Run(KEY_BACK_UP);
				}break;
				case KEY_BACK_DOWN:
				{
					Command_Back_Run(KEY_BACK_DOWN);
				}break;
				case KEY_LEG_UP:
				{
					Command_Leg_Run(KEY_LEG_UP);
				}break;
				case KEY_LEG_DOWN:
				{
					Command_Leg_Run(KEY_LEG_DOWN);
				}break;
				case KEY_BACKLEG_UP:
				{
					Command_BackLeg_Run(KEY_BACKLEG_UP);
				}break;
				case KEY_BACKLEG_DOWN:
				{
					Command_BackLeg_Run(KEY_BACKLEG_DOWN);
				}break;
				case KEY_TILT_ALL_UP:
				{
					Command_Tilt_All_Run(KEY_TILT_ALL_UP);
				}break;
				case KEY_TILT_ALL_DOWN:
				{
					Command_Tilt_All_Run(KEY_TILT_ALL_DOWN);
				}break;
				case KEY_LUMBAR_UP:
				{
					Command_Lumbar_Run(KEY_LUMBAR_UP);
				}break;
				case KEY_LUMBAR_DOWN:
				{
					Command_Lumbar_Run(KEY_LUMBAR_DOWN);
				}break;
				case KEY_LUMBAR2_UP:
				{
					Command_Lumbar2_Run(KEY_LUMBAR2_UP);
				}break;
				case KEY_LUMBAR2_DOWN:
				{
					Command_Lumbar2_Run(KEY_LUMBAR2_DOWN);
				}break;				
				case KEY_NECK_UP:
				{
					Command_Neck_Run(KEY_NECK_UP);
				}break;
				case KEY_NECK_DOWN:
				{
					Command_Neck_Run(KEY_NECK_DOWN);
				}break;
				case KEY_NECK2_UP:
				{
					Command_Neck2_Run(KEY_NECK2_UP);
				}break;
				case KEY_NECK2_DOWN:
				{
					Command_Neck2_Run(KEY_NECK2_DOWN);
				}break;					
				case KEY_ALL_MOTOR_UP:
				{
					Command_All_Motor_Run(KEY_ALL_MOTOR_UP);
				}break;
				case KEY_ALL_MOTOR_DOWN:
				{
					Command_All_Motor_Run(KEY_ALL_MOTOR_DOWN);
				}break;		
				case KEY_FORWARD:
				{
					Command_Pitch_Run(KEY_FORWARD);
				}break;
				case KEY_BACKWARD:
				{
					Command_Pitch_Run(KEY_BACKWARD);
				}break;
				case KEY_TILT1_UP:
				{
					Command_Tilt1_Run(KEY_TILT1_UP);
				}break;
				case KEY_TILT1_DOWN:
				{
					Command_Tilt1_Run(KEY_TILT1_DOWN);
				}break;
				case KEY_TILT2_UP:
				{
					Command_Tilt2_Run(KEY_TILT2_UP);
				}break;
				case KEY_TILT2_DOWN:
				{
					Command_Tilt2_Run(KEY_TILT2_DOWN);
				}break;
				case KEY_ONE_CLICK_BACK_UP:
				{
					if(1 == Command_OnClick_Back_Up())
					{
						 motor_oneclick_cmd = 0;	
					}						
				}break;
				case KEY_ONE_CLICK_BACK_DOWN:
				{
					if(1 == Command_OnClick_Back_Down())
					{
						 motor_oneclick_cmd = 0;	
					}	
				}break;
				case KEY_ONE_CLICK_LEG_UP:
				{
					if(1 == Command_OnClick_Leg_Up())
					{
						 motor_oneclick_cmd = 0;	
					}
				}break;
				case KEY_ONE_CLICK_LEG_DOWN:
				{
					if(1 == Command_OnClick_Leg_Down())
					{
						 motor_oneclick_cmd = 0;	
					}	
				}break;
				case KEY_ONE_CLICK_BACKLEG_UP:
				{
					if(1 == Command_OnClick_BackLeg_Up())
					{
						 motor_oneclick_cmd = 0;	
					}	
				}break;
				case KEY_ONE_CLICK_BACKLEG_DOWN:
				{
					if(1 == Command_OnClick_BackLeg_Down())
					{
						 motor_oneclick_cmd = 0;	
					}	
				}break;
				case KEY_ONE_CLICK_LUMBAR_UP:
				{
					if(1 == Command_OnClick_Lumbar_Up())
					{
						 motor_oneclick_cmd = 0;	
					}
				}break;
				case KEY_ONE_CLICK_LUMBAR_DOWN:
				{
					if(1 == Command_OnClick_Lumbar_Down())
					{
						 motor_oneclick_cmd = 0;	
					}
				}break;
				case KEY_ONE_CLICK_LUMBAR2_UP:
				{
					if(1 == Command_OnClick_Lumbar2_Up())
					{
						 motor_oneclick_cmd = 0;	
					}
				}break;
				case KEY_ONE_CLICK_LUMBAR2_DOWN:
				{
					if(1 == Command_OnClick_Lumbar2_Down())
					{
						 motor_oneclick_cmd = 0;	
					}
				}break;
				case KEY_ONE_CLICK_NECK_UP:
				{
					if(1 == Command_OnClick_Neck_Up())
					{
						 motor_oneclick_cmd = 0;	
					}
				}break;
				case KEY_ONE_CLICK_NECK_DOWN:
				{
					if(1 == Command_OnClick_Neck_Down())
					{
						 motor_oneclick_cmd = 0;	
					}
				}break;
				case KEY_ONE_CLICK_NECK2_UP:
				{
					if(1 == Command_OnClick_Neck2_Up())
					{
						 motor_oneclick_cmd = 0;	
					}
				}break;
				case KEY_ONE_CLICK_NECK2_DOWN:
				{
					if(1 == Command_OnClick_Neck2_Down())
					{
						 motor_oneclick_cmd = 0;	
					}
				}break;
				case KEY_ONE_CLICK_LUMBAR_NECK_UP:
				{
					if(1 == Command_OnClick_LumbarNeck_Up())
					{
						 motor_oneclick_cmd = 0;	
					}
				}break;
				case KEY_ONE_CLICK_LUMBAR_NECK_DOWN:
				{
					if(1 == Command_OnClick_LumbarNeck_Down())
					{
						 motor_oneclick_cmd = 0;	
					}
				}break;
				case KEY_ONE_CLICK_ALL_UP:
				{
					if(1 == Command_OnClick_AllMotor_Up())
					{
						 motor_oneclick_cmd = 0;	
					}
				}break;
				case KEY_ONE_CLICK_ALL_DOWN:
				{
					if(1 == Command_OnClick_AllMotor_Down())
					{
						 motor_oneclick_cmd = 0;	
					}
				}break;
				case KEY_FLAT:
				{
				if(1 == Command_GoFlat_Shape())
				{
					motor_oneclick_cmd = 0;							
				}
				}break;		
				case KEY_GO_GETUP:
				{
					Command_GoGetUp_Shape();
				}break;
				case KEY_GO_NURSING:
				{
					Command_GoNursing_Shape();
				}break;
				case KEY_GO_M1:
				{
					Command_GoM1_Shape();
				}break;
				case KEY_GO_M2:
				{
					Command_GoM2_Shape();
				}break;
        		case KEY_GO_M3:
				{
					Command_GoM3_Shape();
				}break;			
				case KEY_GO_TV:
				{
					Command_GoTV_Shape();
				}break;
				case KEY_GO_ZEROG:
				{
					Command_GoZeroG_Shape();
				}break;
				case KEY_GO_LOUNGE:
				{
					Command_GoLounge_Shape();
				}break;
				case KEY_GO_SNORE:
				{
					if(HALF_SPEED_NO_FLAT == system_config.flags.snore_run_mode || HALF_SPEED_FLAT == system_config.flags.snore_run_mode)
					{
						Motor_Back_ParaStu->motor_slow_run_flag = 1;
						Motor_Leg_ParaStu->motor_slow_run_flag = 1;		
						Motor_Lumbar_ParaStu->motor_slow_run_flag = 1;	
						Motor_Neck_ParaStu->motor_slow_run_flag = 1;							
					}
					if(1 == Command_GoSnore_Shape())
					{
						motor_run_cmd_state = KEY_GO_SNORE;
					}
				}break;
				case KEY_GO_READ:
				{
					Command_GoRead_Shape();
				}break;
				case KEY_GO_YOGA:
				{
					Command_GoYoga_Shape();
				}break;
				case KEY_MOTOR_FOLLOW_UP:
				{
					Motor_Sys_Mode();
				}break;				
				case KEY_DEMO1_MODE:
				{
					if(system_config.flags.help_sleep_mode == HELP_SLEEP_MODE_ONE)
					{
						if(1 == Motor_Demo_Help_Sleep_Mode())
						{
							motor_oneclick_cmd = 0;
						}
					}
					else if(system_config.flags.help_sleep_mode == HELP_SLEEP_MODE_TWO)
					{
						if(1 == Motor_Demo2_Mode())
						{
							motor_oneclick_cmd = 0;
						}
					}
					else if(system_config.flags.help_sleep_mode == HELP_SLEEP_MODE_ZHUIMI)
					{
						if(1 == Motor_Demo_Help_Sleep_ZhuiMi_Mode())
						{
							motor_oneclick_cmd = 0;
						}
					}
					else if(system_config.flags.help_sleep_mode == HELP_SLEEP_MODE_TILT)
					{
						if(1 == Motor_Demo_Tilt_Mode())
						{
							motor_oneclick_cmd = 0;
						}
					}
					else
					{
						motor_oneclick_cmd = 0;
					}
				}break;
				case KEY_DEMO2_MODE:
				{
					if(system_config.flags.help_sleep_mode == HELP_SLEEP_MODE_TILT)
					{
						if(1 == Motor_Demo_Help_Sleep_Tilt_SSB_Mode())
						{
							motor_oneclick_cmd = 0;
						}
					}
					else
					{
						if(1 == Motor_Demo_Help_Sleep_SSB_Mode())
						{
							motor_oneclick_cmd = 0;
						}
					}
				}break;		
				case KEY_ALARM_MODE:
				{
					if(1 == Motor_Alarm_Mode())
					{
						motor_oneclick_cmd = 0;
					}
				}break;						
				case KEY_ALARM_THREE_MODE:
				{
					if(1 == Motor_AlarmThree_Mode())
					{
						motor_oneclick_cmd = 0;
					}
				}break;
				case KEY_ALARM_MSGR_MODE:
				{
					User_Msgr_Demo(1);
					if(alarm_msgr_running == 0)
					{
						motor_oneclick_cmd  = 0;
					}
				}break;						
				case KEY_NO: //记忆马达位置  清标志位				
				default:
				{
					motor_need_click_again = 0;
					if(Motor_SyncTarget_HallArr[1] != 0x0000)
					{
						motor_run_fixed_complate = 0;
						if(!Motor_ArrivePosition(&Motor1_ParaStu,Motor_SyncTarget_HallArr[1]))
						{
							Motor_SyncTarget_HallArr[1] = 0x0000;
						}
					}
					else
					{
						Motor_PwmImStop(&Motor1_ParaStu);      
					}
					//
					if(Motor_SyncTarget_HallArr[2] != 0x0000)
					{
						motor_run_fixed_complate = 0;
						if(!Motor_ArrivePosition(&Motor2_ParaStu,Motor_SyncTarget_HallArr[2]))
						{
							Motor_SyncTarget_HallArr[2] = 0x0000;
						}
					}
					else
					{
						Motor_PwmImStop(&Motor2_ParaStu);      
					}
					//
					if(Motor_SyncTarget_HallArr[3] != 0x0000)
					{
						motor_run_fixed_complate = 0;					
						if(!Motor_ArrivePosition(&Motor3_ParaStu,Motor_SyncTarget_HallArr[3]))
						{
							Motor_SyncTarget_HallArr[3] = 0x0000;
						}
					}
					else
					{
						Motor_PwmImStop(&Motor3_ParaStu);      
					}
					//
					if(Motor_SyncTarget_HallArr[4] != 0x0000)
					{
						motor_run_fixed_complate = 0;
						if(!Motor_ArrivePosition(&Motor4_ParaStu,Motor_SyncTarget_HallArr[4]))
						{
							Motor_SyncTarget_HallArr[4] = 0x0000;
						}
					}
					else
					{
						Motor_PwmImStop(&Motor4_ParaStu);      
					}
					if(Motor_SyncTarget_HallArr[5] != 0x0000)
					{
						motor_run_fixed_complate = 0;
						if(!Motor_ArrivePosition(&Motor5_ParaStu,Motor_SyncTarget_HallArr[5]))
						{
							Motor_SyncTarget_HallArr[5] = 0x0000;
						}
					}
					else
					{
						Motor_PwmImStop(&Motor5_ParaStu);  
											
					}		
					if(Motor_SyncTarget_HallArr[6] != 0x0000)
					{
						motor_run_fixed_complate = 0;
						if(!Motor_ArrivePosition(&Motor6_ParaStu,Motor_SyncTarget_HallArr[6]))
						{
							Motor_SyncTarget_HallArr[6] = 0x0000;
						}
					}
					else
					{
						Motor_PwmImStop(&Motor6_ParaStu);  
											
					}						
				}break;
			}		
		}
	}
	//如果电机运行至snore并且snore定时复位开启后，定时复位
	if(KEY_GO_SNORE == motor_run_cmd_state && (0 == Get_Motor_PortState()) && (ALL_SPEED_FLAT == system_config.flags.snore_run_mode || HALF_SPEED_FLAT == system_config.flags.snore_run_mode))
	{
		motor_snore_ms_time ++;
		if(motor_snore_ms_time >= 200)
		{
			motor_snore_ms_time = 0;
			motor_snore_sec_time ++;
			if(motor_snore_sec_time >= 5)
			{
				motor_snore_sec_time = 0;
				if(HALF_SPEED_FLAT == system_config.flags.snore_run_mode)
				{
					Motor_Back_ParaStu->motor_slow_run_flag = 	1;
					Motor_Leg_ParaStu->motor_slow_run_flag = 	1;
					Motor_Lumbar_ParaStu->motor_slow_run_flag = 1;
					Motor_Neck_ParaStu->motor_slow_run_flag = 	1;
					Motor_Lumbar2_ParaStu->motor_slow_run_flag = 1;
					Motor_Neck2_ParaStu->motor_slow_run_flag = 	1;
	
				}
				motor_oneclick_cmd = KEY_FLAT;
			}
		}
	}
	else
	{
		motor_snore_ms_time = 0;
		motor_snore_sec_time = 0;
	}
	//调用电机底层驱动
	Motor_PwmChange(&Motor1_ParaStu);
	Motor_PwmChange(&Motor2_ParaStu);
	Motor_PwmChange(&Motor3_ParaStu);
	Motor_PwmChange(&Motor4_ParaStu);	
	Motor_PwmChange(&Motor5_ParaStu);	
	Motor_PwmChange(&Motor6_ParaStu);	
	Motor_Run(&Motor1_ParaStu);
	Motor_Run(&Motor2_ParaStu);
	Motor_Run(&Motor3_ParaStu);
	Motor_Run(&Motor4_ParaStu);
	Motor_Run(&Motor5_ParaStu);	
	Motor_Run(&Motor6_ParaStu);	
}
//上电复位
void Command_PowerOn_Reset(void)
{
	Motor_Para_AllReset();
	motor_ctr_cmd = KEY_FLAT;
	Delay_Ms(100);
	while(1)
	{
		Iwdg_Clear();
		if(0 == Get_Motor_PortState())
		{
			break;
		}
		Delay_Ms(10);
	}
	motor_ctr_cmd = 0;
}
//电机动作设置
void Motor_OneClickCmd_Set(unsigned char cmd_temp)
{
	motor_oneclick_cmd = cmd_temp;
}

//获取当前电机动作
unsigned char Motor_Run_Cmd(void)
{
	return motor_ctr_cmd;
}
//判断电机同步是否完成
unsigned char Motor_Sync_Complate(void)
{
	if(Motor_SyncTarget_HallArr[1] != 0 || \
		 Motor_SyncTarget_HallArr[2] != 0 || \
	   Motor_SyncTarget_HallArr[3] != 0 || \
		 Motor_SyncTarget_HallArr[4] != 0 || \
		 Motor_SyncTarget_HallArr[5] != 0 || \
		 Motor_SyncTarget_HallArr[6] != 0 
		)
	{
		return 0;
	}
	
	return 1;
}
extern uint8_t back_cmd;
extern uint8_t leg_cmd;
#define MOTOR_SYS_CMD_VALID_TIME_TICK_5MS (600U) // 3s
unsigned char Motor_Sys_Mode(void)
{
  unsigned char motor_complete_flag = 0;
	static uint8_t old_back_cmd = 0;
	static uint8_t active_back_cmd = 0;
	static uint8_t pending_back_cmd = 0;
	static uint16_t back_cmd_valid_cnt = 0;
	static uint8_t stop_before_switch_cnt = 0; // 5ms 基准，10=50ms
	static uint8_t switch_block_cnt = 0;       // 5ms 基准，60=300ms
	static uint8_t old_leg_cmd = 0;
	static uint8_t active_leg_cmd = 0;
	static uint8_t pending_leg_cmd = 0;
	static uint16_t leg_cmd_valid_cnt = 0;
	static uint8_t leg_stop_before_switch_cnt = 0; // 5ms 基准，10=50ms
	static uint8_t leg_switch_block_cnt = 0;       // 5ms 基准，60=300ms  

	motor_demo_ms_time ++ ;
  if(motor_demo_ms_time == 200)
  {
    motor_demo_ms_time = 0;
    motor_demo_sec_time ++;
    if(motor_demo_sec_time >= 480)
    {
      motor_demo_sec_time = 480;
    }
  }
	if(motor_demo_step == 0)
	{
			//打开RGB灯呼吸模式
			Light_RgbColour_Stu.light_colour_mode[MODE_TYPE_BIT] = 2;
			Light_RgbColour_Stu.light_colour_mode[MODE_ORDER_BIT] = 1;
			//触发模式事件
			light_para_set_event |= LIGHT_UBL_MODE_EVENT;	  
			motor_demo_step++;

	}
	if(motor_demo_step == 1)
	{
		if(system_config.flags.motor_follow_run_tilt_motor == RUN_TILT_MOTOR_DISABLE)
		{
			//停止背腿电机
			Motor_PwmImStop(Motor_Back_ParaStu);
			Motor_PwmImStop(Motor_Leg_ParaStu);
			//复位背腿电机参数
			Motor_Para_Reset(Motor_Back_ParaStu);
			Motor_Para_Reset(Motor_Leg_ParaStu);
		}
		else
		{
			//停止升降电机
			Motor_PwmImStop(Motor_Tilt1_ParaStu);
			Motor_PwmImStop(Motor_Tilt2_ParaStu);
			//复位升降电机参数
			Motor_Para_Reset(Motor_Tilt1_ParaStu);
			Motor_Para_Reset(Motor_Tilt2_ParaStu);			
		}
		motor_demo_step++;
	}
  if(motor_demo_step == 2)
  {
		if(system_config.flags.motor_follow_run_tilt_motor == RUN_TILT_MOTOR_DISABLE)
		{
			//第一步先跑到一个中间位置
			if(!Motor_ArrivePosition(Motor_Back_ParaStu,BACK_FOLLOW_MID_HALL_NUM))
			{
				motor_complete_flag |= 0x01;
			}		
			if(!Motor_ArrivePosition(Motor_Leg_ParaStu,LEG_FOLLOW_MID_HALL_NUM))
			{
				motor_complete_flag |= 0x02;
			}
		}
		else
		{
			//第一步先跑到一个中间位置
			if(!Motor_ArrivePosition(Motor_Tilt1_ParaStu,TILT1_FOLLOW_MID_HALL_NUM))
			{
				motor_complete_flag |= 0x01;
			}		
			if(!Motor_ArrivePosition(Motor_Tilt2_ParaStu,TILT2_FOLLOW_MID_HALL_NUM))
			{
				motor_complete_flag |= 0x02;
			}			
		}
    if((motor_complete_flag & 0x03) == 0x03)
    {
      motor_demo_step++;
      motor_complete_flag = 0;
      motor_demo_ms_time = 0;
      motor_demo_sec_time = 0;
			//参数太多，清理一下
			back_cmd = 0;
			leg_cmd = 0;
			old_back_cmd = 0;
			active_back_cmd = 0;
			pending_back_cmd = 0;
			back_cmd_valid_cnt = 0;
			stop_before_switch_cnt = 0; // 5ms 基准，10=50ms
			switch_block_cnt = 0;       // 5ms 基准，60=300ms
	    old_leg_cmd = 0;
	    active_leg_cmd = 0;
	    pending_leg_cmd = 0;
	    leg_cmd_valid_cnt = 0;
	    leg_stop_before_switch_cnt = 0; // 5ms 基准，10=50ms
	    leg_switch_block_cnt = 0;       // 5ms 基准，60=300ms  
    }
  }
  if(motor_demo_step == 3)
  {
		if(back_cmd != 0)
		{
			if(back_cmd_valid_cnt < MOTOR_SYS_CMD_VALID_TIME_TICK_5MS)
			{
				back_cmd_valid_cnt++;
			}
			else
			{
				back_cmd = 0;
				back_cmd_valid_cnt = 0;
			}
		}
		else
		{
			back_cmd_valid_cnt = 0;
		}

		if(leg_cmd != 0)
		{
			if(leg_cmd_valid_cnt < MOTOR_SYS_CMD_VALID_TIME_TICK_5MS)
			{
				leg_cmd_valid_cnt++;
			}
			else
			{
				leg_cmd = 0;
				leg_cmd_valid_cnt = 0;
			}
		}
		else
		{
			leg_cmd_valid_cnt = 0;
		}

		if(old_back_cmd != back_cmd)
		{
			old_back_cmd = back_cmd;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;		
		}

		if(switch_block_cnt > 0)
		{
			switch_block_cnt--;
		}

		if(stop_before_switch_cnt > 0)
		{
			if(system_config.flags.motor_follow_run_tilt_motor == RUN_TILT_MOTOR_DISABLE)
			{
				// 换向前先停机 50ms
				Motor_PwmImStop(Motor_Back_ParaStu);
				Motor_Para_Reset(Motor_Back_ParaStu);
			}
			else
			{
				// 换向前先停机 50ms
				Motor_PwmImStop(Motor_Tilt1_ParaStu);
				Motor_Para_Reset(Motor_Tilt1_ParaStu);
			}
			if(back_cmd == 0)
			{
				pending_back_cmd = 0;
				active_back_cmd = 0;
				stop_before_switch_cnt = 0;
			}
			else
			{
				pending_back_cmd = back_cmd;
				stop_before_switch_cnt--;
				if(stop_before_switch_cnt == 0)
				{
					active_back_cmd = pending_back_cmd;
					pending_back_cmd = 0;
					if(active_back_cmd != 0)
					{
						switch_block_cnt = 60;
					}
				}
			}
		}
		else
		{
			if(back_cmd == 0)
			{
				active_back_cmd = 0;
				pending_back_cmd = 0;
			}
			else if(active_back_cmd == 0)
			{
				active_back_cmd = back_cmd;
				switch_block_cnt = 60;
			}
			else if(back_cmd != active_back_cmd)
			{
				// 运行后 300ms 内屏蔽方向切换
				if(switch_block_cnt == 0)
				{
					pending_back_cmd = back_cmd;
					active_back_cmd = 0;
					stop_before_switch_cnt = 10;
				}
			}
		}

		if(active_back_cmd == 0)
		{
			if(system_config.flags.motor_follow_run_tilt_motor == RUN_TILT_MOTOR_DISABLE)
			{
				//停止
				Motor_PwmImStop(Motor_Back_ParaStu);
				Motor_Para_Reset(Motor_Back_ParaStu);
			}
			else
			{
				//停止
				Motor_PwmImStop(Motor_Tilt1_ParaStu);
				Motor_Para_Reset(Motor_Tilt1_ParaStu);
			}
			motor_complete_flag &= ~0x01;
		}
		else if(active_back_cmd == 1)
		{
			if(system_config.flags.motor_follow_run_tilt_motor == RUN_TILT_MOTOR_DISABLE)
			{
				//上升
				if(!Motor_ArrivePosition(Motor_Back_ParaStu,BACK_FOLLOW_HIGH_HALL_NUM))
				{
					motor_complete_flag |= 0x01;
				}
			}
			else
			{
				//上升
				if(!Motor_ArrivePosition(Motor_Tilt1_ParaStu,TILT1_FOLLOW_HIGH_HALL_NUM))
				{
					motor_complete_flag |= 0x01;
				}				
			}
		}
		else if(active_back_cmd == 2)
		{
			if(system_config.flags.motor_follow_run_tilt_motor == RUN_TILT_MOTOR_DISABLE)
			{
				//下降
				if(!Motor_ArrivePosition(Motor_Back_ParaStu,BACK_FOLLOW_LOW_HALL_NUM))
				{
					motor_complete_flag |= 0x01;
				}
			}
			else
			{
				//下降
				if(!Motor_ArrivePosition(Motor_Tilt1_ParaStu,TILT1_FOLLOW_LOW_HALL_NUM))
				{
					motor_complete_flag |= 0x01;
				}				
			}
		}

		if(old_leg_cmd != leg_cmd)
		{
			old_leg_cmd = leg_cmd;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;		
		}

		if(leg_switch_block_cnt > 0)
		{
			leg_switch_block_cnt--;
		}

		if(leg_stop_before_switch_cnt > 0)
		{
			if(system_config.flags.motor_follow_run_tilt_motor == RUN_TILT_MOTOR_DISABLE)
			{
				// 换向前先停机 50ms
				Motor_PwmImStop(Motor_Leg_ParaStu);
				Motor_Para_Reset(Motor_Leg_ParaStu);
			}
			else
			{
				// 换向前先停机 50ms
				Motor_PwmImStop(Motor_Tilt2_ParaStu);
				Motor_Para_Reset(Motor_Tilt2_ParaStu);
			}
			if(leg_cmd == 0)
			{
				pending_leg_cmd = 0;
				active_leg_cmd = 0;
				leg_stop_before_switch_cnt = 0;
			}
			else
			{
				pending_leg_cmd = leg_cmd;
				leg_stop_before_switch_cnt--;
				if(leg_stop_before_switch_cnt == 0)
				{
					active_leg_cmd = pending_leg_cmd;
					pending_leg_cmd = 0;
					if(active_leg_cmd != 0)
					{
						leg_switch_block_cnt = 60;
					}
				}
			}
		}
		else
		{
			if(leg_cmd == 0)
			{
				active_leg_cmd = 0;
				pending_leg_cmd = 0;
			}
			else if(active_leg_cmd == 0)
			{
				active_leg_cmd = leg_cmd;
				leg_switch_block_cnt = 60;
			}
			else if(leg_cmd != active_leg_cmd)
			{
				// 运行后 300ms 内屏蔽方向切换
				if(leg_switch_block_cnt == 0)
				{
					pending_leg_cmd = leg_cmd;
					active_leg_cmd = 0;
					leg_stop_before_switch_cnt = 10;
				}
			}
		}

		if(active_leg_cmd == 0)
		{
			if(system_config.flags.motor_follow_run_tilt_motor == RUN_TILT_MOTOR_DISABLE)
			{
				//停止
				Motor_PwmImStop(Motor_Leg_ParaStu);
				Motor_Para_Reset(Motor_Leg_ParaStu);
			}
			else
			{
				//停止
				Motor_PwmImStop(Motor_Tilt2_ParaStu);
				Motor_Para_Reset(Motor_Tilt2_ParaStu);
			}
			motor_complete_flag &= ~0x02;
		}
		else if(active_leg_cmd == 1)
		{
			if(system_config.flags.motor_follow_run_tilt_motor == RUN_TILT_MOTOR_DISABLE)
			{
				//上升
				if(!Motor_ArrivePosition(Motor_Leg_ParaStu,LEG_FOLLOW_HIGH_HALL_NUM))
				{
					motor_complete_flag |= 0x02;
				}
			}
			else
			{
				//上升
				if(!Motor_ArrivePosition(Motor_Tilt2_ParaStu,TILT2_FOLLOW_HIGH_HALL_NUM))
				{
					motor_complete_flag |= 0x02;
				}				
			}
		}
		else if(active_leg_cmd == 2)
		{
			if(system_config.flags.motor_follow_run_tilt_motor == RUN_TILT_MOTOR_DISABLE)
			{
				//下降
				if(!Motor_ArrivePosition(Motor_Leg_ParaStu,LEG_FOLLOW_LOW_HALL_NUM))
				{
					motor_complete_flag |= 0x02;
				}
			}
			else
			{
				//下降
				if(!Motor_ArrivePosition(Motor_Tilt2_ParaStu,TILT2_FOLLOW_LOW_HALL_NUM))
				{
					motor_complete_flag |= 0x02;
				}				
			}
		}
		if(motor_complete_flag != 0)
		{
			motor_demo_step = 1;
			motor_complete_flag = 0;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
		}
  }
	return 0;
}
// 1.背部腿部电机原速度同时运行, 背部15度（33）, 腿部到达35°（102）背部和腿部电机都停止后, 计时5S（DEMO_STOP_TIME）; 
// 2.背部腿部电机同时运行，腿部电机50%速度下降至22°（55）停止（若背部电机先停止则腿部无需到22°就停止）；背部电机50%速度上升至20度（46）后, 停止, 计时2S
// 3.计时完成后,背部腿部电机同时运行，腿部电机50%速度上升至35°（102）后停止（若背部电机先停止则腿部无需到35°就停止）;背部电机50%速度下降至12度（26）后,停止, 计时2S;
// 4.第(2)和(3)步为1个循环, 连续重复执行25个循环后, 背部腿部电机同时运行，背部电机50%速度下降/上升至15°(33)后停止；腿部电机50%速度下降/上升至35°(102)后停止, 计时120S;
// 5. 计时完成后, 背部电机先40%速度下降至10°(21)后停止, 腿部电机再40%速度下降至15°(33)后停止, 计时150S;
// 6. 计时完成后, 背部电机30%速度下降至0°后停止, 腿部电机30%速度下降0°后停止

unsigned char Motor_Demo_Help_Sleep_Mode(void)
{
	unsigned char motor_complete_flag = 0;
	unsigned char motor_fast_change_flag = 0;

	motor_demo_ms_time ++ ;
	if(motor_demo_ms_time >= 200)
	{
		motor_demo_ms_time = 0;
		motor_demo_sec_time ++;
		if(motor_demo_sec_time >= 480)
		{
			motor_demo_sec_time = 480;
		}
	}
	motor_demo_5ms_cont++;
	if(motor_demo_5ms_cont >= 200)
	{
		motor_demo_5ms_cont = 0;
		motor_demo_1sec_cont++;
	}
	Msgr_Clear_TimeCount();
	Light_Clear_TimeCount(&Light_RgbColour_Stu);	
	// 更新步骤逻辑
	if(motor_demo_slave_flag == 1)
	{
		if((lin_received_step != motor_demo_step) && (lin_received_step != 0))
		{
				// 当前步骤未完成就先停止电机
			Motor_PwmImStop(Motor_Back_ParaStu);
			Motor_PwmImStop(Motor_Leg_ParaStu);
			Motor_PwmImStop(Motor_Lumbar_ParaStu);
			Motor_PwmImStop(Motor_Lumbar2_ParaStu);
			Motor_PwmImStop(Motor_Neck_ParaStu);
			Motor_PwmImStop(Motor_Neck2_ParaStu);
			// 再进入下一步
			motor_demo_step = lin_received_step;
			motor_fast_change_flag = 1;
			lin_received_step = 0;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
		}
	}	
	if(motor_demo_step == 0)
	{
		motor_demo_loop_count = 0;
		motor_demo_5ms_cont = 0;
		motor_demo_1sec_cont = 0;
		Motor_PwmImStop(Motor_Back_ParaStu);
		Motor_PwmImStop(Motor_Leg_ParaStu);
		Motor_PwmImStop(Motor_Lumbar_ParaStu);
		Motor_PwmImStop(Motor_Lumbar2_ParaStu);
		Motor_PwmImStop(Motor_Neck_ParaStu);
		Motor_PwmImStop(Motor_Neck2_ParaStu);
		Motor_PwmImStop(Motor_Tilt1_ParaStu);
		Motor_PwmImStop(Motor_Tilt2_ParaStu);
		Light_OneColour_Stu.led_colour_state = 0;
		led_board_state = 0;		
		Music_SW(SWITCH_CTR_OFF);//音响关闭
		if(User_Msgr_Demo(0) == 1)
		{
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;	
		}		
	}	
	if(motor_demo_step == 1)
	{
		if(User_MusicDemo(0) == 1)
		{
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;	
		}		
	}	
	if(motor_demo_step == 2)
	{
		if(User_LightDemo(0) == 1)
		{
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;	
		}		
	}	
	
	if(motor_demo_step == 3)
	{
		if(!Motor_ArrivePosition(Motor_Lumbar_ParaStu,HALL_MIN_NUM))
		{
			Motor_Para_Reset(Motor_Lumbar_ParaStu);
			motor_complete_flag |= 0x01;
		}
		if(!Motor_ArrivePosition(Motor_Lumbar2_ParaStu,HALL_MIN_NUM))
		{
			Motor_Para_Reset(Motor_Lumbar2_ParaStu);
			motor_complete_flag |= 0x02;
		}
		if(!Motor_ArrivePosition(Motor_Neck_ParaStu,HALL_MIN_NUM))
		{
			Motor_Para_Reset(Motor_Neck_ParaStu);
			motor_complete_flag |= 0x04;
		}
		if(!Motor_ArrivePosition(Motor_Neck2_ParaStu,HALL_MIN_NUM))
		{
			Motor_Para_Reset(Motor_Neck2_ParaStu);
			motor_complete_flag |= 0x08;
		}
		if(!Motor_ArrivePosition(Motor_Back_ParaStu,HALL_MIN_NUM))
		{
			Motor_Para_Reset(Motor_Back_ParaStu);
			motor_complete_flag |= 0x10;
		}			
		if(!Motor_ArrivePosition(Motor_Leg_ParaStu,HALL_MIN_NUM))
		{
			Motor_Para_Reset(Motor_Leg_ParaStu);
			motor_complete_flag |= 0x20;
		}		
		
		if((motor_complete_flag & 0x3f) == 0x3f)
		{
			motor_complete_flag = 0x00;
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
		}
	}
	if(motor_demo_step == 4)
	{
		if(motor_demo_sec_time >= 1)
		{
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;		
			if(motor_demo_slave_flag == 0)
			{
				motor_demo_step++;
				LIN_Master_Send_DemoStep_Cmd(2, motor_demo_step);
			}	
		}
	}	
	if(motor_demo_step == 5)//原速度同时运行, 背部15度, 腿部到达35°背部和腿部电机都停止后, 计时5S; 
	{
		if(motor_fast_change_flag)
		{
			Motor_PwmImStop(Motor_Back_ParaStu);
			Motor_PwmImStop(Motor_Leg_ParaStu);
			Motor_PwmImStop(Motor_Lumbar_ParaStu);
			Motor_PwmImStop(Motor_Lumbar2_ParaStu);
			Motor_PwmImStop(Motor_Neck_ParaStu);
			Motor_PwmImStop(Motor_Neck2_ParaStu);
			motor_fast_change_flag = 0;		
			Motor_Para_Reset(Motor_Back_ParaStu);
			Motor_Para_Reset(Motor_Leg_ParaStu);
			Motor_Para_Reset(Motor_Lumbar_ParaStu);
			Motor_Para_Reset(Motor_Lumbar2_ParaStu);
			Motor_Para_Reset(Motor_Neck_ParaStu);
			Motor_Para_Reset(Motor_Neck2_ParaStu);
		}
		else
		{
			if(!Motor_ArrivePosition(Motor_Back_ParaStu,BACK_15DU_HALL_NUM))
			{
				motor_complete_flag |= 0x01;
			} 
			if(!Motor_ArrivePosition(Motor_Leg_ParaStu,LEG_35DU_HALL_NUM))
			{
				motor_complete_flag |= 0x02;
			}
			if((motor_complete_flag & 0x03) == 0x03)
			{
				motor_demo_step++;
				motor_complete_flag = 0;
				motor_demo_ms_time = 0;
				motor_demo_sec_time = 0;
			}
		}
	}
	if(motor_demo_step == 6)
	{
		if(motor_demo_sec_time >= DEMO_STOP_TIME)
		{
			motor_demo_step++;
		}
	}
	if(motor_demo_step == 7)
	{
		motor_demo_ms_time = 0;
		motor_demo_sec_time = 0;		
		if(motor_demo_slave_flag == 0)
		{
			motor_demo_step++;
			LIN_Master_Send_DemoStep_Cmd(2, motor_demo_step);
		}	
	}		
		//.背部腿部电机同时运行，腿部电机50%速度下降至22°停止（若背部电机先停止则腿部无需到22°就停止）；背部电机50%速度上升至20度后, 停止, 计时2S
	if(motor_demo_step == 8)
	{
		if(motor_fast_change_flag)
		{
			Motor_PwmImStop(Motor_Back_ParaStu);
			Motor_PwmImStop(Motor_Leg_ParaStu);
			motor_fast_change_flag = 0;		
			Motor_Para_Reset(Motor_Back_ParaStu);
			Motor_Para_Reset(Motor_Leg_ParaStu);
		}
		else		
		{		
			Motor_Back_ParaStu->motor_slow_run_flag = 1;
			Motor_Leg_ParaStu->motor_slow_run_flag = 1;
			if(!Motor_ArrivePosition(Motor_Back_ParaStu,BACK_20DU_HALL_NUM))
			{
				motor_complete_flag |= 0x01;
			} 
			if((!Motor_ArrivePosition(Motor_Leg_ParaStu,LEG_22DU_HALL_NUM)) || (motor_complete_flag == 0x01))
			{
				Motor_PwmImStop(Motor_Leg_ParaStu);
				motor_complete_flag |= 0x02;
			}
			if(motor_complete_flag == 3)
			{
				motor_complete_flag = 0x00;
				motor_demo_step++;
				motor_demo_ms_time = 0;
				motor_demo_sec_time = 0;
			}
		}
	}
	if(motor_demo_step == 9)
	{
		if(motor_demo_sec_time >= 3)
		{
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
		}
	}
	if(motor_demo_step == 10)
	{
		motor_demo_ms_time = 0;
		motor_demo_sec_time = 0;		
		if(motor_demo_slave_flag == 0)
		{
			motor_demo_step++;
			LIN_Master_Send_DemoStep_Cmd(2, motor_demo_step);
		}	
	}		
		//计时完成后,背部腿部电机同时运行，腿部电机50%速度上升至35°后停止（若背部电机先停止则腿部无需到35°就停止）;
	//背部电机50%速度下降至12度后,停止, 计时2S;
	if(motor_demo_step == 11)
	{
		if(motor_fast_change_flag)
		{
			Motor_PwmImStop(Motor_Back_ParaStu);
			Motor_PwmImStop(Motor_Leg_ParaStu);
			motor_fast_change_flag = 0;		
			Motor_Para_Reset(Motor_Back_ParaStu);
			Motor_Para_Reset(Motor_Leg_ParaStu);
		}
		else
		{		
			Motor_Back_ParaStu->motor_slow_run_flag = 1;
			Motor_Leg_ParaStu->motor_slow_run_flag = 1;		
			if(!Motor_ArrivePosition(Motor_Back_ParaStu,BACK_12DU_HALL_NUM))
			{
				motor_complete_flag |= 0x01;
			} 
			if((!Motor_ArrivePosition(Motor_Leg_ParaStu,LEG_35DU_HALL_NUM)) || (motor_complete_flag == 0x01))
			{
				motor_complete_flag |= 0x02;
				Motor_PwmImStop(Motor_Leg_ParaStu);			
			}
			if(motor_complete_flag == 3)
			{
				motor_complete_flag = 0x00;
				motor_demo_step++;
				motor_demo_ms_time = 0;
				motor_demo_sec_time = 0;
			}
		}
	}	
	if(motor_demo_step == 12)
	{
		if(motor_demo_sec_time >= 3)
		{
			motor_demo_step++;
		}
	}		
	if(motor_demo_step == 13)
	{	
		motor_demo_loop_count ++;
		if(motor_demo_loop_count <= 25)
		{
			motor_demo_step = 7;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
		}
		else
		{
			motor_demo_loop_count = 0;
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
		}
	}
	if(motor_demo_step == 14)
	{
		motor_demo_ms_time = 0;
		motor_demo_sec_time = 0;		
		if(motor_demo_slave_flag == 0)
		{
			motor_demo_step++;
			LIN_Master_Send_DemoStep_Cmd(2, motor_demo_step);
		}	
	}		
	if(motor_demo_step == 15)
	{
		if(motor_fast_change_flag)
		{
			Motor_PwmImStop(Motor_Back_ParaStu);
			Motor_PwmImStop(Motor_Leg_ParaStu);
			motor_fast_change_flag = 0;		
			Motor_Para_Reset(Motor_Back_ParaStu);
			Motor_Para_Reset(Motor_Leg_ParaStu);
		}
		else
		{		
			Motor_Back_ParaStu->motor_slow_run_flag = 1;
			Motor_Leg_ParaStu->motor_slow_run_flag = 1;		
			if(!Motor_ArrivePosition(Motor_Back_ParaStu,BACK_15DU_HALL_NUM))
			{
				motor_complete_flag |= 0x01;
			}
			if(!Motor_ArrivePosition(Motor_Leg_ParaStu,LEG_35DU_HALL_NUM))
			{
				motor_complete_flag |= 0X02;
			}
			if(motor_complete_flag == 0x03) 
			{
				motor_demo_step++;
				motor_complete_flag = 0;
			}
		}
	}
	if(motor_demo_step == 16)
	{
		if(motor_demo_sec_time >= 120)//计时120S
		{
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
			motor_demo_step++;
		}
	}
	if(motor_demo_step == 17)
	{
		motor_demo_ms_time = 0;
		motor_demo_sec_time = 0;		
		if(motor_demo_slave_flag == 0)
		{
			motor_demo_step++;
			LIN_Master_Send_DemoStep_Cmd(2, motor_demo_step);
		}	
	}			
	if(motor_demo_step == 18)
	{
		if(motor_fast_change_flag)
		{
			Motor_PwmImStop(Motor_Back_ParaStu);
			Motor_PwmImStop(Motor_Leg_ParaStu);
			motor_fast_change_flag = 0;		
			Motor_Para_Reset(Motor_Back_ParaStu);
			Motor_Para_Reset(Motor_Leg_ParaStu);
		}
		else
		{		
			Motor_Back_ParaStu->motor_slow_run_flag = 1;
			Motor_Leg_ParaStu->motor_slow_run_flag = 1;			
			if(!Motor_ArrivePosition(Motor_Back_ParaStu,BACK_10DU_HALL_NUM))
			{
				motor_complete_flag |= 0x01;
			}
			if(motor_complete_flag == 0x01)
			{
				if(!Motor_ArrivePosition(Motor_Leg_ParaStu,LEG_15DU_HALL_NUM))
				{
					motor_complete_flag = 0;
					motor_demo_step++;
				}
			}
		}
	}
	if(motor_demo_step == 19)
	{
		if(motor_demo_sec_time >= 150)
		{
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
			motor_demo_step++;
		}
	}
	if(motor_demo_step == 20)
	{
		motor_demo_ms_time = 0;
		motor_demo_sec_time = 0;		
		if(motor_demo_slave_flag == 0)
		{
			motor_demo_step++;
			LIN_Master_Send_DemoStep_Cmd(2, motor_demo_step);
		}	
	}			
	if(motor_demo_step == 21)
	{
		if(motor_fast_change_flag)
		{
			Motor_PwmImStop(Motor_Back_ParaStu);
			Motor_PwmImStop(Motor_Leg_ParaStu);
			motor_fast_change_flag = 0;		
			Motor_Para_Reset(Motor_Back_ParaStu);
			Motor_Para_Reset(Motor_Leg_ParaStu);
		}
		else
		{		
			Motor_Back_ParaStu->motor_slow_run_flag = 1;
			Motor_Leg_ParaStu->motor_slow_run_flag = 1;					
			if(!Motor_ArrivePosition(Motor_Back_ParaStu,HALL_MIN_NUM))
			{
				motor_complete_flag |= 0x01;
			}
			if(motor_complete_flag == 0x01)
			{
				if(!Motor_ArrivePosition(Motor_Leg_ParaStu,HALL_MIN_NUM))
				{
					motor_complete_flag = 0x00;
					motor_demo_ms_time = 0;
					motor_demo_sec_time = 0;
					motor_demo_step = 0;
					lin_received_step = 0;
					return 1;
				}
			}
		}
	}

	return 0;
}
// 追觅哄睡模式逻辑说明:
// 第一阶段: 电机到(41mm,27mm), 摇摆(41mm,27mm)?(28mm,38mm), 2档CONSTANT, 循环7次, 最后到(38mm,31mm), 全程慢速2-3mm/s
// 第二阶段: 摇摆(38mm,31mm)?(28mm,46mm), 1档CONSTANT, 循环14次, 每次循环后休息5s, 最后到(37mm,32mm)
//           头部半速, 脚部正常速追上后半速 (头高脚低→头低脚高→头高脚低=1次循环)
// 第三阶段: 摇摆(37mm,32mm)?(29mm,42mm), 1档CONSTANT, 循环12.5次, 每次循环后休息5s, 最后到(29mm,42mm)
//           头部半速, 脚部正常速追上后半速, 按摩器停止
// 第四阶段: (29mm,42mm)→(24mm,27mm)→(24mm,16mm)→(13mm,16mm)→(13mm,0mm)→(0mm,0mm), 全程慢速2-3mm/s
// 霍尔值计算: hall = 145 * mm + HALL_MIN_NUM

unsigned char Motor_Demo_Help_Sleep_ZhuiMi_Mode(void)
{
	unsigned char motor_complete_flag = 0;
	unsigned char motor_fast_change_flag = 0;

	motor_demo_ms_time ++ ;
	if(motor_demo_ms_time >= 200)
	{
		motor_demo_ms_time = 0;
		motor_demo_sec_time ++;
		if(motor_demo_sec_time >= 480)
		{
			motor_demo_sec_time = 480;
		}
	}
	motor_demo_5ms_cont++;
	if(motor_demo_5ms_cont >= 200)
	{
		motor_demo_5ms_cont = 0;
		motor_demo_1sec_cont++;
	}
	Msgr_Clear_TimeCount();
	Light_Clear_TimeCount(&Light_RgbColour_Stu);	
	// 更新步骤逻辑
	if(motor_demo_slave_flag == 1)
	{
		if((lin_received_step != motor_demo_step) && (lin_received_step != 0))
		{
			// 当前步骤未完成就先停止电机
			Motor_PwmImStop(Motor_Back_ParaStu);
			Motor_PwmImStop(Motor_Leg_ParaStu);
			Motor_PwmImStop(Motor_Lumbar_ParaStu);
			Motor_PwmImStop(Motor_Lumbar2_ParaStu);
			Motor_PwmImStop(Motor_Neck_ParaStu);
			Motor_PwmImStop(Motor_Neck2_ParaStu);
			// 再进入下一步
			motor_demo_step = lin_received_step;
			motor_fast_change_flag = 1;
			lin_received_step = 0;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
		}
	}	
	if(motor_demo_step == 0)
	{
		motor_demo_loop_count = 0;
		motor_demo_5ms_cont = 0;
		motor_demo_1sec_cont = 0;
		Motor_PwmImStop(Motor_Back_ParaStu);
		Motor_PwmImStop(Motor_Leg_ParaStu);
		Motor_PwmImStop(Motor_Lumbar_ParaStu);
		Motor_PwmImStop(Motor_Lumbar2_ParaStu);
		Motor_PwmImStop(Motor_Neck_ParaStu);
		Motor_PwmImStop(Motor_Neck2_ParaStu);
		Light_OneColour_Stu.led_colour_state = 0;
		led_board_state = 0;		
		Music_SW(SWITCH_CTR_OFF);//音响关闭
		if(User_Msgr_Demo(0) == 1)
		{
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;	
		}		
	}	
	if(motor_demo_step == 1)
	{
		if(User_MusicDemo(0) == 1)
		{
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;	
		}		
	}	
	if(motor_demo_step == 2)
	{
		if(User_LightDemo(0) == 1)
		{
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;	
		}		
	}	
	
	if(motor_demo_step == 3)
	{
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
	}
	if(motor_demo_step == 4)
	{
			motor_demo_step++;
	}	
	//=============================================================================
	// 第一阶段: 摇摆7次(41mm,27mm)?(28mm,38mm), 2档CONSTANT, 全程慢速2-3mm/s
	//=============================================================================
	if(motor_demo_step == 5)
	{
		if(motor_fast_change_flag)
		{
			motor_fast_change_flag = 0;	
			Motor_PwmImStop(Motor_Back_ParaStu);
			Motor_PwmImStop(Motor_Leg_ParaStu);
			Motor_Para_Reset(Motor_Back_ParaStu);
			Motor_Para_Reset(Motor_Leg_ParaStu);						
		}
		else
		{
			msgr_mode_set = MSGR_CONSTANT_MODE;
			Msgr_Ints_FlagArr[1] = MSGR_INTS_TWO_LEVEL;
			Msgr_Ints_FlagArr[2] = MSGR_INTS_TWO_LEVEL;
			Motor_Back_ParaStu->motor_slow_run_flag = 1;
			Motor_Leg_ParaStu->motor_slow_run_flag = 1;
			if(!Motor_ArrivePosition(Motor_Back_ParaStu,SLEEP_BACK_29DU_NUM))
			{
				motor_complete_flag |= 0x01;
			}
			if(!Motor_ArrivePosition(Motor_Leg_ParaStu,SLEEP_LEG_20DU_NUM))
			{
				motor_complete_flag |= 0x02;
			}
			if((motor_complete_flag & 0x03) == 0x03)
			{
				motor_complete_flag = 0;
				motor_demo_loop_count = 0;
				motor_demo_step++;
				motor_demo_ms_time = 0;
				motor_demo_sec_time = 0;
			}
		}
	}
	if(motor_demo_step == 6)
	{
		if(motor_demo_sec_time >= 1)
		{
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;		
			if(motor_demo_slave_flag == 0)
			{
				motor_demo_step++;
				LIN_Master_Send_DemoStep_Cmd(2, motor_demo_step);
			}	
		}
	}
	if(motor_demo_step == 7)
	{
		if(motor_fast_change_flag)
		{
			motor_fast_change_flag = 0;	
			Motor_PwmImStop(Motor_Back_ParaStu);
			Motor_PwmImStop(Motor_Leg_ParaStu);
			Motor_Para_Reset(Motor_Back_ParaStu);
			Motor_Para_Reset(Motor_Leg_ParaStu);						
		}
		else
		{
			Motor_Back_ParaStu->motor_slow_run_flag = 1;
			Motor_Leg_ParaStu->motor_slow_run_flag = 1;
			if(!Motor_ArrivePosition(Motor_Back_ParaStu,SLEEP_BACK_20DU_NUM))
			{
				motor_complete_flag |= 0x01;
			}
			if(!Motor_ArrivePosition(Motor_Leg_ParaStu,SLEEP_LEG_28DU_NUM))
			{
				motor_complete_flag |= 0x02;
			}
			if((motor_complete_flag & 0x03) == 0x03)
			{
				motor_complete_flag = 0;
				motor_demo_step++;
				motor_demo_ms_time = 0;
				motor_demo_sec_time = 0;
			}
		}
	}
	if(motor_demo_step == 8)
	{
		if(motor_demo_sec_time >= 1)
		{
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;		
			if(motor_demo_slave_flag == 0)
			{
				motor_demo_step++;
				LIN_Master_Send_DemoStep_Cmd(2, motor_demo_step);
			}	
		}
	}
	if(motor_demo_step == 9)
	{
		if(motor_fast_change_flag)
		{
			motor_fast_change_flag = 0;	
			Motor_PwmImStop(Motor_Back_ParaStu);
			Motor_PwmImStop(Motor_Leg_ParaStu);
			Motor_Para_Reset(Motor_Back_ParaStu);
			Motor_Para_Reset(Motor_Leg_ParaStu);						
		}
		else
		{
			Motor_Back_ParaStu->motor_slow_run_flag = 1;
			Motor_Leg_ParaStu->motor_slow_run_flag = 1;
			if(!Motor_ArrivePosition(Motor_Back_ParaStu,SLEEP_BACK_29DU_NUM))
			{
				motor_complete_flag |= 0x01;
			}
			if(!Motor_ArrivePosition(Motor_Leg_ParaStu,SLEEP_LEG_20DU_NUM))
			{
				motor_complete_flag |= 0x02;
			}
			if((motor_complete_flag & 0x03) == 0x03)
			{
				motor_complete_flag = 0;
				motor_demo_step++;
				motor_demo_ms_time = 0;
				motor_demo_sec_time = 0;
			}
		}
	}
	if(motor_demo_step == 10)
	{
		motor_demo_loop_count++;
		if(motor_demo_loop_count < 7)
		{
			motor_demo_step = 6;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
		}
		else
		{
			motor_demo_loop_count = 0;
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
		}
	}
	if(motor_demo_step == 11)
	{
		if(motor_demo_sec_time >= 1)
		{
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;		
			if(motor_demo_slave_flag == 0)
			{
				motor_demo_step++;
				LIN_Master_Send_DemoStep_Cmd(2, motor_demo_step);
			}	
		}
	}
	if(motor_demo_step == 12)
	{
		if(motor_fast_change_flag)
		{
			motor_fast_change_flag = 0;	
			Motor_PwmImStop(Motor_Back_ParaStu);
			Motor_PwmImStop(Motor_Leg_ParaStu);
			Motor_Para_Reset(Motor_Back_ParaStu);
			Motor_Para_Reset(Motor_Leg_ParaStu);						
		}
		else
		{
			Motor_Back_ParaStu->motor_slow_run_flag = 1;
			Motor_Leg_ParaStu->motor_slow_run_flag = 1;
			if(!Motor_ArrivePosition(Motor_Back_ParaStu,SLEEP_BACK_26DU_NUM))
			{
				motor_complete_flag |= 0x01;
			}
			if(!Motor_ArrivePosition(Motor_Leg_ParaStu,SLEEP_LEG_24DU_NUM))
			{
				motor_complete_flag |= 0x02;
			}
			if((motor_complete_flag & 0x03) == 0x03)
			{
				motor_complete_flag = 0;
				motor_demo_step++;
				motor_demo_ms_time = 0;
				motor_demo_sec_time = 0;
			}
		}
	}
	//=============================================================================
	// 第二阶段: 摇摆14次, 1档CONSTANT
	//=============================================================================
	if(motor_demo_step == 13)
	{
		msgr_mode_set = MSGR_CONSTANT_MODE;
		Msgr_Ints_FlagArr[1] = MSGR_INTS_ONE_LEVEL;
		Msgr_Ints_FlagArr[2] = MSGR_INTS_ONE_LEVEL;
		motor_demo_ms_time = 0;
		motor_demo_sec_time = 0;
		if(motor_demo_slave_flag == 0)
		{
			motor_demo_step++;
			LIN_Master_Send_DemoStep_Cmd(2, motor_demo_step);
		}
	}
	if(motor_demo_step == 14)
	{
		if(motor_fast_change_flag)
		{
			motor_fast_change_flag = 0;	
			Motor_PwmImStop(Motor_Back_ParaStu);
			Motor_PwmImStop(Motor_Leg_ParaStu);
			Motor_Para_Reset(Motor_Back_ParaStu);
			Motor_Para_Reset(Motor_Leg_ParaStu);						
		}
		else
		{
			Motor_Back_ParaStu->motor_slow_run_flag = 1;
			Motor_Leg_ParaStu->motor_slow_run_flag = 0;
			if(!Motor_ArrivePosition(Motor_Back_ParaStu,SLEEP_BACK_20DU_NUM))
			{
				motor_complete_flag |= 0x01;
			}
			if(!Motor_ArrivePosition(Motor_Leg_ParaStu,SLEEP_LEG_34DU_NUM))
			{
				motor_complete_flag |= 0x02;
			}
			if((motor_complete_flag & 0x03) == 0x03)
			{
				motor_complete_flag = 0;
				motor_demo_step++;
				motor_demo_ms_time = 0;
				motor_demo_sec_time = 0;
			}
		}
	}
	if(motor_demo_step == 15)
	{
		if(motor_demo_sec_time >= 1)
		{
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;		
			if(motor_demo_slave_flag == 0)
			{
				motor_demo_step++;
				LIN_Master_Send_DemoStep_Cmd(2, motor_demo_step);
			}	
		}
	}
	if(motor_demo_step == 16)
	{
		if(motor_fast_change_flag)
		{
			motor_fast_change_flag = 0;	
			Motor_PwmImStop(Motor_Back_ParaStu);
			Motor_PwmImStop(Motor_Leg_ParaStu);
			Motor_Para_Reset(Motor_Back_ParaStu);
			Motor_Para_Reset(Motor_Leg_ParaStu);						
		}
		else
		{
			Motor_Back_ParaStu->motor_slow_run_flag = 1;
			Motor_Leg_ParaStu->motor_slow_run_flag = 1;
			if(!Motor_ArrivePosition(Motor_Back_ParaStu,SLEEP_BACK_27DU_NUM))
			{
				motor_complete_flag |= 0x01;
			}
			if(!Motor_ArrivePosition(Motor_Leg_ParaStu,SLEEP_LEG_23DU_NUM))
			{
				motor_complete_flag |= 0x02;
			}
			if((motor_complete_flag & 0x03) == 0x03)
			{
				motor_complete_flag = 0;
				motor_demo_step++;
				motor_demo_ms_time = 0;
				motor_demo_sec_time = 0;
			}
		}
	}
	if(motor_demo_step == 17)
	{
		if(motor_demo_sec_time >= 5)
		{
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
		}
	}
	if(motor_demo_step == 18)
	{
		motor_demo_loop_count++;
		if(motor_demo_loop_count < 14)
		{
			motor_demo_step = 14;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
		}
		else
		{
			motor_demo_loop_count = 0;
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
		}
	}
	if(motor_demo_step == 19)
	{
		if(motor_demo_sec_time >= 1)
		{
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;		
			if(motor_demo_slave_flag == 0)
			{
				motor_demo_step++;
				LIN_Master_Send_DemoStep_Cmd(2, motor_demo_step);
			}	
		}
	}
	if(motor_demo_step == 20)
	{
		if(motor_fast_change_flag)
		{
			motor_fast_change_flag = 0;	
			Motor_PwmImStop(Motor_Back_ParaStu);
			Motor_PwmImStop(Motor_Leg_ParaStu);
			Motor_Para_Reset(Motor_Back_ParaStu);
			Motor_Para_Reset(Motor_Leg_ParaStu);						
		}
		else
		{
			Motor_Back_ParaStu->motor_slow_run_flag = 1;
			Motor_Leg_ParaStu->motor_slow_run_flag = 1;
			if(!Motor_ArrivePosition(Motor_Back_ParaStu,SLEEP_BACK_27DU_NUM))
			{
				motor_complete_flag |= 0x01;
			}
			if(!Motor_ArrivePosition(Motor_Leg_ParaStu,SLEEP_LEG_23DU_NUM))
			{
				motor_complete_flag |= 0x02;
			}
			if((motor_complete_flag & 0x03) == 0x03)
			{
				motor_complete_flag = 0;
				motor_demo_step++;
				motor_demo_ms_time = 0;
				motor_demo_sec_time = 0;
			}
		}
	}
	//=============================================================================
	// 第三阶段: 摇摆12.5次, 1档CONSTANT
	//=============================================================================
	if(motor_demo_step == 21)
	{
		motor_demo_ms_time = 0;
		motor_demo_sec_time = 0;
		if(motor_demo_slave_flag == 0)
		{
			motor_demo_step++;
			LIN_Master_Send_DemoStep_Cmd(2, motor_demo_step);
		}
	}
	if(motor_demo_step == 22)
	{
		if(motor_fast_change_flag)
		{
			motor_fast_change_flag = 0;	
			Motor_PwmImStop(Motor_Back_ParaStu);
			Motor_PwmImStop(Motor_Leg_ParaStu);
			Motor_Para_Reset(Motor_Back_ParaStu);
			Motor_Para_Reset(Motor_Leg_ParaStu);						
		}
		else
		{
			Motor_Back_ParaStu->motor_slow_run_flag = 1;
			Motor_Leg_ParaStu->motor_slow_run_flag = 0;
			if(!Motor_ArrivePosition(Motor_Back_ParaStu,SLEEP_BACK_21DU_NUM))
			{
				motor_complete_flag |= 0x01;
			}
			if(!Motor_ArrivePosition(Motor_Leg_ParaStu,SLEEP_LEG_31DU_NUM))
			{
				motor_complete_flag |= 0x02;
			}
			if((motor_complete_flag & 0x03) == 0x03)
			{
				motor_complete_flag = 0;
				motor_demo_step++;
				motor_demo_ms_time = 0;
				motor_demo_sec_time = 0;
			}
		}
	}
	if(motor_demo_step == 23)
	{
		if(motor_demo_sec_time >= 1)
		{
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;		
			if(motor_demo_slave_flag == 0)
			{
				motor_demo_step++;
				LIN_Master_Send_DemoStep_Cmd(2, motor_demo_step);
			}	
		}
	}
	if(motor_demo_step == 24)
	{
		if(motor_fast_change_flag)
		{
			motor_fast_change_flag = 0;	
			Motor_PwmImStop(Motor_Back_ParaStu);
			Motor_PwmImStop(Motor_Leg_ParaStu);
			Motor_Para_Reset(Motor_Back_ParaStu);
			Motor_Para_Reset(Motor_Leg_ParaStu);						
		}
		else
		{
			Motor_Back_ParaStu->motor_slow_run_flag = 1;
			Motor_Leg_ParaStu->motor_slow_run_flag = 1;
			if(!Motor_ArrivePosition(Motor_Back_ParaStu,SLEEP_BACK_27DU_NUM))
			{
				motor_complete_flag |= 0x01;
			}
			if(!Motor_ArrivePosition(Motor_Leg_ParaStu,SLEEP_LEG_23DU_NUM))
			{
				motor_complete_flag |= 0x02;
			}
			if((motor_complete_flag & 0x03) == 0x03)
			{
				motor_complete_flag = 0;
				motor_demo_step++;
				motor_demo_ms_time = 0;
				motor_demo_sec_time = 0;
			}
		}
	}
	if(motor_demo_step == 25)
	{
		if(motor_demo_sec_time >= 5)
		{
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
		}
	}
	if(motor_demo_step == 26)
	{
		motor_demo_loop_count++;
		if(motor_demo_loop_count < 12)
		{
			motor_demo_step = 22;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
		}
		else
		{
			motor_demo_loop_count = 0;
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
		}
	}
	if(motor_demo_step == 27)
	{
		if(motor_demo_sec_time >= 1)
		{
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;		
			if(motor_demo_slave_flag == 0)
			{
				motor_demo_step++;
				LIN_Master_Send_DemoStep_Cmd(2, motor_demo_step);
			}	
		}
	}
	if(motor_demo_step == 28)
	{
		if(motor_fast_change_flag)
		{
			motor_fast_change_flag = 0;	
			Motor_PwmImStop(Motor_Back_ParaStu);
			Motor_PwmImStop(Motor_Leg_ParaStu);
			Motor_Para_Reset(Motor_Back_ParaStu);
			Motor_Para_Reset(Motor_Leg_ParaStu);						
		}
		else
		{
			Motor_Back_ParaStu->motor_slow_run_flag = 1;
			Motor_Leg_ParaStu->motor_slow_run_flag = 0;
			if(!Motor_ArrivePosition(Motor_Back_ParaStu,SLEEP_BACK_21DU_NUM))
			{
				motor_complete_flag |= 0x01;
			}
			if(!Motor_ArrivePosition(Motor_Leg_ParaStu,SLEEP_LEG_31DU_NUM))
			{
				motor_complete_flag |= 0x02;
			}
			if((motor_complete_flag & 0x03) == 0x03)
			{
				motor_complete_flag = 0;
				msgr_mode_set = MSGR_FOLLOW_MODE;
				Msgr_Ints_FlagArr[0] = MSGR_INTS_ZERO_LEVEL;
				Msgr_Ints_FlagArr[1] = MSGR_INTS_ZERO_LEVEL;
				Msgr_Ints_FlagArr[2] = MSGR_INTS_ZERO_LEVEL;
				motor_demo_step++;
				motor_demo_ms_time = 0;
				motor_demo_sec_time = 0;
			}
		}
	}
	//=============================================================================
	// 第四阶段: 电机下降回零位
	//=============================================================================
	if(motor_demo_step == 29)
	{
		if(motor_demo_sec_time >= 1)
		{
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;		
			if(motor_demo_slave_flag == 0)
			{
				motor_demo_step++;
				LIN_Master_Send_DemoStep_Cmd(2, motor_demo_step);
			}	
		}
	}
	if(motor_demo_step == 30)
	{
		if(motor_fast_change_flag)
		{
			motor_fast_change_flag = 0;	
			Motor_PwmImStop(Motor_Back_ParaStu);
			Motor_PwmImStop(Motor_Leg_ParaStu);
			Motor_Para_Reset(Motor_Back_ParaStu);
			Motor_Para_Reset(Motor_Leg_ParaStu);						
		}
		else
		{
			Motor_Back_ParaStu->motor_slow_run_flag = 1;
			Motor_Leg_ParaStu->motor_slow_run_flag = 1;
			if(!Motor_ArrivePosition(Motor_Back_ParaStu,SLEEP_BACK_17DU_NUM))
			{
				motor_complete_flag |= 0x01;
			}
			if(!Motor_ArrivePosition(Motor_Leg_ParaStu,SLEEP_LEG_20DU_NUM))
			{
				motor_complete_flag |= 0x02;
			}
			if((motor_complete_flag & 0x03) == 0x03)
			{
				motor_complete_flag = 0;
				motor_demo_step++;
				motor_demo_ms_time = 0;
				motor_demo_sec_time = 0;
			}
		}
	}
	if(motor_demo_step == 31)
	{
		Motor_Back_ParaStu->motor_slow_run_flag = 1;
		Motor_Leg_ParaStu->motor_slow_run_flag = 1;
		if(!Motor_ArrivePosition(Motor_Back_ParaStu,SLEEP_BACK_17DU_NUM))
		{
			motor_complete_flag |= 0x01;
		}
		if(!Motor_ArrivePosition(Motor_Leg_ParaStu,SLEEP_LEG_12DU_NUM))
		{
			motor_complete_flag |= 0x02;
		}
		if((motor_complete_flag & 0x03) == 0x03)
		{
			motor_complete_flag = 0;
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
		}
	}
	if(motor_demo_step == 32)
	{
		Motor_Back_ParaStu->motor_slow_run_flag = 1;
		Motor_Leg_ParaStu->motor_slow_run_flag = 1;
		if(!Motor_ArrivePosition(Motor_Back_ParaStu,SLEEP_BACK_9DU_NUM))
		{
			motor_complete_flag |= 0x01;
		}
		if(!Motor_ArrivePosition(Motor_Leg_ParaStu,SLEEP_LEG_12DU_NUM))
		{
			motor_complete_flag |= 0x02;
		}
		if((motor_complete_flag & 0x03) == 0x03)
		{
			motor_complete_flag = 0;
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
		}
	}
	if(motor_demo_step == 33)
	{
		Motor_Back_ParaStu->motor_slow_run_flag = 1;
		Motor_Leg_ParaStu->motor_slow_run_flag = 1;
		if(!Motor_ArrivePosition(Motor_Back_ParaStu,SLEEP_BACK_9DU_NUM))
		{
			motor_complete_flag |= 0x01;
		}
		if(!Motor_ArrivePosition(Motor_Leg_ParaStu,SLEEP_LEG_0DU_NUM))
		{
			motor_complete_flag |= 0x02;
		}
		if((motor_complete_flag & 0x03) == 0x03)
		{
			motor_complete_flag = 0;
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
		}
	}
	if(motor_demo_step == 34)
	{
		Motor_Back_ParaStu->motor_slow_run_flag = 1;
		Motor_Leg_ParaStu->motor_slow_run_flag = 1;
		if(!Motor_ArrivePosition(Motor_Back_ParaStu,SLEEP_BACK_0DU_NUM))
		{
			motor_complete_flag |= 0x01;
		}
		if(!Motor_ArrivePosition(Motor_Leg_ParaStu,SLEEP_LEG_0DU_NUM))
		{
			motor_complete_flag |= 0x02;
		}
		if((motor_complete_flag & 0x03) == 0x03)
		{
			motor_complete_flag = 0x00;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
			motor_demo_step = 0;
			lin_received_step = 0;
			return 1;
		}
	}

	return 0;
}

// 1.背部腿部电机原速度同时运行, 背部15度（33）, 腿部到达35°（102）背部和腿部电机都停止后, 计时5S（DEMO_STOP_TIME）; 
// 2.背部腿部电机同时运行，腿部电机50%速度下降至22°（55）停止（若背部电机先停止则腿部无需到22°就停止）；背部电机50%速度上升至20度（46）后, 停止, 计时2S
// 3.计时完成后,背部腿部电机同时运行，腿部电机50%速度上升至35°（102）后停止（若背部电机先停止则腿部无需到35°就停止）;背部电机50%速度下降至12度（26）后,停止, 计时2S;
// 4.第(2)和(3)步为1个循环, 连续重复执行25个循环后, 背部腿部电机同时运行，背部电机50%速度下降/上升至15°(33)后停止；腿部电机50%速度下降/上升至35°(102)后停止, 计时120S;
// 5. 计时完成后, 背部电机先40%速度下降至10°(21)后停止, 腿部电机再40%速度下降至15°(33)后停止, 计时150S;
// 6. 计时完成后, 背部电机30%速度下降至0°后停止, 腿部电机30%速度下降0°后停止

unsigned char Motor_Demo_Help_Sleep_SSB_Mode(void)
{
	unsigned char motor_complete_flag = 0;
	unsigned char motor_fast_change_flag = 0;
	motor_demo_ms_time ++ ;
	if(motor_demo_ms_time >= 200)
	{
		motor_demo_ms_time = 0;
		motor_demo_sec_time ++;
		if(motor_demo_sec_time >= 480)
		{
			motor_demo_sec_time = 480;
		}
	}
	motor_demo_5ms_cont++;
	if(motor_demo_5ms_cont >= 200)
	{
		motor_demo_5ms_cont = 0;
		motor_demo_1sec_cont++;
	}
	Msgr_Clear_TimeCount();
	Light_Clear_TimeCount(&Light_RgbColour_Stu);
	Music_Time_ClearCount();
	// 更新步骤逻辑
	if(motor_demo_slave_flag == 1)
	{
		if((lin_received_step != motor_demo_step) && (lin_received_step != 0))
		{
				// 当前步骤未完成就先停止电机
			Motor_PwmImStop(Motor_Back_ParaStu);
			Motor_PwmImStop(Motor_Leg_ParaStu);
			Motor_PwmImStop(Motor_Lumbar_ParaStu);
			Motor_PwmImStop(Motor_Lumbar2_ParaStu);
			Motor_PwmImStop(Motor_Neck_ParaStu);
			Motor_PwmImStop(Motor_Neck2_ParaStu);
			// 再进入下一步
			motor_demo_step = lin_received_step;
			motor_fast_change_flag = 1;
			lin_received_step = 0;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
		}
	}

	if(motor_demo_step == 0)
	{
		motor_demo_loop_count = 0;
		motor_demo_5ms_cont = 0;
		motor_demo_1sec_cont = 0;
		Motor_PwmImStop(Motor_Back_ParaStu);
		Motor_PwmImStop(Motor_Leg_ParaStu);
		Motor_PwmImStop(Motor_Lumbar_ParaStu);
		Motor_PwmImStop(Motor_Lumbar2_ParaStu);
		Motor_PwmImStop(Motor_Neck_ParaStu);
		Motor_PwmImStop(Motor_Neck2_ParaStu);
		Motor_PwmImStop(Motor_Tilt1_ParaStu);
		Motor_PwmImStop(Motor_Tilt2_ParaStu);
		ble_report_set_event(BLE_REPORT_EVENT, REPORT_DEMO_RUN_EVENT);
		Light_OneColour_Stu.led_colour_state = 0;
		led_board_state = 0;		
		Music_SW(SWITCH_CTR_OFF);//音响关闭
		if(User_LightDemo(1) == 1)
		{
			motor_demo_step++;
		}
	}
	if(motor_demo_step == 1)
	{
		if(User_MusicDemo(1) == 1)
		{
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
		}	
	}
	if(motor_demo_step == 2)
	{
		if(User_Msgr_Demo(0) == 1)
		{
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
		}	
	}
	if(motor_demo_step == 3)
	{
		if(User_MusicDemo(3) == 1)
		{
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
		}
	}
	if(motor_demo_step == 4)
	{
		motor_demo_step++;
	}
	if(motor_demo_step == 5)
	{
		if(!Motor_ArrivePosition(Motor_Back_ParaStu,HALL_MIN_NUM))
		{
			Motor_Para_Reset(Motor_Back_ParaStu);
			motor_complete_flag |= 0x01;
		}
		if(!Motor_ArrivePosition(Motor_Leg_ParaStu,HALL_MIN_NUM))
		{
			Motor_Para_Reset(Motor_Leg_ParaStu);
			motor_complete_flag |= 0x02;
		}
		if(!Motor_ArrivePosition(Motor_Lumbar_ParaStu,HALL_MIN_NUM))
		{
			Motor_Para_Reset(Motor_Lumbar_ParaStu);
			motor_complete_flag |= 0x04;
		}
		if(!Motor_ArrivePosition(Motor_Lumbar2_ParaStu,HALL_MIN_NUM))
		{
			Motor_Para_Reset(Motor_Lumbar2_ParaStu);
			motor_complete_flag |= 0x08;
		}
		if(!Motor_ArrivePosition(Motor_Neck_ParaStu,HALL_MIN_NUM))
		{
			Motor_Para_Reset(Motor_Neck_ParaStu);
			motor_complete_flag |= 0x10;
		}			
		if(!Motor_ArrivePosition(Motor_Neck2_ParaStu,HALL_MIN_NUM))
		{
			Motor_Para_Reset(Motor_Neck2_ParaStu);
			motor_complete_flag |= 0x20;
		}	
		
		if((motor_complete_flag & 0x3f) == 0x3f)
		{
			motor_complete_flag = 0x00;
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
		}
	}
	if(motor_demo_step == 6)
	{
		if(motor_demo_sec_time >= 1)
		{
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;		
			if(motor_demo_slave_flag == 0)
			{
				motor_demo_step++;
				LIN_Master_Send_DemoStep_Cmd(2, motor_demo_step);
			}	
		}	
	}	
	if(motor_demo_step == 7)//原速度同时运行, 背部15度, 腿部到达35°背部和腿部电机都停止后, 计时5S; 
	{
		if(motor_fast_change_flag)
		{
			motor_fast_change_flag = 0;	
			Motor_PwmImStop(Motor_Back_ParaStu);
			Motor_PwmImStop(Motor_Leg_ParaStu);
			Motor_PwmImStop(Motor_Lumbar_ParaStu);
			Motor_PwmImStop(Motor_Lumbar2_ParaStu);
			Motor_PwmImStop(Motor_Neck_ParaStu);
			Motor_PwmImStop(Motor_Neck2_ParaStu);	
			Motor_Para_Reset(Motor_Back_ParaStu);
			Motor_Para_Reset(Motor_Leg_ParaStu);
			Motor_Para_Reset(Motor_Lumbar_ParaStu);
			Motor_Para_Reset(Motor_Lumbar2_ParaStu);
			Motor_Para_Reset(Motor_Neck_ParaStu);
			Motor_Para_Reset(Motor_Neck2_ParaStu);

		}
		else
		{
			if(!Motor_ArrivePosition(Motor_Back_ParaStu,BACK_15DU_HALL_NUM))
			{
				motor_complete_flag |= 0x01;
			} 
			if(!Motor_ArrivePosition(Motor_Leg_ParaStu,LEG_35DU_HALL_NUM))
			{
				motor_complete_flag |= 0x02;
			}				
			if((motor_complete_flag & 0x03) == 0x03)
			{
				motor_demo_step++;
				motor_complete_flag = 0;
				motor_demo_ms_time = 0;
				motor_demo_sec_time = 0;
			}
		}
	}
	if(motor_demo_step == 8)
	{
		Motor_PwmImStop(Motor_Back_ParaStu);
		Motor_PwmImStop(Motor_Leg_ParaStu);
		if(motor_demo_sec_time >= 5)
		{
			motor_demo_sec_time = 0;
			motor_demo_ms_time = 0;
			motor_demo_step++;
		}
	}
	if(motor_demo_step == 9)
	{
		motor_demo_ms_time = 0;
		motor_demo_sec_time = 0;	
		if(motor_demo_slave_flag == 0)
		{
			motor_demo_step++;
			LIN_Master_Send_DemoStep_Cmd(2, motor_demo_step);
		}
	}	
		//.背部腿部电机同时运行，腿部电机50%速度下降至22°停止（若背部电机先停止则腿部无需到22°就停止）；背部电机50%速度上升至20度后, 停止, 计时2S
	if(motor_demo_step == 10)
	{
		if(motor_fast_change_flag)
		{
			motor_fast_change_flag = 0;	
			Motor_PwmImStop(Motor_Back_ParaStu);
			Motor_PwmImStop(Motor_Leg_ParaStu);
			Motor_Para_Reset(Motor_Back_ParaStu);
			Motor_Para_Reset(Motor_Leg_ParaStu);		
		}
		else
		{			
			Motor_Back_ParaStu->motor_slow_run_flag = 1;
			Motor_Leg_ParaStu->motor_slow_run_flag = 1;
			if(!Motor_ArrivePosition(Motor_Back_ParaStu,BACK_20DU_HALL_NUM))
			{
				motor_complete_flag |= 0x01;
			} 				
			if((!Motor_ArrivePosition(Motor_Leg_ParaStu,LEG_22DU_HALL_NUM)) || (motor_complete_flag == 0x01))
			{
				motor_complete_flag |= 0x02;
				Motor_PwmImStop(Motor_Leg_ParaStu);
			}
			if(motor_complete_flag == 0x03)
			{
				motor_complete_flag = 0x00;
				motor_demo_step++;
				motor_demo_ms_time = 0;
				motor_demo_sec_time = 0;
			}
		}
	}
	if(motor_demo_step == 11)
	{
		Motor_PwmImStop(Motor_Back_ParaStu);
		Motor_PwmImStop(Motor_Leg_ParaStu);
		if(motor_demo_sec_time >= 5)
		{
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
		}
	}
	if(motor_demo_step == 12)
	{
		motor_demo_ms_time = 0;
		motor_demo_sec_time = 0;
		if(motor_demo_slave_flag == 0)
		{
			motor_demo_step++;
			LIN_Master_Send_DemoStep_Cmd(2, motor_demo_step);
		}
	}	
		//计时完成后,背部腿部电机同时运行，腿部电机50%速度上升至35°后停止（若背部电机先停止则腿部无需到35°就停止）;
	//背部电机50%速度下降至12度后,停止, 计时2S;
	if(motor_demo_step == 13)
	{
		if(motor_fast_change_flag)
		{
			motor_fast_change_flag = 0;	
			Motor_PwmImStop(Motor_Back_ParaStu);
			Motor_PwmImStop(Motor_Leg_ParaStu);
			Motor_Para_Reset(Motor_Back_ParaStu);
			Motor_Para_Reset(Motor_Leg_ParaStu);						
		}
		else
		{			
			Motor_Back_ParaStu->motor_slow_run_flag = 1;
			Motor_Leg_ParaStu->motor_slow_run_flag = 1;		
			if(!Motor_ArrivePosition(Motor_Back_ParaStu,BACK_12DU_HALL_NUM))
			{
				motor_complete_flag |= 0x01;
			} 
			if((!Motor_ArrivePosition(Motor_Leg_ParaStu,LEG_35DU_HALL_NUM)) || (motor_complete_flag == 0x01))
			{
				motor_complete_flag |= 0x02;
				Motor_PwmImStop(Motor_Leg_ParaStu);
			}
			if(motor_complete_flag == 0x03)
			{
				motor_complete_flag = 0x00;
				motor_demo_step++;
				motor_demo_ms_time = 0;
				motor_demo_sec_time = 0;
			}
		}
	}	
	if(motor_demo_step == 14)
	{
		Motor_PwmImStop(Motor_Back_ParaStu);
		Motor_PwmImStop(Motor_Leg_ParaStu);	
		if(motor_demo_sec_time >= 5)
		{
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
		}
	}
	if(motor_demo_step == 15)
	{	
		if(system_config.flags.help_sleep_loop == DEMO_NUM_LOOP)
		{
			motor_demo_loop_count++;
			if(motor_demo_loop_count <= 25)
			{
				motor_demo_step = 9;
				motor_demo_ms_time = 0;
				motor_demo_sec_time = 0;
			}
			else
			{
				motor_demo_loop_count = 0;
				motor_demo_step++;
				motor_demo_ms_time = 0;
				motor_demo_sec_time = 0;
			}		
		}
		else
		{
			if(motor_demo_1sec_cont <= demo_run_time * 60)
			{
				motor_demo_step = 9;
				motor_demo_ms_time = 0;
				motor_demo_sec_time = 0;
			}
			else
			{
				motor_demo_loop_count = 0;
				motor_demo_step++;
				motor_demo_ms_time = 0;
				motor_demo_sec_time = 0;
			}
		}
	}
	if(motor_demo_step == 16)
	{
		motor_demo_ms_time = 0;
		motor_demo_sec_time = 0;		
		if(motor_demo_slave_flag == 0)
		{
			motor_demo_step++;
			LIN_Master_Send_DemoStep_Cmd(2, motor_demo_step);
		}	
	}	
	if(motor_demo_step == 17)
	{
		if(User_MusicDemo(2) == 1)
		{
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
			motor_demo_step++;	
		}
	}
	if(motor_demo_step == 18)
	{
		if(User_LightDemo(0) == 1)
		{
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
			motor_demo_step++;	
		}
	}
	if(motor_demo_step == 19)
	{
		if(motor_demo_sec_time >= 60)
		{
			motor_demo_sec_time = 0;
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
		}
	}
	if(motor_demo_step == 20)
	{
		motor_demo_ms_time = 0;
		motor_demo_sec_time = 0;		
		if(motor_demo_slave_flag == 0)
		{
			motor_demo_step++;
			LIN_Master_Send_DemoStep_Cmd(2, motor_demo_step);
		}	
	}
	if(motor_demo_step == 21)
	{
		if(motor_fast_change_flag)
		{
			motor_fast_change_flag = 0;	
			Motor_PwmImStop(Motor_Back_ParaStu);
			Motor_PwmImStop(Motor_Leg_ParaStu);
			Motor_Para_Reset(Motor_Back_ParaStu);
			Motor_Para_Reset(Motor_Leg_ParaStu);				
		}
		else
		{			
			Motor_Back_ParaStu->motor_slow_run_flag = 1;
			Motor_Leg_ParaStu->motor_slow_run_flag = 1;				
			if(!Motor_ArrivePosition(Motor_Back_ParaStu,HALL_MIN_NUM))
			{
				motor_complete_flag |= 0x01;
			}
			if(!Motor_ArrivePosition(Motor_Leg_ParaStu,HALL_MIN_NUM))			
			{
				motor_complete_flag |= 0x02;
			}
			if(motor_complete_flag == 0x03)
			{
					Motor_Back_ParaStu->motor_slow_run_flag = 0;
					Motor_Leg_ParaStu->motor_slow_run_flag = 0;		
					motor_complete_flag = 0x00;
					motor_demo_ms_time = 0;
					motor_demo_sec_time = 0;
					motor_demo_step = 0;
					lin_received_step = 0;
					motor_run_fixed_complate = 1;		
					ble_report_set_event(BLE_REPORT_EVENT, REPORT_DEMO_RUN_EVENT);
					return 1;					
			}
		}
	}
	return 0;
}
unsigned char Motor_Demo2_Mode(void)
{
	unsigned char motor_complete_flag = 0;
	unsigned char motor_fast_change_flag = 0;

	motor_demo_ms_time ++ ;
	if(motor_demo_ms_time >= 200)
	{
		motor_demo_ms_time = 0;
		motor_demo_sec_time ++;
		if(motor_demo_sec_time >= 480)
		{
			motor_demo_sec_time = 480;
		}
	}
	motor_demo_5ms_cont++;
	if(motor_demo_5ms_cont >= 200)
	{
		motor_demo_5ms_cont = 0;
		motor_demo_1sec_cont++;
	}
	Msgr_Clear_TimeCount();
	Music_Time_ClearCount();
	Light_Clear_TimeCount(&Light_RgbColour_Stu);
	
	// 更新步骤逻辑
	if(motor_demo_slave_flag == 1)
	{
		if((lin_received_step != motor_demo_step) && (lin_received_step != 0))
		{
				// 当前步骤未完成就先停止电机
			Motor_PwmImStop(Motor_Back_ParaStu);
			Motor_PwmImStop(Motor_Leg_ParaStu);
			Motor_PwmImStop(Motor_Lumbar_ParaStu);
			Motor_PwmImStop(Motor_Lumbar2_ParaStu);
			Motor_PwmImStop(Motor_Neck_ParaStu);
			Motor_PwmImStop(Motor_Neck2_ParaStu);
			// 再进入下一步
			motor_demo_step = lin_received_step;
			motor_fast_change_flag = 1;
			lin_received_step = 0;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
		}
	}
	if(motor_demo_step == 0)
	{
		motor_demo_loop_count = 0;
		motor_demo_5ms_cont = 0;
		motor_demo_1sec_cont = 0;
		Motor_PwmImStop(Motor_Back_ParaStu);
		Motor_PwmImStop(Motor_Leg_ParaStu);
		Motor_PwmImStop(Motor_Lumbar_ParaStu);
		Motor_PwmImStop(Motor_Lumbar2_ParaStu);
		Motor_PwmImStop(Motor_Neck_ParaStu);
		Motor_PwmImStop(Motor_Neck2_ParaStu);
		Motor_PwmImStop(Motor_Tilt1_ParaStu);
		Motor_PwmImStop(Motor_Tilt2_ParaStu);
		Light_OneColour_Stu.led_colour_state = 0;
		led_board_state = 0;
		Music_SW(SWITCH_CTR_OFF);//音响关闭
		if(User_Msgr_Demo(0) == 1)
		{
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_DEMO_RUN_EVENT);
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;	
		}		
	}
	if(motor_demo_step == 1)
	{
			motor_demo_step++;
	}
	if(motor_demo_step == 2)
	{
		if(User_MusicDemo(0) == 1)
		{
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;	
		}		
	}
	if(motor_demo_step == 3)
	{
		motor_demo_step++;
	}	
	if(motor_demo_step == 4)
	{
		if(User_LightDemo(0) == 1)
		{
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;	
		}		
	}
	if(motor_demo_step == 5)
	{
		if(!Motor_ArrivePosition(Motor_Back_ParaStu,HALL_MIN_NUM))
		{
			Motor_Para_Reset(Motor_Back_ParaStu);
			motor_complete_flag |= 0x01;
		}
		if(!Motor_ArrivePosition(Motor_Leg_ParaStu,HALL_MIN_NUM))
		{
			Motor_Para_Reset(Motor_Leg_ParaStu);
			motor_complete_flag |= 0x02;
		}
		if(!Motor_ArrivePosition(Motor_Lumbar_ParaStu,HALL_MIN_NUM))
		{
			Motor_Para_Reset(Motor_Lumbar_ParaStu);
			motor_complete_flag |= 0x04;
		}
		if(!Motor_ArrivePosition(Motor_Neck_ParaStu,HALL_MIN_NUM))
		{
			Motor_Para_Reset(Motor_Neck_ParaStu);
			motor_complete_flag |= 0x08;
		}
		if(!Motor_ArrivePosition(Motor_Lumbar2_ParaStu,HALL_MIN_NUM))
		{
			Motor_Para_Reset(Motor_Lumbar2_ParaStu);
			motor_complete_flag |= 0x10;
		}			
		if(!Motor_ArrivePosition(Motor_Neck2_ParaStu,HALL_MIN_NUM))
		{
			Motor_Para_Reset(Motor_Neck2_ParaStu);
			motor_complete_flag |= 0x20;
		}	
		
		if((motor_complete_flag & 0x3f) == 0x3f)
		{
			motor_complete_flag = 0x00;
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
		}
	}
	if(motor_demo_step == 6)
	{
		if(motor_demo_sec_time >= 1)
		{
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;		
			if(motor_demo_slave_flag == 0)
			{
				motor_demo_step++;
				LIN_Master_Send_DemoStep_Cmd(2, motor_demo_step);
			}	
		}
	}
	if(motor_demo_step == 7)//原速度同时运行, 背部15度, 腿部到达35°背部和腿部电机都停止后, 计时5S; 
	{
		if(motor_fast_change_flag)
		{
			motor_fast_change_flag = 0;	
			Motor_PwmImStop(Motor_Back_ParaStu);
			Motor_PwmImStop(Motor_Leg_ParaStu);
			Motor_PwmImStop(Motor_Lumbar_ParaStu);
			Motor_PwmImStop(Motor_Lumbar2_ParaStu);
			Motor_PwmImStop(Motor_Neck_ParaStu);
			Motor_PwmImStop(Motor_Neck2_ParaStu);
			Motor_Para_Reset(Motor_Back_ParaStu);
			Motor_Para_Reset(Motor_Leg_ParaStu);
			Motor_Para_Reset(Motor_Lumbar_ParaStu);
			Motor_Para_Reset(Motor_Lumbar2_ParaStu);
			Motor_Para_Reset(Motor_Neck_ParaStu);
			Motor_Para_Reset(Motor_Neck2_ParaStu);
		}
		else
		{
			if(!Motor_ArrivePosition(Motor_Back_ParaStu,BACK_15DU_HALL_NUM))
			{
				motor_complete_flag |= 0x01;
			} 
			if(!Motor_ArrivePosition(Motor_Leg_ParaStu,LEG_35DU_HALL_NUM))
			{
				motor_complete_flag |= 0x02;
			}				
			if((motor_complete_flag & 0x03) == 0x03)
			{
				motor_demo_step++;
				motor_complete_flag = 0;
				motor_demo_ms_time = 0;
				motor_demo_sec_time = 0;
			}
		}
	}
	if(motor_demo_step == 8)
	{
		Motor_PwmImStop(Motor_Back_ParaStu);
		Motor_PwmImStop(Motor_Leg_ParaStu);
		if(motor_demo_sec_time >= 5)
		{
			motor_demo_sec_time = 0;
			motor_demo_ms_time = 0;
			motor_demo_step++;
		}
	}
	if(motor_demo_step == 9)
	{
		motor_demo_ms_time = 0;
		motor_demo_sec_time = 0;		
		if(motor_demo_slave_flag == 0)
		{
			motor_demo_step++;
			LIN_Master_Send_DemoStep_Cmd(2, motor_demo_step);
		}	
	}	
		//.背部腿部电机同时运行，腿部电机50%速度下降至22°停止（若背部电机先停止则腿部无需到22°就停止）；背部电机50%速度上升至20度后, 停止, 计时2S
	if(motor_demo_step == 10)
	{
		if(motor_fast_change_flag)
		{
			motor_fast_change_flag = 0;	
			Motor_PwmImStop(Motor_Back_ParaStu);
			Motor_PwmImStop(Motor_Leg_ParaStu);
			Motor_Para_Reset(Motor_Back_ParaStu);
			Motor_Para_Reset(Motor_Leg_ParaStu);
		}
		else
		{		
			Motor_Back_ParaStu->motor_slow_run_flag = 1;
			Motor_Leg_ParaStu->motor_slow_run_flag = 1;
			if(!Motor_ArrivePosition(Motor_Back_ParaStu,BACK_20DU_HALL_NUM))
			{
				motor_complete_flag |= 0x01;
			} 			
			if((!Motor_ArrivePosition(Motor_Leg_ParaStu,LEG_22DU_HALL_NUM)) || (motor_complete_flag == 0x01))
			{
				motor_complete_flag |= 0x02;
				Motor_PwmImStop(Motor_Leg_ParaStu);
			}
			if(motor_complete_flag == 0x03)
			{
				motor_complete_flag = 0x00;
				motor_demo_step++;
				motor_demo_ms_time = 0;
				motor_demo_sec_time = 0;
			}
		}
	}
	if(motor_demo_step == 11)
	{
		Motor_PwmImStop(Motor_Back_ParaStu);
		Motor_PwmImStop(Motor_Leg_ParaStu);
		if(motor_demo_sec_time >= 5)
		{
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
		}
	}
	if(motor_demo_step == 12)
	{
		motor_demo_ms_time = 0;
		motor_demo_sec_time = 0;		
		if(motor_demo_slave_flag == 0)
		{
			motor_demo_step++;
			LIN_Master_Send_DemoStep_Cmd(2, motor_demo_step);
		}
	}	
		//计时完成后,背部腿部电机同时运行，腿部电机50%速度上升至35°后停止（若背部电机先停止则腿部无需到35°就停止）;
	//背部电机50%速度下降至12度后,停止, 计时2S;
	if(motor_demo_step == 13)
	{
		if(motor_fast_change_flag)
		{
			motor_fast_change_flag = 0;	
			Motor_PwmImStop(Motor_Back_ParaStu);
			Motor_PwmImStop(Motor_Leg_ParaStu);
			Motor_Para_Reset(Motor_Back_ParaStu);
			Motor_Para_Reset(Motor_Leg_ParaStu);
		}
		else
		{		
			Motor_Back_ParaStu->motor_slow_run_flag = 1;
			Motor_Leg_ParaStu->motor_slow_run_flag = 1;		
			if(!Motor_ArrivePosition(Motor_Back_ParaStu,BACK_12DU_HALL_NUM))
			{
				motor_complete_flag |= 0x01;
			} 		
			if((!Motor_ArrivePosition(Motor_Leg_ParaStu,LEG_35DU_HALL_NUM)) || (motor_complete_flag == 0x01))
			{
				motor_complete_flag |= 0x02;
				Motor_PwmImStop(Motor_Leg_ParaStu);
			}
			if(motor_complete_flag == 0x03)
			{
				motor_complete_flag = 0x00;
				motor_demo_step++;
				motor_demo_ms_time = 0;
				motor_demo_sec_time = 0;
			}
		}
	}	
	if(motor_demo_step == 14)
	{
		Motor_PwmImStop(Motor_Back_ParaStu);
		Motor_PwmImStop(Motor_Leg_ParaStu);	
		if(motor_demo_sec_time >= 5)
		{
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
		}
	}
	if(motor_demo_step == 15)
	{	
		if(system_config.flags.help_sleep_loop == DEMO_NUM_LOOP)
		{
			motor_demo_loop_count++;
			if(motor_demo_loop_count <= 25)
			{
				motor_demo_step = 9;
				motor_demo_ms_time = 0;
				motor_demo_sec_time = 0;
			}
			else
			{
				motor_demo_loop_count = 0;
				motor_demo_step++;
				motor_demo_ms_time = 0;
				motor_demo_sec_time = 0;
			}		
		}	
		else
		{	
				if(motor_demo_1sec_cont <= demo_run_time * 60)
				{
					motor_demo_step = 9;
					motor_demo_ms_time = 0;
					motor_demo_sec_time = 0;
				}
				else
				{
					motor_demo_loop_count = 0;
					motor_demo_step++;
					motor_demo_ms_time = 0;
					motor_demo_sec_time = 0;
				}
		}
	}
	if(motor_demo_step == 16)
	{
		motor_demo_ms_time = 0;
		motor_demo_sec_time = 0;	
		if(motor_demo_slave_flag == 0)
		{
			motor_demo_step++;
			LIN_Master_Send_DemoStep_Cmd(2, motor_demo_step);
		}	
	}	
	if(motor_demo_step == 17)
	{
		if(User_MusicDemo(2) == 1)
		{
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
			motor_demo_step++;	
		}
	}
	if(motor_demo_step == 18)
	{
		if(User_LightDemo(0) == 1)
		{
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
			motor_demo_step++;	
		}
	}	
	if(motor_demo_step == 19)
	{
		if(motor_demo_sec_time >= 60)
		{
			motor_demo_sec_time = 0;
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
		}
	}
	if(motor_demo_step == 20)
	{
		motor_demo_ms_time = 0;
		motor_demo_sec_time = 0;		
		if(motor_demo_slave_flag == 0)
		{
			motor_demo_step++;
			LIN_Master_Send_DemoStep_Cmd(2, motor_demo_step);
		}
	}
	if(motor_demo_step == 21)
	{
		if(motor_fast_change_flag)
		{
			motor_fast_change_flag = 0;	
			Motor_PwmImStop(Motor_Back_ParaStu);
			Motor_PwmImStop(Motor_Leg_ParaStu);
			Motor_Para_Reset(Motor_Back_ParaStu);
			Motor_Para_Reset(Motor_Leg_ParaStu);
		}
		else
		{		
			Motor_Back_ParaStu->motor_slow_run_flag = 1;
			Motor_Leg_ParaStu->motor_slow_run_flag = 1;					
			if(!Motor_ArrivePosition(Motor_Back_ParaStu,HALL_MIN_NUM))
			{
				motor_complete_flag |= 0x01;
			}	
			if(!Motor_ArrivePosition(Motor_Leg_ParaStu,HALL_MIN_NUM))			
			{
				motor_complete_flag |= 0x02;
			}
			if(motor_complete_flag == 0x03)
			{
				motor_complete_flag = 0x00;
				motor_demo_ms_time = 0;
				motor_demo_sec_time = 0;
				motor_demo_step = 0;
				lin_received_step = 0;
				motor_run_fixed_complate = 1;		
				Motor_Back_ParaStu->motor_slow_run_flag = 0;
				Motor_Leg_ParaStu->motor_slow_run_flag = 0;		
				ble_report_set_event(BLE_REPORT_EVENT, REPORT_DEMO_RUN_EVENT);				
				return 1;					
			}
		}
	}

	return 0;
} 				
unsigned char Motor_Alarm_Mode(void)
{
	unsigned char motor_complete_flag = 0;

	motor_demo_ms_time ++ ;
	if(motor_demo_ms_time >= 200)
	{
		motor_demo_ms_time = 0;
		motor_demo_sec_time ++;
		if(motor_demo_sec_time >= 480)
		{
			motor_demo_sec_time = 480;
		}
	}
	motor_demo_5ms_cont++;
	if(motor_demo_5ms_cont >= 200)
	{
		motor_demo_5ms_cont = 0;
		motor_demo_1sec_cont++;
	}

	Light_Clear_TimeCount(&Light_RgbColour_Stu);
	Msgr_Clear_TimeCount();	

	if(motor_demo_step == 0)
	{
		motor_demo_5ms_cont = 0;
		motor_demo_1sec_cont = 0;
		motor_complete_flag = 0x00;
		Motor_PwmImStop(Motor_Back_ParaStu);
		Motor_PwmImStop(Motor_Leg_ParaStu);
		Motor_PwmImStop(Motor_Lumbar_ParaStu);
		Motor_PwmImStop(Motor_Lumbar2_ParaStu);
		Motor_PwmImStop(Motor_Neck_ParaStu);
		Motor_PwmImStop(Motor_Neck2_ParaStu);
		Motor_PwmImStop(Motor_Tilt1_ParaStu);
		Motor_PwmImStop(Motor_Tilt2_ParaStu);
		Motor_Para_Reset(Motor_Back_ParaStu);
		Motor_Para_Reset(Motor_Leg_ParaStu);
		Motor_Para_Reset(Motor_Lumbar_ParaStu);
		Motor_Para_Reset(Motor_Lumbar2_ParaStu);
		Motor_Para_Reset(Motor_Neck_ParaStu);
		Motor_Para_Reset(Motor_Neck2_ParaStu);
		Motor_Para_Reset(Motor_Tilt1_ParaStu);
		Motor_Para_Reset(Motor_Tilt2_ParaStu);
		if(alarm_mode_value[3] != 0)
		{
			if(User_MusicDemo(4) == 1)
			{
				motor_demo_step++;
			}
		}
		else
		{
			motor_demo_step++;
		}
	}
	if(motor_demo_step == 1)
	{
		if(alarm_mode_value[2] != 0)
		{
			if(User_LightDemo(2) == 1)
			{
				motor_demo_step++;
				motor_demo_ms_time = 0;
				motor_demo_sec_time = 0;
			}
		}
		else
		{
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
		}		
	}
	if(motor_demo_step == 2)
	{
		if(alarm_mode_value[1] != 0)
		{
			Msgr_Ints_FlagArr[0] = MSGR_INTS_ZERO_LEVEL;
			if(1 == TTL_Master_Send_WriteMsgrFollowInts_Cmd(0x00,Msgr_Ints_FlagArr[0]))
			{
				motor_demo_step++;
				motor_demo_ms_time = 0;
				motor_demo_sec_time = 0;	
			}		
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_MASSAGE_INTS_EVENT);

		}
		else
		{
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;	
		}
	}
	if(motor_demo_step == 3)
	{
		if(alarm_mode_value[1] != 0)
		{
			Msgr_Ints_FlagArr[1] = MSGR_INTS_ONE_LEVEL;
			Msgr_Ints_FlagArr[2] = MSGR_INTS_ONE_LEVEL;
			Msgr_Ints_FlagArr[3] = MSGR_INTS_ZERO_LEVEL;
			msgr_mode_set = MSGR_CONSTANT_MODE;
			if(1 == TTL_Master_Send_WriteMsgrModeTimer_Cmd(0X00,msgr_mode_set,msgr_min_time_set))		
			{					
				motor_demo_step++;
				motor_demo_ms_time = 0;
				motor_demo_sec_time = 0;
			}							
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_MASSAGE_MODE_EVENT);

		}
		else
		{
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;	
		}
	}
	if(motor_demo_step == 4)
	{
		if(alarm_mode_value[1] != 0)
		{
			if(1 == TTL_Master_Send_WriteMsgrTypicalInts_Cmd(Msgr_Ints_FlagArr[1],Msgr_Ints_FlagArr[2])) //强度
			{
				motor_demo_step++;
				motor_demo_ms_time = 0;
				motor_demo_sec_time = 0;	
			}		
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_MASSAGE_INTS_EVENT);
		}
		else
		{
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;	
		}
	}
	if(motor_demo_step == 5)
	{
		if(alarm_mode_value[0] != 0)
		{
			Motor_Back_ParaStu->motor_slow_run_flag = 1;
			Motor_Leg_ParaStu->motor_slow_run_flag = 1;						
			if(!Motor_ArrivePosition(Motor_Back_ParaStu,20000))
			{
				Motor_Para_Reset(Motor_Back_ParaStu);
				motor_complete_flag |= 0x01;
			}	
			if(motor_complete_flag == 0x01)
			{		
				Motor_Back_ParaStu->motor_slow_run_flag = 0;
				Motor_Leg_ParaStu->motor_slow_run_flag = 0;						
				motor_complete_flag = 0x00;
				motor_demo_ms_time = 0;
				motor_demo_sec_time = 0;
				motor_demo_step++;
			}
		}
		else
		{
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;	
		}
	}
	if(motor_demo_step == 6)
	{
		if(motor_demo_1sec_cont >= 60)
		{
			motor_demo_1sec_cont = 0;
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;	
		}
	}
	if(motor_demo_step == 7)
	{
		if(alarm_mode_value[2] != 0)
		{
			if(User_LightDemo(3) == 1)
			{
				motor_demo_step++;
				motor_demo_ms_time = 0;
				motor_demo_sec_time = 0;
			}
		}
		else
		{
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
		}		
	}
	if(motor_demo_step == 8)
	{
		if(alarm_mode_value[1] != 0)
		{
			Msgr_Ints_FlagArr[1] = MSGR_INTS_TWO_LEVEL;
			Msgr_Ints_FlagArr[2] = MSGR_INTS_TWO_LEVEL;			
			if(1 == TTL_Master_Send_WriteMsgrTypicalInts_Cmd(Msgr_Ints_FlagArr[1],Msgr_Ints_FlagArr[2])) //强度
			{
				motor_demo_step++;
				motor_demo_ms_time = 0;
				motor_demo_sec_time = 0;	
			}		
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_MASSAGE_INTS_EVENT);
		}
		else
		{
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;	
		}
	}
	if(motor_demo_step == 9)
	{
		if(motor_demo_1sec_cont >= 60)
		{
			motor_demo_1sec_cont = 0;
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;	
		}
	}	
	if(motor_demo_step == 10)
	{
		if(alarm_mode_value[2] != 0)
		{
			if(User_LightDemo(4) == 1)
			{
				motor_demo_step++;
				motor_demo_ms_time = 0;
				motor_demo_sec_time = 0;
			}
		}
		else
		{
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
		}		
	}
	if(motor_demo_step == 11)
	{
		if(alarm_mode_value[1] != 0)
		{
			Msgr_Ints_FlagArr[1] = MSGR_INTS_THREE_LEVEL;
			Msgr_Ints_FlagArr[2] = MSGR_INTS_THREE_LEVEL;			
			if(1 == TTL_Master_Send_WriteMsgrTypicalInts_Cmd(Msgr_Ints_FlagArr[1],Msgr_Ints_FlagArr[2])) //强度
			{
				motor_demo_step++;
				motor_demo_ms_time = 0;
				motor_demo_sec_time = 0;	
			}		
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_MASSAGE_INTS_EVENT);
		}
		else
		{
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;	
		}
	}
	if(motor_demo_step == 12)
	{
		if(motor_demo_1sec_cont >= 60)
		{
			motor_demo_1sec_cont = 0;
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;	
		}
	}	
	if(motor_demo_step == 13)
	{
		if(alarm_mode_value[3] != 0)
		{
			if(User_MusicDemo(2) == 1)
			{
				motor_demo_step++;
				motor_demo_ms_time = 0;
				motor_demo_sec_time = 0;				
			}
		}
		else
		{
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;			
		}
	}
	if(motor_demo_step == 14)
	{
		if(alarm_mode_value[2] != 0)
		{
			if(User_LightDemo(0) == 1)
			{
				motor_demo_step++;
				motor_demo_ms_time = 0;
				motor_demo_sec_time = 0;
			}
		}
		else
		{
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
		}		
	}
	if(motor_demo_step == 15)
	{
		if(alarm_mode_value[1] != 0)
		{
			Msgr_Ints_FlagArr[1] = MSGR_INTS_ZERO_LEVEL;
			Msgr_Ints_FlagArr[2] = MSGR_INTS_ZERO_LEVEL;		
			msgr_mode_set = MSGR_FOLLOW_MODE;	
			if(1 == TTL_Master_Send_WriteMsgrTypicalInts_Cmd(Msgr_Ints_FlagArr[1],Msgr_Ints_FlagArr[2])) //强度
			{
				motor_demo_step = 0;
				motor_demo_ms_time = 0;
				motor_demo_sec_time = 0;	
				motor_run_fixed_complate = 1;	
				return 1;
			}		
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_MASSAGE_INTS_EVENT);
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_MASSAGE_MODE_EVENT);
		}
		else
		{
			motor_demo_step = 0;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;	
			motor_run_fixed_complate = 1;	
			return 1;
		}
	}

	return 0;
}
unsigned char Motor_Demo_Help_Sleep_Tilt_SSB_Mode(void)
{
	unsigned char motor_complete_flag = 0;
	unsigned char motor_fast_change_flag = 0;
	motor_demo_ms_time ++ ;
	if(motor_demo_ms_time >= 200)
	{
		motor_demo_ms_time = 0;
		motor_demo_sec_time ++;
		if(motor_demo_sec_time >= 480)
		{
			motor_demo_sec_time = 480;
		}
	}
	motor_demo_5ms_cont++;
	if(motor_demo_5ms_cont >= 200)
	{
		motor_demo_5ms_cont = 0;
		motor_demo_1sec_cont++;
	}
	Msgr_Clear_TimeCount();
	Light_Clear_TimeCount(&Light_RgbColour_Stu);
	Music_Time_ClearCount();
	// 更新步骤逻辑
	if(motor_demo_slave_flag == 1)
	{
		if((lin_received_step != motor_demo_step) && (lin_received_step != 0))
		{
				// 当前步骤未完成就先停止电机
			Motor_PwmImStop(Motor_Back_ParaStu);
			Motor_PwmImStop(Motor_Leg_ParaStu);
			Motor_PwmImStop(Motor_Lumbar_ParaStu);
			Motor_PwmImStop(Motor_Lumbar2_ParaStu);
			Motor_PwmImStop(Motor_Neck_ParaStu);
			Motor_PwmImStop(Motor_Neck2_ParaStu);
			Motor_PwmImStop(Motor_Tilt1_ParaStu);
			Motor_PwmImStop(Motor_Tilt2_ParaStu);
			// 再进入下一步
			motor_demo_step = lin_received_step;
			motor_fast_change_flag = 1;
			lin_received_step = 0;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
		}
	}

	if(motor_demo_step == 0)
	{
		motor_demo_loop_count = 0;
		motor_demo_5ms_cont = 0;
		motor_demo_1sec_cont = 0;
		Motor_PwmImStop(Motor_Back_ParaStu);
		Motor_PwmImStop(Motor_Leg_ParaStu);
		Motor_PwmImStop(Motor_Lumbar_ParaStu);
		Motor_PwmImStop(Motor_Lumbar2_ParaStu);
		Motor_PwmImStop(Motor_Neck_ParaStu);
		Motor_PwmImStop(Motor_Neck2_ParaStu);
		Motor_PwmImStop(Motor_Tilt1_ParaStu);
		Motor_PwmImStop(Motor_Tilt2_ParaStu);
		ble_report_set_event(BLE_REPORT_EVENT, REPORT_DEMO_RUN_EVENT);
		Light_OneColour_Stu.led_colour_state = 0;
		led_board_state = 0;		
		Music_SW(SWITCH_CTR_OFF);//音响关闭
		if(User_LightDemo(1) == 1)
		{
			motor_demo_step++;
		}
	}
	if(motor_demo_step == 1)
	{
		if(User_MusicDemo(1) == 1)
		{
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
		}	
	}
	if(motor_demo_step == 2)
	{
		if(User_Msgr_Demo(0) == 1)
		{
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
		}	
	}
	if(motor_demo_step == 3)
	{
		if(User_MusicDemo(3) == 1)
		{
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
		}
	}
	if(motor_demo_step == 4)
	{
		motor_demo_step++;
	}
	if(motor_demo_step == 5)
	{
		if(!Motor_ArrivePosition(Motor_Back_ParaStu,BACK_DEFAULT_SLEEP_HALL_NUM))
		{
			Motor_Para_Reset(Motor_Back_ParaStu);
			motor_complete_flag |= 0x01;
		}
		if(!Motor_ArrivePosition(Motor_Leg_ParaStu,LEG_DEFAULT_SLEEP_HALL_NUM))
		{
			Motor_Para_Reset(Motor_Leg_ParaStu);
			motor_complete_flag |= 0x02;
		}
		if(!Motor_ArrivePosition(Motor_Lumbar_ParaStu,LUMBAR_DEFAULT_SLEEP_HALL_NUM))
		{
			Motor_Para_Reset(Motor_Lumbar_ParaStu);
			motor_complete_flag |= 0x04;
		}
		if(!Motor_ArrivePosition(Motor_Lumbar2_ParaStu,LUMBAR2_DEFAULT_SLEEP_HALL_NUM))
		{
			Motor_Para_Reset(Motor_Lumbar2_ParaStu);
			motor_complete_flag |= 0x08;
		}
		if(!Motor_ArrivePosition(Motor_Neck_ParaStu,NECK_DEFAULT_SLEEP_HALL_NUM))
		{
			Motor_Para_Reset(Motor_Neck_ParaStu);
			motor_complete_flag |= 0x10;
		}			
		if(!Motor_ArrivePosition(Motor_Neck2_ParaStu,NECK2_DEFAULT_SLEEP_HALL_NUM))
		{
			Motor_Para_Reset(Motor_Neck2_ParaStu);
			motor_complete_flag |= 0x20;
		}	
		if(!Motor_ArrivePosition(Motor_Tilt1_ParaStu,HALL_MIN_NUM))
		{
			Motor_Para_Reset(Motor_Tilt1_ParaStu);
			motor_complete_flag |= 0x40;
		}
		if(!Motor_ArrivePosition(Motor_Tilt2_ParaStu,HALL_MIN_NUM))
		{
			Motor_Para_Reset(Motor_Tilt2_ParaStu);
			motor_complete_flag |= 0x80;
		}
		if((motor_complete_flag & 0xff) == 0xff)
		{
			motor_complete_flag = 0x00;
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
		}
	}
	if(motor_demo_step == 6)
	{
		if(motor_demo_sec_time >= 1)
		{
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;		
			if(motor_demo_slave_flag == 0)
			{
				motor_demo_step++;
				LIN_Master_Send_DemoStep_Cmd(2, motor_demo_step);
			}	
		}	
	}	
	if(motor_demo_step == 7)//原速度同时运行, 升降1到位, 升降2到达升降2霍尔值升降1和升降2电机都停止后, 计时5S; 
	{
		if(motor_fast_change_flag)
		{
			motor_fast_change_flag = 0;	
			Motor_PwmImStop(Motor_Back_ParaStu);
			Motor_PwmImStop(Motor_Leg_ParaStu);
			Motor_PwmImStop(Motor_Lumbar_ParaStu);
			Motor_PwmImStop(Motor_Lumbar2_ParaStu);
			Motor_PwmImStop(Motor_Neck_ParaStu);
			Motor_PwmImStop(Motor_Neck2_ParaStu);
			Motor_PwmImStop(Motor_Tilt1_ParaStu);
			Motor_PwmImStop(Motor_Tilt2_ParaStu);

			Motor_Para_Reset(Motor_Lumbar_ParaStu);
			Motor_Para_Reset(Motor_Lumbar2_ParaStu);
			Motor_Para_Reset(Motor_Neck_ParaStu);
			Motor_Para_Reset(Motor_Neck2_ParaStu);
			Motor_Para_Reset(Motor_Back_ParaStu);
			Motor_Para_Reset(Motor_Leg_ParaStu);
			Motor_Para_Reset(Motor_Tilt1_ParaStu);
			Motor_Para_Reset(Motor_Tilt2_ParaStu);

		}
		else
		{
			if(!Motor_ArrivePosition(Motor_Tilt1_ParaStu,TILT_ONE_SLEEP_HALL_NUM))
			{
				motor_complete_flag |= 0x01;
			} 
			if(!Motor_ArrivePosition(Motor_Tilt2_ParaStu,HALL_MIN_NUM))
			{
				motor_complete_flag |= 0x02;
			}				
			if((motor_complete_flag & 0x03) == 0x03)
			{
				motor_demo_step++;
				motor_complete_flag = 0;
				motor_demo_ms_time = 0;
				motor_demo_sec_time = 0;
			}
		}
	}
	if(motor_demo_step == 8)
	{
		Motor_PwmImStop(Motor_Tilt1_ParaStu);
		Motor_PwmImStop(Motor_Tilt2_ParaStu);
		if(motor_demo_sec_time >= 5)
		{
			motor_demo_sec_time = 0;
			motor_demo_ms_time = 0;
			motor_demo_step++;
		}
	}
	if(motor_demo_step == 9)
	{
		motor_demo_ms_time = 0;
		motor_demo_sec_time = 0;	
		if(motor_demo_slave_flag == 0)
		{
			motor_demo_step++;
			LIN_Master_Send_DemoStep_Cmd(2, motor_demo_step);
		}
	}	
		//.升降1升降2电机同时运行，升降2电机50%速度下降至停止（若升降1电机先停止则升降2无需到达就停止）；升降1电机50%速度上升至后, 停止, 计时2S
	if(motor_demo_step == 10)
	{
		if(motor_fast_change_flag)
		{
			motor_fast_change_flag = 0;	
			Motor_PwmImStop(Motor_Tilt1_ParaStu);
			Motor_PwmImStop(Motor_Tilt2_ParaStu);
			Motor_Para_Reset(Motor_Tilt1_ParaStu);
			Motor_Para_Reset(Motor_Tilt2_ParaStu);		
		}
		else
		{			
			Motor_Tilt1_ParaStu->motor_slow_run_flag = 1;
			Motor_Tilt2_ParaStu->motor_slow_run_flag = 1;
			if(!Motor_ArrivePosition(Motor_Tilt1_ParaStu,HALL_MIN_NUM))
			{
				motor_complete_flag |= 0x01;
			} 				
			if((!Motor_ArrivePosition(Motor_Tilt2_ParaStu,TILT_TWO_SLEEP_HALL_NUM)) || (motor_complete_flag == 0x01))
			{
				motor_complete_flag |= 0x02;
				Motor_PwmImStop(Motor_Tilt2_ParaStu);
			}
			if(motor_complete_flag == 0x03)
			{
				motor_complete_flag = 0x00;
				motor_demo_step++;
				motor_demo_ms_time = 0;
				motor_demo_sec_time = 0;
			}
		}
	}
	if(motor_demo_step == 11)
	{
		Motor_PwmImStop(Motor_Tilt1_ParaStu);
		Motor_PwmImStop(Motor_Tilt2_ParaStu);
		if(motor_demo_sec_time >= 5)
		{
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
		}
	}
	if(motor_demo_step == 12)
	{
		motor_demo_ms_time = 0;
		motor_demo_sec_time = 0;
		if(motor_demo_slave_flag == 0)
		{
			motor_demo_step++;
			LIN_Master_Send_DemoStep_Cmd(2, motor_demo_step);
		}
	}	
		//计时完成后,升降1升降2电机同时运行，升降2电机50%速度上升后停止（若升降1电机先停止则升降2无需到达就停止）;
	//升降1电机50%速度下降至后,停止, 计时2S;
	if(motor_demo_step == 13)
	{
		if(motor_fast_change_flag)
		{
			motor_fast_change_flag = 0;	
			Motor_PwmImStop(Motor_Tilt1_ParaStu);
			Motor_PwmImStop(Motor_Tilt2_ParaStu);
			Motor_Para_Reset(Motor_Tilt1_ParaStu);
			Motor_Para_Reset(Motor_Tilt2_ParaStu);						
		}
		else
		{			
			Motor_Tilt1_ParaStu->motor_slow_run_flag = 1;
			Motor_Tilt2_ParaStu->motor_slow_run_flag = 1;		
			if(!Motor_ArrivePosition(Motor_Tilt1_ParaStu,TILT_ONE_SLEEP_HALL_NUM))
			{
				motor_complete_flag |= 0x01;
			} 
			if((!Motor_ArrivePosition(Motor_Tilt2_ParaStu,HALL_MIN_NUM)) || (motor_complete_flag == 0x01))
			{
				motor_complete_flag |= 0x02;
				Motor_PwmImStop(Motor_Tilt2_ParaStu);
			}
			if(motor_complete_flag == 0x03)
			{
				motor_complete_flag = 0x00;
				motor_demo_step++;
				motor_demo_ms_time = 0;
				motor_demo_sec_time = 0;
			}
		}
	}	
	if(motor_demo_step == 14)
	{
		Motor_PwmImStop(Motor_Tilt1_ParaStu);
		Motor_PwmImStop(Motor_Tilt2_ParaStu);	
		if(motor_demo_sec_time >= 5)
		{
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
		}
	}
	if(motor_demo_step == 15)
	{	
		if(system_config.flags.help_sleep_loop == DEMO_NUM_LOOP)
		{
			motor_demo_loop_count++;
			if(motor_demo_loop_count <= 25)
			{
				motor_demo_step = 9;
				motor_demo_ms_time = 0;
				motor_demo_sec_time = 0;
			}
			else
			{
				motor_demo_loop_count = 0;
				motor_demo_step++;
				motor_demo_ms_time = 0;
				motor_demo_sec_time = 0;
			}		
		}
		else
		{
			if(motor_demo_1sec_cont <= demo_run_time * 60)
			{
				motor_demo_step = 9;
				motor_demo_ms_time = 0;
				motor_demo_sec_time = 0;
			}
			else
			{
				motor_demo_loop_count = 0;
				motor_demo_step++;
				motor_demo_ms_time = 0;
				motor_demo_sec_time = 0;
			}
		}
	}
	if(motor_demo_step == 16)
	{
		motor_demo_ms_time = 0;
		motor_demo_sec_time = 0;		
		if(motor_demo_slave_flag == 0)
		{
			motor_demo_step++;
			LIN_Master_Send_DemoStep_Cmd(2, motor_demo_step);
		}	
	}	
	if(motor_demo_step == 17)
	{
		if(User_MusicDemo(2) == 1)
		{
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
			motor_demo_step++;	
		}
	}
	if(motor_demo_step == 18)
	{
		if(User_LightDemo(0) == 1)
		{
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
			motor_demo_step++;	
		}
	}
	if(motor_demo_step == 19)
	{
		if(motor_demo_sec_time >= 60)
		{
			motor_demo_sec_time = 0;
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
		}
	}
	if(motor_demo_step == 20)
	{
		motor_demo_ms_time = 0;
		motor_demo_sec_time = 0;		
		if(motor_demo_slave_flag == 0)
		{
			motor_demo_step++;
			LIN_Master_Send_DemoStep_Cmd(2, motor_demo_step);
		}	
	}
	if(motor_demo_step == 21)
	{
		if(motor_fast_change_flag)
		{
			motor_fast_change_flag = 0;	
			Motor_PwmImStop(Motor_Tilt1_ParaStu);
			Motor_PwmImStop(Motor_Tilt2_ParaStu);
			Motor_Para_Reset(Motor_Tilt1_ParaStu);
			Motor_Para_Reset(Motor_Tilt2_ParaStu);				
		}
		else
		{			
			Motor_Tilt1_ParaStu->motor_slow_run_flag = 1;
			Motor_Tilt2_ParaStu->motor_slow_run_flag = 1;				
			if(!Motor_ArrivePosition(Motor_Tilt1_ParaStu,HALL_MIN_NUM))
			{
				motor_complete_flag |= 0x01;
			}
			if(!Motor_ArrivePosition(Motor_Tilt2_ParaStu,HALL_MIN_NUM))			
			{
				motor_complete_flag |= 0x02;
			}
			if(motor_complete_flag == 0x03)
			{
					Motor_Tilt1_ParaStu->motor_slow_run_flag = 0;
					Motor_Tilt2_ParaStu->motor_slow_run_flag = 0;		
					motor_complete_flag = 0x00;
					motor_demo_ms_time = 0;
					motor_demo_sec_time = 0;
					motor_demo_step = 0;
					lin_received_step = 0;
					motor_run_fixed_complate = 1;		
					ble_report_set_event(BLE_REPORT_EVENT, REPORT_DEMO_RUN_EVENT);
					return 1;					
			}
			
		}
	}
	return 0;
}
unsigned char Motor_Demo_Tilt_Mode(void)
{
	unsigned char motor_complete_flag = 0;
	unsigned char motor_fast_change_flag = 0;

	motor_demo_ms_time ++ ;
	if(motor_demo_ms_time >= 200)
	{
		motor_demo_ms_time = 0;
		motor_demo_sec_time ++;
		if(motor_demo_sec_time >= 480)
		{
			motor_demo_sec_time = 480;
		}
	}
	motor_demo_5ms_cont++;
	if(motor_demo_5ms_cont >= 200)
	{
		motor_demo_5ms_cont = 0;
		motor_demo_1sec_cont++;
	}
	Msgr_Clear_TimeCount();
	Music_Time_ClearCount();
	Light_Clear_TimeCount(&Light_RgbColour_Stu);
	
	// 更新步骤逻辑
	if(motor_demo_slave_flag == 1)
	{
		if((lin_received_step != motor_demo_step) && (lin_received_step != 0))
		{
				// 当前步骤未完成就先停止电机
			Motor_PwmImStop(Motor_Back_ParaStu);
			Motor_PwmImStop(Motor_Leg_ParaStu);
			Motor_PwmImStop(Motor_Lumbar_ParaStu);
			Motor_PwmImStop(Motor_Lumbar2_ParaStu);
			Motor_PwmImStop(Motor_Neck_ParaStu);
			Motor_PwmImStop(Motor_Neck2_ParaStu);
			Motor_PwmImStop(Motor_Tilt1_ParaStu);
			Motor_PwmImStop(Motor_Tilt2_ParaStu);
			// 再进入下一步
			motor_demo_step = lin_received_step;
			motor_fast_change_flag = 1;
			lin_received_step = 0;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
		}
	}
	if(motor_demo_step == 0)
	{
		motor_demo_loop_count = 0;
		motor_demo_5ms_cont = 0;
		motor_demo_1sec_cont = 0;
		Motor_PwmImStop(Motor_Back_ParaStu);
		Motor_PwmImStop(Motor_Leg_ParaStu);
		Motor_PwmImStop(Motor_Lumbar_ParaStu);
		Motor_PwmImStop(Motor_Lumbar2_ParaStu);
		Motor_PwmImStop(Motor_Neck_ParaStu);
		Motor_PwmImStop(Motor_Neck2_ParaStu);
		Motor_PwmImStop(Motor_Tilt1_ParaStu);
		Motor_PwmImStop(Motor_Tilt2_ParaStu);
		Light_OneColour_Stu.led_colour_state = 0;
		led_board_state = 0;
		Music_SW(SWITCH_CTR_OFF);//音响关闭
		if(User_Msgr_Demo(0) == 1)
		{
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_DEMO_RUN_EVENT);
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;	
		}		
	}
	if(motor_demo_step == 1)
	{
			motor_demo_step++;
	}
	if(motor_demo_step == 2)
	{
		if(User_MusicDemo(0) == 1)
		{
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;	
		}		
	}
	if(motor_demo_step == 3)
	{
		motor_demo_step++;
	}	
	if(motor_demo_step == 4)
	{
		if(User_LightDemo(0) == 1)
		{
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;	
		}		
	}
	if(motor_demo_step == 5)
	{
		if(!Motor_ArrivePosition(Motor_Back_ParaStu,BACK_DEFAULT_SLEEP_HALL_NUM))
		{
			Motor_Para_Reset(Motor_Back_ParaStu);
			motor_complete_flag |= 0x01;
		}
		if(!Motor_ArrivePosition(Motor_Leg_ParaStu,LEG_DEFAULT_SLEEP_HALL_NUM))
		{
			Motor_Para_Reset(Motor_Leg_ParaStu);
			motor_complete_flag |= 0x02;
		}
		if(!Motor_ArrivePosition(Motor_Lumbar_ParaStu,LUMBAR_DEFAULT_SLEEP_HALL_NUM))
		{
			Motor_Para_Reset(Motor_Lumbar_ParaStu);
			motor_complete_flag |= 0x04;
		}
		if(!Motor_ArrivePosition(Motor_Neck_ParaStu,NECK_DEFAULT_SLEEP_HALL_NUM))
		{
			Motor_Para_Reset(Motor_Neck_ParaStu);
			motor_complete_flag |= 0x08;
		}
		if(!Motor_ArrivePosition(Motor_Lumbar2_ParaStu,LUMBAR2_DEFAULT_SLEEP_HALL_NUM))
		{
			Motor_Para_Reset(Motor_Lumbar2_ParaStu);
			motor_complete_flag |= 0x10;
		}			
		if(!Motor_ArrivePosition(Motor_Neck2_ParaStu,NECK2_DEFAULT_SLEEP_HALL_NUM))
		{
			Motor_Para_Reset(Motor_Neck2_ParaStu);
			motor_complete_flag |= 0x20;
		}	
		if(!Motor_ArrivePosition(Motor_Tilt1_ParaStu,HALL_MIN_NUM))
		{
			Motor_Para_Reset(Motor_Tilt1_ParaStu);
			motor_complete_flag |= 0x40;
		}
		if(!Motor_ArrivePosition(Motor_Tilt2_ParaStu,HALL_MIN_NUM))
		{
			Motor_Para_Reset(Motor_Tilt2_ParaStu);
			motor_complete_flag |= 0x80;
		}
		if((motor_complete_flag & 0xff) == 0xff)
		{
			motor_complete_flag = 0x00;
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
		}
	}
	if(motor_demo_step == 6)
	{
		if(motor_demo_sec_time >= 1)
		{
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;		
			if(motor_demo_slave_flag == 0)
			{
				motor_demo_step++;
				LIN_Master_Send_DemoStep_Cmd(2, motor_demo_step);
			}	
		}
	}
	if(motor_demo_step == 7)//原速度同时运行, 升降1到位, 升降2到达升降2霍尔值升降1和升降2电机都停止后, 计时5S; 
	{
		if(motor_fast_change_flag)
		{
			motor_fast_change_flag = 0;	
			Motor_PwmImStop(Motor_Tilt1_ParaStu);
			Motor_PwmImStop(Motor_Tilt2_ParaStu);
			Motor_PwmImStop(Motor_Lumbar_ParaStu);
			Motor_PwmImStop(Motor_Lumbar2_ParaStu);
			Motor_PwmImStop(Motor_Neck_ParaStu);
			Motor_PwmImStop(Motor_Neck2_ParaStu);
			Motor_PwmImStop(Motor_Back_ParaStu);
			Motor_PwmImStop(Motor_Leg_ParaStu);
			Motor_Para_Reset(Motor_Tilt1_ParaStu);
			Motor_Para_Reset(Motor_Tilt2_ParaStu);
			Motor_Para_Reset(Motor_Lumbar_ParaStu);
			Motor_Para_Reset(Motor_Lumbar2_ParaStu);
			Motor_Para_Reset(Motor_Neck_ParaStu);
			Motor_Para_Reset(Motor_Neck2_ParaStu);
			Motor_Para_Reset(Motor_Back_ParaStu);
			Motor_Para_Reset(Motor_Leg_ParaStu);
		}
		else
		{
			if(!Motor_ArrivePosition(Motor_Tilt1_ParaStu,TILT_ONE_SLEEP_HALL_NUM))
			{
				motor_complete_flag |= 0x01;
			} 
			if(!Motor_ArrivePosition(Motor_Tilt2_ParaStu,HALL_MIN_NUM))
			{
				motor_complete_flag |= 0x02;
			}				
			if((motor_complete_flag & 0x03) == 0x03)
			{
				motor_demo_step++;
				motor_complete_flag = 0;
				motor_demo_ms_time = 0;
				motor_demo_sec_time = 0;
			}
		}
	}
	if(motor_demo_step == 8)
	{
		Motor_PwmImStop(Motor_Tilt1_ParaStu);
		Motor_PwmImStop(Motor_Tilt2_ParaStu);
		if(motor_demo_sec_time >= 5)
		{
			motor_demo_sec_time = 0;
			motor_demo_ms_time = 0;
			motor_demo_step++;
		}
	}
	if(motor_demo_step == 9)
	{
		motor_demo_ms_time = 0;
		motor_demo_sec_time = 0;		
		if(motor_demo_slave_flag == 0)
		{
			motor_demo_step++;
			LIN_Master_Send_DemoStep_Cmd(2, motor_demo_step);
		}	
	}	
		//.升降1升降2电机同时运行，升降2电机50%速度下降至停止（若升降1电机先停止则升降2无需到达就停止）；升降1电机50%速度上升至后, 停止, 计时2S
	if(motor_demo_step == 10)
	{
		if(motor_fast_change_flag)
		{
			motor_fast_change_flag = 0;	
			Motor_PwmImStop(Motor_Tilt1_ParaStu);
			Motor_PwmImStop(Motor_Tilt2_ParaStu);
			Motor_Para_Reset(Motor_Tilt1_ParaStu);
			Motor_Para_Reset(Motor_Tilt2_ParaStu);
		}
		else
		{		
			Motor_Tilt1_ParaStu->motor_slow_run_flag = 1;
			Motor_Tilt2_ParaStu->motor_slow_run_flag = 1;
			if(!Motor_ArrivePosition(Motor_Tilt1_ParaStu,HALL_MIN_NUM))
			{
				motor_complete_flag |= 0x01;
			} 			
			if((!Motor_ArrivePosition(Motor_Tilt2_ParaStu,TILT_TWO_SLEEP_HALL_NUM)) || (motor_complete_flag == 0x01))
			{
				motor_complete_flag |= 0x02;
				Motor_PwmImStop(Motor_Tilt2_ParaStu);
			}
			if(motor_complete_flag == 0x03)
			{
				motor_complete_flag = 0x00;
				motor_demo_step++;
				motor_demo_ms_time = 0;
				motor_demo_sec_time = 0;
			}
		}
	}
	if(motor_demo_step == 11)
	{
		Motor_PwmImStop(Motor_Tilt1_ParaStu);
		Motor_PwmImStop(Motor_Tilt2_ParaStu);
		if(motor_demo_sec_time >= 5)
		{
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
		}
	}
	if(motor_demo_step == 12)
	{
		motor_demo_ms_time = 0;
		motor_demo_sec_time = 0;		
		if(motor_demo_slave_flag == 0)
		{
			motor_demo_step++;
			LIN_Master_Send_DemoStep_Cmd(2, motor_demo_step);
		}
	}	
		//计时完成后,升降1升降2电机同时运行，升降2电机50%速度上升后停止（若升降1电机先停止则升降2无需到达就停止）;
	//升降1电机50%速度下降至后,停止, 计时2S;
	if(motor_demo_step == 13)
	{
		if(motor_fast_change_flag)
		{
			motor_fast_change_flag = 0;	
			Motor_PwmImStop(Motor_Tilt1_ParaStu);
			Motor_PwmImStop(Motor_Tilt2_ParaStu);
			Motor_Para_Reset(Motor_Tilt1_ParaStu);
			Motor_Para_Reset(Motor_Tilt2_ParaStu);
		}
		else
		{		
			Motor_Tilt1_ParaStu->motor_slow_run_flag = 1;
			Motor_Tilt2_ParaStu->motor_slow_run_flag = 1;		
			if(!Motor_ArrivePosition(Motor_Tilt1_ParaStu,TILT_ONE_SLEEP_HALL_NUM))
			{
				motor_complete_flag |= 0x01;
			} 		
			if((!Motor_ArrivePosition(Motor_Tilt2_ParaStu,HALL_MIN_NUM)) || (motor_complete_flag == 0x01))
			{
				motor_complete_flag |= 0x02;
				Motor_PwmImStop(Motor_Tilt2_ParaStu);
			}
			if(motor_complete_flag == 0x03)
			{
				motor_complete_flag = 0x00;
				motor_demo_step++;
				motor_demo_ms_time = 0;
				motor_demo_sec_time = 0;
			}
		}
	}	
	if(motor_demo_step == 14)
	{
		Motor_PwmImStop(Motor_Tilt1_ParaStu);
		Motor_PwmImStop(Motor_Tilt2_ParaStu);	
		if(motor_demo_sec_time >= 5)
		{
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
		}
	}
	if(motor_demo_step == 15)
	{	
		if(system_config.flags.help_sleep_loop == DEMO_NUM_LOOP)
		{
			motor_demo_loop_count++;
			if(motor_demo_loop_count <= 25)
			{
				motor_demo_step = 9;
				motor_demo_ms_time = 0;
				motor_demo_sec_time = 0;
			}
			else
			{
				motor_demo_loop_count = 0;
				motor_demo_step++;
				motor_demo_ms_time = 0;
				motor_demo_sec_time = 0;
			}		
		}	
		else
		{	
				if(motor_demo_1sec_cont <= demo_run_time * 60)
				{
					motor_demo_step = 9;
					motor_demo_ms_time = 0;
					motor_demo_sec_time = 0;
				}
				else
				{
					motor_demo_loop_count = 0;
					motor_demo_step++;
					motor_demo_ms_time = 0;
					motor_demo_sec_time = 0;
				}
		}
	}
	if(motor_demo_step == 16)
	{
		motor_demo_ms_time = 0;
		motor_demo_sec_time = 0;	
		if(motor_demo_slave_flag == 0)
		{
			motor_demo_step++;
			LIN_Master_Send_DemoStep_Cmd(2, motor_demo_step);
		}	
	}	
	if(motor_demo_step == 17)
	{
		if(User_MusicDemo(2) == 1)
		{
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
			motor_demo_step++;	
		}
	}
	if(motor_demo_step == 18)
	{
		if(User_LightDemo(0) == 1)
		{
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
			motor_demo_step++;	
		}
	}	
	if(motor_demo_step == 19)
	{
		if(motor_demo_sec_time >= 60)
		{
			motor_demo_sec_time = 0;
			motor_demo_step++;
			motor_demo_ms_time = 0;
			motor_demo_sec_time = 0;
		}
	}
	if(motor_demo_step == 20)
	{
		motor_demo_ms_time = 0;
		motor_demo_sec_time = 0;		
		if(motor_demo_slave_flag == 0)
		{
			motor_demo_step++;
			LIN_Master_Send_DemoStep_Cmd(2, motor_demo_step);
		}
	}
	if(motor_demo_step == 21)
	{
		if(motor_fast_change_flag)
		{
			motor_fast_change_flag = 0;	
			Motor_PwmImStop(Motor_Tilt1_ParaStu);
			Motor_PwmImStop(Motor_Tilt2_ParaStu);
			Motor_Para_Reset(Motor_Tilt1_ParaStu);
			Motor_Para_Reset(Motor_Tilt2_ParaStu);
		}
		else
		{		
			Motor_Tilt1_ParaStu->motor_slow_run_flag = 1;
			Motor_Tilt2_ParaStu->motor_slow_run_flag = 1;					
			if(!Motor_ArrivePosition(Motor_Tilt1_ParaStu,HALL_MIN_NUM))
			{
				motor_complete_flag |= 0x01;
			}	
			if(!Motor_ArrivePosition(Motor_Tilt2_ParaStu,HALL_MIN_NUM))			
			{
				motor_complete_flag |= 0x02;
			}
			if(motor_complete_flag == 0x03)
			{
				motor_complete_flag = 0x00;
				motor_demo_ms_time = 0;
				motor_demo_sec_time = 0;
				motor_demo_step = 0;
				lin_received_step = 0;
				motor_run_fixed_complate = 1;		
				Motor_Tilt1_ParaStu->motor_slow_run_flag = 0;
				Motor_Tilt2_ParaStu->motor_slow_run_flag = 0;		
				ble_report_set_event(BLE_REPORT_EVENT, REPORT_DEMO_RUN_EVENT);				
				return 1;					
			}
		}
	}

	return 0;
} 				

//闹钟三次动作: 复位→记忆位置, 循环3次后最终复位
//使用Command_GoFlat_Shape做复位, Command_GoFixed_Shape跑目标位置
unsigned char Motor_AlarmThree_Mode(void)
{
	SAVE_MOTOR_INFO_TYPE info_type;

	if(alarm_three_phase == 0) //初始化
	{
		Motor_PwmImStop(Motor_Back_ParaStu);
		Motor_PwmImStop(Motor_Leg_ParaStu);
		Motor_PwmImStop(Motor_Lumbar_ParaStu);
		Motor_PwmImStop(Motor_Lumbar2_ParaStu);
		Motor_PwmImStop(Motor_Neck_ParaStu);
		Motor_PwmImStop(Motor_Neck2_ParaStu);
		Motor_Para_Reset(Motor_Back_ParaStu);
		Motor_Para_Reset(Motor_Leg_ParaStu);
		Motor_Para_Reset(Motor_Lumbar_ParaStu);
		Motor_Para_Reset(Motor_Lumbar2_ParaStu);
		Motor_Para_Reset(Motor_Neck_ParaStu);
		Motor_Para_Reset(Motor_Neck2_ParaStu);
		alarm_three_count = 0;
		alarm_three_phase = 1;
	}
	else if(alarm_three_phase == 1) //FLAT复位
	{
		if(1 == Command_GoFlat_Shape())
		{
			alarm_three_phase = 2;
		}
	}
	else if(alarm_three_phase == 2) //去目标位置
	{
		//键值→SAVE_MOTOR_INFO_TYPE映射
		if(alarm_three_target_mode == KEY_GO_TV)           info_type = MOTOR_TV_INFO;
		else if(alarm_three_target_mode == KEY_GO_ZEROG)   info_type = MOTOR_ZEROG_INFO;
		else if(alarm_three_target_mode == KEY_GO_LOUNGE)  info_type = MOTOR_LOUNGE_INFO;
		else if(alarm_three_target_mode == KEY_GO_SNORE)   info_type = MOTOR_SNORE_INFO;
		else if(alarm_three_target_mode == KEY_GO_READ)    info_type = MOTOR_READ_INFO;
		else if(alarm_three_target_mode == KEY_GO_YOGA)    info_type = MOTOR_YOGA_INFO;
		else if(alarm_three_target_mode == KEY_GO_GETUP)   info_type = MOTOR_GETUP_INFO;
		else if(alarm_three_target_mode == KEY_GO_M1)      info_type = MOTOR_M1_INFO;
		else if(alarm_three_target_mode == KEY_GO_M2)      info_type = MOTOR_M2_INFO;
		else if(alarm_three_target_mode == KEY_GO_M3)      info_type = MOTOR_M3_INFO;
		else { alarm_three_phase = 0; return 0; } //无效目标,跳到最终复位退出

		if(1 == Command_GoFixed_Shape(info_type))
		{
			alarm_three_count++;
			if(alarm_three_count >= 3)
				alarm_three_phase = 3; //最终复位
			else
				alarm_three_phase = 1; //下一轮FLAT
		}
	}
	else if(alarm_three_phase == 3) //最终FLAT
	{
		if(1 == Command_GoFlat_Shape())
		{
			alarm_three_phase = 0;
			alarm_three_count = 0;
			motor_run_fixed_complate = 1;
			return 1;
		}
	}
	return 0;
}

void Motor_DemoMode_ClearPara(void)
{
	Motor_Demo1Mode_ClearPara();
}
void Motor_Demo_ClearTime(void)
{	
	motor_demo_1sec_cont = 0;
	motor_demo_5ms_cont = 0;
}
void Motor_Step_ClearPara(void)
{
	motor_demo_5ms_cont = 0;
	motor_demo_1sec_cont = 0;	
	motor_demo_loop_count = 0;
	motor_demo_step = 0;
	lin_received_step = 0;
	motor_demo_ms_time = 0;
	motor_demo_sec_time = 0;
	Motor_Back_ParaStu->motor_slow_run_flag = 0;
	Motor_Leg_ParaStu->motor_slow_run_flag = 0;		
	Motor_Lumbar_ParaStu->motor_slow_run_flag = 0;
	Motor_Neck_ParaStu->motor_slow_run_flag = 0;	
	Motor_Lumbar2_ParaStu->motor_slow_run_flag = 0;
	Motor_Neck2_ParaStu->motor_slow_run_flag = 0;
	Motor_Tilt1_ParaStu->motor_slow_run_flag = 0;
	Motor_Tilt2_ParaStu->motor_slow_run_flag = 0;
	
	alarm_three_phase = 0;
	alarm_three_count = 0;		
}
void Motor_Demo1Mode_ClearPara(void)
{
	// KEY_ALARM_THREE_MODE 不控制按摩器, 打断时不关闭按摩器
	if(GetSet_Motor_Ctr_Cmd(0xff) != KEY_ALARM_THREE_MODE)
	{
		//关闭按摩器
		memset(Msgr_Ints_FlagArr, MSGR_INTS_ZERO_LEVEL ,sizeof(Msgr_Ints_FlagArr)/sizeof(Msgr_Ints_FlagArr[0]));
		//同步外设发送典型按摩关闭
		msgr_para_set_event |= MSGR_ALL_INTS_EVENT;
	}

	if(system_config.flags.help_sleep_mode == HELP_SLEEP_MODE_TWO)
	{
		User_MusicDemo(2);
		User_LightDemo(0);
	}	
	else
	{
		User_MusicDemo(0);
	}
	motor_demo_5ms_cont = 0;
	motor_demo_1sec_cont = 0;	
	motor_demo_loop_count = 0;
	motor_demo_step = 0;
	motor_demo_ms_time = 0;
	motor_demo_sec_time = 0;
	lin_received_step = 0;
	Motor_Back_ParaStu->motor_slow_run_flag = 0;
	Motor_Leg_ParaStu->motor_slow_run_flag = 0;		
	Motor_Lumbar_ParaStu->motor_slow_run_flag = 0;
	Motor_Neck_ParaStu->motor_slow_run_flag = 0;	
	Motor_Lumbar2_ParaStu->motor_slow_run_flag = 0;
	Motor_Neck2_ParaStu->motor_slow_run_flag = 0;	
	Motor_Tilt1_ParaStu->motor_slow_run_flag = 0;
	Motor_Tilt2_ParaStu->motor_slow_run_flag = 0;
	alarm_msgr_running = 0;
	alarm_three_phase = 0;
	alarm_three_count = 0;	
	ble_report_set_event(BLE_REPORT_EVENT, REPORT_MOTOR_CMD_EVENT);		
}

void Motor_Sync_EnableSet(unsigned char enable_flag)
{
	motor_sync_enable = enable_flag;
}

unsigned char Get_Motor_Run_CmdState(void)
{
	return motor_run_cmd_state;
}
void Motor_Lin_SendPosition(void)
{
	Motor_SyncCurrent_HallArr[0] = Motor1_ParaStu.motor_port;
	Motor_SyncCurrent_HallArr[1] = Motor_SyncTarget_HallArr[Motor1_ParaStu.motor_port]/256;
	Motor_SyncCurrent_HallArr[2] = Motor_SyncTarget_HallArr[Motor1_ParaStu.motor_port]%256;		

	Motor_SyncCurrent_HallArr[3] = Motor2_ParaStu.motor_port;
	Motor_SyncCurrent_HallArr[4] = Motor_SyncTarget_HallArr[Motor2_ParaStu.motor_port]/256;
	Motor_SyncCurrent_HallArr[5] = Motor_SyncTarget_HallArr[Motor2_ParaStu.motor_port]%256;		

	Motor_SyncCurrent_HallArr[6] = Motor3_ParaStu.motor_port;
	Motor_SyncCurrent_HallArr[7] = Motor_SyncTarget_HallArr[Motor3_ParaStu.motor_port]/256;
	Motor_SyncCurrent_HallArr[8] = Motor_SyncTarget_HallArr[Motor3_ParaStu.motor_port]%256;	

	Motor_SyncCurrent_HallArr[9] = Motor4_ParaStu.motor_port;
	Motor_SyncCurrent_HallArr[10] = Motor_SyncTarget_HallArr[Motor4_ParaStu.motor_port]/256;
	Motor_SyncCurrent_HallArr[11] = Motor_SyncTarget_HallArr[Motor4_ParaStu.motor_port]%256;		

	Motor_SyncCurrent_HallArr[12] = Motor5_ParaStu.motor_port;
	Motor_SyncCurrent_HallArr[13] = Motor_SyncTarget_HallArr[Motor5_ParaStu.motor_port]/256;
	Motor_SyncCurrent_HallArr[14] = Motor_SyncTarget_HallArr[Motor5_ParaStu.motor_port]%256;
	
	Motor_SyncCurrent_HallArr[15] = Motor6_ParaStu.motor_port;
	Motor_SyncCurrent_HallArr[16] = Motor_SyncTarget_HallArr[Motor6_ParaStu.motor_port]/256;
	Motor_SyncCurrent_HallArr[17] = Motor_SyncTarget_HallArr[Motor6_ParaStu.motor_port]%256;
	
	LIN_Master_Send_WriteMotorHall_Cmd(6,Motor_SyncCurrent_HallArr);		
}
void Motor_Lin_SendSync(void)
{
	//电机1
	if(Motor1_ParaStu.motor_limit_flag == LIMIT_DIR_UP)
	{
		Motor_SyncCurrent_HallArr[0] = MOTOR1_PORT;
		Motor_SyncCurrent_HallArr[1] = HALL_UP_NUM/256;
		Motor_SyncCurrent_HallArr[2] = HALL_UP_NUM%256;							
	}
	else if(Motor1_ParaStu.motor_limit_flag == LIMIT_DIR_DOWN)
	{
		Motor_SyncCurrent_HallArr[0] = MOTOR1_PORT;
		Motor_SyncCurrent_HallArr[1] = HALL_MIN_NUM/256;
		Motor_SyncCurrent_HallArr[2] = HALL_MIN_NUM%256;						
	}
	else
	{
		Motor_SyncCurrent_HallArr[0] = MOTOR1_PORT;
		Motor_SyncCurrent_HallArr[1] = Motor1_ParaStu.hall_run_num/256;
		Motor_SyncCurrent_HallArr[2] = Motor1_ParaStu.hall_run_num%256;						
	}
	//电机2
	if(Motor2_ParaStu.motor_limit_flag == LIMIT_DIR_UP)
	{
		Motor_SyncCurrent_HallArr[3] = MOTOR2_PORT;
		Motor_SyncCurrent_HallArr[4] = HALL_UP_NUM/256;
		Motor_SyncCurrent_HallArr[5] = HALL_UP_NUM%256;						
	}
	else if(Motor2_ParaStu.motor_limit_flag == LIMIT_DIR_DOWN)
	{
		Motor_SyncCurrent_HallArr[3] = MOTOR2_PORT;
		Motor_SyncCurrent_HallArr[4] = HALL_MIN_NUM/256;
		Motor_SyncCurrent_HallArr[5] = HALL_MIN_NUM%256;						
	}
	else
	{
		Motor_SyncCurrent_HallArr[3] = MOTOR2_PORT;
		Motor_SyncCurrent_HallArr[4] = Motor2_ParaStu.hall_run_num/256;
		Motor_SyncCurrent_HallArr[5] = Motor2_ParaStu.hall_run_num%256;							
	}

	//电机3
	if(Motor3_ParaStu.motor_limit_flag == LIMIT_DIR_UP)
	{
		Motor_SyncCurrent_HallArr[6] = MOTOR3_PORT;
		Motor_SyncCurrent_HallArr[7] = HALL_UP_NUM/256;
		Motor_SyncCurrent_HallArr[8] = HALL_UP_NUM%256;						
	}
	else if(Motor3_ParaStu.motor_limit_flag == LIMIT_DIR_DOWN)
	{
		Motor_SyncCurrent_HallArr[6] = MOTOR3_PORT;
		Motor_SyncCurrent_HallArr[7] = HALL_MIN_NUM/256;
		Motor_SyncCurrent_HallArr[8] = HALL_MIN_NUM%256;						
	}
	else
	{
		Motor_SyncCurrent_HallArr[6] = MOTOR3_PORT;
		Motor_SyncCurrent_HallArr[7] = Motor3_ParaStu.hall_run_num/256;
		Motor_SyncCurrent_HallArr[8] = Motor3_ParaStu.hall_run_num%256;							
	}
	//电机4
	if(Motor4_ParaStu.motor_limit_flag == LIMIT_DIR_UP)
	{
		Motor_SyncCurrent_HallArr[9] = MOTOR4_PORT;
		Motor_SyncCurrent_HallArr[10] = HALL_UP_NUM/256;
		Motor_SyncCurrent_HallArr[11] = HALL_UP_NUM%256;							
	}
	else if(Motor4_ParaStu.motor_limit_flag == LIMIT_DIR_DOWN)
	{
		Motor_SyncCurrent_HallArr[9] = MOTOR4_PORT;
		Motor_SyncCurrent_HallArr[10] = HALL_MIN_NUM/256;
		Motor_SyncCurrent_HallArr[11] = HALL_MIN_NUM%256;							
	}
	else
	{
		Motor_SyncCurrent_HallArr[9] = MOTOR4_PORT;
		Motor_SyncCurrent_HallArr[10] = Motor4_ParaStu.hall_run_num/256;
		Motor_SyncCurrent_HallArr[11] = Motor4_ParaStu.hall_run_num%256;						
	}
	//
	if(Motor5_ParaStu.motor_limit_flag == LIMIT_DIR_UP)
	{
		Motor_SyncCurrent_HallArr[12] = MOTOR5_PORT;
		Motor_SyncCurrent_HallArr[13] = HALL_UP_NUM/256;
		Motor_SyncCurrent_HallArr[14] = HALL_UP_NUM%256;							
	}
	else if(Motor5_ParaStu.motor_limit_flag == LIMIT_DIR_DOWN)
	{
		Motor_SyncCurrent_HallArr[12] = MOTOR5_PORT;
		Motor_SyncCurrent_HallArr[13] = HALL_MIN_NUM/256;
		Motor_SyncCurrent_HallArr[14] = HALL_MIN_NUM%256;							
	}
	else
	{
		Motor_SyncCurrent_HallArr[12] = MOTOR5_PORT;
		Motor_SyncCurrent_HallArr[13] = Motor5_ParaStu.hall_run_num/256;
		Motor_SyncCurrent_HallArr[14] = Motor5_ParaStu.hall_run_num%256;						
	}
	//
	if(Motor6_ParaStu.motor_limit_flag == LIMIT_DIR_UP)
	{
		Motor_SyncCurrent_HallArr[15] = MOTOR6_PORT;
		Motor_SyncCurrent_HallArr[16] = HALL_UP_NUM/256;
		Motor_SyncCurrent_HallArr[17] = HALL_UP_NUM%256;							
	}
	else if(Motor6_ParaStu.motor_limit_flag == LIMIT_DIR_DOWN)
	{
		Motor_SyncCurrent_HallArr[15] = MOTOR6_PORT;
		Motor_SyncCurrent_HallArr[16] = HALL_MIN_NUM/256;
		Motor_SyncCurrent_HallArr[17] = HALL_MIN_NUM%256;							
	}
	else
	{
		Motor_SyncCurrent_HallArr[15] = MOTOR6_PORT;
		Motor_SyncCurrent_HallArr[16] = Motor6_ParaStu.hall_run_num/256;
		Motor_SyncCurrent_HallArr[17] = Motor6_ParaStu.hall_run_num%256;						
	}
	//
	LIN_Master_Send_WriteMotorHall_Cmd(6,Motor_SyncCurrent_HallArr);
}
unsigned char GetSet_Motor_Ctr_Cmd(unsigned char ctr_state)
{
	if(ctr_state != 0xff)
	{
		if(motor_ctr_cmd != ctr_state)
		{
			motor_cmd_change_fast = 1;
		}
		motor_ctr_cmd = ctr_state;
		motor_oneclick_cmd = ctr_state;
	}
	return motor_ctr_cmd;
}
