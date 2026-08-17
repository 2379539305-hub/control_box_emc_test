#ifndef __MODUL_ADC_POSITION_H
#define __MODUL_ADC_POSITION_H

#include "main.h"
#include "system.h"

#include "modul_motor.h"
#define MOTOR_ADC_START_VALUE  5

typedef struct
{
	//需要赋予实体参数
	unsigned char 	*motor_relay_dir;
	unsigned short 	*motorADC_value;
	unsigned short 	*hall_run_num;	
	//中间变量
	unsigned short AdcSaveLen;
	unsigned short AdcSave[3];
	unsigned short AdcSlopeTemp;
	unsigned short AdcCheck;
	unsigned short AdcInterAve[3];
	unsigned short AdcInterAveError[3];
	unsigned char  AdcInterAveLen;
	unsigned char  AdcInterAveLenError;
	unsigned short AdcInterValue;	
	unsigned short AdcLevelCheck;
	unsigned short AdcLevelAve[3];
	unsigned short AdcLevelAveError[3];
	unsigned char  AdcLevelAveLen;
	unsigned char  AdcLevelAveLenError;		
	unsigned char  AdcCheckError;	
}MOTOR_ADC_PARA;


void Motor_Current_saveTask(MOTOR_ADC_PARA *Motor_ParaStu);

#endif
