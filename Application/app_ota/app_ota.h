#ifndef _APP_OTA_H_
#define _APP_OTA_H_

#include "system.h"

//APP起始地址（根据实际分区表修改）
#define APPLICATION_A_ADDRESS_OFFSET    0x00005000
#define APPLICATION_B_ADDRESS_OFFSET    0x21400
#define APPLICATION_MAX_SIZE            0x1C400

//RAM地址范围（根据芯片手册修改）
#define RAM_START_ADDR    0x20000000
#define RAM_END_ADDR      0x20008000

void App_Ota_Init(void);
void App_Ota_Process(void);
void App_Ota_RxServer(unsigned char uart_recv_temp);
void App_Ota_Timer_1ms(void);
#endif
