#ifndef __APP_SAVE_H
#define __APP_SAVE_H

#include "main.h"
#include "system.h"

#include "driver_eeprom.h"
#include "modul_motor.h"

/*------------------------------------用户数据----------------------------------------*/
//EEPROM扇区个数
#define EEPROM_PAGE_NUM (256)
//EEPROM扇区大小
#define EEPROM_PAGE_SIZE (0x200) 
/*---------------数据存储位置----------------------*/
//用户存储EEPROM起始地址从493页开始-510页
#define EEPROM_SYS_CONFIG_PAGE_BASE (506 * EEPROM_PAGE_SIZE)
#define EEPROM_SYS_STATE_PAGE_BASE 	(507 * EEPROM_PAGE_SIZE)
#define EEPROM_ALARM_PAGE_BASE     	(508 * EEPROM_PAGE_SIZE)
#define EEPROM_A7105_PAGE_BASE     	(509 * EEPROM_PAGE_SIZE)
#define EEPROM_MOTOR_PAGE_BASE     	(510 * EEPROM_PAGE_SIZE)
//511页地址给OTA用
#define OTA_INFO_ADDR 							(511 * EEPROM_PAGE_SIZE)
#define SYS_STATE_INFO_LEN  (10)

#define LOCK_START_BIT  (0)
#define MOTOR_RUN_START_BIT (2)
#define MOTOR_RUN_POWER_OFF_START_BIT  (3)


#define ALARM_INFO_LEN (170)

#define A7105_ID_START_BIT  (0)
#define A7105_ID_LEN        (4)

#define MOTOR_INFO_LEN (200)

extern unsigned char USER_SYS_STATE_DATA_BUF[SYS_STATE_INFO_LEN]; 
extern unsigned char USER_A7105_DATA_BUF[A7105_ID_LEN]; 
extern unsigned char USER_MOTOR_DATA_BUF[MOTOR_INFO_LEN];
extern unsigned char USER_ALARM_DATA_BUF[ALARM_INFO_LEN]; 

typedef enum  
{
	MOTOR_HALL_INFO = 0,
	MOTOR_FLAT_INFO = 1,
	MOTOR_M1_INFO = 2,
	MOTOR_M2_INFO = 3,
	MOTOR_M3_INFO =4,
	MOTOR_TV_INFO = 5,
	MOTOR_ZEROG_INFO = 6,
	MOTOR_LOUNGE_INFO = 7,
	MOTOR_SNORE_INFO = 8,
	MOTOR_READ_INFO = 9,
	MOTOR_YOGA_INFO = 10,
	MOTOR_WAKEUP_INFO = 11,
	MOTOR_GETUP_INFO = 12,
	MOTOR_NURSING_INFO = 13,
	MOTOR_MAX_INFO
}SAVE_MOTOR_INFO_TYPE;

/*-----------------------------------------------------------------------------------------------------*/	

unsigned char User_SaveA7105ID_Info(unsigned char *a7105_id);
void User_ReadA7105ID_Info(unsigned char *a7105_id);

void User_Erase_MotorInfoEeprom(void);
void User_Read_MotorInfoEeprom(void);
unsigned char User_Write_MotorInfoEeprom(void);
void User_SaveMotor_Info(SAVE_MOTOR_INFO_TYPE save_info_type,unsigned char motor_port,unsigned short motor_hall_save);
unsigned short User_ReadMotor_Info(SAVE_MOTOR_INFO_TYPE save_info_type,unsigned char motor_port);
unsigned char User_ReadMotorHall_Info(void);

void User_Erase_AlarmInfoEeprom(void);
void User_Read_AlarmInfoEeprom(void);
unsigned char User_Write_AlarmInfoEeprom(void);

void User_SaveLockState_Info(unsigned short lock_state_temp);
void User_Erase_SysStateInfoEeprom(void);
void User_Read_SysStateInfoEeprom(void);
unsigned char User_Write_SysStateInfoEeprom(void);
unsigned char User_Read_MotorRunPowerOffState(void);
unsigned char User_Read_MotorRunState(void);
void User_SaveMotorRunState(unsigned char motor_run_flag);

void User_Read_SysConfigEeprom(void);
void User_Erase_SysConfigEeprom(void);
void User_Read_SysConfigEeprom(void);
unsigned char User_Write_SysConfigEeprom(void);
#endif






