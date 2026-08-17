#include "modul_musicmsgr.h"

#include "modul_ttlbus.h"

unsigned long musicmsgr_event_flag = 0;

#define MUSIC_MSGR_DEVICE  (0X12)
#define MUSIC_WHITE_NOISE_DEVICE  (0X16)

static unsigned char MUSIC_TXBuffer[25];

unsigned char TTL_MusicMsgr_Write_Cmd(unsigned char device_funccode_temp , unsigned char extend_data_length)   
{
	unsigned char temp_i = 0;
	unsigned short sum_check = 0;
	unsigned char bus_send_length = 10 + extend_data_length;

	if(0 == TTL_IsDeviceOnline(MUSIC_MSGR_DEVICE)) return 1;
	
	if(RingBuffer.ring_write_data_lock != 0) return 0;
	RingBuffer.ring_write_data_lock = 1;		

	if(TTL_RingBuffer_CheckEnough(bus_send_length) == 0)
	{
		RingBuffer.ring_write_data_lock = 0;
		return 0;
	}
	
	MUSIC_TXBuffer[0] = TTL_HEADER_ONE;MUSIC_TXBuffer[1] = TTL_HEADER_TWO;
	MUSIC_TXBuffer[2] = bus_send_length; MUSIC_TXBuffer[3] = 0X00;
	
	MUSIC_TXBuffer[4] = MUSIC_MSGR_DEVICE; //设备型号
	MUSIC_TXBuffer[5] = 0XFF; 
	
	MUSIC_TXBuffer[6] = 0x00;  //密钥
	MUSIC_TXBuffer[7] = device_funccode_temp; //功能码
	MUSIC_TXBuffer[8] = 0x01; //控制命令字
	
	
	for(temp_i = 2;temp_i < bus_send_length - 1;temp_i++)
	{
		sum_check += MUSIC_TXBuffer[temp_i];
	}

	MUSIC_TXBuffer[bus_send_length - 1] = sum_check%256;	
		

	for(temp_i = 0;temp_i < bus_send_length;temp_i ++)
	{
		if(Write_Ring_Data(MUSIC_TXBuffer[temp_i]) == 1) //错误
		{
			break;
		}
	}	
		
	RingBuffer.ring_write_data_lock = 0;
	
	if(temp_i == bus_send_length)  return 1;

	return 0;
}
unsigned char TTL_MusicWhiteNoise_Write_Cmd(unsigned char device_funccode_temp , unsigned char extend_data_length)   
{
	unsigned char temp_i = 0;
	unsigned short sum_check = 0;
	unsigned char bus_send_length = 10 + extend_data_length;
	
	if(0 == TTL_IsDeviceOnline(MUSIC_WHITE_NOISE_DEVICE)) return 1;
	
	if(RingBuffer.ring_write_data_lock != 0) return 0;
	RingBuffer.ring_write_data_lock = 1;	
		
	if(TTL_RingBuffer_CheckEnough(bus_send_length) == 0)
	{
		RingBuffer.ring_write_data_lock = 0;
		return 0;
	}
		
	MUSIC_TXBuffer[0] = TTL_HEADER_ONE;MUSIC_TXBuffer[1] = TTL_HEADER_TWO;
	MUSIC_TXBuffer[2] = bus_send_length; MUSIC_TXBuffer[3] = 0X00;
	
	MUSIC_TXBuffer[4] = MUSIC_WHITE_NOISE_DEVICE; //设备型号
	MUSIC_TXBuffer[5] = 0XFF; 
	
	MUSIC_TXBuffer[6] = 0x00;  //密钥
	MUSIC_TXBuffer[7] = device_funccode_temp; //功能码
	MUSIC_TXBuffer[8] = 0x01; //控制命令字
	
	
	for(temp_i = 2;temp_i < bus_send_length - 1;temp_i++)
	{
		sum_check += MUSIC_TXBuffer[temp_i];
	}

	MUSIC_TXBuffer[bus_send_length - 1] = sum_check%256;	
		

	for(temp_i = 0;temp_i < bus_send_length;temp_i ++)
	{
		if(Write_Ring_Data(MUSIC_TXBuffer[temp_i]) == 1) //错误
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
unsigned char TTL_Master_Send_WriteMsgrModeTimer_Cmd(unsigned char msgr_order , unsigned char msgr_mode_temp , unsigned char msgr_time_temp)   
{
	MUSIC_TXBuffer[TTL_PARA_START] = msgr_order;  //按摩器序号
	MUSIC_TXBuffer[TTL_PARA_START + 1] = msgr_mode_temp;  //按摩器模式
	MUSIC_TXBuffer[TTL_PARA_START + 2] = msgr_time_temp;  //按摩器定时时间
	
	return TTL_MusicMsgr_Write_Cmd(0x21, 3);   
}
/*
//随振模式强度设置
//序号现在默认写0XFF
//强度
*/
unsigned char TTL_Master_Send_WriteMsgrFollowInts_Cmd(unsigned char msgr_order,unsigned char msgr_ints_temp)   
{
	MUSIC_TXBuffer[TTL_PARA_START] = msgr_order;  //按摩器序号
	MUSIC_TXBuffer[TTL_PARA_START + 1] = msgr_ints_temp;  //按摩器强度
	
	return TTL_MusicMsgr_Write_Cmd(0x22, 2); 
}
/*
//典型模式强度
//序号 1头部 2脚部   00全部
//强度 0 1 2 3
*/
unsigned char TTL_Master_Send_WriteMsgrTypicalInts_Cmd(unsigned char msgr1_ints,unsigned char msgr2_ints)
{
	MUSIC_TXBuffer[TTL_PARA_START]     = 1;  //按摩器序号
	MUSIC_TXBuffer[TTL_PARA_START + 1] = msgr1_ints;  //按摩器强度
	MUSIC_TXBuffer[TTL_PARA_START + 2] = 2;  //按摩器序号
	MUSIC_TXBuffer[TTL_PARA_START + 3] = msgr2_ints;  //按摩器强度
	
	return TTL_MusicMsgr_Write_Cmd(0x23, 4); 
}
/*
//典型模式速度参数
//模式 1波浪 2脉冲
//时间 空闲时间   持续时间   波浪上升时间  波浪下降时间
*/
unsigned char TTL_Master_Send_WriteMsgrTypicalSpeed_Cmd(unsigned char msgr_mode,unsigned short idle_time_temp,unsigned short keep_time_temp,unsigned short rise_time_temp,unsigned short desc_time_temp)   
{
	MUSIC_TXBuffer[TTL_PARA_START] = msgr_mode;  //按摩器序号
	//空闲时间
	MUSIC_TXBuffer[TTL_PARA_START + 1] = (unsigned char)(idle_time_temp>>8);  
	MUSIC_TXBuffer[TTL_PARA_START + 2] = (unsigned char)(idle_time_temp); 
	//持续时间
	MUSIC_TXBuffer[TTL_PARA_START + 3] = (unsigned char)(keep_time_temp>>8);  
	MUSIC_TXBuffer[TTL_PARA_START + 4] = (unsigned char)(keep_time_temp);
	//波浪上升时间
	MUSIC_TXBuffer[TTL_PARA_START + 5] = (unsigned char)(rise_time_temp>>8);    
	MUSIC_TXBuffer[TTL_PARA_START + 6] = (unsigned char)(rise_time_temp);    
	//波浪下降时间
	MUSIC_TXBuffer[TTL_PARA_START + 7] = (unsigned char)(desc_time_temp>>8); 
	MUSIC_TXBuffer[TTL_PARA_START + 8] = (unsigned char)(desc_time_temp); 
	//
	return TTL_MusicMsgr_Write_Cmd(0x24, 9); 
}
/*
//音量设置
//volume_cmd_temp 0设置   1音量-1   2音量+1
//音量大小
*/
unsigned char TTL_Master_Send_WriteMusicVolume_Cmd(MUSIC_VOLUME_CMD volume_cmd_temp,unsigned char volume_dat_temp)   
{
	MUSIC_TXBuffer[TTL_PARA_START] = volume_cmd_temp;  //设置命令
	MUSIC_TXBuffer[TTL_PARA_START + 1] = volume_dat_temp;  //音量大小  只有设置指令是0时起作用
	
	return TTL_MusicMsgr_Write_Cmd(0x25, 2); 
}
/*
//音量设置
//volume_cmd_temp 0设置   1音量-1   2音量+1
//音量大小
*/
unsigned char TTL_Master_WhiteNoiseSend_WriteMusicVolume_Cmd(MUSIC_VOLUME_CMD volume_cmd_temp,unsigned char volume_dat_temp)   
{
	MUSIC_TXBuffer[TTL_PARA_START] = volume_cmd_temp;  //设置命令
	MUSIC_TXBuffer[TTL_PARA_START + 1] = volume_dat_temp;  //音量大小  只有设置指令是0时起作用
	
	return TTL_MusicWhiteNoise_Write_Cmd(0x25, 2); 
}
/*
//曲目控制
//track_cmd_temp 1上一曲  2下一曲
*/
unsigned char TTL_Master_Send_WriteMusicTrack_Cmd(MUSIC_TRACK_CMD track_cmd_temp)   
{
	MUSIC_TXBuffer[TTL_PARA_START] = track_cmd_temp;  //设置命令
	
	return TTL_MusicMsgr_Write_Cmd(0x26, 1); 
}
/*
//曲目控制
//track_cmd_temp 1上一曲  2下一曲
*/
unsigned char TTL_Master_WhiteNoiseSend_WriteMusicTrack_Cmd(MUSIC_TRACK_CMD track_cmd_temp)   
{
	MUSIC_TXBuffer[TTL_PARA_START] = track_cmd_temp;  //设置命令
	
	return TTL_MusicWhiteNoise_Write_Cmd(0x26, 1); 
}
/*
//曲目设置
//track_cmd_temp 1上一曲  2下一曲
*/
unsigned char TTL_Master_WhiteNoiseSend_SetMusicTrack_Cmd(unsigned char track_temp)   
{
	MUSIC_TXBuffer[TTL_PARA_START] = 3; 
	MUSIC_TXBuffer[TTL_PARA_START + 1] = track_temp;  //设置命令
	
	return TTL_MusicWhiteNoise_Write_Cmd(0x26, 2); 
}
/*
//播放控制
//play_cmd_temp 0暂停  1播放  2暂停/播放
*/
unsigned char TTL_Master_Send_WriteMusicPlay_Cmd(SWITCH_CMD play_cmd_temp)   
{
	MUSIC_TXBuffer[TTL_PARA_START] = play_cmd_temp;  //设置命令
	
	return TTL_MusicMsgr_Write_Cmd(0x27, 1); 
}
/*
//播放控制
//play_cmd_temp 0暂停  1播放  2暂停/播放
*/
unsigned char TTL_Master_WhiteNoiseSend_WriteMusicPlay_Cmd(SWITCH_CMD play_cmd_temp)   
{
	MUSIC_TXBuffer[TTL_PARA_START] = play_cmd_temp;  //设置命令
	
	return TTL_MusicWhiteNoise_Write_Cmd(0x27, 1); 
}
/*
//音源选择
//src_temp 0蓝牙  1AUX  2U盘
*/
unsigned char TTL_Master_Send_WriteMusicSource_Cmd(MUSIC_SOURCE_CMD src_temp)   
{
	MUSIC_TXBuffer[TTL_PARA_START] = src_temp;  //
	
	return TTL_MusicMsgr_Write_Cmd(0x28, 1); 
}
/*
//蓝牙控制
//ble_func_temp 1蓝牙开关 2TWS开关  3列表操作
//开关
*/
unsigned char TTL_Master_Send_WriteBuleTooth_Cmd(MUSIC_BLUE_CMD ble_func_temp,SWITCH_CMD ble_dat_temp)
{
	MUSIC_TXBuffer[TTL_PARA_START] = ble_func_temp;  //
	MUSIC_TXBuffer[TTL_PARA_START+1] = ble_dat_temp;  //
	
	return TTL_MusicMsgr_Write_Cmd(0x29, 2); 
}
/*
//功率模式
//power_temp 0正常功率  1低功率
*/
unsigned char TTL_Master_Send_WriteMsgrPower_Cmd(unsigned char power_temp)   
{
	MUSIC_TXBuffer[TTL_PARA_START] = power_temp;  //
	
	return TTL_MusicMsgr_Write_Cmd(0x2A, 1); 
}
/*
//系统工作模式
//sys_mode_temp 0正常  1演示  2正常/演示  3OTA
*/
unsigned char TTL_Master_Send_WriteMusicSysMode_Cmd(unsigned char sys_mode_temp)   
{
	MUSIC_TXBuffer[TTL_PARA_START] = sys_mode_temp;  //
	
	return TTL_MusicMsgr_Write_Cmd(0x2B, 1); 
}
//演示模式播放源
unsigned char TTL_Master_Send_WriteMusicDemo_Source(unsigned char source,unsigned char track)
{
	MUSIC_TXBuffer[TTL_PARA_START] = 0x04;  //
	MUSIC_TXBuffer[TTL_PARA_START+1] = source;
	MUSIC_TXBuffer[TTL_PARA_START+2] = track;
	return TTL_MusicMsgr_Write_Cmd(0x2B, 3); 
}
//随动模式开关
unsigned char TTL_Master_Send_WriteMotorSysMode_Cmd(unsigned char motor_sys_mode_temp)
{
	MUSIC_TXBuffer[TTL_PARA_START] = motor_sys_mode_temp;  //
	return TTL_MusicMsgr_Write_Cmd(0x2d, 1); 
}
/*
//查询
//
*/
unsigned char TTL_MusicMsgr_Read_Cmd(unsigned char device_add_temp,unsigned char device_func_code,unsigned char extend_byte_temp)
{
	unsigned char temp_i = 0;
	unsigned short sum_check = 0;
	unsigned char bus_send_length = 11;
	
	if(RingBuffer.ring_write_data_lock != 0) return 0;
	RingBuffer.ring_write_data_lock = 1;			

	if(TTL_RingBuffer_CheckEnough(bus_send_length) == 0)
	{
		RingBuffer.ring_write_data_lock = 0;
		return 0;
	}
	
	MUSIC_TXBuffer[0] = TTL_HEADER_ONE;MUSIC_TXBuffer[1] = TTL_HEADER_TWO;
	MUSIC_TXBuffer[2] = bus_send_length; MUSIC_TXBuffer[3] = 0X00;
	
	MUSIC_TXBuffer[4] = MUSIC_MSGR_DEVICE; //设备型号
	MUSIC_TXBuffer[5] = device_add_temp; //找出类型是0x10的地址Master_SearchIdleAdd(MUSIC_MSGR_DEVICE)
	
	MUSIC_TXBuffer[6] = 0x00;  //密钥
	MUSIC_TXBuffer[7] = device_func_code; //功能码
	MUSIC_TXBuffer[8] = 0x00; //问询命令字
	
	MUSIC_TXBuffer[9] = extend_byte_temp;
	
	for(temp_i = 2;temp_i < bus_send_length - 1;temp_i++)
	{
		sum_check += MUSIC_TXBuffer[temp_i];
	}

	MUSIC_TXBuffer[bus_send_length - 1] = sum_check%256;	
		

	for(temp_i = 0;temp_i < bus_send_length;temp_i ++)
	{
		if(Write_Ring_Data(MUSIC_TXBuffer[temp_i]) == 1) //错误
		{
			break;
		}
	}	
		
	RingBuffer.ring_write_data_lock = 0;
	
	if(temp_i == bus_send_length)  return 1;

	return 0;
}	
/*
//查询
//
*/
unsigned char TTL_MusicWhiteNoise_Read_Cmd(unsigned char device_add_temp,unsigned char device_func_code,unsigned char extend_byte_temp)
{
	unsigned char temp_i = 0;
	unsigned short sum_check = 0;
	unsigned char bus_send_length = 11;
	
	if(RingBuffer.ring_write_data_lock != 0) return 0;
	RingBuffer.ring_write_data_lock = 1;			

	if(TTL_RingBuffer_CheckEnough(bus_send_length) == 0)
	{
		RingBuffer.ring_write_data_lock = 0;
		return 0;
	}
		
	MUSIC_TXBuffer[0] = TTL_HEADER_ONE;MUSIC_TXBuffer[1] = TTL_HEADER_TWO;
	MUSIC_TXBuffer[2] = bus_send_length; MUSIC_TXBuffer[3] = 0X00;
	
	MUSIC_TXBuffer[4] = MUSIC_WHITE_NOISE_DEVICE; //设备型号
	MUSIC_TXBuffer[5] = device_add_temp; //找出类型是0x10的地址Master_SearchIdleAdd(MUSIC_WHITE_NOISE_DEVICE)
	
	MUSIC_TXBuffer[6] = 0x00;  //密钥
	MUSIC_TXBuffer[7] = device_func_code; //功能码
	MUSIC_TXBuffer[8] = 0x00; //问询命令字
	
	MUSIC_TXBuffer[9] = extend_byte_temp;
	
	for(temp_i = 2;temp_i < bus_send_length - 1;temp_i++)
	{
		sum_check += MUSIC_TXBuffer[temp_i];
	}

	MUSIC_TXBuffer[bus_send_length - 1] = sum_check%256;	
		

	for(temp_i = 0;temp_i < bus_send_length;temp_i ++)
	{
		if(Write_Ring_Data(MUSIC_TXBuffer[temp_i]) == 1) //错误
		{
			break;
		}
	}	
		
	RingBuffer.ring_write_data_lock = 0;
	
	if(temp_i == bus_send_length)  return 1;

	return 0;
}	
