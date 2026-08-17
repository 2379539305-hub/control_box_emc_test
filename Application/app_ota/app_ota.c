#include "app_ota.h"
#include "driver_key.h"
#include "driver_eeprom.h"
#include "driver_ble.h"

#include "modul_ttlbus.h"

#include <string.h>

#include "app_config.h"
#include "app_save.h"
//YMODEM 协议组包

typedef enum 
{
  OTA_STATE_FF              = 0xFF, // 初始态 / 未初始化
  OTA_STATE_NORMAL          = 0x01, // 正常运行
  OTA_STATE_START           = 0x02, // 开始OTA
  OTA_STATE_WAIT_HEADER     = 0x03, // 收到升级指令，等待进入 Boot OTA
  OTA_STATE_WAIT_DATA       = 0x04, // Boot 已接收固件信息 Header
  OTA_STATE_RECV_FINISH     = 0x05, // Boot 已接收 B 区固件内容
  OTA_STATE_VERIFY_B        = 0x06, // 校验 B 区固件 CRC/长度
  OTA_STATE_ERASE_A         = 0x07, // 擦除 A 区
  OTA_STATE_COPY_B_TO_A     = 0x08, // B → A 拷贝
  OTA_STATE_VERIFY_A        = 0x09, // 校验 A 区固件 CRC/长度
  OTA_STATE_ERASE_B         = 0x0A, // 擦除 B 区
  OTA_STATE_COMPLETE        = 0x0B  // OTA 完成，写升级状态为 NORMAL
} ota_state_t;

typedef enum 
{
  OTA_SOURCE_R2 = 0x01, // 串口来源
  OTA_SOURCE_BLE  = 0x02  // 蓝牙来源
} ota_source_t;

//=============================================================================
// 需要保存到Flash的OTA信息（支持断点续传）
//=============================================================================
typedef struct 
{
  uint8_t  state;            // OTA状态
  uint8_t  source;           // 固件来源
  uint32_t firmware_length;  // 固件总长度
  uint32_t firmware_crc32;   // 固件CRC32（上位机提供）
} OTA_Info;

//=============================================================================
// 运行时状态（不需要保存）
//=============================================================================
typedef struct 
{
  uint8_t  rx_buffer[256];   // 接收缓冲区
  uint16_t rx_index;         // 接收索引
  uint16_t rx_length;        // 当前帧长度
  uint8_t  expected_seq;     // 期望的下一包序号
  uint8_t  tx_busy;          // 发送忙标志
  uint8_t  first_eot;        // 是否第一次收到EOT
  uint8_t  retry_count;      // 重试计数
  uint32_t timeout_cnt;      // 超时计数器

  uint16_t receive_crc;      // 接收到的CRC值
  uint16_t transmit_crc;     // 计算的CRC值

  uint32_t received_length;  // 已接收长度
  uint32_t current_crc32;    // 当前计算的CRC32
  
  uint8_t  pending_buf[4];   // 上次未写入的字节缓冲
  uint8_t  pending_len;      // 上次未写入的字节数(0-3)
  uint32_t flash_offset;     // 当前Flash写入偏移
} OTA_Runtime;

OTA_Info    ota_info_t = {0};   // 需要保存的信息
OTA_Runtime ota_runtime_t   = {0};   // 运行时状态

#define OTA_REPORT_CODE 0x0001
uint32_t ota_report_event = 0;

//ota启动事件
#define OTA_RECEIVE_START     0x8000
//收到header事件
#define OTA_RECEIVE_HEADER    0x4000
//收到数据事件
#define OTA_RECEIVE_DATA      0x2000
//收到eot事件
#define OTA_RECEIVE_EOT       0x1000
//收到数据结束事件
#define OTA_RECEIVE_FINISH    0x0800
uint32_t ota_receive_event = 0;


#define OTA_FAIL_JUMP					0x8000
uint32_t ota_process_event = 0;
static void App_Ota_Mcu_Reset_Update(void)
{
	RCC->SOFTRST = 0x5C5CAABB; // 然后执行软复位
}
void App_Ota_Info_Init(void)
{
  //初始化OTA信息结构体，并从非易失性存储器读取数据
  Memory_Read(OTA_INFO_ADDR, (uint8_t*)&ota_info_t, sizeof(OTA_Info));
  //清零运行时状态
  memset(&ota_runtime_t, 0, sizeof(OTA_Runtime));
}
void App_Ota_Save_Info(void)
{
	Sector_Erase(OTA_INFO_ADDR);
  //将OTA信息结构体保存到非易失性存储器
  Memory_Write(OTA_INFO_ADDR, (uint8_t*)&ota_info_t, sizeof(OTA_Info));
}

/**
 * @brief 初始化CRC16查表
 */
#define XMODEM_CRC_INIT_VALUE (0x0000)												 // CRC16初始值
static uint16_t crc16_table[256];							// crc16查表
#define XMODEM_POLY_CCITT (0x1021)													 // CRC16-CCITT多项式
static void App_Ota_Ymodem_InitCrc16Table(void)
{
	uint16_t crc = 0x00;
	uint16_t i, j;
	for (i = 0; i < 256; i++)
	{
		crc = i << 8;
		for (j = 0; j < 8; j++)
		{
			if (crc & 0x8000)
			{
				crc = (crc << 1) ^ XMODEM_POLY_CCITT;
			}
			else
			{
				crc = crc << 1;
			}
		}
		crc16_table[i] = crc;
	}
}

void App_Ota_Init(void)
{
  App_Ota_Info_Init();
  App_Ota_Ymodem_InitCrc16Table();
  //初始化只判断初次上电和app正常的情况
  if(ota_info_t.state == OTA_STATE_FF)
  {
    ota_info_t.state = OTA_STATE_NORMAL;
  }
  else if(ota_info_t.state == OTA_STATE_NORMAL)
  {
		
  }
  //其他状态保持不变，由App_Ota_Process处理
}

/**
 * @brief 计算xmodem格式crc值
 *
 * @param data_ptr 要计算crc的数据
 * @param data_len 数据长度
 * @return uint16_t crc校验码
 */

uint16_t App_Ota_Ymodem_CaculateCrc16(const uint8_t *data_ptr, uint32_t data_len)
{
	uint16_t crc = XMODEM_CRC_INIT_VALUE;
	uint32_t i;

	for (i = 0; i < data_len; i++)
	{
		crc = (crc << 8) ^ crc16_table[((crc >> 8) ^ *(data_ptr + i)) & 0xFF];
	}
	return crc & 0xFFFF;
}
void App_Ota_Data_Send(uint8_t source,uint8_t prop,uint8_t func,uint8_t *payload,uint8_t length)
{
	if(source == OTA_SOURCE_R2) //TTL
	{
		uint8_t temp_buff[30];
		//TTL_Upgrade_SendString(temp_buff,);
	}
	else if(source == OTA_SOURCE_BLE) //ble
	{
		uint8_t temp_buff[30];
		uint8_t payload_length = length;
		temp_buff[0] = 0x6e;
		temp_buff[1] = 0x40;
		temp_buff[2] = (payload_length + 9)/256;
		temp_buff[3] = (payload_length + 9)%256;
		temp_buff[4] = prop;
		temp_buff[5] = func;
		
		temp_buff[6] = 0x02;
		
		for(uint8_t i = 0 ; i < length ; i ++)
		{
			temp_buff[7+i] = payload[i];
		}
		// 计算整个帧的crc校验值
		uint16_t crc_val = App_Ota_Ymodem_CaculateCrc16(temp_buff, length+9-2);
		temp_buff[payload_length + 9 - 2] = crc_val/256;
    temp_buff[payload_length + 9 - 1] = crc_val%256;
		BleBlueTooth_SendString(temp_buff,length + 9);
	}
}





void App_Ota_Process(void)
{
  switch(ota_info_t.state)
  {
    case OTA_STATE_NORMAL:
    {
      //正常运行，无需处理
      //等待ota协议握手等等
      if((ota_report_event & OTA_REPORT_CODE) == OTA_REPORT_CODE) //收到管制码读取命令
      {
        uint8_t temp_buff[] = SOURCE_CODE_ID_CONFIG;
        App_Ota_Data_Send(ota_info_t.source,0x01,0x01,temp_buff,sizeof(temp_buff));
        ota_report_event = 0;
      }
      if((ota_receive_event & OTA_RECEIVE_START) == OTA_RECEIVE_START)
      {
        ota_receive_event &= ~OTA_RECEIVE_START;
        ota_info_t.state = OTA_STATE_START;
				App_Ota_Save_Info();
				App_Ota_Mcu_Reset_Update();
      }
    }break;
    default:
    {
      ota_info_t.state = OTA_STATE_NORMAL;
      ota_info_t.firmware_crc32 = 0;
      ota_info_t.firmware_length = 0;
      ota_info_t.source = 0;
      App_Ota_Save_Info();
      ota_process_event |= OTA_FAIL_JUMP;
    }break;
  }
}



void App_Ota_RxServer(unsigned char uart_recv_temp)
{
  //根据协议接收数据
  ota_runtime_t.rx_buffer[ota_runtime_t.rx_index++] = uart_recv_temp;
  if(ota_runtime_t.rx_index >= sizeof(ota_runtime_t.rx_buffer))
  {
    ota_runtime_t.rx_index = 0; //防止溢出
  }
  if(1 == ota_runtime_t.rx_index && ota_runtime_t.rx_buffer[0] != 0x6E)
  {
    ota_runtime_t.rx_index = 0;
  }
  if(5 == ota_runtime_t.rx_index && ota_runtime_t.rx_buffer[1] != 0x40 && ota_runtime_t.rx_buffer[1] != 0x30 && ota_runtime_t.rx_buffer[1] != 0x20)
  {
    ota_runtime_t.rx_index = 0;
  }
	if(ota_runtime_t.rx_index >= 3 && ota_runtime_t.rx_buffer[1] == 0x20)
	{
		ota_runtime_t.rx_index = 0;
	}
  if(ota_runtime_t.rx_index >= 4 && ota_runtime_t.rx_buffer[1] == 0x40)
  {
    uint16_t expected_length = (ota_runtime_t.rx_buffer[2] << 8) | ota_runtime_t.rx_buffer[3];
    if(ota_runtime_t.rx_index >= expected_length)
    {
      uint16_t received_crc = (ota_runtime_t.rx_buffer[expected_length - 2] << 8) | ota_runtime_t.rx_buffer[expected_length - 1];
      uint16_t calculated_crc = App_Ota_Ymodem_CaculateCrc16(ota_runtime_t.rx_buffer, expected_length - 2);
      ota_info_t.source = OTA_SOURCE_BLE;
      ota_runtime_t.timeout_cnt = 0;
      //接收完整包，处理数据
      switch(ota_runtime_t.rx_buffer[5])
      {
        case 0x01: //读取管制码
        {
          ota_report_event |= OTA_REPORT_CODE;
        }break;
        case 0x02: //ota触发
        {
          ota_receive_event |= OTA_RECEIVE_START;
        }break;
        default:
        {
          //未知命令，忽略
        }break;
      }
      ota_runtime_t.rx_index = 0; //重置索引，准备接收下一个包
    }
  }
}



void App_Ota_Timer_1ms(void)
{
  //超时处理
  if(ota_runtime_t.timeout_cnt < 0xFFFFFFFF)
  {
    ota_runtime_t.timeout_cnt++;
  }
}
