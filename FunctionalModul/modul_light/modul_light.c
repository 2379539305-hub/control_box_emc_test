#include "modul_light.h"

#include "modul_ttlbus.h"


unsigned long light_event_flag = 0;

#define LED_PARA_START  (9)
static unsigned char LED_TXBuffer[20];
static unsigned char TTL_RGB_Write_Cmd(unsigned char device_code,unsigned char device_funccode_temp,unsigned char extend_data_length)   
{
	unsigned char temp_i = 0;
	unsigned short sum_check = 0;
	unsigned char bus_send_length = 10 + extend_data_length;
	
	if(RingBuffer.ring_write_data_lock != 0) return 0;
	RingBuffer.ring_write_data_lock = 1;		

	if(TTL_RingBuffer_CheckEnough(bus_send_length) == 0)
	{
		RingBuffer.ring_write_data_lock = 0;
		return 0;
	}
	
	LED_TXBuffer[0] = TTL_HEADER_ONE;LED_TXBuffer[1] = TTL_HEADER_TWO;
	LED_TXBuffer[2] = bus_send_length; LED_TXBuffer[3] = 0X00;
	
	LED_TXBuffer[4] = device_code; //设备型号
	LED_TXBuffer[5] = 0XFF; //找出类型是0x10的地址Master_SearchIdleAdd(MUSIC_MSGR_DEVICE)
	
	LED_TXBuffer[6] = 0x00;  //密钥
	LED_TXBuffer[7] = device_funccode_temp; //功能码
	LED_TXBuffer[8] = 0x01; //控制命令字
	
	
	for(temp_i = 2;temp_i < bus_send_length - 1;temp_i++)
	{
		sum_check += LED_TXBuffer[temp_i];
	}

	LED_TXBuffer[bus_send_length - 1] = sum_check%256;	
		

	for(temp_i = 0;temp_i < bus_send_length;temp_i ++)
	{
		if(Write_Ring_Data(LED_TXBuffer[temp_i]) == 1) //错误
		{
			break;
		}
	}	
		
	RingBuffer.ring_write_data_lock = 0;
	
	if(temp_i == bus_send_length)  return 1;

	return 0;
}
unsigned char TTL_Master_Send_WriteColour_Cmd(unsigned char device_code,unsigned char led_order,unsigned char *rgb_colour)   
{
	LED_TXBuffer[LED_PARA_START] = led_order;  //
	
	//单色灯
	if(device_code == LIGHT_ONE_DEVICE_TYPE)
	{
		if(*rgb_colour != 0)
		{
			LED_TXBuffer[LED_PARA_START + 1] = 255;
			LED_TXBuffer[LED_PARA_START + 2] = 255;
			LED_TXBuffer[LED_PARA_START + 3] = 255;					
		}
		else
		{
			LED_TXBuffer[LED_PARA_START + 1] = 0;
			LED_TXBuffer[LED_PARA_START + 2] = 0;
			LED_TXBuffer[LED_PARA_START + 3] = 0;			
		}
	}
	//RGB灯
	if(device_code == LIGHT_RGB_DEVICE_TYPE)
	{
		LED_TXBuffer[LED_PARA_START + 1] = *rgb_colour;
		LED_TXBuffer[LED_PARA_START + 2] = *(rgb_colour+1);
		LED_TXBuffer[LED_PARA_START + 3] = *(rgb_colour+2);
	}
	//
	TTL_RGB_Write_Cmd(device_code,0x23,4); 
	
	return 1;
}
unsigned char TTL_Master_Send_WriteColour_And_Brightness_Cmd(unsigned char device_code,unsigned char led_order,unsigned char *rgb_colour,unsigned char brightness)   
{
	LED_TXBuffer[LED_PARA_START] = led_order;  //
	
	//单色灯
	if(device_code == LIGHT_ONE_DEVICE_TYPE)
	{
		if(*rgb_colour != 0)
		{
			LED_TXBuffer[LED_PARA_START + 1] = (unsigned char)(((unsigned short)brightness*255)/100);
			LED_TXBuffer[LED_PARA_START + 2] = (unsigned char)(((unsigned short)brightness*255)/100);
			LED_TXBuffer[LED_PARA_START + 3] = (unsigned char)(((unsigned short)brightness*255)/100);					
		}
		else
		{
			LED_TXBuffer[LED_PARA_START + 1] = 0;
			LED_TXBuffer[LED_PARA_START + 2] = 0;
			LED_TXBuffer[LED_PARA_START + 3] = 0;			
		}
	}
	//RGB灯
	if(device_code == LIGHT_RGB_DEVICE_TYPE)
	{
		LED_TXBuffer[LED_PARA_START + 1] = (unsigned char)(((unsigned short)brightness*(*rgb_colour))/100);
		LED_TXBuffer[LED_PARA_START + 2] = (unsigned char)(((unsigned short)brightness*(*(rgb_colour+1)))/100);
		LED_TXBuffer[LED_PARA_START + 3] = (unsigned char)(((unsigned short)brightness*(*(rgb_colour+2)))/100);
	}
	//
	TTL_RGB_Write_Cmd(device_code,0x23,4); 
	
	return 1;
}
unsigned char TTL_Master_Send_WriteRgbMode_Cmd(unsigned char led_order,unsigned char mode_type,unsigned char mode_order)   
{
	LED_TXBuffer[LED_PARA_START] = led_order;  //
	
	LED_TXBuffer[LED_PARA_START + 1] = mode_type;
	
	LED_TXBuffer[LED_PARA_START + 2] = mode_order;

	//
	TTL_RGB_Write_Cmd(LIGHT_RGB_DEVICE_TYPE,0x24,3); 
	
	return 1;
}
unsigned char TTL_Master_Send_WriteRgbBreathMode_Cmd(unsigned char led_order, unsigned char *breath_freq, unsigned char r_color,unsigned char g_color,unsigned char b_color)   
{
	LED_TXBuffer[LED_PARA_START] = led_order;  //
	LED_TXBuffer[LED_PARA_START + 1] = breath_freq[0];
	LED_TXBuffer[LED_PARA_START + 2] = breath_freq[1];
	LED_TXBuffer[LED_PARA_START + 3] = r_color;
	LED_TXBuffer[LED_PARA_START + 4] = g_color;
	LED_TXBuffer[LED_PARA_START + 5] = b_color;
	//
	TTL_RGB_Write_Cmd(LIGHT_RGB_DEVICE_TYPE,0x25,6); 
	
	return 1;
}
unsigned char TTL_Master_Send_WriteRgbSleepMode_Cmd(unsigned char led_order, unsigned char *sleep_time, unsigned char r_color,unsigned char g_color,unsigned char b_color)   
{
	LED_TXBuffer[LED_PARA_START] = led_order;  //
	LED_TXBuffer[LED_PARA_START + 1] = sleep_time[0];
	LED_TXBuffer[LED_PARA_START + 2] = sleep_time[1];
	LED_TXBuffer[LED_PARA_START + 3] = r_color;
	LED_TXBuffer[LED_PARA_START + 4] = g_color;
	LED_TXBuffer[LED_PARA_START + 5] = b_color;
	//
	TTL_RGB_Write_Cmd(LIGHT_RGB_DEVICE_TYPE,0x26,6); 
	
	return 1;
}

unsigned char TTL_Master_Send_WriteRgbBrightness_Cmd(unsigned char led_order, unsigned char brightness)   
{
	LED_TXBuffer[LED_PARA_START] = led_order;  
	LED_TXBuffer[LED_PARA_START + 1] = brightness;
	//
	TTL_RGB_Write_Cmd(LIGHT_RGB_DEVICE_TYPE,0x27,2); 
	
	return 1;
}






