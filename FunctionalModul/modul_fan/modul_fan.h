#ifndef __MODUL_FAN_H
#define __MODUL_FAN_H

#include "main.h"
#include "system.h"

unsigned char TTL_Master_Send_WriteFanModeTime_Cmd(unsigned char fan_order , unsigned char fan_mode_temp , unsigned short fan_time_temp);
unsigned char TTL_Master_Send_WriteFanInts_Cmd(unsigned char *fan_ints,unsigned char *fan_dir);
unsigned char TTL_Master_Send_WtireHeatSwitch_Cmd(unsigned char heat_order,unsigned char heat_switch);
#endif





