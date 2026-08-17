#ifndef __MODUL_LIGHT_H
#define __MODUL_LIGHT_H

#include "main.h"
#include "system.h"


extern u32 light_event_flag;

unsigned char TTL_Master_Send_WriteColour_Cmd(unsigned char device_code,unsigned char led_order,unsigned char* rgb_colour);
unsigned char TTL_Master_Send_WriteRgbMode_Cmd(unsigned char led_order,unsigned char mode_type,unsigned char mode_order);
unsigned char TTL_Master_Send_WriteRgbBreathMode_Cmd(unsigned char led_order, unsigned char *breath_freq, unsigned char r_color,unsigned char g_color,unsigned char b_color);   
unsigned char TTL_Master_Send_WriteRgbSleepMode_Cmd(unsigned char led_order, unsigned char *sleep_time, unsigned char r_color,unsigned char g_color,unsigned char b_color) ;  
unsigned char TTL_Master_Send_WriteRgbBrightness_Cmd(unsigned char led_order, unsigned char brightness);
unsigned char TTL_Master_Send_WriteColour_And_Brightness_Cmd(unsigned char device_code,unsigned char led_order,unsigned char *rgb_colour,unsigned char brightness);
#endif






