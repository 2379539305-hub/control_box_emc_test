#ifndef __DRIVER_KEY_H
#define __DRIVER_KEY_H

#include "system.h"
#include "driver_config.h"

#define STUDY_GPIO		    GPIOA
#define STUDY_GPIO_PIN		FL_GPIO_PIN_14

#define KEY_GPIO		    GPIOA
#define KEY1_GPIO_PIN		FL_GPIO_PIN_9
#define KEY2_GPIO_PIN		FL_GPIO_PIN_10
#define KEY3_GPIO_PIN		FL_GPIO_PIN_11


uint8_t Get_StudyKey_State(void);
uint8_t Get_Key1_State(void);
uint8_t Get_Key2_State(void);
uint8_t Get_Key3_State(void);

void Key_Init(void);
#endif
