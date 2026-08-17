#ifndef __APP_LIGHT_H
#define __APP_LIGHT_H

#include "main.h"
#include "system.h"

#include "modul_light.h"

extern unsigned char light_type_flag;

//参数设置事件
//BIT7:1 UBL事件  BIT7:0 灯牌事件
#define LIGHT_UBL_SW_EVENT       (0X0001)
#define LIGHT_UBL_OFF_EVENT      (0X0002)
#define LIGHT_UBL_COLOUR_EVENT   (0X0004)
#define LIGHT_UBL_TIME_EVENT     (0X0010)
#define LIGHT_BOARD_STATE_EVENT  (0X0020)
#define LIGHT_UBL_ON_EVENT  	 	 (0X0040)
#define LIGHT_UBL_MODE_EVENT   		(0X0080)
#define LIGHT_UBL_BREATH_MODE_EVENT   		(0X0100)
#define LIGHT_UBL_SLEEP_MODE_EVENT   			(0X0200)
#define LIGHT_UBL_BRIGHTNESS_EVENT   			(0X0200)
extern unsigned short light_para_set_event; 

typedef enum
{
	LIGHT_CONTROL_RGB_MODE = 0,
	LIGTH_COLOUR_RGB_MODE = 1,
	LIGTH_MUSIC_RGB_MODE = 2,
	LIGHT_BREATH_RGB_MODE = 3,
	LIGHT_SLEEP_RGB_MODE = 4
}LIGHT_COLOUR_MODE;
typedef struct
{
	unsigned short led_ms_count ;  //毫秒计时
	unsigned long  led_sec_count ; //秒计时
	unsigned long  led_time_sec_set ;  
	
	unsigned char light_sensor_signal;
	
	unsigned char led_colour_state;
#define RGB_R_BIT  0
#define RGB_G_BIT  1	
#define RGB_B_BIT  2	
#define MODE_TYPE_BIT  0	
#define MODE_ORDER_BIT  1	
	unsigned char light_colour_mode[2];	//炫彩，律动
	unsigned char light_colour[3];//rgb
	unsigned char gradient_light_colour[3];
	unsigned char light_breath_time[2];//呼吸时间 s
	LIGHT_COLOUR_MODE light_mode;
	unsigned char light_brightness;//0-100%
}LIGHT_STRUCT;

extern LIGHT_STRUCT  Light_RgbColour_Stu;
extern LIGHT_STRUCT  Light_OneColour_Stu;
extern unsigned char led_board_state;

void APP_LightInit(void);

unsigned char Led_RgbColour_GetState(LIGHT_STRUCT Light_Stu_Temp);

void Light_Control(void);
void Light_Clear_TimeCount(LIGHT_STRUCT *Light_Stu_Temp);
void Light_Time_Set(LIGHT_STRUCT *Light_Stu_Temp,unsigned short set_time);
void Light_TimeManagerTask(void);
void Led_RgbColour_Hex_Set(unsigned char R_color,unsigned char G_color,unsigned char B_color);
unsigned char TTL_Master_Send_Write_Gradient_Colour_Cmd(unsigned char device_code,unsigned char led_order, unsigned char *rgb_colour);

unsigned char Get_Light_OneColour_State(void);
unsigned short Get_Light_Time_State(void);
unsigned char User_LightDemo(unsigned char step);
#endif
