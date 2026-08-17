#include "driver_uart.h"
#include "driver_config.h"

static uint8_t uart0_tx_busy = 0;
static uint8_t uart1_tx_busy = 0;

static uint8_t uart4_tx_busy = 0;
static uint8_t uart5_tx_busy = 0;

/**
  * @brief  UART0 Initialization function
  * @param  void
  * @retval None
  */
void MF_UART0_Init(void)
{
	FL_GPIO_InitTypeDef    GPIO_InitStruct;

	FL_UART_InitTypeDef    UART0_InitStruct;

	/* PA2 UART0_RX */ 
	GPIO_InitStruct.pin = FL_GPIO_PIN_2;
	GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
	GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.pull = FL_ENABLE;
	GPIO_InitStruct.remapPin = FL_DISABLE;
	FL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/* PA3 UART0_TX */ 
	GPIO_InitStruct.pin = FL_GPIO_PIN_3;
	GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
	GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.pull = FL_DISABLE;
	GPIO_InitStruct.remapPin = FL_DISABLE;
	FL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	UART0_InitStruct.clockSrc = FL_RCC_UART0_CLK_SOURCE_APB1CLK;
	UART0_InitStruct.baudRate = 19200;
	UART0_InitStruct.dataWidth = FL_UART_DATA_WIDTH_8B;
	UART0_InitStruct.stopBits = FL_UART_STOP_BIT_WIDTH_1B;
	UART0_InitStruct.parity = FL_UART_PARITY_NONE;
	UART0_InitStruct.transferDirection = FL_UART_DIRECTION_TX_RX;

	FL_UART_Init(UART0, &UART0_InitStruct);  
	uart0_tx_busy = 0;

}
 
/**
  * @brief  UART0 Interrupt Initialization function
  * @param  void
  * @retval None
  */
void MF_UART0_Interrupt_Init(void)
{
	FL_NVIC_ConfigTypeDef    InterruptConfigStruct;	
	
	FL_UART_ClearFlag_RXBuffFull(UART0);
	FL_UART_EnableIT_RXBuffFull(UART0);           //使能接收中断

	FL_UART_ClearFlag_TXShiftBuffEmpty(UART0);
	FL_UART_EnableIT_TXShiftBuffEmpty(UART0);  
	
	InterruptConfigStruct.preemptPriority = 0;
	FL_NVIC_Init(&InterruptConfigStruct, UART0_IRQn);	
}
/*
	串口1发送字节函数
	data_temp : 发送的字节
*/
void Uart0_SendData(uint8_t data_temp)
{
	uart0_tx_busy = 1;
	FL_UART_WriteTXBuff(UART0, data_temp); 
	while(uart0_tx_busy);
}
/*
	串口1直接发送
	data_temp : 发送的字节
*/
void Uart0_SendData_IT(uint8_t data_temp)
{
	FL_UART_WriteTXBuff(UART0, data_temp); 
}
/*
	串口1发送字符串
	send_str ： 发送的字符串地址
	lenght ：发送的字符串长度
*/
void Uart0_SendString(uint8_t *send_str,uint8_t length)
{
	while(length!=0)
	{
		Uart0_SendData(*send_str ++) ;
		length --;
	}
}
/*
	串口发送完成标志
*/
void Uart0_ClearTxBusy(void)
{
	uart0_tx_busy = 0;
}
/**
  * @brief  UART0 Initialization function
  * @param  void
  * @retval None
  */
void MF_UART1_Init(void)
{
	FL_GPIO_InitTypeDef    GPIO_InitStruct;

	FL_UART_InitTypeDef    UART1_InitStruct;

	/* PA2 UART0_RX */ 
	GPIO_InitStruct.pin = FL_GPIO_PIN_13;
	GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
	GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.pull = FL_ENABLE;
	GPIO_InitStruct.remapPin = FL_DISABLE;
	FL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/* PA3 UART0_TX */ 
	GPIO_InitStruct.pin = FL_GPIO_PIN_14;
	GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
	GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.pull = FL_DISABLE;
	GPIO_InitStruct.remapPin = FL_DISABLE;
	FL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	UART1_InitStruct.clockSrc = FL_RCC_UART1_CLK_SOURCE_APB1CLK;
	UART1_InitStruct.baudRate = 115200;
	UART1_InitStruct.dataWidth = FL_UART_DATA_WIDTH_8B;
	UART1_InitStruct.stopBits = FL_UART_STOP_BIT_WIDTH_1B;
	UART1_InitStruct.parity = FL_UART_PARITY_NONE;
	UART1_InitStruct.transferDirection = FL_UART_DIRECTION_TX_RX;

	FL_UART_Init(UART1, &UART1_InitStruct);  
	uart1_tx_busy = 0;

}
 
/**
  * @brief  UART0 Interrupt Initialization function
  * @param  void
  * @retval None
  */
void MF_UART1_Interrupt_Init(void)
{
	FL_NVIC_ConfigTypeDef    InterruptConfigStruct;	
	
	FL_UART_ClearFlag_RXBuffFull(UART1);
	FL_UART_EnableIT_RXBuffFull(UART1);           //使能接收中断

	FL_UART_ClearFlag_TXShiftBuffEmpty(UART1);
	FL_UART_EnableIT_TXShiftBuffEmpty(UART1);  
	
	InterruptConfigStruct.preemptPriority = 1;
	FL_NVIC_Init(&InterruptConfigStruct, UART1_IRQn);	
}
/*
	串口1发送字节函数
	data_temp : 发送的字节
*/
void Uart1_SendData(uint8_t data_temp)
{
	uart1_tx_busy = 1;
	FL_UART_WriteTXBuff(UART1, data_temp); 
	while(uart1_tx_busy);
}
/*
	串口1直接发送
	data_temp : 发送的字节
*/
void Uart1_SendData_IT(uint8_t data_temp)
{
	FL_UART_WriteTXBuff(UART1, data_temp); 
}
/*
	串口1发送字符串
	send_str ： 发送的字符串地址
	lenght ：发送的字符串长度
*/
void Uart1_SendString(uint8_t *send_str,uint8_t length)
{
	while(length!=0)
	{
		Uart1_SendData(*send_str ++) ;
		length --;
	}
}
/*
	串口发送完成标志
*/
void Uart1_ClearTxBusy(void)
{
	uart1_tx_busy = 0;
}
/**
  * @brief  UART4 Initialization function
  * @param  void
  * @retval None
  */
void MF_UART4_Init(void)
{
	FL_GPIO_InitTypeDef    GPIO_InitStruct;

	FL_UART_InitTypeDef    UART4_InitStruct;

	/* PA0 UART4_RX */ 
	GPIO_InitStruct.pin = FL_GPIO_PIN_0;
	GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
	GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.pull = FL_ENABLE;
	GPIO_InitStruct.remapPin = FL_DISABLE;
	FL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/* PA1 UART4_TX */ 
	GPIO_InitStruct.pin = FL_GPIO_PIN_1;
	GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
	GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.pull = FL_DISABLE;
	GPIO_InitStruct.remapPin = FL_DISABLE;
	FL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	UART4_InitStruct.clockSrc = NULL;
	UART4_InitStruct.baudRate = 19200;
	UART4_InitStruct.dataWidth = FL_UART_DATA_WIDTH_8B;
	UART4_InitStruct.stopBits = FL_UART_STOP_BIT_WIDTH_1B;
	UART4_InitStruct.parity = FL_UART_PARITY_NONE;
	UART4_InitStruct.transferDirection = FL_UART_DIRECTION_TX_RX;

	FL_UART_Init(UART4, &UART4_InitStruct);    
}
 
/**
  * @brief  UART4 Interrupt Initialization function
  * @param  void
  * @retval None
  */
void MF_UART4_Interrupt_Init(void)
{
	FL_NVIC_ConfigTypeDef    InterruptConfigStruct;
	
	FL_UART_ClearFlag_RXBuffFull(UART4);
	FL_UART_EnableIT_RXBuffFull(UART4);           //使能接收中断

	FL_UART_ClearFlag_TXShiftBuffEmpty(UART4);
	FL_UART_EnableIT_TXShiftBuffEmpty(UART4);  
	
	InterruptConfigStruct.preemptPriority = 1;
	FL_NVIC_Init(&InterruptConfigStruct, UART4_IRQn);	
	
	uart4_tx_busy = 0;
}
/*
	串口发送字节函数
	data_temp : 发送的字节
*/
void Uart4_SendData(uint8_t data_temp)
{
	uart4_tx_busy = 1;
	FL_UART_WriteTXBuff(UART4, data_temp); 
	while(uart4_tx_busy);
}
/*
	串口发送字符串
	send_str ： 发送的字符串地址
	lenght ：发送的字符串长度
*/
void Uart4_SendString(uint8_t *send_str,uint8_t length)
{
	while(length!=0)
	{
		Uart4_SendData(*send_str ++) ;
		length --;
	}
}
/*
	串口发送完成标志
*/
void Uart4_ClearTxBusy(void)
{
	uart4_tx_busy = 0;
}
/**
  * @brief  UART5 Initialization function
  * @param  void
  * @retval None
  */
void MF_UART5_Init(void)
{
	FL_GPIO_InitTypeDef    GPIO_InitStruct;

	FL_UART_InitTypeDef    UART5_InitStruct;

	/* PC4 UART5_RX */ 
	GPIO_InitStruct.pin = FL_GPIO_PIN_4;
	GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
	GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.pull = FL_ENABLE;
	GPIO_InitStruct.remapPin = FL_DISABLE;
	FL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/* PC5 UART5_TX */ 
	GPIO_InitStruct.pin = FL_GPIO_PIN_5;
	GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
	GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.pull = FL_DISABLE;
	GPIO_InitStruct.remapPin = FL_DISABLE;
	FL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	UART5_InitStruct.clockSrc = NULL;
	UART5_InitStruct.baudRate = 9600;
	UART5_InitStruct.dataWidth = FL_UART_DATA_WIDTH_8B;
	UART5_InitStruct.stopBits = FL_UART_STOP_BIT_WIDTH_1B;
	UART5_InitStruct.parity = FL_UART_PARITY_NONE;
	UART5_InitStruct.transferDirection = FL_UART_DIRECTION_TX_RX;

	FL_UART_Init(UART5, &UART5_InitStruct);  
  
}
 
/**
  * @brief  UART5 Interrupt Initialization function
  * @param  void
  * @retval None
  */
void MF_UART5_Interrupt_Init(void)
{
	FL_NVIC_ConfigTypeDef    InterruptConfigStruct;
	FL_UART_ClearFlag_RXBuffFull(UART5);
	FL_UART_EnableIT_RXBuffFull(UART5);           //使能接收中断

	FL_UART_ClearFlag_TXShiftBuffEmpty(UART5);
	FL_UART_EnableIT_TXShiftBuffEmpty(UART5);
	
	InterruptConfigStruct.preemptPriority = 1;
	FL_NVIC_Init(&InterruptConfigStruct, UART5_IRQn);		
	
	uart5_tx_busy = 0;
}
/*
	串口发送字节函数
	data_temp : 发送的字节
*/
void Uart5_SendData(uint8_t data_temp)
{
	uart5_tx_busy = 1;

	FL_UART_WriteTXBuff(UART5, data_temp); 

	while(uart5_tx_busy);
}
/*
	串口发送字符串
	send_str ： 发送的字符串地址
	lenght ：发送的字符串长度
*/
void Uart5_SendString(uint8_t *send_str,uint8_t length)
{
	while(length!=0)
	{
		Uart5_SendData(*send_str ++) ;
		length --;
	}
}
/*
	串口发送完成标志
*/
void Uart5_ClearTxBusy(void)
{
	uart5_tx_busy = 0;
}
/*------------------------------------------------------------------------------------*/
/*
	串口初始化调用
*/
void Uart_Init(void)
{
	/* Initial UART0 */
	MF_UART0_Init();
	MF_UART0_Interrupt_Init();

	/* Initial UART1 */
	MF_UART1_Init();
	MF_UART1_Interrupt_Init();
	
	/* Initial UART4 */
	MF_UART4_Init();
	MF_UART4_Interrupt_Init();

	/* Initial UART5 */
	MF_UART5_Init();
	MF_UART5_Interrupt_Init();
}

// 将 printf 输出重定向到 USART1_SendByte
int fputc(int ch, FILE *f)
{
    FL_UART_WriteTXBuff(UART0,(uint8_t)ch);                      /* 将发送数据写入发送寄存器 */
    while(FL_UART_IsActiveFlag_TXBuffEmpty(UART0) != 0x01UL){};  /* 等待发送完成 */
    return ch;
}





