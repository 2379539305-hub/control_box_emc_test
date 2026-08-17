#include "driver_led.h"

//指示灯IO初始化 (LED1: PA12, LED2: PB1)
void LED_Init(void)
{
	FL_GPIO_InitTypeDef    GPIO_InitStruct = {0};
	
	GPIO_InitStruct.pin = LED1_GPIO_PIN;
	GPIO_InitStruct.mode = FL_GPIO_MODE_OUTPUT;
	GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.pull = FL_DISABLE;
	GPIO_InitStruct.remapPin = FL_DISABLE;
	FL_GPIO_Init(LED1_GPIO, &GPIO_InitStruct);

	GPIO_InitStruct.pin = LED2_GPIO_PIN;
	FL_GPIO_Init(LED2_GPIO, &GPIO_InitStruct);
	
	LED_OFF();
}

//指示灯全开 (低电平点亮)
void LED_ON(void)
{
	FL_GPIO_ResetOutputPin(LED1_GPIO, LED1_GPIO_PIN);
	FL_GPIO_ResetOutputPin(LED2_GPIO, LED2_GPIO_PIN);
}

//指示灯全关 (高电平熄灭)
void LED_OFF(void)
{
	FL_GPIO_SetOutputPin(LED1_GPIO, LED1_GPIO_PIN);
	FL_GPIO_SetOutputPin(LED2_GPIO, LED2_GPIO_PIN);
}

void LED1_ON(void)
{
	FL_GPIO_ResetOutputPin(LED1_GPIO, LED1_GPIO_PIN);
}

void LED1_OFF(void)
{
	FL_GPIO_SetOutputPin(LED1_GPIO, LED1_GPIO_PIN);
}

void LED2_ON(void)
{
	FL_GPIO_ResetOutputPin(LED2_GPIO, LED2_GPIO_PIN);
}

void LED2_OFF(void)
{
	FL_GPIO_SetOutputPin(LED2_GPIO, LED2_GPIO_PIN);
}

void LED1_Toggle(void)
{
	FL_GPIO_ToggleOutputPin(LED1_GPIO, LED1_GPIO_PIN);
}

void LED2_Toggle(void)
{
	FL_GPIO_ToggleOutputPin(LED2_GPIO, LED2_GPIO_PIN);
}

//内置床底灯开
void UBL_ON(void)
{

}

//内置床底灯关
void UBL_OFF(void)
{

}
