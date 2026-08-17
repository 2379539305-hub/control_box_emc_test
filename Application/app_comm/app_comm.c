#include "app_comm.h"
#include "delay.h"
#include "iwdg.h"
#include "driver_key.h"
#include "driver_beep.h"
#include "driver_ble.h"
#include "driver_a7105.h"
#include "modul_ttlbus.h"
#include "modul_motor.h"
#include "modul_a7105.h"

#include "app_linbus.h"
#include "app_ble.h"
#include "app_ttlbus.h"
#include "app_music.h"
#include "app_msgr.h"
#include "app_motor.h"
#include "app_save.h"
#include "app_config.h"
#include "app_rtc.h"
#include "app_vita.h"
#include "app_backhual.h"
// 管制码，必须手动修改
unsigned char SOURCE_CODE_ID[14] = SOURCE_CODE_ID_CONFIG; 

    
//获取到的控制信号
unsigned char key_value = 0 ,old_key_value = 0;

unsigned short key_para_set_event = 0;
static unsigned char sys_wait_ack_time = 0;      //等待回传间隔
unsigned short study_long_time = 0;
unsigned char study_wifi_reset_flag = 0;
unsigned char a7105_value_temp = 0 , old_a7105_value_temp = 0;
unsigned char a7105_error_count = 0;
unsigned short a7105_long_time = 0;
unsigned char sync_key_state = 0; //用于区分主遥控器还是从遥控器 0 是主遥控器 1是从遥控器 
unsigned char sync_run_mode = 1; //背部同控 背部分控 就2个模式，==0的时候是同控，等于1的时候是分控

/*
马达运行中按下对码按键->马达停止->对码成功->马达无动作
                                ->对码失败->马达无动作
																
马达静止状态按下对码按键->马达停止->对码成功->马达无动作
                                  ->对码失败->马达根据要求，是否执行复位动作																
*/
//对码  返回成功还是失败
unsigned char ControlBox_StudyCodeTask(void) 
{
	//对码成功标志
	unsigned char study_success_flag = 0;
	//获取电机动作标志
	unsigned char motor_run_flag = 0;
	//按下对码按键
	if(!Get_StudyKey_State())  
	{
		//马达停止   清除相关标志位   按摩器状态不变
		motor_run_flag =  Get_Motor_PortState();  //获取当前马达是否在动作
		//马达停止
		Motor_AllStop();
		
		ble_study_mode = 0;
		
		//等待按键释放
		while(!Get_StudyKey_State()) 
		{
			Iwdg_Clear();
			/*-------------射频和蓝牙学习------------------*/
			//学习成功
			if(1 == BleBlueTooth_Study_Mode())
			{
				//学习成功标志位置位
				study_success_flag |= 0x02;
				//蜂鸣器提醒
				Beep_SingSetPara(200,3);
				//等待按键松开
				while(!Get_StudyKey_State())
				{
					Iwdg_Clear();
					study_success_flag |= 0x02;
				}
			}
			else
			{
				study_success_flag &= 0xfd;
			}
			//米家重置
//			if((study_success_flag & 0x03) == 0x00)  //对码失败长按开始
//			{
//				study_wifi_reset_flag = 1;
//				if(study_long_time >= 1000)
//				{
//					study_long_time = 0;
//          Beep_SingSetPara(200,3);
//					while(!Get_StudyKey_State())
//					{
//						study_wifi_reset_flag = 0;
//						study_success_flag = 0x00;
//					}
//					TTL_Radio_Wifi_Reset();//重置米家
//					iot_restore_flag = 1;
//				}
//			}
//			else
//			{
//				study_wifi_reset_flag = 0;
//			}		
		}
		study_wifi_reset_flag = 0;
		ble_study_mode = 1;
		//退出对码模式，清除射频键值缓存，防止ID数据被误判为误码导致异常动作
		old_a7105_value_temp = 0;
		a7105_value_temp = 0;
		a7105_error_count = 0;
		//对码失败，退出学习模式
		if(0 == study_success_flag)  
		{
			//如果使能了对码键复位，判断之前电机是否在动作，如果之前电机不在动作就进行复位，否则不动
			if(PAIR_GO_FLAT_ENABLE == system_config.flags.pair_key_flat)
			{
				if(motor_run_flag == 0) 
				{
					//电机执行复位动作
					Motor_OneClickCmd_Set(KEY_FLAT);
					//对码键复位不同步
					Motor_Sync_EnableSet(0);
				}
			}
		}
	}
	//如果蓝牙进入过对码模式，就让蓝牙进入正常模式		
	BleBlueTooth_Normal_Mode();
	//返回对码成功标志位
	return study_success_flag;
}
//获取射频/蓝牙控制信号
void Get_Key_InfoTask(void)
{
	//哪个先来信号  就以哪个为准
	static unsigned char key_typical_flag = 0; 
	//临时接收数据
	static unsigned char rf_value_temp = 0;
	static unsigned short ble_value_temp = 0;
	static unsigned char ttl_value_temp = 0;
	unsigned char key_analysis = 0;
	//保存上次的数据
	old_key_value = key_value;
	/*------------------------获取各接收方式的键值----------------------------------*/
	//TTL有数据
	ttl_value_temp = TTL_GetClear_KeyValue(1);
//	if(1 == TTL_Check_Busy_Free())
//	{
//		if(0 == Motor_Continue_KeyInfo(ttl_value_temp))
//		{
//			TTL_GetClear_KeyValue(0);
//			ttl_value_temp = 0;
//		}
//	}
	//RF,BLE数据解析
	key_analysis = BleBlueTooth_Get_KeyState() ;
	//RF有数据
	if(key_analysis == 2) 
	{
		//读取RF数据并返回到临时接收数据里
		rf_value_temp = A7105_Analy_KeyValue();
		alarm_key_state = 0;
		vita_key_state = 0;
	}
	//蓝牙有数据
	if(key_analysis == 1)
	{
		//接收并解析蓝牙数据
		ble_value_temp = Ble_Analy_KeyValue(ble_key_value);
		alarm_key_state = 0;
		vita_key_state = 0;	
	}
	if(key_analysis == 3)
	{
		//如果rf返回了有效数据
		rf_value_temp = 0;
		ble_value_temp = 0;
		old_a7105_value_temp = 0;
		a7105_value_temp = 0;
	}
	//A7105，如果之前没有数据或者之前是rf数据
	if(0 == key_typical_flag || (key_typical_flag == 0x01))
	{
		//如果rf返回了有效数据
		if(rf_value_temp != 0)
		{
			TTL_GetClear_KeyValue(0); //遥控器来键值了,就把TTL的清除, 无线遥控器的优先级高
			key_para_set_event &= ~KEY_TTL_CMD_EVENT;	
			//置位接收标志
			key_typical_flag = 0x01;
			//记录接收到的数据

			key_value = rf_value_temp;
			//如果系统音乐锁开着
			if((sys_lock_state & SYS_MUSIC_LOCK_STATE) == SYS_MUSIC_LOCK_STATE) //放键值获取外面测试 来回动背腿后再去 记忆位置 蜂鸣器会一直响
			{
				//接收到的数据是音乐相关按键
				if(1 == Music_AcceptCmd_KeyInfo(key_value))
				{
					//锁定按键
					key_value = KEY_MAX;
				}
			}
			//如果系统儿童锁开着
			if((sys_lock_state & SYS_CHILD_LOCK_STATE) == SYS_CHILD_LOCK_STATE)
			{
				//除锁和重置按键外，锁定其他任何按键
				if(key_value != 0 && key_value != KEY_CTRBOX_SW && key_value != KEY_ID_MATCH)
				{
					key_value = KEY_MAX;
				}
			}		
		}
		else
		{
			//如果rf没返回有效数据，清除接收标志
			key_typical_flag = 0x00;
		}
	}
	//BLE，如果之前没有数据或者之前是蓝牙数据
	if(0 == key_typical_flag || (key_typical_flag == 0x02))
	{
		//如果蓝牙数据有效
		if(ble_value_temp != 0)
		{
			ttl_value_temp = 0; //遥控器来键值了,就把TTL的清除, 无线遥控器的优先级高
			key_para_set_event &= ~KEY_TTL_CMD_EVENT;	
			//接收标志置位
			key_typical_flag = 0x02;
			key_value = ble_value_temp;
			if((sys_lock_state & SYS_MUSIC_LOCK_STATE) == SYS_MUSIC_LOCK_STATE)
			{
				if(1 == Music_AcceptCmd_KeyInfo(key_value))
				{
					key_value = KEY_MAX;
				}
			}
			if((sys_lock_state & SYS_CHILD_LOCK_STATE) == SYS_CHILD_LOCK_STATE)
			{
				if(key_value != 0 && key_value != KEY_CTRBOX_SW && key_value != KEY_ID_MATCH)
				{
					key_value = KEY_MAX;
				}
			}			
		}
		else
		{
			//如果蓝牙数据无效，清除接收标志
			key_typical_flag = 0x00;
		}

	}
	//TTL指令
	if((key_para_set_event & KEY_TTL_CMD_EVENT) == KEY_TTL_CMD_EVENT)  //直接执行控制
	{
		key_para_set_event &= ~KEY_TTL_CMD_EVENT;
		ttl_value_temp = TTL_GetClear_KeyValue(1);
		key_value = ttl_value_temp;
		
		if((sys_lock_state & SYS_MUSIC_LOCK_STATE) == SYS_MUSIC_LOCK_STATE)
		{
			if(1 == Music_AcceptCmd_KeyInfo(key_value))
			{
				key_value = KEY_MAX;
			}
		}
//		if((sys_lock_state & SYS_CHILD_LOCK_STATE) == SYS_CHILD_LOCK_STATE)
//		{
//			if(key_value != 0 && key_value != KEY_CTRBOX_SW && key_value != KEY_ID_MATCH)
//			{
//				key_value = KEY_MAX;
//			}
//		}		//儿童锁APP可以控制
	}
	
	/*-------------------------------系统键值处理---------------------------------------*/
	//如果接收到的是音乐锁按键
	if(key_value == KEY_MUSIC_LOCK && old_key_value != key_value)
	{
		//蜂鸣器提醒
		Beep_SingSetPara(200,2);
		//判断当前状态并切换
		if((sys_lock_state & SYS_MUSIC_LOCK_STATE) != SYS_MUSIC_LOCK_STATE)
		{
			sys_lock_state |= SYS_MUSIC_LOCK_STATE;
		}
		else
		{
			sys_lock_state &= ~SYS_MUSIC_LOCK_STATE;
		}
		LIN_Master_Send_lockstate_Cmd(sys_lock_state);

	}
	//如果接收到的是控制盒锁按键
	if(key_value == KEY_CTRBOX_SW && old_key_value != key_value)
	{
		//蜂鸣器提醒
		Beep_SingSetPara(200,3);
		//判断当前状态并切换
		if((sys_lock_state & SYS_CHILD_LOCK_STATE) != SYS_CHILD_LOCK_STATE)
		{
			sys_lock_state |= SYS_CHILD_LOCK_STATE;
		}
		else
		{
			sys_lock_state &= ~SYS_CHILD_LOCK_STATE;
		}
		LIN_Master_Send_lockstate_Cmd(sys_lock_state);
	}
			//发送LIN协议
	
	//如果控制盒锁开着
	if(((sys_lock_state & SYS_CHILD_LOCK_STATE) == SYS_CHILD_LOCK_STATE)  && (0 == ttl_value_temp))
	{
		//按键不为0且切换了按键
		if(key_value != 0 && key_value != KEY_CTRBOX_SW && key_value != KEY_ID_MATCH && old_key_value != key_value)
		{
			//蜂鸣器提醒
			Beep_SingSetPara(200,3);
			LIN_Master_Send_lockstate_Cmd(sys_lock_state);
		}
	}
	else if(((sys_lock_state & SYS_MUSIC_LOCK_STATE) == SYS_MUSIC_LOCK_STATE)  && (0 == ttl_value_temp))
	{
		//如果音乐锁开着
		if(key_value == KEY_MAX && old_key_value != key_value)
		{
			//蜂鸣器提醒
			Beep_SingSetPara(200,2);
			LIN_Master_Send_lockstate_Cmd(sys_lock_state);
		}
	}
	if(key_value == KEY_SYNC_MODE_SW)
	{
		if(old_key_value != key_value)
		{
			if(Get_Sync_Run_Mode() == 0)
			{
				Set_Sync_Run_Mode(1);
				Beep_SingSetPara(500,2);
				ble_report_set_event(BLE_REPORT_EVENT, REPORT_SYNC_MODE_EVENT);
			}
			else
			{
				Set_Sync_Run_Mode(0);
				Beep_SingSetPara(500,1);	
				ble_report_set_event(BLE_REPORT_EVENT, REPORT_SYNC_MODE_EVENT);		
			}
		}
	}
	//WIFI清除配网
//	if(key_value == KEY_ID_MATCH && old_key_value != key_value)
//	{
//		//蜂鸣器提醒
//		Beep_SingSetPara(200,2);
//		TTL_Radio_Wifi_Reset();
//		iot_restore_flag = 1;
//	}
	//如果没收到数据，清零接收数据
	if(0 == rf_value_temp && 0 == ble_value_temp  && 0 == ttl_value_temp )
	{
		key_value = 0;
	}
	//
}
typedef struct
{
	unsigned char b0,b1,b2,b3;
	unsigned char key;
	unsigned char key_state;
}A7105_CodeMap_t;

/* 备注：同一 key 的不同码可以并列放入（如两个蓝牙开关码） */
static const A7105_CodeMap_t code_map[] = {
	{0x00,0x00,0x00,0x00, 0x00,                 0},
	{0xCC,0xCC,0xAA,0xAA, KEY_CTRBOX_SW,         0},
	{0xCC,0xCC,0x99,0x99, KEY_ID_MATCH,          0},
	{0xAA,0xAA,0xDD,0xDD, KEY_MUSIC_LOCK,        0},
	{0x55,0x55,0x44,0x44, KEY_BACK_UP,           0},
	{0x55,0x55,0x55,0x55, KEY_BACK_DOWN,         0},
	{0x55,0x55,0x66,0x66, KEY_LEG_UP,            0},
	{0x55,0x55,0x77,0x77, KEY_LEG_DOWN,          0},
	{0x55,0x55,0x99,0x99, KEY_BACKLEG_UP,        0},
	{0x55,0x55,0xAA,0xAA, KEY_BACKLEG_DOWN,      0},
	{0x99,0x99,0x77,0x77, KEY_TILT_ALL_UP,    0},
	{0x99,0x99,0x88,0x88, KEY_TILT_ALL_DOWN,  0},
	{0x35,0x35,0x33,0x33, KEY_LUMBAR2_UP,        0},
	{0x35,0x35,0x44,0x44, KEY_LUMBAR2_DOWN,      0},
	{0x55,0x55,0xBB,0xBB, KEY_MEM_M1,            0},
	{0x55,0x55,0xEE,0xEE, KEY_GO_M1,             0},
	{0x55,0x55,0xCC,0xCC, KEY_MEM_M2,            0},
	{0xAA,0xAA,0x11,0x11, KEY_GO_M2,             0},
	{0x55,0x55,0xDD,0xDD, KEY_MEM_M3,            0},
	{0xAA,0xAA,0x22,0x22, KEY_GO_M3,             0},
	{0xCC,0xCC,0xBB,0xBB, KEY_MEM_TV,            0},
	{0x66,0x66,0xDD,0xDD, KEY_GO_TV,             0},
	{0xCC,0xCC,0xDD,0xDD, KEY_MEM_ZEROG,         0},
	{0x99,0x99,0x99,0x99, KEY_GO_ZEROG,          0},
	{0xCC,0xCC,0xCC,0xCC, KEY_MEM_LOUNGE,        0},
	{0x66,0x66,0xEE,0xEE, KEY_GO_LOUNGE,         0},
	{0x33,0x33,0xBB,0xBB, KEY_MEM_SNORE,         0},
	{0x99,0x99,0xAA,0xAA, KEY_GO_SNORE,          0},
	{0x33,0x33,0x44,0x44, KEY_MEM_READ,          0},
	{0x33,0x33,0x33,0x33, KEY_GO_READ,           0},
	{0x33,0x33,0xEE,0xEE, KEY_GO_YOGA,           0},
	{0x33,0x33,0xDD,0xDD, KEY_MEM_YOGA,          0},
	{0x99,0x99,0x11,0x11, KEY_GO_GETUP,          0},
	{0x99,0x99,0xB1,0xB1, KEY_MEM_GETUP,         0},
	{0x99,0x99,0x12,0x12, KEY_GO_NURSING,        0},
	{0x99,0x99,0xB2,0xB2, KEY_MEM_NURSING,       0},
	{0xAA,0xAA,0x33,0x33, KEY_FLAT,              0},
	{0xCC,0xCC,0x11,0x11, KEY_MOTOR_STOP,        0},
	{0x95,0x95,0x99,0x99, KEY_DEMO1_MODE,        0},
	{0x95,0x95,0x88,0x88, KEY_DEMO2_MODE,				 0},
	{0x95,0x95,0x77,0x77,	KEY_MUSIC_MEM_HELP_SLEEP, 0},
	{0x99,0x99,0xc1,0xc1, KEY_MUSIC_WHITE_NOISE,		0},
	{0xee,0xee,0x33,0x33, KEY_LIGHT_SW, 						0},
	{0xee,0xee,0x44,0x44, KEY_LIGHT_OFF, 						0},	
	{0x63,0x63,0x55,0x55, KEY_ONE_CLICK_BACK_UP,     0},
	{0x63,0x63,0x66,0x66, KEY_ONE_CLICK_BACK_DOWN,   0},
	{0x63,0x63,0x77,0x77, KEY_ONE_CLICK_LEG_UP,      0},
	{0x63,0x63,0x88,0x88, KEY_ONE_CLICK_LEG_DOWN,    0},
	{0x63,0x63,0x99,0x99, KEY_ONE_CLICK_BACKLEG_UP,  0},
	{0x63,0x63,0xAA,0xAA, KEY_ONE_CLICK_BACKLEG_DOWN,0},
	{0x63,0x63,0xBB,0xBB, KEY_ONE_CLICK_ALL_UP,      0},
	{0x63,0x63,0xCC,0xCC, KEY_ONE_CLICK_ALL_DOWN,    0},
	{0x95,0x95,0x11,0x11, KEY_ONE_CLICK_LUMBAR_UP,   0},
	{0x95,0x95,0x22,0x22, KEY_ONE_CLICK_LUMBAR_DOWN, 0},
	{0x95,0x95,0x33,0x33, KEY_ONE_CLICK_NECK_UP,     0},
	{0x95,0x95,0x44,0x44, KEY_ONE_CLICK_NECK_DOWN,   0},
	{0x95,0x95,0x55,0x55, KEY_ONE_CLICK_LUMBAR2_UP,  0},
	{0x95,0x95,0x66,0x66, KEY_ONE_CLICK_LUMBAR2_DOWN,0},
	{0x95,0x95,0x77,0x77, KEY_ONE_CLICK_LUMBAR_NECK_UP, 0},
	{0x95,0x95,0x88,0x88, KEY_ONE_CLICK_LUMBAR_NECK_DOWN,0},
	{0xA3,0xA3,0x11,0x11, KEY_ONE_CLICK_NECK2_UP,    0},
	{0xA3,0xA3,0x22,0x22, KEY_ONE_CLICK_NECK2_DOWN,  0},
	{0xA3,0xA3,0x66,0x66, KEY_MSGR1_INTS_OFF,        0},
	{0xA3,0xA3,0x77,0x77, KEY_MSGR2_INTS_OFF,        0},
	{0x99,0x99,0xBB,0xBB, KEY_MSGR_ALL_OFF,          0},
	{0xCC,0xCC,0x44,0x44, KEY_MSGR_ALL_ON,           0},
	{0x99,0x99,0xCC,0xCC, KEY_MSGR_MODE,             0},
	{0xAA,0xAA,0xCC,0xCC, KEY_MSGR_CONSTANT,         0},
	{0xAA,0xAA,0xAA,0xAA, KEY_MSGR_PULSE,            0},
	{0xAA,0xAA,0xBB,0xBB, KEY_MSGR_WAVE,             0},
	{0xCC,0xCC,0x66,0x66, KEY_MSGR_10MIN,            0},
	{0xCC,0xCC,0x77,0x77, KEY_MSGR_20MIN,            0},
	{0xCC,0xCC,0x88,0x88, KEY_MSGR_30MIN,            0},
	{0x66,0x66,0x11,0x11, KEY_MSGR1_INTS_ADD,        0},
	{0x66,0x66,0x22,0x22, KEY_MSGR1_INTS_DCR,        0},
	{0x66,0x66,0x33,0x33, KEY_MSGR2_INTS_ADD,        0},
	{0x66,0x66,0x44,0x44, KEY_MSGR2_INTS_DCR,        0},
	{0xC3,0xC3,0x11,0x11, KEY_MSGR3_INTS_ADD,        0},
	{0xC3,0xC3,0x66,0x66, KEY_MSGR3_INTS_DCR,        0},
	{0xA3,0xA3,0x44,0x44, KEY_MSGR1_ON,              0},
	{0xAA,0xAA,0x44,0x44, KEY_MSGR1_SW,              0},
	{0xC3,0xC3,0x99,0x99, KEY_MSGR1_INTS_ONE,        0},
	{0xC3,0xC3,0xAA,0xAA, KEY_MSGR1_INTS_TWO,        0},
	{0xC3,0xC3,0xBB,0xBB, KEY_MSGR1_INTS_THREE,      0},
	{0xA3,0xA3,0x55,0x55, KEY_MSGR2_ON,              0},
	{0xAA,0xAA,0x55,0x55, KEY_MSGR2_SW,              0},
	{0xC3,0xC3,0xCC,0xCC, KEY_MSGR2_INTS_ONE,        0},
	{0xC3,0xC3,0xDD,0xDD, KEY_MSGR2_INTS_TWO,        0},
	{0xC3,0xC3,0xEE,0xEE, KEY_MSGR2_INTS_THREE,      0},
	{0xC3,0xC3,0x33,0x33, KEY_MSGR3_INTS_ONE,        0},
	{0xC3,0xC3,0x44,0x44, KEY_MSGR3_INTS_TWO,        0},
	{0xC3,0xC3,0x55,0x55, KEY_MSGR3_INTS_THREE,      0},
	{0xAA,0xAA,0xEE,0xEE, KEY_UBL_SW,                0},
	{0xEE,0xEE,0x99,0x99, KEY_UBL_ON,                0},
	{0xEE,0xEE,0xAA,0xAA, KEY_UBL_OFF,               0},
	{0x99,0x99,0x71,0x71, KEY_LED_BOARD_SW,          0},
	{0xCC,0xCC,0x55,0x55, KEY_MUSIC_FOLLOW_INTS_ZERO,0},
	{0xCC,0xCC,0xEE,0xEE, KEY_MUSIC_FOLLOW_INTS_ONE, 0},
	{0x33,0x33,0x11,0x11, KEY_MUSIC_FOLLOW_INTS_TWO, 0},
	{0x33,0x33,0x22,0x22, KEY_MUSIC_FOLLOW_INTS_THREE,0},
	{0xAA,0xAA,0x88,0x88, KEY_MUSIC_VOLUME_ADD,      0},
	{0xAA,0xAA,0x99,0x99, KEY_MUSIC_VOLUME_DCR,      0},
	{0x66,0x66,0x66,0x66, KEY_MUSIC_SW,              0},
	{0x99,0x99,0xDD,0xDD, KEY_MUSIC_PRE,             0},
	{0x99,0x99,0xEE,0xEE, KEY_MUSIC_NEXT,            0},
	{0x33,0x33,0xAA,0xAA, KEY_MUSIC_BLE_SW,          0},
	{0x33,0x33,0x55,0x55, KEY_MUSIC_BLE_SW,          0},
	{0x55,0x55,0x88,0x88, KEY_MUSIC_TWS_SW,          0},
	{0x99,0x99,0x81,0x81, KEY_MUSIC_DEVICE_MODE,     0},
	{0x77,0x77,0xEE,0xEE, KEY_FAN_ON,                0},
	{0x77,0x77,0xDD,0xDD, KEY_FAN_OFF,               0},
	{0x88,0x88,0xCC,0xCC, KEY_FAN_DIR_SW,            0},
	{0x88,0x88,0x11,0x11, KEY_FAN_MODE,              0},
	{0x88,0x88,0x22,0x22, KEY_FAN_CONSTANT,          0},
	{0x88,0x88,0x33,0x33, KEY_FAN_PULSE,             0},
	{0x88,0x88,0x44,0x44, KEY_FAN_WAVE,              0},
	{0x88,0x88,0x55,0x55, KEY_FAN_2H,                0},
	{0x88,0x88,0x66,0x66, KEY_FAN_3H,                0},
	{0x88,0x88,0x77,0x77, KEY_FAN_5H,                0},
	{0x88,0x88,0xAA,0xAA, KEY_FAN1_INC,              0},
	{0x88,0x88,0xBB,0xBB, KEY_FAN2_INC,              0},
	{0x77,0x77,0x11,0x11, KEY_FAN1_FORWARD_ONE,      0},
	{0x77,0x77,0x22,0x22, KEY_FAN1_FORWARD_TWO,      0},
	{0x77,0x77,0x33,0x33, KEY_FAN1_FORWARD_THREE,    0},
	{0x77,0x77,0x44,0x44, KEY_FAN1_BACKWARD_ONE,     0},
	{0x77,0x77,0x55,0x55, KEY_FAN1_BACKWARD_TWO,     0},
	{0x77,0x77,0x66,0x66, KEY_FAN1_BACKWARD_THREE,   0},
	{0x77,0x77,0x77,0x77, KEY_FAN2_FORWARD_ONE,      0},
	{0x77,0x77,0x88,0x88, KEY_FAN2_FORWARD_TWO,      0},
	{0x77,0x77,0x99,0x99, KEY_FAN2_FORWARD_THREE,    0},
	{0x77,0x77,0xAA,0xAA, KEY_FAN2_BACKWARD_ONE,     0},
	{0x77,0x77,0xBB,0xBB, KEY_FAN2_BACKWARD_TWO,     0},
	{0x77,0x77,0xCC,0xCC, KEY_FAN2_BACKWARD_THREE,   0},
	{0x88,0x88,0x88,0x88, KEY_FAN1_OFF,              0},
	{0x88,0x88,0x99,0x99, KEY_FAN2_OFF,              0},
	{0x55,0x55,0x11,0x11, KEY_HEAT_SW,               0},
			/* 系统/锁 */
	{0xC1,0xC1,0xAA,0xAA, KEY_CTRBOX_SW, 								1},
	{0xA1,0xA1,0xDD,0xDD, KEY_MUSIC_LOCK, 							1},
	{0xC1,0xC1,0x99,0x99, KEY_ID_MATCH, 								1},
	{0x51,0x51,0x44,0x44, KEY_BACK_UP, 									1},/* 马达 - 头/脚/背腿 */
	{0x51,0x51,0x55,0x55, KEY_BACK_DOWN, 								1},
	{0x51,0x51,0x66,0x66, KEY_LEG_UP, 									1},
	{0x51,0x51,0x77,0x77, KEY_LEG_DOWN, 								1},
	{0x51,0x51,0x99,0x99, KEY_BACKLEG_UP,								1},
	{0x51,0x51,0xAA,0xAA, KEY_BACKLEG_DOWN,							1},
	{0x35,0x35,0x31,0x31, KEY_LUMBAR2_UP,        				1},
	{0x35,0x35,0x41,0x41, KEY_LUMBAR2_DOWN,      				1},
	{0x51,0x51,0xBB,0xBB, KEY_MEM_M1, 									1},/* Memory / Go 组 */
	{0x51,0x51,0xEE,0xEE, KEY_GO_M1, 										1},
	{0x51,0x51,0xCC,0xCC, KEY_MEM_M2, 									1},
	{0xA1,0xA1,0x11,0x11, KEY_GO_M2, 										1},
	{0x51,0x51,0xDD,0xDD, KEY_MEM_M3, 									1},
	{0xA1,0xA1,0x22,0x22, KEY_GO_M3, 										1},
	{0xC1,0xC1,0xBB,0xBB, KEY_MEM_TV, 									1},
	{0x61,0x61,0xDD,0xDD, KEY_GO_TV, 										1},
	{0xC1,0xC1,0xDD,0xDD, KEY_MEM_ZEROG, 								1},
	{0x91,0x91,0x99,0x99, KEY_GO_ZEROG, 								1},
	{0xC1,0xC1,0xCC,0xCC, KEY_MEM_LOUNGE, 							1},
	{0x61,0x61,0xEE,0xEE, KEY_GO_LOUNGE, 								1},
	{0x31,0x31,0xBB,0xBB, KEY_MEM_SNORE, 								1},
	{0x91,0x91,0xAA,0xAA, KEY_GO_SNORE, 								1},
	{0x31,0x31,0x44,0x44, KEY_MEM_READ, 								1},
	{0x31,0x31,0x33,0x33, KEY_GO_READ,	 								1},
	{0x31,0x31,0xDD,0xDD, KEY_MEM_YOGA, 								1},
	{0x31,0x31,0xEE,0xEE, KEY_GO_YOGA, 									1},
	{0x91,0x91,0xB1,0xB1, KEY_MEM_GETUP, 								1},
	{0x91,0x91,0x11,0x11, KEY_GO_GETUP, 								1},
	{0x91,0x91,0xB2,0xB2, KEY_MEM_NURSING, 							1},
	{0x91,0x91,0x12,0x12, KEY_GO_NURSING, 								1},
	{0xA1,0xA1,0x33,0x33, KEY_FLAT, 										1},
	{0xC1,0xC1,0x11,0x11, KEY_MOTOR_STOP,								1},
	{0x91,0x91,0x91,0x91, KEY_DEMO1_MODE, 							1},
	{0x91,0x91,0xA1,0xA1, KEY_DEMO2_MODE, 							1},
//{0xX1,0xX1,0xXX,0xXX, KEY_DEMO3_MODE},					    
	{0x91,0x91,0xBB,0xBB, KEY_MSGR_ALL_OFF, 						1},/* 按摩器总控/模式 */
	{0xC1,0xC1,0x44,0x44, KEY_MSGR_ALL_ON, 							1},
	{0x91,0x91,0xCC,0xCC, KEY_MSGR_MODE, 								1},
	{0xA1,0xA1,0x66,0x66, KEY_MSGR1_INTS_OFF, 					1},/* 按摩器单路开关/关闭 */
	{0xA1,0xA1,0x77,0x77, KEY_MSGR2_INTS_OFF, 					1},
	{0xA1,0xA1,0x44,0x44, KEY_MSGR1_ON, 								1},
	{0xA1,0xA1,0x55,0x55, KEY_MSGR2_ON, 								1},
	{0xA1,0xA1,0x44,0x44, KEY_MSGR1_SW, 								1},
	{0xA1,0xA1,0x55,0x55, KEY_MSGR2_SW, 								1},
	{0xA1,0xA1,0xCC,0xCC, KEY_MSGR_CONSTANT,						1},/* 按摩模式 */
	{0xA1,0xA1,0xAA,0xAA, KEY_MSGR_PULSE, 							1},
	{0xA1,0xA1,0xBB,0xBB, KEY_MSGR_WAVE, 								1},
	{0xC1,0xC1,0x66,0x66, KEY_MSGR_10MIN, 							1},/* 按摩定时 */
	{0xC1,0xC1,0x77,0x77, KEY_MSGR_20MIN, 							1},
	{0xC1,0xC1,0x88,0x88, KEY_MSGR_30MIN, 							1},
	{0x61,0x61,0x11,0x11, KEY_MSGR1_INTS_ADD, 					1},/* 按摩强度 调整 */
	{0x61,0x61,0x22,0x22, KEY_MSGR1_INTS_DCR, 					1},
	{0x61,0x61,0x33,0x33, KEY_MSGR2_INTS_ADD, 					1},
	{0x61,0x61,0x44,0x44, KEY_MSGR2_INTS_DCR, 					1},
	{0xC1,0xC1,0x99,0x99, KEY_MSGR1_INTS_ONE, 					1},
	{0xC1,0xC1,0xAA,0xAA, KEY_MSGR1_INTS_TWO, 					1},
	{0xC1,0xC1,0xBB,0xBB, KEY_MSGR1_INTS_THREE, 				1},
	{0xC1,0xC1,0xCC,0xCC, KEY_MSGR2_INTS_ONE, 					1},
	{0xC1,0xC1,0xDD,0xDD, KEY_MSGR2_INTS_TWO,						1},
	{0xC1,0xC1,0xEE,0xEE, KEY_MSGR2_INTS_THREE, 				1},
	{0xA1,0xA1,0xEE,0xEE, KEY_UBL_SW, 									1},/* 灯控制 */
	{0xE1,0xE1,0x99,0x99, KEY_UBL_ON, 									1},
	{0xE1,0xE1,0xAA,0xAA, KEY_UBL_OFF, 									1},
	{0x91,0x91,0x71,0x71, KEY_LED_BOARD_SW, 						1},
	{0x51,0x51,0x33,0x33, KEY_MUSIC_FOLLOW_INTS_ADD, 		1},/* 音乐随动强度 */
	{0xC1,0xC1,0x55,0x55, KEY_MUSIC_FOLLOW_INTS_ZERO, 	1},
	{0xC1,0xC1,0xEE,0xEE, KEY_MUSIC_FOLLOW_INTS_ONE, 		1},
	{0x31,0x31,0x11,0x11, KEY_MUSIC_FOLLOW_INTS_TWO, 		1},
	{0x31,0x31,0x22,0x22, KEY_MUSIC_FOLLOW_INTS_THREE, 	1},
	{0xA1,0xA1,0x88,0x88, KEY_MUSIC_VOLUME_ADD, 				1},/* 音乐控制 */
	{0xA1,0xA1,0x99,0x99, KEY_MUSIC_VOLUME_DCR, 				1},
	{0x61,0x61,0x66,0x66, KEY_MUSIC_SW, 								1},
	{0x91,0x91,0xDD,0xDD, KEY_MUSIC_PRE,							 	1},
	{0x91,0x91,0xEE,0xEE, KEY_MUSIC_NEXT, 							1},
	{0x31,0x31,0xAA,0xAA, KEY_MUSIC_BLE_SW, 						1},
	{0x31,0x31,0x55,0x55, KEY_MUSIC_BLE_SW, 						1}, /* 第二种蓝牙开关码 */
	{0x51,0x51,0x88,0x88, KEY_MUSIC_TWS_SW, 						1},
	{0x91,0x91,0x81,0x81, KEY_MUSIC_DEVICE_MODE, 				1},	
};

#define CODE_MAP_SIZE (sizeof(code_map)/sizeof(code_map[0]))

static unsigned char A7105_MapLookup(const unsigned char *code, unsigned char *key_state)
{
	for(unsigned short i = 0; i < CODE_MAP_SIZE; i++)
	{
		if(code_map[i].b0 == code[0] && code_map[i].b1 == code[1] && code_map[i].b2 == code[2] && code_map[i].b3 == code[3])
		{
			if(key_state != NULL)
			{
				*key_state = code_map[i].key_state;
			}
			return code_map[i].key;
		}
	}
	return 0;
}
//射频协议解析
unsigned char A7105_Analy_KeyValue(void)
{
	unsigned char key_state = 0;

	old_a7105_value_temp = a7105_value_temp;
	a7105_value_temp = 0;

	/* 表驱动解析 */
	a7105_value_temp = A7105_MapLookup(HJ_A7105_CODE, &key_state);

	/* 需要根据配置动态映射的码值 */
	if(HJ_A7105_CODE[0] == 0x99 && HJ_A7105_CODE[1] == 0x99 && HJ_A7105_CODE[2] == 0x33 && HJ_A7105_CODE[3] == 0x33)
	{
		a7105_value_temp = (system_config.flags.rf_lumbar_neck_order == RF_LUMBAR_NECK_EXCHAGE_DISABLE) ? KEY_LUMBAR_UP : KEY_NECK_UP;
	}
	if(HJ_A7105_CODE[0] == 0x99 && HJ_A7105_CODE[1] == 0x99 && HJ_A7105_CODE[2] == 0x44 && HJ_A7105_CODE[3] == 0x44)
	{
		a7105_value_temp = (system_config.flags.rf_lumbar_neck_order == RF_LUMBAR_NECK_EXCHAGE_DISABLE) ? KEY_LUMBAR_DOWN : KEY_NECK_DOWN;
	}
	if(HJ_A7105_CODE[0] == 0x99 && HJ_A7105_CODE[1] == 0x99 && HJ_A7105_CODE[2] == 0x55 && HJ_A7105_CODE[3] == 0x55)
	{
		a7105_value_temp = (system_config.flags.rf_lumbar_neck_order == RF_LUMBAR_NECK_EXCHAGE_DISABLE) ? KEY_NECK_UP : KEY_LUMBAR_UP;
	}
	if(HJ_A7105_CODE[0] == 0x99 && HJ_A7105_CODE[1] == 0x99 && HJ_A7105_CODE[2] == 0x66 && HJ_A7105_CODE[3] == 0x66)
	{
		a7105_value_temp = (system_config.flags.rf_lumbar_neck_order == RF_LUMBAR_NECK_EXCHAGE_DISABLE) ? KEY_NECK_DOWN : KEY_LUMBAR_DOWN;
	}

	if(HJ_A7105_CODE[0] == 0x00 && HJ_A7105_CODE[1] == 0x00 && HJ_A7105_CODE[2] == 0x00 && HJ_A7105_CODE[3] == 0x00)
	{
		a7105_error_count = 0;
	}
	
	/*-----------------------------------------加滤波  超过10次误码才反馈0-----------------------------------------------------*/
	if(0 == a7105_value_temp)
	{
		if(HJ_A7105_CODE[0] != 0x00 || HJ_A7105_CODE[1] != 0x00 || HJ_A7105_CODE[2] != 0x00 || HJ_A7105_CODE[3] != 0x00)
		{
			a7105_error_count ++;
			if(a7105_error_count > 10)
			{
//				Beep_SingSetPara(100,3);
				a7105_error_count = 0;
				a7105_value_temp = 0;
			}
			else
			{
				a7105_value_temp = old_a7105_value_temp;
			}
		}
		else
		{
			a7105_error_count = 0;
		}
	}
	else
	{
		Set_Sync_Key_State(key_state);
		a7105_error_count = 0;
		a7105_long_time = 0; //收到有效数据，重新计时
	}
	//
	return a7105_value_temp;
}
//蓝牙指令解析
unsigned char Ble_Analy_KeyValue(unsigned char key_temp)
{
	unsigned char  ble_value_temp = 0;
	//将接收到的蓝牙值转换成本地统一值  按照485协议来定
	switch(key_temp)
	{
		case BLE_tmpbuf_PAIR:  ble_value_temp = KEY_ID_MATCH;break;
		case BLE_tmpbuf_LOCK:  ble_value_temp = KEY_CTRBOX_SW;break;
		case BLE_tmpbuf_MUSIC_LOCK:  ble_value_temp = KEY_MUSIC_LOCK;break;
		case BLE_tmpbuf_DATA_RESET: ble_value_temp = KEY_DATA_RESET; break;
		case BLE_tmpbuf_SYNC_MODE_SW: ble_value_temp = KEY_SYNC_MODE_SW; break;
		/*---------------------------马达控制相关-----------------------------*/
		case BLE_tmpbuf_ALL_STOP:       ble_value_temp = KEY_MOTOR_STOP;break;
		case BLE_tmpbuf_HEAD_UP:        ble_value_temp = KEY_BACK_UP;break;
		case BLE_tmpbuf_HEAD_DOWN:      ble_value_temp = KEY_BACK_DOWN;break;
		case BLE_tmpbuf_FOOT_UP:        ble_value_temp = KEY_LEG_UP;break;
		case BLE_tmpbuf_FOOT_DOWN:      ble_value_temp = KEY_LEG_DOWN;break;		
		case BLE_tmpbuf_HEAD_FOOT_UP:   ble_value_temp = KEY_BACKLEG_UP;break;
		case BLE_tmpbuf_HEAD_FOOT_DOWN: ble_value_temp = KEY_BACKLEG_DOWN;break;
		
		case BLE_tmpbuf_L3_UP: 					ble_value_temp = KEY_LUMBAR2_UP;break;
		case BLE_tmpbuf_L3_DOWN: 				ble_value_temp = KEY_LUMBAR2_DOWN;break;		
		case BLE_tmpbuf_NECK2_UP: 			ble_value_temp = KEY_NECK2_UP;break;
		case BLE_tmpbuf_NECK2_DOWN: 		ble_value_temp = KEY_NECK2_DOWN;break;			
		
		case BLE_tmpbuf_L1_L2_UP: 			ble_value_temp = KEY_TILT_ALL_UP;break;
		case BLE_tmpbuf_L1_L2_DOWN: 		ble_value_temp = KEY_TILT_ALL_DOWN;break;		

		case BLE_tmpbuf_FORWARD: 				ble_value_temp = KEY_FORWARD;break;
		case BLE_tmpbuf_BACKWARD: 			ble_value_temp = KEY_BACKWARD;break;
		case BLE_tmpbuf_TILT1_UP: 			ble_value_temp = KEY_TILT1_UP;break;
		case BLE_tmpbuf_TILT1_DOWN: 		ble_value_temp = KEY_TILT1_DOWN;break;
		case BLE_tmpbuf_TILT2_UP: 			ble_value_temp = KEY_TILT2_UP;break;
		case BLE_tmpbuf_TILT2_DOWN: 		ble_value_temp = KEY_TILT2_DOWN;break;
		//颈腰指令区分
		case BLE_tmpbuf_MOTOR3_UP:
		{
			if(system_config.flags.ble_lumbar_neck_order == BLE_LUMBAR_NECK_EXCHAGE_DISABLE )
			{
				ble_value_temp = KEY_LUMBAR_UP;
			}
			else
			{
				ble_value_temp = KEY_NECK_UP;
			};
		}break;    //外托一 腰部
		case BLE_tmpbuf_MOTOR3_DOWN:  
		{
			if(system_config.flags.ble_lumbar_neck_order == BLE_LUMBAR_NECK_EXCHAGE_DISABLE )
			{
				ble_value_temp = KEY_LUMBAR_DOWN;
			}
			else
			{
				ble_value_temp = KEY_NECK_DOWN;
			};		
		}break;	
		case BLE_tmpbuf_MOTOR4_UP:  
		{
			if(system_config.flags.ble_lumbar_neck_order == BLE_LUMBAR_NECK_EXCHAGE_DISABLE )
			{
				ble_value_temp = KEY_NECK_UP;
			}
			else
			{
				ble_value_temp = KEY_LUMBAR_UP;
			};
		}break;    //外托二 头靠
		case BLE_tmpbuf_MOTOR4_DOWN:  
		{
			if(system_config.flags.ble_lumbar_neck_order == BLE_LUMBAR_NECK_EXCHAGE_DISABLE )
			{
				ble_value_temp = KEY_NECK_DOWN;
			}
			else
			{
				ble_value_temp = KEY_LUMBAR_DOWN;
			};
		}break;			
		//一键调节指令
		case BLE_tmpbuf_ONE_CLICK_HEAD_UP:        ble_value_temp = KEY_ONE_CLICK_BACK_UP;break;
		case BLE_tmpbuf_ONE_CLICK_HEAD_DOWN:      ble_value_temp = KEY_ONE_CLICK_BACK_DOWN;break;
		case BLE_tmpbuf_ONE_CLICK_FOOT_UP:        ble_value_temp = KEY_ONE_CLICK_LEG_UP;break;
		case BLE_tmpbuf_ONE_CLICK_FOOT_DOWN:      ble_value_temp = KEY_ONE_CLICK_LEG_DOWN;break;		
		case BLE_tmpbuf_ONE_CLICK_HEAD_FOOT_UP:   ble_value_temp = KEY_ONE_CLICK_BACKLEG_UP;break;
		case BLE_tmpbuf_ONE_CLICK_HEAD_FOOT_DOWN: ble_value_temp = KEY_ONE_CLICK_BACKLEG_DOWN;break;
		case BLE_tmpbuf_ONE_CLICK_ALL_UP:         ble_value_temp = KEY_ONE_CLICK_ALL_UP;break;
		case BLE_tmpbuf_ONE_CLICK_ALL_DOWN:       ble_value_temp = KEY_ONE_CLICK_ALL_DOWN;break;
		case BLE_tmpbuf_ONE_CLICK_LUMBAR_UP:      ble_value_temp = KEY_ONE_CLICK_LUMBAR_UP;break;
		case BLE_tmpbuf_ONE_CLICK_LUMBAR_DOWN:    ble_value_temp = KEY_ONE_CLICK_LUMBAR_DOWN;break;
		case BLE_tmpbuf_ONE_CLICK_NECK_UP:        ble_value_temp = KEY_ONE_CLICK_NECK_UP;break;
		case BLE_tmpbuf_ONE_CLICK_NECK_DOWN:      ble_value_temp = KEY_ONE_CLICK_NECK_DOWN;break;		
		case BLE_tmpbuf_ONE_CLICK_LUMBAR2_UP:     ble_value_temp = KEY_ONE_CLICK_LUMBAR2_UP;break;
		case BLE_tmpbuf_ONE_CLICK_LUMBAR2_DOWN:   ble_value_temp = KEY_ONE_CLICK_LUMBAR2_DOWN;break;		
		case BLE_tmpbuf_ONE_CLICK_OUTER1_2_UP:    ble_value_temp = KEY_ONE_CLICK_LUMBAR_NECK_UP;break;
		case BLE_tmpbuf_ONE_CLICK_OUTER1_2_DOWN:  ble_value_temp = KEY_ONE_CLICK_LUMBAR_NECK_DOWN;break;		
		case BLE_tmpbuf_ONE_CLICK_NECK2_UP:       ble_value_temp = KEY_ONE_CLICK_NECK2_UP;break;
		case BLE_tmpbuf_ONE_CLICK_NECK2_DOWN:     ble_value_temp = KEY_ONE_CLICK_NECK2_DOWN;break;			
		/*---------------------------按摩预设相关-----------------------------*/
		//位置指令
		case BLE_tmpbuf_MEM1_SAVE:      ble_value_temp = KEY_MEM_M1;break;
		case BLE_tmpbuf_MEM1:           ble_value_temp = KEY_GO_M1;break;
		
		case BLE_tmpbuf_MEM2_SAVE:      ble_value_temp = KEY_MEM_M2;break;
		case BLE_tmpbuf_MEM2:           ble_value_temp = KEY_GO_M2;break;		
		
		case BLE_tmpbuf_MEM3_SAVE:      ble_value_temp = KEY_MEM_M3;break;
		case BLE_tmpbuf_MEM3:           ble_value_temp = KEY_GO_M3;break;

		case BLE_tmpbuf_MEM4_SAVE:      ble_value_temp = KEY_MEM_M4;break;
		case BLE_tmpbuf_MEM4:           ble_value_temp = KEY_GO_M4;break;
		
		case BLE_tmpbuf_TV_RESET:       ble_value_temp = KEY_MEM_TV;break;
		case BLE_tmpbuf_TV:             ble_value_temp = KEY_GO_TV;break;
		
		case BLE_tmpbuf_ZG_RESET:       ble_value_temp = KEY_MEM_ZEROG;break;
		case BLE_tmpbuf_ZG:             ble_value_temp = KEY_GO_ZEROG;break;		
		
		case BLE_tmpbuf_LOUNGE_RESET:   ble_value_temp = KEY_MEM_LOUNGE;break;
		case BLE_tmpbuf_LOUNGE:         ble_value_temp = KEY_GO_LOUNGE;break;	

		case BLE_tmpbuf_SNORE_RESET: 	 	ble_value_temp = KEY_MEM_SNORE;break;
    case BLE_tmpbuf_SNORE:          ble_value_temp = KEY_GO_SNORE;break;	

		case BLE_tmpbuf_READ_RESET:     ble_value_temp = KEY_MEM_READ;break;
		case BLE_tmpbuf_READ:           ble_value_temp = KEY_GO_READ;break;	
		
		case BLE_tmpbuf_YOGA_RESET:     ble_value_temp = KEY_MEM_YOGA;break;
		case BLE_tmpbuf_YOGA:           ble_value_temp = KEY_GO_YOGA;break;	
		
		case BLE_tmpbuf_FLAT:  					ble_value_temp = KEY_FLAT;break;
		
		case BLE_tmpbuf_GETUP:			 		ble_value_temp = KEY_GO_GETUP;break;	
		case BLE_tmpbuf_GETUP_RESET:		ble_value_temp = KEY_MEM_GETUP;break;
		case BLE_tmpbuf_NURSING:				ble_value_temp = KEY_GO_NURSING;break;	
		case BLE_tmpbuf_NURSING_RESET:	ble_value_temp = KEY_MEM_NURSING;break;

		case BLE_tmpbuf_SLEEP_MODE:  		ble_value_temp = KEY_DEMO1_MODE;break; 
		case BLE_tmpbuf_SLEEP_TWO: 		ble_value_temp = KEY_DEMO2_MODE;break;

		case BLE_tmpbuf_MOTOR_FOLLOW_MODE_SW: ble_value_temp = KEY_MOTOR_FOLLOW_MODE_SW;break;
		/*---------------------------按摩器控制相关-----------------------------*/
		
		case BLE_tmpbuf_MUSIC_FOLLOW_INTS_ADD:  ble_value_temp = KEY_MUSIC_FOLLOW_INTS_ADD;break; //随动模式
		case BLE_tmpbuf_MUSIC_FOLLOW_INTS_ZERO: ble_value_temp = KEY_MUSIC_FOLLOW_INTS_ZERO;break; //音乐随动模式
		case BLE_tmpbuf_MUSIC_FOLLOW_INTS_ONE:  ble_value_temp = KEY_MUSIC_FOLLOW_INTS_ONE;break; //音乐随动模式
		case BLE_tmpbuf_MUSIC_FOLLOW_INTS_TWO:  ble_value_temp = KEY_MUSIC_FOLLOW_INTS_TWO;break; //音乐随动模式
		case BLE_tmpbuf_MUSIC_FOLLOW_INTS_THREE:ble_value_temp = KEY_MUSIC_FOLLOW_INTS_THREE;break; //音乐随动模式

		case BLE_tmpbuf_MODE:               ble_value_temp = KEY_MSGR_MODE;break;
		case BLE_tmpbuf_MSGR1_INTS_ADD:     ble_value_temp = KEY_MSGR1_INTS_ADD;break;
		case BLE_tmpbuf_MSGR1_INTS_DCR:     ble_value_temp = KEY_MSGR1_INTS_DCR;break;
		case BLE_tmpbuf_MSGR2_INTS_ADD:     ble_value_temp = KEY_MSGR2_INTS_ADD;break;
		case BLE_tmpbuf_MSGR2_INTS_DCR:     ble_value_temp = KEY_MSGR2_INTS_DCR;break;
		case BLE_tmpbuf_MSGR3_INTS_ADD:     ble_value_temp = KEY_MSGR3_INTS_ADD;break;
		case BLE_tmpbuf_MSGR3_INTS_DCR:     ble_value_temp = KEY_MSGR3_INTS_DCR;break;

		case BLE_tmpbuf_MSGR_ALL_ON:        ble_value_temp = KEY_MSGR_ALL_ON;break;
		case BLE_tmpbuf_MSGR_10MIN:         ble_value_temp = KEY_MSGR_10MIN;break;
		case BLE_tmpbuf_MSGR_20MIN:         ble_value_temp = KEY_MSGR_20MIN;break;
		case BLE_tmpbuf_MSGR_30MIN:         ble_value_temp = KEY_MSGR_30MIN;break;
		case BLE_tmpbuf_MASSAGE_OFF: ble_value_temp = KEY_MSGR_ALL_OFF;break;
		case BLE_tmpbuf_MASSAGE_CON: ble_value_temp = KEY_MSGR_CONSTANT;break;
		case BLE_tmpbuf_MASSAGE_PLUS: ble_value_temp = KEY_MSGR_PULSE;break;
		case BLE_tmpbuf_MASSAGE_WAVE: ble_value_temp = KEY_MSGR_WAVE;break;
		
		case BLE_tmpbuf_MASSAGE_1_ON:ble_value_temp = KEY_MSGR1_ON;break;		
		case BLE_tmpbuf_MASSAGE_1_OFF:ble_value_temp = KEY_MSGR1_INTS_OFF;break;
		case BLE_tmpbuf_MASSAGE_1_ONE: ble_value_temp = KEY_MSGR1_INTS_ONE; break;
		case BLE_tmpbuf_MASSAGE_1_TWO: ble_value_temp = KEY_MSGR1_INTS_TWO; break;
		case BLE_tmpbuf_MASSAGE_1_THREE: ble_value_temp = KEY_MSGR1_INTS_THREE; break;

		case BLE_tmpbuf_MASSAGE_2_ON:ble_value_temp = KEY_MSGR2_ON;break;		
		case BLE_tmpbuf_MASSAGE_2_OFF:ble_value_temp = KEY_MSGR2_INTS_OFF;break;
		case BLE_tmpbuf_MASSAGE_2_ONE: ble_value_temp = KEY_MSGR2_INTS_ONE; break;
		case BLE_tmpbuf_MASSAGE_2_TWO: ble_value_temp = KEY_MSGR2_INTS_TWO; break;
		case BLE_tmpbuf_MASSAGE_2_THREE: ble_value_temp = KEY_MSGR2_INTS_THREE; break;
		
		case BLE_tmpbuf_MASSAGE_3_OFF:   ble_value_temp = KEY_MSGR3_INTS_OFF;break;
		case BLE_tmpbuf_MASSAGE_3_ONE:   ble_value_temp = KEY_MSGR3_INTS_ONE; break;
		case BLE_tmpbuf_MASSAGE_3_TWO:   ble_value_temp = KEY_MSGR3_INTS_TWO; break;
		case BLE_tmpbuf_MASSAGE_3_THREE: ble_value_temp = KEY_MSGR3_INTS_THREE; break;
		/*---------------------------灯控制相关----------------------------------*/
		case BLE_tmpbuf_UBL_SW:  ble_value_temp = KEY_UBL_SW;break;
		case BLE_tmpbuf_UBL_OFF:  ble_value_temp = KEY_UBL_OFF;break;
		case BLE_tmpbuf_LED_BOARD_SW:  ble_value_temp = KEY_LED_BOARD_SW;break;
		case BLE_tmpbuf_LED_BOARD_ON:  ble_value_temp = KEY_LED_BOARD_ON;break;
		case BLE_tmpbuf_LED_BOARD_OFF: ble_value_temp = KEY_LED_BOARD_OFF;break;
		case BLE_tmpbuf_UBL_ON:    		ble_value_temp = KEY_UBL_ON;break;
		case BLE_tmpbuf_LIGHT_SW:  ble_value_temp = KEY_LIGHT_SW;break;
		case BLE_tmpbuf_LIGHT_OFF:  ble_value_temp = KEY_LIGHT_OFF;break;
		
		/*-----------------------------音乐控制---------------------------------*/
		case BLE_tmpbuf_MUSIC_SW:          ble_value_temp = KEY_MUSIC_SW;break;
		case BLE_tmpbuf_MUSIC_PRE:         ble_value_temp = KEY_MUSIC_PRE;break;
		case BLE_tmpbuf_MUSIC_NEXT:        ble_value_temp = KEY_MUSIC_NEXT;break;		
		case BLE_tmpbuf_MUSIC_VOLUME_ADD:  ble_value_temp = KEY_MUSIC_VOLUME_ADD;break;
		case BLE_tmpbuf_MUSIC_VOLUME_DCR:  ble_value_temp = KEY_MUSIC_VOLUME_DCR;break;
		case BLE_tmpbuf_MEM_HELP_SLEEP:		 ble_value_temp = KEY_MUSIC_MEM_HELP_SLEEP;break;
		/*-----------------------------蓝牙控制---------------------------------*/
		case BLE_tmpbuf_MUSIC_BLE_SW:      ble_value_temp = KEY_MUSIC_BLE_SW;break;  //蓝牙开关
		case BLE_tmpbuf_MUSIC_TWS_SW:      ble_value_temp = KEY_MUSIC_TWS_SW;break;  //组队开关
		case BLE_tmpbuf_MUSIC_DEVICE_MODE: ble_value_temp = KEY_MUSIC_DEVICE_MODE;break;//正常/演示模式
		case BLE_tmpbuf_MUSIC_WHITE_NOISE:  ble_value_temp = KEY_MUSIC_WHITE_NOISE;break; //白噪音
		//
		case BLE_tmpbuf_KEY_RELEASE:  ble_value_temp = 0;break; //APP按键松开指令
		/*------------------------------加热垫风扇指令-------------------------------------*/
		case BLE_tmpbuf_HEAT_SW: 								ble_value_temp = KEY_HEAT_SW;break;
		case BLE_tmpbuf_HEAT_OFF: 							ble_value_temp = KEY_HEAT_OFF;break;
		case BLE_tmpbuf_HEAT_LOW: 							ble_value_temp = KEY_HEAT_LOW;break;
		case BLE_tmpbuf_HEAT_MID: 							ble_value_temp = KEY_HEAT_MID;break;
		case BLE_tmpbuf_FAN_ON: 								ble_value_temp = KEY_FAN_ON;break;
		case BLE_tmpbuf_FAN_OFF: 								ble_value_temp = KEY_FAN_OFF;break;
		case BLE_tmpbuf_FAN_MODE: 							ble_value_temp = KEY_FAN_MODE;break;
		case BLE_tmpbuf_FAN_CONSTANT: 					ble_value_temp = KEY_FAN_CONSTANT;break;
		case BLE_tmpbuf_FAN_PULSE: 							ble_value_temp = KEY_FAN_PULSE;break;
		case BLE_tmpbuf_FAN_WAVE: 							ble_value_temp = KEY_FAN_WAVE;break;
		case BLE_tmpbuf_FAN_2H: 								ble_value_temp = KEY_FAN_2H;break;
		case BLE_tmpbuf_FAN_3H: 								ble_value_temp = KEY_FAN_3H;break;
		case BLE_tmpbuf_FAN_5H: 								ble_value_temp = KEY_FAN_5H;break;
		case BLE_tmpbuf_FAN1_FORWARD_ONE: 			ble_value_temp = KEY_FAN1_FORWARD_ONE;break;
		case BLE_tmpbuf_FAN1_FORWARD_TWO: 			ble_value_temp = KEY_FAN1_FORWARD_TWO;break;
		case BLE_tmpbuf_FAN1_FORWARD_THREE: 		ble_value_temp = KEY_FAN1_FORWARD_THREE;break;
		case BLE_tmpbuf_FAN1_BACKWARD_ONE: 			ble_value_temp = KEY_FAN1_BACKWARD_ONE;break;
		case BLE_tmpbuf_FAN1_BACKWARD_TWO: 			ble_value_temp = KEY_FAN1_BACKWARD_TWO;break;
		case BLE_tmpbuf_FAN1_BACKWARD_THREE: 		ble_value_temp = KEY_FAN1_BACKWARD_THREE;break;
		case BLE_tmpbuf_FAN2_FORWARD_ONE: 			ble_value_temp = KEY_FAN2_FORWARD_ONE;break;
		case BLE_tmpbuf_FAN2_FORWARD_TWO: 			ble_value_temp = KEY_FAN2_FORWARD_TWO;break;
		case BLE_tmpbuf_FAN2_FORWARD_THREE: 		ble_value_temp = KEY_FAN2_FORWARD_THREE;break;
		case BLE_tmpbuf_FAN2_BACKWARD_ONE: 			ble_value_temp = KEY_FAN2_BACKWARD_ONE;break;
		case BLE_tmpbuf_FAN2_BACKWARD_TWO: 			ble_value_temp = KEY_FAN2_BACKWARD_TWO;break;
		case BLE_tmpbuf_FAN2_BACKWARD_THREE: 		ble_value_temp = KEY_FAN2_BACKWARD_THREE;break;
		case BLE_tmpbuf_FAN1_OFF: 							ble_value_temp = KEY_FAN1_OFF;break;
		case BLE_tmpbuf_FAN2_OFF: 							ble_value_temp = KEY_FAN2_OFF;break;		
		case BLE_tmpbuf_FAN1_INC: 							ble_value_temp = KEY_FAN1_INC;break;
		case BLE_tmpbuf_FAN2_INC: 							ble_value_temp = KEY_FAN2_INC;break;	
		case BLE_tmpbuf_FAN_DIR_SW:							ble_value_temp = KEY_FAN_DIR_SW;break;
		default:break;
	}
	
	
	return ble_value_temp ;
}

//获取控制键值
unsigned char Get_Key_Value(void)
{
	return key_value;
}
unsigned char Get_Sync_Key_State(void)
{
	return sync_key_state;
}
void Set_Sync_Key_State(unsigned char state_temp)
{
	sync_key_state = state_temp;
}
unsigned char Get_Sync_Run_Mode(void)
{
	return sync_run_mode;
}
void Set_Sync_Run_Mode(unsigned char mode_temp)
{
	sync_run_mode = mode_temp;
}

//管制码串口接接收函数
unsigned char Get_SourceCodeCmd_Analy(unsigned char recv_temp)
{
	static unsigned char Uart_CodeCmd_Buff[2] = {0};	
	
	static unsigned char recv_length = 0;
	
	if(recv_length < 2)
	{
		Uart_CodeCmd_Buff[recv_length ++] = recv_temp;
	}
	else
	{
		recv_length = 0;
	}
	
	if(1 == recv_length && Uart_CodeCmd_Buff[0] != 0x55)
	{
		recv_length = 0;
		Uart_CodeCmd_Buff[0] = 0;Uart_CodeCmd_Buff[1] = 0;
	}
	if(2 == recv_length && Uart_CodeCmd_Buff[1] != 0x35)
	{
		recv_length = 0;
		Uart_CodeCmd_Buff[0] = 0;Uart_CodeCmd_Buff[1] = 0;
	}
	if(recv_length >= 2)
	{
		Uart_CodeCmd_Buff[0] = 0;Uart_CodeCmd_Buff[1] = 0;
		
		for(recv_length = 0; recv_length < 13; recv_length++)
		{
			Write_Ring_Data(SOURCE_CODE_ID[recv_length]);
		}
		
		recv_length = 0;
		
		return 1;
	}

	return 0;
}
void Study_TimeManagerTask(void)
{
	// if(study_wifi_reset_flag == 1)
	// {
	// 	study_long_time ++;
	// 	if(study_long_time >= 1000)
	// 	{
	// 		study_long_time = 1000;
	// 	}
	// }
	// else
	// {
	// 	study_long_time = 0;
	// }
	if(old_a7105_value_temp != 0)
	{
		a7105_long_time ++;
		if(a7105_long_time >= 100)
		{
			a7105_long_time = 0;
			old_a7105_value_temp = 0;
		}
	}
	else
	{
		a7105_long_time = 0;
	}
}
