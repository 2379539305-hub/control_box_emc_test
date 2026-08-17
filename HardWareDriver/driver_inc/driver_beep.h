#ifndef __DRIVER_BEEP_H
#define __DRIVER_BEEP_H

#include "system.h"
#include "driver_config.h"


#define BEEP_GPIO		       GPIOA
#define BEEP_GPIO_PIN		   FL_GPIO_PIN_8                        



void Beep_Init(void);
void Beep_SoundTask(void);
void Beep_TimerSingTask(void);
void Beep_ON(void);
void Beep_OFF(void);
uint8_t Beep_SingSetPara(uint16_t sing_hz , uint8_t num);
void Beep_StopSing(void);
#endif






