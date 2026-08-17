#include "modul_fan.h"

#include "modul_ttlbus.h"

#define FAN_DEVICE  (0X14)

static unsigned char FAN_TXBuffer[25];

unsigned char TTL_Fan_Write_Cmd(unsigned char device_funccode_temp , unsigned char extend_data_length)   
{
	unsigned char temp_i = 0;
	unsigned short sum_check = 0;
	unsigned char bus_send_length = 10 + extend_data_length;
	
	if(RingBuffer.ring_write_data_lock != 0) return 0;
	RingBuffer.ring_write_data_lock = 1;		
	
	FAN_TXBuffer[0] = TTL_HEADER_ONE;FAN_TXBuffer[1] = TTL_HEADER_TWO;
	FAN_TXBuffer[2] = bus_send_length; FAN_TXBuffer[3] = 0X00;
	
	FAN_TXBuffer[4] = FAN_DEVICE; //设备型号
	FAN_TXBuffer[5] = 0XFF; 
	
	FAN_TXBuffer[6] = 0x00;  //密钥
	FAN_TXBuffer[7] = device_funccode_temp; //功能码
	FAN_TXBuffer[8] = 0x01; //控制命令字
	
	
	for(temp_i = 2;temp_i < bus_send_length - 1;temp_i++)
	{
		sum_check += FAN_TXBuffer[temp_i];
	}

	FAN_TXBuffer[bus_send_length - 1] = sum_check%256;	
		

	for(temp_i = 0;temp_i < bus_send_length;temp_i ++)
	{
		if(Write_Ring_Data(FAN_TXBuffer[temp_i]) == 1) //错误
		{
			break;
		}
	}	
		
	RingBuffer.ring_write_data_lock = 0;
	
	if(temp_i == bus_send_length)  return 1;

	return 0;
}

/*
//按摩器模式设置
//序号现在默认写0XFF
//模式:随动  持续35HZ  脉冲35HZ  波浪35HZ
*/
unsigned char TTL_Master_Send_WriteFanModeTime_Cmd(unsigned char fan_order , unsigned char fan_mode_temp , unsigned short fan_time_temp)   
{
	FAN_TXBuffer[TTL_PARA_START] = fan_order;  //序号
	FAN_TXBuffer[TTL_PARA_START + 1] = fan_time_temp/256;  //定时时间
	FAN_TXBuffer[TTL_PARA_START + 2] = fan_time_temp%256;  //定时时间
	FAN_TXBuffer[TTL_PARA_START + 3] = fan_mode_temp;  //模式
	return TTL_Fan_Write_Cmd(0x21, 4);   
}

/*
//典型模式强度
//序号 1头部 2脚部   00全部
//强度 0 1 2 3
*/
unsigned char TTL_Master_Send_WriteFanInts_Cmd(unsigned char *fan_ints,unsigned char *fan_dir)
{
	FAN_TXBuffer[TTL_PARA_START] = 1;
	FAN_TXBuffer[TTL_PARA_START + 1] = fan_dir[1];  //强度
	FAN_TXBuffer[TTL_PARA_START + 2] = fan_ints[1];  //强度
	FAN_TXBuffer[TTL_PARA_START + 3] = 2;  //序号
	FAN_TXBuffer[TTL_PARA_START + 4] = fan_dir[2];  //强度
	FAN_TXBuffer[TTL_PARA_START + 5] = fan_ints[2];  //强度
	FAN_TXBuffer[TTL_PARA_START + 6] = 3;  //序号
	FAN_TXBuffer[TTL_PARA_START + 7] = fan_dir[3];  //强度
	FAN_TXBuffer[TTL_PARA_START + 8] = fan_ints[3];  //强度
	FAN_TXBuffer[TTL_PARA_START + 9] =  4;  //序号
	FAN_TXBuffer[TTL_PARA_START + 10] = fan_dir[4];  //强度
	FAN_TXBuffer[TTL_PARA_START + 11] = fan_ints[4];  //强度
	
	return TTL_Fan_Write_Cmd(0x22, 12); 
}

unsigned char TTL_Master_Send_WtireHeatSwitch_Cmd(unsigned char heat_order,unsigned char heat_switch)
{
	FAN_TXBuffer[TTL_PARA_START] = heat_order;  //序号
	FAN_TXBuffer[TTL_PARA_START + 1] = heat_switch;
	return TTL_Fan_Write_Cmd(0x30, 2);   	
}
