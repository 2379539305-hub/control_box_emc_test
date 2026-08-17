#ifndef __APP_VITA_H
#define __APP_VITA_H

#include "main.h"
#include "system.h"


typedef struct
{
	unsigned char snore_step;
	unsigned char snore_value;
	unsigned char heart_value;
	unsigned char breathe_value;
}VITA_STRUCT;


extern VITA_STRUCT  Vita_EMM_Stu;

extern VITA_STRUCT  Vita_Left_Stu;
extern VITA_STRUCT  Vita_Right_Stu;
extern unsigned char vita_key_state;

unsigned char Get_Auto_SnoreState(void);
void EMM_Snore_Control(void);
void Snore_TimeManagerTask(void);

extern unsigned char Vita_Ble_RadioName[9];
#endif






