#ifndef __system_H
#define __system_H

#include "main.h"
#include "string.h"
#include "stdio.h"   //用于printf
#include "stdarg.h"  //用于vsprintf函数原型
#include "stdlib.h" 

#include "fm33lc0xx_fl.h"
#include "fm33lc0xx.h"

#if !defined(UNUSED)
#define UNUSED(X) (void)X      /* To avoid gcc/g++ warnings */
#endif /* UNUSED */

typedef   unsigned char   u8;
typedef   unsigned short  u16;
typedef   unsigned long   u32;

typedef 	unsigned char		uint8_t;
typedef 	unsigned short	uint16_t;
typedef 	unsigned int		uint32_t;

#define SYS_MUSIC_LOCK_STATE    (0X0004)  //音乐锁
#define SYS_CHILD_LOCK_STATE    (0X0001)  //儿童锁
#define SYS_SNORE_CHECK_STATE   (0X0002)  //打鼾干预
#define SYS_SMART_LOCK_STATE    (0x0008) //smart_lock
#define SYS_WIFI_REST_STATE			(0x0020)  //wifi重置标志
extern unsigned short sys_lock_state,old_sys_lock_state;

#define SYS_RESET_FLAG     (0X01)  //重置
#define SYS_REBOOT_STATE   (0X02)  //重启
extern unsigned char sys_re_flag;

#define MOTOR_TOTAL_NUM  (4)


#define SYS_TIME_BASE   5 //单位ms

#define MAIN_FOSC       24000000L   //定义主时钟（精确计算48000000波特率）  22118400L

#define POWER_ON_TIME_MAX 518400000

#define USE_MF_GPIO_INIT  1 /* 如果需要使用MF_Gpio_Init函数，则定义为1，否则定义为0 */
void MF_Gpio_Init(void);


extern unsigned short local_mcu_id;
extern unsigned long power_on_time;

void system_Init(void);

unsigned short GetMcu_CrcID(void);
unsigned char Decimal_To_Bcd(unsigned char val);
int Bcd_To_Decimal(unsigned char bcd_num);

int user_strncmp(const char * str1,const char * str2,size_t n);
void Sys_Control(void);

#endif






