#ifndef __DRIVER_LED_H
#define __DRIVER_LED_H

#include "system.h"
#include "driver_config.h"

// 原理图对应：
// LED1 (Net: MODE)           -> PA12
// LED2 (Net: PowerLed_ctrl)  -> PB1

#define LED1_GPIO		    GPIOA
#define LED1_GPIO_PIN		FL_GPIO_PIN_12

#define LED2_GPIO		    GPIOB
#define LED2_GPIO_PIN		FL_GPIO_PIN_1

void LED_Init(void);
void LED_ON(void);
void LED_OFF(void);
void LED1_ON(void);
void LED1_OFF(void);
void LED2_ON(void);
void LED2_OFF(void);
void LED1_Toggle(void);
void LED2_Toggle(void);
void UBL_ON(void);
void UBL_OFF(void);

#endif
