#ifndef __DRIVER_MOTOR_H
#define __DRIVER_MOTOR_H

#include "system.h"
#include "driver_config.h"



/*----------------------继电器IO配置------------------------*/
//电机1
#define MOTOR1_UP_GPIO         GPIOB
#define MOTOR1_UP_GPIO_PIN     FL_GPIO_PIN_8

#define MOTOR1_DOWN_GPIO         GPIOB
#define MOTOR1_DOWN_GPIO_PIN     FL_GPIO_PIN_2

//电机2
#define MOTOR2_UP_GPIO         GPIOB
#define MOTOR2_UP_GPIO_PIN     FL_GPIO_PIN_9

#define MOTOR2_DOWN_GPIO         GPIOB
#define MOTOR2_DOWN_GPIO_PIN     FL_GPIO_PIN_3

//电机3
#define MOTOR3_UP_GPIO         GPIOD
#define MOTOR3_UP_GPIO_PIN     FL_GPIO_PIN_5

#define MOTOR3_DOWN_GPIO         GPIOA
#define MOTOR3_DOWN_GPIO_PIN     FL_GPIO_PIN_7

//电机4
#define MOTOR4_UP_GPIO         GPIOA
#define MOTOR4_UP_GPIO_PIN     FL_GPIO_PIN_5

#define MOTOR4_DOWN_GPIO         GPIOA
#define MOTOR4_DOWN_GPIO_PIN     FL_GPIO_PIN_6


//电机5
#define MOTOR5_UP_GPIO         GPIOA
#define MOTOR5_UP_GPIO_PIN     FL_GPIO_PIN_9

#define MOTOR5_DOWN_GPIO        GPIOA
#define MOTOR5_DOWN_GPIO_PIN    FL_GPIO_PIN_10

//电机6
#define MOTOR6_UP_GPIO         GPIOB
#define MOTOR6_UP_GPIO_PIN     FL_GPIO_PIN_12

#define MOTOR6_DOWN_GPIO        GPIOD
#define MOTOR6_DOWN_GPIO_PIN    FL_GPIO_PIN_4
/*-----------------------------------------------------------*/

//速度端口
#define MOTOR1_SPEED_GPIO   	  GPIOB
#define MOTOR1_SPEED_GPIO_PIN	  FL_GPIO_PIN_10

#define MOTOR2_SPEED_GPIO   	  GPIOB
#define MOTOR2_SPEED_GPIO_PIN	  FL_GPIO_PIN_11

#define MOTOR3_SPEED_GPIO   	  GPIOC
#define MOTOR3_SPEED_GPIO_PIN	  FL_GPIO_PIN_11

#define MOTOR4_SPEED_GPIO   	  GPIOC
#define MOTOR4_SPEED_GPIO_PIN	  FL_GPIO_PIN_12

/*----------------------------霍尔端口----------------------------------*/
#define HALL_M1A_GPIO   		  GPIOC
#define HALL_M1A_GPIO_PIN		  FL_GPIO_PIN_2

#define HALL_M2A_GPIO   		  GPIOD
#define HALL_M2A_GPIO_PIN		  FL_GPIO_PIN_3

#define HALL_M3A_GPIO   		  GPIOD
#define HALL_M3A_GPIO_PIN		  FL_GPIO_PIN_6

#define HALL_M4A_GPIO   		  GPIOA
#define HALL_M4A_GPIO_PIN		  FL_GPIO_PIN_11

#define HALL_M5A_GPIO   		  GPIOA
#define HALL_M5A_GPIO_PIN		  FL_GPIO_PIN_12

#define HALL_M6A_GPIO   		  GPIOB
#define HALL_M6A_GPIO_PIN		  FL_GPIO_PIN_0

/*------------------------------------------------------------------------*/


void Motor_Init(void);

void Motor_M1_Up(void);
void Motor_M1_Down(void);
void Motor_M1_Stop(void);
void Motor_M1_Speed(uint16_t Compare);


void Motor_M2_Up(void);
void Motor_M2_Down(void);
void Motor_M2_Stop(void);
void Motor_M2_Speed(uint16_t Compare);


void Motor_M3_Up(void);
void Motor_M3_Down(void);
void Motor_M3_Stop(void);
void Motor_M3_Speed(uint16_t Compare);

void Motor_M4_Up(void);
void Motor_M4_Down(void);
void Motor_M4_Stop(void);
void Motor_M4_Speed(uint16_t Compare);

void Motor_M5_Up(void);
void Motor_M5_Down(void);
void Motor_M5_Stop(void);
void Motor_M5_Speed(uint16_t Compare);

void Motor_M6_Up(void);
void Motor_M6_Down(void);
void Motor_M6_Stop(void);
void Motor_M6_Speed(uint16_t Compare);

/*----------------------霍尔检测---------------------------*/
void Hall_Init(void);
uint8_t Get_M1A_Hall_Level(void);
uint8_t Get_M2A_Hall_Level(void);
uint8_t Get_M3A_Hall_Level(void);
uint8_t Get_M4A_Hall_Level(void);
uint8_t Get_M5A_Hall_Level(void);
uint8_t Get_M6A_Hall_Level(void);
#endif
