#ifndef __DRIVER_BLE_H
#define __DRIVER_BLE_H

#include "system.h"
#include "driver_config.h"


#define BLE_STATE_GPIO		    GPIOC
#define BLE_STATE_GPIO_PIN		FL_GPIO_PIN_3


void BleBlueTooth_HardInit(void);
void BleBlueTooth_SendData(uint8_t send_data);
void BleBlueTooth_SendString(uint8_t *send_str,uint8_t length);
uint8_t BleBlueTooth_Connect_State(void);



#endif






