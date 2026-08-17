#include "modul_a7105.h"
#include "delay.h"

#include "driver_a7105.h"

#include "driver_beep.h"
#include "app_config.h"
/*-----------------------A7105收发模式---------------------------*/
unsigned char ID_USE[4] = {0xE2 ,0x47 ,0xE2 ,0x47}; //此数组会被写入eeprom  或者从eeprom中读取

unsigned char HJ_A7105_CODE[4] = {0}; 

/*
* 描述:  A7105初始化函数
* 参数:  无
* 返回值: 无*/

unsigned char A7105_Init(void)						  
{
	return 0;
}


unsigned char A7105_Study_Mode(void)
{
	return 0; 
}

void A7105_Recv_Mode(void)
{

}
/*
*  描述: 取出频道偏移值  由ID[3]计算出来
*  参数: 0 接受模式  1发送模式
*  返回值: 无*/
unsigned char A7105_Read_CHdata(unsigned char mode)
{
	return 0;
}

void A7105_TimerManager(void)
{
	return ;
}


unsigned char A7105_Comm_Free(void)
{

  return 1;
}

unsigned char A7105_Get_KeyState(void)
{
	return 0;
}

