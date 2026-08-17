#include "driver_key.h"
#include "delay.h"

//按键端口初始化
void Key_Init(void)
{
	FL_GPIO_InitTypeDef    GPIO_InitStruct = {0};

	GPIO_InitStruct.pin        = (STUDY_GPIO_PIN | KEY1_GPIO_PIN | KEY2_GPIO_PIN | KEY3_GPIO_PIN);
	GPIO_InitStruct.mode       = FL_GPIO_MODE_INPUT;
	GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.pull       = FL_ENABLE;
	GPIO_InitStruct.remapPin   = FL_DISABLE;
	FL_GPIO_Init(GPIOA, &GPIO_InitStruct);	
}

//获取对码按键状态 (0:按下, 1:未按下)
uint8_t Get_StudyKey_State(void)
{
	if(0 == FL_GPIO_GetInputPin(STUDY_GPIO, STUDY_GPIO_PIN))
	{
		return 0;
	}
	return 1;
}

//获取按键1状态 (0:按下, 1:未按下)
uint8_t Get_Key1_State(void)
{
	if(0 == FL_GPIO_GetInputPin(KEY_GPIO, KEY1_GPIO_PIN))
	{
		return 0;
	}
	return 1;
}

//获取按键2状态 (0:按下, 1:未按下)
uint8_t Get_Key2_State(void)
{
	if(0 == FL_GPIO_GetInputPin(KEY_GPIO, KEY2_GPIO_PIN))
	{
		return 0;
	}
	return 1;
}

//获取按键3状态 (0:按下, 1:未按下)
uint8_t Get_Key3_State(void)
{
	if(0 == FL_GPIO_GetInputPin(KEY_GPIO, KEY3_GPIO_PIN))
	{
		return 0;
	}
	return 1;
}
