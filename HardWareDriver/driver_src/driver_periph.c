#include "driver_periph.h"
#include "app_config.h"

//外设控制
void Periph_Init(void) 
{
	FL_GPIO_InitTypeDef    GPIO_InitStruct;

	FL_ATIM_OC_InitTypeDef    ATIM_InitStruct;

	FL_ATIM_InitTypeDef    TimerBase_InitStruct;	
		
	FL_GPTIM_InitTypeDef    GPTIMBase_InitStruct;	
	
	FL_GPTIM_OC_InitTypeDef    GPTIM_InitStruct;
	
	TimerBase_InitStruct.clockSource = FL_RCC_ATIM_CLK_SOURCE_APB2CLK;
	TimerBase_InitStruct.prescaler = PERIPH_PWM_PSC;
	TimerBase_InitStruct.counterMode = FL_ATIM_COUNTER_DIR_UP;
	TimerBase_InitStruct.autoReload = PERIPH_MASSAGE_PWM_PERIOD;
	TimerBase_InitStruct.autoReloadState = FL_ENABLE;
	TimerBase_InitStruct.clockDivision = FL_ATIM_CLK_DIVISION_DIV1;
	TimerBase_InitStruct.repetitionCounter = 0;

	FL_ATIM_Init(ATIM, &TimerBase_InitStruct); 

	ATIM_InitStruct.OCMode = FL_ATIM_OC_MODE_PWM1;
	ATIM_InitStruct.OCState = FL_ENABLE;
	ATIM_InitStruct.OCNState = FL_DISABLE;
	ATIM_InitStruct.OCPolarity = FL_ATIM_OC_POLARITY_NORMAL;
	ATIM_InitStruct.OCNPolarity = FL_ATIM_OCN_POLARITY_NORMAL;
	ATIM_InitStruct.OCFastMode = FL_DISABLE;
	ATIM_InitStruct.OCPreload = FL_DISABLE;
	ATIM_InitStruct.compareValue = 0;
	ATIM_InitStruct.OCIdleState = FL_ATIM_OC_IDLE_STATE_LOW;
	ATIM_InitStruct.OCETRFStatus = FL_DISABLE;
	ATIM_InitStruct.OCNIdleState = FL_ATIM_OCN_IDLE_STATE_LOW;

	if(system_config.flags.pb4_config == HEAD_MASSAGE || system_config.flags.pb4_config == FOOT_MASSAGE || system_config.flags.pb4_config == LUMBAR_MASSAGE || system_config.flags.pb4_config == HEAT)
	{
		GPIO_InitStruct.pin = FL_GPIO_PIN_4;
		GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
		GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
		GPIO_InitStruct.pull = FL_DISABLE;
		GPIO_InitStruct.remapPin = FL_DISABLE;
		FL_GPIO_Init(GPIOB, &GPIO_InitStruct);
		FL_ATIM_OC_Init(ATIM, FL_ATIM_CHANNEL_1, &ATIM_InitStruct); 
	}
	else
	{
		GPIO_InitStruct.pin = FL_GPIO_PIN_4;
		GPIO_InitStruct.mode = FL_GPIO_MODE_OUTPUT;
		GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
		GPIO_InitStruct.pull = FL_DISABLE;
		GPIO_InitStruct.remapPin = FL_DISABLE;
		FL_GPIO_Init(GPIOB, &GPIO_InitStruct);
		if(system_config.flags.pb4_config == USB_MODE)
		{
			FL_GPIO_SetOutputPin(PB4_CONTROL_GPIO, PB4_CONTROL_GPIO_PIN);
		}
		else
		{
			FL_GPIO_ResetOutputPin(PB4_CONTROL_GPIO, PB4_CONTROL_GPIO_PIN);
		}
	}
	if(system_config.flags.pb5_config == HEAD_MASSAGE || system_config.flags.pb5_config == FOOT_MASSAGE || system_config.flags.pb5_config == LUMBAR_MASSAGE || system_config.flags.pb5_config == HEAT)
	{
		GPIO_InitStruct.pin = FL_GPIO_PIN_5;
		GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
		GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
		GPIO_InitStruct.pull = FL_DISABLE;
		GPIO_InitStruct.remapPin = FL_DISABLE;
		FL_GPIO_Init(GPIOB, &GPIO_InitStruct);
		FL_ATIM_OC_Init(ATIM, FL_ATIM_CHANNEL_2, &ATIM_InitStruct); 
	}
	else
	{
		GPIO_InitStruct.pin = FL_GPIO_PIN_5;
		GPIO_InitStruct.mode = FL_GPIO_MODE_OUTPUT;
		GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
		GPIO_InitStruct.pull = FL_DISABLE;
		GPIO_InitStruct.remapPin = FL_DISABLE;
		FL_GPIO_Init(GPIOB, &GPIO_InitStruct);
		if(system_config.flags.pb5_config == USB_MODE)
		{
			FL_GPIO_SetOutputPin(PB5_CONTROL_GPIO, PB5_CONTROL_GPIO_PIN);
		}
		else
		{
			FL_GPIO_ResetOutputPin(PB5_CONTROL_GPIO, PB5_CONTROL_GPIO_PIN);
		}
	}
	if(system_config.flags.pb6_config == HEAD_MASSAGE || system_config.flags.pb6_config == FOOT_MASSAGE || system_config.flags.pb6_config == LUMBAR_MASSAGE || system_config.flags.pb6_config == HEAT)
	{
		GPIO_InitStruct.pin = FL_GPIO_PIN_6;
		GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
		GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
		GPIO_InitStruct.pull = FL_DISABLE;
		GPIO_InitStruct.remapPin = FL_DISABLE;
		FL_GPIO_Init(GPIOB, &GPIO_InitStruct);
		FL_ATIM_OC_Init(ATIM, FL_ATIM_CHANNEL_3, &ATIM_InitStruct); 
	}
	else
	{
		GPIO_InitStruct.pin = FL_GPIO_PIN_6;
		GPIO_InitStruct.mode = FL_GPIO_MODE_OUTPUT;
		GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
		GPIO_InitStruct.pull = FL_DISABLE;
		GPIO_InitStruct.remapPin = FL_DISABLE;
		FL_GPIO_Init(GPIOB, &GPIO_InitStruct);
		if(system_config.flags.pb6_config == USB_MODE)
		{
			FL_GPIO_SetOutputPin(PB6_CONTROL_GPIO, PB6_CONTROL_GPIO_PIN);
		}
		else
		{
			FL_GPIO_ResetOutputPin(PB6_CONTROL_GPIO, PB6_CONTROL_GPIO_PIN);
		}
	}
	if(system_config.flags.pb7_config == HEAD_MASSAGE || system_config.flags.pb7_config == FOOT_MASSAGE || system_config.flags.pb7_config == LUMBAR_MASSAGE || system_config.flags.pb7_config == HEAT)
	{
		GPIO_InitStruct.pin = FL_GPIO_PIN_7;
		GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
		GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
		GPIO_InitStruct.pull = FL_DISABLE;
		GPIO_InitStruct.remapPin = FL_DISABLE;
		FL_GPIO_Init(GPIOB, &GPIO_InitStruct);
		FL_ATIM_OC_Init(ATIM, FL_ATIM_CHANNEL_4, &ATIM_InitStruct); 
	}
	else
	{
		GPIO_InitStruct.pin = FL_GPIO_PIN_7;
		GPIO_InitStruct.mode = FL_GPIO_MODE_OUTPUT;
		GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
		GPIO_InitStruct.pull = FL_DISABLE;
		GPIO_InitStruct.remapPin = FL_DISABLE;
		FL_GPIO_Init(GPIOB, &GPIO_InitStruct);
		if(system_config.flags.pb7_config == USB_MODE)
		{
			FL_GPIO_SetOutputPin(PB7_CONTROL_GPIO, PB7_CONTROL_GPIO_PIN);
		}
		else
		{
			FL_GPIO_ResetOutputPin(PB7_CONTROL_GPIO, PB7_CONTROL_GPIO_PIN);
		}
	}
	/*  ATIM_CH1-4  */ 
	
	
	//使能ATIM
	FL_ATIM_Enable(ATIM);
	FL_ATIM_EnableALLOutput(ATIM);

	//GPTIM1 定时器配置


	GPTIMBase_InitStruct.prescaler = PERIPH_UBL_PWM_PSC;
	GPTIMBase_InitStruct.counterMode = FL_GPTIM_COUNTER_DIR_UP;
	GPTIMBase_InitStruct.autoReload = PERIPH_UBL_PWM_PERIOD;
	GPTIMBase_InitStruct.autoReloadState = FL_ENABLE;
	GPTIMBase_InitStruct.clockDivision = FL_GPTIM_CLK_DIVISION_DIV1;
	FL_GPTIM_Init(GPTIM1,&GPTIMBase_InitStruct);


	GPTIM_InitStruct.OCMode = FL_GPTIM_OC_MODE_PWM1;
	GPTIM_InitStruct.OCPolarity = FL_GPTIM_OC_POLARITY_NORMAL;
	GPTIM_InitStruct.OCFastMode = FL_DISABLE;
	GPTIM_InitStruct.OCPreload = FL_DISABLE;
	GPTIM_InitStruct.compareValue = 0;
	GPTIM_InitStruct.OCETRFStatus = FL_DISABLE;
	if(system_config.flags.pc0_1_motor_pwm_enable == 0)
	{
		//PC1配置
		if(system_config.flags.pc1_config == COLD_WARM_UBL || system_config.flags.pc1_config == RGB_UBL)
		{
			GPIO_InitStruct.pin = FL_GPIO_PIN_1;
			GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
			GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
			GPIO_InitStruct.pull = FL_DISABLE;
			GPIO_InitStruct.remapPin = FL_DISABLE;
			FL_GPIO_Init(GPIOC, &GPIO_InitStruct);
			FL_GPTIM_OC_Init(GPTIM1,FL_GPTIM_CHANNEL_2,&GPTIM_InitStruct);
		}
		else
		{
			GPIO_InitStruct.pin = FL_GPIO_PIN_1;
			GPIO_InitStruct.mode = FL_GPIO_MODE_OUTPUT;
			GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
			GPIO_InitStruct.pull = FL_DISABLE;
			GPIO_InitStruct.remapPin = FL_DISABLE;
			FL_GPIO_Init(GPIOC, &GPIO_InitStruct);
			if(system_config.flags.pc1_config == USB_MODE)
			{
				FL_GPIO_SetOutputPin(PC1_CONTROL_GPIO, PC1_CONTROL_GPIO_PIN);
			}
			else
			{
				FL_GPIO_ResetOutputPin(PC1_CONTROL_GPIO, PC1_CONTROL_GPIO_PIN);
			}
		}
		//PC0配置
		if(system_config.flags.pc0_config == COLD_WARM_UBL || system_config.flags.pc0_config == RGB_UBL)
		{
			GPIO_InitStruct.pin = FL_GPIO_PIN_0;
			GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
			GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
			GPIO_InitStruct.pull = FL_DISABLE;
			GPIO_InitStruct.remapPin = FL_DISABLE;
			FL_GPIO_Init(GPIOC, &GPIO_InitStruct);
			FL_GPTIM_OC_Init(GPTIM1,FL_GPTIM_CHANNEL_1,&GPTIM_InitStruct);
		}
		else
		{
			GPIO_InitStruct.pin = FL_GPIO_PIN_0;
			GPIO_InitStruct.mode = FL_GPIO_MODE_OUTPUT;
			GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
			GPIO_InitStruct.pull = FL_DISABLE;
			GPIO_InitStruct.remapPin = FL_DISABLE;
			FL_GPIO_Init(GPIOC, &GPIO_InitStruct);
			if(system_config.flags.pc0_config == USB_MODE)
			{
				FL_GPIO_SetOutputPin(PC0_CONTROL_GPIO, PC0_CONTROL_GPIO_PIN);
			}
			else
			{
				FL_GPIO_ResetOutputPin(PC0_CONTROL_GPIO, PC0_CONTROL_GPIO_PIN);
			}
		}
	}
	//PA4配置
	if(system_config.flags.pa4_config == COLD_WARM_UBL || system_config.flags.pa4_config == RGB_UBL)
	{
		GPIO_InitStruct.pin = FL_GPIO_PIN_4;
		GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
		GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
		GPIO_InitStruct.pull = FL_DISABLE;
		GPIO_InitStruct.remapPin = FL_DISABLE;
		FL_GPIO_Init(GPIOA, &GPIO_InitStruct);
		FL_GPTIM_OC_Init(GPTIM1,FL_GPTIM_CHANNEL_3,&GPTIM_InitStruct);
	}
	else
	{
		GPIO_InitStruct.pin = FL_GPIO_PIN_4;
		GPIO_InitStruct.mode = FL_GPIO_MODE_OUTPUT;
		GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
		GPIO_InitStruct.pull = FL_DISABLE;
		GPIO_InitStruct.remapPin = FL_DISABLE;
		FL_GPIO_Init(GPIOA, &GPIO_InitStruct);
		if(system_config.flags.pa4_config == USB_MODE)
		{
			FL_GPIO_SetOutputPin(PA4_CONTROL_GPIO, PA4_CONTROL_GPIO_PIN);
		}
		else
		{
			FL_GPIO_ResetOutputPin(PA4_CONTROL_GPIO, PA4_CONTROL_GPIO_PIN);
		}
	}
	//使能GPTIM
	FL_GPTIM_Enable(GPTIM1);
	
}

void Periph_PB4_Init(XS_MODE xs_mode)
{
	system_config.flags.pb4_config = xs_mode;
}
void Periph_PB5_Init(XS_MODE xs_mode)
{
	system_config.flags.pb5_config = xs_mode;
}
void Periph_PB6_Init(XS_MODE xs_mode)
{
	system_config.flags.pb6_config = xs_mode;
}
void Periph_PB7_Init(XS_MODE xs_mode)
{
	system_config.flags.pb7_config = xs_mode;
}
void Periph_PC0_Init(XS_MODE xs_mode)
{
	system_config.flags.pc0_config = xs_mode;
}
void Periph_PC1_Init(XS_MODE xs_mode)
{
	system_config.flags.pc1_config = xs_mode;
}
void Periph_PA4_Init(XS_MODE xs_mode)
{
	system_config.flags.pa4_config = xs_mode;
}




//端口PB4输出控制
void Periph_PB4_SW(uint8_t sw_state)
{
	if(sw_state != 0)
	{
		FL_GPIO_SetOutputPin(PB4_CONTROL_GPIO, PB4_CONTROL_GPIO_PIN);
	}
	else
	{
		FL_GPIO_ResetOutputPin(PB4_CONTROL_GPIO, PB4_CONTROL_GPIO_PIN);
	}
}
//端口PB5输出控制
void Periph_PB5_SW(uint8_t sw_state)
{
	if(sw_state != 0)
	{
		FL_GPIO_SetOutputPin(PB5_CONTROL_GPIO, PB5_CONTROL_GPIO_PIN);
	}
	else
	{
		FL_GPIO_ResetOutputPin(PB5_CONTROL_GPIO, PB5_CONTROL_GPIO_PIN);
	}
}
//端口PB6输出控制
void Periph_PB6_SW(uint8_t sw_state)
{
	if(sw_state != 0)
	{
		FL_GPIO_SetOutputPin(PB6_CONTROL_GPIO, PB6_CONTROL_GPIO_PIN);
	}
	else
	{
		FL_GPIO_ResetOutputPin(PB6_CONTROL_GPIO, PB6_CONTROL_GPIO_PIN);
	}
}
//端口PB7输出控制
void Periph_PB7_SW(uint8_t sw_state)
{
	if(sw_state != 0)
	{
		FL_GPIO_SetOutputPin(PB7_CONTROL_GPIO, PB7_CONTROL_GPIO_PIN);
	}
	else
	{
		FL_GPIO_ResetOutputPin(PB7_CONTROL_GPIO, PB7_CONTROL_GPIO_PIN);
	}
}
//端口PC0输出控制
void Periph_PC0_SW(uint8_t sw_state)
{
	if(system_config.flags.pc0_1_motor_pwm_enable == 1)
	{
		return;
	}
	if(sw_state != 0)
	{
		FL_GPIO_SetOutputPin(PC0_CONTROL_GPIO, PC0_CONTROL_GPIO_PIN);
	}
	else
	{
		FL_GPIO_ResetOutputPin(PC0_CONTROL_GPIO, PC0_CONTROL_GPIO_PIN);
	}
}
//端口PC1输出控制
void Periph_PC1_SW(uint8_t sw_state)
{
	if(system_config.flags.pc0_1_motor_pwm_enable == 1)
	{
		return;
	}
	if(sw_state != 0)
	{
		FL_GPIO_SetOutputPin(PC1_CONTROL_GPIO, PC1_CONTROL_GPIO_PIN);
	}
	else
	{
		FL_GPIO_ResetOutputPin(PC1_CONTROL_GPIO, PC1_CONTROL_GPIO_PIN);
	}
}
//端口PA4输出控制
void Periph_PA4_SW(uint8_t sw_state)
{
	if(sw_state != 0)
	{
		FL_GPIO_SetOutputPin(PA4_CONTROL_GPIO, PA4_CONTROL_GPIO_PIN);
	}
	else
	{
		FL_GPIO_ResetOutputPin(PA4_CONTROL_GPIO, PA4_CONTROL_GPIO_PIN);
	}
}

//端口PB4PWM控制
void Periph_PB4_Speed(uint16_t Compare)
{
	FL_ATIM_WriteCompareCH1(ATIM,Compare);
}
//端口PB5PWM控制
void Periph_PB5_Speed(uint16_t Compare)
{
	FL_ATIM_WriteCompareCH2(ATIM,Compare);
}
//端口PB6PWM控制
void Periph_PB6_Speed(uint16_t Compare)
{
	FL_ATIM_WriteCompareCH3(ATIM,Compare);
}
//端口PB7PWM控制
void Periph_PB7_Speed(uint16_t Compare)
{
	FL_ATIM_WriteCompareCH4(ATIM,Compare);
}
//端口PC0PWM控制
void Periph_PC0_Speed(uint16_t Compare)
{
	if(system_config.flags.pc0_1_motor_pwm_enable == 1)
	{
		return;
	}
	FL_GPTIM_WriteCompareCH1(GPTIM1,Compare);
}
//端口PC1PWM控制
void Periph_PC1_Speed(uint16_t Compare)
{
	if(system_config.flags.pc0_1_motor_pwm_enable == 1)
	{
		return;
	}
	FL_GPTIM_WriteCompareCH2(GPTIM1,Compare);
}
//端口PA4PWM控制
void Periph_PA4_Speed(uint16_t Compare)
{
	FL_GPTIM_WriteCompareCH3(GPTIM1,Compare);
}




//外设端口PWM设置
void Periph_Set_Pwm(XS_MODE xs_mode,uint16_t Compare)
{
	if(system_config.flags.pb4_config == xs_mode)
	{
		Periph_PB4_Speed(Compare);
	}
	if(system_config.flags.pb5_config == xs_mode)
	{
		Periph_PB5_Speed(Compare);
	}
	if(system_config.flags.pb6_config == xs_mode)
	{
		Periph_PB6_Speed(Compare);
	}
	if(system_config.flags.pb7_config == xs_mode)
	{
		Periph_PB7_Speed(Compare);
	}	
}
//外设端口开关控制
void Periph_Set_Switch(XS_MODE xs_mode,unsigned char state)
{
	if(system_config.flags.pb4_config == xs_mode)
	{
		Periph_PB4_SW(state);
	}
	if(system_config.flags.pb5_config == xs_mode)
	{
		Periph_PB5_SW(state);
	}
	if(system_config.flags.pb6_config == xs_mode)
	{
		Periph_PB6_SW(state);
	}
	if(system_config.flags.pb7_config == xs_mode)
	{
		Periph_PB7_SW(state);
	}
	if(system_config.flags.pc0_config == xs_mode)
	{
		Periph_PC0_SW(state);
	}
	if(system_config.flags.pc1_config == xs_mode)
	{
		Periph_PC1_SW(state);
	}
	if(system_config.flags.pa4_config == xs_mode)
	{
		Periph_PA4_SW(state);
	}
}
//按摩器1速度控制
void Massager_M1_Speed(uint16_t Compare)
{
	Periph_Set_Pwm(HEAD_MASSAGE,Compare);
}
//按摩器2速度控制
void Massager_M2_Speed(uint16_t Compare)
{
	Periph_Set_Pwm(FOOT_MASSAGE,Compare);
}
//按摩器3速度控制
void Massager_M3_Speed(uint16_t Compare)
{
	Periph_Set_Pwm(LUMBAR_MASSAGE,Compare);
}
//加热垫占空比设置
void Heat_Set_Pwm(uint16_t Compare)
{
	Periph_Set_Pwm(HEAT,Compare);
}
//灯牌控制
void Led_Board_SW(unsigned char state)
{
	Periph_Set_Switch(LED_BOARD,state);
}
//单色床底灯控制
void Led_OneColour_SW(unsigned char state)
{
	Periph_Set_Switch(UBL,state);
}

//外接音响设备电源控制
void Music_SW(unsigned char state)
{
	Periph_Set_Switch(MUSIC_SW,state);
}
