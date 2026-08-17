#ifndef __MODUL_MUSICMSGR_H
#define __MODUL_MUSICMSGR_H

#include "main.h"
#include "system.h"


extern u32 musicmsgr_event_flag;


typedef struct
{
	//按摩模式、时间设置
	unsigned char Msgr_Mode_State;
	unsigned char Msgr_Timer_State;
	//随振模式振动强度
	unsigned char MsgrFollowMode_IntState;
	//典型模式振动强度选择
	unsigned char MsgrTypicalMode_IntsState[3];
	//按摩速度参数
	unsigned char MsgrTypical_WaveSpeedState[8]; //波浪模式
	unsigned char MsgrTypical_PulseSpeedState[8]; //脉冲模式
	//音乐音量
	unsigned char MusicVolume_Level;  //初始化默认12  用户直接拿这个变量控制音量即可，加减操作在协议内部已经处理
	//音乐曲目控制
	unsigned char Music_TrackState; 
	//音乐播放控制
	unsigned char Music_PlayState; //00暂停状态 01播放状态   用户也需要根据自身情况更新此状态(比如用音乐播放软件暂停了,也要更新)
	//音源通道选择
	unsigned char Music_SrcChannelSelet; 
	//蓝牙相关控制
#define BLUETOOTH_SW_STATE   1
#define BLUETOOTH_TWS_STATE  2
#define BLUETOOTH_LIST  3
#define BLUETOOTH_CONNECT_STATE  4
	unsigned char BlueTooth_State[5]; //0 无效 1 蓝牙开关  2 TWS  3 清组队列表	4蓝牙连接状态  用户也需要根据自身情况更新此状态
	//阵子功率状态
	unsigned char MsgrPower_State; 
	//系统工作模式
	unsigned char SysMode_State;
	//演示模式播放源
	unsigned char DemoMode_Source; //0-USB,1-白噪音,2-demo音乐 MUSIC_DEMO_MODE_SOURCE
	//演示模式曲目
	unsigned char DemoMode_TrackState[2];//0-白噪音曲目，1-demo音乐曲目 MUSIC_TRACK_LIST
	//电机随动开关
	unsigned char MotorFollowMode_State; //0-关闭 1-开启
}MUSIC_OSC_STRUCT;


//系统开关状态
typedef enum 
{
	SWITCH_STATE_OFF = 0,
	SWITCH_STATE_ON = 1,
}SWITCH_STATE;

//蓝牙控制指令
typedef enum 
{
	SWITCH_CTR_OFF = 0,
	SWITCH_CTR_ON = 1,
	SWITCH_CTR_ON_OFF = 2,
	SWITCH_MUTE_OFF = 0x10,
	SWITCH_MUTE_ON = 0x11,
	SWITCH_MUTE_ON_OFF = 0x12,	
}SWITCH_CMD;

//按摩模式选择
typedef enum 
{
  MUSIC_FOLLOW_MODE = 0,
	
  MSGR_CONTINUE_25HZMODE = 1,
	MSGR_CONTINUE_30HZMODE = 2,
	MSGR_CONTINUE_35HZMODE = 3,
	MSGR_CONTINUE_40HZMODE = 4,

  MSGR_PULSE_25HZMODE = 5,
	MSGR_PULSE_30HZMODE = 6,
	MSGR_PULSE_35HZMODE = 7,
	MSGR_PULSE_40HZMODE = 8,

  MSGR_WAVE_25HZMODE = 9,
	MSGR_WAVE_30HZMODE = 10,
	MSGR_WAVE_35HZMODE = 11,
	MSGR_WAVE_40HZMODE = 12,	
	
}MUSIC_MSGR_MODE;

//音量设置
typedef enum 
{
	MUSIC_VOLUME_SET = 0,
	MUSIC_VOLUME_DCR = 1,
	MUSIC_VOLUME_ADD = 2,
}MUSIC_VOLUME_CMD;

//曲目控制
typedef enum 
{
	MUSIC_TRACK_PRE = 1,
	MUSIC_TRACK_NEXT = 2,
	
}MUSIC_TRACK_CMD;

//音源选择
typedef enum 
{
	MUSIC_SOURCE_BLUE = 0,
	MUSIC_SOURCE_AUX = 1,
	MUSIC_SOURCE_U = 2,
}MUSIC_SOURCE_CMD;

//演示模式播放源
typedef enum
{
	MUSIC_DEMO_SOURCE_USB = 0,
	MUSIC_DEMO_SOURCE_WHITE_NOISE = 1,
	MUSIC_DEMO_SOURCE_MUSIC = 2,
	MUSIC_DEMO_SOURCE_CLOCK = 3,
}MUSIC_DEMO_MODE_SOURCE;

//音乐播放列表
typedef enum
{
	MUSIC_TRACK_NOMAL = 0,
	MUSIC_TRACK_1 = 1,
	MUSIC_TRACK_2 = 2,
	MUSIC_TRACK_3 = 3,
	MUSIC_TRACK_4 = 4,
	MUSIC_TRACK_5 = 5,
	MUSIC_TRACK_6 = 6,
}MUSIC_TRACK_LIST;

//蓝牙控制指令
typedef enum 
{
	MUSIC_BLUE_SW = 1,
	MUSIC_TWS_SW = 2,
	MUSIC_BLUE_LIST = 3,
}MUSIC_BLUE_CMD;

#endif


/*
//按摩器模式设置
//序号现在默认写0XFF
//模式:随动  持续35HZ  脉冲35HZ  波浪35HZ
*/
unsigned char TTL_Master_Send_WriteMsgrModeTimer_Cmd(unsigned char msgr_order,unsigned char msgr_mode_temp , unsigned char msgr_time_temp);
/*
//随振模式强度设置
//序号现在默认写00
//强度 0 1 2 3
*/
unsigned char TTL_Master_Send_WriteMsgrFollowInts_Cmd(unsigned char msgr_order,unsigned char msgr_ints_temp);
/*
//序号 1头部 2脚部   00全部
//强度 0 1 2 3
//时间
*/
unsigned char TTL_Master_Send_WriteMsgrTypicalInts_Cmd(unsigned char msgr1_ints,unsigned char msgr2_ints);
/*
//典型模式速度参数
//模式 1波浪 2脉冲
//时间 空闲时间   持续时间   波浪上升时间  波浪下降时间 ms
*/
unsigned char TTL_Master_Send_WriteMsgrTypicalSpeed_Cmd(unsigned char msgr_mode,unsigned short idle_time_temp,unsigned short keep_time_temp,unsigned short rise_time_temp,unsigned short desc_time_temp);
/*
//音量设置
//volume_cmd_temp 0设置   1音量-1   2音量+1
//音量大小
*/
unsigned char TTL_Master_Send_WriteMusicVolume_Cmd(MUSIC_VOLUME_CMD volume_cmd_temp,unsigned char volume_dat_temp);
/*
//曲目控制
//track_cmd_temp 1上一曲  2下一曲
*/
unsigned char TTL_Master_Send_WriteMusicTrack_Cmd(MUSIC_TRACK_CMD track_cmd_temp);
/*
//播放
//play_cmd_temp 0暂停  1播放  2暂停/播放
*/
unsigned char TTL_Master_Send_WriteMusicPlay_Cmd(SWITCH_CMD play_cmd_temp);
/*
//音源选择
//src_temp 0蓝牙  1AUX  2U盘
*/
unsigned char TTL_Master_Send_WriteMusicSource_Cmd(MUSIC_SOURCE_CMD src_temp);
/*
//demo播放源
//source 0usb  1白噪音  2demo
//track  xx首
*/
unsigned char TTL_Master_Send_WriteMusicDemo_Source(unsigned char source,unsigned char track);
/*
//蓝牙控制
//ble_func_temp 1蓝牙开关 2TWS开关  3列表操作
//开关 0关  1开  2开关
*/
unsigned char TTL_Master_Send_WriteBuleTooth_Cmd(MUSIC_BLUE_CMD ble_func_temp,SWITCH_CMD ble_dat_temp);
/*
//功率模式
//power_temp 0正常功率  1低功率
*/
unsigned char TTL_Master_Send_WriteMsgrPower_Cmd(unsigned char power_temp);
/*
//系统工作模式
//sys_mode_temp 0正常  1演示  2OTA
*/
unsigned char TTL_Master_Send_WriteMusicSysMode_Cmd(unsigned char sys_mode_temp);
/*
//查询
//
*/
unsigned char TTL_MusicMsgr_Read_Cmd(unsigned char device_add_temp,unsigned char device_func_code,unsigned char extend_byte_temp);



unsigned char TTL_Master_WhiteNoiseSend_WriteMusicVolume_Cmd(MUSIC_VOLUME_CMD volume_cmd_temp,unsigned char volume_dat_temp);
unsigned char TTL_Master_WhiteNoiseSend_WriteMusicTrack_Cmd(MUSIC_TRACK_CMD track_cmd_temp);
unsigned char TTL_Master_WhiteNoiseSend_WriteMusicPlay_Cmd(SWITCH_CMD play_cmd_temp);
unsigned char TTL_Master_WhiteNoiseSend_SetMusicTrack_Cmd(unsigned char track_temp);
unsigned char TTL_Master_Send_WriteMotorSysMode_Cmd(unsigned char motor_sys_mode_temp);

