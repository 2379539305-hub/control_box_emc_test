#ifndef __MODUL_MOTOR_H
#define __MODUL_MOTOR_H

#include "main.h"
#include "system.h"

#define MOTOR_PWM_MAX  250
#define MOTOR_PWM_MIN  75

#define MOTOR_CTR_TIME 4 //速度变化时间
#define RELAY_MOS_DELAY  20 //MOS关闭后延时多久关闭继电器 毫秒

//电机运行方向
typedef enum 
{
  MOTOR_DIR_NO = 0,
  MOTOR_DIR_UP = 1,
  MOTOR_DIR_DOWN = 2
}MOTOR_DIR;

typedef enum 
{
	LIMIT_DIR_NO = 0,
  LIMIT_DIR_UP = 1,
  LIMIT_DIR_DOWN = 2,
  LIMIT_DIR_MID = 3
}LIMIT_DIR;

typedef struct
{
	unsigned char motor_port;
	unsigned char *motor_type;
  unsigned char motor_dir;
  unsigned char motor_relay_dir;
	unsigned char realy_delay_time;
  unsigned char motor_pwm_duty;
  unsigned char motor_pwm_max;
  unsigned char motor_slowchange_step;
  unsigned char motor_slowchange_time;
	
	unsigned char motor_limit_flag;
	
	unsigned char motor_hall_time;
	unsigned char motor_run_time;
	
	unsigned short hall_change_time;
	unsigned short motor_adc_count;
	
	unsigned short *motorADC_value;
	
	
	unsigned short hall_run_num;
	unsigned short hall_old_num;
	unsigned short old_hall_run_num;
	
	float motor_run_hall_temp;
	
	void (*Motor_Up)(void);
	void (*Motor_Down)(void);
	void (*Motor_Stop)(void);
	void (*Motor_Speed)(u16 Compare);
	unsigned char (*Get_Hall_Level)(void);
	unsigned char motor_slow_run_flag;
}MOTOR_BASE_PARA;


extern MOTOR_BASE_PARA  Motor1_ParaStu;
extern MOTOR_BASE_PARA  Motor2_ParaStu;
extern MOTOR_BASE_PARA  Motor3_ParaStu;
extern MOTOR_BASE_PARA  Motor4_ParaStu;
extern MOTOR_BASE_PARA  Motor5_ParaStu;
extern MOTOR_BASE_PARA  Motor6_ParaStu;
extern MOTOR_BASE_PARA  Motor_Invalid_ParaStu;
/*----------------------霍尔相关检测-----------------------------*/
#define HALL_STOP_NUM  (0)

#define HALL_DOWN_NUM  (-2147483647)
#define HALL_UP_NUM    (0XFFFF)

#define HALL_ERROR_NUM  (5)
#define CURRENT_ERROR_NUM  (100)

#define HALL_MIN_NUM   (10000)

void Motor_Func_Init(void(*callback)(void));

void Motor_Run(MOTOR_BASE_PARA *Motor_ParaStu);
void Motor_PwmChange(MOTOR_BASE_PARA *Motor_ParaStu);
void Motor_PwmMax(MOTOR_BASE_PARA *Motor_ParaStu,u8 pwm_max);
void Motor_PwmSlowStart(MOTOR_BASE_PARA *Motor_ParaStu);
void Motor_PwmSlowStop(MOTOR_BASE_PARA *Motor_ParaStu);
void Motor_PwmImStop(MOTOR_BASE_PARA *Motor_ParaStu);

void Motor_Hall_TakePositionTask(void);
void Motor_Hall_LimitTask(void);
void Motor_Current_LimitTask(void);

unsigned char Motor_ArrivePosition(MOTOR_BASE_PARA *Motor_ParaStu, long hall_target_num);
void Motor_Para_Reset(MOTOR_BASE_PARA *Motor_ParaStu);

unsigned char Get_Motor_PortState(void);

void Motor_saveAdc_Task(void);
void Motor_Adc_Init(void);
unsigned char Get_Motor_AllReset(void);

#endif






