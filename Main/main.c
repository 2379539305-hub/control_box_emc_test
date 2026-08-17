#include "main.h"
#include "iwdg.h"
#include "system.h"
#include "mf_config.h"
#include "delay.h"

/*--------------- 硬件驱动头文件 -----------------*/
#include "driver_beep.h"
#include "driver_led.h"
#include "driver_key.h"
#include "driver_motor.h"
#include "driver_periph.h"
#include "driver_uart.h"
#include "driver_timer.h"
#include "driver_rtc.h"
#include "driver_adc.h"
#include "SEGGER_RTT.h"

/*--------------- 模块与应用层 ------------------*/
#include "modul_motor.h"
#include "app_config.h"
#include "app_motor.h"

/*----------------- 电机模式与状态定义 -----------------*/
typedef enum
{
	MOTOR_MODE_1 = 1, // 模式1：不缓启缓停、无调速，PWM固定16K (100%占空比)
	MOTOR_MODE_2 = 2, // 模式2：有缓启缓停、无调速
	MOTOR_MODE_3 = 3  // 模式3：无缓启缓停、有调速 (70%~90%循环调速)
} Motor_Mode_Enum;

typedef enum
{
	MOTOR_ACT_IDLE = 0, // 停止
	MOTOR_ACT_UP   = 1, // 背升
	MOTOR_ACT_DOWN = 2  // 背降
} Motor_Action_Enum;

typedef enum
{
	AGING_IDLE       = 0, // 未进入老化
	AGING_RESET_FLAT = 1, // 阶段1：复位到flat (通过限位判断到达底部)
	AGING_UP         = 2, // 阶段2：电机升到上限位(限位判断停止)，并精准计时 T_up
	AGING_WAIT_TOP   = 3, // 阶段3：上限位等待 3 * T_up
	AGING_DOWN       = 4  // 阶段4：电机下降到下限位(限位判断停止)
} Aging_State_Enum;

typedef struct
{
	uint8_t  work_mode;           // 当前工作模式 (1, 2, 3)
	uint8_t  motor_act;           // 当前电机动作 (0:停止, 1:升, 2:降)
	uint16_t current_duty_percent;// 当前占空比百分比 (0~100)
	int8_t   duty_step_dir;       // 模式3调速步进方向 (+5 或 -5)
	uint16_t speed_adj_timer;     // 模式3调速计时器 (ms)
	
	uint16_t key3_press_timer;    // 按键3按下计时 (ms)
	uint8_t  key3_long_pressed;   // 按键3长按触发标志
	uint8_t  key3_can_interrupt;  // 允许按键3打断标志 (松手后置1)
	
	uint8_t  aging_state;         // 老化测试状态机
	uint32_t aging_timer;         // 老化阶段计时器 (ms)
	uint32_t aging_up_time;       // 动态记录的上升耗时 T_up (ms)
	uint16_t aging_led_timer;     // 老化时LED闪烁计时
	uint16_t run_filter_timer;    // 启动防抖计时器 (ms)
} Motor_Control_Stu;

Motor_Control_Stu g_motor_ctrl = {
	.work_mode = MOTOR_MODE_1,
	.motor_act = MOTOR_ACT_IDLE,
	.current_duty_percent = 0,
	.duty_step_dir = 1,
	.speed_adj_timer = 0,
	.key3_press_timer = 0,
	.key3_long_pressed = 0,
	.key3_can_interrupt = 0,
	.aging_state = AGING_IDLE,
	.aging_timer = 0,
	.aging_up_time = 0,
	.aging_led_timer = 0,
	.run_filter_timer = 0
};

/*---------------- 函数前置声明 -----------------*/
void Motor_Key_Control(void);
static void Motor_Led_Update(void);
static void Motor_SetDutyPercent(uint16_t percent);
static void Motor_Immediate_Stop(void);
static void Motor_Start_Dir(uint8_t dir);
static void Motor_Aging_Process(void);
static uint8_t Motor_Check_Is_Limit_Up(void);
static uint8_t Motor_Check_Is_Limit_Down(void);
static void Motor_Rtt_Print_Task(void);

int main(void)
{
	FL_Init();
	MF_Clock_Init();
	MF_Config_Init();
	
	/* 系统配置初始化 */
	System_Config_Init();
	
	/* IO与外设初始化: 先初始化Motor，再初始化Key，确保按键上拉输入不被覆盖 */
	Motor_Init();
	LED_Init();
	Key_Init();
	
	/* ADC电流采样初始化 (用于电流限位判断) */
	ADC_init();
	
	/* 定时器中断初始化 (使能 100us 霍尔脉冲采集与 1ms 电流限位检测) */
	Time100us_Init();
	Time1ms_Init();
	
	/* 电机参数结构体关联初始化 (使能霍尔/电流限位检测) */
	Motor_Func_Init(Motor_ParaStu_Init);
	
	/* 看门狗初始化 */
	Iwdg_Init();
	
	/* 初始模式LED状态更新 */
	Motor_Led_Update();
	
	/* SEGGER RTT 调试输出初始化 */
	SEGGER_RTT_Init();
	SEGGER_RTT_WriteString(0, "\r\n========================================\r\n");
	SEGGER_RTT_WriteString(0, "  Motor Current RTT Monitor Ready!\r\n");
	SEGGER_RTT_WriteString(0, "========================================\r\n");
	
	while(1)
	{
		Iwdg_Clear();
		Motor_Key_Control();
		// Motor_Rtt_Print_Task();
		Delay_Ms(10);
	}
}

/*
* 描述：设置电机1的PWM占空比百分比 (0 ~ 100)
*/
static void Motor_SetDutyPercent(uint16_t percent)
{
	if(percent > 100) percent = 100;
	// MOTOR_PWM_MAX 为 250
	uint16_t compare_val = (uint16_t)((250 * percent) / 100);
	Motor_M1_Speed(compare_val);
	Motor1_ParaStu.motor_pwm_duty = percent;
}

/*
* 描述：电机立即停止
*/
static void Motor_Immediate_Stop(void)
{
	Motor_SetDutyPercent(0);
	Motor_M1_Stop();
	g_motor_ctrl.motor_act = MOTOR_ACT_IDLE;
	g_motor_ctrl.current_duty_percent = 0;
	g_motor_ctrl.run_filter_timer = 0;
	
	Motor1_ParaStu.motor_dir = MOTOR_DIR_NO;
	Motor1_ParaStu.motor_relay_dir = MOTOR_DIR_NO;
	Motor1_ParaStu.motor_pwm_duty = 0;
}

/*
* 描述：启动电机指定方向
*/
static void Motor_Start_Dir(uint8_t dir)
{
	Motor1_ParaStu.motor_limit_flag = LIMIT_DIR_NO;
	Motor1_ParaStu.hall_change_time = 0;
	Motor1_ParaStu.motor_adc_count = 0;
	g_motor_ctrl.run_filter_timer = 0;
	g_motor_ctrl.motor_act = dir;
	
	if(dir == MOTOR_ACT_UP)
	{
		Motor1_ParaStu.motor_dir = MOTOR_DIR_UP;
		Motor1_ParaStu.motor_relay_dir = MOTOR_DIR_UP;
		Motor_M1_Up();
	}
	else if(dir == MOTOR_ACT_DOWN)
	{
		Motor1_ParaStu.motor_dir = MOTOR_DIR_DOWN;
		Motor1_ParaStu.motor_relay_dir = MOTOR_DIR_DOWN;
		Motor_M1_Down();
	}
}

/*
* 描述：检查电机是否到达上限位 (基于程序中断限位标志与霍尔/电流堵转检测)
*/
static uint8_t Motor_Check_Is_Limit_Up(void)
{
	// 1. 软件运行消抖保护 (启动前500ms避开电机启动冲击与电流扰动)
	if(g_motor_ctrl.run_filter_timer < 500)
	{
		return 0;
	}
	
	// 2. 中断限位标志判断 (由定时器中断的霍尔/电流限位服务维护)
	if(Motor1_ParaStu.motor_limit_flag == LIMIT_DIR_UP)
	{
		return 1;
	}
	
	// 3. 霍尔脉冲停止判定 (100us时基下，5000次即500ms无跳变判定堵转限位)
	if(Motor1_ParaStu.hall_change_time >= 5000)
	{
		Motor1_ParaStu.motor_limit_flag = LIMIT_DIR_UP;
		return 1;
	}
	
	return 0;
}

/*
* 描述：检查电机是否到达下限位/Flat (基于程序中断限位标志与霍尔/电流堵转检测)
*/
static uint8_t Motor_Check_Is_Limit_Down(void)
{
	// 1. 软件运行消抖保护 (启动前500ms避开电机启动冲击与电流扰动)
	if(g_motor_ctrl.run_filter_timer < 500)
	{
		return 0;
	}
	
	// 2. 中断限位标志判断 (由定时器中断的霍尔/电流限位服务维护)
	if(Motor1_ParaStu.motor_limit_flag == LIMIT_DIR_DOWN)
	{
		return 1;
	}
	
	// 3. 霍尔脉冲停止判定 (100us时基下，5000次即500ms无跳变判定堵转限位)
	if(Motor1_ParaStu.hall_change_time >= 5000)
	{
		Motor1_ParaStu.motor_limit_flag = LIMIT_DIR_DOWN;
		return 1;
	}
	
	return 0;
}

/*
* 描述：根据当前工作模式更新指示灯 (LED1: PA12, LED2: PB1)
*       模式1：LED1亮，LED2灭
*       模式2：LED1灭，LED2亮
*       模式3：LED1、LED2同时亮
*/
static void Motor_Led_Update(void)
{
	if(g_motor_ctrl.aging_state != AGING_IDLE)
	{
		return;
	}
	
	switch(g_motor_ctrl.work_mode)
	{
		case MOTOR_MODE_1:
			LED1_ON();
			LED2_OFF();
			break;
		case MOTOR_MODE_2:
			LED1_OFF();
			LED2_ON();
			break;
		case MOTOR_MODE_3:
			LED1_ON();
			LED2_ON();
			break;
		default:
			LED1_OFF();
			LED2_OFF();
			break;
	}
}

#define AGING_STAGE_PAUSE_MS   1000   // 动作间隔停顿 (1s)

/*
* 描述：自动执行老化动作程序
*       阶段1：复位到flat (通过限位判断到达下限位停机)
*       阶段2：升到上限位 (通过限位判断到达上限位停机)，并精确记录实际上升耗时 T_up
*       阶段3：上限位静止等待 3 * T_up (休息时间为运行时间的三倍)
*       阶段4：下降回到原位 (通过限位判断到达下限位停机)
*/
static void Motor_Aging_Process(void)
{
	// 老化运行中：指示灯双闪提示
	g_motor_ctrl.aging_led_timer += 10;
	if(g_motor_ctrl.aging_led_timer >= 200)
	{
		g_motor_ctrl.aging_led_timer = 0;
		LED1_Toggle();
		LED2_Toggle();
	}
	
	switch(g_motor_ctrl.aging_state)
	{
		/*====== 阶段1：复位到 Flat (回到底部下限位) ======*/
		case AGING_RESET_FLAT:
		{
			if(g_motor_ctrl.aging_timer == 0)
			{
				Motor_Start_Dir(MOTOR_ACT_DOWN);
				Motor_SetDutyPercent(100);
			}
			g_motor_ctrl.aging_timer += 10;
			g_motor_ctrl.run_filter_timer += 10;
			
			// 通过限位判断是否到达底部
			if(Motor_Check_Is_Limit_Down())
			{
				Motor_Immediate_Stop();
				g_motor_ctrl.aging_timer = 0;
				g_motor_ctrl.aging_state = AGING_UP;
				Delay_Ms(AGING_STAGE_PAUSE_MS);
			}
			break;
		}
		
		/*====== 阶段2：上升到上限位并动态计量耗时 T_up ======*/
		case AGING_UP:
		{
			if(g_motor_ctrl.aging_timer == 0)
			{
				Motor_Start_Dir(MOTOR_ACT_UP);
				Motor_SetDutyPercent(100);
			}
			g_motor_ctrl.aging_timer += 10;
			g_motor_ctrl.run_filter_timer += 10;
			
			// 通过限位判断是否到达上限位
			if(Motor_Check_Is_Limit_Up())
			{
				g_motor_ctrl.aging_up_time = g_motor_ctrl.aging_timer; // 精准保存实际上升耗时 T_up
				Motor_Immediate_Stop();
				g_motor_ctrl.aging_timer = 0;
				g_motor_ctrl.aging_state = AGING_WAIT_TOP;
			}
			break;
		}
		
		/*====== 阶段3：上限位等待 3 倍上升时间 (3 * T_up) ======*/
		case AGING_WAIT_TOP:
		{
			g_motor_ctrl.aging_timer += 10;
			// 休息时间严格为运行时间的三倍
			if(g_motor_ctrl.aging_timer >= (3 * g_motor_ctrl.aging_up_time))
			{
				g_motor_ctrl.aging_timer = 0;
				g_motor_ctrl.aging_state = AGING_DOWN;
			}
			break;
		}
		
		/*====== 阶段4：下降回到底部下限位 ======*/
		case AGING_DOWN:
		{
			if(g_motor_ctrl.aging_timer == 0)
			{
				Motor_Start_Dir(MOTOR_ACT_DOWN);
				Motor_SetDutyPercent(100);
			}
			g_motor_ctrl.aging_timer += 10;
			g_motor_ctrl.run_filter_timer += 10;
			
			// 通过限位判断是否到达底部
			if(Motor_Check_Is_Limit_Down())
			{
				Motor_Immediate_Stop();
				g_motor_ctrl.aging_timer = 0;
				Delay_Ms(AGING_STAGE_PAUSE_MS);
				// 循环进入下一轮上升老化
				g_motor_ctrl.aging_state = AGING_UP;
			}
			break;
		}
		
		default:
			g_motor_ctrl.aging_state = AGING_IDLE;
			break;
	}
}

/*
* 说明：
* 	1、按键1 按下背升 按动松停
* 	2、按键2 按下背降 按动松停
* 	3、按键3 按下切换模式 按动松发 模式1 对应led1 亮起；模式2 对应led2 亮起；模式3 对应led1、2同时亮起
* 		模式1：不缓启缓停、无调速 pwm固定 16K;
* 		模式2：有缓启缓停、无调速
* 		模式3：无缓启缓停、有调速 运行时调速70-90%占空比区间，每50ms增加5%到达最大时变为减小5%直到70%，循环
* 		长按按键3三秒实现电机复位到flat 自动执行老化程序 --动作模式(电机升到上限位等待三倍上升时间后下降) 上升时间不需要测试程序中计时即可
* 		此模式下按下其他按键打断
*/
void Motor_Key_Control(void)
{
	uint8_t k1 = Get_Key1_State(); // 0: 按下背升, 1: 松开
	uint8_t k2 = Get_Key2_State(); // 0: 按下背降, 1: 松开
	uint8_t k3 = Get_Key3_State(); // 0: 按下, 1: 松开
	
	/*---------------- 1. 老化模式处理与打断 ----------------*/
	if(g_motor_ctrl.aging_state != AGING_IDLE)
	{
		uint8_t interrupt_flag = 0;
		// 1) 按下其他按键 (K1 或 K2) 立即打断退出老化
		if((k1 == 0) || (k2 == 0))
		{
			interrupt_flag = 1;
		}
		// 2) K3 在手松开后重新按下，也打断退出老化
		if((k3 == 0) && (g_motor_ctrl.key3_can_interrupt == 1))
		{
			interrupt_flag = 1;
		}
		
		// 监测 K3 手是否已经松开 
		if(k3 != 0)
		{
			g_motor_ctrl.key3_can_interrupt = 1; // 手已松开，下次再按K3才允许打断
			g_motor_ctrl.key3_press_timer = 0;
			g_motor_ctrl.key3_long_pressed = 0;
		}
		
		if(interrupt_flag)
		{
			g_motor_ctrl.aging_state = AGING_IDLE;
			Motor_Immediate_Stop();
			Motor_Led_Update();
			g_motor_ctrl.key3_press_timer = 0;
			g_motor_ctrl.key3_long_pressed = 0;
			g_motor_ctrl.key3_can_interrupt = 0;
			return;
		}
		
		Motor_Aging_Process();
		return;
	}
	
	/*---------------- 2. 按键3：短按切换模式 / 长按3秒进老化 ----------------*/
	if(k3 == 0)
	{
		g_motor_ctrl.key3_press_timer += 10;
		if((g_motor_ctrl.key3_press_timer >= 3000) && (!g_motor_ctrl.key3_long_pressed))
		{
			g_motor_ctrl.key3_long_pressed = 1;
			g_motor_ctrl.key3_can_interrupt = 0; // 长按刚触发，用户手还未松开，禁止误判定打断
			Motor_Immediate_Stop();
			g_motor_ctrl.aging_state = AGING_RESET_FLAT; // 进入阶段1：复位到Flat (限位判断)
			g_motor_ctrl.aging_timer = 0;
			g_motor_ctrl.aging_led_timer = 0;
			return;
		}
	}
	else
	{
		if((!g_motor_ctrl.key3_long_pressed) && (g_motor_ctrl.key3_press_timer >= 30))
		{
			// 有效短按：松开时触发模式循环切换 1 -> 2 -> 3 -> 1
			g_motor_ctrl.work_mode = (g_motor_ctrl.work_mode % 3) + 1;
			Motor_Immediate_Stop();
			Motor_Led_Update();
		}
		g_motor_ctrl.key3_press_timer = 0;
		g_motor_ctrl.key3_long_pressed = 0;
		g_motor_ctrl.key3_can_interrupt = 0;
	}
	
	/*---------------- 3. 按键1与按键2：电机运行控制 ----------------*/
	uint8_t req_act = MOTOR_ACT_IDLE;
	if(k1 == 0 && k2 != 0)
	{
		req_act = MOTOR_ACT_UP;   // 背升
	}
	else if(k2 == 0 && k1 != 0)
	{
		req_act = MOTOR_ACT_DOWN; // 背降
	}
	
	switch(g_motor_ctrl.work_mode)
	{
		/*====== 模式1：不缓启缓停、无调速，PWM固定16K (100%占空比) ======*/
		case MOTOR_MODE_1:
		{
			if(req_act != MOTOR_ACT_IDLE)
			{
				if(g_motor_ctrl.motor_act != req_act)
				{
					Motor_Start_Dir(req_act);
					Motor_SetDutyPercent(100);
					g_motor_ctrl.current_duty_percent = 100;
				}
			}
			else
			{
				if(g_motor_ctrl.motor_act != MOTOR_ACT_IDLE)
				{
					Motor_Immediate_Stop();
				}
			}
			break;
		}
		
		/*====== 模式2：有缓启缓停、无调速 ======*/
		case MOTOR_MODE_2:
		{
			if(req_act != MOTOR_ACT_IDLE)
			{
				if(g_motor_ctrl.motor_act != req_act)
				{
					Motor_Start_Dir(req_act);
					if(g_motor_ctrl.current_duty_percent < 30)
					{
						g_motor_ctrl.current_duty_percent = 30; // 缓启初值30%
					}
					Motor_SetDutyPercent(g_motor_ctrl.current_duty_percent);
				}
				else
				{
					// 缓启递增：每10ms增加5%，直到100%
					if(g_motor_ctrl.current_duty_percent < 100)
					{
						g_motor_ctrl.current_duty_percent += 5;
						if(g_motor_ctrl.current_duty_percent > 100)
						{
							g_motor_ctrl.current_duty_percent = 100;
						}
						Motor_SetDutyPercent(g_motor_ctrl.current_duty_percent);
					}
				}
			}
			else
			{
				// 缓停递减：每10ms减少5%，减到30%以下停止
				if(g_motor_ctrl.motor_act != MOTOR_ACT_IDLE)
				{
					if(g_motor_ctrl.current_duty_percent > 30)
					{
						g_motor_ctrl.current_duty_percent -= 5;
						Motor_SetDutyPercent(g_motor_ctrl.current_duty_percent);
					}
					else
					{
						Motor_Immediate_Stop();
					}
				}
			}
			break;
		}
		
		/*====== 模式3：无缓启缓停、有调速 (70%~90%动态调速，每50ms变动5%) ======*/
		case MOTOR_MODE_3:
		{
			if(req_act != MOTOR_ACT_IDLE)
			{
				if(g_motor_ctrl.motor_act != req_act)
				{
					Motor_Start_Dir(req_act);
					g_motor_ctrl.current_duty_percent = 70; // 初始70%
					g_motor_ctrl.duty_step_dir = 1;         // 初始递增
					g_motor_ctrl.speed_adj_timer = 0;
					Motor_SetDutyPercent(70);
				}
				else
				{
					// 持续运行时动态调速：每50ms增减5%
					g_motor_ctrl.speed_adj_timer += 10;
					if(g_motor_ctrl.speed_adj_timer >= 50)
					{
						g_motor_ctrl.speed_adj_timer = 0;
						if(g_motor_ctrl.duty_step_dir > 0)
						{
							g_motor_ctrl.current_duty_percent += 5;
							if(g_motor_ctrl.current_duty_percent >= 90)
							{
								g_motor_ctrl.current_duty_percent = 90;
								g_motor_ctrl.duty_step_dir = -1; // 到达90%转为减小
							}
						}
						else
						{
							if(g_motor_ctrl.current_duty_percent >= 5)
								g_motor_ctrl.current_duty_percent -= 5;
							if(g_motor_ctrl.current_duty_percent <= 70)
							{
								g_motor_ctrl.current_duty_percent = 70;
								g_motor_ctrl.duty_step_dir = 1;  // 到达70%转为增加
							}
						}
						Motor_SetDutyPercent(g_motor_ctrl.current_duty_percent);
					}
				}
			}
			else
			{
				if(g_motor_ctrl.motor_act != MOTOR_ACT_IDLE)
				{
					Motor_Immediate_Stop();
				}
			}
			break;
		}
		
		default:
			g_motor_ctrl.work_mode = MOTOR_MODE_1;
			break;
	}
}


/*
* 描述：通过 RTT 周期打印电机 1 的 ADC 采样值、引脚采样电压及换算出的真实电流
*/
static void Motor_Rtt_Print_Task(void)
{
	static uint8_t print_cnt = 0;
	print_cnt++;
	if(print_cnt >= 10) // 每 10 * 10ms = 100ms 打印一次
	{
		print_cnt = 0;
		uint16_t adc = g_scope_motor1.adc_raw;
		uint32_t curr_ma = (uint32_t)(g_scope_motor1.current_ma + 0.5f);
		uint32_t v_mv = (uint32_t)(g_scope_motor1.voltage_mv + 0.5f);
		
		const char *act_str = "IDLE";
		if(g_motor_ctrl.motor_act == MOTOR_ACT_UP) act_str = "UP  ";
		else if(g_motor_ctrl.motor_act == MOTOR_ACT_DOWN) act_str = "DOWN";
		
		SEGGER_RTT_printf(0, "[%s] M1 ADC: %4u | V_pin: %4u mV | Current: %5u mA (%u.%03u A) | Duty: %3u%%\r\n",
		                  act_str, adc, v_mv, curr_ma, curr_ma / 1000, curr_ma % 1000, (unsigned)g_motor_ctrl.current_duty_percent);
	}
}
