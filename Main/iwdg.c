#include "iwdg.h"

#define  ENABLE_IWDG   0

/*
* 描述： 初始化看门狗
* 参数： 无
* 返回： 无*/
void Iwdg_Init(void)
{
	//使能看门狗寄存器，定时2.276s后复位
	#if ENABLE_IWDG
	FL_IWDT_InitTypeDef    IWDT_InitStruct;

	IWDT_InitStruct.overflowPeriod = FL_IWDT_PERIOD_16000MS;
	IWDT_InitStruct.iwdtWindows = 0;

	FL_IWDT_Init(IWDT, &IWDT_InitStruct);
	#endif
}


/*
* 描述： 喂狗
* 参数： 无
* 返回： 无*/
void Iwdg_Clear(void)
{
	#if ENABLE_IWDG
	FL_IWDT_ReloadCounter(IWDT);
	#endif
}	



