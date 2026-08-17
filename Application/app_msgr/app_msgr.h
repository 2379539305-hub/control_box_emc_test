#ifndef _APP_MSGR_H_
#define _APP_MSGR_H_

#include "main.h"
#include "system.h"

//#define MSGR_ALL_INTS_EVENT      (0X00000001)
//#define MSGR_ALL_INTS_EVENT      (0X00000002)
//#define MSGR_ALL_INTS_EVENT      (0X00000004)
//#define MSGR_ALL_INTS_EVENT      (0X00000008)
#define MSGR_ALL_INTS_EVENT      (0X0000000F)
#define MSGR_FOLLOW_INTS_EVENT   (0X00000010)
#define MSGR_MODE_TIME_EVENT     (0X00000020)

extern unsigned long msgr_para_set_event; 

extern unsigned long msgr_state_updata_event;

//按摩器模式  随振  持续  脉冲  波浪
extern unsigned char msgr_mode_set;
//按摩器定时
extern unsigned char msgr_min_time_set;

extern unsigned char msgr_mode_stop_flag; //模式切换  需要停止1S在启动
extern unsigned char msgr_stop_time;

//头部1按摩器强度  
//脚部2按摩器强度  
#define SYS_MSGR_NUM (3)
extern unsigned char Msgr_Ints_FlagArr[SYS_MSGR_NUM + 1];

//按摩时间选择
typedef enum 
{
  MSGR_TIME_SHORT = 10,
  MSGR_TIME_MID = 20,
  MSGR_TIME_LONG = 30,
	
}TYPE_MSGR_TIME;

//按摩模式选择  数值对应音乐阵子的模式
typedef enum 
{
	MSGR_FOLLOW_MODE = 0,
  MSGR_CONSTANT_MODE = 3,
  MSGR_PULSE_MODE = 7,
  MSGR_WAVE_MODE = 11,
}TYPE_MSGR_MODE;


//按摩强度等级
typedef enum 
{
	MSGR_INTS_ZERO_LEVEL = 0,
	MSGR_INTS_ONE_LEVEL = 1,
	MSGR_INTS_TWO_LEVEL = 2,
	MSGR_INTS_THREE_LEVEL = 3,
	MSGR_INTS_MAX_LEVEL
}TYPE_MSGR_INTS_LEVEL;

//按摩强度PWM
typedef enum 
{
//模式周期
//	脉冲
#define MSGR_PULSE_CYCLE_TIME   (750/SYS_TIME_BASE)  //1400ms  
// 波浪
#define MSGR_WAVE_ONE_CYCLE_TIME    (1500/SYS_TIME_BASE) 
#define MSGR_WAVE_TWO_CYCLE_TIME    (4000/SYS_TIME_BASE)  
#define MSGR_WAVE_THREE_CYCLE_TIME  (7000/SYS_TIME_BASE) 	
	//
	MSGR_CONSTANT_ONE_PWM = 2000,//10%
	MSGR_CONSTANT_TWO_PWM = 4000,//20%
	MSGR_CONSTANT_THREE_PWM = 6000,//30%
	//
	MSGR_PULSE_ONE_PWM = 3200,//16%
	MSGR_PULSE_TWO_PWM = 5600,//28%
	MSGR_PULSE_THREE_PWM = 8000,//40%
	//
	MSGR_WAVE_LOW_PWM = 1200,//6%
	MSGR_WAVE_ONE_PWM = 2400,//12%
	MSGR_WAVE_TWO_PWM = 4800,//24%
	MSGR_WAVE_THREE_PWM = 7200,//36%	
	
	MSGR3_CONSTANT_ONE_PWM = 2400,//10%
	MSGR3_CONSTANT_TWO_PWM = 2800,//20%  
	MSGR3_CONSTANT_THREE_PWM = 3600,//30%
	//
	MSGR3_PULSE_ONE_PWM = 3400,//16%
	MSGR3_PULSE_TWO_PWM = 3800,//28%
	MSGR3_PULSE_THREE_PWM = 4600,//40%
	//
	MSGR3_WAVE_LOW_PWM = 1500,//6%
	MSGR3_WAVE_ONE_PWM = 3000,//12%
	MSGR3_WAVE_TWO_PWM = 3500,//24%
	MSGR3_WAVE_THREE_PWM = 4500,//36%	
}TYPE_MSGR_INTS_PWM;


typedef struct
{
	unsigned char msgr_typeints_level;
	unsigned char msgr_followints_level;
	
  unsigned char  wave_dir;
	unsigned short cycle_time_count;
	unsigned short cycle_time_set;
	
	unsigned short msgr_pwm;
	unsigned short msgr_pwm_min;
	unsigned short msgr_pwm_max;
	
	float wave_time_slope;

}MSGR_BASE_PARA;


void Msgr_Pulse_Mode(MSGR_BASE_PARA *Msgr_ParaStu);
void Msgr_Wave_Mode(MSGR_BASE_PARA *Msgr_ParaStu);

void User_SetFollowInts_Level(unsigned char ints_temp);
void User_SetMassage_Mode(unsigned char *ints, unsigned char mode, unsigned char set_time);

void Msgr_Control(void);
void Msgr_Clear_TimeCount(void);
void Msgr_TimeManagerTask(void);

unsigned char Msgr_AcceptCmd_KeyInfo(unsigned char key_temp);
unsigned char User_Msgr_Demo(unsigned char step);
#endif














