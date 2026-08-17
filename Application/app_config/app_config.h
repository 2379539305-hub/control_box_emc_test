#ifndef __APP_CONFIG_H
#define __APP_CONFIG_H

#include "main.h"
#include "system.h"

#define SYS_CONFIG_LENGTH 300

typedef union 
{
	unsigned char config_buff[SYS_CONFIG_LENGTH]; // 定义一个100字节的缓冲区
	struct 
	{
		unsigned char msgr_all_close_memory_order;//按摩器全关记忆 0 按摩器全关不记忆 1全关 再次按下恢复记忆状态
		unsigned char ttl_production_prompt;//生产下载配置文件标志
		unsigned char rf_config;   
		unsigned char bt_enable;
		unsigned char poweron_flat;
		unsigned char pair_key_flat;
		unsigned char alarm_enable;
		unsigned char memory_position_saved;
		unsigned char fixed_position_saved;
		unsigned char fixed_position_extend_move;
		unsigned char fixed_position_motor_move_order;
		unsigned char flat_motor_move_order;
		unsigned char flat_run_tilt_motor;
		unsigned char ubl_remote_change_color;
		unsigned char snore_run_mode;
		unsigned char rf_lumbar_neck_order;
		unsigned char ble_lumbar_neck_order;
		unsigned char timer_close_massage;
		unsigned char msgr_allon_switch_massage;
		unsigned char lock_saved;
		unsigned char help_sleep_mode;
		unsigned char help_sleep_loop;
		unsigned char motor_follow_run_tilt_motor;
		unsigned char help_sleep_mode_one_run_massage;
		unsigned char help_sleep_mode_two_run_massage;
		unsigned char ble_mesh_sync_config;
		//0是默认常开模式（USB)，1是头部按摩器，2是脚部按摩器，3是床底灯，4是灯牌
		unsigned char pc0_1_motor_pwm_enable;
		unsigned char pb4_config;
		unsigned char pb5_config;
		
		unsigned char pb6_config;
		unsigned char pb7_config;
		unsigned char pc1_config;
		unsigned char pc0_config;
		unsigned char pa4_config;
		
		unsigned char motor_back_port;
		unsigned char motor_leg_port;
		unsigned char motor_lumbar_port;
		unsigned char motor_lumbar2_port;
		unsigned char motor_neck_port;
		unsigned char motor_neck2_port;
		unsigned char motor_tilt1_port;
		unsigned char motor_tilt2_port;

		unsigned char motor_back_type;
		unsigned char motor_leg_type;
		unsigned char motor_lumbar_type;
		unsigned char motor_neck_type;
		unsigned char motor_lumbar2_type;
		unsigned char motor_neck2_type;
		unsigned char motor_tilt1_type;
		unsigned char motor_tilt2_type;

		unsigned char   msgr_time_default_min;
		unsigned short light_one_default_time_sec;
		unsigned short light_rgb_default_time_sec;
		unsigned char  light_rgb_gradient_color;
		unsigned short fan_default_time_min;
		unsigned short heat_default_time_min;
		
		unsigned short tv_hall_back_default;
		unsigned short tv_hall_leg_default;
		unsigned short tv_hall_lumbar_default;
		unsigned short tv_hall_neck_default;
		unsigned short tv_hall_lumbar2_default;
		unsigned short tv_hall_neck2_default;
		
		unsigned short zerog_hall_back_default;
		unsigned short zerog_hall_leg_default;
		unsigned short zerog_hall_lumbar_default;
		unsigned short zerog_hall_neck_default;
		unsigned short zerog_hall_lumbar2_default;
		unsigned short zerog_hall_neck2_default;
		
		unsigned short lounge_hall_back_default;
		unsigned short lounge_hall_leg_default;
		unsigned short lounge_hall_lumbar_default;
		unsigned short lounge_hall_neck_default;
		unsigned short lounge_hall_lumbar2_default;
		unsigned short lounge_hall_neck2_default;
		
		unsigned short snore_hall_back_default;
		unsigned short snore_hall_leg_default;
		unsigned short snore_hall_lumbar_default;
		unsigned short snore_hall_neck_default;
		unsigned short snore_hall_lumbar2_default;
		unsigned short snore_hall_neck2_default;
		
		unsigned short read_hall_back_default;
		unsigned short read_hall_leg_default;
		unsigned short read_hall_lumbar_default;
		unsigned short read_hall_neck_default;
		unsigned short read_hall_lumbar2_default;
		unsigned short read_hall_neck2_default;
		
		unsigned short yoga_hall_back_default;
		unsigned short yoga_hall_leg_default;
		unsigned short yoga_hall_lumbar_default;
		unsigned short yoga_hall_neck_default;
		unsigned short yoga_hall_lumbar2_default;
		unsigned short yoga_hall_neck2_default;
		
		unsigned short getup_hall_back_default;
		unsigned short getup_hall_leg_default;
		unsigned short getup_hall_lumbar_default;
		unsigned short getup_hall_neck_default;
		unsigned short getup_hall_lumbar2_default;
		unsigned short getup_hall_neck2_default;

		unsigned short nursing_hall_back_default;
		unsigned short nursing_hall_leg_default;
		unsigned short nursing_hall_lumbar_default;
		unsigned short nursing_hall_neck_default;
		unsigned short nursing_hall_lumbar2_default;
		unsigned short nursing_hall_neck2_default;

		unsigned short mem1_position_hall_back;
		unsigned short mem1_position_hall_leg;
		unsigned short mem1_position_hall_lumbar;
		unsigned short mem1_position_hall_neck;
		unsigned short mem1_position_hall_lumbar2;
		unsigned short mem1_position_hall_neck2;

		unsigned short mem2_position_hall_back;
		unsigned short mem2_position_hall_leg;
		unsigned short mem2_position_hall_lumbar;
		unsigned short mem2_position_hall_neck;
		unsigned short mem2_position_hall_lumbar2;
		unsigned short mem2_position_hall_neck2;

		unsigned short mem3_position_hall_back;
		unsigned short mem3_position_hall_leg;
		unsigned short mem3_position_hall_lumbar;
		unsigned short mem3_position_hall_neck;
		unsigned short mem3_position_hall_lumbar2;
		unsigned short mem3_position_hall_neck2;

		unsigned char  demo_music_config;
		unsigned char  msgr_dcr_cycle;//按摩器减档是否循环 0 不循环(减到0停止) 1 循环(0变为3档)
		unsigned char  alarm_action_mode;//闹钟动作模式 0 单次 1 三次
	}flags;
}SYSTEM_UNION;
//校验SYSTEM_UNION中flags结构体的大小是否超过SYS_CONFIG_LENGTH,若提示错误,加大SYS_CONFIG_LENGTH
typedef char SYSTEM_CONFIG_SIZE_CHECK[
    (sizeof(((SYSTEM_UNION*)0)->flags) <= SYS_CONFIG_LENGTH) ? 1 : -1
];

extern SYSTEM_UNION system_config;
enum
{
	NO_RF_CONFIG = 0,//无射频
	OLD_RF_CONFIG = 1,//射频旧配置
	NEW_RF_CONFIG = 2,//射频新配置
};
enum
{
	BLE_CONFIG_DISENABLE = 0,//无模块
	BLE_CONFIG_BLE_RF_ENABLE = 1,//有蓝牙，有2.4广播 合二为一
};
enum
{
	POWER_ON_FLAT_DISABLE = 0,//上电不复位
	POWER_ON_FLAT_ENABLE = 1,//上电复位
};
enum
{
	PAIR_GO_FLAT_DISABLE = 0,//对码不复位
	PAIR_GO_FLAT_ENABLE = 1,//对码复位
};
enum
{
	CLOCK_DISABLE_CONFIG = 0,//无闹钟
	CLOCK_SINGLE_CONFIG = 1,//单次闹钟
	CLOCK_REPEAT_CONFIG = 2,//重复闹钟
};
enum
{
	POWER_OFF_DONT_CLEAR_MEMORY = 0,//记忆位置断电不清除
	POWER_OFF_CLEAR_MEMORY = 1,//记忆位置断电清除
};
enum
{
	POWER_OFF_DONT_CLEAR_POSITION = 0,//固定位置断电不清除
	POWER_OFF_CLEAR_POSITION = 1,//固定位置断电清除
};
enum
{
	POWER_OFF_DONT_CLEAR_LOCK = 0,//锁定状态断电不清除
	POWER_OFF_CLEAR_LOCK = 1,//锁定状态断电清除
};
enum
{
	ALL_SPEED_NO_FLAT = 0,//全部速度不复位
	HALF_SPEED_NO_FLAT = 1,//半速不复位
	ALL_SPEED_FLAT = 2,//全部速度复位
	HALF_SPEED_FLAT = 3,//半速复位
};
enum
{
	OUTSIDE_MOTOR_NO_MEMORY = 0,//外拖不参与记忆
	OUTSIDE_MOTOR_MEMORY = 1,//外拖参与记忆
};
enum
{
	RF_LUMBAR_NECK_EXCHAGE_DISABLE= 0,//默认射频外托指令 不交换（）
	RF_LUMBAR_NECK_EXCHAGE_ENABLE = 1,//交换
};
enum
{
	BLE_LUMBAR_NECK_EXCHAGE_DISABLE = 0,//默认蓝牙外托指令
	BLE_LUMBAR_NECK_EXCHAGE_ENABLE = 1,//
};
enum
{
	TIMER_CLOSE_MASSAGE_DISABLE = 0,//默认定时不关闭按摩器
	TIMER_CLOSE_MASSAGE_ENABLE  = 1,//定时关闭按摩器
};
enum
{
	MOTOR_5_MOTOR_6_NO_SPEED = 0,
	MOTOR_5_MOTOR_6_SPEED_ADJUST = 1,
};
enum
{
	MSGR_ALL_ON_DISABLE = 0,//默认按摩器全开指令 关闭
	MSGR_ALL_ON_ENABLE = 1,//默认按摩器全开指令 打开
};
enum
{
	FIX_POSITION_MOVE_NO_TOGETHER = 0,//固定位置动作顺序 分开动
	FIX_POSITION_MOVE_TOGETHER = 1,//固定位置动作顺序 同时动
};
enum
{
	FLAT_MOVE_NO_TOGETHER = 0,//断电复位动作顺序 分开动
	FLAT_MOVE_TOGETHER = 1,//断电复位动作顺序 同时动
};
enum
{
	FLAT_RUN_TILT_MOTOR_DISABLE = 0,//断电复位不运行升降电机
	FLAT_RUN_TILT_MOTOR_ENABLE = 1,//断电复位运行升降电机
};
enum
{
	UBL_CHANGE_COLOR_DISABLE = 0,//床底灯不改变颜色
	UBL_CHANGE_COLOR_ENABLE = 1,//普通RGB床底灯改变颜色8种颜色切换
	UBL_DOUBLE_COLOR_CONFIG = 2,//双颜色床底灯
	UBL_THERAPY_COLOR_CONFIG = 3,//RGB光疗-暖光，冷光，自然光，切换颜色
	
};
enum
{
	NO_HALL_MOTOR_TYPE = 0,//非霍尔电机
	HALL_MOTOR_TYPE = 1,//霍尔电机
	NO_INPUT_TYPE	= 0xff,//无效电机
};
enum
{
	MSGR_ALL_CLOSE_NO_MEMORY = 0,//全关不记忆
	MSGR_ALL_CLOSE_MEMORY = 1,//全关记忆
};
enum
{
	MSGR_DCR_NO_CYCLE = 0,//减档不循环,减到0停止
	MSGR_DCR_CYCLE = 1,//减档循环,0变为3档
};
enum
{
	ALARM_ACTION_SINGLE = 0,//闹钟单次执行
	ALARM_ACTION_THREE = 1,//闹钟三次执行
};
enum
{
	UBL_GRADIENT_COLOR_DISABLE = 0,//不会渐灭
	UBL_GRADIENT_COLOR_ENABLE = 1,//渐灭
};
enum
{
	MOTOR_NO = 0,
	MOTOR1_PORT = 1,
	MOTOR2_PORT = 2,	
	MOTOR3_PORT = 3,
	MOTOR4_PORT = 4,
	MOTOR5_PORT = 5,
	MOTOR6_PORT = 6,
};
enum
{
	DEMO_NUM_LOOP = 0,
	DEMO_TIME_LOOP = 1,
};
typedef enum 
{
	UBL = 1,
	LED_BOARD = 2,
	USB_MODE = 3,
	HEAT				 = 4,
	HEAD_MASSAGE = 5,
	FOOT_MASSAGE = 6,
	MUSIC_SW = 7,
	LUMBAR_MASSAGE = 8,
	UBL_BOARD = 9,
	COLD_WARM_UBL = 10,
	RGB_UBL = 11
}XS_MODE;
enum
{
	HELP_SLEEP_MODE_ONE = 1,//哄睡模式用第1种
	HELP_SLEEP_MODE_TWO = 2,//SSB哄睡带白噪音和灯带记忆的模式
	HELP_SLEEP_MODE_ZHUIMI = 3,//追觅哄睡模式
	HELP_SLEEP_MODE_TILT = 4,//升降哄睡模式
};
enum
{
	RUN_TILT_MOTOR_DISABLE = 0,//不运行升降电机
	RUN_TILT_MOTOR_ENABLE = 1,//只运行升降电机
};
enum
{
	BLE_MESH_SYNC_DISABLE = 0,//蓝牙mesh不同步
	BLE_MESH_SYNC_ENABLE = 1,//蓝牙mesh同步
};
enum
{
	HELP_SLEEP_MODE_NO_MASSAGE = 0,
	HELP_SLEEP_MODE_RUN_MASSAGE = 1,
};
void System_Config_Init(void);

extern unsigned char pb4_config_copy,pb5_config_copy,pb6_config_copy,pb7_config_copy;
extern unsigned char pc0_config_copy,pc1_config_copy,pa4_config_copy;

#define SOURCE_CODE_ID_CONFIG  				"*C25M00126V110" //一共14位，不够用前面*补14位，前13位共用管制码，最后一位OTA区分细分版本

#define BT_CONFIG	 										BLE_CONFIG_BLE_RF_ENABLE //蓝牙+2.4g遥控器
#define POWER_ON_FLAT_CONFIG 					POWER_ON_FLAT_DISABLE //默认上电不复位
#define PAIR_KEY_FLAT_CONFIG					PAIR_GO_FLAT_DISABLE //默认对码不复位
#define ALARM_CONFIG									CLOCK_REPEAT_CONFIG //闹钟
#define OUTSIDE_MOTOR_MEMORY_CONFIG 	OUTSIDE_MOTOR_MEMORY  //外拖是否参与记忆
#define FIXED_POSITION_SAVED_CONFIG 	POWER_OFF_CLEAR_POSITION //固定位置断电是否清除，恢复默认
#define MEMORY_POSITION_SAVED_CONFIG	POWER_OFF_DONT_CLEAR_MEMORY //记忆位置断电是否清除，恢复默认
#define SNORE_RUN_MODE_CONFIG					ALL_SPEED_NO_FLAT		//snore默认运行方式 全部速度不复位
#define TIMER_CLOSE_MASSAGE_CONFIG		TIMER_CLOSE_MASSAGE_DISABLE //定时是否关闭按摩器
#define MSGR_ALLON_SWITCH_CONFIG			MSGR_ALL_ON_ENABLE	//按摩器全开指令切换按摩器全开/关 0 不切换 1 切换
#define MSGR_ALLOFF_MEMORY_CONFIG			MSGR_ALL_CLOSE_NO_MEMORY //按摩器全关指令记忆 0 不记忆 1 记忆
#define MSGR_DCR_CYCLE_CONFIG				MSGR_DCR_CYCLE //按摩器减档是否循环 0 不循环 1 循环
#define ALARM_ACTION_MODE_CONFIG			ALARM_ACTION_THREE //闹钟动作模式 0 单次 1 三次
#define MOTOR_56_SPEED_ADJUST_CONFIG	MOTOR_5_MOTOR_6_NO_SPEED //MOTOR5，6不调速，PC0可以用于外设。
#define RF_LUMBAR_NECK_ORDER_CONFIG	 	RF_LUMBAR_NECK_EXCHAGE_DISABLE //射频外托指令
#define BLE_LUMBAR_NECK_ORDER_CONFIG	BLE_LUMBAR_NECK_EXCHAGE_DISABLE //蓝牙外托指令


#define PB4_CONFIG  HEAD_MASSAGE		//常用按摩器口，加热垫,可配置成普通IO//C27-xs6-//c26-xs7---------------//c52-xs4------//C57-XS10
#define PB6_CONFIG	LED_BOARD				//常用按摩器口，加热垫,可配置成普通IO//C27-XS7-//c26-xs9--- //c25-xs4--//c52-xs6------//C57-XS13/XS6
#define PB5_CONFIG	FOOT_MASSAGE		//常用按摩器口，加热垫,可配置成普通IO//C27-XS8-//c26-xs8---------------//c52-xs5------//C57-XS11/XS5/XS6
#define PB7_CONFIG	USB_MODE		//常用按摩器口，加热垫,可配置成普通IO    //C27-XS9 ---------------------------------------//C57-XS5
#define PC1_CONFIG	UBL      	//常用普通IO											   //C27-xs10//c26-xs11--//c25-xs5---//c52-xs1------//C57-XS12
#define PA4_CONFIG	LED_BOARD      	//常用普通IO											   //C27-XS11 --------------------------------------//C57-XS13
#define PC0_CONFIG	USB_MODE				//常用普通IO，当MOTOR_56_SPEED_ADJUST_CONFIG配置为MOTOR_5_MOTOR_6_NO_SPEED才行//c52-xs7

#define FIXED_POSITION_MOTOR_MOVE_ORDER_CONFIG FIX_POSITION_MOVE_NO_TOGETHER //固定位置动作顺序 0分开动，1是同时动
#define FLAT_MOVE_MOTOR_MOVE_ORDER_CONFIG 			FLAT_MOVE_TOGETHER           //复位电机动作顺序配置
#define FLAT_RUN_TILT_MOTOR_CONFIG						FLAT_RUN_TILT_MOTOR_DISABLE //flat复位运行升降电机使能,0不运行升降电机，1运行升降电机
#define UBL_REMOTE_CHANGE_COLOR_CONFIG 				UBL_CHANGE_COLOR_ENABLE //遥控器切换床底灯颜色
#define UBL_GRADIENT_COLOR_CONFIG						UBL_GRADIENT_COLOR_DISABLE	//非光疗模式不要设置为渐变
#define MSGR_TIME_DEFAULT_MIN_CONFIG  30 //默认按摩器时间为30分钟
#define LIGHT_ONE_DEFAULT_TIME_SEC_CONFIG 900 //默认床底灯时间为900S
#define LIGHT_RGB_DEFAULT_TIME_SEC_CONFIG 300 //默认RGB彩色灯带时间为300S
#define HEAT_DEFAULT_TIME_MIN_CONFIG	45 //默认加热垫时间为45分钟
#define FAN_DEFAULT_TIME_MIN_CONFIG		120 //默认风扇时间为120分钟
#define MOTOR_START_RUN_INTRVAL_MS 50 //电机启动运行间隔时间ms，保持默认

#define POWER_OFF_CLEAR_LOCK_CONFIG 		POWER_OFF_CLEAR_LOCK//锁定状态断电清除
#define BLE_MESH_SYNC_CONFIG						BLE_MESH_SYNC_DISABLE//用蓝牙mesh同步则不用lin同步

//电机没有用到的设置为无效电机类型NO_INPUT_TYPE
#define MOTOR_BACK_TYPE_CONFIG 						NO_HALL_MOTOR_TYPE
#define MOTOR_LEG_TYPE_CONFIG 						NO_HALL_MOTOR_TYPE
#define MOTOR_LUMBAR_TYPE_CONFIG 					NO_HALL_MOTOR_TYPE
#define MOTOR_NECK_TYPE_CONFIG 						NO_INPUT_TYPE
#define MOTOR_LUMBAR2_TYPE_CONFIG 				NO_INPUT_TYPE
#define MOTOR_NECK2_TYPE_CONFIG 					NO_INPUT_TYPE
#define MOTOR_TILT1_TYPE_CONFIG 				NO_INPUT_TYPE	//升降电机1
#define MOTOR_TILT2_TYPE_CONFIG 				NO_INPUT_TYPE	//升降电机2
//电机端口配置,控制盒最多6路电机,其他写MOTOR_NO,
//硬件PORT1对应XS1，PORT2对应XS2，PORT3对应XS3，PORT4对应XS4，PORT5对应XS5，PORT6对应XS6
#define MOTOR_BACK_PORT_CONFIG 					MOTOR1_PORT
#define MOTOR_LEG_PORT_CONFIG 					MOTOR2_PORT
#define MOTOR_LUMBAR_PORT_CONFIG 				MOTOR3_PORT
#define MOTOR_NECK_PORT_CONFIG 					MOTOR_NO
#define MOTOR_LUMBAR2_PORT_CONFIG 			MOTOR_NO
#define MOTOR_NECK2_PORT_CONFIG 				MOTOR_NO
#define MOTOR_TILT1_PORT_CONFIG 				MOTOR_NO	//升降电机1
#define MOTOR_TILT2_PORT_CONFIG 				MOTOR_NO	//升降电机2
//电机初始霍尔值配置
#define TV_HALL_BACK_DEFAULT_CONFIG             	(HALL_MIN_NUM + 13200)
#define TV_HALL_LEG_DEFAULT_CONFIG              	(HALL_MIN_NUM + 13000)
#define TV_HALL_LUMBAR_DEFAULT_CONFIG            	(HALL_MIN_NUM + 0)
#define TV_HALL_NECK_DEFAULT_CONFIG              	(HALL_MIN_NUM + 0)
#define TV_HALL_LUMBAR2_DEFAULT_CONFIG           	(HALL_MIN_NUM + 0)
#define TV_HALL_NECK2_DEFAULT_CONFIG             	(HALL_MIN_NUM + 0)

#define ZG_HALL_BACK_DEFAULT_CONFIG              	(HALL_MIN_NUM + 3600)
#define ZG_HALL_LEG_DEFAULT_CONFIG              	(HALL_MIN_NUM + 13000)
#define ZG_HALL_LUMBAR_DEFAULT_CONFIG            	(HALL_MIN_NUM + 0)
#define ZG_HALL_NECK_DEFAULT_CONFIG              	(HALL_MIN_NUM + 0)
#define ZG_HALL_LUMBAR2_DEFAULT_CONFIG           	(HALL_MIN_NUM + 0)
#define ZG_HALL_NECK2_DEFAULT_CONFIG             	(HALL_MIN_NUM + 0)

#define LOUNGE_HALL_BACK_DEFAULT_CONFIG          	(HALL_MIN_NUM + 9900)
#define LOUNGE_HALL_LEG_DEFAULT_CONFIG            (HALL_MIN_NUM + 9300)
#define LOUNGE_HALL_LUMBAR_DEFAULT_CONFIG        	(HALL_MIN_NUM + 0)
#define LOUNGE_HALL_NECK_DEFAULT_CONFIG          	(HALL_MIN_NUM + 0)
#define LOUNGE_HALL_LUMBAR2_DEFAULT_CONFIG       	(HALL_MIN_NUM + 0)
#define LOUNGE_HALL_NECK2_DEFAULT_CONFIG         	(HALL_MIN_NUM + 0)

#define SNORE_HALL_BACK_DEFAULT_CONFIG           	(HALL_MIN_NUM + 2900)
#define SNORE_HALL_LEG_DEFAULT_CONFIG            	(HALL_MIN_NUM + 0)
#define SNORE_HALL_LUMBAR_DEFAULT_CONFIG         	(HALL_MIN_NUM + 0)
#define SNORE_HALL_NECK_DEFAULT_CONFIG           	(HALL_MIN_NUM + 0)
#define SNORE_HALL_LUMBAR2_DEFAULT_CONFIG        	(HALL_MIN_NUM + 0)
#define SNORE_HALL_NECK2_DEFAULT_CONFIG          	(HALL_MIN_NUM + 0)

#define READ_HALL_BACK_DEFAULT_CONFIG           	(HALL_MIN_NUM + 0)
#define READ_HALL_LEG_DEFAULT_CONFIG             	(HALL_MIN_NUM + 0)
#define READ_HALL_LUMBAR_DEFAULT_CONFIG         	(HALL_MIN_NUM + 0)
#define READ_HALL_NECK_DEFAULT_CONFIG           	(HALL_MIN_NUM + 0)
#define READ_HALL_LUMBAR2_DEFAULT_CONFIG        	(HALL_MIN_NUM + 0)
#define READ_HALL_NECK2_DEFAULT_CONFIG          	(HALL_MIN_NUM + 0)

#define YOGA_HALL_BACK_DEFAULT_CONFIG            	(HALL_MIN_NUM + 0)
#define YOGA_HALL_LEG_DEFAULT_CONFIG             	(HALL_MIN_NUM + 0)
#define YOGA_HALL_LUMBAR_DEFAULT_CONFIG          	(HALL_MIN_NUM + 0)
#define YOGA_HALL_NECK_DEFAULT_CONFIG            	(HALL_MIN_NUM + 0)
#define YOGA_HALL_LUMBAR2_DEFAULT_CONFIG         	(HALL_MIN_NUM + 0)
#define YOGA_HALL_NECK2_DEFAULT_CONFIG           	(HALL_MIN_NUM + 0)

#define GETUP_HALL_BACK_DEFAULT_CONFIG           	(HALL_MIN_NUM + 0)
#define GETUP_HALL_LEG_DEFAULT_CONFIG            	(HALL_MIN_NUM + 0)
#define GETUP_HALL_LUMBAR_DEFAULT_CONFIG         	(HALL_MIN_NUM + 0)
#define GETUP_HALL_NECK_DEFAULT_CONFIG           	(HALL_MIN_NUM + 0)
#define GETUP_HALL_LUMBAR2_DEFAULT_CONFIG        	(HALL_MIN_NUM + 0)
#define GETUP_HALL_NECK2_DEFAULT_CONFIG          	(HALL_MIN_NUM + 0)

#define NURSING_HALL_BACK_DEFAULT_CONFIG         	(HALL_MIN_NUM + 12325)
#define NURSING_HALL_LEG_DEFAULT_CONFIG          	(HALL_MIN_NUM + 6815)
#define NURSING_HALL_LUMBAR_DEFAULT_CONFIG       	(HALL_MIN_NUM + 0)
#define NURSING_HALL_NECK_DEFAULT_CONFIG         	(HALL_MIN_NUM + 0)
#define NURSING_HALL_LUMBAR2_DEFAULT_CONFIG      	(HALL_MIN_NUM + 0)
#define NURSING_HALL_NECK2_DEFAULT_CONFIG        	(HALL_MIN_NUM + 0)

#define MEM1_POSITION_HALL_BACK_CONFIG   				 	(HALL_MIN_NUM + 0)
#define MEM1_POSITION_HALL_LEG_CONFIG            	(HALL_MIN_NUM + 0)
#define MEM1_POSITION_HALL_LUMBAR_CONFIG         	(HALL_MIN_NUM + 0)
#define MEM1_POSITION_HALL_NECK_CONFIG           	(HALL_MIN_NUM + 0)
#define MEM1_POSITION_HALL_LUMBAR2_CONFIG        	(HALL_MIN_NUM + 0)
#define MEM1_POSITION_HALL_NECK2_CONFIG          	(HALL_MIN_NUM + 0)

#define MEM2_POSITION_HALL_BACK_CONFIG   				 	(HALL_MIN_NUM + 0)
#define MEM2_POSITION_HALL_LEG_CONFIG            	(HALL_MIN_NUM + 0)
#define MEM2_POSITION_HALL_LUMBAR_CONFIG         	(HALL_MIN_NUM + 0)
#define MEM2_POSITION_HALL_NECK_CONFIG           	(HALL_MIN_NUM + 0)
#define MEM2_POSITION_HALL_LUMBAR2_CONFIG        	(HALL_MIN_NUM + 0)
#define MEM2_POSITION_HALL_NECK2_CONFIG          	(HALL_MIN_NUM + 0)

#define MEM3_POSITION_HALL_BACK_CONFIG   				 	(HALL_MIN_NUM + 0)
#define MEM3_POSITION_HALL_LEG_CONFIG            	(HALL_MIN_NUM + 0)
#define MEM3_POSITION_HALL_LUMBAR_CONFIG         	(HALL_MIN_NUM + 0)
#define MEM3_POSITION_HALL_NECK_CONFIG           	(HALL_MIN_NUM + 0)
#define MEM3_POSITION_HALL_LUMBAR2_CONFIG        	(HALL_MIN_NUM + 0)
#define MEM3_POSITION_HALL_NECK2_CONFIG          	(HALL_MIN_NUM + 0)

//哄睡模式设置
#define HELP_SLEEP_MODE_CONFIG 										HELP_SLEEP_MODE_TWO   //哄睡1是普通模式，哄睡2是SSB带白噪音保存专用模式
#define DEMO_RUN_LOOP_CONFIG											DEMO_NUM_LOOP         //仅用于SSB模式和升降哄睡中的配置，演示模式DEMO_NUM_LOOP按次数循环，DEMO_TIME_LOOP按时间循环

//哄睡参数设置 (角度→行程mm→霍尔值: HALL_MIN_NUM + 145*行程)
//背部角度配置
#define BACK_10DU_HALL_NUM				(HALL_MIN_NUM + 2400)
#define BACK_12DU_HALL_NUM				(HALL_MIN_NUM + 2900) 
#define BACK_15DU_HALL_NUM				(HALL_MIN_NUM + 3600) 
#define BACK_20DU_HALL_NUM				(HALL_MIN_NUM + 4800)
//腿部角度配置
#define LEG_15DU_HALL_NUM					(HALL_MIN_NUM + 2500)
#define LEG_22DU_HALL_NUM					(HALL_MIN_NUM + 4100)
#define LEG_35DU_HALL_NUM					(HALL_MIN_NUM + 7100)

#define DEMO_STOP_TIME						5

//升降电机-哄睡霍尔值，以及默认背腿腰颈哄睡初始值霍尔值配置
#define TILT_ONE_SLEEP_HALL_NUM   			(HALL_MIN_NUM + 1158) //升降1电机霍尔值
#define TILT_TWO_SLEEP_HALL_NUM   			(HALL_MIN_NUM + 1158) //升降2电机霍尔值
#define BACK_DEFAULT_SLEEP_HALL_NUM   		ZG_HALL_BACK_DEFAULT_CONFIG //背部默认哄睡霍尔值
#define LEG_DEFAULT_SLEEP_HALL_NUM   			ZG_HALL_LEG_DEFAULT_CONFIG //腿部默认哄睡霍尔值
#define LUMBAR_DEFAULT_SLEEP_HALL_NUM   		ZG_HALL_LUMBAR_DEFAULT_CONFIG //腰部默认哄睡霍尔值
#define NECK_DEFAULT_SLEEP_HALL_NUM   			ZG_HALL_NECK_DEFAULT_CONFIG //颈部默认哄睡霍尔值
#define LUMBAR2_DEFAULT_SLEEP_HALL_NUM   		ZG_HALL_LUMBAR2_DEFAULT_CONFIG //腰部2默认哄睡霍尔值
#define NECK2_DEFAULT_SLEEP_HALL_NUM   		ZG_HALL_NECK2_DEFAULT_CONFIG //颈部2默认哄睡霍尔值

//豪江标准哄睡参数设置
//哄睡摇篮模式行程配置 (行程mm→霍尔值: HALL_MIN_NUM + 145*行程mm)
//背部行程
#define SLEEP_BACK_29DU_NUM				(HALL_MIN_NUM + 5945)   //41mm  145*41
#define SLEEP_BACK_27DU_NUM				(HALL_MIN_NUM + 5365)   //37mm  145*37
#define SLEEP_BACK_26DU_NUM				(HALL_MIN_NUM + 5510)   //38mm  145*38
#define SLEEP_BACK_21DU_NUM				(HALL_MIN_NUM + 4205)   //29mm  145*29
#define SLEEP_BACK_20DU_NUM				(HALL_MIN_NUM + 4060)   //28mm  145*28
#define SLEEP_BACK_17DU_NUM				(HALL_MIN_NUM + 3480)   //24mm  145*24
#define SLEEP_BACK_9DU_NUM				(HALL_MIN_NUM + 1885)   //13mm  145*13
#define SLEEP_BACK_0DU_NUM				(HALL_MIN_NUM + 0)      //0mm
//腿部行程
#define SLEEP_LEG_34DU_NUM				(HALL_MIN_NUM + 6670)   //46mm  145*46
#define SLEEP_LEG_31DU_NUM				(HALL_MIN_NUM + 6090)   //42mm  145*42
#define SLEEP_LEG_28DU_NUM				(HALL_MIN_NUM + 5510)   //38mm  145*38
#define SLEEP_LEG_24DU_NUM				(HALL_MIN_NUM + 4495)   //31mm  145*31
#define SLEEP_LEG_23DU_NUM				(HALL_MIN_NUM + 4640)   //32mm  145*32
#define SLEEP_LEG_20DU_NUM				(HALL_MIN_NUM + 3915)   //27mm  145*27
#define SLEEP_LEG_12DU_NUM				(HALL_MIN_NUM + 2320)   //16mm  145*16
#define SLEEP_LEG_0DU_NUM				(HALL_MIN_NUM + 0)      //0mm


//音乐电机随动配置
#define MOTOR_FOLLOW_MODE_RUN_TILT_MOTOR_CONFIG	RUN_TILT_MOTOR_ENABLE  //电机随动模式是否只运行升降电机，0不运行，1运行
//电机随动模式霍尔值，取电机中间段位置--如果是《背腿电机》随动配置以下
#define BACK_FOLLOW_LOW_HALL_NUM			(HALL_MIN_NUM + 300) //背部跟随低位霍尔值
#define BACK_FOLLOW_MID_HALL_NUM			(HALL_MIN_NUM + 600) //背部跟随中位霍尔值
#define BACK_FOLLOW_HIGH_HALL_NUM			(HALL_MIN_NUM + 1000) //背部跟随高位霍尔值
#define LEG_FOLLOW_LOW_HALL_NUM				(HALL_MIN_NUM + 300)  //腿部跟随低位霍尔值
#define LEG_FOLLOW_MID_HALL_NUM				(HALL_MIN_NUM + 600) //腿部跟随中位霍尔值
#define LEG_FOLLOW_HIGH_HALL_NUM			(HALL_MIN_NUM + 1000) //腿部跟随高位霍尔值
//电机随动模式霍尔值，取电机中间段位置--如果是《升降马达》随动配置以下
#define TILT1_FOLLOW_LOW_HALL_NUM			(HALL_MIN_NUM + 300) //升降1电机跟随低位霍尔值
#define TILT1_FOLLOW_MID_HALL_NUM			(HALL_MIN_NUM + 600) //升降1电机跟随中位霍尔值
#define TILT1_FOLLOW_HIGH_HALL_NUM			(HALL_MIN_NUM + 1000) //升降1电机跟随高位霍尔值
#define TILT2_FOLLOW_LOW_HALL_NUM				(HALL_MIN_NUM + 300)  //升降2电机跟随低位霍尔值
#define TILT2_FOLLOW_MID_HALL_NUM				(HALL_MIN_NUM + 600) //升降2电机跟随中位霍尔值
#define TILT2_FOLLOW_HIGH_HALL_NUM			(HALL_MIN_NUM + 1000) //升降2电机跟随高位霍尔值

#endif
