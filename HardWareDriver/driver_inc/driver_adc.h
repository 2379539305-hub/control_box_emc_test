#ifndef __DRIVER_ADC_H
#define __DRIVER_ADC_H

#include "system.h"
#include "driver_config.h"


/*---------------- 硬件采样参数 (基于原理图 SS6548D_V0.1) ----------------*/
#define CURRENT_ADC_VREF_MV          3300.0f  // 3.3V LDO 供电 (U4 AMS1117-3.3V)
#define CURRENT_ADC_MAX_CODE         4095.0f  // 12位 ADC 满量程
#define CURRENT_SHUNT_RESISTOR_OHM   0.10f    // R14(0.2Ω) // R15(0.2Ω) = 0.10Ω (100mΩ)
#define CURRENT_AMP_GAIN             1.0f     // 直连 RC 滤波 (R12 1K, C12 100nF)，无放大倍数

/* J-Scope (lscope) 实时观测数据结构体 */
typedef struct
{
    uint16_t adc_raw;       // ADC 原始采样码值 (0 ~ 4095)
    float    voltage_mv;    // 采样引脚电压 (mV)
    float    current_ma;    // 换算真实电流 (mA)
    float    current_a;     // 换算真实电流 (A)
} Motor_Scope_Data_Stu;

extern Motor_Scope_Data_Stu g_scope_motor1;     // 电机1实时数据 (J-Scope主要观测)
extern Motor_Scope_Data_Stu g_scope_motors[6];  // 全部6路电机实时数据

void ADC_init(void);

typedef struct
{
  uint16_t motor1ADC_value ;
  uint16_t motor2ADC_value ;
  uint16_t motor3ADC_value ;
  uint16_t motor4ADC_value ;
  uint16_t motor5ADC_value ;	
  uint16_t motor6ADC_value ;		
}MotorOcp_ValueTypeDef;

extern MotorOcp_ValueTypeDef MotorOcp_Value;

#endif






