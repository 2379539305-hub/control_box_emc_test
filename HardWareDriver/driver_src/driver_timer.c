#include "driver_timer.h"
#include "driver_config.h"


/*
* 描述： 定时器0配置初始化 (100uS)
* 参数： 无
* 返回： 无
*/
void Time100us_Init(void) 
{
	FL_LPTIM32_InitTypeDef lptim32;
	FL_NVIC_ConfigTypeDef nvic;

	lptim32.clockSource = FL_RCC_LPTIM32_CLK_SOURCE_APB1CLK;
	lptim32.prescalerClockSource = FL_LPTIM32_CLK_SOURCE_INTERNAL;
	lptim32.prescaler = FL_LPTIM32_PSC_DIV8;
	lptim32.autoReload = 800;
	lptim32.mode = FL_LPTIM32_OPERATION_MODE_NORMAL;
	lptim32.onePulseMode = FL_LPTIM32_ONE_PULSE_MODE_CONTINUOUS;
	lptim32.countEdge = FL_LPTIM32_ETR_COUNT_EDGE_RISING;
	lptim32.triggerEdge = FL_LPTIM32_ETR_TRIGGER_EDGE_RISING;

	FL_LPTIM32_Init(LPTIM32, &lptim32);

	FL_LPTIM32_ClearFlag_Update(LPTIM32);
	FL_LPTIM32_EnableIT_Update(LPTIM32);

	nvic.preemptPriority = 0x02;
	FL_NVIC_Init(&nvic, LPTIM_IRQn);
	
	FL_LPTIM32_Enable(LPTIM32);
}
/*
* 描述： 定时器3配置初始化 (1ms)
* 参数： 无
* 返回： 无
*/
void Time1ms_Init(void)
{
	FL_BSTIM32_InitTypeDef    defaultInitStruct;

	defaultInitStruct.prescaler       = 640-1;
	defaultInitStruct.autoReload      = 100-1;
	defaultInitStruct.autoReloadState = FL_ENABLE;
	defaultInitStruct.clockSource     = FL_RCC_BSTIM32_CLK_SOURCE_APB2CLK;

	FL_BSTIM32_Init(BSTIM32,&defaultInitStruct );

	FL_BSTIM32_ClearFlag_Update(BSTIM32);
	FL_BSTIM32_Enable(BSTIM32);
	
	FL_BSTIM32_ClearFlag_Update(BSTIM32);    /* 清除计数器中断标志位 */
	FL_BSTIM32_EnableIT_Update(BSTIM32);     /* 开启计数器中断 */
	
	NVIC_DisableIRQ(BSTIM_IRQn);
	NVIC_SetPriority(BSTIM_IRQn, 2);        /* 中断优先级配置 */
	NVIC_EnableIRQ(BSTIM_IRQn);	
}












