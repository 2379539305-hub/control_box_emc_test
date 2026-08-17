#ifndef __MODUL_A7105_H
#define __MODUL_A7105_H

#include "main.h"
#include "system.h"


#define A7105_FREE_LONG_TIME  (300/SYS_TIME_BASE)   //A7105接收超时
/*-----------------------A7105新旧配置---------------------------*/

/*---------------------------------------------------------------*/

extern unsigned char ID_USE[4];
extern unsigned char HJ_A7105_CODE[4];

/*---------------------------------------------------------------*/
unsigned char A7105_Init(void);
unsigned char A7105_Study_Mode(void);
void A7105_Recv_Mode(void);
unsigned char A7105_Get_KeyState(void);
void A7105_TimerManager(void);
unsigned char A7105_Comm_Free(void);

unsigned char A7105_Read_CHdata(unsigned char mode);


#endif






