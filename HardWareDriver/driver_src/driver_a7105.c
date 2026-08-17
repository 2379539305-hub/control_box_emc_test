#include "driver_a7105.h"
#include "delay.h"
//射频寄存器配置参数
/*----------------------------------------A7105  用户使用函数--------------------------------*/
/*
*  描述: 配置新ID模式  12R2
*  参数: 无
*  返回值: 无*/
void A7105_NewConfig(void)					 
{

}
/*
*  描述: 配置新ID模式  1219
*  参数: 无
*  返回值: 无*/
void A7105_OldConfig(void)					 
{

}
/*
*  描述: 给各个寄存器初始化
*  参数: 无
*  返回值: 无*/
void A7105_Config(uint8_t a7105_config_flag)					 
{
	if(a7105_config_flag == A7105_OLD_ID)
	{
		A7105_OldConfig();	
	}
	if(a7105_config_flag == A7105_NEW_ID)
	{
		A7105_NewConfig();
	}
}
/*
*  描述 : 设置频道偏移值 0-160 加1相当于偏移500hz
*  参数: 偏移值
*  返回值 : 无*/
void A7105_SetCH(uint8_t ch_temp)						 
{

}
/*
*  描述: 写A7105自身ID
*  参数 : 无
*  返回值: 无*/
void A7105_WriteID(uint8_t *id_buf_temp)				 
{

}
/*
*  描述: A7105接收模式下  读取A7105接收的数据
*  参数: 数据地址
*  返回值: 无*/
void A7105_RecvDataToBuf(uint8_t *databuf)
{

}

uint8_t A7105_Check_Online(void)
{
	return 1;
}



