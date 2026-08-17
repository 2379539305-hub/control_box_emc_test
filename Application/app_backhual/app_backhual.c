/*---------------应用层------------------*/
#include "app_backhual.h"
#include "app_music.h"
#include "app_linbus.h"
#include "app_ble.h"
#include "app_ttlbus.h"
#include "app_msgr.h"
#include "app_motor.h"
#include "app_save.h"
#include "app_config.h"
#include "app_comm.h"
#include "app_vita.h"
#include "app_light.h"
#include "app_fan.h"
/*---------------模块---------------------*/
#include "modul_ttlbus.h"
#include "modul_motor.h"
#include "modul_a7105.h"
/*---------------外设驱动-----------------*/
#include "driver_key.h"
#include "driver_beep.h"
#include "driver_ble.h"
#include "driver_a7105.h"

#include "delay.h"

unsigned char ack_buff_0xb0[5] = {0x6e,0x90,0xb0,0x00,0x00};
unsigned char ack_buff_0xd1[5] = {0x6e,0x90,0xd1,0x00,0x00};
unsigned char ack_buff_0xd2[5] = {0x6e,0x90,0xd2,0x00,0x00};
unsigned char ack_buff[30] = {0};
unsigned ack_event_states[MAX_EVENTS / 8] = {0};
unsigned report_event_states[MAX_EVENTS / 8] = {0};
//等待回传间隔
static unsigned char sys_wait_ack_time = 0;  

//上报间隔
static unsigned char sys_state_updata_time = 0;  


//状态回传定时器中断函数，用于计时回传间隔
void SYS_UpData_TimeManagerTask(void)
{
	sys_state_updata_time ++;
	if(sys_state_updata_time >= SYS_STATE_UPDATA_TIME)
	{
		sys_state_updata_time = SYS_STATE_UPDATA_TIME;
	}
	sys_wait_ack_time ++;
	if(sys_wait_ack_time >= SYS_WAIT_ACK_TIME)
	{
		sys_wait_ack_time = SYS_WAIT_ACK_TIME;
	}
}
// 校验和函数
unsigned char calculate_ack_checksum(const unsigned char *udata, unsigned char length)
{
  unsigned char i = 0;
  unsigned char sum = 0;
  for(i = 0; i < length; i++)
  {
    sum += udata[i];
  }
  return sum;
}
// 设置事件
void ble_report_set_event(unsigned char type, unsigned short event) 
{
    if (event < MAX_EVENTS) 
	{
        if (type == BLE_ACK_EVENT) // ACK event
		{
            ack_event_states[event / 8] |= (1 << (event % 8));
        }
		else if (type == BLE_REPORT_EVENT) // Report event
		{
            report_event_states[event / 8] |= (1 << (event % 8));
        }
    }
}
// 检查事件
bool ble_report_check_event(unsigned char type, unsigned short event) 
{
    if (event >= MAX_EVENTS) 
	{
		return false;
	}
	if (type == BLE_ACK_EVENT) // ACK event
	{
		return (ack_event_states[event / 8] & (1 << (event % 8))) != 0;
	}
	else if (type == BLE_REPORT_EVENT) // Report event
	{
		return (report_event_states[event / 8] & (1 << (event % 8))) != 0;
	}
	return false;
}
// 清除事件
void ble_report_clear_event(unsigned char type, unsigned short event) 
{
	if (event < MAX_EVENTS) 
	{
		if (type == BLE_ACK_EVENT) // ACK event
		{
			ack_event_states[event / 8] &= ~(1 << (event % 8));
		}
		else if (type == BLE_REPORT_EVENT) // Report event
		{
			report_event_states[event / 8] &= ~(1 << (event % 8));
		}
	}
}
void bluetooth_ask(unsigned char func,unsigned char prop,unsigned char *payload,unsigned char length)
{
	unsigned char temp_buff[100];
	unsigned char payload_length = length;
	unsigned char i;
	temp_buff[0] = 0x6e;
	temp_buff[1] = 0x20;
	temp_buff[2] = payload_length + 7;
	temp_buff[3] = func;
	temp_buff[4] = prop;
	
	temp_buff[5] = 0x02;
	
	for(i = 0 ; i < length ; i ++)
	{
		temp_buff[6+i] = payload[i];
	}
	temp_buff[payload_length + 7 - 1] = calculate_checksum(temp_buff,payload_length + 7 - 1);
	BleBlueTooth_SendString(temp_buff, payload_length + 7);
}

void bluetooth_report(unsigned char func,unsigned char prop,unsigned char *payload,unsigned char length)
{
	unsigned char temp_buff[100];
	unsigned char payload_length = length;
	unsigned char i;
	temp_buff[0] = 0x6e;
	temp_buff[1] = 0x20;
	temp_buff[2] = payload_length + 7;
	temp_buff[3] = func;
	temp_buff[4] = prop;
	
	temp_buff[5] = 0x03;
	
	for(i = 0 ; i < length ; i ++)
	{
		temp_buff[6+i] = payload[i];
	}
	temp_buff[payload_length + 7 - 1] = calculate_checksum(temp_buff,payload_length + 7 - 1);
	BleBlueTooth_SendString(temp_buff, payload_length + 7);
}
//report 0-回复问询，1，主动上报
void bluetooth_report_rgb_state(unsigned char report_type)
{
	unsigned char payload[10];
	payload[0] = 0;
	if(Light_RgbColour_Stu.light_mode == LIGHT_CONTROL_RGB_MODE)
	{
		if((Light_RgbColour_Stu.light_colour[RGB_R_BIT] == 0x00 && Light_RgbColour_Stu.light_colour[RGB_G_BIT] == 0X00 && Light_RgbColour_Stu.light_colour[RGB_B_BIT] == 0X00)
		&& (Light_RgbColour_Stu.light_mode == LIGHT_CONTROL_RGB_MODE))		
		{
			payload[1] = 0;
		}
		else
		{
			payload[1] = 1;
		}
	}
	else
	{
		payload[1] = 1;
	}	
	if(report_type == 0)
	{
		bluetooth_ask(0x04, 0x04, payload, 2);
	}
	else
	{
		bluetooth_report(0x04, 0x04, payload, 2);
	}
}
void bluetooth_report_borad_state(unsigned char report_type)
{
	unsigned char payload[10];
	payload[0] = 0;	
	payload[1] = led_board_state;
		
	if(report_type == 0)
	{
		bluetooth_ask(0x03, 0x03, payload, 2);
	}
	else
	{
		bluetooth_report(0x03, 0x03, payload, 2);
	}	
}
void bluetooth_report_rgb_time(unsigned char report_type)
{
	unsigned char payload[10];
	payload[0] = 0;	
	payload[1] = Light_RgbColour_Stu.led_time_sec_set/60/256;
	payload[2] = Light_RgbColour_Stu.led_time_sec_set/60%256;
		
	if(report_type == 0)
	{
		bluetooth_ask(0x04, 0x02, payload, 3);
	}
	else
	{
		bluetooth_report(0x04, 0x02, payload, 3);
	}	
}
void bluetooth_report_rgb_color(unsigned char report_type)
{
	unsigned char payload[10];
	payload[0] = 0;
	payload[1] = Light_RgbColour_Stu.light_colour[RGB_R_BIT];
	payload[2] = Light_RgbColour_Stu.light_colour[RGB_G_BIT];
	payload[3] = Light_RgbColour_Stu.light_colour[RGB_B_BIT];	
	
	if(report_type == 0)
	{
		bluetooth_ask(0x04, 0x01, payload, 4);	
	}
	else
	{
		bluetooth_report(0x04, 0x01, payload, 4);	
	}		
}
void bluetooth_report_rgb_mode(unsigned char report_type)
{
	unsigned char payload[10];
	payload[0] = 0;	
	if(Light_RgbColour_Stu.light_mode == LIGTH_COLOUR_RGB_MODE)
	{
		payload[1] = 1;
	}
	else if(Light_RgbColour_Stu.light_mode == LIGTH_MUSIC_RGB_MODE)
	{
		payload[1] = 2;
	}
	else
	{
		payload[1] = 0;
	}
	payload[2] = Light_RgbColour_Stu.light_colour_mode[MODE_ORDER_BIT];
		
	if(report_type == 0)
	{
		bluetooth_ask(0x04, 0x05, payload, 3);
	}
	else
	{
		bluetooth_report(0x04, 0x05, payload, 3);
	}	
}
void bluetooth_report_rgb_breath_mode(unsigned char report_type)
{
	unsigned char payload[10];
	payload[0] = 0;
	if(Light_RgbColour_Stu.light_mode == LIGHT_BREATH_RGB_MODE)
	{
		payload[1] = Light_RgbColour_Stu.light_breath_time[0];
		payload[2] = Light_RgbColour_Stu.light_breath_time[1];
		payload[3] = Light_RgbColour_Stu.light_colour[RGB_R_BIT];
		payload[4] = Light_RgbColour_Stu.light_colour[RGB_G_BIT];
		payload[5] = Light_RgbColour_Stu.light_colour[RGB_B_BIT];	
	
	
		if(report_type == 0)
		{
			bluetooth_ask(0x04, 0x06, payload, 6);	
		}
		else
		{
			bluetooth_report(0x04, 0x06, payload, 6);	
		}	
	}
}
void bluetooth_report_rgb_brightness(unsigned char report_type)
{
	unsigned char payload[10];
	payload[0] = 0;	
	payload[1] = Light_RgbColour_Stu.light_brightness;
		
	if(report_type == 0)
	{
		bluetooth_ask(0x04, 0x08, payload, 2);
	}
	else
	{
		bluetooth_report(0x04, 0x08, payload, 2);
	}	
}
void bluetooth_report_massage_mode(unsigned char report_type)
{
	uint8_t payload[10];
	payload[0] = 0x00;
	
	if((Msgr_Ints_FlagArr[1] == 0) && (Msgr_Ints_FlagArr[2] == 0) && (Msgr_Ints_FlagArr[3] == 0)) 
	{
		payload[1] = 0;
	}
	else
	{
		if(msgr_mode_set == MSGR_FOLLOW_MODE)
		{
			payload[1] = 0;
		}
		else if(msgr_mode_set == MSGR_CONSTANT_MODE)
		{
			payload[1] = 1;
		}
		else if(msgr_mode_set == MSGR_PULSE_MODE)
		{
			payload[1] = 3;
		}
		else if(msgr_mode_set == MSGR_WAVE_MODE)
		{
			payload[1] = 2;
		}
		else
		{
			payload[1] = 0;
		}
	}
	if(report_type == 0)
	{
		bluetooth_ask(0x02,0x02,payload,2);
	}
	else
	{
		bluetooth_report(0x02,0x02,payload,2);
	}		
}
void bluetooth_report_massage_inits(unsigned char report_type)
{
	uint8_t payload[10];
	payload[0] = 0x01;
	payload[1] = Msgr_Ints_FlagArr[1];
	payload[2] = 0x02;
	payload[3] = Msgr_Ints_FlagArr[2];
	payload[4] = 0x03;
	payload[5] = Msgr_Ints_FlagArr[3];
	
	if(report_type == 0)
	{
		bluetooth_ask(0x02,0x01,payload,6);
	}
	else
	{
		bluetooth_report(0x02,0x01,payload,6);
	}		
}
void bluetooth_report_massage_state(unsigned char report_type)
{
	uint8_t payload[10];
	payload[0] = 0x00;
	if(Msgr_Ints_FlagArr[0] != 0 || Msgr_Ints_FlagArr[1] != 0 || Msgr_Ints_FlagArr[2] != 0 || Msgr_Ints_FlagArr[3] != 0)
	{
		payload[1] = 1;
	}
	else 
	{
		payload[1] = 0;
	}

	if(report_type == 0)
	{
		bluetooth_ask(0x02,0x06,payload,2);
	}
	else
	{
		bluetooth_report(0x02,0x06,payload,2);
	}		
}
void bluetooth_report_massage_time(unsigned char report_type)
{
	uint8_t payload[10];
	payload[0] = 0x01;
	payload[1] = msgr_min_time_set/256;
	payload[2] = msgr_min_time_set%256;

	if(report_type == 0)
	{
		bluetooth_ask(0x02,0x03,payload,3);
	}
	else
	{
		bluetooth_report(0x02,0x03,payload,3);
	}			
}
void bluetooth_report_music_sw(unsigned char report_type)
{
	uint8_t payload[10];
	payload[0] = 0x00;
	payload[1] = MusicalOsc_Stu.BlueTooth_State[BLUETOOTH_SW_STATE];
	if(report_type == 0)
	{
		bluetooth_ask(0x09,0x03,payload,2);
	}
	else
	{
		bluetooth_report(0x09,0x03,payload,2);
	}		
}
void bluetooth_report_music_play(unsigned char report_type)
{
	uint8_t payload[10];
	payload[0] = 0x00;
	payload[1] = MusicalOsc_Stu.Music_PlayState;

	if(report_type == 0)
	{
		bluetooth_ask(0x09,0x01,payload,2);
	}
	else
	{
		bluetooth_report(0x09,0x01,payload,2);
	}			
}
void bluetooth_report_music_follow_inits(unsigned char report_type)
{
	uint8_t payload[10];
	payload[0] = 0x00;
	payload[1] = Msgr_Ints_FlagArr[0];

	if(report_type == 0)
	{
		bluetooth_ask(0x02,0x05,payload,2);
	}
	else
	{
		bluetooth_report(0x02,0x05,payload,2);
	}			
}
void bluetooth_report_music_demo(unsigned char report_type)
{
	uint8_t payload[10];
	payload[0] = 0x00;
	if(MusicalOsc_Stu.SysMode_State == MUSIC_SOURCE_BLUE)
	{
		payload[1] = 0;
		if(report_type == 0)
		{
			bluetooth_ask(0x09,0x04,payload,2);	
		}
		else
		{
			bluetooth_report(0x09,0x04,payload,2);	
		}			
	}
	else 
	{
		if(MusicalOsc_Stu.DemoMode_Source == MUSIC_DEMO_SOURCE_USB)
		{
			payload[1] = 1;
			if(report_type == 0)
			{
				bluetooth_ask(0x09,0x04,payload,2);	
			}
			else
			{
				bluetooth_report(0x09,0x04,payload,2);	
			}				
		}
		else if(MusicalOsc_Stu.DemoMode_Source == MUSIC_DEMO_SOURCE_WHITE_NOISE)
		{
			payload[1] = 2;
			payload[2] = MusicalOsc_Stu.DemoMode_TrackState[0];
			if(report_type == 0)
			{
				bluetooth_ask(0x09,0x04,payload,3);	
			}
			else
			{
				bluetooth_report(0x09,0x04,payload,3);	
			}				
		}
		else if(MusicalOsc_Stu.DemoMode_Source == MUSIC_DEMO_SOURCE_MUSIC)
		{
			payload[1] = 3;
			payload[2] = MusicalOsc_Stu.DemoMode_TrackState[1];
			if(report_type == 0)
			{
				bluetooth_ask(0x09,0x04,payload,3);	
			}
			else
			{
				bluetooth_report(0x09,0x04,payload,3);	
			}				
		}
		else
		{
			payload[1] = 4;
			payload[2] = 0;
			if(report_type == 0)
			{
				bluetooth_ask(0x09,0x04,payload,3);	
			}
			else
			{
				bluetooth_report(0x09,0x04,payload,3);	
			}			

		}		
	}	
}
void bluetooth_report_ubl_state(unsigned char report_type)
{
	uint8_t payload[10];
	payload[0] = 0x00;
	if(Light_OneColour_Stu.led_colour_state)
	{
		payload[1] = 1;
	}
	else 
	{
		payload[1] = 0;
	}
	
	if(report_type == 0)
	{
		bluetooth_ask(0x03,0x01,payload,2);	
	}
	else
	{
		bluetooth_report(0x03,0x01,payload,2);		
	}		
}
void bluetooth_report_vol_set(unsigned char report_type)
{
	uint8_t payload[10];
	payload[0] = 0x02;
	payload[1] = MusicalOsc_Stu.MusicVolume_Level;

	if(report_type == 0)
	{
		bluetooth_ask(0x09,0x02,payload,2);	
	}
	else
	{
		bluetooth_report(0x09,0x02,payload,2);	
	}			
}
void bluetooth_report_fan_mode(unsigned char report_type)
{
	uint8_t payload[10];
	payload[0] = 0x00;
	payload[1] = Fan_Stu.fan_mode_set;

	if((Fan_Stu.Fan_Ints_FlagArr[1] == 0) && (Fan_Stu.Fan_Ints_FlagArr[2] == 0) 
		&& (Fan_Stu.Fan_Ints_FlagArr[3] == 0) && (Fan_Stu.Fan_Ints_FlagArr[4] == 0)) 
	{
		payload[1] = 0; 
	}
	else
	{
		if(Fan_Stu.fan_mode_set == FAN_CONSTANT_MODE)
		{
			payload[1] = 1;
		}
		else if(Fan_Stu.fan_mode_set == FAN_PULSE_MODE)
		{
			payload[1] = 3;
		}
		else if(Fan_Stu.fan_mode_set == FAN_WAVE_MODE)
		{
			payload[1] = 2;
		}
		else
		{
			payload[1] = 0;
		}	
	}
	
	if(report_type == 0)
	{
		bluetooth_ask(0x0e,0x02,payload,2);	
	}
	else
	{
		bluetooth_report(0x0e,0x02,payload,2);	
	}			
}
void bluetooth_report_fan_ints(unsigned char report_type)
{
	uint8_t payload[10];
	payload[0] = 0x01;
	payload[1] = Fan_Stu.Fan_Ints_FlagArr[1];
	payload[2] = 0x02;
	payload[3] = Fan_Stu.Fan_Ints_FlagArr[3];
	
	if(report_type == 0)
	{
		bluetooth_ask(0x0e,0x01,payload,4);
	}
	else
	{
		bluetooth_report(0x0e,0x01,payload,4);
	}	
}
void bluetooth_report_fan_time(unsigned char report_type)
{
	uint8_t payload[10];
	payload[0] = 0x00;
	payload[1] = Fan_Stu.fan_time_min_set/256;
	payload[2] = Fan_Stu.fan_time_min_set%256;

	if(report_type == 0)
	{
		bluetooth_ask(0x0e,0x03,payload,3);
	}
	else
	{
		bluetooth_report(0x0e,0x03,payload,3);
	}	
}
void bluetooth_report_fan_dir(unsigned char report_type)
{
	uint8_t payload[10];
	payload[0] = 0x01;
	payload[2] = 0x02;
	if((Fan_Stu.Fan_Ints_FlagArr[1] == 0) && (Fan_Stu.Fan_Ints_FlagArr[3] == 0))
	{
		payload[1] = 0;
		payload[3] = 0;
	}
	else
	{
		if(Fan_Stu.Fan_Dir_FlagArr[1] == FAN_FORWARD)
		{
			payload[1] = 1;
			payload[3] = 1;
		}
		else
		{
			payload[1] = 2;
			payload[3] = 2;			
		}
	}

	
	if(report_type == 0)
	{
		bluetooth_ask(0x0e,0x04,payload,4);
	}
	else
	{
		bluetooth_report(0x0e,0x04,payload,4);
	}	
}
void bluetooth_report_motor_cmd_state(unsigned char report_type)
{
	uint8_t payload[10];
	if(((0 == Get_Motor_PortState())) && ((GetSet_Motor_Ctr_Cmd(0xff) == KEY_ALL_STOP) || (GetSet_Motor_Ctr_Cmd(0xff) == KEY_MOTOR_STOP)))
	{
		payload[0] = 0;
	}
	else
	{
		payload[0] = GetSet_Motor_Ctr_Cmd(0xff);
	}

	if(report_type == 0)
	{
		bluetooth_ask(0x01,0x04,payload,1);
	}
	else
	{
		bluetooth_report(0x01,0x04,payload,1);
	}		
}

void bluetooth_report_sync_mode_state(unsigned char report_type)
{
	uint8_t payload[10];
	payload[0] = 1;
	if(Get_Sync_Run_Mode() == 0)
	{
		payload[1] = 1;
	}
	else
	{
		payload[1] = 0;
	}

	if(report_type == 0)
	{
		bluetooth_ask(0x0b,0x02,payload,2);
	}
	else
	{
		bluetooth_report(0x0b,0x02,payload,2);
	}		
}
void bluetooth_report_demo_sleep_time(unsigned char report_type)
{
	uint8_t payload[10];
	payload[0] = 0x00;
	payload[1] = demo_run_time/256;
	payload[2] = demo_run_time%256;

	if(report_type == 0)
	{
		bluetooth_ask(0x0f,0x02,payload,3);
	}
	else
	{
		bluetooth_report(0x0f,0x02,payload,3);
	}			
}
void bluetooth_report_sleep_mode_demo(unsigned char report_type)
{
	uint8_t payload[10];
	payload[0] = 0x01;
	if(KEY_DEMO1_MODE == GetSet_Motor_Ctr_Cmd(0xff))
	{
		payload[1] = 1;
	}
	else
	{
		payload[1] = 0;
	}
	payload[2] = 0x02;
	if((KEY_DEMO2_MODE == GetSet_Motor_Ctr_Cmd(0xff)) || ((KEY_DEMO1_MODE == GetSet_Motor_Ctr_Cmd(0xff)) )) 
	{
		payload[3] = 1;
	}
	else
	{
		payload[3] = 0;
	}	
	if(report_type == 0)
	{
		bluetooth_ask(0x0f,0x01,payload,4);
	}
	else
	{
		bluetooth_report(0x0f,0x01,payload,4);
	}	
}
void bluetooth_report_vita_addr(unsigned char report_type)
{
	uint8_t payload[20];
	payload[0] = 0x00;
	payload[1] =  Vita_Ble_RadioName[0];
	payload[2] =  Vita_Ble_RadioName[1];
	payload[3] =  Vita_Ble_RadioName[2];
	payload[4] = Vita_Ble_RadioName[3];
	payload[5] = Vita_Ble_RadioName[4];
	payload[6] = Vita_Ble_RadioName[5];
	payload[7] = Vita_Ble_RadioName[6];
	payload[8] = Vita_Ble_RadioName[7];
	payload[9] = Vita_Ble_RadioName[8];	
			
	bluetooth_ask(0x06,0x03,payload,10);
}
void bluetooth_report_rgb_device_state(unsigned char report_type)
{
	unsigned char ack_buff[5];	
	if(Master_SearchIdleAdd(LIGHT_RGB_DEVICE_TYPE) != 0) //检测RGB等待是否存在
	{
		ack_buff[0] = 0x6E;ack_buff[1] = 0x0E;ack_buff[2] = 0x00;ack_buff[3] = 0x03;ack_buff[4] = 0x7F;
		BleBlueTooth_SendString(ack_buff, 5);	
	}		
}
void bluetooth_report_alarm_have(unsigned char report_type)
{
	unsigned char ack_buff[5];	
	if(system_config.flags.alarm_enable != 0)//检测闹钟等待是否存在
	{
		ack_buff[0] = 0x6E;ack_buff[1] = 0x09;ack_buff[2] = 0x01;ack_buff[3] = 0x00;ack_buff[4] = 0x78;
		BleBlueTooth_SendString(ack_buff, 5);	
	}		
}
void bluetooth_report_alarm_set(unsigned char report_type)
{
	unsigned char ack_buff[5];
	ack_buff[0] = 0x6E;ack_buff[1] = 0x07;ack_buff[2] = 0x01;ack_buff[3] = 0x01;ack_buff[4] = 0x77;
	BleBlueTooth_SendString(ack_buff, 5);			
}
void bluetooth_report_alarm_clear(unsigned char report_type)
{
	unsigned char ack_buff[5];
	ack_buff[0] = 0x6E;ack_buff[1] = 0x07;ack_buff[2] = 0x01;ack_buff[3] = 0x02;ack_buff[4] = 0x78;
	BleBlueTooth_SendString(ack_buff, 5);		
}
void bluetooth_report_music_have(unsigned char report_type)
{
	unsigned char ack_buff[5];
	if(Master_SearchIdleAdd(MUSIC_DEVICE_TYPE) != 0) //检测音乐阵子是否存在
	{
		ack_buff[0] = 0x6E;ack_buff[1] = 0x16;ack_buff[2] = 0x01;ack_buff[3] = 0x01;ack_buff[4] = 0x86;
		BleBlueTooth_SendString(ack_buff, 5);	
		musicmsgr_device_online = 1;
	}
}

void TTL_Ack_Music(void)
{
	/*-----------------------------------音乐阵子状态问询-----------------------------------------*/
	if(Master_SearchIdleAdd(0x12) != 0)
	{
		if(musicmsgr_device_online != 0)
		{
			musicmsgr_device_online = 0;
			music_device_ack_flag = 0;
			sys_wait_ack_time = 0;
			musicmsgr_status_check |= MUSIC_BLE_SW_CHECK;
			musicmsgr_status_check |= MUSIC_PLAY_CHECK;
			musicmsgr_status_check |= MUSIC_VOLUME_CHECK;
			musicmsgr_status_check |= MUSIC_FOLLOW_INTS_CHECK;
			musicmsgr_status_check |= MUSIC_DEMO_CHECK;
		}

		if((musicmsgr_status_check & MUSIC_BLE_SW_CHECK) == MUSIC_BLE_SW_CHECK)  //蓝牙开关状态
		{
			if(0 == music_device_ack_flag)
			{
				TTL_User_AskDevice_Status(0x12,0x29,0x01,0x00); //蓝牙开关状态
				music_device_ack_flag = 1;
				sys_wait_ack_time = 0;
			}
			if((music_state_updata_event & MUSIC_BLE_ON_EVENT) == MUSIC_BLE_ON_EVENT || (music_state_updata_event & MUSIC_BLE_OFF_EVENT) == MUSIC_BLE_OFF_EVENT)
			{
				music_device_ack_flag = 0;
				musicmsgr_status_check &= ~MUSIC_BLE_SW_CHECK;
			}					
		}
		else if((musicmsgr_status_check & MUSIC_PLAY_CHECK) == MUSIC_PLAY_CHECK)  //问询播放状态
		{
			if(0 == music_device_ack_flag)
			{
				TTL_User_AskDevice_Status(0x12,0x27,0x00,0x00); //音乐播放状态
				
				music_device_ack_flag = 1;
				sys_wait_ack_time = 0;				
			}
			if((music_state_updata_event & MUSIC_PLAY_EVENT) == MUSIC_PLAY_EVENT || (music_state_updata_event & MUSIC_PAUSE_EVENT) == MUSIC_PAUSE_EVENT)
			{
				music_device_ack_flag = 0;
				musicmsgr_status_check &= ~MUSIC_PLAY_CHECK;
			}
		}			
		else if((musicmsgr_status_check & MUSIC_VOLUME_CHECK) == MUSIC_VOLUME_CHECK)  //蓝牙音量状态
		{
			if(0 == music_device_ack_flag)
			{
				
				TTL_User_AskDevice_Status(0x12,0x25,0x00,0x00); 
				
				music_device_ack_flag = 1;
				sys_wait_ack_time = 0;
			}
			if((music_state_updata_event & MUSIC_VOL_SET_EVENT) == MUSIC_VOL_SET_EVENT)
			{
				music_device_ack_flag = 0;
				musicmsgr_status_check &= ~MUSIC_VOLUME_CHECK;
			}					
		}
		else if((musicmsgr_status_check & MUSIC_MSGR_MODE_CHECK) == MUSIC_MSGR_MODE_CHECK) //问询阵子模式
		{
			if(0 == music_device_ack_flag)
			{
				TTL_User_AskDevice_Status(0x12,0x21,0x00,0x00); //阵子模式
				
				music_device_ack_flag = 1;
				sys_wait_ack_time = 0;				
			}
			if((msgr_state_updata_event & MSGR_MODE_TIME_EVENT) == MSGR_MODE_TIME_EVENT)
			{
				music_device_ack_flag = 0;
				musicmsgr_status_check &= ~MUSIC_MSGR_MODE_CHECK;
				
				if(0 == msgr_mode_set)  //如果是随振模式  就再问一下随振强度
				{
					musicmsgr_status_check |= MUSIC_FOLLOW_INTS_CHECK;
				}
			}
		}		
		else if((musicmsgr_status_check & MUSIC_FOLLOW_INTS_CHECK) == MUSIC_FOLLOW_INTS_CHECK) //问询随振强度
		{
			if(0 == music_device_ack_flag)
			{
				TTL_User_AskDevice_Status(0x12,0x22,0x00,0x00); //随振强度
				
				music_device_ack_flag = 1;
				sys_wait_ack_time = 0;				
			}
			if((msgr_state_updata_event & MSGR_FOLLOW_INTS_EVENT) == MSGR_FOLLOW_INTS_EVENT)
			{
				music_device_ack_flag = 0;
				musicmsgr_status_check &= ~MUSIC_FOLLOW_INTS_CHECK;
			}
		}
		else if((musicmsgr_status_check & MUSIC_DEMO_CHECK) == MUSIC_DEMO_CHECK) //问询演示模式
		{
			if(0 == music_device_ack_flag)
			{
				TTL_User_AskDevice_Status(0x12,0x2B,0x00,0x00); //演示模式
				
				music_device_ack_flag = 1;
				sys_wait_ack_time = 0;				
			}
			if((music_state_updata_event & MUSIC_DEMO_ON_EVENT) == MUSIC_DEMO_ON_EVENT || (music_state_updata_event & MUSIC_DEMO_OFF_EVENT) == MUSIC_DEMO_OFF_EVENT)
			{
				music_device_ack_flag = 0;
				musicmsgr_status_check &= ~MUSIC_DEMO_CHECK;
				
				musicmsgr_device_online = 0;
			}
		}	
	}
	if(sys_wait_ack_time >= SYS_WAIT_ACK_TIME)  //等待回传超时
	{
		if(music_device_ack_flag != 0)
		{
			musicmsgr_status_check = 0;
			music_device_ack_flag = 0;
		}
	}
}

// ACK事件处理表
const ReportEventEntry ack_event_table[] = {
    {ACK_RGB_HAVE_EVENT, bluetooth_report_rgb_device_state},
    {ACK_ALARM_HAVE_EVENT, bluetooth_report_alarm_have},
    {ACK_ALARM_SET_EVENT, bluetooth_report_alarm_set},
    {ACK_ALARM_CLEAR_EVENT, bluetooth_report_alarm_clear},
    {ACK_MUSIC_HAVE_EVENT, bluetooth_report_music_have},
    {ACK_RGB_STATE_CHANGE_EVENT, bluetooth_report_rgb_state},
    {ACK_RGB_COLOR_CHANGE_EVENT, bluetooth_report_rgb_color},
    {ACK_RGB_TIME_CHANGE_EVENT, bluetooth_report_rgb_time},
    {ACK_RGB_MODE_CHANGE_EVENT, bluetooth_report_rgb_mode},
    {ACK_RGB_BREATH_MODE_EVENT, bluetooth_report_rgb_breath_mode},
    {ACK_RGB_BRIGHTNESS_EVENT, bluetooth_report_rgb_brightness},
    {ACK_MASSAGE_MODE_EVENT, bluetooth_report_massage_mode},
    {ACK_MASSAGE_INTS_EVENT, bluetooth_report_massage_inits},
    {ACK_MASSAGE_TIME_EVENT, bluetooth_report_massage_time},
    {ACK_MUSIC_DEMO_EVENT, bluetooth_report_music_demo},
    {ACK_MUSIC_BLE_EVENT, bluetooth_report_music_sw},
    {ACK_MUSIC_PLAY_EVENT, bluetooth_report_music_play},
    {ACK_MUSIC_FOLLOW_INITS_EVENT, bluetooth_report_music_follow_inits},
    {ACK_MUSIC_VOL_SET_EVENT, bluetooth_report_vol_set},
    {ACK_FAN_MODE_EVENT, bluetooth_report_fan_mode},
    {ACK_FAN_INTS_EVENT, bluetooth_report_fan_ints},
    {ACK_FAN_TIME_EVENT, bluetooth_report_fan_time},
    {ACK_FAN_DIR_EVENT, bluetooth_report_fan_dir},
		{ACK_BORAD_STATE_EVENT, bluetooth_report_borad_state},
		{ACK_MOTOR_CMD_EVENT, bluetooth_report_motor_cmd_state},
		{ACK_SYNC_MODE_EVENT, bluetooth_report_sync_mode_state},
		{ACK_DEMO_SLEEP_TIME_EVENT, bluetooth_report_demo_sleep_time},
		{ACK_DEMO_RUN_EVENT, bluetooth_report_sleep_mode_demo},
		{ACK_REPORT_VITA_ADDR, bluetooth_report_vita_addr},
    // 可继续添加
};
const int ack_event_table_size = sizeof(ack_event_table) / sizeof(ack_event_table[0]);

// REPORT事件处理表
const ReportEventEntry report_event_table[] = {
    {REPORT_RGB_STATE_CHANGE_EVENT, bluetooth_report_rgb_state},
    {REPORT_RGB_COLOR_CHANGE_EVENT, bluetooth_report_rgb_color},
    {REPORT_RGB_TIME_CHANGE_EVENT, bluetooth_report_rgb_time},
    {REPORT_RGB_MODE_CHANGE_EVENT, bluetooth_report_rgb_mode},
    {REPORT_RGB_BREATH_MODE_EVENT, bluetooth_report_rgb_breath_mode},
    {REPORT_RGB_BRIGHTNESS_EVENT, bluetooth_report_rgb_brightness},
    {REPORT_MASSAGE_MODE_EVENT, bluetooth_report_massage_mode},
    {REPORT_MASSAGE_INTS_EVENT, bluetooth_report_massage_inits},
    {REPORT_MASSAGE_TIME_EVENT, bluetooth_report_massage_time},
    {REPORT_MASSAGE_STATE_EVENT, bluetooth_report_massage_state},
    {REPORT_MUSIC_DEMO_EVENT, bluetooth_report_music_demo},
    {REPORT_MUSIC_BLE_EVENT, bluetooth_report_music_sw},
    {REPORT_MUSIC_PLAY_EVENT, bluetooth_report_music_play},
    {REPORT_MUSIC_FOLLOW_INITS_EVENT, bluetooth_report_music_follow_inits},
    {REPORT_MUSIC_VOL_SET_EVENT, bluetooth_report_vol_set},
    {REPORT_FAN_MODE_EVENT, bluetooth_report_fan_mode},
    {REPORT_FAN_INTS_EVENT, bluetooth_report_fan_ints},
    {REPORT_FAN_TIME_EVENT, bluetooth_report_fan_time},
    {REPORT_FAN_DIR_EVENT, bluetooth_report_fan_dir},
		{REPORT_BORAD_STATE_EVENT, bluetooth_report_borad_state},
		{REPORT_MOTOR_CMD_EVENT, bluetooth_report_motor_cmd_state},
		{REPORT_SYNC_MODE_EVENT, bluetooth_report_sync_mode_state},
		{REPORT_DEMO_SLEEP_TIME_EVENT, bluetooth_report_demo_sleep_time},
		{REPORT_DEMO_RUN_EVENT, bluetooth_report_sleep_mode_demo},
    // 可继续添加
};
const int report_event_table_size = sizeof(report_event_table) / sizeof(report_event_table[0]);

void Ble_Report(void)
{
	static unsigned char report_event_flag = 0;
	unsigned char ack_buff[30] = {0};
	unsigned char i;	
/*-------------------------------------------状态上报----------------------------------------------------*/
	
	/*--------------------------------老协议上报（不用，改新协议）---------------------------------------*/
	if(music_state_updata_event != 0)
	{
		//0xB事件
		//蓝牙开启
		if((music_state_updata_event & MUSIC_BLE_ON_EVENT) == MUSIC_BLE_ON_EVENT)
		{
			ack_buff_0xb0[3] |= 0X20;
			report_event_flag |= 0x01;
			music_state_updata_event &= ~MUSIC_BLE_ON_EVENT;
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_MUSIC_BLE_EVENT);
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_MUSIC_VOL_SET_EVENT);		
		}
		//蓝牙关闭
		if((music_state_updata_event & MUSIC_BLE_OFF_EVENT) == MUSIC_BLE_OFF_EVENT)
		{
			ack_buff_0xb0[3] &= 0xdf;
			report_event_flag |= 0x01;
			music_state_updata_event &= ~MUSIC_BLE_OFF_EVENT;
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_MUSIC_BLE_EVENT);
		}
		//音乐播放  
		if((music_state_updata_event & MUSIC_PLAY_EVENT) == MUSIC_PLAY_EVENT)
		{
			ack_buff_0xb0[3] |= 0x80;
			report_event_flag |= 0x01;
			music_state_updata_event &= ~MUSIC_PLAY_EVENT;
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_MUSIC_PLAY_EVENT);	
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_MUSIC_VOL_SET_EVENT);		
		}
		//音乐暂停
		if((music_state_updata_event & MUSIC_PAUSE_EVENT) == MUSIC_PAUSE_EVENT)
		{
			ack_buff_0xb0[3] &= 0x7f; 
			report_event_flag |= 0x01;  
			music_state_updata_event &= ~MUSIC_PAUSE_EVENT;		
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_MUSIC_PLAY_EVENT);			
		}
		//演示模式开
		if((music_state_updata_event & MUSIC_DEMO_ON_EVENT) == MUSIC_DEMO_ON_EVENT)
		{
			ack_buff_0xb0[2] |= 0X01;  
			report_event_flag |= 0x01;
			music_state_updata_event &= ~MUSIC_DEMO_ON_EVENT;
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_MUSIC_DEMO_EVENT);			
		}
		//演示模式关
		if((music_state_updata_event & MUSIC_DEMO_OFF_EVENT) == MUSIC_DEMO_OFF_EVENT)
		{
			ack_buff_0xb0[2] &= 0xbe; 
			report_event_flag |= 0x01;
			music_state_updata_event &= ~MUSIC_DEMO_OFF_EVENT;		
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_MUSIC_DEMO_EVENT);			
		}
		//音乐播放源
		if((music_state_updata_event & MUSIC_DEMO_SET_TRACK_EVENT) == MUSIC_DEMO_SET_TRACK_EVENT)
		{
			music_state_updata_event &= ~MUSIC_DEMO_SET_TRACK_EVENT;
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_MUSIC_DEMO_EVENT);	
		}
		//随震开启
		if((music_state_updata_event & MUSIC_SHOCK_ON_EVENT) == MUSIC_SHOCK_ON_EVENT)
		{
			ack_buff_0xb0[3] |= 0X40; 
			report_event_flag |= 0x01;
			music_state_updata_event &= ~MUSIC_SHOCK_ON_EVENT;
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_MASSAGE_INTS_EVENT);
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_MUSIC_FOLLOW_INITS_EVENT);
		}
		//随震关闭
		if((music_state_updata_event & MUSIC_SHOCK_OFF_EVENT) == MUSIC_SHOCK_OFF_EVENT)
		{
			ack_buff_0xb0[3] &= 0XBF; 
			report_event_flag |= 0x01;
			music_state_updata_event &= ~MUSIC_SHOCK_OFF_EVENT;
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_MASSAGE_INTS_EVENT);
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_MUSIC_FOLLOW_INITS_EVENT);
		}
		//0XD1音量  
		if((music_state_updata_event & MUSIC_VOL_SET_EVENT) == MUSIC_VOL_SET_EVENT)
		{
			ack_buff_0xd1[2] = 0xd1;
			ack_buff_0xd1[3] = MusicalOsc_Stu.MusicVolume_Level;  

			report_event_flag |= 0x02;
			music_state_updata_event &= ~MUSIC_VOL_SET_EVENT;		
			ble_report_set_event(BLE_REPORT_EVENT, REPORT_MUSIC_VOL_SET_EVENT);			
		}
	}
	//0XD2随振强度
	if((msgr_state_updata_event & MSGR_FOLLOW_INTS_EVENT) == MSGR_FOLLOW_INTS_EVENT)
	{
		ack_buff_0xd2[2] = 0xd2;
		ack_buff_0xd2[3] = Msgr_Ints_FlagArr[0];
		if(Msgr_Ints_FlagArr[0] == 0)
		{
			ack_buff_0xb0[3] &= 0XBF;
		}
		else
		{
			ack_buff_0xb0[3] |= 0X40;
			report_event_flag |= 0x04;	
		}  

		report_event_flag |= 0x01;
		msgr_state_updata_event &= ~MSGR_FOLLOW_INTS_EVENT;
		ble_report_set_event(BLE_REPORT_EVENT, REPORT_MASSAGE_INTS_EVENT);
		ble_report_set_event(BLE_REPORT_EVENT, REPORT_MUSIC_FOLLOW_INITS_EVENT);
			
	}	
	//-------------BLE统一管理上传--------------------------------------------------------
	if(sys_state_updata_time >= SYS_STATE_UPDATA_TIME )//&& 1 == Ble_Comm_Free())
	{
		sys_state_updata_time = 0;
		//老协议状态上报
//		if(report_event_flag != 0)
//		{
//			//音响状态
//			if((report_event_flag & 0x01) == 0x01)
//			{
//				report_event_flag &= 0xfe;
//				ack_buff_0xb0[4] = (unsigned char)(ack_buff_0xb0[0] + ack_buff_0xb0[1] + ack_buff_0xb0[2] + ack_buff_0xb0[3]);
//				BleBlueTooth_SendString(ack_buff_0xb0, 5);        
//				return;
//			}
//			//0XD1音量  
//			if((report_event_flag & 0x02) == 0x02)
//			{
//				report_event_flag &= 0xfd;
//				ack_buff_0xd1[4] = (unsigned char)(ack_buff_0xd1[0] + ack_buff_0xd1[1] + ack_buff_0xd1[2] + ack_buff_0xd1[3]);
//				BleBlueTooth_SendString(ack_buff_0xd1, 5);        
//				return;        
//			}
//			//0XD2随振强度
//			if((report_event_flag & 0x04) == 0x04)
//			{
//				report_event_flag &= 0xfb;
//				ack_buff_0xd2[4] = (unsigned char)(ack_buff_0xd2[0] + ack_buff_0xd2[1] + ack_buff_0xd2[2] + ack_buff_0xd2[3]);
//				BleBlueTooth_SendString(ack_buff_0xd2, 5);
//				return;
//			}
//		}
			// 蓝牙主动问询（ACK事件）
			for (int i = 0; i < ack_event_table_size; ++i) 
			{
				if (ble_report_check_event(BLE_ACK_EVENT, ack_event_table[i].event_id))
				{               
					ble_report_clear_event(BLE_ACK_EVENT, ack_event_table[i].event_id);
					ack_event_table[i].handler(BLE_ACK_EVENT); // 0表示ACK
					return;
				}
			}

			// 控制盒主动上报（REPORT事件）
			for (int i = 0; i < report_event_table_size; ++i) 
			{
				if (ble_report_check_event(BLE_REPORT_EVENT, report_event_table[i].event_id)) 
				{
					ble_report_clear_event(BLE_REPORT_EVENT, report_event_table[i].event_id);
					report_event_table[i].handler(BLE_REPORT_EVENT); // 1表示主动上报
					return;
				}
			}
	}		
}
void SYS_UpData_Show(void)
{
	TTL_Ack_Music();
	Ble_Report();
}

