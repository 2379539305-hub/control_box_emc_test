#include "driver_beep.h"
#include "delay.h"
//蜂鸣器结构体
typedef struct 
{
  uint16_t  beep_sing_sethz ;
  uint8_t   beep_sing_setnum ;  
}STUBEEP;

STUBEEP StuBeepSing = {0,0};
uint8_t beep_sound_flag = 0;

static uint8_t   beep_sing_num = 0; //响的次数
static uint16_t  beep_sing_hz = 0; //响的频率

static uint8_t sing_complate_flag = 0;

/*
蜂鸣器端口初始化
*/
void Beep_Init(void)
{
	FL_GPIO_InitTypeDef    GPIO_InitStruct  = {0};
	
	GPIO_InitStruct.pin = BEEP_GPIO_PIN;
	GPIO_InitStruct.mode = FL_GPIO_MODE_OUTPUT;
	GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.pull = FL_DISABLE;
	GPIO_InitStruct.remapPin = FL_DISABLE;
	FL_GPIO_Init(BEEP_GPIO, &GPIO_InitStruct);
}

void Beep_ON(void)
{
	FL_GPIO_SetOutputPin(BEEP_GPIO,BEEP_GPIO_PIN);
}

void Beep_OFF(void)
{
	FL_GPIO_ResetOutputPin(BEEP_GPIO,BEEP_GPIO_PIN);
}

void Beep_SoundTask(void)
{
	static uint8_t beep_task_time = 0;
	
	//驱动
	if(1 == beep_sound_flag)
	{
		Beep_ON();
	}
	else
	{
		Beep_OFF();
	}
	//逻辑
	beep_task_time ++;
	if(beep_task_time >= 10)
	{
		beep_task_time = 0;
		Beep_TimerSingTask();
	}
}
/*
* 描述： 蜂鸣器发声函数
* 参数： sing_hz 本次唱歌频率
*        num 本次唱歌响的次数
*        sing_delay 两次唱歌的时间间距       
* 返回： 0 完成  1未完成
*/
uint8_t Beep_SingSetPara(uint16_t sing_hz , uint8_t num)
{	
  if(0 == sing_hz && 0 == num)
  {
    sing_complate_flag = 0;
		beep_sing_num = 0;
    beep_sing_hz = 0;
		StuBeepSing.beep_sing_setnum = 0;
		StuBeepSing.beep_sing_sethz = 0;		
		beep_sound_flag = 0;  //灭
  }
  if(StuBeepSing.beep_sing_setnum == 0) 
  {
    sing_complate_flag = 0;
  }
  if(0 == sing_complate_flag)
  {
		sing_complate_flag = 1;
		beep_sing_num = 0;
    beep_sing_hz = 0;		
		StuBeepSing.beep_sing_setnum = num;
		StuBeepSing.beep_sing_sethz = sing_hz;
  }
	
	return sing_complate_flag;
}
void Beep_StopSing(void)
{
	sing_complate_flag = 0;
	beep_sing_num = 0;
	beep_sing_hz = 0;
	StuBeepSing.beep_sing_setnum = 0;
	beep_sound_flag = 0;  //灭
}
/*
* 描述： 蜂鸣器发声函数
* 参数： sing_hz 本次唱歌频率
*        num 本次唱歌响的次数
*        sing_delay 两次唱歌的时间间距       
* 返回： 无*/
void Beep_TimerSingTask(void)
{	
  if(StuBeepSing.beep_sing_setnum!=0) 
  {
    beep_sing_hz++;
    if(beep_sing_num<StuBeepSing.beep_sing_setnum)
    {
      if(beep_sing_hz<(StuBeepSing.beep_sing_sethz)/2)
      {
        beep_sound_flag = 1;  //响
      }
      else if(beep_sing_hz<StuBeepSing.beep_sing_sethz)
      {
        beep_sound_flag = 0;  //灭
      }
      else 
      {
        beep_sing_hz = 0;
        beep_sing_num ++;
      }
    }
    else
    {
			sing_complate_flag = 0;
      beep_sound_flag = 0;  //灭
      beep_sing_hz = 0;	
      StuBeepSing.beep_sing_setnum = 0;
    }		
  }
  else
  {
    
  }
}












