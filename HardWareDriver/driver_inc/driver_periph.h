#ifndef __DRIVER_PERIPH_H
#define __DRIVER_PERIPH_H

#include "system.h"
#include "driver_config.h"
#include "app_config.h"


#define PB4_CONTROL_GPIO       GPIOB
#define PB4_CONTROL_GPIO_PIN   FL_GPIO_PIN_4
#define PB5_CONTROL_GPIO       GPIOB
#define PB5_CONTROL_GPIO_PIN   FL_GPIO_PIN_5
#define PB6_CONTROL_GPIO       GPIOB
#define PB6_CONTROL_GPIO_PIN   FL_GPIO_PIN_6
#define PB7_CONTROL_GPIO       GPIOB
#define PB7_CONTROL_GPIO_PIN   FL_GPIO_PIN_7
#define PC0_CONTROL_GPIO       GPIOC
#define PC0_CONTROL_GPIO_PIN   FL_GPIO_PIN_0
#define PC1_CONTROL_GPIO       GPIOC
#define PC1_CONTROL_GPIO_PIN   FL_GPIO_PIN_1
#define PA4_CONTROL_GPIO       GPIOA
#define PA4_CONTROL_GPIO_PIN   FL_GPIO_PIN_4





void Periph_Init(void); //Õ‚…Ëøÿ÷∆

void Massager_M1_Speed(uint16_t Compare);
void Massager_M2_Speed(uint16_t Compare);
void Massager_M3_Speed(uint16_t Compare);
void Heat_Set_Pwm(uint16_t Compare);
void Led_Board_SW(uint8_t state);
void Led_OneColour_SW(uint8_t state);
void Music_SW(uint8_t state);

void Periph_PB4_Init(XS_MODE xs_mode);
void Periph_PB5_Init(XS_MODE xs_mode);
void Periph_PB6_Init(XS_MODE xs_mode);
void Periph_PB7_Init(XS_MODE xs_mode);
void Periph_PC0_Init(XS_MODE xs_mode);
void Periph_PC1_Init(XS_MODE xs_mode);
void Periph_PA4_Init(XS_MODE xs_mode);
#endif






