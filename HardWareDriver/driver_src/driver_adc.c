#include "driver_adc.h"
#include "app_config.h"
#include "delay.h"


MotorOcp_ValueTypeDef MotorOcp_Value;
Motor_Scope_Data_Stu g_scope_motor1 = {0};
Motor_Scope_Data_Stu g_scope_motors[6] = {0};

static inline void ADC_CalcScopeData(uint16_t adc_val, Motor_Scope_Data_Stu *pScope)
{
    pScope->adc_raw = adc_val;
    pScope->voltage_mv = ((float)adc_val * CURRENT_ADC_VREF_MV) / CURRENT_ADC_MAX_CODE;
    pScope->current_ma = pScope->voltage_mv / CURRENT_SHUNT_RESISTOR_OHM;
    pScope->current_a  = pScope->current_ma / 1000.0f;
}

static inline void ADC_UpdateAllScopeData(void)
{
    ADC_CalcScopeData(MotorOcp_Value.motor1ADC_value, &g_scope_motors[0]);
    ADC_CalcScopeData(MotorOcp_Value.motor2ADC_value, &g_scope_motors[1]);
    ADC_CalcScopeData(MotorOcp_Value.motor3ADC_value, &g_scope_motors[2]);
    ADC_CalcScopeData(MotorOcp_Value.motor4ADC_value, &g_scope_motors[3]);
    ADC_CalcScopeData(MotorOcp_Value.motor5ADC_value, &g_scope_motors[4]);
    ADC_CalcScopeData(MotorOcp_Value.motor6ADC_value, &g_scope_motors[5]);
    g_scope_motor1 = g_scope_motors[0];
}
#define ADC_CHANNEL_NUM  6
#define ADC_PORT_MAX_NUM 6
#define ADC_SLOT_INVALID 0xFF
uint16_t DMAResult[ADC_CHANNEL_NUM];
uint16_t DMAResult_1[ADC_CHANNEL_NUM];
uint16_t DMAResult_2[ADC_CHANNEL_NUM];

uint16_t ArvResult_1[ADC_CHANNEL_NUM];
uint16_t ArvResult_2[ADC_CHANNEL_NUM];
uint16_t ArvResult_3[ADC_CHANNEL_NUM];
uint8_t adc_active_channel_num = 0;
uint8_t adc_slot_by_port[ADC_PORT_MAX_NUM + 1] = {ADC_SLOT_INVALID};

typedef struct
{
  uint8_t motor_port;
  uint32_t adc_channel;
} ADC_PortChannelMapTypeDef;

static inline uint16_t max3_u16(uint16_t a, uint16_t b, uint16_t c);

static const ADC_PortChannelMapTypeDef g_ADC_PortChannelMap[ADC_CHANNEL_NUM] =
{
  {MOTOR4_PORT, FL_ADC_EXTERNAL_CH0},
  {MOTOR5_PORT, FL_ADC_EXTERNAL_CH1},
  {MOTOR6_PORT, FL_ADC_EXTERNAL_CH6},
  {MOTOR2_PORT, FL_ADC_EXTERNAL_CH8},
  {MOTOR3_PORT, FL_ADC_EXTERNAL_CH9},
  {MOTOR1_PORT, FL_ADC_EXTERNAL_CH11},
};

static inline void ADC_ClearMotorOcpValue(uint8_t motor_port)
{
  switch(motor_port)
  {
    case MOTOR1_PORT: MotorOcp_Value.motor1ADC_value = 0; break;
    case MOTOR2_PORT: MotorOcp_Value.motor2ADC_value = 0; break;
    case MOTOR3_PORT: MotorOcp_Value.motor3ADC_value = 0; break;
    case MOTOR4_PORT: MotorOcp_Value.motor4ADC_value = 0; break;
    case MOTOR5_PORT: MotorOcp_Value.motor5ADC_value = 0; break;
    case MOTOR6_PORT: MotorOcp_Value.motor6ADC_value = 0; break;
    default: break;
  }
}

static inline uint16_t ADC_GetFilteredValueByPort(uint8_t motor_port)
{
  uint8_t slot = adc_slot_by_port[motor_port];
  if(slot == ADC_SLOT_INVALID)
  {
    return 0;
  }
  return max3_u16(ArvResult_1[slot], ArvResult_2[slot], ArvResult_3[slot]);
}

static inline uint8_t ADC_IsMotorPortValid(uint8_t motor_port)
{
  return (motor_port >= MOTOR1_PORT) && (motor_port <= MOTOR6_PORT);
}

static inline void ADC_UpdatePortEnableMask(uint8_t motor_type, uint8_t motor_port, uint8_t *port_enable_mask)
{
  if((motor_type != NO_INPUT_TYPE) && ADC_IsMotorPortValid(motor_port))
  {
    *port_enable_mask |= (uint8_t)(1U << (motor_port - 1U));
  }
}

static void ADC_RebuildSequencerChannels(void)
{
  uint8_t i;
  uint8_t motor_port;
  uint8_t port_enable_mask = 0;

  FL_ADC_DisableSequencerChannel(ADC, FL_ADC_EXTERNAL_CH0);
  FL_ADC_DisableSequencerChannel(ADC, FL_ADC_EXTERNAL_CH1);
  FL_ADC_DisableSequencerChannel(ADC, FL_ADC_EXTERNAL_CH6);
  FL_ADC_DisableSequencerChannel(ADC, FL_ADC_EXTERNAL_CH8);
  FL_ADC_DisableSequencerChannel(ADC, FL_ADC_EXTERNAL_CH9);
  FL_ADC_DisableSequencerChannel(ADC, FL_ADC_EXTERNAL_CH11);

  for(i = 0; i <= ADC_PORT_MAX_NUM; i++)
  {
    adc_slot_by_port[i] = ADC_SLOT_INVALID;
  }

  ADC_UpdatePortEnableMask(system_config.flags.motor_back_type, system_config.flags.motor_back_port, &port_enable_mask);
  ADC_UpdatePortEnableMask(system_config.flags.motor_leg_type, system_config.flags.motor_leg_port, &port_enable_mask);
  ADC_UpdatePortEnableMask(system_config.flags.motor_lumbar_type, system_config.flags.motor_lumbar_port, &port_enable_mask);
  ADC_UpdatePortEnableMask(system_config.flags.motor_neck_type, system_config.flags.motor_neck_port, &port_enable_mask);
  ADC_UpdatePortEnableMask(system_config.flags.motor_lumbar2_type, system_config.flags.motor_lumbar2_port, &port_enable_mask);
  ADC_UpdatePortEnableMask(system_config.flags.motor_neck2_type, system_config.flags.motor_neck2_port, &port_enable_mask);
  ADC_UpdatePortEnableMask(system_config.flags.motor_tilt1_type, system_config.flags.motor_tilt1_port, &port_enable_mask);
  ADC_UpdatePortEnableMask(system_config.flags.motor_tilt2_type, system_config.flags.motor_tilt2_port, &port_enable_mask);

  adc_active_channel_num = 0;
  for(i = 0; i < ADC_CHANNEL_NUM; i++)
  {
    motor_port = g_ADC_PortChannelMap[i].motor_port;
    if((port_enable_mask & (uint8_t)(1U << (motor_port - 1U))) != 0U)
    {
      FL_ADC_EnableSequencerChannel(ADC, g_ADC_PortChannelMap[i].adc_channel);
      adc_slot_by_port[motor_port] = adc_active_channel_num;
      adc_active_channel_num++;
    }
    else
    {
      ADC_ClearMotorOcpValue(motor_port);
    }
  }
}
/**
  * @brief  ADC_Common Initialization function
  * @param  void
  * @retval None
  */
void MF_ADC_Common_Init(void)
{
    FL_ADC_CommonInitTypeDef    Common_InitStruct;
	
    FL_GPIO_InitTypeDef    GPIO_InitStruct;

    FL_ADC_InitTypeDef    Sampling_InitStruct;
	
    Common_InitStruct.clockSource = FL_RCC_ADC_CLK_SOURCE_RCHF;
    Common_InitStruct.clockPrescaler = FL_RCC_ADC_PSC_DIV1;

    FL_ADC_CommonInit(&Common_InitStruct);

    FL_ADC_EnableDMAReq(ADC);    

    /* PA13 ADC_IN6 */ 
    GPIO_InitStruct.pin = FL_GPIO_PIN_9;
    GPIO_InitStruct.mode = FL_GPIO_MODE_ANALOG;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    FL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* PC6 ADC_IN11 */ 
    GPIO_InitStruct.pin = FL_GPIO_PIN_6;
    GPIO_InitStruct.mode = FL_GPIO_MODE_ANALOG;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    FL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* PC7 ADC_IN8 */ 
    GPIO_InitStruct.pin = FL_GPIO_PIN_7;
    GPIO_InitStruct.mode = FL_GPIO_MODE_ANALOG;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    FL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* PC8 ADC_IN9 */ 
    GPIO_InitStruct.pin = FL_GPIO_PIN_8;
    GPIO_InitStruct.mode = FL_GPIO_MODE_ANALOG;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    FL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* PC10 ADC_IN1 */ 
    GPIO_InitStruct.pin = FL_GPIO_PIN_10;
    GPIO_InitStruct.mode = FL_GPIO_MODE_ANALOG;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    FL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* PA13 ADC_IN6 */ 
    GPIO_InitStruct.pin = FL_GPIO_PIN_13;
    GPIO_InitStruct.mode = FL_GPIO_MODE_ANALOG;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    FL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    Sampling_InitStruct.conversionMode = FL_ADC_CONV_MODE_SINGLE;
    Sampling_InitStruct.autoMode = FL_ADC_SINGLE_CONV_MODE_AUTO;
    Sampling_InitStruct.waitMode = FL_ENABLE;
    Sampling_InitStruct.overrunMode = FL_ENABLE;
    Sampling_InitStruct.scanDirection = FL_ADC_SEQ_SCAN_DIR_FORWARD;
    Sampling_InitStruct.externalTrigConv = FL_ADC_TRIGGER_EDGE_NONE;
    Sampling_InitStruct.triggerSource = FL_ADC_TRGI_PA8;
    Sampling_InitStruct.fastChannelTime = FL_ADC_FAST_CH_SAMPLING_TIME_4_ADCCLK;
    Sampling_InitStruct.lowChannelTime = FL_ADC_SLOW_CH_SAMPLING_TIME_6_ADCCLK;
    Sampling_InitStruct.oversamplingMode = FL_DISABLE;

    FL_ADC_Init(ADC, &Sampling_InitStruct);

  ADC_RebuildSequencerChannels();
}
void ADC_DMA_Config(uint16_t *buffer, uint32_t length)
{
	FL_DMA_ConfigTypeDef DMA_ConfigStruct = {0};
	DMA_ConfigStruct.memoryAddress = (uint32_t)buffer;                                //配置DMA_RAM地址
	DMA_ConfigStruct.transmissionCount = length - 1;                                  //配置DMA传输长度
	(void)FL_DMA_StartTransmission(DMA, &DMA_ConfigStruct, FL_DMA_CHANNEL_0);
}
/**
  * @brief  DMA_Channel0 Initialization function
  * @param  void
  * @retval None
  */
void MF_DMA_Channel0_Init(void)
{
    FL_DMA_InitTypeDef    DMA_InitStruct;
		
    FL_NVIC_ConfigTypeDef    InterruptConfigStruct;

    DMA_InitStruct.periphAddress = FL_DMA_PERIPHERAL_FUNCTION1;
    DMA_InitStruct.direction = FL_DMA_DIR_PERIPHERAL_TO_RAM;
    DMA_InitStruct.memoryAddressIncMode = FL_DMA_MEMORY_INC_MODE_INCREASE;
    DMA_InitStruct.flashAddressIncMode = FL_DMA_CH7_FLASH_INC_MODE_INCREASE;
    DMA_InitStruct.dataSize = FL_DMA_BANDWIDTH_16B;
    DMA_InitStruct.priority = FL_DMA_PRIORITY_VERYHIGH;
    DMA_InitStruct.circMode = FL_DISABLE;

    FL_DMA_Init(DMA, &DMA_InitStruct, FL_DMA_CHANNEL_0);
		FL_DMA_Enable(DMA);                                                               //配置DMA全局开关    
			
		FL_DMA_ClearFlag_TransferComplete(DMA, FL_DMA_CHANNEL_0);                         //清标志
    FL_DMA_EnableIT_TransferComplete(DMA, FL_DMA_CHANNEL_0);                          //配置DMA全程中断
		
		InterruptConfigStruct.preemptPriority = 2U;                                       //配置DMA的优先级
    FL_NVIC_Init(&InterruptConfigStruct, DMA_IRQn);

    if(adc_active_channel_num > 0U)
    {
      ADC_DMA_Config(DMAResult, adc_active_channel_num);
      FL_ADC_ClearFlag_EndOfConversion(ADC);                                           //清标志
      FL_ADC_Enable(ADC);                                                              //启动ADC
      FL_ADC_EnableSWConversion(ADC);
    }
}


void ADC_init(void)
{
	/* Initial ADC */
	MF_ADC_Common_Init();

	/* Initial DMA */
	MF_DMA_Channel0_Init();
	
//	FL_ADC_ClearFlag_EndOfConversion(ADC);                                             //清标志
//	FL_ADC_Enable(ADC);                                                                //启动ADC
//	FL_ADC_EnableSWConversion(ADC);                                                    //开始转换	
}
uint8_t i,adc_conut = 0;
uint8_t adc_prv = 0;
//unsigned short adc_len[500] = {0};
//unsigned short adc_tt = 0;
static inline uint16_t max3_u16(uint16_t a, uint16_t b, uint16_t c)
{
    uint16_t max = a;
    if (b > max) max = b;
    if (c > max) max = c;
    return max;
}
static inline uint16_t mid3_u16(uint16_t a, uint16_t b, uint16_t c)
{
    uint16_t max = a;
    uint16_t min = a;

    if (b > max) max = b;
    if (c > max) max = c;

    if (b < min) min = b;
    if (c < min) min = c;

    uint32_t sum = (uint32_t)a + b + c;
    return (uint16_t)(sum - max - min);
}

void DMA_IRQHandler(void)
{
	uint32_t IE_Flag, IF_Flag;
  if(adc_active_channel_num == 0U)
  {
    return;
  }

	IE_Flag = FL_DMA_IsEnabledIT_TransferComplete(DMA, FL_DMA_CHANNEL_0);             //获取中断使能以及中断标志状态
	IF_Flag = FL_DMA_IsActiveFlag_TransferComplete(DMA, FL_DMA_CHANNEL_0);
	if((0x01U == IE_Flag) && (0x01U == IF_Flag))
	{
		FL_DMA_ClearFlag_TransferComplete(DMA, FL_DMA_CHANNEL_0);                     //清标志   
		
		if(adc_conut == 0)
		{
      for(i=0; i<adc_active_channel_num; i++)
			{
				DMAResult_1[i] = DMAResult[i];				
				adc_conut = 1;
			}
		}
		else if(adc_conut == 1)
		{
      for(i=0; i<adc_active_channel_num; i++)
			{
				DMAResult_2[i] = DMAResult[i];				
				adc_conut = 2;
			}
		}	
		else if(adc_conut == 2)
		{
			adc_conut = 0;		
			
			if(adc_prv == 0)
			{
        for(i=0; i<adc_active_channel_num; i++)
				{
					ArvResult_1[i] = max3_u16(DMAResult_1[i], DMAResult_2[i], DMAResult[i]);
				}
				adc_prv = 1;
			}
			else if(adc_prv == 1)
			{
				adc_prv = 2;
        for(i=0; i<adc_active_channel_num; i++)
				{
					ArvResult_2[i] = max3_u16(DMAResult_1[i], DMAResult_2[i], DMAResult[i]);
				}				
			}
			else if(adc_prv == 2)
			{
				adc_prv = 0;
        for(i=0; i<adc_active_channel_num; i++)
				{
					ArvResult_3[i] = max3_u16(DMAResult_1[i], DMAResult_2[i], DMAResult[i]);
        }
        MotorOcp_Value.motor1ADC_value = ADC_GetFilteredValueByPort(MOTOR1_PORT);
        MotorOcp_Value.motor2ADC_value = ADC_GetFilteredValueByPort(MOTOR2_PORT);
        MotorOcp_Value.motor3ADC_value = ADC_GetFilteredValueByPort(MOTOR3_PORT);
        MotorOcp_Value.motor4ADC_value = ADC_GetFilteredValueByPort(MOTOR4_PORT);
        MotorOcp_Value.motor5ADC_value = ADC_GetFilteredValueByPort(MOTOR5_PORT);
        MotorOcp_Value.motor6ADC_value = ADC_GetFilteredValueByPort(MOTOR6_PORT);
        ADC_UpdateAllScopeData();
			}	
		}				
		FL_ADC_ClearFlag_EndOfConversion(ADC);	
	}
  ADC_DMA_Config(DMAResult, adc_active_channel_num);
	FL_ADC_Enable(ADC);
	FL_ADC_EnableSWConversion(ADC);		
}






