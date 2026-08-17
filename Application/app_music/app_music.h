#ifndef __APP_MUSIC_H
#define __APP_MUSIC_H

#include "main.h"
#include "system.h"

#include "modul_musicmsgr.h"


//控制、回传事件
#define MUSIC_BLE_ON_EVENT          (0X00000001)
#define MUSIC_BLE_OFF_EVENT         (0X00000002)
#define MUSIC_BLE_CONNECT_EVENT     (0X00000004)
#define MUSIC_BLE_DISCONNECT_EVENT  (0X00000008)

#define MUSIC_TWS_ON_EVENT          (0X00000010)
#define MUSIC_TWS_OFF_EVENT         (0X00000020)

#define MUSIC_DEMO_ON_EVENT         (0X00000040)
#define MUSIC_DEMO_OFF_EVENT        (0X00000080)

#define MUSIC_PLAY_EVENT            (0X00000100)
#define MUSIC_PAUSE_EVENT           (0X00000200)
#define MUSIC_PRE_EVENT             (0X00000400)
#define MUSIC_NEXT_EVENT            (0X00000800)
#define MUSIC_VOL_ADD_EVENT         (0X00001000)
#define MUSIC_VOL_DCR_EVENT         (0X00002000)
#define MUSIC_VOL_SET_EVENT         (0X00004000)
#define MUSIC_SHOCK_ON_EVENT        (0X00008000)
#define MUSIC_SHOCK_OFF_EVENT       (0X00010000)
#define MUSIC_DEMO_SET_TRACK_EVENT	(0X00020000)
#define MUSIC_DEMO_SAVE_EVENT				(0X00040000)
#define MUSIC_MUTE_OFF_EVENT				(0X00080000)
#define MUSIC_FOLLOW_MODE_SW_EVENT	(0X00100000)
extern unsigned long music_para_set_event;  //音乐同步状态事件
extern unsigned long music_state_updata_event; //回传状态显示

//问询事件
#define MUSIC_BLE_SW_CHECK      (0x0001)
#define MUSIC_PLAY_CHECK        (0x0002)
#define MUSIC_VOLUME_CHECK      (0x0004)
#define MUSIC_MSGR_MODE_CHECK   (0x0008)
#define MUSIC_FOLLOW_INTS_CHECK (0x0010)
#define MUSIC_DEMO_CHECK        (0x0020)
extern unsigned char musicmsgr_device_online;

extern unsigned short musicmsgr_status_check;
extern unsigned char music_device_ack_flag;
extern unsigned char demo_light_save[];
extern unsigned char demo_volume_save;
extern unsigned char demo_run_time;
extern unsigned char demo_run_time_save;
extern MUSIC_OSC_STRUCT  MusicalOsc_Stu; 

void Music_Control(void);

void User_SetMusic_Volume(unsigned char vol_temp);

unsigned char Music_AcceptCmd_KeyInfo(unsigned char key_temp);
unsigned char User_MusicDemo(unsigned char step);
void Music_TimeManagerTask(void);
void Music_Time_ClearCount(void);
#endif
