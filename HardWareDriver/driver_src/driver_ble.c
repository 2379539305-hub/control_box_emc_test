#include "driver_ble.h"
#include "driver_uart.h"
//蓝牙端口初始化
void BleBlueTooth_HardInit(void)
{
	FL_GPIO_InitTypeDef    GPIO_InitStruct = {0};
	
	GPIO_InitStruct.pin        = BLE_STATE_GPIO_PIN;
	GPIO_InitStruct.mode       = FL_GPIO_MODE_INPUT;
	GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.pull       = FL_DISABLE;
	GPIO_InitStruct.remapPin   = FL_DISABLE;
	FL_GPIO_Init(BLE_STATE_GPIO, &GPIO_InitStruct);	
}
void BleBlueTooth_SendData(uint8_t send_data)
{
	Uart5_SendData(send_data);
}
//蓝牙串口发送驱动
void BleBlueTooth_SendString(uint8_t *send_str,uint8_t length)
{
	Uart5_SendString(send_str,length);
}
//获取蓝牙连接状态
uint8_t BleBlueTooth_Connect_State(void)
{
	if(1 == FL_GPIO_GetInputPin(BLE_STATE_GPIO,BLE_STATE_GPIO_PIN))
	{
		return 1;
	}
	
	return 0;
}










