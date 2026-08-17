#ifndef _APP_COMM_H_
#define _APP_COMM_H_

#include "system.h"

#define KEY_TTL_CMD_EVENT       (0X0001)

extern unsigned short key_para_set_event;


typedef enum  
{
	KEY_NO = 0,
	KEY_BOARD_LOCK = 1,
	KEY_LAMP = 2,
	KEY_STUDY_MODE = 3,     //进入学习模式
	KEY_CLEAR_STUDY_ID = 4,       //解除学习到的ID
	/*-----------------------------------------------------*/

	KEY_MUSIC_LOCK = 5,// 			音乐阵子锁
	KEY_ID_MATCH = 6, //    对码  wifi清除配网
	
	KEY_CTRBOX_SW = 7,//  控制盒上锁/解锁 
	KEY_ALL_STOP = 8,  //   停止所有驱动器
	KEY_SYNC_MODE_SW = 9, // 同步模式开关
	KEY_CLEAR_MESH_PAIR = 10,       //清除组网
	/*-------------------灯控制----------------------*/
#define KEY_LIGHT_START (20)
	
	KEY_UBL_ON 				= 21,
	KEY_UBL_SW 				= 22,//   床底灯开/状态切换
	KEY_UBL_OFF 			= 23,
	KEY_LED_BOARD_SW  = 24,//  灯牌打开/状态切换
	KEY_LED_BOARD_ON	= 25,
	KEY_LED_BOARD_OFF = 26,
	KEY_LIGHT_SW			= 27, //灯光开关
	KEY_LIGHT_OFF			= 28, //灯光关
#define KEY_LIGHT_END (29)
	/*-------------------推杆伸缩控制----------------------*/
#define KEY_MOTOR_START (30)
#define KEY_MOTOR_RUN_START  (30)	
#define KEY_MOTOR_COUNTINUE_RUN_START  (30)
	KEY_MOTOR_STOP 		= 30, 
	
	KEY_BACK_UP 			= 31, 
	KEY_BACK_DOWN 		= 32, 
	KEY_LEG_UP 				= 33, 
	KEY_LEG_DOWN 			= 34, 
	KEY_BACKLEG_UP 		= 35, 
	KEY_BACKLEG_DOWN 	= 36, 
	
	KEY_LUMBAR_UP 		= 37,
	KEY_LUMBAR_DOWN 	= 38,
	KEY_NECK_UP 			= 39,
	KEY_NECK_DOWN 		= 40,
		
	KEY_TILT_ALL_UP		 	= 41,
	KEY_TILT_ALL_DOWN 		= 42,
	KEY_LUMBAR2_UP		 	= 43,
	KEY_LUMBAR2_DOWN 		= 44,
	KEY_NECK2_UP 					= 45, 
	KEY_NECK2_DOWN 				= 46,	
	KEY_ALL_MOTOR_UP				= 47,	
	KEY_ALL_MOTOR_DOWN			= 48,			
	KEY_FORWARD = 49,
	KEY_BACKWARD = 50,
	KEY_TILT1_UP = 51,
	KEY_TILT1_DOWN = 52,
	KEY_TILT2_UP = 53,
	KEY_TILT2_DOWN = 54,

	//固定位置控制
#define KEY_MOTOR_COUNTINUE_RUN_END  (59)

#define KEY_MOTOR_ONE_CLICK_START  (60)
#define KEY_MOTOR_ONE_CLICK_RUN_START (60)
	KEY_FLAT 				= 60,
	KEY_GO_TV				= 61,
	KEY_GO_ZEROG		= 62,
	KEY_GO_LOUNGE		= 63,
	KEY_GO_SNORE		= 64,
	KEY_GO_M1				= 65,
	KEY_GO_M2				= 66,
	KEY_GO_M3				= 67,
	KEY_GO_READ 		= 68,
	KEY_GO_YOGA 		= 69,
	KEY_GO_GETUP		= 70,
	KEY_GO_M4				= 71,
	KEY_GO_M5				= 72,	
	KEY_ALARM_MODE = 73,
	KEY_ALARM_THREE_MODE = 74,//闹钟三次动作模式
	KEY_ALARM_MSGR_MODE  = 75,//闹钟按摩器模式	
	KEY_GO_NURSING	= 76,		
	KEY_DEMO1_MODE = 77,
	KEY_DEMO2_MODE = 78,
	KEY_DEMO3_MODE = 79,
	KEY_MOTOR_FOLLOW_UP 		= 80,
#define KEY_MOTOR_ONE_CLICK_RUN_END (80)	

#define KEY_MOTOR_RUN_END  (80)	
#define KEY_MOTOR_MEMORY_START  (81)	
	
	KEY_DATA_RESET 	= 81,  // 恢复出厂设置  控制盒的数据
	
	KEY_MEM_ZEROG 	= 82,
	KEY_MEM_TV 			= 83,    
	KEY_MEM_LOUNGE 	= 84,
	KEY_MEM_SNORE 	= 85,	
	KEY_MEM_M1 			= 86,
	KEY_MEM_M2 			= 87,
	KEY_MEM_M3 			= 88,
	KEY_MEM_READ 		= 89,
	KEY_MEM_YOGA 		= 90,
	KEY_MEM_GETUP 	= 91,
	KEY_MEM_NURSING	= 94,
	KEY_MEM_M4 			= 92,
	KEY_MEM_M5 			= 93,	
#define KEY_MOTOR_MEMORY_END  (99)
#define KEY_MOTOR_ONE_CLICK_END  (99)	

#define KEY_MOTOR_END (99)	
	/*-------------------按摩器控制---------------------*/
#define KEY_MSGR_START 	(100)		
	KEY_MSGR_TIMER_SET 		= 100,    // 按摩器定时时间设置 
	KEY_MSGR_TIMER_CANCEL = 101,  // 取消定时
	KEY_MSGR_10MIN 				= 102,   //定时十分钟
	KEY_MSGR_20MIN 				= 103,   //
	KEY_MSGR_30MIN 				= 104,   //
	
	KEY_MSGR_MODE 				= 105,  // 按摩器模式切换
	KEY_MSGR_CONSTANT 		= 106, //  持续
	KEY_MSGR_PULSE 				= 107, //  脉冲
	KEY_MSGR_WAVE 				= 108,	//  波浪
	
	KEY_MSGR_INTS_ADD		  = 109,  //强度增强
	KEY_MSGR_INTS_DCR		  = 110,  //强度减弱
	KEY_MSGR_INTS_ONE		  = 111,  //按摩器强度一
	KEY_MSGR_INTS_TWO		  = 112,  //按摩器强度二
	KEY_MSGR_INTS_THREE 	= 113,//按摩器强度三

	KEY_MSGR_ALL_ON 			= 114,  //  按摩器同时开
	KEY_MSGR_ALL_OFF 			= 115, //  按摩器同时关
	
	KEY_MSGR1_SW 					= 116, //按摩器一 开/关
	KEY_MSGR2_SW 					= 117, //按摩器二 开/关
	KEY_MSGR3_SW 					= 118, //按摩器二 开/关
	
	KEY_MSGR1_ON 					= 119, //按摩器一 开/关
	KEY_MSGR2_ON 					= 120, //按摩器二 开/关
	KEY_MSGR3_ON 					= 121, //按摩器二 开/关
	
	KEY_MSGR1_INTS_ADD 		= 122, //  按摩器一强度增强
	KEY_MSGR1_INTS_DCR 		= 123,	// 按摩器一强度减弱
	KEY_MSGR1_INTS_OFF 		= 124,	// 按摩器一关闭
	KEY_MSGR1_INTS_ONE 		= 125,// 按摩器一 强度一
	KEY_MSGR1_INTS_TWO 		= 126,// 按摩器一 强度二
	KEY_MSGR1_INTS_THREE 	= 127,	// 按摩器一 强度三
	
	KEY_MSGR2_INTS_ADD 		= 128, //  按摩器二强度增强
	KEY_MSGR2_INTS_DCR 		= 129,	// 按摩器二强度减弱
	KEY_MSGR2_INTS_OFF 		= 130,	// 按摩器2关闭
	KEY_MSGR2_INTS_ONE 		= 131,	// 按摩器一 强度一
	KEY_MSGR2_INTS_TWO 		= 132,	// 按摩器一 强度二
	KEY_MSGR2_INTS_THREE 	= 133,	// 按摩器一 强度三	
	
	
	KEY_MSGR3_INTS_ADD 		= 134,		// 按摩器三 增强
	KEY_MSGR3_INTS_DCR 		= 135,		// 按摩器三 减弱
	KEY_MSGR3_INTS_OFF 		= 136,		// 按摩器三 关闭
	KEY_MSGR3_INTS_ONE 		= 137,		// 按摩器三 强度一
	KEY_MSGR3_INTS_TWO 		= 138,		// 按摩器三 强度二
	KEY_MSGR3_INTS_THREE 	= 139,	// 按摩器三 强度三	
		
	KEY_MUSIC_FOLLOW_INTS_ADD 	= 140, //音乐随动增强  随振模式
	KEY_MUSIC_FOLLOW_INTS_DCR 	= 141, //音乐随动减弱
	
	KEY_MUSIC_FOLLOW_INTS_ZERO 	= 142,
	KEY_MUSIC_FOLLOW_INTS_ONE 	= 143,
	KEY_MUSIC_FOLLOW_INTS_TWO 	= 144,
	KEY_MUSIC_FOLLOW_INTS_THREE = 145,
	
#define KEY_MSGR_END (159)		
	/*-------------------音乐控制---------------------*/	
#define KEY_MUSIC_START (160)			
	//111
	KEY_MUSIC_SW 					= 161,	//音乐播放开关
	KEY_MUSIC_PLAY			  = 162,	//音乐播放
	KEY_MUSIC_PAUSE 			= 163,	//音乐暂停
	
	KEY_MUSIC_PRE 				= 164,//上一曲
	KEY_MUSIC_NEXT				= 165,//下一曲	
	KEY_MUSIC_VOLUME_ADD 	= 166,	//音乐音量加1
	KEY_MUSIC_VOLUME_DCR 	= 167,	//音乐音量减1
	
	KEY_MUSIC_BLE_SW 			= 168, //音乐蓝牙开关
	KEY_MUSIC_CHANNEL 		= 169, //音乐通道切换
	
	KEY_MUSIC_TWS_SW 			= 170, //音乐TWS组队开关
	KEY_MUSIC_TWS_ON 			= 171, //音乐TWS开启组队
	KEY_MUSIC_TWS_OFF	 		= 172, //音乐TWS关闭组队
	
	KEY_MUSIC_DEVICE_MODE = 179, //正常模式/演示模式
	KEY_HEAT_SW 					= 180, //加热垫开关
	KEY_HEAT_ON 					= 181,//加热垫开
	KEY_HEAT_OFF 					= 182,//加热垫关
	KEY_HEAT_LOW 					= 183,//加热垫关
	KEY_HEAT_MID					= 184,//加热垫关	
	KEY_MUSIC_MEM_HELP_SLEEP	= 185,//哄睡保存
	KEY_MUSIC_WHITE_NOISE			= 186, //白噪音开关
	KEY_MOTOR_FOLLOW_MODE_SW = 187, //马达随动模式开关	
#define KEY_MUSIC_END (189)			
	
#define KEY_FAN_START (190)	
	KEY_FAN_ON = 190,
	KEY_FAN_OFF   = 191,
	KEY_FAN_MODE = 192,  		//  模式切换
	KEY_FAN_CONSTANT = 193, //  持续
	KEY_FAN_PULSE = 194, 		//  脉冲
	KEY_FAN_WAVE = 195,			//  波浪
	KEY_FAN_2H = 196,
	KEY_FAN_3H = 197,
	KEY_FAN_5H = 198,
	KEY_FAN1_FORWARD_ONE = 199,
	KEY_FAN1_FORWARD_TWO = 200,
	KEY_FAN1_FORWARD_THREE = 201,
	KEY_FAN1_BACKWARD_ONE = 202,
	KEY_FAN1_BACKWARD_TWO = 203,
	KEY_FAN1_BACKWARD_THREE = 204,	
	KEY_FAN2_FORWARD_ONE = 205,
	KEY_FAN2_FORWARD_TWO = 206,
	KEY_FAN2_FORWARD_THREE = 207,
	KEY_FAN2_BACKWARD_ONE = 208,
	KEY_FAN2_BACKWARD_TWO = 209,
	KEY_FAN2_BACKWARD_THREE = 210,	
	KEY_FAN1_OFF = 211,
	KEY_FAN2_OFF = 212,
	KEY_FAN1_INC = 213,
	KEY_FAN2_INC = 214,
	KEY_FAN_DIR_SW = 215,
#define KEY_FAN_END (215)	

#define KEY_ONE_CLICK_RUN_START (220)
	KEY_ONE_CLICK_BACK_UP 			= 220, 
	KEY_ONE_CLICK_BACK_DOWN 		= 221, 
	KEY_ONE_CLICK_LEG_UP 			= 222,
	KEY_ONE_CLICK_LEG_DOWN 		= 223,
	KEY_ONE_CLICK_BACKLEG_UP 		= 224,
	KEY_ONE_CLICK_BACKLEG_DOWN 	= 225,
	KEY_ONE_CLICK_LUMBAR_UP 		= 226,
	KEY_ONE_CLICK_LUMBAR_DOWN 	= 227,
	KEY_ONE_CLICK_NECK_UP 			= 228,
	KEY_ONE_CLICK_NECK_DOWN 		= 229,
	KEY_ONE_CLICK_LUMBAR_NECK_UP		 	= 230,
	KEY_ONE_CLICK_LUMBAR_NECK_DOWN 		= 231,
	KEY_ONE_CLICK_LUMBAR2_UP		 	= 232,
	KEY_ONE_CLICK_LUMBAR2_DOWN 		= 233,
	KEY_ONE_CLICK_NECK2_UP 					= 234, 
	KEY_ONE_CLICK_NECK2_DOWN 				= 235,
	KEY_ONE_CLICK_ALL_UP					= 236,	
	KEY_ONE_CLICK_ALL_DOWN				= 237,			
#define KEY_ONE_CLICK_RUN_END (239)

	KEY_DELAY = 254,
  KEY_MAX = 255
}KEY_VALUE;

typedef enum  
{
	KEY_TYPE_ALL = 0,
	KEY_TYPE_SYS = 1,
	KEY_TYPE_MOTOR = 2,
	KEY_TYPE_MSGR = 3,
	KEY_MUSIC = 4,
	KEY_LIGHT = 5
}KEY_TYPE;

unsigned char Get_SourceCodeCmd_Analy(unsigned char recv_temp);

unsigned char  ControlBox_StudyCodeTask(void); //对码  返回成功还是失败
void Get_Key_InfoTask(void);

unsigned char A7105_Analy_KeyValue(void);
unsigned char Ble_Analy_KeyValue(unsigned char key_temp);

unsigned char Get_Key_Value(void);
unsigned char Get_Sync_Key_State(void);
void Set_Sync_Key_State(unsigned char state_temp);
unsigned char Get_Sync_Run_Mode(void);
void Set_Sync_Run_Mode(unsigned char mode_temp);
void Study_TimeManagerTask(void);
#endif














