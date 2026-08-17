#include "system.h"

/*---------------外设驱动-----------------*/
#include "driver_uart.h"
#include "driver_led.h"
#include "driver_beep.h"
#include "driver_adc.h"
/*---------------软件驱动----------------*/

/*---------------------------------------*/
#include "modul_ttlbus.h"
#include "modul_a7105.h"
#include "modul_motor.h"
#include "modul_key.h"
/*---------------应用层------------------*/
#include "app_ttlbus.h"
#include "app_linbus.h"
#include "app_ble.h"
#include "app_motor.h"
#include "app_comm.h"
#include "app_msgr.h"
#include "app_light.h"
#include "app_rtc.h"
#include "app_vita.h"
#include "app_heat.h"
#include "app_backhual.h"
#include "app_ota.h"
#include "app_mesh.h"
#include "app_music.h"
#include "app_fan.h"
#include "app_config.h"
/********************** Timer0 100us中断函数 ************************/
void Time100us_Interrupt_Callback(void)
{
	//蜂鸣器驱动函数
	Beep_SoundTask();
	//TTL总线发送函数
	TTL_Bus_TxServer();
	//电机霍尔到顶到底检测
	Motor_Hall_LimitTask();
	//电机霍尔计数
	Motor_Hall_TakePositionTask();
	//电机电流获取位置函数
	Motor_saveAdc_Task();	
}


void LPTIM_IRQHandler(void)
{
	if(FL_LPTIM32_IsActiveFlag_Update(LPTIM32))
	{
		FL_LPTIM32_ClearFlag_Update(LPTIM32);
			
		Time100us_Interrupt_Callback();
	}
}
/********************** Timer3 中断函数 ************************/
void Time1ms_Interrupt_Callback(void)
{
	//5ms时间基准
	static unsigned char timer_base_count = 0;

	timer_base_count ++;
	App_Ota_Timer_1ms();
	if(1 == timer_base_count)
	{
		//ttl总线定时器中断函数
		TTL_Bus_TimerManager();	
		//lin总线定时器中断函数
		LIN_Bus_TimeManager();
		//蓝牙定时器中断函数
		BleBlueTooth_TimerManager();
//		//RF定时器中断函数
//		A7105_TimerManager();
//		//外设-灯-定时器中断函数
		Light_TimeManagerTask();
		//加热垫定时器中断函数
		Heat_TimeManagerTask();
		//打鼾复位定时器中断函数
		Snore_TimeManagerTask();
		//数据回传定时器中断函数
		SYS_UpData_TimeManagerTask();
		//mesh
		Mesh_TimeManagerTask();
		//误码超时
		Study_TimeManagerTask();
		//风扇定时器中断函数
		Fan_TimeManagerTask();
	}
	if(2 == timer_base_count)
	{
		// Motor_TimerManagerTask disabled (PWM controlled by main.c)
		// Motor_TimerManagerTask();
	}
	if(3 == timer_base_count)	
	{
		//按摩器控制定时器中断函数
		Msgr_TimeManagerTask();
		//音乐控制定时器中断函数
		Music_TimeManagerTask();
	}
	if(4 == timer_base_count)
	{
		//TTL总线广播定时器中断函数
		TTL_MasterRadio_TimerManagerTask();
	}
	if(5 <= timer_base_count)
	{
		//电机电流检测到顶到底函数
		Motor_Current_LimitTask();
		timer_base_count = 0;
	}
}
void BSTIM_IRQHandler(void)
{
	if(FL_BSTIM32_IsActiveFlag_Update(BSTIM32))
	{
		FL_BSTIM32_ClearFlag_Update(BSTIM32);
		
		Time1ms_Interrupt_Callback(); 
	}
}
/*--------------------------------------------------------------------------------------*/
//TTL串口接收中断
void Uart_TTL_RxCallback(unsigned char uart_recv_temp)
{
	//用户接受函数
	//TTL协议
	TTL_Bus_RxServer(uart_recv_temp);
	//管制码
	Get_SourceCodeCmd_Analy(uart_recv_temp);
	
}
//TTL串口发送完成中断
void Uart_TTL_TxCallback(void)
{
	unsigned char send_data = 0;

	if(!Read_Ring_Data((unsigned char*) &send_data))
	{
		ttl_tx_busy = 1;
		TTL_Bus_SendData(send_data);
	}
	else
	{
		TTL_Bus_ClearTxBusy();
	}
}
void UART0_IRQHandler(void)
{
	//
	if((FL_ENABLE == FL_UART_IsEnabledIT_RXBuffFull(UART0)) && (FL_SET == FL_UART_IsActiveFlag_RXBuffFull(UART0)))
	{
		FL_UART_ClearFlag_RXBuffFull(UART0);    //Clear Rx flag
		
		Uart_TTL_RxCallback(FL_UART_ReadRXBuff(UART0));
	}
	//
	//发送缓存空且发送移位寄存器空中断使能，1 有效
	//发送缓存空且移位寄存器发送完成中断标志，硬件置位，软件写1 或者发送数据被载入移位寄存器时清零	
	if((FL_ENABLE == FL_UART_IsEnabledIT_TXShiftBuffEmpty(UART0))  && (FL_SET == FL_UART_IsActiveFlag_TXShiftBuffEmpty(UART0)))
	{
		Uart0_ClearTxBusy();
		FL_UART_ClearFlag_TXShiftBuffEmpty(UART0);    //Clear Tx flag
		
		Uart_TTL_TxCallback();
	}
}
/*--------------------------------------------------------------------------------------*/
//wifi
void Uart_Wifi_RxCallback(unsigned char uart_recv_temp)
{
	
}
void Uart_Wifi_TxCallback(void)
{
	
}
void UART1_IRQHandler(void)
{
	//接收中断处理
	if((FL_ENABLE == FL_UART_IsEnabledIT_RXBuffFull(UART1)) && (FL_SET == FL_UART_IsActiveFlag_RXBuffFull(UART1)))
	{
		FL_UART_ClearFlag_RXBuffFull(UART1);    //Clear Rx flag
		
		Uart_Wifi_RxCallback(FL_UART_ReadRXBuff(UART1));
	}
	//
	//发送中断处理
	if((FL_ENABLE == FL_UART_IsEnabledIT_TXShiftBuffEmpty(UART1)) && (FL_SET == FL_UART_IsActiveFlag_TXShiftBuffEmpty(UART1)))
	{
		Uart1_ClearTxBusy();
		FL_UART_ClearFlag_TXShiftBuffEmpty(UART1);    //Clear Tx flag

		Uart_Wifi_TxCallback();
	}	
	
}
/*--------------------------------------------------------------------------------------*/
//lin
void Uart_Lin_RxCallback(unsigned char uart_recv_temp)
{
	LIN_Bus_RxServer(uart_recv_temp);
}

void Uart_Lin_TxCallback(void)
{
	
}

void UART4_IRQHandler(void)
{
	//接收中断处理
	if((FL_ENABLE == FL_UART_IsEnabledIT_RXBuffFull(UART4)) && (FL_SET == FL_UART_IsActiveFlag_RXBuffFull(UART4)))
	{
		FL_UART_ClearFlag_RXBuffFull(UART4);    //Clear Rx flag
		if(system_config.flags.ble_mesh_sync_config == BLE_MESH_SYNC_DISABLE)
		{
			Uart_Lin_RxCallback(FL_UART_ReadRXBuff(UART4));
		}
	}
	//
	//发送中断处理
	if((FL_ENABLE == FL_UART_IsEnabledIT_TXShiftBuffEmpty(UART4)) && (FL_SET == FL_UART_IsActiveFlag_TXShiftBuffEmpty(UART4)))
	{
		Uart4_ClearTxBusy();
		FL_UART_ClearFlag_TXShiftBuffEmpty(UART4);    //Clear Tx flag
		 
		Uart_Lin_TxCallback();
	}	
}
/*--------------------------------------------------------------------------------------*/
//BLE
void Uart_Ble_RxCallback(unsigned char uart_recv_temp) 
{
	BleBlueTooth_RxServer(uart_recv_temp);
	mesh_uart_process(uart_recv_temp);
	App_Ota_RxServer(uart_recv_temp);
	if(system_config.flags.ble_mesh_sync_config != BLE_MESH_SYNC_DISABLE)
	{
		Uart_Lin_RxCallback(uart_recv_temp);
	}
}
void Uart_Ble_TxCallback(void) 
{
	
}
void UART5_IRQHandler(void)
{
	//接收中断处理
	if((FL_ENABLE == FL_UART_IsEnabledIT_RXBuffFull(UART5)) && (FL_SET == FL_UART_IsActiveFlag_RXBuffFull(UART5)))
	{
		FL_UART_ClearFlag_RXBuffFull(UART5);    //Clear Rx flag

		Uart_Ble_RxCallback(FL_UART_ReadRXBuff(UART5));
	}
	//
	//发送中断处理
	if((FL_ENABLE == FL_UART_IsEnabledIT_TXShiftBuffEmpty(UART5)) && (FL_SET == FL_UART_IsActiveFlag_TXShiftBuffEmpty(UART5)))
	{
		Uart5_ClearTxBusy();
		FL_UART_ClearFlag_TXShiftBuffEmpty(UART5);    //Clear Tx flag
		
		Uart_Ble_TxCallback();
	}		
}
/*--------------------------------------------------------------------------------------*/
//闹钟中断
void RTC_Interrupt_Callback(void) 
{

}
// RTC中断处理函数
void RTC_IRQHandler(void)
{
	//闹钟中断
	if(FL_ENABLE == FL_RTC_IsEnabledIT_Alarm(RTC) &&  FL_SET == FL_RTC_IsActiveFlag_Alarm(RTC))         //查询标志是否置起
	{
		FL_RTC_ClearFlag_Alarm(RTC);                          //清除中断标志
		//FL_GPIO_ResetOutputPin(GPIOC, FL_GPIO_PIN_0);
	}

	//1KHz中断
	if(FL_ENABLE == FL_RTC_IsEnabledIT_1KHz(RTC) && FL_SET == FL_RTC_IsActiveFlag_1KHz(RTC))          //查询标志是否置起
	{
		FL_RTC_ClearFlag_1KHz(RTC);                           //清除中断标志
	}

	//256Hz中断
	if(FL_ENABLE == FL_RTC_IsEnabledIT_256Hz(RTC) && FL_SET == FL_RTC_IsActiveFlag_256Hz(RTC))         //查询标志是否置起
	{
		FL_RTC_ClearFlag_256Hz(RTC);                          //清除中断标志
	}

	//64Hz中断
	if(FL_ENABLE == FL_RTC_IsEnabledIT_64Hz(RTC) && FL_SET == FL_RTC_IsActiveFlag_64Hz(RTC))          //查询标志是否置起
	{
		FL_RTC_ClearFlag_64Hz(RTC);                           //清除中断标志
	}

	//16Hz中断
	if(FL_ENABLE == FL_RTC_IsEnabledIT_16Hz(RTC) && FL_SET == FL_RTC_IsActiveFlag_16Hz(RTC))          //查询标志是否置起
	{
		FL_RTC_ClearFlag_16Hz(RTC);                           //清除中断标志
	}

	//8Hz中断
	if(FL_ENABLE == FL_RTC_IsEnabledIT_8Hz(RTC) && FL_SET == FL_RTC_IsActiveFlag_8Hz(RTC))           //查询标志是否置起
	{
		FL_RTC_ClearFlag_8Hz(RTC);                            //清除中断标志
	}

	//4Hz中断
	if(FL_ENABLE == FL_RTC_IsEnabledIT_4Hz(RTC) && FL_SET == FL_RTC_IsActiveFlag_4Hz(RTC))           //查询标志是否置起
	{
		FL_RTC_ClearFlag_4Hz(RTC);                            //清除中断标志
	}

	//2Hz中断
	if(FL_ENABLE == FL_RTC_IsEnabledIT_2Hz(RTC) && FL_SET == FL_RTC_IsActiveFlag_2Hz(RTC))           //查询标志是否置起
	{
		FL_RTC_ClearFlag_2Hz(RTC);                            //清除中断标志
	}

	//秒中断
	if(FL_ENABLE == FL_RTC_IsEnabledIT_Second(RTC) && FL_SET == FL_RTC_IsActiveFlag_Second(RTC))        //查询标志是否置起
	{
		FL_RTC_ClearFlag_Second(RTC);                         //清除中断标志
		//FL_GPIO_ToggleOutputPin(GPIOC, FL_GPIO_PIN_0);
	}

	//分钟中断
	if(FL_ENABLE == FL_RTC_IsEnabledIT_Minute(RTC) && FL_SET == FL_RTC_IsActiveFlag_Minute(RTC))        //查询标志是否置起
	{
		FL_RTC_ClearFlag_Minute(RTC);                         //清除中断标志
	}

	//小时中断
	if(FL_ENABLE == FL_RTC_IsEnabledIT_Hour(RTC) && FL_SET == FL_RTC_IsActiveFlag_Hour(RTC))          //查询标志是否置起
	{
		FL_RTC_ClearFlag_Hour(RTC);                           //清除中断标志
	}

	//天中断
	if(FL_ENABLE == FL_RTC_IsEnabledIT_Day(RTC) && FL_SET == FL_RTC_IsActiveFlag_Day(RTC))           //查询标志是否置起
	{
		FL_RTC_ClearFlag_Day(RTC);                            //清除中断标志
	}
}








