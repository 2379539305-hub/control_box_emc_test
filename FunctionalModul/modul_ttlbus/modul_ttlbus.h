#ifndef __MODUL_TTLBUS_H
#define __MODUL_TTLBUS_H

#include "main.h"
#include "system.h"

#define SLAVE_BOX_TYPE					(0x01)  //从控制盒
#define HAND_DEVICE_TYPE       (0X02)  //手控器

#define IOT_DEVICE_TYPE         (0X04)  //物联

#define LIGHT_RGB_DEVICE_TYPE    (0X10)  //RGB灯
#define LIGHT_ONE_DEVICE_TYPE    (0X11)  //单色灯
#define LIGHT_BOARD_DEVICE_TYPE  (0X18)  //灯牌

#define MUSIC_DEVICE_TYPE        (0X12)  //音乐阵子
#define VITA_DEVICE_TYPE         (0X13)  //生命体征检测
#define FAN_DEVICE_TYPE					(0x14)	//风扇
#define SW_DEVICE_TYPE          (0X15)  //开关设备
#define MUSIC_WHITE_NOISE_DEVICE_TYPE (0X16) //白噪音音响
#define CONFIG_DEVICE_TYPE    (0xC9)  //上位机配置工具

#define GESTURE_DEVICE_TYPE  (0X19)  //手势识别

#define TTL_BUS_TIME_BASE  5

#define TTL_FREE_LONG_TIME (300/TTL_BUS_TIME_BASE)  //判定总线是否空闲

#define TTL_WAIT_TIME (100/TTL_BUS_TIME_BASE)  //判定从机回复超时50ms
#define TTL_ASK_TIME  (100/TTL_BUS_TIME_BASE)  //主机问询时间100ms

#define TTL_RADIO_SYSINFO_TIME  (1000/TTL_BUS_TIME_BASE)  //主机问询时间100ms

#define RINGBUFF_LEN    200     //定义最大接收字节数
typedef struct
{
	unsigned char ring_write_data_lock;
	unsigned char heard;
	unsigned char tail;
	unsigned char ring_buffer[RINGBUFF_LEN]; //环形缓冲区数组
}RingBufferStruct;

extern RingBufferStruct  RingBuffer;
//TTL总线协议相关
#define TTL_PARA_START  (9)
#define TTL_HEADER_ONE  0XFA
#define TTL_HEADER_TWO  0X5A

#define TTL_SLAVE_TYPE_BIT   (4)
#define TTL_SLAVE_ADD_BIT    (5)
#define TTL_SECRET_KEY_BIT   (6)
#define TTL_CODE_BIT         (7)

//设备注册信息相关
#define DEVICE_NUM_MAX  						10
#define DEVICE_INFO_ARR_TYPE         0
#define DEVICE_INFO_ARR_SK           1
#define DEVICE_INFO_ARR_COMM         2
#define DEVICE_INFO_ARR_CODE         3
#define DEVICE_INFO_ARR_EXTEND1      4 
#define DEVICE_INFO_ARR_EXTEND2      5 
#define DEVICE_INFO_ARR_POLLNUM      6
#define DEVICE_INFO_ARR_LINE         7
#define DEVICE_INFO_MAX								8
extern unsigned char ttl_tx_busy;
extern unsigned char Device_InfoArr[DEVICE_NUM_MAX + 1][DEVICE_INFO_MAX+1];
extern unsigned char TTL_DeviceOnlineCount[256];

unsigned char Master_SearchIdleAdd(unsigned char device_type_temp); //查找设备类型地址
void TTL_Bus_Init(void);//需要用户放在串口初始化后
void TTL_Bus_TxServer(void);
void TTL_Bus_RxServer(unsigned char uart_recv_value);//需要用户放在串口接收中断  并将每次接收的字节传参
void TTL_Bus_TimerManager(void); //需要用户放在定时器中断   1ms调用一次本函数
void TTL_Bus_ClearTxBusy(void); //清发送完成标志位  需要用户放在串口发送完成中断

unsigned char Write_Ring_Data(unsigned char data_temp);
unsigned char TTL_RingBuffer_CheckEnough(unsigned char need_len);
//用户修改
void TTL_Bus_SendData(unsigned char data_temp); 


//在其他.c文件中实现串口发送的函数  无阻塞式  直接将data_temp赋值给串口发送缓冲寄存器即可,无需阻塞判断
//比如51单片机 
//void TTL_Bus_SendData(unsigned char data_temp)
//{
//	 SBUF	= data_temp;
//}

//
#define TTL_BUF_LENGTH    30
extern unsigned char TTL_BUS_RXBuffer[TTL_BUF_LENGTH]; 
extern unsigned char TTL_BUS_TXBuffer[TTL_BUF_LENGTH]; 
void TTL_Bus_UartDataAnaly(unsigned char device_type);
unsigned char TTL_Check_Busy_Free(void);

unsigned char Get_Master_Ask_Mutex(void);
void Set_Master_Ask_Mutex(unsigned char mutex_lock);
void TTL_User_AskDevice_Status(unsigned char device_type,unsigned char func_code,unsigned char func_code_extend1,unsigned char func_code_extend2);
unsigned char TTL_IsDeviceOnline(unsigned char device_type);
unsigned char TTL_GetDeviceOnlineCount(unsigned char device_type);

unsigned char Read_Ring_Data(unsigned char *rData);
#endif






