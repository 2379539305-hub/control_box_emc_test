#include "app_config.h"
#include "app_save.h"
#include "app_msgr.h"
#include "driver_periph.h"
#include "driver_beep.h"

SYSTEM_UNION system_config = {0};
unsigned char pb4_config_copy = 0,pb5_config_copy = 0,pb6_config_copy = 0,pb7_config_copy = 0;
unsigned char pc0_config_copy = 0,pc1_config_copy = 0,pa4_config_copy = 0;

void System_Config_Init(void)
{
	unsigned char i = 0 ;

	system_config.flags.bt_enable = BT_CONFIG;
	system_config.flags.poweron_flat = POWER_ON_FLAT_CONFIG; 
	system_config.flags.pair_key_flat = PAIR_KEY_FLAT_CONFIG; 
	system_config.flags.alarm_enable = ALARM_CONFIG;
	system_config.flags.memory_position_saved = MEMORY_POSITION_SAVED_CONFIG;
	system_config.flags.fixed_position_saved = FIXED_POSITION_SAVED_CONFIG; 
	system_config.flags.fixed_position_extend_move = OUTSIDE_MOTOR_MEMORY_CONFIG;
	system_config.flags.snore_run_mode = SNORE_RUN_MODE_CONFIG;
	system_config.flags.rf_lumbar_neck_order = 	RF_LUMBAR_NECK_ORDER_CONFIG;
	system_config.flags.ble_lumbar_neck_order = BLE_LUMBAR_NECK_ORDER_CONFIG;
	system_config.flags.timer_close_massage = TIMER_CLOSE_MASSAGE_CONFIG;
	system_config.flags.msgr_allon_switch_massage = MSGR_ALLON_SWITCH_CONFIG; 
	system_config.flags.msgr_all_close_memory_order = MSGR_ALLOFF_MEMORY_CONFIG; 
	system_config.flags.msgr_dcr_cycle = MSGR_DCR_CYCLE_CONFIG;
	system_config.flags.alarm_action_mode = ALARM_ACTION_MODE_CONFIG;
	system_config.flags.pc0_1_motor_pwm_enable = MOTOR_56_SPEED_ADJUST_CONFIG;
	system_config.flags.pb4_config = PB4_CONFIG;
	system_config.flags.pb5_config = PB5_CONFIG;
	system_config.flags.pb6_config = PB6_CONFIG;
	system_config.flags.pb7_config = PB7_CONFIG;
	system_config.flags.pc0_config = PC0_CONFIG;
	system_config.flags.pc1_config = PC1_CONFIG;
	system_config.flags.pa4_config = PA4_CONFIG;	
	system_config.flags.fixed_position_motor_move_order = FIXED_POSITION_MOTOR_MOVE_ORDER_CONFIG; //0分开动，1是同时动
	system_config.flags.flat_motor_move_order = FLAT_MOVE_MOTOR_MOVE_ORDER_CONFIG; //0分开动，1是同时动
	system_config.flags.flat_run_tilt_motor = FLAT_RUN_TILT_MOTOR_CONFIG; //0不运行升降电机，1运行升降电机
	system_config.flags.ubl_remote_change_color = UBL_REMOTE_CHANGE_COLOR_CONFIG;//0遥控器不切换颜色，1遥控器可以切换颜色
	system_config.flags.light_rgb_gradient_color = UBL_GRADIENT_COLOR_CONFIG;
	system_config.flags.msgr_time_default_min = MSGR_TIME_DEFAULT_MIN_CONFIG;
	system_config.flags.fan_default_time_min =	FAN_DEFAULT_TIME_MIN_CONFIG;
	msgr_min_time_set = system_config.flags.msgr_time_default_min;
	system_config.flags.light_one_default_time_sec = LIGHT_ONE_DEFAULT_TIME_SEC_CONFIG; 
	system_config.flags.light_rgb_default_time_sec = LIGHT_RGB_DEFAULT_TIME_SEC_CONFIG;
	system_config.flags.heat_default_time_min = HEAT_DEFAULT_TIME_MIN_CONFIG;
	//
	system_config.flags.motor_back_type = MOTOR_BACK_TYPE_CONFIG; 
	system_config.flags.motor_leg_type = MOTOR_LEG_TYPE_CONFIG;
	system_config.flags.motor_lumbar_type = MOTOR_LUMBAR_TYPE_CONFIG;
	system_config.flags.motor_neck_type = MOTOR_NECK_TYPE_CONFIG;
	system_config.flags.motor_lumbar2_type = MOTOR_LUMBAR2_TYPE_CONFIG;
	system_config.flags.motor_neck2_type = MOTOR_NECK2_TYPE_CONFIG;
	system_config.flags.motor_tilt1_type = MOTOR_TILT1_TYPE_CONFIG;
	system_config.flags.motor_tilt2_type = MOTOR_TILT2_TYPE_CONFIG;
	//
	system_config.flags.motor_back_port = MOTOR_BACK_PORT_CONFIG;
	system_config.flags.motor_leg_port = MOTOR_LEG_PORT_CONFIG;
	system_config.flags.motor_lumbar_port = MOTOR_LUMBAR_PORT_CONFIG;
	system_config.flags.motor_lumbar2_port = MOTOR_LUMBAR2_PORT_CONFIG;
	system_config.flags.motor_neck_port = MOTOR_NECK_PORT_CONFIG;
	system_config.flags.motor_neck2_port = MOTOR_NECK2_PORT_CONFIG;
	system_config.flags.motor_tilt1_port = MOTOR_TILT1_PORT_CONFIG;
	system_config.flags.motor_tilt2_port = MOTOR_TILT2_PORT_CONFIG;
	//
	system_config.flags.lock_saved = POWER_OFF_CLEAR_LOCK_CONFIG;
	system_config.flags.help_sleep_mode = HELP_SLEEP_MODE_CONFIG;
	system_config.flags.help_sleep_loop = DEMO_RUN_LOOP_CONFIG;
	system_config.flags.motor_follow_run_tilt_motor = MOTOR_FOLLOW_MODE_RUN_TILT_MOTOR_CONFIG;
	system_config.flags.ble_mesh_sync_config = BLE_MESH_SYNC_CONFIG;
	
	pb4_config_copy = system_config.flags.pb4_config;
	pb5_config_copy = system_config.flags.pb5_config;
	pb6_config_copy = system_config.flags.pb6_config;
	pb7_config_copy = system_config.flags.pb7_config;
	pc0_config_copy = system_config.flags.pc0_config;
	pc1_config_copy = system_config.flags.pc1_config;
	pa4_config_copy = system_config.flags.pa4_config;
	
	/*----------------------------设置马达固定位置初始霍尔数-------------------------------*/
	system_config.flags.tv_hall_back_default =  TV_HALL_BACK_DEFAULT_CONFIG;
	system_config.flags.tv_hall_leg_default  =  TV_HALL_LEG_DEFAULT_CONFIG;
	system_config.flags.tv_hall_lumbar_default = TV_HALL_LUMBAR_DEFAULT_CONFIG;
	system_config.flags.tv_hall_neck_default = TV_HALL_NECK_DEFAULT_CONFIG;
	system_config.flags.tv_hall_lumbar2_default = TV_HALL_LUMBAR2_DEFAULT_CONFIG;
	system_config.flags.tv_hall_neck2_default = TV_HALL_NECK2_DEFAULT_CONFIG;

	system_config.flags.zerog_hall_back_default = ZG_HALL_BACK_DEFAULT_CONFIG;
	system_config.flags.zerog_hall_leg_default  = ZG_HALL_LEG_DEFAULT_CONFIG;
	system_config.flags.zerog_hall_lumbar_default = ZG_HALL_LUMBAR_DEFAULT_CONFIG;
	system_config.flags.zerog_hall_neck_default = ZG_HALL_NECK_DEFAULT_CONFIG;
	system_config.flags.zerog_hall_lumbar2_default = ZG_HALL_LUMBAR2_DEFAULT_CONFIG;
	system_config.flags.zerog_hall_neck2_default = ZG_HALL_NECK2_DEFAULT_CONFIG;

	system_config.flags.lounge_hall_back_default 		= LOUNGE_HALL_BACK_DEFAULT_CONFIG;
	system_config.flags.lounge_hall_leg_default  		= LOUNGE_HALL_LEG_DEFAULT_CONFIG;
	system_config.flags.lounge_hall_lumbar_default 	= LOUNGE_HALL_LUMBAR_DEFAULT_CONFIG;
	system_config.flags.lounge_hall_neck_default 		= LOUNGE_HALL_NECK_DEFAULT_CONFIG;
	system_config.flags.lounge_hall_lumbar2_default = LOUNGE_HALL_LUMBAR2_DEFAULT_CONFIG;
	system_config.flags.lounge_hall_neck2_default 	= LOUNGE_HALL_NECK2_DEFAULT_CONFIG;
	
	system_config.flags.snore_hall_back_default = SNORE_HALL_BACK_DEFAULT_CONFIG;
	system_config.flags.snore_hall_leg_default  = SNORE_HALL_LEG_DEFAULT_CONFIG;
	system_config.flags.snore_hall_lumbar_default = SNORE_HALL_LUMBAR_DEFAULT_CONFIG;
	system_config.flags.snore_hall_neck_default = SNORE_HALL_NECK_DEFAULT_CONFIG;
	system_config.flags.snore_hall_lumbar2_default = SNORE_HALL_LUMBAR2_DEFAULT_CONFIG;
	system_config.flags.snore_hall_neck2_default = SNORE_HALL_NECK2_DEFAULT_CONFIG;
	
	system_config.flags.read_hall_back_default = READ_HALL_BACK_DEFAULT_CONFIG;
	system_config.flags.read_hall_leg_default  = READ_HALL_LEG_DEFAULT_CONFIG;
	system_config.flags.read_hall_lumbar_default = READ_HALL_LUMBAR_DEFAULT_CONFIG;
	system_config.flags.read_hall_neck_default = READ_HALL_NECK_DEFAULT_CONFIG;
	system_config.flags.read_hall_lumbar2_default = READ_HALL_LUMBAR2_DEFAULT_CONFIG;
	system_config.flags.read_hall_neck2_default = READ_HALL_NECK2_DEFAULT_CONFIG;
	
	system_config.flags.yoga_hall_back_default = YOGA_HALL_BACK_DEFAULT_CONFIG;
	system_config.flags.yoga_hall_leg_default  = YOGA_HALL_LEG_DEFAULT_CONFIG;
	system_config.flags.yoga_hall_lumbar_default = YOGA_HALL_LUMBAR_DEFAULT_CONFIG;
	system_config.flags.yoga_hall_neck_default = YOGA_HALL_NECK_DEFAULT_CONFIG;
	system_config.flags.yoga_hall_lumbar2_default = YOGA_HALL_LUMBAR2_DEFAULT_CONFIG;
	system_config.flags.yoga_hall_neck2_default = YOGA_HALL_NECK2_DEFAULT_CONFIG;
	
	system_config.flags.getup_hall_back_default = GETUP_HALL_BACK_DEFAULT_CONFIG;
	system_config.flags.getup_hall_leg_default  = GETUP_HALL_LEG_DEFAULT_CONFIG;
	system_config.flags.getup_hall_lumbar_default = GETUP_HALL_LUMBAR_DEFAULT_CONFIG;
	system_config.flags.getup_hall_neck_default = GETUP_HALL_NECK_DEFAULT_CONFIG;
	system_config.flags.getup_hall_lumbar2_default = GETUP_HALL_LUMBAR2_DEFAULT_CONFIG;
	system_config.flags.getup_hall_neck2_default = GETUP_HALL_NECK2_DEFAULT_CONFIG;

	system_config.flags.nursing_hall_back_default = NURSING_HALL_BACK_DEFAULT_CONFIG;
	system_config.flags.nursing_hall_leg_default  = NURSING_HALL_LEG_DEFAULT_CONFIG;
	system_config.flags.nursing_hall_lumbar_default = NURSING_HALL_LUMBAR_DEFAULT_CONFIG;
	system_config.flags.nursing_hall_neck_default = NURSING_HALL_NECK_DEFAULT_CONFIG;
	system_config.flags.nursing_hall_lumbar2_default = NURSING_HALL_LUMBAR2_DEFAULT_CONFIG;
	system_config.flags.nursing_hall_neck2_default = NURSING_HALL_NECK2_DEFAULT_CONFIG;

	system_config.flags.mem1_position_hall_back = MEM1_POSITION_HALL_BACK_CONFIG;
	system_config.flags.mem1_position_hall_leg = MEM1_POSITION_HALL_LEG_CONFIG;
	system_config.flags.mem1_position_hall_lumbar = MEM1_POSITION_HALL_LUMBAR_CONFIG;
	system_config.flags.mem1_position_hall_neck = MEM1_POSITION_HALL_NECK_CONFIG;
	system_config.flags.mem1_position_hall_lumbar2 = MEM1_POSITION_HALL_LUMBAR2_CONFIG;
	system_config.flags.mem1_position_hall_neck2 = MEM1_POSITION_HALL_NECK2_CONFIG;

	system_config.flags.mem2_position_hall_back = MEM2_POSITION_HALL_BACK_CONFIG;
	system_config.flags.mem2_position_hall_leg = MEM2_POSITION_HALL_LEG_CONFIG;	
	system_config.flags.mem2_position_hall_lumbar = MEM2_POSITION_HALL_LUMBAR_CONFIG;
	system_config.flags.mem2_position_hall_neck = MEM2_POSITION_HALL_NECK_CONFIG;
	system_config.flags.mem2_position_hall_lumbar2 = MEM2_POSITION_HALL_LUMBAR2_CONFIG;
	system_config.flags.mem2_position_hall_neck2 = MEM2_POSITION_HALL_NECK2_CONFIG;

	system_config.flags.mem3_position_hall_back = MEM3_POSITION_HALL_BACK_CONFIG;
	system_config.flags.mem3_position_hall_leg = MEM3_POSITION_HALL_LEG_CONFIG;	
	system_config.flags.mem3_position_hall_lumbar = MEM3_POSITION_HALL_LUMBAR_CONFIG;
	system_config.flags.mem3_position_hall_neck = MEM3_POSITION_HALL_NECK_CONFIG;
	system_config.flags.mem3_position_hall_lumbar2 = MEM3_POSITION_HALL_LUMBAR2_CONFIG;
	system_config.flags.mem3_position_hall_neck2 = MEM3_POSITION_HALL_NECK2_CONFIG;	


}
