#include "modul_motor.h"
#include "delay.h"
#include "modul_adc_position.h"
#include "driver_motor.h"
#include "driver_adc.h"
#include "driver_beep.h"

#include "app_config.h"

#define LIMIT_BEEP  0

// ADC限位校验位
#define MOTOR_ADC_LIMIT_COUNT  250
#define MOTOR_ADC_FILTER_COUNT  3
// 霍尔限位校验
#define MOTOR_HALL_LIMIT_COUNT  5000  //(500ms)


MOTOR_BASE_PARA  Motor1_ParaStu = {0};
MOTOR_BASE_PARA  Motor2_ParaStu = {0};
MOTOR_BASE_PARA  Motor3_ParaStu = {0};
MOTOR_BASE_PARA  Motor4_ParaStu = {0};
MOTOR_BASE_PARA  Motor5_ParaStu = {0};
MOTOR_BASE_PARA  Motor6_ParaStu = {0};

static unsigned char motor_invalid_type = NO_INPUT_TYPE;

static void Motor_Invalid_NoOp(void) { }
static void Motor_Invalid_Speed(u16 Compare) { (void)Compare; }
static unsigned char Motor_Invalid_HallNoOp(void) { return 0; }
static unsigned short motor_invalid_adc_value = 0;

MOTOR_BASE_PARA  Motor_Invalid_ParaStu = {
	.motor_port = MOTOR_NO,
	.motor_type = &motor_invalid_type,
	.motorADC_value = &motor_invalid_adc_value,
	.Motor_Up = Motor_Invalid_NoOp,
	.Motor_Down = Motor_Invalid_NoOp,
	.Motor_Stop = Motor_Invalid_NoOp,
	.Motor_Speed = Motor_Invalid_Speed,
	.Get_Hall_Level = Motor_Invalid_HallNoOp,
};

MOTOR_ADC_PARA	Motor1_AdcStu = {0};
MOTOR_ADC_PARA  Motor2_AdcStu = {0};
MOTOR_ADC_PARA  Motor3_AdcStu = {0};
MOTOR_ADC_PARA  Motor4_AdcStu = {0};
MOTOR_ADC_PARA  Motor5_AdcStu = {0};
MOTOR_ADC_PARA  Motor6_AdcStu = {0};

typedef struct
{
  unsigned short filter_value;
  unsigned short filter_buf[MOTOR_ADC_FILTER_COUNT];
  unsigned char filter_index;
  unsigned char filter_count;
}MOTOR_ADC_FILTER_PARA;

static MOTOR_ADC_FILTER_PARA Motor1_AdcFilterStu = {0};
static MOTOR_ADC_FILTER_PARA Motor2_AdcFilterStu = {0};
static MOTOR_ADC_FILTER_PARA Motor3_AdcFilterStu = {0};
static MOTOR_ADC_FILTER_PARA Motor4_AdcFilterStu = {0};
static MOTOR_ADC_FILTER_PARA Motor5_AdcFilterStu = {0};
static MOTOR_ADC_FILTER_PARA Motor6_AdcFilterStu = {0};

static unsigned char motor_run_port = 0;

static unsigned short Motor_GetMaxAdcValue(unsigned short value1, unsigned short value2)
{
  return (value1 >= value2) ? value1 : value2;
}

static unsigned short Motor_FilterSlowAdcValue(unsigned short adc_value, MOTOR_ADC_FILTER_PARA *Motor_FilterStu)
{
  unsigned char filter_index;
  unsigned short max_value;

  Motor_FilterStu->filter_buf[Motor_FilterStu->filter_index] = adc_value;
  Motor_FilterStu->filter_index ++;
  if(Motor_FilterStu->filter_index >= MOTOR_ADC_FILTER_COUNT)
  {
    Motor_FilterStu->filter_index = 0;
  }

  if(Motor_FilterStu->filter_count < MOTOR_ADC_FILTER_COUNT)
  {
    Motor_FilterStu->filter_count ++;
  }

  max_value = Motor_FilterStu->filter_buf[0];
  for(filter_index = 1; filter_index < Motor_FilterStu->filter_count; filter_index ++)
  {
    max_value = Motor_GetMaxAdcValue(max_value, Motor_FilterStu->filter_buf[filter_index]);
  }

  return max_value;
}

static void Motor_UpdateAdcFilterValue(MOTOR_BASE_PARA *Motor_ParaStu, MOTOR_ADC_FILTER_PARA *Motor_FilterStu)
{
  unsigned short adc_value;

  adc_value = *Motor_ParaStu->motorADC_value;
  if(Motor_ParaStu->motor_slow_run_flag == 1)
  {
    Motor_FilterStu->filter_value = Motor_FilterSlowAdcValue(adc_value, Motor_FilterStu);
  }
  else
  {
    Motor_FilterStu->filter_value = adc_value;
    Motor_FilterStu->filter_index = 0;
    Motor_FilterStu->filter_count = 0;
    Motor_FilterStu->filter_buf[0] = adc_value;
    Motor_FilterStu->filter_buf[1] = adc_value;
    Motor_FilterStu->filter_buf[2] = adc_value;
  }
}

void Motor_Func_Init(void(*callback)(void))
{
	Motor1_ParaStu.motor_port = MOTOR1_PORT;
	Motor1_ParaStu.Motor_Up = Motor_M1_Up;
	Motor1_ParaStu.Motor_Down = Motor_M1_Down;
	Motor1_ParaStu.Motor_Stop = Motor_M1_Stop;
	Motor1_ParaStu.Motor_Speed = Motor_M1_Speed;
	Motor1_ParaStu.Get_Hall_Level = Get_M1A_Hall_Level;
	Motor1_ParaStu.motorADC_value = &(MotorOcp_Value.motor1ADC_value);

	Motor2_ParaStu.motor_port = MOTOR2_PORT;
	Motor2_ParaStu.Motor_Up = Motor_M2_Up;
	Motor2_ParaStu.Motor_Down = Motor_M2_Down;
	Motor2_ParaStu.Motor_Stop = Motor_M2_Stop;
	Motor2_ParaStu.Motor_Speed = Motor_M2_Speed;
	Motor2_ParaStu.Get_Hall_Level = Get_M2A_Hall_Level;
	Motor2_ParaStu.motorADC_value = &(MotorOcp_Value.motor2ADC_value);

	Motor3_ParaStu.motor_port = MOTOR3_PORT;
	Motor3_ParaStu.Motor_Up = Motor_M3_Up;
	Motor3_ParaStu.Motor_Down = Motor_M3_Down;
	Motor3_ParaStu.Motor_Stop = Motor_M3_Stop;
	Motor3_ParaStu.Motor_Speed = Motor_M3_Speed;
	Motor3_ParaStu.Get_Hall_Level = Get_M3A_Hall_Level;
	Motor3_ParaStu.motorADC_value = &(MotorOcp_Value.motor3ADC_value);

	Motor4_ParaStu.motor_port = MOTOR4_PORT;
	Motor4_ParaStu.Motor_Up = Motor_M4_Up;
	Motor4_ParaStu.Motor_Down = Motor_M4_Down;
	Motor4_ParaStu.Motor_Stop = Motor_M4_Stop;
	Motor4_ParaStu.Motor_Speed = Motor_M4_Speed;
	Motor4_ParaStu.Get_Hall_Level = Get_M4A_Hall_Level;
	Motor4_ParaStu.motorADC_value = &(MotorOcp_Value.motor4ADC_value);

	Motor5_ParaStu.motor_port = MOTOR5_PORT;
	Motor5_ParaStu.Motor_Up = Motor_M5_Up;
	Motor5_ParaStu.Motor_Down = Motor_M5_Down;
	Motor5_ParaStu.Motor_Stop = Motor_M5_Stop;
	Motor5_ParaStu.Motor_Speed = Motor_M5_Speed;
	Motor5_ParaStu.Get_Hall_Level = Get_M5A_Hall_Level;
	Motor5_ParaStu.motorADC_value = &(MotorOcp_Value.motor5ADC_value);

	Motor6_ParaStu.motor_port = MOTOR6_PORT;
	Motor6_ParaStu.Motor_Up = Motor_M6_Up;
	Motor6_ParaStu.Motor_Down = Motor_M6_Down;
	Motor6_ParaStu.Motor_Stop = Motor_M6_Stop;
	Motor6_ParaStu.Motor_Speed = Motor_M6_Speed;
	Motor6_ParaStu.Get_Hall_Level = Get_M6A_Hall_Level;
	Motor6_ParaStu.motorADC_value = &(MotorOcp_Value.motor6ADC_value);
	
	Motor_Adc_Init();
	callback();
}
void Motor_Run(MOTOR_BASE_PARA *Motor_ParaStu)
{
	if(Motor_ParaStu == NULL)
	{
		return;
	}
	switch(Motor_ParaStu->motor_dir)
  {
    case MOTOR_DIR_UP:
    {
      Motor_ParaStu->realy_delay_time = 0;
      if(LIMIT_DIR_DOWN == Motor_ParaStu->motor_limit_flag)
      {
        Motor_ParaStu->motor_limit_flag = LIMIT_DIR_NO;
      }
      Motor_ParaStu->Motor_Up();
      Motor_ParaStu->motor_relay_dir = MOTOR_DIR_UP;
    }
    break;
    case MOTOR_DIR_DOWN:
    {
      Motor_ParaStu->realy_delay_time = 0;
      if(LIMIT_DIR_UP == Motor_ParaStu->motor_limit_flag)
      {
        Motor_ParaStu->motor_limit_flag = LIMIT_DIR_NO;
      }
      Motor_ParaStu->Motor_Down();
      Motor_ParaStu->motor_relay_dir = MOTOR_DIR_DOWN;
    }
    break;
    case MOTOR_DIR_NO:
    default:
    {
      Motor_ParaStu->Motor_Stop();
      if(Motor_ParaStu->motor_relay_dir != MOTOR_DIR_NO)
      {
        Motor_ParaStu->realy_delay_time ++;
        if(Motor_ParaStu->realy_delay_time >= RELAY_MOS_DELAY)
        {
          Motor_ParaStu->realy_delay_time = 0;
          Motor_ParaStu->motor_relay_dir = MOTOR_DIR_NO;
        }
      }
      else
      {
        Motor_ParaStu->motor_relay_dir = MOTOR_DIR_NO;
      }
    }
    break;
  }
}
/*---------------------------------电机缓启缓停控制函数------------------------------*/
/*
* 描述：
* 参数： 无
* 返回： 无
*/
void Motor_PwmChange(MOTOR_BASE_PARA *Motor_ParaStu)
{
	if(Motor_ParaStu == NULL)
	{
		return;
	}	
  if(Motor_ParaStu->motor_relay_dir != 0)
  {
    if(0 == Motor_ParaStu->motor_slowchange_step)
    {
      Motor_ParaStu->motor_slowchange_time ++;
      if(Motor_ParaStu->motor_slowchange_time >= 10)
      {
        Motor_ParaStu->motor_slowchange_step = 1;
        Motor_ParaStu->motor_slowchange_time = 0;
				if(*Motor_ParaStu->motor_type == 0x00)//&& (Motor_ParaStu->motor_slow_run_flag == 0))//非霍尔电机不调速
				{
					Motor_ParaStu->motor_pwm_duty = Motor_ParaStu->motor_pwm_max;
				}
				else
				{
					Motor_ParaStu->motor_pwm_duty = (unsigned char)(MOTOR_PWM_MAX * 0.3);
				}
      }
      else
      {
        Motor_ParaStu->motor_pwm_duty = 0;
      }
    }
    if(1 == Motor_ParaStu->motor_slowchange_step)
    {
			if(Motor_ParaStu->motor_slow_run_flag != 0)
			{
				Motor_ParaStu->motor_pwm_max = MOTOR_PWM_MAX/2;
			}
      else
      {
        Motor_ParaStu->motor_pwm_max = MOTOR_PWM_MAX;
      }
			if((*Motor_ParaStu->motor_type == 0x00) && (Motor_ParaStu->motor_slow_run_flag == 0))
			{
				Motor_ParaStu->motor_pwm_duty = Motor_ParaStu->motor_pwm_max;
			}
			else
			{
				Motor_ParaStu->motor_slowchange_time ++;
				if(Motor_ParaStu->motor_slowchange_time >= MOTOR_CTR_TIME)
				{
					Motor_ParaStu->motor_slowchange_time = 0;
					Motor_ParaStu->motor_pwm_duty += (unsigned char)(MOTOR_PWM_MAX * 0.02);
					if(Motor_ParaStu->motor_pwm_duty >= Motor_ParaStu->motor_pwm_max)
					{
						Motor_ParaStu->motor_pwm_duty = Motor_ParaStu->motor_pwm_max;
					}
				}
			}
    }
    if(2 == Motor_ParaStu->motor_slowchange_step)
    {
      Motor_ParaStu->motor_slowchange_time ++;
      if(Motor_ParaStu->motor_slowchange_time >= MOTOR_CTR_TIME)
      {
        Motor_ParaStu->motor_slowchange_time = 0;
        if(Motor_ParaStu->motor_pwm_duty >= (unsigned char)(MOTOR_PWM_MAX * 0.3))
        {
          Motor_ParaStu->motor_pwm_duty -= (unsigned char)(MOTOR_PWM_MAX * 0.06);
        }
        else
        {
          Motor_ParaStu->motor_pwm_duty = 0;
          Motor_ParaStu->motor_slowchange_step = 3;
        }
      }
    }
    if(3 == Motor_ParaStu->motor_slowchange_step)
    {
      Motor_ParaStu->motor_pwm_duty = 0;
      Motor_ParaStu->motor_slowchange_time ++;
      if(Motor_ParaStu->motor_slowchange_time >= 10)
      {
        Motor_ParaStu->motor_slowchange_step = 4;
        Motor_ParaStu->motor_slowchange_time = 0;
      }
    }
    if(4 == Motor_ParaStu->motor_slowchange_step)
    {
      Motor_ParaStu->motor_pwm_duty = 0;
      Motor_ParaStu->motor_dir = MOTOR_DIR_NO;
    }
  }
  else
  {
    Motor_ParaStu->motor_slowchange_time = 0;
    Motor_ParaStu->motor_slowchange_step = 0;
    Motor_ParaStu->motor_pwm_duty = 0;
  }
	Motor_ParaStu->Motor_Speed(Motor_ParaStu->motor_pwm_duty);
}

void Motor_PwmMax(MOTOR_BASE_PARA *Motor_ParaStu, unsigned char pwm_max)
{
	if(Motor_ParaStu == NULL)
	{
		return;
	}
  Motor_ParaStu->motor_pwm_max = pwm_max;
}
void Motor_PwmSlowStart(MOTOR_BASE_PARA *Motor_ParaStu)
{
	if(Motor_ParaStu == NULL)
	{
		return;
	}	
  if(0 == Motor_ParaStu->motor_relay_dir)
  {
    Motor_ParaStu->motor_slowchange_step = 0;
    Motor_ParaStu->motor_slowchange_time = 0;
  }
}
void Motor_PwmSlowStop(MOTOR_BASE_PARA *Motor_ParaStu)
{
	if(Motor_ParaStu == NULL)
	{
		return;
	}	
  if(Motor_ParaStu->motor_relay_dir != 0)
  {
    if(Motor_ParaStu->motor_slowchange_step < 2)
    {
      Motor_ParaStu->motor_slowchange_step = 2;
      Motor_ParaStu->motor_slowchange_time = 0;
    }
  }
}
void Motor_PwmImStop(MOTOR_BASE_PARA *Motor_ParaStu)
{
	if(Motor_ParaStu == NULL)
	{
		return;
	}		
  if(Motor_ParaStu->motor_relay_dir != 0)
  {
    if(Motor_ParaStu->motor_slowchange_step < 3  || Motor_ParaStu->motor_slowchange_step == 5)
    {
      Motor_ParaStu->motor_slowchange_step = 3;
      Motor_ParaStu->motor_slowchange_time = 0;
    }
  }
  else
  {
    Motor_ParaStu->motor_slowchange_step = 4;
  }
}
/*------------------------------------马达到达指定位置--------------------------------------*/
/*
* 描述： 马达到达指定位置
* 参数： 无
* 返回： */
unsigned char Motor_ArrivePosition(MOTOR_BASE_PARA *Motor_ParaStu, long hall_target_num)
{
  unsigned char motor_run_flag = 0XFF;
	hall_target_num == 10000 ? hall_target_num = HALL_DOWN_NUM : hall_target_num;

	if(Motor_ParaStu == NULL)
	{
		return MOTOR_DIR_NO;
	}
	if(*Motor_ParaStu->motor_type == NO_INPUT_TYPE)
	{
		return MOTOR_DIR_NO;
	}
	
  if(hall_target_num != 0)
  {
    if(abs(hall_target_num - Motor_ParaStu->old_hall_run_num) > (*Motor_ParaStu->motor_type ? HALL_ERROR_NUM : CURRENT_ERROR_NUM))
    {
      if(hall_target_num > Motor_ParaStu->old_hall_run_num)
      {
        if(Motor_ParaStu->hall_run_num < hall_target_num - (*Motor_ParaStu->motor_type ? HALL_ERROR_NUM : CURRENT_ERROR_NUM))
        {
          if(Motor_ParaStu->motor_limit_flag != LIMIT_DIR_UP)
          {
            Motor_PwmSlowStart(Motor_ParaStu);
            Motor_ParaStu->motor_dir = MOTOR_DIR_UP;

            motor_run_flag = MOTOR_DIR_UP;
          }
          else
          {
            Motor_PwmImStop(Motor_ParaStu);
          }
        }
        else
        {
          Motor_PwmImStop(Motor_ParaStu);
        }
      }
      else
      {
        if(Motor_ParaStu->hall_run_num > hall_target_num +  (*Motor_ParaStu->motor_type ? HALL_ERROR_NUM : CURRENT_ERROR_NUM))
        {
          if(Motor_ParaStu->motor_limit_flag != LIMIT_DIR_DOWN)
          {
            Motor_PwmSlowStart(Motor_ParaStu);
            Motor_ParaStu->motor_dir = MOTOR_DIR_DOWN;

            motor_run_flag = MOTOR_DIR_DOWN;
          }
          else
          {
            Motor_PwmImStop(Motor_ParaStu);
          }
        }
        else
        {
          Motor_PwmImStop(Motor_ParaStu);
        }
      }
    }
    else
    {
      Motor_PwmImStop(Motor_ParaStu);
    }
  }
  else
  {
    Motor_PwmImStop(Motor_ParaStu);
  }

  if(Motor_ParaStu->motor_dir == MOTOR_DIR_NO)
  {
    motor_run_flag = MOTOR_DIR_NO;

    Motor_ParaStu->motor_pwm_max = MOTOR_PWM_MAX;
    Motor_ParaStu->motor_slow_run_flag = 0;
    Motor_ParaStu->old_hall_run_num = Motor_ParaStu->hall_run_num;
  }

  return motor_run_flag;
}

void Motor_Para_Reset(MOTOR_BASE_PARA *Motor_ParaStu)
{
	if(Motor_ParaStu == NULL)
	{
		return;
	}		
  Motor_ParaStu->motor_pwm_max = MOTOR_PWM_MAX;
  Motor_ParaStu->motor_slow_run_flag = 0;
	Motor_ParaStu->motor_run_hall_temp = Motor_ParaStu->hall_run_num;
  Motor_ParaStu->old_hall_run_num = Motor_ParaStu->hall_run_num;
}
/*---------------------------------霍尔检测函数------------------------------*/

void Motor_Hall_TakePosition(MOTOR_BASE_PARA *Motor_ParaStu)
{
	if(Motor_ParaStu == NULL)
	{
		return;
	}		
	#define HALL_FILT_DELAY  5
	if(Motor_ParaStu->Get_Hall_Level() != 0)
	{
		Motor_ParaStu->motor_hall_time ++;
		if(Motor_ParaStu->motor_hall_time >= HALL_FILT_DELAY) 
		{
			Motor_ParaStu->motor_hall_time = HALL_FILT_DELAY;
		}
	}
	else
	{
		if(Motor_ParaStu->motor_hall_time >= HALL_FILT_DELAY)
    {
      if(Motor_ParaStu->motor_dir == MOTOR_DIR_UP)
      {
        Motor_ParaStu->hall_run_num ++;
      }
      if(Motor_ParaStu->motor_dir == MOTOR_DIR_DOWN)
      {
        Motor_ParaStu->hall_run_num --;
      }
    }
    Motor_ParaStu->motor_hall_time = 0;
	}
}
void Motor_Hall_TakePositionTask(void)
{
	if(*Motor6_ParaStu.motor_type == 0x01)
	{
		Motor_Hall_TakePosition(&Motor6_ParaStu);
	}		
	if(*Motor5_ParaStu.motor_type == 0x01)
	{
		Motor_Hall_TakePosition(&Motor5_ParaStu);
	}	
	if(*Motor4_ParaStu.motor_type == 0x01)
	{
		Motor_Hall_TakePosition(&Motor4_ParaStu);
	}
	if(*Motor3_ParaStu.motor_type == 0x01)
	{
		Motor_Hall_TakePosition(&Motor3_ParaStu);
	}
	if(*Motor2_ParaStu.motor_type == 0x01)
	{
		Motor_Hall_TakePosition(&Motor2_ParaStu);
	}
	if(*Motor1_ParaStu.motor_type == 0x01)
	{
		Motor_Hall_TakePosition(&Motor1_ParaStu);
	}
}
/*------------------------------------------使用霍尔检测是否到位----------------------------------------------*/
/*
* 描述：  根据霍尔变化值，判断是否到位
* 参数：  无
* 返回值：*/
void Motor_Hall_Limit(MOTOR_BASE_PARA *Motor_ParaStu)
{
	if(Motor_ParaStu == NULL)
	{
		return;
	}		
	if(Motor_ParaStu->motor_relay_dir != MOTOR_DIR_NO && Motor_ParaStu->motor_pwm_duty != 0)
	{
		Motor_ParaStu->hall_change_time ++;
		if(Motor_ParaStu->hall_change_time >= MOTOR_HALL_LIMIT_COUNT)
		{
			Motor_ParaStu->hall_change_time = MOTOR_HALL_LIMIT_COUNT;
		}
		if(Motor_ParaStu->motor_limit_flag == LIMIT_DIR_UP ||  Motor_ParaStu->motor_limit_flag == LIMIT_DIR_DOWN)
    {
      Motor_ParaStu->hall_change_time = 0;
    }

    if(Motor_ParaStu->hall_old_num != Motor_ParaStu->hall_run_num)
    {
      Motor_ParaStu->hall_change_time  = 0;
    }
    else
    {
      if(Motor_ParaStu->hall_change_time >= MOTOR_HALL_LIMIT_COUNT) //超过 ? ms没有变化
      {
        Motor_ParaStu->hall_change_time  = 0;

        if(Motor_ParaStu->motor_relay_dir == MOTOR_DIR_UP) //上升
        {
          Motor_ParaStu->motor_limit_flag = LIMIT_DIR_UP;
        }
        if(Motor_ParaStu->motor_relay_dir == MOTOR_DIR_DOWN) //下降
        {
          Motor_ParaStu->motor_limit_flag = LIMIT_DIR_DOWN;
					Motor_ParaStu->hall_run_num = HALL_MIN_NUM;  //霍尔重置
        }
      }
    }
	}
	else
  {
		Motor_ParaStu->hall_change_time = 0;
  }
	Motor_ParaStu->hall_old_num = Motor_ParaStu->hall_run_num;
}
void Motor_Hall_LimitTask(void)
{
	if(*Motor6_ParaStu.motor_type == 0x01)
	{
		Motor_Hall_Limit(&Motor6_ParaStu);
	}		
	if(*Motor5_ParaStu.motor_type == 0x01)
	{
		Motor_Hall_Limit(&Motor5_ParaStu);
	}	
	if(*Motor4_ParaStu.motor_type == 0x01)
	{
		Motor_Hall_Limit(&Motor4_ParaStu);
	}
	if(*Motor3_ParaStu.motor_type == 0x01)
	{
		Motor_Hall_Limit(&Motor3_ParaStu);
	}
	if(*Motor2_ParaStu.motor_type == 0x01)
	{
		Motor_Hall_Limit(&Motor2_ParaStu);
	}
	if(*Motor1_ParaStu.motor_type == 0x01)
	{
		Motor_Hall_Limit(&Motor1_ParaStu);
	}
}

/*------------------------------------------使用电流检测是否到位----------------------------------------------*/
void Motor_Current_Limit(MOTOR_BASE_PARA *Motor_ParaStu)
{
	if(Motor_ParaStu == NULL)
	{
		return;
	}		
	// 如果继电器处于关闭状态，直接重置ADC计数器并返回
	if(Motor_ParaStu->motor_relay_dir == MOTOR_DIR_NO) 
	{
		Motor_ParaStu->motor_adc_count = 0;
		return;
	}

	// 如果电流检测值小于等于限制值
	if(*Motor_ParaStu->motorADC_value <= MOTOR_ADC_START_VALUE) 
	{
		Motor_ParaStu->motor_adc_count++;

		// 如果ADC计数达到或超过限制次数
		if(Motor_ParaStu->motor_adc_count >= MOTOR_ADC_LIMIT_COUNT) 
		{
			Motor_ParaStu->motor_adc_count = MOTOR_ADC_LIMIT_COUNT;

			// 根据当前电机方向设置相应的限制标志
			if(Motor_ParaStu->motor_relay_dir == MOTOR_DIR_UP) 
			{
				Motor_ParaStu->motor_limit_flag = LIMIT_DIR_UP;
			} 
			else if (Motor_ParaStu->motor_relay_dir == MOTOR_DIR_DOWN) 
			{
				Motor_ParaStu->motor_limit_flag = LIMIT_DIR_DOWN;
				Motor_ParaStu->hall_run_num = HALL_MIN_NUM;  // 霍尔重置
			}
		}
	}
	else 
	{
		// 如果电流检测值大于限制值，重置ADC计数器
		Motor_ParaStu->motor_adc_count = 0;
	}
}
/*------------------------------------------使用电流获取电机位置----------------------------------------------*/
/**
* 描述：  电流位置算法，更新电机霍尔数,需在100us定时器中运行
* 参数：  无
* 返回值：*/
void Motor_saveAdc_Task(void)
{
  static unsigned char AdcPositionTime;

	if(AdcPositionTime == 0)
	{
		AdcPositionTime = 1;

		if(*Motor1_ParaStu.motor_type == 0x00)
		{
			Motor_UpdateAdcFilterValue(&Motor1_ParaStu, &Motor1_AdcFilterStu);
			Motor_Current_saveTask(&Motor1_AdcStu);
		}

		if(*Motor2_ParaStu.motor_type == 0x00)
		{
			Motor_UpdateAdcFilterValue(&Motor2_ParaStu, &Motor2_AdcFilterStu);
			Motor_Current_saveTask(&Motor2_AdcStu);
		}
	}
	else if(AdcPositionTime == 1)
	{
		if(*Motor3_ParaStu.motor_type == 0x00)
		{
			Motor_UpdateAdcFilterValue(&Motor3_ParaStu, &Motor3_AdcFilterStu);
			Motor_Current_saveTask(&Motor3_AdcStu);
		}

		if(*Motor4_ParaStu.motor_type == 0x00)
		{
			Motor_UpdateAdcFilterValue(&Motor4_ParaStu, &Motor4_AdcFilterStu);
			Motor_Current_saveTask(&Motor4_AdcStu);
		}
		AdcPositionTime = 2;
	}
	else if(AdcPositionTime == 2)
	{
		if(*Motor5_ParaStu.motor_type == 0x00)
		{
			Motor_UpdateAdcFilterValue(&Motor5_ParaStu, &Motor5_AdcFilterStu);
			Motor_Current_saveTask(&Motor5_AdcStu);
		}

		if(*Motor6_ParaStu.motor_type == 0x00)
		{
			Motor_UpdateAdcFilterValue(&Motor6_ParaStu, &Motor6_AdcFilterStu);
			Motor_Current_saveTask(&Motor6_AdcStu);
		}
		AdcPositionTime = 0;
	}
}
void Motor_Adc_Init(void)
{
	Motor1_AdcFilterStu.filter_value = *Motor1_ParaStu.motorADC_value;
	Motor2_AdcFilterStu.filter_value = *Motor2_ParaStu.motorADC_value;
	Motor3_AdcFilterStu.filter_value = *Motor3_ParaStu.motorADC_value;
	Motor4_AdcFilterStu.filter_value = *Motor4_ParaStu.motorADC_value;
	Motor5_AdcFilterStu.filter_value = *Motor5_ParaStu.motorADC_value;
	Motor6_AdcFilterStu.filter_value = *Motor6_ParaStu.motorADC_value;

	Motor1_AdcStu.motorADC_value = &Motor1_AdcFilterStu.filter_value;
	Motor2_AdcStu.motorADC_value = &Motor2_AdcFilterStu.filter_value;
	Motor3_AdcStu.motorADC_value = &Motor3_AdcFilterStu.filter_value;
	Motor4_AdcStu.motorADC_value = &Motor4_AdcFilterStu.filter_value;
	Motor5_AdcStu.motorADC_value = &Motor5_AdcFilterStu.filter_value;
	Motor6_AdcStu.motorADC_value = &Motor6_AdcFilterStu.filter_value;
	
	Motor1_AdcStu.motor_relay_dir = &Motor1_ParaStu.motor_relay_dir;
	Motor2_AdcStu.motor_relay_dir = &Motor2_ParaStu.motor_relay_dir;
	Motor3_AdcStu.motor_relay_dir = &Motor3_ParaStu.motor_relay_dir;
	Motor4_AdcStu.motor_relay_dir = &Motor4_ParaStu.motor_relay_dir;
	Motor5_AdcStu.motor_relay_dir = &Motor5_ParaStu.motor_relay_dir;
	Motor6_AdcStu.motor_relay_dir = &Motor6_ParaStu.motor_relay_dir;
	
	Motor1_AdcStu.hall_run_num = &Motor1_ParaStu.hall_run_num;
	Motor2_AdcStu.hall_run_num = &Motor2_ParaStu.hall_run_num;
	Motor3_AdcStu.hall_run_num = &Motor3_ParaStu.hall_run_num;
	Motor4_AdcStu.hall_run_num = &Motor4_ParaStu.hall_run_num;
	Motor5_AdcStu.hall_run_num = &Motor5_ParaStu.hall_run_num;
	Motor6_AdcStu.hall_run_num = &Motor6_ParaStu.hall_run_num;	
}
void Motor_Current_LimitTask(void)
{	
	if(*Motor6_ParaStu.motor_type == 0x00)
	{
		Motor_Current_Limit(&Motor6_ParaStu);
	}	
	if(*Motor5_ParaStu.motor_type == 0x00)
	{
		Motor_Current_Limit(&Motor5_ParaStu);
	}	
	if(*Motor4_ParaStu.motor_type == 0x00)
	{
		Motor_Current_Limit(&Motor4_ParaStu);
	}
	if(*Motor3_ParaStu.motor_type == 0x00)
	{
		Motor_Current_Limit(&Motor3_ParaStu);
	}
	if(*Motor2_ParaStu.motor_type == 0x00)
	{
		Motor_Current_Limit(&Motor2_ParaStu);
	}
	if(*Motor1_ParaStu.motor_type == 0x00)
	{
		Motor_Current_Limit(&Motor1_ParaStu);
	}
}


unsigned char Get_Motor_PortState(void)
{
	(Motor1_ParaStu.motor_relay_dir) ? (motor_run_port = motor_run_port | 0x01) : (motor_run_port = motor_run_port & 0xfe);
	(Motor2_ParaStu.motor_relay_dir) ? (motor_run_port = motor_run_port | 0x02) : (motor_run_port = motor_run_port & 0xfd);
	(Motor3_ParaStu.motor_relay_dir) ? (motor_run_port = motor_run_port | 0x04) : (motor_run_port = motor_run_port & 0xfb);
	(Motor4_ParaStu.motor_relay_dir) ? (motor_run_port = motor_run_port | 0x08) : (motor_run_port = motor_run_port & 0xf7);
	(Motor5_ParaStu.motor_relay_dir) ? (motor_run_port = motor_run_port | 0x10) : (motor_run_port = motor_run_port & 0xef);	
	(Motor6_ParaStu.motor_relay_dir) ? (motor_run_port = motor_run_port | 0x20) : (motor_run_port = motor_run_port & 0xdf);	
  return motor_run_port;
}

unsigned char Get_Motor_AllReset(void)
{
  if(Motor1_ParaStu.hall_run_num <= HALL_MIN_NUM && \
      Motor2_ParaStu.hall_run_num <= HALL_MIN_NUM && \
      Motor3_ParaStu.hall_run_num <= HALL_MIN_NUM && \
      Motor4_ParaStu.hall_run_num <= HALL_MIN_NUM && \
			Motor5_ParaStu.hall_run_num <= HALL_MIN_NUM && \
			Motor6_ParaStu.hall_run_num <= HALL_MIN_NUM
    )
  {
    return 1;
  }

  return 0;
}
