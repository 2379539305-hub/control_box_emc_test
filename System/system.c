#include "system.h"
#include "app_save.h"

#include "app_motor.h"

#include "modul_ttlbus.h"

#include "driver_key.h"
#include "driver_beep.h"
#include "driver_motor.h"

//系统控制标志位，按位bit0是儿童锁，bit1是音乐锁，bit2是打鼾锁，默认开启打鼾
unsigned short sys_lock_state = 0x02,old_sys_lock_state = 0;
//系统重启/重置标志位
unsigned char sys_re_flag = 0;
//单片机ID
unsigned short local_mcu_id = 0;
//crc16校验
unsigned short Sys_Crc16Check(u8 *addr,u8 num);
//读取单片机ID,返回值位CRC16后的值，范围0-65535
unsigned short readSTCUniqueID(void);
//上电时间，用于判断持续上电多久
unsigned long power_on_time = 0;

//系统初始化，用于读取MCUid和判断上电是否按下按键重置eeprom
void system_Init(void)
{
	local_mcu_id = readSTCUniqueID();
	while(!Get_StudyKey_State())
	{
		if(power_on_time >= 600)
		{
			Beep_SingSetPara(200,3);
			User_Erase_MotorInfoEeprom();
			while(!Get_StudyKey_State());
		}
	}
}
int user_strncmp(const char * str1,const char * str2,size_t n)
{
	if((str1 == NULL) || (str2 == NULL) || (n == 0))
	{
		return 1;
	}

	while((*str1 == *str2) && (*str1 != '\0') && (*str2 != '\0') && (n--))
	{
		str1++;
		str2++;
	}
	if( (*str1 == *str2) || (n == 0))
	{
		return 0;
	}
	else if( *str1 > *str2)
	{
		return 1;
	}
	else
	{
		return -1;
	}
}
//crc16校验
unsigned short Sys_Crc16Check(u8 *addr,u8 num) 
{ 
	unsigned short i,j,temp; 
	unsigned short crc=0xFFFF;    
	for(i=0;i<num;i++) 
	{ 
		crc=crc^(*addr); 
		for(j=0;j<8;j++) 
		{ 
			 temp=crc&0x0001; 
			 crc=crc>>1; 
			 if(temp) 
			 { 
					crc=crc^0xA001; 
			 } 
		} 
		addr++; 
	} 
	return crc;
}
//读取单片机ID，返回16位数据
unsigned short readSTCUniqueID(void) 
{
	  unsigned char uniqueId[12];
	
	  unsigned short value ;
	
		uint32_t *UID_ADDR = (uint32_t *)0x1FFFFA10;

		uint32_t uid_part1 = UID_ADDR[0];

		uint32_t uid_part2 = UID_ADDR[1];

		uint32_t uid_part3 = UID_ADDR[2];
		
		uniqueId[0] = (uid_part1>>24)&0xff;
		uniqueId[1] = (uid_part1>>16)&0xff;
		uniqueId[2] = (uid_part1>>8)&0xff;
		uniqueId[3] = (uid_part1)&0xff;
		uniqueId[4] = (uid_part2>>24)&0xff;
		uniqueId[5] = (uid_part2>>16)&0xff;
		uniqueId[6] = (uid_part2>>8)&0xff;
		uniqueId[7] = (uid_part2)&0xff;
		uniqueId[8] = (uid_part3>>24)&0xff;
		uniqueId[9] = (uid_part3>>16)&0xff;
		uniqueId[10] = (uid_part3>>8)&0xff;
		uniqueId[11] = (uid_part3)&0xff;	
		
	  value=Sys_Crc16Check(uniqueId,12);
	  return value;
}
//获取单片机id
unsigned short GetMcu_CrcID(void)
{
	return local_mcu_id;
}
unsigned char Decimal_To_Bcd(unsigned char val) 
{
	return ((val / 10) << 4) | (val % 10);
}
int Bcd_To_Decimal(unsigned char bcd_num) 
{
	return ((bcd_num >> 4) * 10) + (bcd_num & 0x0F);
}
//系统重启控制
void Sys_Control(void)
{
	if(1 == TTL_Check_Busy_Free())
	{
		if(sys_re_flag == SYS_RESET_FLAG) //重置
		{
			//清除位置
			User_Erase_MotorInfoEeprom();
			Command_Save_MotorHall();//保存霍尔
			//锁住控制盒需要重新复位才可以运行
			sys_lock_state |= SYS_WIFI_REST_STATE;
			User_SaveLockState_Info(sys_lock_state);

			User_Erase_SysStateInfoEeprom();
			User_Write_SysStateInfoEeprom();			
			//写入新配置
			User_Erase_SysConfigEeprom();
			User_Write_SysConfigEeprom();
			old_sys_lock_state = sys_lock_state;	
			//重启
			NVIC_SystemReset();
			sys_re_flag = 0;
		}
		if(sys_re_flag == SYS_REBOOT_STATE) //重启
		{
			NVIC_SystemReset();
			sys_re_flag = 0;
		}
		if(sys_lock_state != old_sys_lock_state)
		{
			User_SaveLockState_Info(sys_lock_state);
			User_Erase_SysStateInfoEeprom();
			User_Write_SysStateInfoEeprom();
			old_sys_lock_state = sys_lock_state;
		}		
	}
}
//系统时间中断处理函数，用于计时启动时间，最大值为1个月
void Sys_TimerManager(void)
{
	power_on_time ++;
	if(power_on_time >= POWER_ON_TIME_MAX)
	{
		power_on_time = POWER_ON_TIME_MAX;
	}
}

void Config_Gpio(GPIO_Type *gpiox, uint32_t pin)
{
    /* 置位 DRST 中对应引脚 */
    SET_BIT(gpiox->DRST, ((pin & 0xFFFFU) << 0x0U));

    /* 配置 FCR 为输出模式（保持你原来的宏用法） */
    MODIFY_REG(gpiox->FCR,
               ((pin * pin) * GPIO_FCR),
               ((pin * pin) * FL_GPIO_MODE_OUTPUT));
}
void MF_Gpio_Init(void)
{ 
  //若需要解决上电电机动一下的问题BM_BASE系列，都需要配置电机相关IO为输出低电平
    Config_Gpio(MOTOR1_UP_GPIO, MOTOR1_UP_GPIO_PIN);
    Config_Gpio(MOTOR1_DOWN_GPIO, MOTOR1_DOWN_GPIO_PIN);
    Config_Gpio(MOTOR2_UP_GPIO, MOTOR2_UP_GPIO_PIN);
    Config_Gpio(MOTOR2_DOWN_GPIO, MOTOR2_DOWN_GPIO_PIN);
    Config_Gpio(MOTOR3_UP_GPIO, MOTOR3_UP_GPIO_PIN);
    Config_Gpio(MOTOR3_DOWN_GPIO, MOTOR3_DOWN_GPIO_PIN);
    Config_Gpio(MOTOR4_UP_GPIO, MOTOR4_UP_GPIO_PIN);
    Config_Gpio(MOTOR4_DOWN_GPIO, MOTOR4_DOWN_GPIO_PIN);       
    Config_Gpio(MOTOR5_UP_GPIO, MOTOR5_UP_GPIO_PIN);  
		Config_Gpio(MOTOR5_DOWN_GPIO, MOTOR5_DOWN_GPIO_PIN);  
		Config_Gpio(MOTOR6_UP_GPIO, MOTOR6_UP_GPIO_PIN);
    Config_Gpio(MOTOR6_DOWN_GPIO, MOTOR6_DOWN_GPIO_PIN);
		Config_Gpio(MOTOR1_SPEED_GPIO, MOTOR1_SPEED_GPIO_PIN);
    Config_Gpio(MOTOR2_SPEED_GPIO, MOTOR2_SPEED_GPIO_PIN);
    Config_Gpio(MOTOR3_SPEED_GPIO, MOTOR3_SPEED_GPIO_PIN);
    Config_Gpio(MOTOR4_SPEED_GPIO, MOTOR4_SPEED_GPIO_PIN);
}





