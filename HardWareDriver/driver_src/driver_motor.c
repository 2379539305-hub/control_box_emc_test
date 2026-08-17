#include "driver_motor.h"
#include "app_config.h"
//16kHZ
#define MOTOR_PWM_PSC      15
#define MOTOR_PWM_PERIOD   249

/*******************************
* @brief  GPTIM0初始化      推杆电机初始化：系统主频48MHZ，频率16KHZ，重装载值250  
																						频率=主频/（预分频值*重装载值）
* @param  void
* @retval void
******************************/
void GPTIM0_PWM_Init(void)
{
	FL_GPTIM_InitTypeDef        timInit;
	FL_GPTIM_OC_InitTypeDef     timOCInit;
	FL_GPIO_InitTypeDef         gpioInit = {0};
	
	/*----------------------------------GPIO结构体初始化-------------------------------------*/	
//GPTIM0_CH1		 PB10   电机1PWM
	gpioInit.pin          = FL_GPIO_PIN_10;
	gpioInit.mode         = FL_GPIO_MODE_DIGITAL;
	gpioInit.outputType   = FL_GPIO_OUTPUT_PUSHPULL;
	gpioInit.pull         = FL_DISABLE;
	gpioInit.remapPin     = FL_ENABLE;
	FL_GPIO_Init(GPIOB, &gpioInit);
//GPTIM0_CH2			PB11  电机2PWM
	gpioInit.pin          = FL_GPIO_PIN_11;
	gpioInit.mode         = FL_GPIO_MODE_DIGITAL;
	gpioInit.outputType   = FL_GPIO_OUTPUT_PUSHPULL;
	gpioInit.pull         = FL_DISABLE;
	gpioInit.remapPin     = FL_ENABLE;
	FL_GPIO_Init(GPIOB, &gpioInit);
//GPTIM0_CH3		 PC11   电机3PWM
	gpioInit.pin          = FL_GPIO_PIN_11;
	gpioInit.mode         = FL_GPIO_MODE_DIGITAL;
	gpioInit.outputType   = FL_GPIO_OUTPUT_PUSHPULL;
	gpioInit.pull         = FL_DISABLE;
	gpioInit.remapPin     = FL_ENABLE;
	FL_GPIO_Init(GPIOC, &gpioInit);
//GPTIM0_CH4			PC12  电机4PWM
	gpioInit.pin          = FL_GPIO_PIN_12;
	gpioInit.mode         = FL_GPIO_MODE_DIGITAL;
	gpioInit.outputType   = FL_GPIO_OUTPUT_PUSHPULL;
	gpioInit.pull         = FL_DISABLE;
	gpioInit.remapPin     = FL_ENABLE;
	FL_GPIO_Init(GPIOC, &gpioInit);	
//	/*----------------------------------时基结构体初始化------------------------------------*/
	timInit.prescaler             = MOTOR_PWM_PSC;                      /* 分频系数 */
	timInit.counterMode           = FL_GPTIM_COUNTER_DIR_UP;      /* 向上计数 */
	timInit.autoReload            = MOTOR_PWM_PERIOD;                     /* 自动重装载值 */
	timInit.clockDivision         = FL_GPTIM_CLK_DIVISION_DIV1;   /* 死区和滤波分频 */
	timInit.autoReloadState       = FL_ENABLE;                    /* 预装载preload使能 */
	FL_GPTIM_Init(GPTIM0, &timInit);

	/*----------------------------------通道初始化------------------------------------*/
	timOCInit.OCMode       = FL_GPTIM_OC_MODE_PWM1;               /* 输出比较模式PWM1 */
	timOCInit.OCETRFStatus = FL_DISABLE;                          /* OC1REF不受ETR影响 */
	timOCInit.OCFastMode   = FL_DISABLE;                          /* 关闭快速使能 */
	timOCInit.compareValue = 0;                                 	/* 比较值 */
	timOCInit.OCPolarity   = FL_GPTIM_OC_POLARITY_NORMAL;					/* OC1 高有效 */ //FL_GPTIM_OC_POLARITY_INVERT;         
	timOCInit.OCPreload    = FL_DISABLE;                          /* OC preload 无效 */
	
	FL_GPTIM_OC_Init(GPTIM0, FL_GPTIM_CHANNEL_1, &timOCInit);
	FL_GPTIM_OC_Init(GPTIM0, FL_GPTIM_CHANNEL_2, &timOCInit);
	FL_GPTIM_OC_Init(GPTIM0, FL_GPTIM_CHANNEL_3, &timOCInit);
	FL_GPTIM_OC_Init(GPTIM0, FL_GPTIM_CHANNEL_4, &timOCInit);
//	FL_GPTIM_EnableIT_Update(GPTIM0);
	FL_GPTIM_Enable(GPTIM0);    /* 使能定时器 */

	if(system_config.flags.pc0_1_motor_pwm_enable == 1)
	{
			//GPTIM1 定时器配置
		FL_GPTIM_InitTypeDef    TimerBase_InitStruct;	
		FL_GPTIM_OC_InitTypeDef    GPTIM_InitStruct;

		//GPTIM1_CH1			PC0  电机5PWM
		gpioInit.pin          = FL_GPIO_PIN_0;
		gpioInit.mode         = FL_GPIO_MODE_DIGITAL;
		gpioInit.outputType   = FL_GPIO_OUTPUT_PUSHPULL;
		gpioInit.pull         = FL_DISABLE;
		gpioInit.remapPin     = FL_ENABLE;
		FL_GPIO_Init(GPIOC, &gpioInit);	

		//GPTIM1_CH2			PC1  电机6PWM
		gpioInit.pin          = FL_GPIO_PIN_1;
		gpioInit.mode         = FL_GPIO_MODE_DIGITAL;
		gpioInit.outputType   = FL_GPIO_OUTPUT_PUSHPULL;
		gpioInit.pull         = FL_DISABLE;
		gpioInit.remapPin     = FL_ENABLE;
		FL_GPIO_Init(GPIOC, &gpioInit);	
		
		TimerBase_InitStruct.prescaler = MOTOR_PWM_PSC;
		TimerBase_InitStruct.counterMode = FL_GPTIM_COUNTER_DIR_UP;
		TimerBase_InitStruct.autoReload = MOTOR_PWM_PERIOD;
		TimerBase_InitStruct.autoReloadState = FL_ENABLE;
		TimerBase_InitStruct.clockDivision = FL_GPTIM_CLK_DIVISION_DIV1;
		FL_GPTIM_Init(GPTIM1,&TimerBase_InitStruct);

		GPTIM_InitStruct.OCMode = FL_GPTIM_OC_MODE_PWM1;
		GPTIM_InitStruct.OCPolarity = FL_GPTIM_OC_POLARITY_NORMAL;
		GPTIM_InitStruct.OCFastMode = FL_DISABLE;
		GPTIM_InitStruct.OCPreload = FL_DISABLE;
		GPTIM_InitStruct.compareValue = 0;
		GPTIM_InitStruct.OCETRFStatus = FL_DISABLE;

		FL_GPTIM_OC_Init(GPTIM1, FL_GPTIM_CHANNEL_1, &GPTIM_InitStruct);
		FL_GPTIM_OC_Init(GPTIM1, FL_GPTIM_CHANNEL_2, &GPTIM_InitStruct);
	}
}

/*******************************
  * @brief  ATIM初始化   推杆电机初始化：系统主频48MHZ，频率16KHZ，重装载值250  
																				 频率=主频/（预分频值*重装载值）
  * @param  void
  * @retval void
  ******************************/
	
//电机端口初始化
void Motor_Init(void)
{
	FL_GPIO_InitTypeDef    GPIO_InitStruct  = {0};
	/*----------------------------------------------------------------------*/
	/*-----------------------------继电器初始化--------------------------------*/
	/*----------------------------------------------------------------------*/

	GPIO_InitStruct.mode = FL_GPIO_MODE_OUTPUT;
	GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.pull = FL_ENABLE;
	GPIO_InitStruct.remapPin = FL_DISABLE;	
	/*-----------------------电机1继电器初始化------------------------------*/
	
	GPIO_InitStruct.pin = MOTOR1_UP_GPIO_PIN;
	FL_GPIO_Init(MOTOR1_UP_GPIO, &GPIO_InitStruct);

	GPIO_InitStruct.pin = MOTOR1_DOWN_GPIO_PIN;
	FL_GPIO_Init(MOTOR1_DOWN_GPIO, &GPIO_InitStruct);
	
	FL_GPIO_ResetOutputPin(MOTOR1_UP_GPIO,MOTOR1_UP_GPIO_PIN);
	FL_GPIO_ResetOutputPin(MOTOR1_DOWN_GPIO,MOTOR1_DOWN_GPIO_PIN);	
	/*-----------------------电机2继电器初始化------------------------------*/
	GPIO_InitStruct.pin = MOTOR2_UP_GPIO_PIN;
	FL_GPIO_Init(MOTOR2_UP_GPIO, &GPIO_InitStruct);
	
	GPIO_InitStruct.pin = MOTOR2_DOWN_GPIO_PIN;
	FL_GPIO_Init(MOTOR2_DOWN_GPIO, &GPIO_InitStruct);
	
	FL_GPIO_ResetOutputPin(MOTOR2_UP_GPIO,MOTOR2_UP_GPIO_PIN);
	FL_GPIO_ResetOutputPin(MOTOR2_DOWN_GPIO,MOTOR2_DOWN_GPIO_PIN);
	/*-----------------------电机3继电器初始化------------------------------*/
	GPIO_InitStruct.pin = MOTOR3_UP_GPIO_PIN;
	FL_GPIO_Init(MOTOR3_UP_GPIO, &GPIO_InitStruct);	

	GPIO_InitStruct.pin = MOTOR3_DOWN_GPIO_PIN;
	FL_GPIO_Init(MOTOR3_DOWN_GPIO, &GPIO_InitStruct);	
	
	FL_GPIO_ResetOutputPin(MOTOR3_UP_GPIO,MOTOR3_UP_GPIO_PIN);
	FL_GPIO_ResetOutputPin(MOTOR3_DOWN_GPIO,MOTOR3_DOWN_GPIO_PIN);
	/*-----------------------电机4继电器初始化------------------------------*/
	GPIO_InitStruct.pin = MOTOR4_UP_GPIO_PIN;
	FL_GPIO_Init(MOTOR4_UP_GPIO, &GPIO_InitStruct);	

	GPIO_InitStruct.pin = MOTOR4_DOWN_GPIO_PIN;
	FL_GPIO_Init(MOTOR4_DOWN_GPIO, &GPIO_InitStruct);	
	
	FL_GPIO_ResetOutputPin(MOTOR4_UP_GPIO,MOTOR4_UP_GPIO_PIN);
	FL_GPIO_ResetOutputPin(MOTOR4_DOWN_GPIO,MOTOR4_DOWN_GPIO_PIN);	
	

	/*-----------------------电机5继电器初始化(PA9/PA10用作按键，此处跳过)------------------------------*/
//	GPIO_InitStruct.pin = MOTOR5_UP_GPIO_PIN;
//	FL_GPIO_Init(MOTOR5_UP_GPIO, &GPIO_InitStruct);
//	
//	GPIO_InitStruct.pin = MOTOR5_DOWN_GPIO_PIN;
//	FL_GPIO_Init(MOTOR5_DOWN_GPIO, &GPIO_InitStruct);	
//	
//	FL_GPIO_ResetOutputPin(MOTOR5_UP_GPIO,MOTOR5_UP_GPIO_PIN);
//	FL_GPIO_ResetOutputPin(MOTOR5_DOWN_GPIO,MOTOR5_DOWN_GPIO_PIN);	
	/*-----------------------电机6继电器初始化------------------------------*/
	GPIO_InitStruct.pin = MOTOR6_UP_GPIO_PIN;
	FL_GPIO_Init(MOTOR6_UP_GPIO, &GPIO_InitStruct);
	
	GPIO_InitStruct.pin = MOTOR6_DOWN_GPIO_PIN;
	FL_GPIO_Init(MOTOR6_DOWN_GPIO, &GPIO_InitStruct);	
	
	FL_GPIO_ResetOutputPin(MOTOR6_UP_GPIO,MOTOR6_UP_GPIO_PIN);
	FL_GPIO_ResetOutputPin(MOTOR6_DOWN_GPIO,MOTOR6_DOWN_GPIO_PIN);		
	/*----------------------------------------------------------------------*/
	/*----------------------------PWM初始化---------------------------------*/
	/*----------------------------------------------------------------------*/
	GPTIM0_PWM_Init();


	/*----------------------------------------------------------------------*/
	/*-----------------------------霍尔端口初始化--------------------------------*/
	/*----------------------------------------------------------------------*/	
	GPIO_InitStruct.mode       = FL_GPIO_MODE_INPUT;
	GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.pull       = FL_DISABLE;
	GPIO_InitStruct.remapPin   = FL_DISABLE;
	
	GPIO_InitStruct.pin        = HALL_M1A_GPIO_PIN;
	FL_GPIO_Init(HALL_M1A_GPIO, &GPIO_InitStruct);	

	GPIO_InitStruct.pin        = HALL_M2A_GPIO_PIN;
	FL_GPIO_Init(HALL_M2A_GPIO, &GPIO_InitStruct);	
	
	GPIO_InitStruct.pin        = HALL_M3A_GPIO_PIN;
	FL_GPIO_Init(HALL_M3A_GPIO, &GPIO_InitStruct);	

//	GPIO_InitStruct.pin        = HALL_M4A_GPIO_PIN;
//	FL_GPIO_Init(HALL_M4A_GPIO, &GPIO_InitStruct);

	GPIO_InitStruct.pin        = HALL_M5A_GPIO_PIN;
	FL_GPIO_Init(HALL_M5A_GPIO, &GPIO_InitStruct);	

	GPIO_InitStruct.pin        = HALL_M6A_GPIO_PIN;
	FL_GPIO_Init(HALL_M6A_GPIO, &GPIO_InitStruct);	
}


/*------------------------电机1驱动函数--------------------------*/
void Motor_M1_Up(void)
{
	FL_GPIO_SetOutputPin(MOTOR1_UP_GPIO,MOTOR1_UP_GPIO_PIN);
	FL_GPIO_ResetOutputPin(MOTOR1_DOWN_GPIO,MOTOR1_DOWN_GPIO_PIN);
}
void Motor_M1_Down(void)
{
	FL_GPIO_ResetOutputPin(MOTOR1_UP_GPIO,MOTOR1_UP_GPIO_PIN);
	FL_GPIO_SetOutputPin(MOTOR1_DOWN_GPIO,MOTOR1_DOWN_GPIO_PIN);
}
void Motor_M1_Stop(void)
{
	FL_GPIO_ResetOutputPin(MOTOR1_UP_GPIO,MOTOR1_UP_GPIO_PIN);
	FL_GPIO_ResetOutputPin(MOTOR1_DOWN_GPIO,MOTOR1_DOWN_GPIO_PIN);
}
void Motor_M1_Speed(uint16_t Compare)
{
	 FL_GPTIM_WriteCompareCH1(GPTIM0, Compare);           
}
/*------------------------电机2驱动函数--------------------------*/
void Motor_M2_Up(void)
{
   FL_GPIO_SetOutputPin(MOTOR2_UP_GPIO,MOTOR2_UP_GPIO_PIN);
	 FL_GPIO_ResetOutputPin(MOTOR2_DOWN_GPIO,MOTOR2_DOWN_GPIO_PIN);
}
void Motor_M2_Down(void)
{
   FL_GPIO_ResetOutputPin(MOTOR2_UP_GPIO,MOTOR2_UP_GPIO_PIN);
	 FL_GPIO_SetOutputPin(MOTOR2_DOWN_GPIO,MOTOR2_DOWN_GPIO_PIN);
}
void Motor_M2_Stop(void)
{
   FL_GPIO_ResetOutputPin(MOTOR2_UP_GPIO,MOTOR2_UP_GPIO_PIN);
	 FL_GPIO_ResetOutputPin(MOTOR2_DOWN_GPIO,MOTOR2_DOWN_GPIO_PIN);
}
void Motor_M2_Speed(uint16_t Compare)
{
	 FL_GPTIM_WriteCompareCH2(GPTIM0, Compare);  
}
/*------------------------电机3驱动函数--------------------------*/
void Motor_M3_Up(void)
{
   FL_GPIO_SetOutputPin(MOTOR3_UP_GPIO,MOTOR3_UP_GPIO_PIN);
   FL_GPIO_ResetOutputPin(MOTOR3_DOWN_GPIO,MOTOR3_DOWN_GPIO_PIN);	
}
void Motor_M3_Down(void)
{
   FL_GPIO_ResetOutputPin(MOTOR3_UP_GPIO,MOTOR3_UP_GPIO_PIN);
   FL_GPIO_SetOutputPin(MOTOR3_DOWN_GPIO,MOTOR3_DOWN_GPIO_PIN);	
}
void Motor_M3_Stop(void)
{
   FL_GPIO_ResetOutputPin(MOTOR3_UP_GPIO,MOTOR3_UP_GPIO_PIN);
   FL_GPIO_ResetOutputPin(MOTOR3_DOWN_GPIO,MOTOR3_DOWN_GPIO_PIN);		
}
void Motor_M3_Speed(uint16_t Compare)
{
	 FL_GPTIM_WriteCompareCH3(GPTIM0, Compare);  
}
/*------------------------电机4驱动函数--------------------------*/
void Motor_M4_Up(void)
{
   FL_GPIO_SetOutputPin(MOTOR4_UP_GPIO,MOTOR4_UP_GPIO_PIN);
   FL_GPIO_ResetOutputPin(MOTOR4_DOWN_GPIO,MOTOR4_DOWN_GPIO_PIN);	
}
void Motor_M4_Down(void)
{
   FL_GPIO_ResetOutputPin(MOTOR4_UP_GPIO,MOTOR4_UP_GPIO_PIN);
   FL_GPIO_SetOutputPin(MOTOR4_DOWN_GPIO,MOTOR4_DOWN_GPIO_PIN);	
}
void Motor_M4_Stop(void)
{
   FL_GPIO_ResetOutputPin(MOTOR4_UP_GPIO,MOTOR4_UP_GPIO_PIN);
   FL_GPIO_ResetOutputPin(MOTOR4_DOWN_GPIO,MOTOR4_DOWN_GPIO_PIN);	
}
void Motor_M4_Speed(uint16_t Compare)
{
	FL_GPTIM_WriteCompareCH4(GPTIM0, Compare);  
}
/*------------------------电机5驱动函数--------------------------*/
void Motor_M5_Up(void)
{
   FL_GPIO_SetOutputPin(MOTOR5_UP_GPIO,MOTOR5_UP_GPIO_PIN);
   FL_GPIO_ResetOutputPin(MOTOR5_DOWN_GPIO,MOTOR5_DOWN_GPIO_PIN);		
}
void Motor_M5_Down(void)
{
	 FL_GPIO_ResetOutputPin(MOTOR5_UP_GPIO,MOTOR5_UP_GPIO_PIN);
	 FL_GPIO_SetOutputPin(MOTOR5_DOWN_GPIO,MOTOR5_DOWN_GPIO_PIN);	
}
void Motor_M5_Stop(void)
{
   FL_GPIO_ResetOutputPin(MOTOR5_UP_GPIO,MOTOR5_UP_GPIO_PIN);
   FL_GPIO_ResetOutputPin(MOTOR5_DOWN_GPIO,MOTOR5_DOWN_GPIO_PIN);		
}
void Motor_M5_Speed(uint16_t Compare)
{
	if(system_config.flags.pc0_1_motor_pwm_enable == 1)
	{
		FL_GPTIM_WriteCompareCH1(GPTIM1, Compare);  
	}
}
/*------------------------电机6驱动函数--------------------------*/
void Motor_M6_Up(void)
{
   FL_GPIO_SetOutputPin(MOTOR6_UP_GPIO,MOTOR6_UP_GPIO_PIN);
	 FL_GPIO_ResetOutputPin(MOTOR6_DOWN_GPIO,MOTOR6_DOWN_GPIO_PIN);	
}
void Motor_M6_Down(void)
{
	 FL_GPIO_ResetOutputPin(MOTOR6_UP_GPIO,MOTOR6_UP_GPIO_PIN);
	 FL_GPIO_SetOutputPin(MOTOR6_DOWN_GPIO,MOTOR6_DOWN_GPIO_PIN);	
}
void Motor_M6_Stop(void)
{
   FL_GPIO_ResetOutputPin(MOTOR6_UP_GPIO,MOTOR6_UP_GPIO_PIN);
   FL_GPIO_ResetOutputPin(MOTOR6_DOWN_GPIO,MOTOR6_DOWN_GPIO_PIN);		
}
void Motor_M6_Speed(uint16_t Compare)
{
	if(system_config.flags.pc0_1_motor_pwm_enable == 1)
	{
		FL_GPTIM_WriteCompareCH2(GPTIM1, Compare);  
	}
}
/*--------------------------霍尔检测------------------------------*/
uint8_t Get_M1A_Hall_Level(void)
{
	if(0 == FL_GPIO_GetInputPin(HALL_M1A_GPIO,HALL_M1A_GPIO_PIN))
	{
		return 0;
	}
	
	return 1;
}


uint8_t Get_M2A_Hall_Level(void)
{
	if(0 == FL_GPIO_GetInputPin(HALL_M2A_GPIO,HALL_M2A_GPIO_PIN))
	{
		return 0;
	}
	
	return 1;
}

uint8_t Get_M3A_Hall_Level(void)
{
	if(0 == FL_GPIO_GetInputPin(HALL_M3A_GPIO,HALL_M3A_GPIO_PIN))
	{
		return 0;
	}
	
	return 1;
}

uint8_t Get_M4A_Hall_Level(void)
{
	if(0 == FL_GPIO_GetInputPin(HALL_M4A_GPIO,HALL_M4A_GPIO_PIN))
	{
		return 0;
	}
	
	return 1;
}
uint8_t Get_M5A_Hall_Level(void)
{
	if(0 == FL_GPIO_GetInputPin(HALL_M5A_GPIO,HALL_M5A_GPIO_PIN))
	{
		return 0;
	}
	
	return 1;
}
uint8_t Get_M6A_Hall_Level(void)
{
	if(0 == FL_GPIO_GetInputPin(HALL_M6A_GPIO,HALL_M6A_GPIO_PIN))
	{
		return 0;
	}
	
	return 1;
}
