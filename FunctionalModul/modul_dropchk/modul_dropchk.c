#include "modul_dropchk.h"
#include "modul_ttlbus.h"

#include "driver_beep.h"

unsigned char drop_set_event = 0;
unsigned char drop_check_step = 0;
unsigned char drop_time_ms = 0;
DROP_TTL_STRUCT Drop_Ttl_Stu = {0};

unsigned char MotorPort_Check(void)
{

	
	
	return 1;

}

unsigned char TTL_DeviceOnline_Reset(void)
{
	Device_InfoArr[Master_SearchIdleAdd(LIGHT_RGB_DEVICE_TYPE)][DEVICE_INFO_ARR_LINE] = 0;
	Device_InfoArr[Master_SearchIdleAdd(MUSIC_DEVICE_TYPE)][DEVICE_INFO_ARR_LINE] = 0;
	Device_InfoArr[Master_SearchIdleAdd(VITA_DEVICE_TYPE)][DEVICE_INFO_ARR_LINE] = 0;
	return 1;		
}

//运行所有异常检测
unsigned char DropCheck_Run(void)
{
	if(drop_set_event == 0)
	{
//		MotorPort_Check();
//		UBLPort_Check();
		TTL_DeviceOnline_Reset();
		drop_set_event = 1;
		drop_check_step = 1;
	}
	if(drop_set_event == 2)
	{
		drop_set_event = 0;
		return 1;
	}
	
	return 0;
}

//定时去获取异常 5ms
void TTL_Check_OnLine(void)
{	
	if((drop_check_step == 1) && (drop_time_ms == 0))
	{		
		TTL_User_AskDevice_Status(LIGHT_RGB_DEVICE_TYPE,0x21,0x00,0x00); 
		drop_time_ms = 0;
	}
	if((drop_check_step == 2) && (drop_time_ms == 20))
	{		
		TTL_User_AskDevice_Status(MUSIC_DEVICE_TYPE,0x22,0x00,0x00); 
		drop_time_ms = 0;
	}
	if((drop_check_step == 3) && (drop_time_ms == 20))
	{	
		TTL_User_AskDevice_Status(VITA_DEVICE_TYPE,0x21,0x01,0x00); 
		drop_time_ms = 0;
	}
	if((drop_check_step == 4) && (drop_time_ms == 20))
	{
		if(Device_InfoArr[Master_SearchIdleAdd(LIGHT_RGB_DEVICE_TYPE)][DEVICE_INFO_ARR_LINE] != 0) //检测RGB是否存在
		{
			Drop_Ttl_Stu.RGB_Light_Online = 1;
		}
		else
		{
			Drop_Ttl_Stu.RGB_Light_Online = 0;
		}
		
		if(Device_InfoArr[Master_SearchIdleAdd(VITA_DEVICE_TYPE)][DEVICE_INFO_ARR_LINE] != 0) //检测C65是否存在
		{
			Drop_Ttl_Stu.Sleep_Sensor_Online = 1;
		}
		else
		{
			Drop_Ttl_Stu.Sleep_Sensor_Online = 0;
		}
		
		if(Device_InfoArr[Master_SearchIdleAdd(MUSIC_DEVICE_TYPE)][DEVICE_INFO_ARR_LINE] != 0) //检测音乐振子是否存在
		{
			Drop_Ttl_Stu.Music_Msg_Online = 1;
		}
		else
		{
			Drop_Ttl_Stu.Music_Msg_Online = 0;		
		}
		
		drop_check_step = 0;
		drop_time_ms = 0;
		drop_set_event = 2;
	}
	
	if(drop_check_step)
	{
		drop_time_ms++;
		if(drop_time_ms >= 20)
		{
			drop_check_step++;	
			drop_time_ms = 20;			
		}
	}
	else
	{
		drop_time_ms = 0;
	}		
}
