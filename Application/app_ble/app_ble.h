#ifndef __APP_BLE_H
#define __APP_BLE_H

#include "main.h"
#include "system.h"

#define BLE_FREE_LONG_TIME  (300/SYS_TIME_BASE)

#define BLE_BROKEN_FRAME_TIME  (50/SYS_TIME_BASE)



extern unsigned long control_ack_event;

extern unsigned short ble_key_value;

extern unsigned char ttl_vita_set_radioenable;

extern unsigned char ble_study_mode; 
void BleBlueTooth_Init(void);
unsigned char BleBlueTooth_Study_Mode(void);
void BleBlueTooth_Normal_Mode(void);
unsigned char calculate_checksum(const unsigned char *udata, unsigned char length);

void BleBlueTooth_RxServer(unsigned char uart_recv_value);//需要用户放在串口接收中断  并将每次接收的字节传参
void BleBlueTooth_TimerManager(void);
unsigned char BleBlueTooth_Get_KeyState(void);
unsigned char Ble_Comm_Free(void);

typedef enum
{
  BLE_tmpbuf_HEAD_UP     		= 0x0024,
  BLE_tmpbuf_HEAD_DOWN     	= 0x0025,
  BLE_tmpbuf_FOOT_UP     		= 0x0026,
  BLE_tmpbuf_FOOT_DOWN    	= 0x0027,
  BLE_tmpbuf_PAIR      			= 0x0028,
  BLE_tmpbuf_HEAD_FOOT_UP  	= 0x0029,
  BLE_tmpbuf_HEAD_FOOT_DOWN = 0x002a,
  BLE_tmpbuf_MEM1_SAVE    = 0x002b,
  BLE_tmpbuf_MEM2_SAVE    = 0x002c,
  BLE_tmpbuf_MEM3_SAVE    = 0x002d,
	BLE_tmpbuf_MEM4_SAVE    = 0x00b3,
  BLE_tmpbuf_MEM1       	= 0x002e,
  BLE_tmpbuf_MEM2       	= 0x002f,
  BLE_tmpbuf_MEM3       	= 0x0030,
  BLE_tmpbuf_MEM4       	= 0x00b2,
  BLE_tmpbuf_FLAT     		= 0x0031,
  BLE_tmpbuf_MASSAGE_1_SWITCH = 0x0032,
  BLE_tmpbuf_MASSAGE_2_SWITCH = 0x0033,
  BLE_tmpbuf_L1_L2_UP = 0x0043,	
  BLE_tmpbuf_L1_L2_DOWN = 0x0044,	
  BLE_tmpbuf_L3_UP = 0x0071,	
  BLE_tmpbuf_L3_DOWN = 0x0072,	
  BLE_tmpbuf_NECK2_UP = 0x007A,	
  BLE_tmpbuf_NECK2_DOWN = 0x007B,	
	//前倾
	BLE_tmpbuf_FORWARD = 0x005B,	
	//后倾
	BLE_tmpbuf_BACKWARD = 0x005C,	
  //倾斜升降
	BLE_tmpbuf_TILT1_UP = 0x0073,	
	BLE_tmpbuf_TILT1_DOWN = 0x0074,	
	BLE_tmpbuf_TILT2_UP = 0x00D0,	
	BLE_tmpbuf_TILT2_DOWN = 0x00D1,

	BLE_tmpbuf_ONE_CLICK_HEAD_UP = 0x007c,
	BLE_tmpbuf_ONE_CLICK_HEAD_DOWN = 0x007d,
	BLE_tmpbuf_ONE_CLICK_FOOT_UP = 0x007e,
	BLE_tmpbuf_ONE_CLICK_FOOT_DOWN = 0x007f,
	BLE_tmpbuf_ONE_CLICK_HEAD_FOOT_UP = 0x0080,
	BLE_tmpbuf_ONE_CLICK_HEAD_FOOT_DOWN = 0x0081,
	BLE_tmpbuf_ONE_CLICK_ALL_UP = 0x0082,
	BLE_tmpbuf_ONE_CLICK_ALL_DOWN = 0x0083,
	BLE_tmpbuf_ONE_CLICK_LUMBAR_UP = 0x008d,
	BLE_tmpbuf_ONE_CLICK_LUMBAR_DOWN = 0x008e,
	BLE_tmpbuf_ONE_CLICK_NECK_UP = 0x008f,
	BLE_tmpbuf_ONE_CLICK_NECK_DOWN = 0x0091,
	BLE_tmpbuf_ONE_CLICK_LUMBAR2_UP = 0x0092,
	BLE_tmpbuf_ONE_CLICK_LUMBAR2_DOWN = 0x0093,
	BLE_tmpbuf_ONE_CLICK_OUTER1_2_UP = 0x0094,
	BLE_tmpbuf_ONE_CLICK_OUTER1_2_DOWN = 0x0095,
	BLE_tmpbuf_ONE_CLICK_NECK2_UP = 0x00c0,
	BLE_tmpbuf_ONE_CLICK_NECK2_DOWN = 0x00c1,

	BLE_tmpbuf_MASSAGE_PLUS = 0X0038,
	BLE_tmpbuf_MASSAGE_WAVE = 0X0039,
	BLE_tmpbuf_MASSAGE_CON = 0X003A,
	
  BLE_tmpbuf_MOTOR3_UP = 0x003F,
  BLE_tmpbuf_MOTOR3_DOWN = 0x0040,
  BLE_tmpbuf_MOTOR4_UP = 0x0041,
  BLE_tmpbuf_MOTOR4_DOWN = 0x0042,
  BLE_tmpbuf_ZG     = 0X0045,
  BLE_tmpbuf_SNORE      = 0X0046,
  BLE_tmpbuf_ALL_STOP   = 0x0047,
  BLE_tmpbuf_MODE     = 0X0048,

	BLE_tmpbuf_GETUP = 0x003D,	
  BLE_tmpbuf_TV      = 0X0058,
  BLE_tmpbuf_LOUNGE    = 0x0059,
  BLE_tmpbuf_MASSAGE_OFF   = 0x005E,
  
  BLE_tmpbuf_TV_RESET      = 0X0064, 
  BLE_tmpbuf_LOUNGE_RESET      = 0x0065,
  BLE_tmpbuf_ZG_RESET      = 0x0066,
  BLE_tmpbuf_SNORE_RESET      = 0x0069,
	BLE_tmpbuf_GETUP_RESET = 0x00F8,
	BLE_tmpbuf_NURSING = 0x006C,
	BLE_tmpbuf_NURSING_RESET = 0x006D,
	
  BLE_tmpbuf_UBL_OFF = 0x0075,
  BLE_tmpbuf_LOCK = 0x0084,
  BLE_tmpbuf_WAKE_UP    = 0x003d,
  BLE_tmpbuf_HELP_SLEEP      = 0x00c5,
	BLE_tmpbuf_MEM_HELP_SLEEP  = 0x0089,
  BLE_tmpbuf_YOGA = 0x006B,
	BLE_tmpbuf_YOGA_RESET = 0x006A,
	BLE_tmpbuf_SLEEP_MODE = 0x0086,
	BLE_tmpbuf_SLEEP_TWO = 0x0087,
	BLE_tmpbuf_MOTOR_FOLLOW_MODE_SW = 0x0085,
	BLE_tmpbuf_SYNC_MODE_SW = 0x00BC,
	BLE_tmpbuf_MASSAGE_1_ON = 0x0096,
	BLE_tmpbuf_MASSAGE_1_OFF = 0x0098,
	BLE_tmpbuf_MASSAGE_1_ONE = 0x0099,
	BLE_tmpbuf_MASSAGE_1_TWO = 0x009A,
	BLE_tmpbuf_MASSAGE_1_THREE = 0x009B,

	BLE_tmpbuf_MASSAGE_2_ON = 0x0097,
	BLE_tmpbuf_MASSAGE_2_OFF = 0x009C,
	BLE_tmpbuf_MASSAGE_2_ONE = 0x009D,
	BLE_tmpbuf_MASSAGE_2_TWO = 0x009E,
	BLE_tmpbuf_MASSAGE_2_THREE = 0x009F,

	BLE_tmpbuf_MASSAGE_3_OFF = 0x00E1,
	BLE_tmpbuf_MASSAGE_3_ONE = 0x00E2,
	BLE_tmpbuf_MASSAGE_3_TWO = 0x00E3,
	BLE_tmpbuf_MASSAGE_3_THREE = 0x00E4,

	BLE_tmpbuf_HEAT_SW = 0X21,
	BLE_tmpbuf_HEAT_OFF = 0X77,
	BLE_tmpbuf_HEAT_LOW = 0X78,
	BLE_tmpbuf_HEAT_MID = 0X79,
	
  BLE_tmpbuf_READ = 0x00F2,
  BLE_tmpbuf_READ_RESET = 0x00F3,
	BLE_tmpbuf_FAN_ON = 0xA0,
	BLE_tmpbuf_FAN_OFF   = 0xA1,
	BLE_tmpbuf_FAN_MODE = 0xA2,  		//  模式切换
	BLE_tmpbuf_FAN_CONSTANT = 0xA3, //  持续
	BLE_tmpbuf_FAN_PULSE = 0xA4, 		//  脉冲
	BLE_tmpbuf_FAN_WAVE = 0xA5,			//  波浪
	BLE_tmpbuf_FAN_2H = 0xA6,
	BLE_tmpbuf_FAN_3H = 0xA7,
	BLE_tmpbuf_FAN_5H = 0xA8,
	BLE_tmpbuf_FAN1_FORWARD_ONE = 0xA9,
	BLE_tmpbuf_FAN1_FORWARD_TWO = 0xAA,
	BLE_tmpbuf_FAN1_FORWARD_THREE = 0xAB,
	BLE_tmpbuf_FAN1_BACKWARD_ONE = 0xAC,
	BLE_tmpbuf_FAN1_BACKWARD_TWO = 0xAD,
	BLE_tmpbuf_FAN1_BACKWARD_THREE = 0xAE,	
	BLE_tmpbuf_FAN2_FORWARD_ONE = 0xAF,
	BLE_tmpbuf_FAN2_FORWARD_TWO = 0xB0,
	BLE_tmpbuf_FAN2_FORWARD_THREE = 0xB1,
	BLE_tmpbuf_FAN2_BACKWARD_ONE = 0xB6,
	BLE_tmpbuf_FAN2_BACKWARD_TWO = 0xB7,
	BLE_tmpbuf_FAN2_BACKWARD_THREE = 0xB8,	
	BLE_tmpbuf_FAN1_OFF = 0xB9,
	BLE_tmpbuf_FAN2_OFF = 0xBA,	
	BLE_tmpbuf_FAN1_INC = 0x8A,
	BLE_tmpbuf_FAN2_INC = 0x8B,	
	BLE_tmpbuf_FAN_DIR_SW = 0x8C,
  tmpbuf_MAX        = 0x00ff,

	/* 额外按键映射（供 Ble_Analy_KeyValue 使用） */
	BLE_tmpbuf_MUSIC_LOCK          = 0x00D9,
	BLE_tmpbuf_DATA_RESET          = 0x0062,
	BLE_tmpbuf_MUSIC_FOLLOW_INTS_ADD   = 0x00D6,
	BLE_tmpbuf_MUSIC_FOLLOW_INTS_ZERO  = 0x00DC,
	BLE_tmpbuf_MUSIC_FOLLOW_INTS_ONE   = 0x00DD,
	BLE_tmpbuf_MUSIC_FOLLOW_INTS_TWO   = 0x00DE,
	BLE_tmpbuf_MUSIC_FOLLOW_INTS_THREE = 0x00DF,
	BLE_tmpbuf_MSGR1_INTS_ADD      = 0x004C,
	BLE_tmpbuf_MSGR1_INTS_DCR      = 0x004D,
	BLE_tmpbuf_MSGR2_INTS_ADD      = 0x004E,
	BLE_tmpbuf_MSGR2_INTS_DCR      = 0x004F,
	BLE_tmpbuf_MSGR3_INTS_ADD      = 0x00E0,
	BLE_tmpbuf_MSGR3_INTS_DCR      = 0x00E7,
	BLE_tmpbuf_MSGR_ALL_ON         = 0x005D,
	BLE_tmpbuf_MSGR_10MIN          = 0x005F,
	BLE_tmpbuf_MSGR_20MIN          = 0x0063,
	BLE_tmpbuf_MSGR_30MIN          = 0x0061,
	BLE_tmpbuf_UBL_SW              = 0x003C,
	BLE_tmpbuf_LED_BOARD_SW        = 0x0050,
	BLE_tmpbuf_LED_BOARD_ON        = 0x0051,
	BLE_tmpbuf_LED_BOARD_OFF       = 0x0052,
	BLE_tmpbuf_UBL_ON              = 0x00BF,
	BLE_tmpbuf_MUSIC_SW            = 0x00D7,
	BLE_tmpbuf_MUSIC_PRE           = 0x00D2,
	BLE_tmpbuf_MUSIC_NEXT          = 0x00D3,
	BLE_tmpbuf_MUSIC_VOLUME_ADD    = 0x00D4,
	BLE_tmpbuf_MUSIC_VOLUME_DCR    = 0x00D5,
	BLE_tmpbuf_MUSIC_BLE_SW        = 0x00D8,
	BLE_tmpbuf_MUSIC_TWS_SW        = 0x00DA,
	BLE_tmpbuf_MUSIC_DEVICE_MODE   = 0x00C4,
	BLE_tmpbuf_KEY_RELEASE         = 0x006e,
	BLE_tmpbuf_MUSIC_WHITE_NOISE 	 = 0x00CA,
	BLE_tmpbuf_LIGHT_SW						 = 0x00C8,
	BLE_tmpbuf_LIGHT_OFF					 = 0x00FD,
	BLE_tmpbuf_LIGHT_WARM					 = 0x00FA,
	BLE_tmpbuf_LIGHT_NEUTRAL			 = 0x00FB,
	BLE_tmpbuf_LIGHT_COLD		 			 = 0x00FC,

} BT_VALUE;

#endif






