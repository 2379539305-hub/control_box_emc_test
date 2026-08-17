#ifndef __APP_BACKHUAL
#define __APP_BACKHUAL

#define SYS_STATE_UPDATA_TIME  (50/SYS_TIME_BASE)
#define SYS_WAIT_ACK_TIME  (300/SYS_TIME_BASE)
#define BLE_ACK_EVENT 0
#define BLE_REPORT_EVENT 1
#define MAX_EVENTS 256  // 根据需要调整
//control_ack_event
typedef enum{
	ACK_RGB_HAVE_EVENT  = 1  				,
	ACK_ALARM_HAVE_EVENT  					,   //问是否有闹钟功能
	ACK_ALARM_CLEAR_EVENT 					,  //取消闹钟成功
	ACK_ALARM_SET_EVENT   					,  //设置闹钟成功
	ACK_VITA_ACTUAL_EVENT           ,  //睡眠检测实时数据
	ACK_REPORT_VITA_ADDR            ,  //睡眠分区设置上传
	TTL_VITA_BLE_ID_READ_EVENT    	,  //睡眠分区设置上传
	ACK_VITA_BLE_ID_EVENT           ,  //睡眠蓝牙广播ID
	ACK_MUSIC_HAVE_EVENT  					,  //问是否有音乐阵子功能
	ACK_MASSAGE_MODE_EVENT					,//模式
	ACK_MASSAGE_INTS_EVENT					,//强度
	ACK_MASSAGE_STATE_EVENT					,//开关
	ACK_MASSAGE_TIME_EVENT					,//时间
	ACK_MUSIC_DEMO_EVENT						,//演示
	ACK_MUSIC_BLE_EVENT							,//蓝牙
	ACK_MUSIC_PLAY_EVENT						,//播放
	ACK_MUSIC_FOLLOW_INITS_EVENT		,//强度
	ACK_MUSIC_VOL_SET_EVENT					,//音量
	ACK_UBL_STATE_EVENT							,//开关
	ACK_RGB_STATE_CHANGE_EVENT 			, //RGB状态
	ACK_RGB_COLOR_CHANGE_EVENT 			, //RGB颜色
	ACK_RGB_TIME_CHANGE_EVENT 			, //RGB时间
	ACK_RGB_MODE_CHANGE_EVENT				, //模式
	ACK_RGB_BREATH_MODE_EVENT				, //呼吸
	ACK_RGB_BRIGHTNESS_EVENT				, //亮度
	ACK_FAN_MODE_EVENT							,//模式
	ACK_FAN_INTS_EVENT							,//强度
	ACK_FAN_TIME_EVENT							,//定时
	ACK_FAN_DIR_EVENT								,//方向
	ACK_BORAD_STATE_EVENT						,//灯牌状态
	ACK_MOTOR_CMD_EVENT							,//电机指令
	ACK_SYNC_MODE_EVENT							,//同控分控
	ACK_DEMO_SLEEP_TIME_EVENT				,//演示模式睡眠时间
	ACK_DEMO_RUN_EVENT							,//演示模式运行状态	
	ACK_MAX_EVENTS									
}ble_ack_event_id;


//ble_report_event

typedef enum {
	REPORT_MASSAGE_MODE_EVENT = 1,        //模式
	REPORT_MASSAGE_INTS_EVENT,        //强度
	REPORT_MASSAGE_STATE_EVENT,       //开关
	REPORT_MASSAGE_TIME_EVENT,        //时间
	REPORT_MUSIC_DEMO_EVENT,          //演示
	REPORT_MUSIC_BLE_EVENT,           //蓝牙
	REPORT_MUSIC_PLAY_EVENT,          //播放
	REPORT_MUSIC_FOLLOW_INITS_EVENT,  //强度
	REPORT_MUSIC_VOL_SET_EVENT,       //音量
	REPORT_UBL_STATE_EVENT,           //开关
	REPORT_RGB_STATE_CHANGE_EVENT,    //RGB状态
	REPORT_RGB_COLOR_CHANGE_EVENT,    //RGB颜色
	REPORT_RGB_TIME_CHANGE_EVENT,     //RGB时间
	REPORT_RGB_MODE_CHANGE_EVENT,     //模式
	REPORT_RGB_BREATH_MODE_EVENT,     //呼吸
	REPORT_RGB_BRIGHTNESS_EVENT,      //亮度
	REPORT_FAN_MODE_EVENT,            //模式
	REPORT_FAN_INTS_EVENT,            //强度
	REPORT_FAN_TIME_EVENT,            //定时
	REPORT_FAN_DIR_EVENT,              //方向
	REPORT_BORAD_STATE_EVENT,         //灯牌状态
	REPORT_MOTOR_CMD_EVENT,						//电机指令
	REPORT_SYNC_MODE_EVENT,           //同控分控
	REPORT_DEMO_SLEEP_TIME_EVENT,			//演示模式睡眠时间
	REPORT_DEMO_RUN_EVENT,						//演示模式运行状态	
	REPORT_MAX_EVENTS
} ble_report_event_id;

typedef void (*ReportHandler)(unsigned char report_type);

typedef struct {
    unsigned short event_id;
    ReportHandler handler;
} ReportEventEntry;




void SYS_UpData_TimeManagerTask(void);
void SYS_UpData_Show(void);
void ble_report_set_event(unsigned char type, unsigned short event) ;

#endif
