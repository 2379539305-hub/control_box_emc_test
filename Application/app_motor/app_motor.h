#ifndef _APP_MOTOR_H_
#define _APP_MOTOR_H_

#include "main.h"
#include "system.h"

#include "app_save.h"
#include "modul_motor.h"


#define MOTOR_LIN_RUN_EVENT       (0X0001)
#define MOTOR_LIN_STOP_EVENT      (0X0002)


#define MOTOR_TARGET_POS_EVENT    (0X0008)
#define MOTOR_MEM_POS_EVENT       (0X0010)
#define MOTOR_CMD_RUN_EVENT				  (0X0020)
#define MOTOR_DEMO_SLEEP_RUN_EVENT  (0X0040)
#define MOTOR_ALARM_RUN_EVENT       (0X0080)
#define MOTOR_ALARM_THREE_RUN_EVENT (0X0100)
#define MOTOR_ALARM_MSGR_RUN_EVENT  (0X0200)
#define KEY_MOTOR_FOLLOW_MODE_SWITCH_EVENT (0X0400)
extern unsigned short motor_para_set_event;
extern unsigned char alarm_three_target_mode;
extern unsigned char alarm_three_phase;
extern unsigned char alarm_three_count;
extern unsigned char alarm_msgr_running;

unsigned char Motor_DemoMode_RunState(void);
unsigned char Motor_DemoMode_KeyInfo(unsigned char key_temp);
extern unsigned short Motor_SyncTarget_HallArr[7];

extern MOTOR_BASE_PARA *Motor_Back_ParaStu  ;
extern MOTOR_BASE_PARA *Motor_Leg_ParaStu   ;
extern MOTOR_BASE_PARA *Motor_Lumbar_ParaStu;
extern MOTOR_BASE_PARA *Motor_Neck_ParaStu  ;
extern MOTOR_BASE_PARA *Motor_Lumbar2_ParaStu;
extern MOTOR_BASE_PARA *Motor_Neck2_ParaStu  ;
void Motor_Control(void);
void Motor_OneClickCmd_Set(unsigned char cmd_temp);
void Motor_Para_Reset(MOTOR_BASE_PARA *Motor_ParaStu);
void Motor_Para_AllReset(void);
void Command_Back_Run(unsigned char key_func);
void Command_Leg_Run(unsigned char key_func);
void Command_BackLeg_Run(unsigned char key_func);
void Command_Neck_Run(unsigned char key_func);
void Command_Lumbar_Run(unsigned char key_func);
void Command_Lumbar2_Run(unsigned char key_func);
void Command_Neck2_Run(unsigned char key_func);

void Motor_AllStop(void);

void Command_PowerOn_Reset(void);

unsigned char Command_GoFlat_Shape(void);
unsigned char Command_GoM1_Shape(void);
unsigned char Command_GoM2_Shape(void);
unsigned char Command_GoM3_Shape(void);
unsigned char Command_GoTV_Shape(void);
unsigned char Command_GoZeroG_Shape(void);
unsigned char Command_GoLounge_Shape(void);
unsigned char Command_GoSnore_Shape(void);
unsigned char Command_GoRead_Shape(void);
unsigned char Command_GoYoga_Shape(void);
unsigned char Command_GoGetUp_Shape(void);
unsigned char Command_GoNursing_Shape(void);

unsigned char Command_Save_MotorHall(void);
unsigned char Command_SaveM1_Shape(void);
unsigned char Command_SaveM2_Shape(void);
unsigned char Command_SaveM3_Shape(void);
unsigned char Command_SaveTV_Shape(void);
unsigned char Command_SaveZeroG_Shape(void);
unsigned char Command_SaveLounge_Shape(void);
unsigned char Command_SaveSnore_Shape(void);
unsigned char Command_SaveRead_Shape(void);
unsigned char Command_SaveYoga_Shape(void);
unsigned char Command_SaveGetUp_Shape(void);
unsigned char Command_SaveNursing_Shape(void);
unsigned char Command_FactoryReset_Shape(void);

unsigned char Command_Save_MotorHall(void);

void Motor_TimerManagerTask(void);
unsigned char Motor_Run_Cmd(void);
unsigned char Motor_Sync_Complate(void);

void Motor_Sync_EnableSet(unsigned char enable_flag);

unsigned char Motor_Continue_KeyInfo(unsigned char key_temp);
unsigned char Get_Motor_Run_CmdState(void);
unsigned char GetSet_Motor_Ctr_Cmd(unsigned char ctr_state);
void Motor_DemoMode_ClearPara(void);
void Motor_Demo1Mode_ClearPara(void);
void Motor_Demo2Mode_ClearPara(void);
void Motor_Demo3Mode_ClearPara(void);
void Motor_ParaStu_Init(void);
void Set_Motor_Demo_Step(unsigned char step);
void Motor_Demo_ClearTime(void);
#endif



















