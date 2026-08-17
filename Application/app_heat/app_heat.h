#ifndef __APP_HEAT_H
#define __APP_HEAT_H

typedef struct
{
	unsigned short heat_ms_count ;  //毫秒计时
	unsigned long  heat_min_count ; //秒计时
	unsigned long  heat_time_min_set ;  
	
	unsigned char heat_state;
	unsigned char old_heat_state;
}HEAT_STRUCT;
typedef enum 
{
	HEAT_LEVEL_0_PWM     	= 0,
	HEAT_LEVEL_1_PWM     	= 10000,
	HEAT_LEVEL_MAX_PWM		= 20000,
}HEAT_LEVEL_PWM;

extern HEAT_STRUCT  Heat_Stu;

#define HEAT_SWITCH_ON_EVENT  (0X00000001)
#define HEAT_SWITCH_OFF_EVENT  (0X00000002)
extern unsigned long heat_para_set_event;

void APP_HeatInit(void);
void Heat_Control(void);
void Heat_TimeManagerTask(void);
#endif
