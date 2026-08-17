#include "app_save.h"
#include "delay.h"
#include "app_motor.h"
#include "modul_motor.h"

#include "app_rtc.h"

#include "driver_uart.h"

#include "app_config.h"

unsigned char USER_SYS_STATE_DATA_BUF[SYS_STATE_INFO_LEN] = {0};
unsigned char USER_MOTOR_DATA_BUF[MOTOR_INFO_LEN] = {0};
unsigned char USER_A7105_DATA_BUF[A7105_ID_LEN] = {0};
unsigned char USER_ALARM_DATA_BUF[ALARM_INFO_LEN] = {0};
/*-------------------------------------------马达相关信息----------------------------------------------------*/
void User_Read_MotorInfoEeprom(void)
{
	Memory_Read(EEPROM_MOTOR_PAGE_BASE,USER_MOTOR_DATA_BUF,MOTOR_INFO_LEN);
}
unsigned char User_Write_MotorInfoEeprom(void)
{
	unsigned char temp_i = 0;
	Memory_Write(EEPROM_MOTOR_PAGE_BASE,USER_MOTOR_DATA_BUF,MOTOR_INFO_LEN);
	Delay_Ms(10);
//	for(temp_i = 0;temp_i < MOTOR_INFO_LEN;temp_i++)
//	{
//		if(USER_MOTOR_DATA_BUF[temp_i] != Memory_ReadByte(EEPROM_MOTOR_PAGE_BASE + temp_i))
//		{
//			return 0;
//		}
//	}
	return 1;
}
void User_Erase_MotorInfoEeprom(void)
{
	Sector_Erase(EEPROM_MOTOR_PAGE_BASE);
	Delay_Ms(10);
}

void User_SaveMotor_Info(SAVE_MOTOR_INFO_TYPE save_info_type,unsigned char motor_port,unsigned short motor_hall_save)
{
	unsigned short motor_info_add_start = 0;
	
	if(motor_port == MOTOR_NO) return;
	
	motor_info_add_start = save_info_type * 12;
	
	USER_MOTOR_DATA_BUF[motor_info_add_start + (motor_port * 2) - 2] = (unsigned char)(motor_hall_save >> 8);
	USER_MOTOR_DATA_BUF[motor_info_add_start + (motor_port * 2) - 1] = (unsigned char)(motor_hall_save);	
}

unsigned short User_ReadMotor_Info(SAVE_MOTOR_INFO_TYPE save_info_type,unsigned char motor_port)
{
	unsigned short motor_hall_temp = 0, motor_hall_start_add = 0;
	
	if(motor_port == MOTOR_NO) return 0;
	
	motor_hall_start_add = save_info_type * 12;
	
	motor_hall_temp = USER_MOTOR_DATA_BUF[motor_hall_start_add + (motor_port * 2) - 2];
	motor_hall_temp = (motor_hall_temp << 8) | USER_MOTOR_DATA_BUF[motor_hall_start_add + (motor_port * 2) - 1];
	
	return motor_hall_temp;
}
#include "driver_beep.h"
unsigned char User_ReadMotorHall_Info(void)
{
	//读取原有数据
	User_Read_MotorInfoEeprom(); 	
	/*----------------------------读取马达实时霍尔数---------------------------------------*/
	//电机1霍尔数35/3
	Motor1_ParaStu.hall_run_num = User_ReadMotor_Info(MOTOR_HALL_INFO,MOTOR1_PORT);

	if(Motor1_ParaStu.hall_run_num == 0xFFFF)  Motor1_ParaStu.hall_run_num = HALL_MIN_NUM;
	Motor1_ParaStu.old_hall_run_num = Motor1_ParaStu.hall_run_num;
	
	//电机2霍尔数30/3
	Motor2_ParaStu.hall_run_num = User_ReadMotor_Info(MOTOR_HALL_INFO,MOTOR2_PORT);
	
	if(Motor2_ParaStu.hall_run_num == 0xFFFF)  Motor2_ParaStu.hall_run_num = HALL_MIN_NUM;	
	Motor2_ParaStu.old_hall_run_num = Motor2_ParaStu.hall_run_num;
	//电机3霍尔数
	Motor3_ParaStu.hall_run_num = User_ReadMotor_Info(MOTOR_HALL_INFO,MOTOR3_PORT);
	
	if(Motor3_ParaStu.hall_run_num == 0xFFFF)  Motor3_ParaStu.hall_run_num = HALL_MIN_NUM;	
	Motor3_ParaStu.old_hall_run_num = Motor3_ParaStu.hall_run_num;	
	//电机4霍尔数
	Motor4_ParaStu.hall_run_num = User_ReadMotor_Info(MOTOR_HALL_INFO,MOTOR4_PORT);
	
	if(Motor4_ParaStu.hall_run_num == 0xFFFF)  Motor4_ParaStu.hall_run_num = HALL_MIN_NUM;		
	Motor4_ParaStu.old_hall_run_num = Motor4_ParaStu.hall_run_num;
	//电机5霍尔数
	Motor5_ParaStu.hall_run_num = User_ReadMotor_Info(MOTOR_HALL_INFO,MOTOR5_PORT);
	
	if(Motor5_ParaStu.hall_run_num == 0xFFFF)  Motor5_ParaStu.hall_run_num = HALL_MIN_NUM;		
	Motor5_ParaStu.old_hall_run_num = Motor5_ParaStu.hall_run_num;
	//电机6霍尔数
	Motor6_ParaStu.hall_run_num = User_ReadMotor_Info(MOTOR_HALL_INFO,MOTOR6_PORT);
	
	if(Motor6_ParaStu.hall_run_num == 0xFFFF)  Motor6_ParaStu.hall_run_num = HALL_MIN_NUM;		
	Motor6_ParaStu.old_hall_run_num = Motor6_ParaStu.hall_run_num;


	User_SaveMotor_Info(MOTOR_HALL_INFO,MOTOR1_PORT,Motor1_ParaStu.hall_run_num);
	User_SaveMotor_Info(MOTOR_HALL_INFO,MOTOR2_PORT,Motor2_ParaStu.hall_run_num);
	User_SaveMotor_Info(MOTOR_HALL_INFO,MOTOR3_PORT,Motor3_ParaStu.hall_run_num);
	User_SaveMotor_Info(MOTOR_HALL_INFO,MOTOR4_PORT,Motor4_ParaStu.hall_run_num);
	User_SaveMotor_Info(MOTOR_HALL_INFO,MOTOR5_PORT,Motor5_ParaStu.hall_run_num);
	User_SaveMotor_Info(MOTOR_HALL_INFO,MOTOR6_PORT,Motor6_ParaStu.hall_run_num);
	
	//记忆位置
	if(1 == system_config.flags.memory_position_saved)
	{
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
	}
	else
	{
		//M1
		if(User_ReadMotor_Info(MOTOR_M1_INFO,Motor_Back_ParaStu->motor_port) == 0xffff)
		{
			User_SaveMotor_Info(MOTOR_M1_INFO,Motor_Back_ParaStu->motor_port,system_config.flags.mem1_position_hall_back);
		}
		if(User_ReadMotor_Info(MOTOR_M1_INFO,Motor_Leg_ParaStu->motor_port) == 0xffff)
		{
			User_SaveMotor_Info(MOTOR_M1_INFO,Motor_Leg_ParaStu->motor_port,system_config.flags.mem1_position_hall_leg);
		}
		if(User_ReadMotor_Info(MOTOR_M1_INFO,Motor_Lumbar_ParaStu->motor_port) == 0xffff)
		{
			User_SaveMotor_Info(MOTOR_M1_INFO,Motor_Lumbar_ParaStu->motor_port,system_config.flags.mem1_position_hall_lumbar);
		}
		if(User_ReadMotor_Info(MOTOR_M1_INFO,Motor_Neck_ParaStu->motor_port) == 0xffff)
		{
			User_SaveMotor_Info(MOTOR_M1_INFO,Motor_Neck_ParaStu->motor_port,system_config.flags.mem1_position_hall_neck);
		}
		if(User_ReadMotor_Info(MOTOR_M1_INFO,Motor_Lumbar2_ParaStu->motor_port) == 0xffff)
		{
			User_SaveMotor_Info(MOTOR_M1_INFO,Motor_Lumbar2_ParaStu->motor_port,system_config.flags.mem1_position_hall_lumbar2);
		}	
		if(User_ReadMotor_Info(MOTOR_M1_INFO,Motor_Neck2_ParaStu->motor_port) == 0xffff)
		{
			User_SaveMotor_Info(MOTOR_M1_INFO,Motor_Neck2_ParaStu->motor_port,system_config.flags.mem1_position_hall_neck2);
		}
		//M2
		if(User_ReadMotor_Info(MOTOR_M2_INFO,Motor_Back_ParaStu->motor_port) == 0xffff)
		{
			User_SaveMotor_Info(MOTOR_M2_INFO,Motor_Back_ParaStu->motor_port,system_config.flags.mem2_position_hall_back);
		}
		if(User_ReadMotor_Info(MOTOR_M2_INFO,Motor_Leg_ParaStu->motor_port) == 0xffff)
		{
			User_SaveMotor_Info(MOTOR_M2_INFO,Motor_Leg_ParaStu->motor_port,system_config.flags.mem2_position_hall_leg);
		}
		if(User_ReadMotor_Info(MOTOR_M2_INFO,Motor_Lumbar_ParaStu->motor_port) == 0xffff)
		{
			User_SaveMotor_Info(MOTOR_M2_INFO,Motor_Lumbar_ParaStu->motor_port,system_config.flags.mem2_position_hall_lumbar);
		}
		if(User_ReadMotor_Info(MOTOR_M2_INFO,Motor_Neck_ParaStu->motor_port) == 0xffff)
		{
			User_SaveMotor_Info(MOTOR_M2_INFO,Motor_Neck_ParaStu->motor_port,system_config.flags.mem2_position_hall_neck);
		}
		if(User_ReadMotor_Info(MOTOR_M2_INFO,Motor_Lumbar2_ParaStu->motor_port) == 0xffff)
		{
			User_SaveMotor_Info(MOTOR_M2_INFO,Motor_Lumbar2_ParaStu->motor_port,system_config.flags.mem2_position_hall_lumbar2);
		}
		if(User_ReadMotor_Info(MOTOR_M2_INFO,Motor_Neck2_ParaStu->motor_port) == 0xffff)
		{
			User_SaveMotor_Info(MOTOR_M2_INFO,Motor_Neck2_ParaStu->motor_port,system_config.flags.mem2_position_hall_neck2);
		}
		//M3
		if(User_ReadMotor_Info(MOTOR_M3_INFO,Motor_Back_ParaStu->motor_port) == 0xffff)
		{
			User_SaveMotor_Info(MOTOR_M3_INFO,Motor_Back_ParaStu->motor_port,system_config.flags.mem3_position_hall_back);
		}
		if(User_ReadMotor_Info(MOTOR_M3_INFO,Motor_Leg_ParaStu->motor_port) == 0xffff)
		{
			User_SaveMotor_Info(MOTOR_M3_INFO,Motor_Leg_ParaStu->motor_port,system_config.flags.mem3_position_hall_leg);
		}
		if(User_ReadMotor_Info(MOTOR_M3_INFO,Motor_Lumbar_ParaStu->motor_port) == 0xffff)
		{
			User_SaveMotor_Info(MOTOR_M3_INFO,Motor_Lumbar_ParaStu->motor_port,system_config.flags.mem3_position_hall_lumbar);
		}
		if(User_ReadMotor_Info(MOTOR_M3_INFO,Motor_Neck_ParaStu->motor_port) == 0xffff)
		{
			User_SaveMotor_Info(MOTOR_M3_INFO,Motor_Neck_ParaStu->motor_port,system_config.flags.mem3_position_hall_neck);
		}
		if(User_ReadMotor_Info(MOTOR_M3_INFO,Motor_Lumbar2_ParaStu->motor_port) == 0xffff)
		{
			User_SaveMotor_Info(MOTOR_M3_INFO,Motor_Lumbar2_ParaStu->motor_port,system_config.flags.mem3_position_hall_lumbar2);
		}	
		if(User_ReadMotor_Info(MOTOR_M3_INFO,Motor_Neck2_ParaStu->motor_port) == 0xffff)
		{
			User_SaveMotor_Info(MOTOR_M3_INFO,Motor_Neck2_ParaStu->motor_port,system_config.flags.mem3_position_hall_neck2);
		}	
	}
		/*----------------------------设置马达固定位置初始霍尔数-------------------------------*/

		
	if(1 == system_config.flags.fixed_position_saved)
	{
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
		
		User_SaveMotor_Info(MOTOR_GETUP_INFO,Motor_Back_ParaStu->motor_port,	  system_config.flags.getup_hall_back_default);
		User_SaveMotor_Info(MOTOR_GETUP_INFO,Motor_Leg_ParaStu->motor_port,		system_config.flags.getup_hall_leg_default);
		User_SaveMotor_Info(MOTOR_GETUP_INFO,Motor_Lumbar_ParaStu->motor_port, system_config.flags.getup_hall_lumbar_default);
		User_SaveMotor_Info(MOTOR_GETUP_INFO,Motor_Neck_ParaStu->motor_port,	  system_config.flags.getup_hall_neck_default);	
		User_SaveMotor_Info(MOTOR_GETUP_INFO,Motor_Lumbar2_ParaStu->motor_port, system_config.flags.getup_hall_lumbar2_default);
		User_SaveMotor_Info(MOTOR_GETUP_INFO,Motor_Neck2_ParaStu->motor_port,	  system_config.flags.getup_hall_neck2_default);
		
		User_SaveMotor_Info(MOTOR_NURSING_INFO,Motor_Back_ParaStu->motor_port,	  system_config.flags.nursing_hall_back_default);
		User_SaveMotor_Info(MOTOR_NURSING_INFO,Motor_Leg_ParaStu->motor_port,		system_config.flags.nursing_hall_leg_default);
		User_SaveMotor_Info(MOTOR_NURSING_INFO,Motor_Lumbar_ParaStu->motor_port, system_config.flags.nursing_hall_lumbar_default);
		User_SaveMotor_Info(MOTOR_NURSING_INFO,Motor_Neck_ParaStu->motor_port,	  system_config.flags.nursing_hall_neck_default);	
		User_SaveMotor_Info(MOTOR_NURSING_INFO,Motor_Lumbar2_ParaStu->motor_port, system_config.flags.nursing_hall_lumbar2_default);
		User_SaveMotor_Info(MOTOR_NURSING_INFO,Motor_Neck2_ParaStu->motor_port,	  system_config.flags.nursing_hall_neck2_default);
	}
	else
	{
		//TV
		if(User_ReadMotor_Info(MOTOR_TV_INFO,Motor_Back_ParaStu->motor_port) == 0xffff)
		{
			User_SaveMotor_Info(MOTOR_TV_INFO,Motor_Back_ParaStu->motor_port,	system_config.flags.tv_hall_back_default);
		}
		if(User_ReadMotor_Info(MOTOR_TV_INFO,Motor_Leg_ParaStu->motor_port) == 0xffff)
		{
			User_SaveMotor_Info(MOTOR_TV_INFO,Motor_Leg_ParaStu->motor_port,	system_config.flags.tv_hall_leg_default);
		}
		if(User_ReadMotor_Info(MOTOR_TV_INFO,Motor_Lumbar_ParaStu->motor_port) == 0xffff)
		{
			User_SaveMotor_Info(MOTOR_TV_INFO,Motor_Lumbar_ParaStu->motor_port,	system_config.flags.tv_hall_lumbar_default);
		}
		if(User_ReadMotor_Info(MOTOR_TV_INFO,Motor_Neck_ParaStu->motor_port) == 0xffff)
		{
			User_SaveMotor_Info(MOTOR_TV_INFO,Motor_Neck_ParaStu->motor_port,	system_config.flags.tv_hall_neck_default);
		}		
		if(User_ReadMotor_Info(MOTOR_TV_INFO,Motor_Lumbar2_ParaStu->motor_port) == 0xffff)
		{
			User_SaveMotor_Info(MOTOR_TV_INFO,Motor_Lumbar2_ParaStu->motor_port,	system_config.flags.tv_hall_lumbar2_default);
		}
		if(User_ReadMotor_Info(MOTOR_TV_INFO,Motor_Neck2_ParaStu->motor_port) == 0xffff)
		{
			User_SaveMotor_Info(MOTOR_TV_INFO,Motor_Neck2_ParaStu->motor_port,	system_config.flags.tv_hall_neck2_default);
		}		
		//ZG
		if(User_ReadMotor_Info(MOTOR_ZEROG_INFO,Motor_Back_ParaStu->motor_port) == 0xffff)
		{
			User_SaveMotor_Info(MOTOR_ZEROG_INFO,Motor_Back_ParaStu->motor_port,	system_config.flags.zerog_hall_back_default);
		}
		if(User_ReadMotor_Info(MOTOR_ZEROG_INFO,Motor_Leg_ParaStu->motor_port) == 0xffff)
		{
			User_SaveMotor_Info(MOTOR_ZEROG_INFO,Motor_Leg_ParaStu->motor_port,	system_config.flags.zerog_hall_leg_default);
		}
		if(User_ReadMotor_Info(MOTOR_ZEROG_INFO,Motor_Lumbar_ParaStu->motor_port) == 0xffff)
		{
			User_SaveMotor_Info(MOTOR_ZEROG_INFO,Motor_Lumbar_ParaStu->motor_port,	system_config.flags.zerog_hall_lumbar_default);
		}
		if(User_ReadMotor_Info(MOTOR_ZEROG_INFO,Motor_Neck_ParaStu->motor_port) == 0xffff)
		{
			User_SaveMotor_Info(MOTOR_ZEROG_INFO,Motor_Neck_ParaStu->motor_port,	system_config.flags.zerog_hall_neck_default);
		}
		if(User_ReadMotor_Info(MOTOR_ZEROG_INFO,Motor_Lumbar2_ParaStu->motor_port) == 0xffff)
		{
			User_SaveMotor_Info(MOTOR_ZEROG_INFO,Motor_Lumbar2_ParaStu->motor_port,	system_config.flags.zerog_hall_lumbar2_default);
		}
		if(User_ReadMotor_Info(MOTOR_ZEROG_INFO,Motor_Neck2_ParaStu->motor_port) == 0xffff)
		{
			User_SaveMotor_Info(MOTOR_ZEROG_INFO,Motor_Neck2_ParaStu->motor_port,	system_config.flags.zerog_hall_neck2_default);
		}
		//LOUNGE
		if(User_ReadMotor_Info(MOTOR_LOUNGE_INFO,Motor_Back_ParaStu->motor_port) == 0xffff)
		{
			User_SaveMotor_Info(MOTOR_LOUNGE_INFO,Motor_Back_ParaStu->motor_port,	system_config.flags.lounge_hall_back_default);
		}
		if(User_ReadMotor_Info(MOTOR_LOUNGE_INFO,Motor_Leg_ParaStu->motor_port) == 0xffff)
		{
			User_SaveMotor_Info(MOTOR_LOUNGE_INFO,Motor_Leg_ParaStu->motor_port,	system_config.flags.lounge_hall_leg_default);
		}
		if(User_ReadMotor_Info(MOTOR_LOUNGE_INFO,Motor_Lumbar_ParaStu->motor_port) == 0xffff)
		{
			User_SaveMotor_Info(MOTOR_LOUNGE_INFO,Motor_Lumbar_ParaStu->motor_port,	system_config.flags.lounge_hall_lumbar_default);
		}
		if(User_ReadMotor_Info(MOTOR_LOUNGE_INFO,Motor_Neck_ParaStu->motor_port) == 0xffff)
		{
			User_SaveMotor_Info(MOTOR_LOUNGE_INFO,Motor_Neck_ParaStu->motor_port,	system_config.flags.lounge_hall_neck_default);
		}
		if(User_ReadMotor_Info(MOTOR_LOUNGE_INFO,Motor_Lumbar2_ParaStu->motor_port) == 0xffff)
		{
			User_SaveMotor_Info(MOTOR_LOUNGE_INFO,Motor_Lumbar2_ParaStu->motor_port,	system_config.flags.lounge_hall_lumbar2_default);
		}
		if(User_ReadMotor_Info(MOTOR_LOUNGE_INFO,Motor_Neck2_ParaStu->motor_port) == 0xffff)
		{
			User_SaveMotor_Info(MOTOR_LOUNGE_INFO,Motor_Neck2_ParaStu->motor_port,	system_config.flags.lounge_hall_neck2_default);
		}
		//SNORE
		if(User_ReadMotor_Info(MOTOR_SNORE_INFO,Motor_Back_ParaStu->motor_port) == 0xffff)
		{
			User_SaveMotor_Info(MOTOR_SNORE_INFO,Motor_Back_ParaStu->motor_port,	system_config.flags.snore_hall_back_default);
		}
		if(User_ReadMotor_Info(MOTOR_SNORE_INFO,Motor_Leg_ParaStu->motor_port) == 0xffff)
		{
			User_SaveMotor_Info(MOTOR_SNORE_INFO,Motor_Leg_ParaStu->motor_port,	system_config.flags.snore_hall_leg_default);
		}
		if(User_ReadMotor_Info(MOTOR_SNORE_INFO,Motor_Lumbar_ParaStu->motor_port) == 0xffff)
		{
			User_SaveMotor_Info(MOTOR_SNORE_INFO,Motor_Lumbar_ParaStu->motor_port,	system_config.flags.snore_hall_lumbar_default);
		}
		if(User_ReadMotor_Info(MOTOR_SNORE_INFO,Motor_Neck_ParaStu->motor_port) == 0xffff)
		{
			User_SaveMotor_Info(MOTOR_SNORE_INFO,Motor_Neck_ParaStu->motor_port,	system_config.flags.snore_hall_neck_default);
		}
		if(User_ReadMotor_Info(MOTOR_SNORE_INFO,Motor_Lumbar2_ParaStu->motor_port) == 0xffff)
		{
			User_SaveMotor_Info(MOTOR_SNORE_INFO,Motor_Lumbar2_ParaStu->motor_port,	system_config.flags.snore_hall_lumbar2_default);
		}
		if(User_ReadMotor_Info(MOTOR_SNORE_INFO,Motor_Neck2_ParaStu->motor_port) == 0xffff)
		{
			User_SaveMotor_Info(MOTOR_SNORE_INFO,Motor_Neck2_ParaStu->motor_port,	system_config.flags.snore_hall_neck2_default);
		}
		//READ		
		if(User_ReadMotor_Info(MOTOR_READ_INFO,Motor_Back_ParaStu->motor_port) == 0xffff)
		{
			User_SaveMotor_Info(MOTOR_READ_INFO,Motor_Back_ParaStu->motor_port,	system_config.flags.read_hall_back_default);
		}
		if(User_ReadMotor_Info(MOTOR_READ_INFO,Motor_Leg_ParaStu->motor_port) == 0xffff)
		{
			User_SaveMotor_Info(MOTOR_READ_INFO,Motor_Leg_ParaStu->motor_port,	system_config.flags.read_hall_leg_default);
		}
		if(User_ReadMotor_Info(MOTOR_READ_INFO,Motor_Lumbar_ParaStu->motor_port) == 0xffff)
		{
			User_SaveMotor_Info(MOTOR_READ_INFO,Motor_Lumbar_ParaStu->motor_port,	system_config.flags.read_hall_lumbar_default);
		}
		if(User_ReadMotor_Info(MOTOR_READ_INFO,Motor_Neck_ParaStu->motor_port) == 0xffff)
		{
			User_SaveMotor_Info(MOTOR_READ_INFO,Motor_Neck_ParaStu->motor_port,	system_config.flags.read_hall_neck_default);
		}
		if(User_ReadMotor_Info(MOTOR_READ_INFO,Motor_Lumbar2_ParaStu->motor_port) == 0xffff)
		{
			User_SaveMotor_Info(MOTOR_READ_INFO,Motor_Lumbar2_ParaStu->motor_port,	system_config.flags.read_hall_lumbar2_default);
		}
		if(User_ReadMotor_Info(MOTOR_READ_INFO,Motor_Neck2_ParaStu->motor_port) == 0xffff)
		{
			User_SaveMotor_Info(MOTOR_READ_INFO,Motor_Neck2_ParaStu->motor_port,	system_config.flags.read_hall_neck2_default);
		}
		//YOGA		
		if(User_ReadMotor_Info(MOTOR_YOGA_INFO,Motor_Back_ParaStu->motor_port) == 0xffff)
		{
			User_SaveMotor_Info(MOTOR_YOGA_INFO,Motor_Back_ParaStu->motor_port,	system_config.flags.yoga_hall_back_default);
		}
		if(User_ReadMotor_Info(MOTOR_YOGA_INFO,Motor_Leg_ParaStu->motor_port) == 0xffff)
		{
			User_SaveMotor_Info(MOTOR_YOGA_INFO,Motor_Leg_ParaStu->motor_port,	system_config.flags.yoga_hall_leg_default);
		}
		if(User_ReadMotor_Info(MOTOR_YOGA_INFO,Motor_Lumbar_ParaStu->motor_port) == 0xffff)
		{
			User_SaveMotor_Info(MOTOR_YOGA_INFO,Motor_Lumbar_ParaStu->motor_port,	system_config.flags.yoga_hall_lumbar_default);
		}
		if(User_ReadMotor_Info(MOTOR_YOGA_INFO,Motor_Neck_ParaStu->motor_port) == 0xffff)
		{
			User_SaveMotor_Info(MOTOR_YOGA_INFO,Motor_Neck_ParaStu->motor_port,	system_config.flags.yoga_hall_neck_default);
		}
		if(User_ReadMotor_Info(MOTOR_YOGA_INFO,Motor_Lumbar2_ParaStu->motor_port) == 0xffff)
		{
			User_SaveMotor_Info(MOTOR_YOGA_INFO,Motor_Lumbar2_ParaStu->motor_port,	system_config.flags.yoga_hall_lumbar2_default);
		}
		if(User_ReadMotor_Info(MOTOR_YOGA_INFO,Motor_Neck2_ParaStu->motor_port) == 0xffff)
		{
			User_SaveMotor_Info(MOTOR_YOGA_INFO,Motor_Neck2_ParaStu->motor_port,	system_config.flags.yoga_hall_neck2_default);
		}
		//GETUP
		if(User_ReadMotor_Info(MOTOR_GETUP_INFO,Motor_Back_ParaStu->motor_port) == 0xffff)
		{                            
			 User_SaveMotor_Info(MOTOR_GETUP_INFO,Motor_Back_ParaStu->motor_port,	system_config.flags.getup_hall_back_default);
		}                            
		if(User_ReadMotor_Info(MOTOR_GETUP_INFO,Motor_Leg_ParaStu->motor_port) == 0xffff)
		{                            
			 User_SaveMotor_Info(MOTOR_GETUP_INFO,Motor_Leg_ParaStu->motor_port,	system_config.flags.getup_hall_leg_default);
		}                            
		if(User_ReadMotor_Info(MOTOR_GETUP_INFO,Motor_Lumbar_ParaStu->motor_port) == 0xffff)
		{                            
			 User_SaveMotor_Info(MOTOR_GETUP_INFO,Motor_Lumbar_ParaStu->motor_port,	system_config.flags.getup_hall_lumbar_default);
		}                            
		if(User_ReadMotor_Info(MOTOR_GETUP_INFO,Motor_Neck_ParaStu->motor_port) == 0xffff)
		{                            
			 User_SaveMotor_Info(MOTOR_GETUP_INFO,Motor_Neck_ParaStu->motor_port,	system_config.flags.getup_hall_neck_default);
		}                           
		if(User_ReadMotor_Info(MOTOR_GETUP_INFO,Motor_Lumbar2_ParaStu->motor_port) == 0xffff)
		{                            
			 User_SaveMotor_Info(MOTOR_GETUP_INFO,Motor_Lumbar2_ParaStu->motor_port,	system_config.flags.getup_hall_lumbar2_default);
		}      	
		if(User_ReadMotor_Info(MOTOR_GETUP_INFO,Motor_Neck2_ParaStu->motor_port) == 0xffff)
		{                            
			 User_SaveMotor_Info(MOTOR_GETUP_INFO,Motor_Neck2_ParaStu->motor_port,	system_config.flags.getup_hall_neck2_default);
		}   	
		//NURSING
		if(User_ReadMotor_Info(MOTOR_NURSING_INFO,Motor_Back_ParaStu->motor_port) == 0xffff)
		{                            
			 User_SaveMotor_Info(MOTOR_NURSING_INFO,Motor_Back_ParaStu->motor_port,	system_config.flags.nursing_hall_back_default);
		}                            
		if(User_ReadMotor_Info(MOTOR_NURSING_INFO,Motor_Leg_ParaStu->motor_port) == 0xffff)
		{                            
			 User_SaveMotor_Info(MOTOR_NURSING_INFO,Motor_Leg_ParaStu->motor_port,	system_config.flags.nursing_hall_leg_default);
		}                            
		if(User_ReadMotor_Info(MOTOR_NURSING_INFO,Motor_Lumbar_ParaStu->motor_port) == 0xffff)
		{                            
			 User_SaveMotor_Info(MOTOR_NURSING_INFO,Motor_Lumbar_ParaStu->motor_port,	system_config.flags.nursing_hall_lumbar_default);
		}                            
		if(User_ReadMotor_Info(MOTOR_NURSING_INFO,Motor_Neck_ParaStu->motor_port) == 0xffff)
		{                            
			 User_SaveMotor_Info(MOTOR_NURSING_INFO,Motor_Neck_ParaStu->motor_port,	system_config.flags.nursing_hall_neck_default);
		}                            
		if(User_ReadMotor_Info(MOTOR_NURSING_INFO,Motor_Lumbar2_ParaStu->motor_port) == 0xffff)
		{                            
			 User_SaveMotor_Info(MOTOR_NURSING_INFO,Motor_Lumbar2_ParaStu->motor_port,	system_config.flags.nursing_hall_lumbar2_default);
		}      	
		if(User_ReadMotor_Info(MOTOR_NURSING_INFO,Motor_Neck2_ParaStu->motor_port) == 0xffff)
		{                            
			 User_SaveMotor_Info(MOTOR_NURSING_INFO,Motor_Neck2_ParaStu->motor_port,	system_config.flags.nursing_hall_neck2_default);
		}   	
	}
	//擦出扇区
	User_Erase_MotorInfoEeprom();
	//
	return User_Write_MotorInfoEeprom();	
}

/*--------------------------------------A7105相关-----------------------------------------------*/
void User_Read_A7105InfoEeprom(void)
{
	Memory_Read(EEPROM_A7105_PAGE_BASE,USER_A7105_DATA_BUF,A7105_ID_LEN);
}
unsigned char User_Write_A7105InfoEeprom(void)
{
	unsigned char temp_i = 0;
	
	Memory_Write(EEPROM_A7105_PAGE_BASE,USER_A7105_DATA_BUF,A7105_ID_LEN);
	
//	for(temp_i = 0;temp_i < A7105_ID_LEN;temp_i++)
//	{
//		if(USER_A7105_DATA_BUF[A7105_ID_START_BIT + temp_i] != Memory_ReadByte(EEPROM_A7105_PAGE_BASE + temp_i))
//		{
//			return 0;
//		}
//	}

	return 1;	
}
void User_Erase_A7105InfoEeprom(void)
{
	Sector_Erase(EEPROM_A7105_PAGE_BASE);
	Delay_Ms(10);
}

unsigned char User_SaveA7105ID_Info(unsigned char *a7105_id_temp)
{
	//读取原有数据
	User_Read_A7105InfoEeprom();
	//更新BUFF
	USER_A7105_DATA_BUF[A7105_ID_START_BIT + 0] = a7105_id_temp[0]; //
	USER_A7105_DATA_BUF[A7105_ID_START_BIT + 1] = a7105_id_temp[1]; //
	USER_A7105_DATA_BUF[A7105_ID_START_BIT + 2] = a7105_id_temp[2]; //
	USER_A7105_DATA_BUF[A7105_ID_START_BIT + 3] = a7105_id_temp[3]; //
	//擦出扇区
	User_Erase_A7105InfoEeprom(); 
	//写EEPROM
	return User_Write_A7105InfoEeprom();
}

void User_ReadA7105ID_Info(unsigned char *a7105_id_temp)
{
	Memory_Read(EEPROM_A7105_PAGE_BASE,USER_A7105_DATA_BUF,A7105_ID_LEN);
	
	a7105_id_temp[0] = USER_A7105_DATA_BUF[A7105_ID_START_BIT + 0];
	a7105_id_temp[1] = USER_A7105_DATA_BUF[A7105_ID_START_BIT + 1];
	a7105_id_temp[2] = USER_A7105_DATA_BUF[A7105_ID_START_BIT + 2];
	a7105_id_temp[3] = USER_A7105_DATA_BUF[A7105_ID_START_BIT + 3];
}
/*-------------------------------------闹钟-----------------------------------------*/
void User_Read_AlarmInfoEeprom(void)
{
	unsigned char temp_i = 0;
	Memory_Read(EEPROM_ALARM_PAGE_BASE,USER_ALARM_DATA_BUF,ALARM_INFO_LEN);
}

void User_Erase_AlarmInfoEeprom(void)
{
	Sector_Erase(EEPROM_ALARM_PAGE_BASE);
	Delay_Ms(10);
}

unsigned char User_Write_AlarmInfoEeprom(void)
{
	unsigned char temp_i = 0;

	User_Erase_AlarmInfoEeprom();
	
	Memory_Write(EEPROM_ALARM_PAGE_BASE,USER_ALARM_DATA_BUF,ALARM_INFO_LEN);
	Delay_Ms(10);
//	for(temp_i = 0;temp_i < ALARM_INFO_LEN;temp_i++)
//	{
//		if(USER_ALARM_DATA_BUF[temp_i] != Memory_ReadByte(EEPROM_ALARM_PAGE_BASE + temp_i))
//		{
//			return 0;
//		}
//	}	
	
	return 1;
}
/*-------------------------------------状态-----------------------------------------*/
void User_Read_SysStateInfoEeprom(void)
{
	Memory_Read(EEPROM_SYS_STATE_PAGE_BASE,USER_SYS_STATE_DATA_BUF,SYS_STATE_INFO_LEN);
	
	if((USER_SYS_STATE_DATA_BUF[LOCK_START_BIT + 0] & 0x80) == 0x80)
	{
		USER_SYS_STATE_DATA_BUF[LOCK_START_BIT + 0] = 0x00;
		USER_SYS_STATE_DATA_BUF[LOCK_START_BIT + 1] = 0x00;
	}
	if(USER_SYS_STATE_DATA_BUF[MOTOR_RUN_START_BIT] == 0xff)
	{
		USER_SYS_STATE_DATA_BUF[MOTOR_RUN_START_BIT] = 1;
	}	
	if(USER_SYS_STATE_DATA_BUF[MOTOR_RUN_START_BIT] == 1)
	{
		USER_SYS_STATE_DATA_BUF[MOTOR_RUN_POWER_OFF_START_BIT] = 1;
	}
	sys_lock_state = ((unsigned short)USER_SYS_STATE_DATA_BUF[LOCK_START_BIT + 0] << 8) | USER_SYS_STATE_DATA_BUF[LOCK_START_BIT + 1];	
	sys_lock_state |=  SYS_SNORE_CHECK_STATE;
	if(system_config.flags.lock_saved == POWER_OFF_CLEAR_LOCK)
	{//断电清除童锁
		sys_lock_state &= ~SYS_CHILD_LOCK_STATE;
	}
	old_sys_lock_state = sys_lock_state;	
}

void User_SaveLockState_Info(unsigned short lock_state_temp)
{
	USER_SYS_STATE_DATA_BUF[LOCK_START_BIT + 0] = (unsigned char)(lock_state_temp >> 8);
	USER_SYS_STATE_DATA_BUF[LOCK_START_BIT + 1] = (unsigned char)(lock_state_temp);	
}
void User_SaveMotorRunState(unsigned char motor_run_flag)
{
	USER_SYS_STATE_DATA_BUF[MOTOR_RUN_START_BIT] = motor_run_flag;
}
unsigned char User_Read_MotorRunState(void)
{
	return USER_SYS_STATE_DATA_BUF[MOTOR_RUN_START_BIT];
}

unsigned char User_Write_SysStateInfoEeprom(void)
{
	unsigned char temp_i = 0;
	
	USER_SYS_STATE_DATA_BUF[LOCK_START_BIT + 0] &= 0x7F;
	
	Memory_Write(EEPROM_SYS_STATE_PAGE_BASE,USER_SYS_STATE_DATA_BUF,SYS_STATE_INFO_LEN);
	
//	for(temp_i = 0;temp_i < SYS_STATE_INFO_LEN;temp_i++)
//	{
//		if(USER_SYS_STATE_DATA_BUF[LOCK_START_BIT + temp_i] != Memory_ReadByte(EEPROM_SYS_STATE_PAGE_BASE + temp_i))
//		{
//			return 0;
//		}
//	}

	return 1;	
}
void User_Erase_SysStateInfoEeprom(void)
{
	Sector_Erase(EEPROM_SYS_STATE_PAGE_BASE);
	Delay_Ms(10);
}
/*-------------------------------------系统配置信息-----------------------------------------*/
void User_Read_SysConfigEeprom(void)
{
	Memory_Read(EEPROM_SYS_CONFIG_PAGE_BASE,system_config.config_buff,SYS_CONFIG_LENGTH);
}
unsigned char User_Write_SysConfigEeprom(void)
{
	unsigned char temp_i = 0;
	
	Memory_Write(EEPROM_SYS_CONFIG_PAGE_BASE,system_config.config_buff,SYS_CONFIG_LENGTH);

	
//	for(temp_i = 0;temp_i < SYS_CONFIG_LENGTH;temp_i++)
//	{
////		if(system_config.config_buff[temp_i] != Memory_ReadByte(EEPROM_SYS_CONFIG_PAGE_BASE + temp_i))
////		{
////			return 0;
////		}
//	}
	return 1;	
}
void User_Erase_SysConfigEeprom(void)
{
	Sector_Erase(EEPROM_SYS_CONFIG_PAGE_BASE);
	Delay_Ms(10);
}





