#ifndef __DRIVER_UART_H
#define __DRIVER_UART_H

#include "system.h"




void Uart_Init(void);

void Uart0_SendData(uint8_t data_temp);
void Uart0_SendData_IT(uint8_t data_temp);
void Uart0_SendString(uint8_t *send_str,uint8_t length);
void Uart0_ClearTxBusy(void);

void Uart1_SendData(uint8_t data_temp);
void Uart1_SendString(uint8_t *send_str,uint8_t length);
void Uart1_ClearTxBusy(void);


void Uart4_SendData(uint8_t data_temp);
void Uart4_SendString(uint8_t *send_str,uint8_t length);
void Uart4_ClearTxBusy(void);

void Uart5_SendData(uint8_t data_temp);
void Uart5_SendString(uint8_t *send_str,uint8_t length);
void Uart5_ClearTxBusy(void);
#endif






