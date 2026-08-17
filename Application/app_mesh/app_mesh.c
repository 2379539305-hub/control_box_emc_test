#include "app_mesh.h"
#include "app_comm.h"
#include "app_linbus.h"
#include "app_ble.h"
#include "app_music.h"
#include "app_config.h"

#include "stdio.h"
#include "string.h"
#include "delay.h"

#include "driver_ble.h"
#include "driver_beep.h"

#include "modul_a7105.h"

unsigned char mesh_start_pair_buff[] = "AT+MESHPAIR\r\n";//开始配对
unsigned char mesh_stop_pair_buff[]  = "AT+MESHSTOP\r\n";
unsigned char mesh_clear_pair_buff[] = "AT+MESHCLEAR\r\n";//清除配对
unsigned char mesh_connect_pair_buff[] = "AT+MESHCONN\r\n";//开始连接

char *mesh_pair_success_ack_buff = "+IND=MESHPAOK\r";//配对成功
char *mesh_connect_ack_buff = "+IND=MESHC\r";//连接成功
char *mesh_pair_clear_ack_buff = "+IND=MESHD\r";//连接断开
char *mesh_stop_pair_ack_buff = "+IND=MESHNOPAIR\r";//没有配对
char *mesh_connect_fail_buff = "+IND=MESHCONFAIL\r";//没有连接
unsigned char mesh_uart_rx_buff[100];
unsigned char mesh_uart_rx_length = 0;
unsigned char mesh_start_pair_flag = 0;//开始对码

unsigned char mesh_start_connect = 1;//配对成功，开始连接，上电主动连接一次
unsigned long mesh_long_time = 0;//连接超时
unsigned char mesh_pair_timeout_flag = 0;

unsigned char mesh_send_flag = 0;//0-断开不能发送，1-使能发送


//串口中断中用于接收组网消息是否成功或者是否清除
void mesh_uart_process(unsigned char data)
{
	if(system_config.flags.ble_mesh_sync_config == BLE_MESH_SYNC_DISABLE)
	{
		return;
	}
	if(mesh_uart_rx_length >= 100)
	{
		mesh_uart_rx_length = 0;
	}
	
	if(data!=0)
	{
		 mesh_uart_rx_buff[mesh_uart_rx_length++] = data;
	}
	
	if(mesh_uart_rx_length == 1)
	{
		if(mesh_uart_rx_buff[0] != '+')
		{
			mesh_uart_rx_length = 0;
		}
	}
	else if(data == '\n')
	{
		if(user_strncmp((const char *)mesh_uart_rx_buff, mesh_pair_success_ack_buff, 14) == 0)  // 如果是组队成功
		{
			mesh_start_pair_flag = 0;//清除组队标志
			mesh_pair_timeout_flag = 0;//清除组队超时标志	
			Beep_SingSetPara(500,2);
			music_para_set_event |= MUSIC_TWS_ON_EVENT;	//蓝牙音响组队					
			mesh_start_connect = 1;
		}
		if(user_strncmp((const char *)mesh_uart_rx_buff, mesh_connect_ack_buff, 11) == 0) //如果连接成功
		{
			mesh_send_flag = 1;//使能发送
		}			
		if(user_strncmp((const char *)mesh_uart_rx_buff, mesh_stop_pair_ack_buff, 16) == 0) //如果没有配对
		{
			mesh_send_flag = 0;//关闭发送
		}		
		if(user_strncmp((const char *)mesh_uart_rx_buff, mesh_connect_fail_buff, 17) == 0) //没连接
		{
			mesh_send_flag = 0;//关闭发送
		}		
		mesh_uart_rx_length = 0;	
	}
}
unsigned char Get_Mesh_Connect(void)
{
	return mesh_send_flag;
}
void Set_Mesh_Connect_Flag(unsigned char flag)
{
	mesh_send_flag = flag;
}
//开启组网
void mesh_start_pair(void)//组队同时会清除
{
	BleBlueTooth_SendString(mesh_start_pair_buff,strlen((char *)mesh_start_pair_buff));
}
//结束组网
void mesh_stop_pair(void)
{
	BleBlueTooth_SendString(mesh_stop_pair_buff,strlen((char *)mesh_stop_pair_buff));
}
//清除组网
void mesh_clear_pair(void)//尽量不用，有问题
{
	BleBlueTooth_SendString(mesh_clear_pair_buff,strlen((char *)mesh_clear_pair_buff));
}
void mesh_connect_pair(void)
{
	BleBlueTooth_SendString(mesh_connect_pair_buff,strlen((char *)mesh_connect_pair_buff));
}

//组网控制
void mesh_control(void)
{
	static unsigned char key_value_temp = 0, old_key_value_temp = 0;	

	if(system_config.flags.ble_mesh_sync_config == BLE_MESH_SYNC_DISABLE)
	{
		return;
	}
	//组队成功发起连接
	if(mesh_start_connect)
	{
		Delay_Ms(500);	//增加延时，不然配对失败概率很大
		mesh_connect_pair();
		mesh_start_connect = 0;
	}
	//超时清除组队
	if(mesh_pair_timeout_flag == 1)
	{
		mesh_pair_timeout_flag = 0;
		if(mesh_start_connect == 0)
		{
			mesh_stop_pair();
//			Delay_Ms(20);
//			mesh_clear_pair();
//			//清除成功播报
			music_para_set_event |= MUSIC_TWS_OFF_EVENT;
			mesh_send_flag = 0;
			Beep_SingSetPara(500,3);			
		}
	}
	//保存历史键值
	old_key_value_temp = key_value_temp;
	//接收新的键值
	key_value_temp = Get_Key_Value();
	
	/*--------------------------获取有用的键值信息---------------------------*/
	if((0 == key_value_temp) || KEY_MUSIC_TWS_SW == key_value_temp || KEY_CLEAR_MESH_PAIR == key_value_temp)
	{
		
	}
	else  //无用信息
	{
		return;
	}
	//如果是开始组网，只发一次开始
	static unsigned char mesh_state = 0;
	if((key_value_temp != KEY_NO) && (key_value_temp != old_key_value_temp))
	{
		if(key_value_temp == KEY_MUSIC_TWS_SW)
		{
			if(mesh_start_pair_flag == 0)
			{
				mesh_start_pair_flag = 1;
				mesh_send_flag = 0;
				Beep_SingSetPara(200,1);		
				Delay_Ms(100);	//增加延时，要不然链接成功不响				
				mesh_start_pair();
				
			}
		}
	}
	 if((key_value_temp == 0) && (1 == Ble_Comm_Free()) && (1 == LIN_Check_Busy_Free()) && (1 == A7105_Comm_Free()))
	 {
			if(mesh_send_flag == 0)
			{
				mesh_send_flag = 1;
			}
	 }
}
void Mesh_TimeManagerTask(void)
{
	if(mesh_start_pair_flag==1)
	{
		mesh_long_time++;
		if(mesh_long_time >= 1000)
		{
			mesh_pair_timeout_flag = 1;
			mesh_start_pair_flag = 0;
		}
	}
	else
	{
		mesh_long_time = 0;
	}		
}
