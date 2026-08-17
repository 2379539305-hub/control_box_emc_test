#ifndef __APP_LINBUS_H
#define __APP_LINBUS_H

#include "main.h"
#include "system.h"

#include "modul_musicmsgr.h"

#define LIN_FREE_LONG_TIME (300/SYS_TIME_BASE)  //判定总线是否空闲

#define LIN_BROKEN_FRAME_TIME  (50/SYS_TIME_BASE) //断帧

#define LIN_PARA_START  (9)
#define LIN_HEADER_ONE  0XFA
#define LIN_HEADER_TWO  0X5A

#define LIN_SLAVE_TYPE_BIT   (4)
#define LIN_SLAVE_ADD_BIT    (5)
#define LIN_SECRET_KEY_BIT   (6)
#define LIN_CODE_BIT         (7)
#define LIN_CMD_BIT          (8)


extern unsigned short lin_key_value;

#define LIN_BUF_LENGTH    40
extern unsigned char LIN_BUS_RXBuffer[LIN_BUF_LENGTH]; 
extern unsigned char LIN_BUS_TXBuffer[LIN_BUF_LENGTH];

void LIN_Bus_RxServer(unsigned char uart_recv_value);
void LIN_Bus_TimeManager(void);
unsigned char LIN_Check_Busy_Free(void);

unsigned char Lin_Get_KeyState(void);
unsigned short Lin_Analy_KeyValue(void);



void LIN_Master_Send_lockstate_Cmd(unsigned short key_value_temp);
void LIN_Master_Send_WriteKeyValue_Cmd(unsigned short key_value_temp);
void LIN_Master_Send_WriteMotorHall_Cmd(unsigned char motor_num,unsigned char* motor_hall);

void LIN_Master_Send_WriteMsgrTypicalInts_Cmd(unsigned char msgr1_ints,unsigned char msgr2_ints,unsigned char msgr3_ints);
void LIN_Master_Send_WriteMsgrFollowInts_Cmd(unsigned char msgr_order,unsigned char msgr_ints_temp);
void LIN_Master_Send_WriteMsgrModeTimer_Cmd(unsigned char msgr_order,unsigned char msgr_mode_temp,unsigned char msgr_time_temp);


void LIN_Master_Send_WriteColour_Cmd(unsigned char device_code,unsigned char led_order,unsigned char* rgb_colour);
void LIN_Master_Send_WriteUBLTime_Cmd(unsigned char led_order,unsigned short time_sec_temp);

void LIN_Master_Send_DemoRunTime_Cmd(unsigned short time_min_temp);
void LIN_Master_Send_AlarmRun_Cmd(unsigned char motor_run_temp,unsigned char msgr_run_temp, unsigned char ubl_run_temp, unsigned char music_temp);
void LIN_Master_Send_DemoStep_Cmd(unsigned char demo_num, char step_temp);

void LIN_Master_Send_WriteMusicVolume_Cmd(MUSIC_VOLUME_CMD volume_cmd_temp,unsigned char volume_dat_temp);
void LIN_Master_Send_WriteMusicTrack_Cmd(MUSIC_TRACK_CMD track_cmd_temp);
void LIN_Master_Send_WriteMusicPlay_Cmd(SWITCH_CMD play_cmd_temp);
void LIN_Master_Send_WriteBuleTooth_Cmd(MUSIC_BLUE_CMD ble_func_temp,SWITCH_CMD ble_dat_temp);
void LIN_Master_Send_WriteMusicSysMode_Cmd(unsigned char sys_mode_temp);
void LIN_Master_Send_WriteMusicDemo_Source(unsigned char source,unsigned char track);   
void LIN_Master_Send_WriteFanInts_Cmd(unsigned char *fan_ints,unsigned char *fan_dir)  ;
void LIN_Master_Send_WriteFanModeTime_Cmd(unsigned char msgr_order,unsigned char msgr_mode_temp,unsigned short msgr_time_temp)   ;

void LIN_Master_Send_WriteHeatSwitch(unsigned char hear_switch);

void LIN_Master_Send_WriteRgbMode_Cmd(unsigned char led_order,unsigned char mode_type,unsigned char mode_order);
void LIN_Master_Send_WriteRgbBreathMode_Cmd(unsigned char led_order, unsigned char *breath_freq, unsigned char r_color,unsigned char g_color,unsigned char b_color) ;
void LIN_Master_Send_WriteRgbBrightness_Cmd(unsigned char led_order, unsigned char brightness);
#endif






