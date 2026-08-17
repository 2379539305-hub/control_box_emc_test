#ifndef __APP_FAN_H
#define __APP_FAN_H


#define FAN_ALL_INTS_EVENT      (0X0000000F)
#define FAN_MODE_TIME_EVENT     (0X00000020)
extern unsigned long fan_para_set_event; 

//按摩时间选择
typedef enum 
{
  FAN_TIME_SHORT = 120,
  FAN_TIME_MID = 180,
  FAN_TIME_LONG = 300,
	
}TYPE_FAN_TIME;
//按摩模式选择  数值对应音乐阵子的模式
typedef enum 
{
	FAN_NONE_MODE = 0,
  FAN_CONSTANT_MODE = 1,
  FAN_PULSE_MODE = 2,
  FAN_WAVE_MODE = 3,
}TYPE_FAN_MODE;
//按摩模式选择  数值对应音乐阵子的模式
typedef enum 
{
	FAN_FORWARD = 0,
	FAN_BACKWARD = 1,
}TYPE_FAN_DIR;

//按摩强度等级
typedef enum 
{
	FAN_INTS_ZERO_LEVEL = 0,
	FAN_INTS_ONE_LEVEL = 1,
	FAN_INTS_TWO_LEVEL = 2,
	FAN_INTS_THREE_LEVEL = 3,
	FAN_INTS_MAX_LEVEL
}TYPE_FAN_INTS_LEVEL;

typedef struct
{
	unsigned short fan_ms_count ;  //毫秒计时
	unsigned short fan_sec_count ;  //毫秒计时
	unsigned long  fan_min_count ; //秒计时
	unsigned long  fan_time_min_set ;  
	unsigned char fan_mode_set;
	unsigned char Fan_Ints_FlagArr[5]; 
	unsigned char Fan_Dir_FlagArr[5]; 
	unsigned char fan_state;
	unsigned char old_fan_state;

}FAN_STRUCT;
extern FAN_STRUCT  Fan_Stu;

void APP_FanInit(void);
void Fan_Control(void);
void User_SetFan_Mode(unsigned char *ints, unsigned char mode, unsigned char *dir, unsigned char set_time);
void Fan_Clear_TimeCount(void);
void Fan_TimeManagerTask(void);
#endif
