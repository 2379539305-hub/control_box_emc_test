#include "app_light.h"
#include "app_motor.h"

#include "modul_ttlbus.h"
#include "app_ttlbus.h"
#include "driver_beep.h"
#include "driver_periph.h"

#include "app_linbus.h"
#include "app_comm.h"
#include "app_config.h"
#include "app_ble.h"
#include "app_backhual.h"
#include "app_music.h"

LIGHT_STRUCT  Light_OneColour_Stu = {0}; //默认的灯颜色是
LIGHT_STRUCT  Light_RgbColour_Stu = {0}; //默认的灯颜色是白灯

unsigned char light_type_flag = LIGHT_RGB_DEVICE_TYPE;  //当前灯带类型

unsigned short light_para_set_event = 0;  //参数设置事件

unsigned char led_board_state = 0; //灯牌状态

static unsigned char  light_ctr_cmd = 0 , old_light_ctr_cmd = 0;

unsigned char rgb_target_gradient_colour[3];
unsigned char rgb_gradient_run_flag = 0;
unsigned char led_gradient_step = 0;

/* Fixed palettes: UBL key uses 8 colors, LIGHT key uses 3 therapy colors. */
#define RGB_SOURCE_UBL   0
#define RGB_SOURCE_LIGHT 1

static unsigned char rgb_last_source = RGB_SOURCE_UBL;

/* Track cycle/last state separately for different keys/modes. */
static unsigned char rgb_cycle_state_ubl = 1;
static unsigned char rgb_cycle_state_light = 1;

static unsigned char Light_GetLastRgbStateByConfig(void)
{
	if(rgb_last_source == RGB_SOURCE_LIGHT)
	{
		return rgb_cycle_state_light;
	}
	return rgb_cycle_state_ubl;
}

static void Light_SaveLastRgbStateByConfig(unsigned char state)
{
	if(state == 0)
	{
		return;
	}

	if(rgb_last_source == RGB_SOURCE_LIGHT)
	{
		rgb_cycle_state_light = state;
	}
	else
	{
		rgb_cycle_state_ubl = state;
	}
}

void Led_Board_ON(void);
void Led_Board_OFF(void);
void Led_OneColour_ON(void);
void Led_OneColour_OFF(void);
void Led_RgbColour_Set(unsigned char rgb_state_temp);
void Led_Rgb_OFF(void);
void Periph_LedBoardPort_Reconfig(void);
void Periph_UblPort_Reconfig(void);
unsigned char TTL_Start_Gradient_Colour(unsigned char device_code,unsigned char led_order, unsigned char *rgb_colour) ;

//void APP_LightInit(void)
//{
//	Light_OneColour_Stu.led_time_sec_set = system_config.flags.light_one_default_time_sec; 
//	Light_RgbColour_Stu.led_time_sec_set = system_config.flags.light_rgb_default_time_sec; 
//	
//	Light_RgbColour_Stu.led_colour_state = 0;
//	Light_RgbColour_Stu.light_colour[0] = 255;Light_RgbColour_Stu.light_colour[1] = 255;Light_RgbColour_Stu.light_colour[2] = 255;
//}

void APP_LightInit(void)
{
		Light_OneColour_Stu.led_time_sec_set = system_config.flags.light_one_default_time_sec;
		Light_RgbColour_Stu.led_time_sec_set = system_config.flags.light_rgb_default_time_sec;
		
		Light_RgbColour_Stu.led_colour_state = 0;
		Light_RgbColour_Stu.light_colour[0] = 0;Light_RgbColour_Stu.light_colour[1] = 0;Light_RgbColour_Stu.light_colour[2] = 0;
		Light_RgbColour_Stu.light_mode = LIGHT_CONTROL_RGB_MODE;
		Light_RgbColour_Stu.light_brightness = 100;

		rgb_last_source = RGB_SOURCE_UBL;
		rgb_cycle_state_ubl = 1;
		rgb_cycle_state_light = 1;
		
}
unsigned char Light_AcceptCmd_KeyInfo(unsigned char key_temp)
{
	if(key_temp >= KEY_LIGHT_START && key_temp <= KEY_LIGHT_END)
	{
		return 1;
	}
	return 0;
}

void Light_Control(void)
{
	unsigned char key_value_temp = Get_Key_Value();
	/*--------------------------获取有用的键值信息---------------------------*/
	if((0 == key_value_temp) || (1 == Light_AcceptCmd_KeyInfo(key_value_temp)))
	{
		
	}
	else  //无用信息
	{
		return;
	}
	old_light_ctr_cmd = light_ctr_cmd;
	
	light_ctr_cmd = key_value_temp;
//	
//	TTL_GetClear_KeyValue(0); //此处是防止重复进入 
	
	if((light_ctr_cmd != 0 && light_ctr_cmd != old_light_ctr_cmd))
	{
		if(light_ctr_cmd == KEY_UBL_ON)
		{
			light_para_set_event |= LIGHT_UBL_SW_EVENT;
			Light_OneColour_Stu.led_colour_state = 1;
			rgb_last_source = RGB_SOURCE_UBL;
			Light_RgbColour_Stu.led_colour_state = Light_GetLastRgbStateByConfig();
			Led_RgbColour_Set(Light_RgbColour_Stu.led_colour_state);
			
			if(Light_RgbColour_Stu.led_colour_state != 0) //颜色状态
			{
				Light_SaveLastRgbStateByConfig(Light_RgbColour_Stu.led_colour_state);
			}
		}
		//开灯
		if(light_ctr_cmd == KEY_UBL_SW)
		{
			rgb_last_source = RGB_SOURCE_UBL;
			light_para_set_event |= LIGHT_UBL_SW_EVENT;
			
			//单色灯
			if(Light_OneColour_Stu.led_colour_state != 0)
			{
				Light_OneColour_Stu.led_colour_state = 0;
			}
			else
			{
				Light_OneColour_Stu.led_colour_state = 1;
			}
			if(system_config.flags.ubl_remote_change_color == UBL_CHANGE_COLOR_DISABLE)
			{
				if(Light_RgbColour_Stu.light_colour[RGB_R_BIT] == 0x00 && Light_RgbColour_Stu.light_colour[RGB_G_BIT] == 0X00 && Light_RgbColour_Stu.light_colour[RGB_B_BIT] == 0X00)
				{
					Light_RgbColour_Stu.led_colour_state = rgb_cycle_state_ubl;
					Led_RgbColour_Set(Light_RgbColour_Stu.led_colour_state);        
				}
				else
				{
					Led_Rgb_OFF();
				}
			}
			else
			{
				//如果冷暖双色配置
				if(system_config.flags.ubl_remote_change_color == UBL_DOUBLE_COLOR_CONFIG)
				{//RGB灯
					Light_RgbColour_Stu.led_colour_state ++;
					if(Light_RgbColour_Stu.led_colour_state > 2)
					{
						Light_RgbColour_Stu.led_colour_state = 0;
					}
				}
				else if(system_config.flags.ubl_remote_change_color == UBL_CHANGE_COLOR_ENABLE)
				{
					//RGB灯
					Light_RgbColour_Stu.led_colour_state ++;
					if(Light_RgbColour_Stu.led_colour_state > 8)
					{
						Light_RgbColour_Stu.led_colour_state = 0;
					}
				}
				else if(system_config.flags.ubl_remote_change_color == UBL_THERAPY_COLOR_CONFIG)
				{
					//RGB灯
					Light_RgbColour_Stu.led_colour_state ++;
					if(Light_RgbColour_Stu.led_colour_state > 3)
					{
						Light_RgbColour_Stu.led_colour_state = 0;
					}
				}	
					
				Led_RgbColour_Set(Light_RgbColour_Stu.led_colour_state);	
			}
			if(Light_RgbColour_Stu.led_colour_state != 0) //颜色状态
			{
				Light_SaveLastRgbStateByConfig(Light_RgbColour_Stu.led_colour_state);
			}		
		}
		//关灯
		if(light_ctr_cmd == KEY_UBL_OFF)
		{
			light_para_set_event |= LIGHT_UBL_OFF_EVENT;
		}
		//开灯
		if(light_ctr_cmd == KEY_LIGHT_SW)
		{
			light_para_set_event |= LIGHT_UBL_SW_EVENT;
			//光疗灯
			rgb_last_source = RGB_SOURCE_LIGHT;

			if(system_config.flags.ubl_remote_change_color == UBL_CHANGE_COLOR_DISABLE)
			{
				if(Light_RgbColour_Stu.light_colour[RGB_R_BIT] == 0x00 && Light_RgbColour_Stu.light_colour[RGB_G_BIT] == 0X00 && Light_RgbColour_Stu.light_colour[RGB_B_BIT] == 0X00)
				{
					Light_RgbColour_Stu.led_colour_state = rgb_cycle_state_light;
					Led_RgbColour_Set(Light_RgbColour_Stu.led_colour_state);        
				}
				else
				{
					Led_Rgb_OFF();
				}
			}
			else
			{
				Light_RgbColour_Stu.led_colour_state ++;
				if(Light_RgbColour_Stu.led_colour_state > 3)
				{
					Light_RgbColour_Stu.led_colour_state = 0;
				}
				Led_RgbColour_Set(Light_RgbColour_Stu.led_colour_state);
			}
			if(Light_RgbColour_Stu.led_colour_state != 0) //颜色状态
			{
				Light_SaveLastRgbStateByConfig(Light_RgbColour_Stu.led_colour_state);
			}		
		}		
		//关灯
		if(light_ctr_cmd == KEY_LIGHT_OFF)
		{
			light_para_set_event |= LIGHT_UBL_OFF_EVENT;
		}
		//灯牌控制
		if(light_ctr_cmd == KEY_LED_BOARD_SW)
		{
			if(led_board_state != 0)
			{
				led_board_state = 0;
			}
			else
			{
				led_board_state = 1;
			}	
			light_para_set_event |= LIGHT_BOARD_STATE_EVENT;	
		}
		if(light_ctr_cmd == KEY_LED_BOARD_ON)
		{
			led_board_state = 1;	
			light_para_set_event |= LIGHT_BOARD_STATE_EVENT;	
		}		//
		if(light_ctr_cmd == KEY_LED_BOARD_OFF)
		{
			led_board_state = 0;	
			light_para_set_event |= LIGHT_BOARD_STATE_EVENT;	
		}		//		
	}
	/*------------------------------感应控制-----------------------------------*/
#if 1	//感应不需要LIN同步
	if(Light_OneColour_Stu.light_sensor_signal != 0)
	{
		Light_Clear_TimeCount(&Light_OneColour_Stu);
		
		Led_OneColour_ON();		
		if(1 == TTL_Master_Send_WriteColour_Cmd(LIGHT_ONE_DEVICE_TYPE,0x01,&Light_OneColour_Stu.led_colour_state))
		{
			Light_OneColour_Stu.light_sensor_signal = 0;
		}		
	}
	if(Light_RgbColour_Stu.light_sensor_signal != 0)
	{
		Light_Clear_TimeCount(&Light_RgbColour_Stu);
		Light_RgbColour_Stu.led_colour_state = Light_GetLastRgbStateByConfig();
		Led_RgbColour_Set(Light_RgbColour_Stu.led_colour_state);
		if(system_config.flags.light_rgb_gradient_color == UBL_GRADIENT_COLOR_DISABLE)
		{
			if(1 == TTL_Master_Send_WriteColour_Cmd(LIGHT_RGB_DEVICE_TYPE,0x01,Light_RgbColour_Stu.light_colour))
			{
				Light_RgbColour_Stu.light_sensor_signal = 0;
			}	
		}	
		else if(system_config.flags.light_rgb_gradient_color == UBL_GRADIENT_COLOR_ENABLE)
		{
			if(1 == TTL_Start_Gradient_Colour(LIGHT_RGB_DEVICE_TYPE,0x01,Light_RgbColour_Stu.light_colour))
			{
				Light_RgbColour_Stu.light_sensor_signal = 0;
			}	
		}			
	}
#endif
	/*------------------------------------------定时关闭------------------------------------------------------------*/
	//定时关不需要LIN同步
	if(Light_OneColour_Stu.led_sec_count >= Light_OneColour_Stu.led_time_sec_set && Light_OneColour_Stu.led_time_sec_set > 0) //单色灯
	{
		Led_OneColour_OFF();
		
		if(1 == TTL_Master_Send_WriteColour_Cmd(LIGHT_ONE_DEVICE_TYPE,0x01,&Light_OneColour_Stu.led_colour_state))
		{
			Light_Clear_TimeCount(&Light_OneColour_Stu);
		}			
		//
	}
	if(Light_RgbColour_Stu.led_sec_count >= Light_RgbColour_Stu.led_time_sec_set && Light_RgbColour_Stu.led_time_sec_set > 0) //RGB灯
	{
		Led_Rgb_OFF();
		if(system_config.flags.light_rgb_gradient_color == UBL_GRADIENT_COLOR_DISABLE)
		{
			if(1 == TTL_Master_Send_WriteColour_Cmd(LIGHT_RGB_DEVICE_TYPE,0x01,Light_RgbColour_Stu.light_colour))
			{
				Light_Clear_TimeCount(&Light_RgbColour_Stu);
			}
		}
		else if(system_config.flags.light_rgb_gradient_color == UBL_GRADIENT_COLOR_ENABLE)
		{
			if(1 == TTL_Start_Gradient_Colour(LIGHT_RGB_DEVICE_TYPE,0x01,Light_RgbColour_Stu.light_colour))
			{
				Light_Clear_TimeCount(&Light_RgbColour_Stu);
			}
		}
	}
	
	
	/*----------------------------状态同步设置事件-----------------------------------*/
	if(light_para_set_event != 0x0000)
	{		
		 //颜色状态切换
		if((light_para_set_event & LIGHT_UBL_SW_EVENT) == LIGHT_UBL_SW_EVENT) 
		{
			Periph_UblPort_Reconfig();
			Light_Clear_TimeCount(&Light_RgbColour_Stu);
			Light_RgbColour_Stu.light_mode = LIGHT_CONTROL_RGB_MODE;
			
			/*-----------------------------LIN从机控制盒------------------------------------*/
			LIN_Master_Send_WriteColour_Cmd(LIGHT_ONE_DEVICE_TYPE,0x01,&Light_OneColour_Stu.led_colour_state); 
			LIN_Master_Send_WriteColour_Cmd(LIGHT_RGB_DEVICE_TYPE,0x01,Light_RgbColour_Stu.light_colour); 
			/*------------------------------TTL设备控制-------------------------------------*/
			if(1 == TTL_Master_Send_WriteColour_Cmd(LIGHT_ONE_DEVICE_TYPE,0x01,&Light_OneColour_Stu.led_colour_state))//单色灯
			{
				if(system_config.flags.light_rgb_gradient_color == UBL_GRADIENT_COLOR_DISABLE)
				{
					if(1 == TTL_Master_Send_WriteColour_Cmd(LIGHT_RGB_DEVICE_TYPE,0x01,Light_RgbColour_Stu.light_colour))	   //RGB灯
					{
						light_para_set_event &= ~LIGHT_UBL_SW_EVENT;
					}
				}
				else if(system_config.flags.light_rgb_gradient_color == UBL_GRADIENT_COLOR_ENABLE)
				{
					if(1 == TTL_Start_Gradient_Colour(LIGHT_RGB_DEVICE_TYPE,0x01,Light_RgbColour_Stu.light_colour))	   //RGB灯
					{
						light_para_set_event &= ~LIGHT_UBL_SW_EVENT;
					}
				} 
			}
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_RGB_STATE_CHANGE_EVENT);
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_RGB_MODE_CHANGE_EVENT);
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_RGB_COLOR_CHANGE_EVENT);
		}
		//关灯
		if((light_para_set_event & LIGHT_UBL_OFF_EVENT) == LIGHT_UBL_OFF_EVENT)  
		{
			Light_Clear_TimeCount(&Light_RgbColour_Stu);
			
			Led_OneColour_OFF();
			Led_Rgb_OFF();
			Light_RgbColour_Stu.light_mode = LIGHT_CONTROL_RGB_MODE;
			/*-----------------------------LIN从机控制盒------------------------------------*/
			LIN_Master_Send_WriteColour_Cmd(LIGHT_ONE_DEVICE_TYPE,0x01,&Light_OneColour_Stu.led_colour_state);
			LIN_Master_Send_WriteColour_Cmd(LIGHT_RGB_DEVICE_TYPE,0x01,Light_RgbColour_Stu.light_colour);
			/*------------------------------TTL设备控制-------------------------------------*/
			if(1 == TTL_Master_Send_WriteColour_Cmd(LIGHT_ONE_DEVICE_TYPE,0x01,&Light_OneColour_Stu.led_colour_state))//单色灯
			{
				if(system_config.flags.light_rgb_gradient_color == UBL_GRADIENT_COLOR_DISABLE)
				{
					if(1 == TTL_Master_Send_WriteColour_Cmd(LIGHT_RGB_DEVICE_TYPE,0x01,Light_RgbColour_Stu.light_colour))	  //RGB灯	
					{
						light_para_set_event &= ~LIGHT_UBL_OFF_EVENT;
					}		
				}	
				else if(system_config.flags.light_rgb_gradient_color == UBL_GRADIENT_COLOR_ENABLE)
				{
					if(1 == TTL_Start_Gradient_Colour(LIGHT_RGB_DEVICE_TYPE,0x01,Light_RgbColour_Stu.light_colour))	  //RGB灯	
					{
						light_para_set_event &= ~LIGHT_UBL_OFF_EVENT;
					}		
				}				
			}
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_RGB_STATE_CHANGE_EVENT);
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_RGB_MODE_CHANGE_EVENT);
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_RGB_COLOR_CHANGE_EVENT);
		}
		
		if((light_para_set_event & LIGHT_UBL_COLOUR_EVENT) == LIGHT_UBL_COLOUR_EVENT)  //设置颜色
		{
			Light_Clear_TimeCount(&Light_RgbColour_Stu);
			
			Light_RgbColour_Stu.light_mode = LIGHT_CONTROL_RGB_MODE;
			
			Light_RgbColour_Stu.led_colour_state = Led_RgbColour_GetState(Light_RgbColour_Stu);
			if(Light_RgbColour_Stu.led_colour_state != 0)
			{
				Light_SaveLastRgbStateByConfig(Light_RgbColour_Stu.led_colour_state);
			}
			
			/*-----------------------------LIN从机控制盒------------------------------------*/
			LIN_Master_Send_WriteColour_Cmd(LIGHT_RGB_DEVICE_TYPE,0x01,Light_RgbColour_Stu.light_colour); 
			/*------------------------------TTL设备控制-------------------------------------*/
			if(system_config.flags.light_rgb_gradient_color == UBL_GRADIENT_COLOR_DISABLE)
			{
				if(1 == TTL_Master_Send_WriteColour_Cmd(LIGHT_RGB_DEVICE_TYPE,0x01,Light_RgbColour_Stu.light_colour))	   //RGB灯	
				{
					light_para_set_event &= ~LIGHT_UBL_COLOUR_EVENT;
				}	
			}
			else if(system_config.flags.light_rgb_gradient_color == UBL_GRADIENT_COLOR_ENABLE)
			{
				if(1 == TTL_Start_Gradient_Colour(LIGHT_RGB_DEVICE_TYPE,0x01,Light_RgbColour_Stu.light_colour))	   //RGB灯	
				{
					light_para_set_event &= ~LIGHT_UBL_COLOUR_EVENT;
				}	
			}			
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_RGB_STATE_CHANGE_EVENT);
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_RGB_MODE_CHANGE_EVENT);
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_RGB_COLOR_CHANGE_EVENT);
		}
		if((light_para_set_event & LIGHT_UBL_ON_EVENT) == LIGHT_UBL_ON_EVENT)  //on
		{
			Light_OneColour_Stu.led_colour_state = 1;
			Light_RgbColour_Stu.led_colour_state = Light_GetLastRgbStateByConfig();
			Led_RgbColour_Set(Light_RgbColour_Stu.led_colour_state);
			
			Light_RgbColour_Stu.light_mode = LIGHT_CONTROL_RGB_MODE;
			
			if(Light_RgbColour_Stu.led_colour_state != 0) //颜色状态
			{
				Light_SaveLastRgbStateByConfig(Light_RgbColour_Stu.led_colour_state);
			}
			Light_Clear_TimeCount(&Light_RgbColour_Stu);
			
			/*-----------------------------LIN从机控制盒------------------------------------*/
			LIN_Master_Send_WriteColour_Cmd(LIGHT_ONE_DEVICE_TYPE,0x01,&Light_OneColour_Stu.led_colour_state); 
			LIN_Master_Send_WriteColour_Cmd(LIGHT_RGB_DEVICE_TYPE,0x01,Light_RgbColour_Stu.light_colour); 
			/*------------------------------TTL设备控制-------------------------------------*/
			if(1 == TTL_Master_Send_WriteColour_Cmd(LIGHT_ONE_DEVICE_TYPE,0x01,&Light_OneColour_Stu.led_colour_state))//单色灯
			{
				if(system_config.flags.light_rgb_gradient_color == UBL_GRADIENT_COLOR_DISABLE)
				{
					if(1 == TTL_Master_Send_WriteColour_Cmd(LIGHT_RGB_DEVICE_TYPE,0x01,Light_RgbColour_Stu.light_colour))	   //RGB灯
					{
						light_para_set_event &= ~LIGHT_UBL_ON_EVENT;
					}
				}
				else if(system_config.flags.light_rgb_gradient_color == UBL_GRADIENT_COLOR_ENABLE)
				{
					if(1 == TTL_Start_Gradient_Colour(LIGHT_RGB_DEVICE_TYPE,0x01,Light_RgbColour_Stu.light_colour))	   //RGB灯
					{
						light_para_set_event &= ~LIGHT_UBL_ON_EVENT;
					}
				}				
			}
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_RGB_STATE_CHANGE_EVENT);
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_RGB_MODE_CHANGE_EVENT);
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_RGB_COLOR_CHANGE_EVENT);
		}		
		//时间
		if((light_para_set_event & LIGHT_UBL_TIME_EVENT) == LIGHT_UBL_TIME_EVENT) 
		{
			Light_Clear_TimeCount(&Light_RgbColour_Stu);

			/*-----------------------------LIN从机控制盒------------------------------------*/
			LIN_Master_Send_WriteUBLTime_Cmd(0x00,Light_OneColour_Stu.led_time_sec_set);
			LIN_Master_Send_WriteUBLTime_Cmd(0x00,Light_RgbColour_Stu.led_time_sec_set);
			//
			light_para_set_event &= ~LIGHT_UBL_TIME_EVENT;
			//ble_report_event |= REPORT_RGB_STATE_CHANGE_EVENT;
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_RGB_TIME_CHANGE_EVENT);
			//ble_report_event |= REPORT_RGB_COLOR_CHANGE_EVENT;			
		}
		if((light_para_set_event & LIGHT_UBL_MODE_EVENT) == LIGHT_UBL_MODE_EVENT) 
		{
			Light_Clear_TimeCount(&Light_RgbColour_Stu);
			
			if(Light_RgbColour_Stu.light_colour_mode[MODE_TYPE_BIT] == 1)
			{
				Light_RgbColour_Stu.light_mode = LIGTH_COLOUR_RGB_MODE;
			}
			else if(Light_RgbColour_Stu.light_colour_mode[MODE_TYPE_BIT] == 2)
			{
				Light_RgbColour_Stu.light_mode = LIGTH_MUSIC_RGB_MODE;
			}
			LIN_Master_Send_WriteRgbMode_Cmd(0x01,Light_RgbColour_Stu.light_colour_mode[MODE_TYPE_BIT],Light_RgbColour_Stu.light_colour_mode[MODE_ORDER_BIT]); 
			
			if(1 == TTL_Master_Send_WriteRgbMode_Cmd(0x01,Light_RgbColour_Stu.light_colour_mode[MODE_TYPE_BIT],Light_RgbColour_Stu.light_colour_mode[MODE_ORDER_BIT]))	   //RGB灯
			{
				light_para_set_event &= ~LIGHT_UBL_MODE_EVENT;
			}	
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_RGB_MODE_CHANGE_EVENT);
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_RGB_BRIGHTNESS_EVENT);
		}		
		if((light_para_set_event & LIGHT_UBL_BREATH_MODE_EVENT) == LIGHT_UBL_BREATH_MODE_EVENT) 
		{
			Light_Clear_TimeCount(&Light_RgbColour_Stu);
			
			Light_RgbColour_Stu.light_mode = LIGHT_BREATH_RGB_MODE;
			
			if((Light_RgbColour_Stu.light_colour[RGB_R_BIT] == 0) && (Light_RgbColour_Stu.light_colour[RGB_G_BIT] == 0) && (Light_RgbColour_Stu.light_colour[RGB_B_BIT] == 0))
			{
				Light_RgbColour_Stu.led_colour_state = Light_GetLastRgbStateByConfig();
				Led_RgbColour_Set(Light_RgbColour_Stu.led_colour_state);
				if(Light_RgbColour_Stu.led_colour_state != 0)
				{
					Light_SaveLastRgbStateByConfig(Light_RgbColour_Stu.led_colour_state);
				}
			}				
			
			LIN_Master_Send_WriteRgbBreathMode_Cmd(0x01,&Light_RgbColour_Stu.light_breath_time[0],Light_RgbColour_Stu.light_colour[RGB_R_BIT],Light_RgbColour_Stu.light_colour[RGB_G_BIT],Light_RgbColour_Stu.light_colour[RGB_B_BIT]); 
			
			if(1 == TTL_Master_Send_WriteRgbBreathMode_Cmd(0x01,&Light_RgbColour_Stu.light_breath_time[0],Light_RgbColour_Stu.light_colour[RGB_R_BIT],Light_RgbColour_Stu.light_colour[RGB_G_BIT],Light_RgbColour_Stu.light_colour[RGB_B_BIT]))	   //RGB灯
			{
				light_para_set_event &= ~LIGHT_UBL_BREATH_MODE_EVENT;
			}	
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_RGB_MODE_CHANGE_EVENT);
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_RGB_BREATH_MODE_EVENT);
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_RGB_BRIGHTNESS_EVENT);
		}	
		if((light_para_set_event & LIGHT_UBL_BRIGHTNESS_EVENT) == LIGHT_UBL_BRIGHTNESS_EVENT) 
		{
			Light_Clear_TimeCount(&Light_RgbColour_Stu);							
			
			LIN_Master_Send_WriteRgbBrightness_Cmd(0x01,Light_RgbColour_Stu.light_brightness); 
			
			if(1 == TTL_Master_Send_WriteRgbBrightness_Cmd(0x01,Light_RgbColour_Stu.light_brightness))	   //RGB灯
			{
				light_para_set_event &= ~LIGHT_UBL_BRIGHTNESS_EVENT;
			}	
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_RGB_STATE_CHANGE_EVENT);	
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_RGB_BRIGHTNESS_EVENT);			
		}
		//灯牌
		if((light_para_set_event & LIGHT_BOARD_STATE_EVENT) == LIGHT_BOARD_STATE_EVENT) 
		{
			Periph_LedBoardPort_Reconfig();
			/*-----------------------------LIN从机控制盒------------------------------------*/
			LIN_Master_Send_WriteColour_Cmd(LIGHT_BOARD_DEVICE_TYPE,0x01,&led_board_state);
			//
			light_para_set_event &= ~LIGHT_BOARD_STATE_EVENT;
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_BORAD_STATE_EVENT);
		}
	}	
}

void Led_Board_ON(void)
{
	led_board_state = 1;
}
void Led_Board_OFF(void)
{
	led_board_state = 0;
}
void Led_OneColour_ON(void)
{
	Light_OneColour_Stu.led_colour_state = 1;
}
void Led_OneColour_OFF(void)
{
	Light_OneColour_Stu.led_colour_state = 0;
}
void Led_RgbColour_Set(unsigned char rgb_state_temp)
{
	Light_Clear_TimeCount(&Light_RgbColour_Stu);
	if(rgb_last_source == RGB_SOURCE_LIGHT)
	{
		switch(rgb_state_temp)
		{
			case 0:
			{
				Light_RgbColour_Stu.light_colour[RGB_R_BIT] = 0;Light_RgbColour_Stu.light_colour[RGB_G_BIT] = 0;Light_RgbColour_Stu.light_colour[RGB_B_BIT] = 0;//关闭
			}break;				
			case 1:
			{
				Light_RgbColour_Stu.light_colour[RGB_R_BIT] = 255;Light_RgbColour_Stu.light_colour[RGB_G_BIT] = 140;Light_RgbColour_Stu.light_colour[RGB_B_BIT] = 20; //暖
			}break;
			case 2:
			{
				Light_RgbColour_Stu.light_colour[RGB_R_BIT] = 255;Light_RgbColour_Stu.light_colour[RGB_G_BIT] = 220;Light_RgbColour_Stu.light_colour[RGB_B_BIT] = 120; //自然
			}break;
			case 3:
			{
				Light_RgbColour_Stu.light_colour[RGB_R_BIT] = 255;Light_RgbColour_Stu.light_colour[RGB_G_BIT] = 255;Light_RgbColour_Stu.light_colour[RGB_B_BIT] = 255;//冷
			}break;
			default:
			{
				Light_RgbColour_Stu.led_colour_state = 0;
				Light_RgbColour_Stu.light_colour[RGB_R_BIT] = 0;Light_RgbColour_Stu.light_colour[RGB_G_BIT] = 0;Light_RgbColour_Stu.light_colour[RGB_B_BIT] = 0;
			}break;
		}	
	}
	else
	{
		//UBL模式，有三种配置模式，分别是双色配置、全彩配置、光疗配置，可以通过system_config.flags.ubl_remote_change_color来判断当前配置模式
		if(system_config.flags.ubl_remote_change_color == UBL_DOUBLE_COLOR_CONFIG)
		{
			switch(rgb_state_temp)
			{
				case 0:
				{
					Light_RgbColour_Stu.light_colour[RGB_R_BIT] = 0;Light_RgbColour_Stu.light_colour[RGB_G_BIT] = 0;Light_RgbColour_Stu.light_colour[RGB_B_BIT] = 0;//关闭
				}break;				
				case 1:
				{
					Light_RgbColour_Stu.light_colour[RGB_R_BIT] = 255;Light_RgbColour_Stu.light_colour[RGB_G_BIT] = 255;Light_RgbColour_Stu.light_colour[RGB_B_BIT] = 255; //白
				}break;
				case 2:
				{
					Light_RgbColour_Stu.light_colour[RGB_R_BIT] = 255;Light_RgbColour_Stu.light_colour[RGB_G_BIT] = 140;Light_RgbColour_Stu.light_colour[RGB_B_BIT] = 20; //暖白
				}break;
				default:
				{
					Light_RgbColour_Stu.led_colour_state = 0;
					Light_RgbColour_Stu.light_colour[RGB_R_BIT] = 0;Light_RgbColour_Stu.light_colour[RGB_G_BIT] = 0;Light_RgbColour_Stu.light_colour[RGB_B_BIT] = 0;
				}break;
			}
		
		}
		else if(system_config.flags.ubl_remote_change_color == UBL_CHANGE_COLOR_ENABLE)
		{
			switch(rgb_state_temp)
			{
				case 0:
				{
					Light_RgbColour_Stu.light_colour[RGB_R_BIT] = 0;Light_RgbColour_Stu.light_colour[RGB_G_BIT] = 0;Light_RgbColour_Stu.light_colour[RGB_B_BIT] = 0;//关闭
				}break;				
				case 1:
				{
					Light_RgbColour_Stu.light_colour[RGB_R_BIT] = 255;Light_RgbColour_Stu.light_colour[RGB_G_BIT] = 255;Light_RgbColour_Stu.light_colour[RGB_B_BIT] = 255; //白
				}break;
				case 2:
				{
					Light_RgbColour_Stu.light_colour[RGB_R_BIT] = 255;Light_RgbColour_Stu.light_colour[RGB_G_BIT] = 0;Light_RgbColour_Stu.light_colour[RGB_B_BIT] = 0; //红
				}break;
				case 3:
				{
					Light_RgbColour_Stu.light_colour[RGB_R_BIT] = 255;Light_RgbColour_Stu.light_colour[RGB_G_BIT] = 165;Light_RgbColour_Stu.light_colour[RGB_B_BIT] = 0;//橙
				}break;
				case 4:
				{
					Light_RgbColour_Stu.light_colour[RGB_R_BIT] = 255;Light_RgbColour_Stu.light_colour[RGB_G_BIT] = 255;Light_RgbColour_Stu.light_colour[RGB_B_BIT] = 0;//黄
				}break;
				case 5:
				{
					Light_RgbColour_Stu.light_colour[RGB_R_BIT] = 0;Light_RgbColour_Stu.light_colour[RGB_G_BIT] = 255;Light_RgbColour_Stu.light_colour[RGB_B_BIT] = 0; //绿
				}break;
				case 6:
				{
					Light_RgbColour_Stu.light_colour[RGB_R_BIT] = 0;Light_RgbColour_Stu.light_colour[RGB_G_BIT] = 127;Light_RgbColour_Stu.light_colour[RGB_B_BIT] = 255;//青
				}break;
				case 7:
				{
					Light_RgbColour_Stu.light_colour[RGB_R_BIT] = 0;Light_RgbColour_Stu.light_colour[RGB_G_BIT] = 0;Light_RgbColour_Stu.light_colour[RGB_B_BIT] = 255;//蓝
				}break;
				case 8:
				{
					Light_RgbColour_Stu.light_colour[RGB_R_BIT] = 139;Light_RgbColour_Stu.light_colour[RGB_G_BIT] = 0;Light_RgbColour_Stu.light_colour[RGB_B_BIT] = 255;//紫
				}break;
				default:
				{
					Light_RgbColour_Stu.led_colour_state = 0;
					Light_RgbColour_Stu.light_colour[RGB_R_BIT] = 0;Light_RgbColour_Stu.light_colour[RGB_G_BIT] = 0;Light_RgbColour_Stu.light_colour[RGB_B_BIT] = 0;
				}break;
			}	
		}
		else if(system_config.flags.ubl_remote_change_color == UBL_THERAPY_COLOR_CONFIG)
		{
			switch(rgb_state_temp)
			{
				case 0:
				{
					Light_RgbColour_Stu.light_colour[RGB_R_BIT] = 0;Light_RgbColour_Stu.light_colour[RGB_G_BIT] = 0;Light_RgbColour_Stu.light_colour[RGB_B_BIT] = 0;//关闭
				}break;				
				case 1:
				{
					Light_RgbColour_Stu.light_colour[RGB_R_BIT] = 255;Light_RgbColour_Stu.light_colour[RGB_G_BIT] = 140;Light_RgbColour_Stu.light_colour[RGB_B_BIT] = 20; //暖
				}break;
				case 2:
				{
					Light_RgbColour_Stu.light_colour[RGB_R_BIT] = 255;Light_RgbColour_Stu.light_colour[RGB_G_BIT] = 220;Light_RgbColour_Stu.light_colour[RGB_B_BIT] = 120; //自然
				}break;
				case 3:
				{
					Light_RgbColour_Stu.light_colour[RGB_R_BIT] = 255;Light_RgbColour_Stu.light_colour[RGB_G_BIT] = 255;Light_RgbColour_Stu.light_colour[RGB_B_BIT] = 255;//冷
				}break;
				default:
				{
					Light_RgbColour_Stu.led_colour_state = 0;
					Light_RgbColour_Stu.light_colour[RGB_R_BIT] = 0;Light_RgbColour_Stu.light_colour[RGB_G_BIT] = 0;Light_RgbColour_Stu.light_colour[RGB_B_BIT] = 0;
				}break;
			}	
		}	
		else
		{
			switch(rgb_state_temp)
			{
				case 0:
				{
					Light_RgbColour_Stu.light_colour[RGB_R_BIT] = 0;Light_RgbColour_Stu.light_colour[RGB_G_BIT] = 0;Light_RgbColour_Stu.light_colour[RGB_B_BIT] = 0;//关闭
				}break;				
				case 1:
				{
					Light_RgbColour_Stu.light_colour[RGB_R_BIT] = 255;Light_RgbColour_Stu.light_colour[RGB_G_BIT] = 255;Light_RgbColour_Stu.light_colour[RGB_B_BIT] = 255; //白
				}break;
			}				
		}
	}
}
unsigned char Led_RgbColour_GetState(LIGHT_STRUCT Light_Stu_Temp)
{
	if(rgb_last_source == RGB_SOURCE_LIGHT)
	{
		if(Light_Stu_Temp.light_colour[RGB_R_BIT] == 0   && Light_Stu_Temp.light_colour[RGB_G_BIT] == 0   && Light_Stu_Temp.light_colour[RGB_B_BIT] == 0)   return 0;
		if(Light_Stu_Temp.light_colour[RGB_R_BIT] == 255 && Light_Stu_Temp.light_colour[RGB_G_BIT] == 140 && Light_Stu_Temp.light_colour[RGB_B_BIT] == 20)  return 1;
		if(Light_Stu_Temp.light_colour[RGB_R_BIT] == 255 && Light_Stu_Temp.light_colour[RGB_G_BIT] == 220 && Light_Stu_Temp.light_colour[RGB_B_BIT] == 120) return 2;
		if(Light_Stu_Temp.light_colour[RGB_R_BIT] == 255 && Light_Stu_Temp.light_colour[RGB_G_BIT] == 255 && Light_Stu_Temp.light_colour[RGB_B_BIT] == 255) return 3;
	}
	else
	{
		if(system_config.flags.ubl_remote_change_color == UBL_DOUBLE_COLOR_CONFIG)
		{
			if(Light_Stu_Temp.light_colour[RGB_R_BIT] == 0   && Light_Stu_Temp.light_colour[RGB_G_BIT] == 0   && Light_Stu_Temp.light_colour[RGB_B_BIT] == 0)   return 0;
			if(Light_Stu_Temp.light_colour[RGB_R_BIT] == 255 && Light_Stu_Temp.light_colour[RGB_G_BIT] == 255 && Light_Stu_Temp.light_colour[RGB_B_BIT] == 255) return 1;
			if(Light_Stu_Temp.light_colour[RGB_R_BIT] == 255 && Light_Stu_Temp.light_colour[RGB_G_BIT] == 140 && Light_Stu_Temp.light_colour[RGB_B_BIT] == 20)  return 2;
		}
		else if(system_config.flags.ubl_remote_change_color == UBL_CHANGE_COLOR_ENABLE)
		{
			if(Light_Stu_Temp.light_colour[RGB_R_BIT] == 0   && Light_Stu_Temp.light_colour[RGB_G_BIT] == 0   && Light_Stu_Temp.light_colour[RGB_B_BIT] == 0)   return 0;
			if(Light_Stu_Temp.light_colour[RGB_R_BIT] == 255 && Light_Stu_Temp.light_colour[RGB_G_BIT] == 255 && Light_Stu_Temp.light_colour[RGB_B_BIT] == 255) return 1;
			if(Light_Stu_Temp.light_colour[RGB_R_BIT] == 255 && Light_Stu_Temp.light_colour[RGB_G_BIT] == 0   && Light_Stu_Temp.light_colour[RGB_B_BIT] == 0)   return 2;
			if(Light_Stu_Temp.light_colour[RGB_R_BIT] == 255 && Light_Stu_Temp.light_colour[RGB_G_BIT] == 165 && Light_Stu_Temp.light_colour[RGB_B_BIT] == 0)   return 3;
			if(Light_Stu_Temp.light_colour[RGB_R_BIT] == 255 && Light_Stu_Temp.light_colour[RGB_G_BIT] == 255 && Light_Stu_Temp.light_colour[RGB_B_BIT] == 0)   return 4;
			if(Light_Stu_Temp.light_colour[RGB_R_BIT] == 0   && Light_Stu_Temp.light_colour[RGB_G_BIT] == 255 && Light_Stu_Temp.light_colour[RGB_B_BIT] == 0)   return 5;
			if(Light_Stu_Temp.light_colour[RGB_R_BIT] == 0   && Light_Stu_Temp.light_colour[RGB_G_BIT] == 127 && Light_Stu_Temp.light_colour[RGB_B_BIT] == 255) return 6;
			if(Light_Stu_Temp.light_colour[RGB_R_BIT] == 0   && Light_Stu_Temp.light_colour[RGB_G_BIT] == 0   && Light_Stu_Temp.light_colour[RGB_B_BIT] == 255) return 7;
			if(Light_Stu_Temp.light_colour[RGB_R_BIT] == 139 && Light_Stu_Temp.light_colour[RGB_G_BIT] == 0   && Light_Stu_Temp.light_colour[RGB_B_BIT] == 255) return 8;
		}
		else if(system_config.flags.ubl_remote_change_color == UBL_THERAPY_COLOR_CONFIG)
		{
			if(Light_Stu_Temp.light_colour[RGB_R_BIT] == 0   && Light_Stu_Temp.light_colour[RGB_G_BIT] == 0   && Light_Stu_Temp.light_colour[RGB_B_BIT] == 0)   return 0;
			if(Light_Stu_Temp.light_colour[RGB_R_BIT] == 255 && Light_Stu_Temp.light_colour[RGB_G_BIT] == 140 && Light_Stu_Temp.light_colour[RGB_B_BIT] == 20)  return 1;
			if(Light_Stu_Temp.light_colour[RGB_R_BIT] == 255 && Light_Stu_Temp.light_colour[RGB_G_BIT] == 220 && Light_Stu_Temp.light_colour[RGB_B_BIT] == 120) return 2;
			if(Light_Stu_Temp.light_colour[RGB_R_BIT] == 255 && Light_Stu_Temp.light_colour[RGB_G_BIT] == 255 && Light_Stu_Temp.light_colour[RGB_B_BIT] == 255) return 3;
		}
		else
		{
			if(Light_Stu_Temp.light_colour[RGB_R_BIT] == 0   && Light_Stu_Temp.light_colour[RGB_G_BIT] == 0   && Light_Stu_Temp.light_colour[RGB_B_BIT] == 0)   return 0;
			if(Light_Stu_Temp.light_colour[RGB_R_BIT] == 255 && Light_Stu_Temp.light_colour[RGB_G_BIT] == 255 && Light_Stu_Temp.light_colour[RGB_B_BIT] == 255) return 1;
		}
	}

	return 0;
}
void Led_Rgb_OFF(void)
{
	if(system_config.flags.ubl_remote_change_color == 0)
	{
		Light_RgbColour_Stu.led_colour_state = 0;
	}
	else
	{
		Light_RgbColour_Stu.led_colour_state = Light_GetLastRgbStateByConfig() - 1;
	}
	Light_RgbColour_Stu.light_colour[RGB_R_BIT] = 0;Light_RgbColour_Stu.light_colour[RGB_G_BIT] = 0;Light_RgbColour_Stu.light_colour[RGB_B_BIT] = 0;//关闭
}

void Light_Clear_TimeCount(LIGHT_STRUCT *Light_Stu_Temp)
{
	//Light_Stu_Temp->led_time_sec_set = system_config.flags.light_one_default_time_sec;
	Light_Stu_Temp->led_ms_count = 0;
	Light_Stu_Temp->led_sec_count = 0;
}

void Light_Time_Set(LIGHT_STRUCT *Light_Stu_Temp,unsigned short set_time)
{
	Light_Stu_Temp->led_ms_count = 0;
	Light_Stu_Temp->led_sec_count = 0;
	
	Light_Stu_Temp->led_time_sec_set = set_time;
}


void Light_TimeManagerTask(void)
{
	//灯牌
	if(led_board_state != 0)
	{
		Led_Board_SW(1);
	}
	else
	{
		Led_Board_SW(0);
	}
	//单色床底灯
	if(0 == Light_OneColour_Stu.led_colour_state)
	{
		Led_OneColour_SW(0);
	}
	else
	{
		Led_OneColour_SW(1);
	}	
	
	//单色灯时间控制
	if(Light_OneColour_Stu.led_colour_state != 0)
	{
		if(Light_OneColour_Stu.led_time_sec_set > 0)  //设置时间大于0
		{
			Light_OneColour_Stu.led_ms_count	++;
			if(Light_OneColour_Stu.led_ms_count >= 1000/SYS_TIME_BASE) //毫秒
			{
				Light_OneColour_Stu.led_ms_count = 0;
				
				Light_OneColour_Stu.led_sec_count ++;
				
				if(Light_OneColour_Stu.led_sec_count >= Light_OneColour_Stu.led_time_sec_set) //默认30分钟 30*60
				{
					Light_OneColour_Stu.led_sec_count = Light_OneColour_Stu.led_time_sec_set;
				}
			}
		}
		else
		{
			Light_OneColour_Stu.led_sec_count = 0;
		}
	}
	else
	{
		Light_Clear_TimeCount(&Light_OneColour_Stu);
	}
	//三色灯时间控制
	if((((Light_RgbColour_Stu.light_colour[RGB_R_BIT] != 0 || Light_RgbColour_Stu.light_colour[RGB_G_BIT] != 0 || Light_RgbColour_Stu.light_colour[RGB_B_BIT] != 0))
		&& (Light_RgbColour_Stu.light_mode == LIGHT_CONTROL_RGB_MODE) ) || (Light_RgbColour_Stu.light_mode == LIGTH_COLOUR_RGB_MODE) || (Light_RgbColour_Stu.light_mode == LIGHT_BREATH_RGB_MODE))
	{
		if(Light_RgbColour_Stu.led_time_sec_set > 0)
		{
			Light_RgbColour_Stu.led_ms_count ++;
			if(Light_RgbColour_Stu.led_ms_count >= 1000/SYS_TIME_BASE) //毫秒
			{
				Light_RgbColour_Stu.led_ms_count = 0;
				
				Light_RgbColour_Stu.led_sec_count ++;
				
				if(Light_RgbColour_Stu.led_sec_count >= Light_RgbColour_Stu.led_time_sec_set) //默认30分钟 30*60
				{
					Light_RgbColour_Stu.led_sec_count = Light_RgbColour_Stu.led_time_sec_set;
				}
			}			
		}
		else
		{
			Light_RgbColour_Stu.led_sec_count = 0;
		}
	}
	else
	{
		Light_Clear_TimeCount(&Light_RgbColour_Stu);
	}

	if(rgb_gradient_run_flag == 1)
	{
		TTL_Master_Send_Write_Gradient_Colour_Cmd(LIGHT_RGB_DEVICE_TYPE,0x01,rgb_target_gradient_colour);
	}
	
}


unsigned char Get_Light_OneColour_State(void)
{
	return Light_OneColour_Stu.led_colour_state;
}

unsigned short Get_Light_Time_State(void)
{
	unsigned short led_remaining_time_min = 0;
	
	led_remaining_time_min = (Light_OneColour_Stu.led_time_sec_set - Light_OneColour_Stu.led_sec_count) / 60;
	
	if(((Light_OneColour_Stu.led_time_sec_set - Light_OneColour_Stu.led_sec_count) % 60) != 0)
	{
		led_remaining_time_min += 1;
	}
	
	return led_remaining_time_min;
}

void Periph_LedBoardPort_Reconfig(void)
{
	if(pc1_config_copy == UBL_BOARD)//上位机配置为可兼容灯牌和床底灯
	{
		Periph_PC1_Init(LED_BOARD);
	}
}

void Periph_UblPort_Reconfig(void)
{
	if(pc1_config_copy == UBL_BOARD)//上位机配置为可兼容灯牌和床底灯
	{
		if(Master_SearchIdleAdd(LIGHT_RGB_DEVICE_TYPE)!= 0)//检测RGB灯带是否存在
		{
			Periph_PC1_Init(LED_BOARD);
		}
		else
		{
			Periph_PC1_Init(UBL);
		}
	}
}
void Led_RgbColour_Hex_Set(unsigned char R_color,unsigned char G_color,unsigned char B_color)
{
		Light_RgbColour_Stu.light_colour[RGB_R_BIT] = R_color;
		Light_RgbColour_Stu.light_colour[RGB_G_BIT] = G_color;
		Light_RgbColour_Stu.light_colour[RGB_B_BIT] = B_color;
	
		light_para_set_event |= LIGHT_UBL_COLOUR_EVENT;			
}
unsigned char TTL_Start_Gradient_Colour(unsigned char device_code,unsigned char led_order, unsigned char *rgb_colour)  
{
	if(device_code == LIGHT_RGB_DEVICE_TYPE)
	{
		for(unsigned char i = 0; i < 3; i++)
		{
			rgb_target_gradient_colour[i] = rgb_colour[i];
		}
		rgb_gradient_run_flag = 1;
		led_gradient_step = 0;
	}
	return 1;
}	
unsigned char TTL_Master_Send_Write_Gradient_Colour_Cmd(unsigned char device_code,unsigned char led_order, unsigned char *rgb_colour)   
{
	unsigned char i,ttl_state = 0;
	
	static unsigned char dt_rgb_color[3];
	static unsigned char led_time_cont = 0;
	
	led_time_cont++;
	if(led_time_cont < 6)
	{
		return 0;
	}
	else
	{
		led_time_cont = 0;
	}
	if(device_code == LIGHT_RGB_DEVICE_TYPE)
	{
		for(i=0; i < 3; i++)		
		{
			if(led_gradient_step == 0)//第一次切换颜色，要对步长赋值，保证亮度改变，不改变颜色
			{
				if((rgb_colour[2] == 0) && (rgb_colour[1] == 0) && (rgb_colour[0] == 0))
				{
					dt_rgb_color[i] = 10;
				}
				else
				{
					dt_rgb_color[i] = abs(Light_RgbColour_Stu.gradient_light_colour[i] - rgb_colour[i])/30;		
				}
				if(dt_rgb_color[i] == 0)
				{
					dt_rgb_color[i] = 1;
				}
			}		
		}
		for(i=0; i < 3; i++)
		{
			if(abs(Light_RgbColour_Stu.gradient_light_colour[i] - rgb_colour[i]) < dt_rgb_color[i])//最后不足一步，则直接赋值并完成
			{
				Light_RgbColour_Stu.gradient_light_colour[i] = rgb_colour[i];
			}
			else 
			{
				led_gradient_step++;//一共xx步
				if(Light_RgbColour_Stu.gradient_light_colour[i] < rgb_colour[i])//判断是增加还是减少
				{
					Light_RgbColour_Stu.gradient_light_colour[i] += dt_rgb_color[i]; 
				}
				else
				{
					Light_RgbColour_Stu.gradient_light_colour[i] -= dt_rgb_color[i];
				}
			}
		}
		TTL_Master_Send_WriteColour_Cmd(device_code,led_order,Light_RgbColour_Stu.gradient_light_colour);
		if((Light_RgbColour_Stu.gradient_light_colour[0] == rgb_colour[0])
			&& (Light_RgbColour_Stu.gradient_light_colour[1] == rgb_colour[1])
			&& (Light_RgbColour_Stu.gradient_light_colour[2] == rgb_colour[2]))
		{
			led_gradient_step = 0;
			rgb_gradient_run_flag = 0;
			ttl_state = 1;
		}		
	}

	return ttl_state;
}

unsigned char User_LightDemo(unsigned char step)
{
	unsigned char run_state = 0;
	unsigned char sleep_time[2] = {0x02,0x58};//{0,60};//{0x02,0x58};
	switch(step)
	{
		case 0:
		{
			Led_Rgb_OFF();
			Light_RgbColour_Stu.light_mode = LIGHT_CONTROL_RGB_MODE;
			Light_RgbColour_Stu.led_colour_state = 0;
			Led_RgbColour_Set(Light_RgbColour_Stu.led_colour_state);
			
			/*------------------------------TTL设备控制-------------------------------------*/
//			if(1 == TTL_Master_Send_WriteColour_Cmd(LIGHT_RGB_DEVICE_TYPE,0x01,Light_RgbColour_Stu.light_colour))	  //RGB灯	
//			{
//				light_para_set_event &= ~LIGHT_UBL_OFF_EVENT;
//				run_state = 1;
//				ble_report_event |= REPORT_RGB_STATE_CHANGE_EVENT;
//			}	
			if(system_config.flags.light_rgb_gradient_color == UBL_GRADIENT_COLOR_ENABLE)
			{
				if(1 == TTL_Start_Gradient_Colour(LIGHT_RGB_DEVICE_TYPE,0x01,Light_RgbColour_Stu.light_colour))	  //RGB灯	
				{
					run_state = 1;	
				}		
			}	
		  else
			{
				if(1 == TTL_Master_Send_WriteColour_Cmd(LIGHT_RGB_DEVICE_TYPE,0x01,Light_RgbColour_Stu.light_colour))
				{
					run_state = 1;	
				}	
			}	
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_RGB_STATE_CHANGE_EVENT);
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_RGB_MODE_CHANGE_EVENT);
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_RGB_COLOR_CHANGE_EVENT);	
		}
		break;
		case 1:
		{
			Light_Clear_TimeCount(&Light_RgbColour_Stu);
//			Light_RgbColour_Stu.led_colour_state = demo_light_save;
//			Led_RgbColour_Set(Light_RgbColour_Stu.led_colour_state);
			Light_RgbColour_Stu.light_colour[RGB_R_BIT] = demo_light_save[0];
			Light_RgbColour_Stu.light_colour[RGB_G_BIT] = demo_light_save[1];
			Light_RgbColour_Stu.light_colour[RGB_B_BIT] = demo_light_save[2];
			Light_RgbColour_Stu.light_mode = LIGHT_CONTROL_RGB_MODE;

			if(system_config.flags.light_rgb_gradient_color == UBL_GRADIENT_COLOR_ENABLE)
			{
				if(1 == TTL_Start_Gradient_Colour(LIGHT_RGB_DEVICE_TYPE,0x01,Light_RgbColour_Stu.light_colour))	  //RGB灯	
				{
					run_state = 1;	
				}		
			}	
		  else
			{
				if(1 == TTL_Master_Send_WriteColour_Cmd(LIGHT_RGB_DEVICE_TYPE,0x01,Light_RgbColour_Stu.light_colour))
				{
					run_state = 1;	
				}	
			}
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_RGB_STATE_CHANGE_EVENT);
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_RGB_MODE_CHANGE_EVENT);
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_RGB_COLOR_CHANGE_EVENT);			
		}
		break;
		case 2://开灯
		{
			Light_Clear_TimeCount(&Light_RgbColour_Stu);
			Light_RgbColour_Stu.led_colour_state = Light_GetLastRgbStateByConfig();
			if(Light_RgbColour_Stu.led_colour_state != 0) //颜色状态
			{
				Light_SaveLastRgbStateByConfig(Light_RgbColour_Stu.led_colour_state);
			}
			Light_RgbColour_Stu.light_mode = LIGHT_CONTROL_RGB_MODE;
			Led_RgbColour_Set(Light_RgbColour_Stu.led_colour_state);
			Led_RgbColour_Hex_Set(Light_RgbColour_Stu.light_colour[RGB_R_BIT]*0.2,Light_RgbColour_Stu.light_colour[RGB_G_BIT]*0.2,Light_RgbColour_Stu.light_colour[RGB_B_BIT]*0.2);
			if(system_config.flags.light_rgb_gradient_color == UBL_GRADIENT_COLOR_ENABLE)
			{
				if(1 == TTL_Start_Gradient_Colour(LIGHT_RGB_DEVICE_TYPE,0x01,Light_RgbColour_Stu.light_colour))	  //RGB灯	
				{
					run_state = 1;	
				}		
			}	
		  else
			{
				if(1 == TTL_Master_Send_WriteColour_Cmd(LIGHT_RGB_DEVICE_TYPE,0x01,Light_RgbColour_Stu.light_colour))
				{
					run_state = 1;	
				}	
			}
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_RGB_STATE_CHANGE_EVENT);
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_RGB_MODE_CHANGE_EVENT);
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_RGB_COLOR_CHANGE_EVENT);				
		}break;
		case 3://亮度2
		{
			Light_Clear_TimeCount(&Light_RgbColour_Stu);
			Light_RgbColour_Stu.led_colour_state = Light_GetLastRgbStateByConfig();
			if(Light_RgbColour_Stu.led_colour_state != 0) //颜色状态
			{
				Light_SaveLastRgbStateByConfig(Light_RgbColour_Stu.led_colour_state);
			}
			Light_RgbColour_Stu.light_mode = LIGHT_CONTROL_RGB_MODE;
			Led_RgbColour_Set(Light_RgbColour_Stu.led_colour_state);
			Led_RgbColour_Hex_Set(Light_RgbColour_Stu.light_colour[RGB_R_BIT]*0.5,Light_RgbColour_Stu.light_colour[RGB_G_BIT]*0.5,Light_RgbColour_Stu.light_colour[RGB_B_BIT]*0.5);
			if(system_config.flags.light_rgb_gradient_color == UBL_GRADIENT_COLOR_ENABLE)
			{
				if(1 == TTL_Start_Gradient_Colour(LIGHT_RGB_DEVICE_TYPE,0x01,Light_RgbColour_Stu.light_colour))	  //RGB灯	
				{
					run_state = 1;	
				}		
			}	
		  else
			{
				if(1 == TTL_Master_Send_WriteColour_Cmd(LIGHT_RGB_DEVICE_TYPE,0x01,Light_RgbColour_Stu.light_colour))
				{
					run_state = 1;	
				}	
			}
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_RGB_STATE_CHANGE_EVENT);
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_RGB_MODE_CHANGE_EVENT);
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_RGB_COLOR_CHANGE_EVENT);				
		}break;
		case 4://亮度3
		{
			Light_Clear_TimeCount(&Light_RgbColour_Stu);
			Light_RgbColour_Stu.led_colour_state = Light_GetLastRgbStateByConfig();
			if(Light_RgbColour_Stu.led_colour_state != 0) //颜色状态
			{
				Light_SaveLastRgbStateByConfig(Light_RgbColour_Stu.led_colour_state);
			}
			Light_RgbColour_Stu.light_mode = LIGHT_CONTROL_RGB_MODE;
			Led_RgbColour_Set(Light_RgbColour_Stu.led_colour_state);
			Led_RgbColour_Hex_Set(Light_RgbColour_Stu.light_colour[RGB_R_BIT],Light_RgbColour_Stu.light_colour[RGB_G_BIT],Light_RgbColour_Stu.light_colour[RGB_B_BIT]);
			if(system_config.flags.light_rgb_gradient_color == UBL_GRADIENT_COLOR_ENABLE)
			{
				if(1 == TTL_Start_Gradient_Colour(LIGHT_RGB_DEVICE_TYPE,0x01,Light_RgbColour_Stu.light_colour))	  //RGB灯	
				{
					run_state = 1;	
				}		
			}	
		  else
			{
				if(1 == TTL_Master_Send_WriteColour_Cmd(LIGHT_RGB_DEVICE_TYPE,0x01,Light_RgbColour_Stu.light_colour))
				{
					run_state = 1;	
				}	
			}
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_RGB_STATE_CHANGE_EVENT);
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_RGB_MODE_CHANGE_EVENT);
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_RGB_COLOR_CHANGE_EVENT);				
		}break;		
	}
	return run_state;	
}

