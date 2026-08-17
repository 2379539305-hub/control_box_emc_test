#include "modul_ttlbus.h"

RingBufferStruct  RingBuffer = {0};


unsigned char TTL_BUS_RXBuffer[TTL_BUF_LENGTH]; 
unsigned char TTL_BUS_TXBuffer[TTL_BUF_LENGTH];

unsigned char Device_InfoArr[DEVICE_NUM_MAX + 1][DEVICE_INFO_MAX + 1] = {0}; 
unsigned char TTL_DeviceOnlineCount[256] = {0};


static unsigned char master_radio_add = 0XFF;
static unsigned char master_clear_add = 0x00;

static unsigned char master_release_step = 0; 
static unsigned char slave_replycall_flag = 0;

static unsigned char slave_device_type = 0;
static unsigned char slave_device_add = 0;
static unsigned char slave_secret_key = 0;


static unsigned char master_poll_ask_wait = 0;
static unsigned char master_poll_ask_mutex = 0;
static unsigned char master_poll_ask_add = 0;


static unsigned char ttl_recv_length = 0;  
static unsigned char ttl_recv_succeed_flag = 0; 
static unsigned short ttl_recv_long_time = TTL_FREE_LONG_TIME;

unsigned char ttl_tx_busy = 0; 

static unsigned char ttl_bus_init = 0;

void TTL_Master_UartDataReturn(unsigned char data_temp);
void TTL_Master_RecvDataAnalyTask(void);
unsigned char TTL_Master_ClearAddTask(unsigned char clear_add_temp);

unsigned char TTL_Master_ReplyReleaseAddCmd(unsigned char slave_type_temp,unsigned char release_add_temp,unsigned char secret_key_temp); 
unsigned char TTL_Master_ClearAddCmd(unsigned char clear_add_temp); 
unsigned char TTL_Master_SendCallSlaveCmd(unsigned char slave_type_temp,unsigned char slave_add_temp); 

unsigned char TTL_Master_ConnectSlaveCmd(unsigned char slave_type_temp,unsigned char slave_add_temp,unsigned char secret_key_temp,unsigned char connect_temp); 
unsigned char TTL_Master_AskSlaveStatusCmd(unsigned char slave_add_temp,unsigned char cmd_temp,unsigned char extend_byte1,unsigned char extend_byte2);
unsigned char TTL_Master_ReturnDataCompareCmd(void); //0,1,2

unsigned char Check_Busy_Free(void);

static void TTL_DeviceTypeOnlineAdd(unsigned char device_type)
{
	if((device_type != 0) && (TTL_DeviceOnlineCount[device_type] < 0xFF))
	{
		TTL_DeviceOnlineCount[device_type] = 1;
	}
}

static void TTL_DeviceTypeOnlineSub(unsigned char device_type)
{
	if((device_type != 0) && (TTL_DeviceOnlineCount[device_type] > 0))
	{
		TTL_DeviceOnlineCount[device_type] = 0;
	}
}

static void TTL_DeviceTypeSetByAdd(unsigned char device_add, unsigned char new_type)
{
	unsigned char old_type = Device_InfoArr[device_add][DEVICE_INFO_ARR_TYPE];

	if(old_type == new_type)
	{
		return;
	}

	TTL_DeviceTypeOnlineAdd(new_type);
}

void RingBuffer_Init(void)
{
	RingBuffer.ring_write_data_lock = 0;
	RingBuffer.tail = 0;
	RingBuffer.heard = 0;
}
unsigned char TTL_RingBuffer_CheckEnough(unsigned char need_len)
{
	unsigned short ring_free_len = 0;
	unsigned short ring_used_len = 0;

	if(RingBuffer.tail >= RingBuffer.heard)
	{
		ring_used_len = (unsigned char)(RingBuffer.tail - RingBuffer.heard);
	}
	else
	{
		ring_used_len = (unsigned char)(RINGBUFF_LEN - (RingBuffer.heard - RingBuffer.tail));
	}

	ring_free_len = (unsigned char)(RINGBUFF_LEN - ring_used_len);

	if(need_len > (ring_free_len - 1))
	{
		return 0;
	}

	return 1;
}	

unsigned char Write_Ring_Data(unsigned char data_temp)
{
	if(((RingBuffer.tail + 1) % RINGBUFF_LEN) == RingBuffer.heard)
	{
		return 1;
	}
	RingBuffer.ring_buffer[RingBuffer.tail] = data_temp;
	RingBuffer.tail = (RingBuffer.tail + 1) % RINGBUFF_LEN; 
	return 0;
}

unsigned char Read_Ring_Data(unsigned char *rData)
{
	if(RingBuffer.heard == RingBuffer.tail)
	{
		return 1;
	}	
	*rData = RingBuffer.ring_buffer[RingBuffer.heard];
	RingBuffer.heard = (RingBuffer.heard + 1) % RINGBUFF_LEN; 
	
	return 0;
}

void TTL_Bus_RxServer(unsigned char uart_recv_value)
{
	unsigned char temp_i = 0;
	unsigned short recv_check_sum = 0; 
	
	ttl_recv_long_time = 0;
	
	if(0 == ttl_bus_init) return;
	//
	TTL_Master_UartDataReturn(uart_recv_value); 
	//
	if(ttl_recv_length < TTL_BUF_LENGTH)
	{
		TTL_BUS_RXBuffer[ttl_recv_length ++] = uart_recv_value;
	}
	else
	{
		ttl_recv_succeed_flag = 2;
		ttl_recv_length = 0;
	}
	
	if(1 == ttl_recv_length && TTL_BUS_RXBuffer[0] != 0XFB)
	{
		ttl_recv_length = 0;
		ttl_recv_succeed_flag = 0;
	}
	if(2 == ttl_recv_length && TTL_BUS_RXBuffer[1] != 0X5A)
	{
		ttl_recv_length = 0;
		ttl_recv_succeed_flag = 0;		
	}
	
	if(ttl_recv_length >= 3)
	{
		if(ttl_recv_length >= TTL_BUS_RXBuffer[2])
		{
			for(temp_i = 2;temp_i < TTL_BUS_RXBuffer[2]-1;temp_i ++)
			{
				recv_check_sum += TTL_BUS_RXBuffer[temp_i];
			}
			if(recv_check_sum%256 == TTL_BUS_RXBuffer[ttl_recv_length - 1])
			{
				ttl_recv_succeed_flag = 1;
			}
			else
			{
				ttl_recv_succeed_flag = 2;
			}
			
			ttl_recv_length = 0;
		}	
	}
	//
	if(1 == ttl_recv_succeed_flag)
	{
		ttl_recv_succeed_flag = 0;
		TTL_Master_RecvDataAnalyTask();
	}
}

void TTL_Bus_TxServer(void)
{
	static unsigned char send_data = 0;
	if(0 == ttl_tx_busy)
	{
		if(!Read_Ring_Data((unsigned char*) &send_data))
		{
			ttl_tx_busy = 1;
			TTL_Bus_SendData(send_data);
		}
	}
}


unsigned char Master_SearchIdleAdd(u8 device_type_temp) 
{
	unsigned char temp_i = 0;
	
	for(temp_i = 1;temp_i <= DEVICE_NUM_MAX;temp_i ++)
	{
		if(Device_InfoArr[temp_i][DEVICE_INFO_ARR_TYPE] == device_type_temp)  
		{
			break;
		}
	}
	
	if(temp_i > DEVICE_NUM_MAX)
	{
		return 0;
	}

	return temp_i;
}

unsigned char Master_CompareConnectAttr(unsigned char new_arrt_temp)
{
	unsigned char slave_add_i = 0;
	if((new_arrt_temp & 0x03) == 0x02) 
	{
		for(slave_add_i = 1;slave_add_i<=DEVICE_NUM_MAX;slave_add_i++)
		{
			if(Device_InfoArr[slave_add_i][DEVICE_INFO_ARR_TYPE] != 0 && (Device_InfoArr[slave_add_i][DEVICE_INFO_ARR_COMM] & 0x03) == 0X02) 
			{
				if((Device_InfoArr[slave_add_i][DEVICE_INFO_ARR_COMM] & 0x0c) >= (new_arrt_temp & 0x0c))
				{
					return 0;
				}
				else
				{
					Device_InfoArr[slave_add_i][DEVICE_INFO_ARR_COMM] = 0x00; 
					return 1;
				}
			}
		}
	}
	
	return 1;
}

void Device_AddInit(void)
{
	memset(Device_InfoArr,0x00,sizeof(Device_InfoArr)/sizeof(Device_InfoArr[0][0]));
	memset(TTL_DeviceOnlineCount,0x00,sizeof(TTL_DeviceOnlineCount));
}

unsigned char TTL_Master_ClearAddTask(unsigned char clear_add_temp) 
{
	unsigned char send_count = 0;
	
	for(send_count = 0;send_count<3;send_count++)
	{
		TTL_Master_ClearAddCmd(clear_add_temp);
	}
	
	return 1;
}

unsigned char Slave_AllDisConnect(void)
{
	unsigned char temp_i = 0;
	
	for(temp_i = 1;temp_i <= DEVICE_NUM_MAX;temp_i++)
	{
		if(Device_InfoArr[temp_i][DEVICE_INFO_ARR_COMM] != 0x00)
		{
			return 0;
		}
	}
	
	return 1;
}


void TTL_Master_RecvDataAnalyTask(void) //
{
	unsigned char temp_para = 0;
	
	if(TTL_BUS_RXBuffer[TTL_CODE_BIT] == 0x01) 
	{
		if(TTL_BUS_RXBuffer[TTL_SLAVE_TYPE_BIT] == MUSIC_DEVICE_TYPE) 
		{
			temp_para = Master_SearchIdleAdd(TTL_BUS_RXBuffer[TTL_SLAVE_TYPE_BIT]);
			if(0 == temp_para)
			{
				slave_device_type = TTL_BUS_RXBuffer[TTL_SLAVE_TYPE_BIT];
				slave_secret_key = TTL_BUS_RXBuffer[TTL_SECRET_KEY_BIT];
				master_release_step = 1;		
			}
			else 
			{
				master_clear_add = temp_para;
				TTL_DeviceTypeSetByAdd(master_clear_add,0);
				Device_InfoArr[master_clear_add][DEVICE_INFO_ARR_TYPE] = 0;
			}
			
		}
		else
		{
			slave_device_type = TTL_BUS_RXBuffer[TTL_SLAVE_TYPE_BIT];
			slave_secret_key = TTL_BUS_RXBuffer[TTL_SECRET_KEY_BIT];
			master_release_step = 1;				
		}
	}
	else if(TTL_BUS_RXBuffer[TTL_CODE_BIT] == 0x02) 
	{
		if(TTL_BUS_RXBuffer[TTL_SLAVE_TYPE_BIT] == slave_device_type && TTL_BUS_RXBuffer[TTL_SLAVE_ADD_BIT] == master_radio_add)
		{
			slave_replycall_flag = 1;
		}
	}
	else if(TTL_BUS_RXBuffer[TTL_CODE_BIT] == 0x03) 
	{
		if(Device_InfoArr[TTL_BUS_RXBuffer[TTL_SLAVE_ADD_BIT]][DEVICE_INFO_ARR_TYPE] == TTL_BUS_RXBuffer[TTL_SLAVE_TYPE_BIT]) 
		{
			if(0 == Master_CompareConnectAttr(TTL_BUS_RXBuffer[8])) 
			{
				TTL_Master_ConnectSlaveCmd(TTL_BUS_RXBuffer[TTL_SLAVE_TYPE_BIT],TTL_BUS_RXBuffer[TTL_SLAVE_ADD_BIT],TTL_BUS_RXBuffer[TTL_SECRET_KEY_BIT],0x00);
			}
			else
			{
				Device_InfoArr[TTL_BUS_RXBuffer[TTL_SLAVE_ADD_BIT]][DEVICE_INFO_ARR_SK] = TTL_BUS_RXBuffer[TTL_SECRET_KEY_BIT];
				Device_InfoArr[TTL_BUS_RXBuffer[TTL_SLAVE_ADD_BIT]][DEVICE_INFO_ARR_COMM] = TTL_BUS_RXBuffer[8]; 	
				Device_InfoArr[TTL_BUS_RXBuffer[TTL_SLAVE_ADD_BIT]][DEVICE_INFO_ARR_POLLNUM] = 0;
				TTL_Master_ConnectSlaveCmd(TTL_BUS_RXBuffer[TTL_SLAVE_TYPE_BIT],TTL_BUS_RXBuffer[TTL_SLAVE_ADD_BIT],TTL_BUS_RXBuffer[TTL_SECRET_KEY_BIT],TTL_BUS_RXBuffer[8]);
			}
		}
		else
		{
			master_clear_add = TTL_BUS_RXBuffer[TTL_SLAVE_ADD_BIT];
		}
	}
	else 
	{
		if(TTL_BUS_RXBuffer[TTL_CODE_BIT] >= 0x20)		
		{
			if(
				 master_poll_ask_add == TTL_BUS_RXBuffer[TTL_SLAVE_ADD_BIT] && 
				 Device_InfoArr[master_poll_ask_add][DEVICE_INFO_ARR_TYPE] == TTL_BUS_RXBuffer[TTL_SLAVE_TYPE_BIT] //&& 
//				 Device_InfoArr[master_poll_ask_add][DEVICE_INFO_ARR_SK] == TTL_BUS_RXBuffer[TTL_SECRET_KEY_BIT]
				) 
			{
				TTL_Bus_UartDataAnaly(TTL_BUS_RXBuffer[TTL_SLAVE_TYPE_BIT]);		
			}
			else
			{
				master_clear_add = TTL_BUS_RXBuffer[TTL_SLAVE_ADD_BIT];
			}

//			if(TTL_BUS_RXBuffer[8] == 0x00)
//			{
//				Device_InfoArr[master_poll_ask_add][DEVICE_INFO_ARR_COMM] = 0X00; 
//			}	
			Device_InfoArr[master_poll_ask_add][DEVICE_INFO_ARR_COMM] = TTL_BUS_RXBuffer[8]; 		//从长连接直接切换到短链接  需要实时更新连接属性值
			
			if((Device_InfoArr[master_poll_ask_add][DEVICE_INFO_ARR_COMM] & 0x03) == 0x00)  
			{
				Device_InfoArr[master_poll_ask_add][DEVICE_INFO_ARR_COMM] = 0X00; 
			}
			else if((Device_InfoArr[master_poll_ask_add][DEVICE_INFO_ARR_COMM] & 0x03) == 0x01) 
			{
				TTL_Master_ReturnDataCompareCmd();
			}				
			else if((Device_InfoArr[master_poll_ask_add][DEVICE_INFO_ARR_COMM] & 0x03) == 0x02) 
			{
				
			}
			if(Device_InfoArr[master_poll_ask_add][DEVICE_INFO_ARR_POLLNUM] > 0) Device_InfoArr[master_poll_ask_add][DEVICE_INFO_ARR_POLLNUM] = 0;
			master_poll_ask_mutex = 0;
			
		}				
	}
}

void Master_ReleaseAddProcc(void)
{
	static unsigned char master_call_add = 0x00;
	static unsigned char wait_time_temp = 0;
	
	if(1 == master_release_step) 
	{
		wait_time_temp = 0;
		master_radio_add = Master_SearchIdleAdd(0);
		if(master_radio_add != 0)
		{
			if(1 == TTL_Master_ReplyReleaseAddCmd(slave_device_type,master_radio_add,slave_secret_key))
			{
				master_release_step = 2;
			}		
		}
		else 
		{
			master_release_step = 2;
			
			master_call_add ++;
			if(master_call_add > DEVICE_NUM_MAX) master_call_add = 1;
			
			master_radio_add = master_call_add;
			slave_device_type = Device_InfoArr[master_radio_add][DEVICE_INFO_ARR_TYPE];
		}
	}
	else if(2 == master_release_step)
	{
		wait_time_temp = 0;
		if(1 == TTL_Master_SendCallSlaveCmd(slave_device_type,master_radio_add))
		{
			slave_replycall_flag = 0;
			master_release_step = 3;
		}
	}
	else if(master_release_step >= 3)
	{
		wait_time_temp ++;
		if(wait_time_temp <= TTL_WAIT_TIME)
		{
			if(1 == slave_replycall_flag) 
			{
				slave_replycall_flag = 0;
				master_release_step = 0;
				TTL_DeviceTypeSetByAdd(master_radio_add,slave_device_type);
				Device_InfoArr[master_radio_add][DEVICE_INFO_ARR_TYPE] = slave_device_type;
			}			
		}
		else
		{
			master_release_step = 0;
			master_clear_add = master_radio_add;
			TTL_DeviceTypeSetByAdd(master_radio_add,0);
			Device_InfoArr[master_radio_add][DEVICE_INFO_ARR_TYPE] = 0;
		}
		
	}	
}

void Master_PollingProcc(void)
{
	static unsigned char long_connect_add = 1;
	static unsigned char short_connect_add = 1;
	static unsigned char master_ask_time = 0; 
	

	master_ask_time ++;
	if(master_ask_time >= TTL_ASK_TIME) 
	{
		master_ask_time = TTL_ASK_TIME;
		if(0 == master_poll_ask_mutex)
		{
			master_ask_time = 0; 
			for(long_connect_add = 1;long_connect_add <= DEVICE_NUM_MAX;long_connect_add ++) 
			{
				if(Device_InfoArr[long_connect_add][DEVICE_INFO_ARR_TYPE] != 0 && (Device_InfoArr[long_connect_add][DEVICE_INFO_ARR_COMM] & 0X03) == 0X02) 
				{
					master_poll_ask_add = long_connect_add;
					
					Device_InfoArr[master_poll_ask_add][DEVICE_INFO_ARR_POLLNUM] ++;	
					
					if(Device_InfoArr[master_poll_ask_add][DEVICE_INFO_ARR_POLLNUM] < 3) 
					{
						master_poll_ask_mutex = 1;
						master_poll_ask_wait = 0;
						if(Device_InfoArr[master_poll_ask_add][DEVICE_INFO_ARR_CODE] != 0x00)
						{
							TTL_Master_AskSlaveStatusCmd(master_poll_ask_add,Device_InfoArr[master_poll_ask_add][DEVICE_INFO_ARR_CODE],Device_InfoArr[master_poll_ask_add][DEVICE_INFO_ARR_EXTEND1],Device_InfoArr[master_poll_ask_add][DEVICE_INFO_ARR_EXTEND2]); 
							Device_InfoArr[master_poll_ask_add][DEVICE_INFO_ARR_CODE] = 0x00;
						}
						else
						{
							TTL_Master_AskSlaveStatusCmd(master_poll_ask_add,0x20,0x00,0x00); 
						}
						
					}
					else  
					{
						Device_InfoArr[master_poll_ask_add][DEVICE_INFO_ARR_COMM] = 0x00; 
						Device_InfoArr[master_poll_ask_add][DEVICE_INFO_ARR_CODE] = 0x00;
						Device_InfoArr[master_poll_ask_add][DEVICE_INFO_ARR_POLLNUM] = 0;
						
						//清空执行的指令或者其他动作
										
					}
					break;
				}
			}			
		}
	}
	else
	{
		if(0 == master_poll_ask_mutex)
		{
			//问询状态
			if(short_connect_add > DEVICE_NUM_MAX)  short_connect_add = 1;
			
			for(;short_connect_add <= DEVICE_NUM_MAX;short_connect_add ++) 
			{
				if(Device_InfoArr[short_connect_add][DEVICE_INFO_ARR_TYPE] != 0 && (Device_InfoArr[short_connect_add][DEVICE_INFO_ARR_COMM] & 0X03) == 0X01) 
				{
					master_poll_ask_add = short_connect_add;
					
					Device_InfoArr[master_poll_ask_add][DEVICE_INFO_ARR_POLLNUM] ++;	
					
					if(Device_InfoArr[master_poll_ask_add][DEVICE_INFO_ARR_POLLNUM] < 3)
					{
						master_poll_ask_mutex = 1;
						master_poll_ask_wait = 0;
						if(Device_InfoArr[master_poll_ask_add][DEVICE_INFO_ARR_CODE] != 0x00)
						{
							TTL_Master_AskSlaveStatusCmd(master_poll_ask_add,Device_InfoArr[master_poll_ask_add][DEVICE_INFO_ARR_CODE],Device_InfoArr[master_poll_ask_add][DEVICE_INFO_ARR_EXTEND1],Device_InfoArr[master_poll_ask_add][DEVICE_INFO_ARR_EXTEND2]); 
							Device_InfoArr[master_poll_ask_add][DEVICE_INFO_ARR_CODE] = 0x00;
						}		
						else
						{
							TTL_Master_AskSlaveStatusCmd(master_poll_ask_add,0x20,0x00,0x00); 
						}
					}
					else
					{
						Device_InfoArr[master_poll_ask_add][DEVICE_INFO_ARR_COMM] = 0x00; 
						Device_InfoArr[master_poll_ask_add][DEVICE_INFO_ARR_CODE] = 0X00;
						Device_InfoArr[master_poll_ask_add][DEVICE_INFO_ARR_POLLNUM] = 0;					
					}
					
					break;
				}
			}		
		}
	}
	if(1 == master_poll_ask_mutex)
	{
		master_poll_ask_wait ++;
		if(master_poll_ask_wait >= TTL_WAIT_TIME) 
		{
			master_poll_ask_wait = 0;
			master_poll_ask_mutex = 0;
		}
	}
	else
	{
		master_poll_ask_wait = 0;
	}
	/*--------------------------------------------------------------------------------------------------------*/
}

void Master_CallAllSlave(void)
{
	static unsigned char slave_add_temp = 1;
	static unsigned char master_ask_flag = 0;
	
	static unsigned char wait_time_temp = 0;
	
	if(0 == master_release_step)
	{
		if(slave_add_temp > DEVICE_NUM_MAX) slave_add_temp = 1;
		
		if(0 == master_ask_flag)
		{
			if(Device_InfoArr[slave_add_temp][DEVICE_INFO_ARR_TYPE] != 0)
			{
				master_ask_flag = 1;
				
				master_radio_add = slave_add_temp;
				slave_device_type = Device_InfoArr[master_radio_add][DEVICE_INFO_ARR_TYPE];		
				
				if(TTL_Master_SendCallSlaveCmd(slave_device_type,master_radio_add))
				{
					slave_replycall_flag = 0;
					wait_time_temp = 0;
				}
			}
			else
			{
				slave_add_temp ++;
			}
		}
		else
		{
			wait_time_temp ++;
			if(wait_time_temp <= TTL_WAIT_TIME)
			{
				if(1 == slave_replycall_flag) 
				{
					master_ask_flag = 0;
					slave_add_temp ++;
				}			
			}
			else  
			{
				master_ask_flag = 0;
				master_clear_add = master_radio_add;
				TTL_DeviceTypeSetByAdd(master_radio_add,0);
				Device_InfoArr[master_radio_add][DEVICE_INFO_ARR_TYPE] = 0;
			}
		}	
	}
	else
	{
		slave_add_temp = 1;
		master_ask_flag = 0;
	}
}
void Master_ReplySlaveTask(void)
{
	if(RingBuffer.ring_write_data_lock != 0) return;
	
	RingBuffer.ring_write_data_lock = 1;
	Master_ReleaseAddProcc();
	Master_PollingProcc();
	if(master_clear_add != 0)
	{
		if(1 == TTL_Master_ClearAddTask(master_clear_add))
		{
			master_clear_add = 0;
		}
	}
	if(1 == Slave_AllDisConnect())
	{
//		Master_CallAllSlave();
	}
	
	/*-------------------------------------------------------------*/
	RingBuffer.ring_write_data_lock = 0;
	//
}

unsigned char TTL_Check_Busy_Free(void)
{
  if(ttl_recv_long_time>=TTL_FREE_LONG_TIME)
  {
    return 1;
  }
  return 0;
}


void TTL_Bus_TimerManager(void)
{
  ttl_recv_long_time ++;
  if(ttl_recv_long_time>=TTL_FREE_LONG_TIME)
  {
    ttl_recv_long_time = TTL_FREE_LONG_TIME;
    ttl_recv_length = 0;
  }
	//
	Master_ReplySlaveTask(); 
	//
}

void TTL_Bus_ClearTxBusy(void)
{
	ttl_tx_busy = 0;
}

void TTL_Bus_Init(void)
{
	TTL_Bus_ClearTxBusy();
	RingBuffer_Init();
	master_clear_add = 0xFF;
	ttl_bus_init = 1;
}

/*--------------------------------------------------------------------------------------------------------------------*/
unsigned char TTL_Master_ClearAddCmd(unsigned char clear_add_temp) 
{
	unsigned short temp_i = 0;
	
	unsigned char bus_send_length = 10;

	if(TTL_RingBuffer_CheckEnough(bus_send_length) == 0) return 0;
	
//	if(RingBuffer.ring_write_data_lock != 0) return 0;
//	
//	RingBuffer.ring_write_data_lock = 1;		
	
	TTL_BUS_TXBuffer[0] = TTL_HEADER_ONE;TTL_BUS_TXBuffer[1] = TTL_HEADER_TWO;
	TTL_BUS_TXBuffer[2] = bus_send_length; TTL_BUS_TXBuffer[3] = 0X00;
	TTL_BUS_TXBuffer[4] = 0XFF; TTL_BUS_TXBuffer[5] = clear_add_temp; 
	TTL_BUS_TXBuffer[6] = 0X00; TTL_BUS_TXBuffer[7] = 0X00; 
	TTL_BUS_TXBuffer[8] = 0XFF;
	temp_i = TTL_BUS_TXBuffer[2]+TTL_BUS_TXBuffer[3]+TTL_BUS_TXBuffer[4]+TTL_BUS_TXBuffer[5]+TTL_BUS_TXBuffer[6]+TTL_BUS_TXBuffer[7]+TTL_BUS_TXBuffer[8];
	TTL_BUS_TXBuffer[9] = temp_i%256;	
		
	for(temp_i = 0;temp_i < bus_send_length;temp_i ++)
	{
		if(Write_Ring_Data(TTL_BUS_TXBuffer[temp_i]) == 1)
		{
			break;
		}
	}		
		
//	RingBuffer.ring_write_data_lock = 0;
	
	if(temp_i == bus_send_length)  return 1;

	return 0;
}

unsigned char TTL_Master_ReplyReleaseAddCmd(unsigned char slave_type_temp,unsigned char release_add_temp,unsigned char secret_key_temp)
{
	unsigned short temp_i = 0;

	unsigned char bus_send_length = 10;

	if(TTL_RingBuffer_CheckEnough(bus_send_length) == 0) return 0;
	
//	if(RingBuffer.ring_write_data_lock != 0) return 0;
//	RingBuffer.ring_write_data_lock = 1;		
	
	TTL_BUS_TXBuffer[0] = TTL_HEADER_ONE;TTL_BUS_TXBuffer[1] = TTL_HEADER_TWO;
	TTL_BUS_TXBuffer[2] = bus_send_length; TTL_BUS_TXBuffer[3] = 0X00;
	TTL_BUS_TXBuffer[4] = slave_type_temp; TTL_BUS_TXBuffer[5] = 0XFF; 
	TTL_BUS_TXBuffer[6] = secret_key_temp; TTL_BUS_TXBuffer[7] = 0X01; 
	TTL_BUS_TXBuffer[8] = release_add_temp;
	temp_i = TTL_BUS_TXBuffer[2]+TTL_BUS_TXBuffer[3]+TTL_BUS_TXBuffer[4]+TTL_BUS_TXBuffer[5]+TTL_BUS_TXBuffer[6]+TTL_BUS_TXBuffer[7]+TTL_BUS_TXBuffer[8];
	TTL_BUS_TXBuffer[9] = temp_i%256;	
		

	for(temp_i = 0;temp_i < bus_send_length;temp_i ++)
	{
		if(Write_Ring_Data(TTL_BUS_TXBuffer[temp_i]) == 1)
		{
			break;
		}
	}	

//	RingBuffer.ring_write_data_lock = 0;
	
	if(temp_i == bus_send_length)  return 1;

	return 0;
}

unsigned char TTL_Master_SendCallSlaveCmd(unsigned char slave_type_temp,unsigned char slave_add_temp) 
{
	unsigned short temp_i = 0;
	
	unsigned char bus_send_length = 9;

	if(TTL_RingBuffer_CheckEnough(bus_send_length) == 0) return 0;
	
//	if(RingBuffer.ring_write_data_lock != 0) return 0;
//	RingBuffer.ring_write_data_lock = 1;		
	
	TTL_BUS_TXBuffer[0] = TTL_HEADER_ONE;TTL_BUS_TXBuffer[1] = TTL_HEADER_TWO;
	TTL_BUS_TXBuffer[2] = bus_send_length; TTL_BUS_TXBuffer[3] = 0X00;
	TTL_BUS_TXBuffer[4] = slave_type_temp; TTL_BUS_TXBuffer[5] = slave_add_temp; 
	TTL_BUS_TXBuffer[6] = 0X00; TTL_BUS_TXBuffer[7] = 0X02; 
	temp_i = TTL_BUS_TXBuffer[2]+TTL_BUS_TXBuffer[3]+TTL_BUS_TXBuffer[4]+TTL_BUS_TXBuffer[5]+TTL_BUS_TXBuffer[6]+TTL_BUS_TXBuffer[7];
	TTL_BUS_TXBuffer[8] = temp_i%256;	
		

	for(temp_i = 0;temp_i < bus_send_length;temp_i ++)
	{
		if(Write_Ring_Data(TTL_BUS_TXBuffer[temp_i]) == 1)
		{
			break;
		}
	}	
		
//	RingBuffer.ring_write_data_lock = 0;
	
	if(temp_i == bus_send_length)  return 1;

	return 0;
}

unsigned char TTL_Master_ConnectSlaveCmd(unsigned char slave_type_temp,unsigned char slave_add_temp,unsigned char secret_key_temp,unsigned char connect_temp) 
{
	unsigned short temp_i = 0;
	
	unsigned char bus_send_length = 10;

	if(TTL_RingBuffer_CheckEnough(bus_send_length) == 0) return 0;
	
//	if(RingBuffer.ring_write_data_lock != 0) return 0;	
//	RingBuffer.ring_write_data_lock = 1;		
	
	TTL_BUS_TXBuffer[0] = TTL_HEADER_ONE;TTL_BUS_TXBuffer[1] = TTL_HEADER_TWO;
	TTL_BUS_TXBuffer[2] = bus_send_length; TTL_BUS_TXBuffer[3] = 0X00;
	TTL_BUS_TXBuffer[4] = slave_type_temp; TTL_BUS_TXBuffer[5] = slave_add_temp; 
	TTL_BUS_TXBuffer[6] = secret_key_temp; TTL_BUS_TXBuffer[7] = 0X03; 
	TTL_BUS_TXBuffer[8] = connect_temp;  //0断开连接  1建立连接
	temp_i = TTL_BUS_TXBuffer[2]+TTL_BUS_TXBuffer[3]+TTL_BUS_TXBuffer[4]+TTL_BUS_TXBuffer[5]+TTL_BUS_TXBuffer[6]+TTL_BUS_TXBuffer[7]+TTL_BUS_TXBuffer[8];
	TTL_BUS_TXBuffer[9] = temp_i%256;	
		

	for(temp_i = 0;temp_i < bus_send_length;temp_i ++)
	{
		if(Write_Ring_Data(TTL_BUS_TXBuffer[temp_i]) == 1)
		{
			break;
		}
	}	
		
//	RingBuffer.ring_write_data_lock = 0;
	
	if(temp_i == bus_send_length)  return 1;

	return 0;
}
unsigned char TTL_Master_AskSlaveStatusCmd(unsigned char slave_add_temp,unsigned char cmd_temp,unsigned char extend_byte1,unsigned char extend_byte2) 
{
	unsigned short temp_i = 0;

	unsigned char bus_send_length = 12;

	if(TTL_RingBuffer_CheckEnough(bus_send_length) == 0) return 0;
	
//	if(RingBuffer.ring_write_data_lock != 0) return 0;
//	RingBuffer.ring_write_data_lock = 1;		
	
	TTL_BUS_TXBuffer[0] = TTL_HEADER_ONE;TTL_BUS_TXBuffer[1] = TTL_HEADER_TWO;
	TTL_BUS_TXBuffer[2] = bus_send_length; TTL_BUS_TXBuffer[3] = 0X00;
	TTL_BUS_TXBuffer[4] = Device_InfoArr[slave_add_temp][DEVICE_INFO_ARR_TYPE]; 
	TTL_BUS_TXBuffer[5] = slave_add_temp; 
	TTL_BUS_TXBuffer[6] = Device_InfoArr[slave_add_temp][DEVICE_INFO_ARR_SK]; 
	TTL_BUS_TXBuffer[7] = cmd_temp; 
	TTL_BUS_TXBuffer[8] = 0x00;
	TTL_BUS_TXBuffer[9] = extend_byte1;
	TTL_BUS_TXBuffer[10] = extend_byte2;
	temp_i = TTL_BUS_TXBuffer[2]+TTL_BUS_TXBuffer[3]+TTL_BUS_TXBuffer[4]+TTL_BUS_TXBuffer[5]+TTL_BUS_TXBuffer[6]+TTL_BUS_TXBuffer[7] + TTL_BUS_TXBuffer[8] + TTL_BUS_TXBuffer[9] + TTL_BUS_TXBuffer[10];
	TTL_BUS_TXBuffer[11] = temp_i%256;	
		

	for(temp_i = 0;temp_i < bus_send_length;temp_i ++)
	{
		if(Write_Ring_Data(TTL_BUS_TXBuffer[temp_i]) == 1)
		{
			break;
		}
	}	
		
//	RingBuffer.ring_write_data_lock = 0;
	
	if(temp_i == bus_send_length)  return 1;

	return 0;
}




unsigned char TTL_Master_ReturnDataCompareCmd(void) //0,1,2
{
	unsigned char temp_i = 0;
	unsigned short sum_temp = 0;
	unsigned char bus_send_length = TTL_BUS_RXBuffer[2];

	if(TTL_RingBuffer_CheckEnough(bus_send_length) == 0) return 0;
	
//	if(RingBuffer.ring_write_data_lock != 0) return 0;
//	RingBuffer.ring_write_data_lock = 1;	
	
	for(temp_i = 0;temp_i < TTL_BUS_RXBuffer[2];temp_i ++)
	{
		TTL_BUS_TXBuffer[temp_i] = TTL_BUS_RXBuffer[temp_i];
	}
	TTL_BUS_TXBuffer[0] = TTL_HEADER_ONE;TTL_BUS_TXBuffer[1] = TTL_HEADER_TWO;
	TTL_BUS_TXBuffer[8] = 0x02;//命令字
	
	for(temp_i = 2;temp_i < TTL_BUS_TXBuffer[2] - 1;temp_i ++)
	{
		sum_temp += TTL_BUS_TXBuffer[temp_i];
	}
	TTL_BUS_TXBuffer[TTL_BUS_TXBuffer[2] - 1] = sum_temp%256;
		
		
	for(temp_i = 0;temp_i < bus_send_length;temp_i ++)
	{
		if(Write_Ring_Data(TTL_BUS_TXBuffer[temp_i]) == 1)
		{
			break;
		}
	}	
			
//	RingBuffer.ring_write_data_lock = 0;
	
	if(temp_i == bus_send_length)  return 1;

	return 0;
}

unsigned char Master_ReturnDataNoEffect(void) //
{
	unsigned char temp_i = 0;
	unsigned short sum_temp = 0;
	unsigned char bus_send_length = TTL_BUS_RXBuffer[2];

	if(TTL_RingBuffer_CheckEnough(bus_send_length) == 0) return 0;
	
//	if(RingBuffer.ring_write_data_lock != 0) return 0;
//	RingBuffer.ring_write_data_lock = 1;
	
	for(temp_i = 0;temp_i < TTL_BUS_RXBuffer[2];temp_i ++)
	{
		TTL_BUS_TXBuffer[temp_i] = TTL_BUS_RXBuffer[temp_i];
	}
	TTL_BUS_TXBuffer[0] = TTL_HEADER_ONE;TTL_BUS_TXBuffer[1] = TTL_HEADER_TWO;
	TTL_BUS_TXBuffer[8] = 0x03;//命令字
	
	for(temp_i = 2;temp_i < TTL_BUS_TXBuffer[2] - 1;temp_i ++)
	{
		sum_temp += TTL_BUS_TXBuffer[temp_i];
	}
	TTL_BUS_TXBuffer[TTL_BUS_TXBuffer[2] - 1] = sum_temp%256;
		
		
	for(temp_i = 0;temp_i < bus_send_length;temp_i ++)
	{
		if(Write_Ring_Data(TTL_BUS_TXBuffer[temp_i]) == 1)
		{
			break;
		}
	}

//	RingBuffer.ring_write_data_lock = 0;
	
	if(temp_i == bus_send_length)  return 1;
	
	return 0;
}

void TTL_Master_UartDataReturn(unsigned char data_temp)
{
	if(RingBuffer.ring_write_data_lock != 0) return;
	if(TTL_RingBuffer_CheckEnough(1) == 0) return;

	Write_Ring_Data(data_temp);	
}


unsigned char Get_Master_Ask_Mutex(void)
{
	return master_poll_ask_mutex;
}

void Set_Master_Ask_Mutex(unsigned char mutex_lock)
{
	if(1 == mutex_lock)
	{
		master_poll_ask_wait = 0;
	}
	master_poll_ask_mutex = mutex_lock;
	
}

void TTL_User_AskDevice_Status(unsigned char device_type,unsigned char func_code,unsigned char func_code_extend1,unsigned char func_code_extend2)
{
	unsigned char device_add = Master_SearchIdleAdd(device_type);
	
	if(device_add != 0)
	{
		Device_InfoArr[device_add][DEVICE_INFO_ARR_COMM] = 0x01;
		Device_InfoArr[device_add][DEVICE_INFO_ARR_CODE] = func_code;
		Device_InfoArr[device_add][DEVICE_INFO_ARR_EXTEND1] = func_code_extend1;
		Device_InfoArr[device_add][DEVICE_INFO_ARR_EXTEND2] = func_code_extend2;
		Device_InfoArr[device_add][DEVICE_INFO_ARR_POLLNUM] = 0x00;
	}
}

unsigned char TTL_IsDeviceOnline(unsigned char device_type)
{
	if(0 == device_type)
	{
		return 0;
	}

	if(TTL_DeviceOnlineCount[device_type] > 0)
	{
		return 1;
	}

	return 0;
}


