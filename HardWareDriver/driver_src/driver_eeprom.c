#include "driver_eeprom.h"

/* ============================================  Define  ============================================ */
#define OPERATE_EEPROM_AREA_FLAG            0xFFFFFFFFUL                        /* operate space状态标志，0xffffffff代表operate space，0x00000000代表backup space */
#define BACKUP_EEPROM_AREA_FLAG             0x00000000UL
#define COPY_DATA_COMPLETE_FLAG             0x00000000UL                        /* backup space拷贝完成标志，0x00000000代表拷贝完成*/
#define EEPROM_SPACE_FLAG_OFFSET            4UL                                 /* the offset保存operate space状态标志 */
#define EEPROM_COPY_FLAG_OFFSET             (EEPROM_SPACE_FLAG_OFFSET + 4)      /* the offset保存backup space拷贝完成标志 */
#define EEPROM_END_ADDRESS_OFFSET           (EEPROM_COPY_FLAG_OFFSET + 4)       /* eeprom end address offset */
#define WRITE_DATA                           0
#define WRITE_FLAG                           1

/* ==========================================  Variables  =========================================== */
uint32_t s_eepromSize = FMEEPROM_SIZE;                                             /* EE中定义需要使用EE的空间（单位字节），默认512字节*/
uint32_t s_eepromPages = FMEEPROM_PAGE_NUM;                                               /* EE中分配的页数*/       
uint32_t s_eepromStartAddr = FLASH_END_ADDRESS - FLASH_PAGE_SIZE * FMEEPROM_PAGE_NUM;/* EE中分配的operate space起始页*/
uint32_t CurWrAddress = FLASH_END_ADDRESS - FLASH_PAGE_SIZE * FMEEPROM_PAGE_NUM;;
uint32_t BackupStartAddr = FLASH_END_ADDRESS - FLASH_PAGE_SIZE * FMEEPROM_PAGE_NUM * 2;
uint32_t OperateStartAddr = FLASH_END_ADDRESS - FLASH_PAGE_SIZE * FMEEPROM_PAGE_NUM; 

uint32_t EE_Address[USER_NUM_OF_DATA] = {0};
/* ======================================  Functions define  ======================================== */
/*!
 * @brief Flash擦函数
 *
 * @param[in] 固定擦2K空间
 * @return flash擦状态
 */
FLASH_StatusType FL_FLASH_FixSizeErase(uint32_t address)
{
    FLASH_StatusType statusRes = FLASH_STATUS_SUCCESS;
    __disable_irq();
    RCC->PCLKCR2 |= 0X20;                           //打开FLASH总线时钟
    RCC->OPCCR2 |= 0X00400000;                      //打开FLASH工作时钟
    FLASH->EPCR = 0x101;                             //sector擦并使能EREQ
    FLASH->KEY = 0x96969696U;  
    FLASH->KEY = 0x3C3C3C3CU;   
    M32(address) =  0x1234ABCDU;
    while(!(FLASH->ISR & 0x01));                      // 等待擦除完成
    FLASH->ISR |= 0x01;                               // 清除擦除完成标志
    FLASH->KEY = 0x00000000;                          // 恢复密钥保护  
    RCC->OPCCR2  &= ~0X00400000;                    // 关闭FLASH工作时钟
    RCC->PCLKCR2 &= ~0X20;                          // 关闭Flash总线时钟 
    __enable_irq(); 
    if(((FLASH->ISR) & 0xF00) != 0)
    {
        statusRes = FLASH_STATUS_CMD_INVALID;
    } 
    return statusRes; 
}


/*!
 * @brief Flash擦函数
 *
 * @param[in] 固定擦512空间
 * @return flash擦状态
 */
FLASH_StatusType FL_FLASH_DynamicPageErase(uint32_t address)
{
     FLASH_StatusType statusRes = FLASH_STATUS_SUCCESS;
    __disable_irq();
    RCC->PCLKCR2 |= 0X20;                           //打开FLASH总线时钟
    RCC->OPCCR2 |= 0X00400000;                      //打开FLASH工作时钟
    FLASH->EPCR = 0x1;                              //page擦并使能EREQ
    FLASH->KEY = 0x96969696U;  
    FLASH->KEY = 0xEAEAEAEAU;   
    M32(address) =  0x1234ABCDU;
    while(!(FLASH->ISR & 0x01));                      // 等待擦除完成
    FLASH->ISR |= 0x01;                               // 清除擦除完成标志
    FLASH->KEY = 0x00000000;                          // 恢复密钥保护  
    RCC->OPCCR2  &= ~0X00400000;                    // 关闭FLASH工作时钟
    RCC->PCLKCR2 &= ~0X20;                          // 关闭Flash总线时钟 
    __enable_irq(); 
    if(((FLASH->ISR) & 0xF00) != 0)
    {
        statusRes = FLASH_STATUS_CMD_INVALID;
    } 
    
    return statusRes; 
}


/*!
 * @brief Flash写函数
 *
 * @param[in] Address: flash写地址
 * @param[in] data: 写入数据
 * @return flash写入状态
 */
FLASH_StatusType FL_EE_Write_Word(uint32_t address, uint32_t data)
{
    FLASH_StatusType statusRes = FLASH_STATUS_SUCCESS;
    __disable_irq();
    RCC->PCLKCR2 |= 0X20;                           //打开FLASH总线时钟
    RCC->OPCCR2 |= 0X00400000;                      //打开FLASH工作时钟
    FLASH->ISR |= 0x2;                              // 清除编程完成标志        
    FLASH->EPCR |= 0x2;                             //启动编程
    FLASH->KEY = 0xA5A5A5A5;                        // 写编程密钥
    FLASH->KEY = 0xF1F1F1F1;                        // 写编程密钥  
    M32(address) = data;
    while(!(FLASH->ISR & 0x2));                     // 等待编程完成
    FLASH->ISR |= 0x2;                              // 清除编程完成标志      
    FLASH->KEY = 0x00000000;                        // 恢复密钥保护  
    RCC->OPCCR2  &= ~0X00400000;                    // 关闭FLASH工作时钟
    RCC->PCLKCR2 &= ~0X20;                          // 关闭Flash总线时钟 
    __enable_irq(); 
    if(((FLASH->ISR) & 0xF00) != 0)
    {
        statusRes = FLASH_STATUS_CMD_INVALID;
    } 
    return statusRes;    
}

/*!
 * @brief Flash写函数
 *
 * @param[in] Address: flash写地址
 * @param[in] data: 写入数据
 * @param[in] dataORflag: flash写入数据还是标志
 * @return flash写入状态
 */
FLASH_StatusType FL_EE_Program_Word(uint32_t address, uint32_t data,uint8_t dataORflag)
{
    FLASH_StatusType statusRes = FLASH_STATUS_SUCCESS;
    uint32_t BackupEndAddr = BackupStartAddr + s_eepromPages * FLASH_PAGE_SIZE - EEPROM_COPY_FLAG_OFFSET;
    uint32_t OperateEndAddr = OperateStartAddr + s_eepromPages * FLASH_PAGE_SIZE - EEPROM_COPY_FLAG_OFFSET;
   
    if(dataORflag == WRITE_DATA)
    {
        if(((BackupStartAddr <= address) && (address < BackupEndAddr)) || ((OperateStartAddr <= address) && (address < OperateEndAddr)))
        {
            statusRes = FL_EE_Write_Word(address,data);
            if(statusRes == FLASH_STATUS_CMD_INVALID)
            {
                return FLASH_STATUS_CMD_INVALID;
            }
        }
        else
        {
            statusRes = FLASH_STATUS_INVALID_DATAFLASH_ADDRESS;
        }
    
    }
    else if(dataORflag == WRITE_FLAG)
    {

        if(((BackupEndAddr <= address) && (address < BackupEndAddr + EEPROM_COPY_FLAG_OFFSET)) || ((OperateEndAddr <= address) && (address < OperateEndAddr + EEPROM_COPY_FLAG_OFFSET)))
        {
            statusRes = FL_EE_Write_Word(address,data);
            if(statusRes == FLASH_STATUS_CMD_INVALID)
            {
                return FLASH_STATUS_CMD_INVALID;
            }
        }       
        else
        {
            statusRes = FLASH_STATUS_INVALID_DATAFLASH_ADDRESS;
        }        
    }
    else
    {
        statusRes = FLASH_STATUS_CMD_INVALID;
    }


    return statusRes;
}


/*!
 * @brief Flash模拟EE初始化
 *
 * @param[in] size: EE需要开辟分配的空间大小
 * @param[in] pageIndex: 根据size大小，宏定义时自动分配operate space的第一个page。
 * @return none
 */
void FMEEPROM_Init(uint16_t size, uint32_t pageIndex)
{
    s_eepromSize = size;
    s_eepromStartAddr = FLASH_BASE_ADDRESS + pageIndex * FLASH_PAGE_SIZE;
    s_eepromPages = s_eepromSize / (FLASH_PAGE_SIZE);
    OperateStartAddr = s_eepromStartAddr;
    BackupStartAddr = s_eepromStartAddr - s_eepromPages * FLASH_PAGE_SIZE;
    CurWrAddress = s_eepromStartAddr;       
}



/*!
 * @brief 获取space中最新有效数据地址
 *
 * @param[in] endAddr: space中末地址
 * @param[in] validDataAddr: space中最新有效数据地址
 * @return 0: no valid data, 1: found valid data
 */
static uint8_t GetValidDataStartAddr(uint32_t endAddr, uint32_t *validDataAddr)
{
    uint32_t i = 0;
    uint8_t ret = 0;
    uint32_t ValidEndAddr;
    
    ValidEndAddr = endAddr;   
    for (i = 0; i < ((s_eepromPages * FLASH_PAGE_SIZE) >> 2) - 2; i++)
    {
        if (0xffffffff != *((uint32_t *)(ValidEndAddr - i * 4)))
        {
            ret = 1;
            break;
        }
    }

    *validDataAddr = ValidEndAddr - i * 4;

    return ret;
}



/*!
 * @brief 数据校验和计算
 *
 * @param[in] EE_Addr: EE的地址
 * @param[in] dataBuffer: EE的写入数据地址
 * @return 数据校验和
 */
uint8_t ChecksumUint8(uint16_t ee_Addr,uint8_t dataBuff)
{
	uint8_t Sum =0;
    Sum = (uint8_t)(ee_Addr + dataBuff);
    
	return Sum;
}



/*!
 * @brief 判断ID地址是否有效
 *
 * @param[in] Address: 实际DataFlash的地址，word地址的数据格式是（16bit的EE地址+8bit的checksum++8bitEE的数据）
 * @return statusRes: 读数据状态
 */
static FLASH_StatusType IsDataValid(uint32_t address)
{
    FLASH_StatusType statusRes = FLASH_STATUS_SUCCESS; 
    uint16_t EE_Addr;
    uint8_t SumTmp;
 	uint8_t  DataTmp;  

    /* 获取EE地址*/
	EE_Addr = (M32(address)>>16) & 0xFFFF;
    
    if(EE_Addr <= USER_NUM_OF_DATA)
    {
        /* 获取ID索引长度*/        
        DataTmp = M8(address);
        SumTmp = (M16(address) >> 8) & 0xFF;
         /* 获取EE地址校验和与软件计算的校验和对比*/ 
        if(SumTmp != ChecksumUint8(EE_Addr,DataTmp))
        {
            statusRes = FLASH_STATUS_DATACHECK_ERR;  
        }
    }
    else
    {
        statusRes = FLASH_STATUS_INVALID_EE_ADDRESS;
    
    }

    return statusRes;
}


/*!
 * @brief ID数据帧格式填充
 *
 * @param[in] DataBuff: 写入数据地址
 * @param[in] DataLen: 数据帧长度
 * @param[in] WriteBuff: 数据填充后的待写入的数据地址
 * @return None
 */
void FillWriteBuff(uint16_t ee_Addr,uint8_t Data,uint32_t *DataComb)
{
	uint8_t Checksum;  
    /* 计算数据的校验码*/
	  Checksum = ChecksumUint8(ee_Addr,Data);
    /* 数据组帧*/
    *DataComb = (ee_Addr<<16) | (Checksum<<8) | Data;

}



/*!
 * @brief ID数据帧格式临时填充
 *
 * @param[in] addr: backup space中ID地址
 * @param[in] DataLen: 数据帧长度（半字）
 * @param[in] WriteBuff: 写入数据地址
 * @return None
 */
void FillWriteBuff_Temp(uint32_t addr,uint16_t dataLen,uint16_t *writeBuff)
{
	uint16_t DataIndex;
 	
	for(DataIndex = 0; DataIndex < dataLen - 1; DataIndex++)
	{
        writeBuff[DataIndex] = M16(addr + 4 * DataIndex);	
	}

}


/*!
 * @brief backup space有效数据拷贝到operate space
 *
 * @param[in] BackupStartAddr: backup space基地址
 * @return statusRes: 拷贝数据状态
 */
static FLASH_StatusType CopyDataToNewArea(uint32_t backupStartAddr)
{
    FLASH_StatusType statusRes = FLASH_STATUS_SUCCESS;
    uint16_t EE_Addr;
    uint32_t Address;
	uint32_t DataTmp; 
    
    for(EE_Addr = 0; EE_Addr < USER_NUM_OF_DATA; EE_Addr++)
    {
        Address = EE_Address[EE_Addr];  
        if((Address >= backupStartAddr) && (Address < backupStartAddr + s_eepromPages * FLASH_PAGE_SIZE - EEPROM_COPY_FLAG_OFFSET))
        {                    
            if(FLASH_STATUS_SUCCESS == IsDataValid(Address))
            {    
                DataTmp = M32(Address);                                     
                 /* 数据写flash*/                     
                statusRes = FL_EE_Program_Word(CurWrAddress, DataTmp,WRITE_DATA);
                
                if( FLASH_STATUS_SUCCESS != statusRes)
                {
                    return statusRes;
                }   
                
                EE_Address[EE_Addr] = CurWrAddress;              
                CurWrAddress = CurWrAddress + 4;
                
            }
        } 
    }   


    return statusRes;
}

/*!
 * @brief 软件模拟EE写函数
 *
 * @param[in] ee_Addr: EE中地址索引
 * @param[in] dataBuffer: 写入数据地址
 * @param[in] writeNum: 写入数据长度
 * @return statusRes: 软件模拟EE写操作的状态
 */
FLASH_StatusType FMEEPROM_WriteExt(uint16_t ee_Addr, const uint8_t *dataBuffer, uint16_t writeNum)
{
    FLASH_StatusType statusRes = FLASH_STATUS_SUCCESS;
    uint32_t leftSize = 0;
    uint32_t tmpWriteStartAddr = 0;
    uint16_t i = 0;
    uint8_t DataTmp;
    uint32_t DataComb;
    uint16_t EE_Addr_Tmp;
   /* 计算operate space中剩余数据空间*/
    leftSize = (OperateStartAddr + s_eepromPages * FLASH_PAGE_SIZE - EEPROM_COPY_FLAG_OFFSET - CurWrAddress) >> 2;
   
    /* 判断剩余空间是否够用 */
    if (leftSize >= writeNum)
    {  /* 有足够的剩余空间 */
        
        for(i = 0;i < writeNum;i++)
        {    
            EE_Addr_Tmp = ee_Addr + i;
            DataTmp = *(dataBuffer + i);
            /* 发送数据填充*/
            FillWriteBuff(EE_Addr_Tmp,DataTmp,&DataComb);       
             /* 数据写flash*/
            statusRes = FL_EE_Program_Word(CurWrAddress, DataComb, WRITE_DATA);
            
            if( FLASH_STATUS_SUCCESS != statusRes)
            {
                return statusRes;
            }            
            EE_Address[EE_Addr_Tmp] = CurWrAddress;        
            CurWrAddress = CurWrAddress + 4;
        }
        
    }
    else
    {
        
        #if(EE_SIZE_FIXED)
        {
            for(i = 0;i < SECTOR_NUM;i++)
            {
                if(FLASH_STATUS_SUCCESS != FL_FLASH_FixSizeErase(BackupStartAddr + i * FLASH_SECTOR_SIZE))
                {
                    statusRes = FLASH_STATUS_CMD_INVALID;              
                }
                if (FLASH_STATUS_SUCCESS != statusRes)
                {
                    return statusRes;
                }               
            }
        }
        #else
        {
            for (i = 0; i < s_eepromPages; i++)
            {
                /* 擦除backup space*/
                if (FLASH_STATUS_SUCCESS != FL_FLASH_DynamicPageErase(BackupStartAddr + FLASH_PAGE_SIZE * i))
                {
                    statusRes = FLASH_STATUS_CMD_INVALID;
                }
                
                if (FLASH_STATUS_SUCCESS != statusRes)
                {
                    return statusRes;
                }
            }
        }
        #endif

        /* backup space标志写入operate space最后一个word地址 */    
        if(FLASH_STATUS_SUCCESS != FL_EE_Program_Word(OperateStartAddr + s_eepromPages * FLASH_PAGE_SIZE - EEPROM_SPACE_FLAG_OFFSET, BACKUP_EEPROM_AREA_FLAG, WRITE_FLAG))
        {
            statusRes = FLASH_STATUS_CMD_INVALID;
            return statusRes;
        }
        
        /* 新数据写入backup space */
        /* 发送数据填充*/
        for(i = 0;i < writeNum;i++)
        {    
            EE_Addr_Tmp = ee_Addr + i;
            DataTmp = *(dataBuffer + i);
            /* 发送数据填充*/
            FillWriteBuff(EE_Addr_Tmp,DataTmp,&DataComb);       
             /* 数据写flash*/
            if(i == 0)
            {
                statusRes = FL_EE_Program_Word(BackupStartAddr, DataComb, WRITE_DATA);               
                if( FLASH_STATUS_SUCCESS != statusRes)
                {
                    return statusRes;
                }            
                EE_Address[EE_Addr_Tmp] = BackupStartAddr;        
                CurWrAddress = BackupStartAddr + 4;
            }
            else
            {
                statusRes = FL_EE_Program_Word(CurWrAddress, DataComb, WRITE_DATA);               
                if( FLASH_STATUS_SUCCESS != statusRes)
                {
                    return statusRes;
                }            
                EE_Address[EE_Addr_Tmp] = CurWrAddress;        
                CurWrAddress = CurWrAddress + 4;                              
            }
        }
                    
        /* 将历史有效的数据写入backup space中 */
        if (FLASH_STATUS_SUCCESS == statusRes)
        {
            statusRes = CopyDataToNewArea(OperateStartAddr);                         
            if (FLASH_STATUS_SUCCESS == statusRes)
            {
                /* 拷贝完成标志写入operate space倒数第二个word地址 */             
                if(FLASH_STATUS_SUCCESS != FL_EE_Program_Word(OperateStartAddr + s_eepromPages * FLASH_PAGE_SIZE - EEPROM_COPY_FLAG_OFFSET, COPY_DATA_COMPLETE_FLAG, WRITE_FLAG))
                {
                    statusRes = FLASH_STATUS_CMD_INVALID;
                }

            }
        }
       tmpWriteStartAddr =  OperateStartAddr;
       OperateStartAddr = BackupStartAddr;
       BackupStartAddr = tmpWriteStartAddr; 
       /* 至此backup space 和 operate space空间意义互换*/
    }

    return statusRes;
}

/*!
 * @brief 更新EE_Address地址
 *
 * @param[in] Addr:space基地址
 * @param[in] checkStartAddr: space可用空间末地址
 * @return None
 */
void UpdataEEAddr(uint32_t addr,uint32_t checkStartAddr,uint32_t* areaWriteAddr)
{
    uint16_t EE_Addr;
    uint32_t Address;
    Address = addr;
    
    /* 搜索space内非FFFFFFFF的空间地址——oldAreaWriteAddr*/
    if (GetValidDataStartAddr(checkStartAddr, areaWriteAddr))
    {
        while(Address <= *areaWriteAddr)
        {
            EE_Addr = (M32(Address)>>16) & 0xFFFF;
            if((EE_Addr < USER_NUM_OF_DATA))
            { 
                 /* 搜索有效的数据帧格式*/
                if(FLASH_STATUS_SUCCESS == IsDataValid(Address))
                {
                    EE_Address[EE_Addr] = Address; 
                }
            }                
            Address = Address + 4;      
        }
    }  

}


/*!
 * @brief 检查backup space数据拷贝到operate space
 * @return statusRes: 软件模拟EE的状态
 */
FLASH_StatusType CheckUpdate(void)
{
    FLASH_StatusType statusRes = FLASH_STATUS_SUCCESS;
    
    /* AreaEndAddr为backup space空间的末地址 */    
    uint32_t AreaEndAddr = 0;    
    uint32_t oldAreaWriteAddr = 0,newAreaWriteAddr = 0;

    AreaEndAddr = s_eepromStartAddr + s_eepromPages * FLASH_PAGE_SIZE;
    /* 检测operate space 和 backup space位置 */
    if (BACKUP_EEPROM_AREA_FLAG == M32(AreaEndAddr - EEPROM_SPACE_FLAG_OFFSET))
    {
        if(OPERATE_EEPROM_AREA_FLAG == M32(s_eepromStartAddr - EEPROM_SPACE_FLAG_OFFSET))
        {
            /* 当前operate space为分配总空间的前半部分 */
            OperateStartAddr = s_eepromStartAddr - s_eepromPages * FLASH_PAGE_SIZE;
            BackupStartAddr = s_eepromStartAddr;
        }
        else
        {
            statusRes = FLASH_STATUS_SPACE_DEFINE_ERR;
            return statusRes;        
        }
    }
    else if((OPERATE_EEPROM_AREA_FLAG == M32(AreaEndAddr - EEPROM_SPACE_FLAG_OFFSET)))
    {
        if((BACKUP_EEPROM_AREA_FLAG == M32(s_eepromStartAddr - EEPROM_SPACE_FLAG_OFFSET)))
        {
            /* 当前operate space为分配总空间的后半部分 */
            OperateStartAddr = s_eepromStartAddr;
            BackupStartAddr = s_eepromStartAddr - s_eepromPages * FLASH_PAGE_SIZE;
            AreaEndAddr = s_eepromStartAddr;
            
        }else if((OPERATE_EEPROM_AREA_FLAG == M32(s_eepromStartAddr - EEPROM_SPACE_FLAG_OFFSET)))
        {
            /* 当前operate space为分配总空间的后半部分 */
            OperateStartAddr = s_eepromStartAddr;
            BackupStartAddr = s_eepromStartAddr - s_eepromPages * FLASH_PAGE_SIZE;
            AreaEndAddr = s_eepromStartAddr;
             
            /* backup space标志写入最后一个word地址 */    
            if(FLASH_STATUS_SUCCESS == FL_EE_Program_Word(AreaEndAddr - EEPROM_SPACE_FLAG_OFFSET, BACKUP_EEPROM_AREA_FLAG, WRITE_FLAG))
            {
                statusRes = FLASH_STATUS_SUCCESS;
            }
            else
            {
                statusRes = FLASH_STATUS_CMD_INVALID;
                return statusRes;             
            }
        }
        else
        {
            statusRes = FLASH_STATUS_SPACE_DEFINE_ERR;
            return statusRes;  
        }
    }
    else
    {
         statusRes = FLASH_STATUS_SPACE_DEFINE_ERR;
         return statusRes;        
    }  
    
    /* 检查backup space数据拷贝是否成功，避免因复位导致backup space中有效数据未拷贝*/
    /* 更新EE_Address关于在backup space的ID地址*/
    if ((~COPY_DATA_COMPLETE_FLAG) == M32(AreaEndAddr - EEPROM_COPY_FLAG_OFFSET))
    {
        /* backup space中updata ee的地址*/         
        UpdataEEAddr(BackupStartAddr,AreaEndAddr - EEPROM_END_ADDRESS_OFFSET,&oldAreaWriteAddr);        
    } 
      
    /* operation space中updata EE的地址*/  
    UpdataEEAddr(OperateStartAddr,OperateStartAddr + s_eepromPages * FLASH_PAGE_SIZE - EEPROM_END_ADDRESS_OFFSET,&newAreaWriteAddr);

    /* 更新当operation space 可写入的地址*/      
    CurWrAddress = newAreaWriteAddr + 4;
     
    /* 检查backup space数据拷贝是否成功，避免因复位导致backup space中有效数据未拷贝*/
    if ((~COPY_DATA_COMPLETE_FLAG) == *(uint32_t *)(AreaEndAddr - EEPROM_COPY_FLAG_OFFSET))
    {
        /* 数据拷贝*/ 
        statusRes = CopyDataToNewArea(BackupStartAddr);     
       
        if (FLASH_STATUS_SUCCESS == statusRes)
        {            
            /* 拷贝完成标志写入backup space倒数第二个word地址 */
            if(FLASH_STATUS_SUCCESS != FL_EE_Program_Word(AreaEndAddr - EEPROM_COPY_FLAG_OFFSET, COPY_DATA_COMPLETE_FLAG, WRITE_FLAG))
            {
                statusRes = FLASH_STATUS_CMD_INVALID;
            }
        }
    }

    return statusRes;
}

/*!
 * @brief 软件模拟EE写操作
 *
 * @param[in] EE_Addr: EE中地址
 * @param[in] dataBuffer: 写入数据地址
 * @param[in] writeNum: 写入数据长度（字节）
 * @return statusRes: 软件模拟EE写操作的状态
 */
FLASH_StatusType FMEEPROM_Write(uint16_t ee_Addr, const uint8_t *dataBuffer, uint16_t writeNum)
{
    FLASH_StatusType statusRes = FLASH_STATUS_SUCCESS;

    /* 检查EE地址以及要写入数据地址的有效性 */
    if(ee_Addr >= USER_NUM_OF_DATA)
    {
        statusRes  = FLASH_STATUS_INVALID_EE_ADDRESS; 
    }
    else if(dataBuffer == NULL)
    {
       statusRes = FLASH_STATUS_PDATA_INVALID;  
    }
    else if(writeNum > USER_NUM_OF_DATA)
    {
       statusRes = FLASH_STATUS_IDLEN_ERR;        
    }       
    else
    {
        statusRes = FMEEPROM_WriteExt(ee_Addr, dataBuffer, writeNum);
    }

    return statusRes;
}




/*!
 * @brief 软件模拟EE读操作
 *
 * @param[in] readID: EE中地址索引
 * @param[in] dataBuffer: 读出数据地址
 * @return 软件模拟EE读操作的状态
 */
FLASH_StatusType FMEEPROM_Read(uint16_t ee_addr, uint8_t *dataBuffer,uint16_t datalen)
{
    FLASH_StatusType statusRes = FLASH_STATUS_SUCCESS;   
    uint32_t Address = 0;
    uint16_t i;
    
    /* 判断数组地址或者指针有效性*/
    if(dataBuffer == NULL)
    {
        statusRes = FLASH_STATUS_PDATA_INVALID;  
    }
    else
    {
        for(i = 0;i < datalen;i++)
        {
            /* 判断读EE地址有效性*/    
            if((ee_addr + i)  >= USER_NUM_OF_DATA)
            {
                *(dataBuffer + i) = 0xFF;
                statusRes = FLASH_STATUS_INVALID_EE_ADDRESS;
            }
            else
            {
                Address= EE_Address[ee_addr + i];  
                if((OperateStartAddr <= Address) && (Address <= (OperateStartAddr + s_eepromPages * FLASH_PAGE_SIZE - EEPROM_END_ADDRESS_OFFSET)))
                {
                    /* EE地址的数据校验有效*/ 
                    if(FLASH_STATUS_SUCCESS == IsDataValid(Address))
                    {                                
                        *(dataBuffer + i) = M8(Address);
                    }
                    else
                    {
                        *(dataBuffer + i) = 0xFF;
                        statusRes = FLASH_STATUS_DATACHECK_ERR;                    
                    }
                }
                else
                {
                    *(dataBuffer + i) = 0xFF;
                    statusRes = FLASH_STATUS_INVALID_DATAFLASH_ADDRESS; 
                }
            
            
            }
        }
    }
    
    return statusRes;
}

/*!
 * @brief 软件模拟EE空间擦除操作（包括backup space）
 *
 * @param[in] none
 * @return statusRes: 软件模拟EE擦操作的状态
 */
FLASH_StatusType FMEEPROM_Erase(void)
{
    FLASH_StatusType statusRes = FLASH_STATUS_SUCCESS;
    uint8_t i = 0;
    uint32_t EraseBaseAddress = FLASH_END_ADDRESS - FLASH_PAGE_SIZE *  s_eepromPages * 2;
    for (i = 0; i < s_eepromPages * 2; i++)
    {
        /* 擦除operate space和backup space*/
        if (FLASH_STATUS_SUCCESS != FL_FLASH_DynamicPageErase( EraseBaseAddress + i * FLASH_PAGE_SIZE))
        {
            statusRes = FLASH_STATUS_CMD_INVALID;
        }
        if (FLASH_STATUS_SUCCESS != statusRes)
        {
            break;
        }
    }

    return statusRes;
}


/*!
 * @brief 软件模拟EE空间初始化
 *
 * @param[in] none
 * @return none
 */
FLASH_StatusType FMEEPROM_SoftInit(void)
{
    FLASH_StatusType statusRes = FLASH_STATUS_SUCCESS;    

    /* 当固定长度时，判断需要分配的长度是否超限 */
    #if(EE_SIZE_FIXED)
    {
        if(USER_NUM_OF_DATA > 510)
        {
            statusRes = FLASH_STATUS_IDLEN_ERR;
            return statusRes;
        }
    }
    #endif   
    
    FMEEPROM_Init(FMEEPROM_SIZE, FMEEPROM_START_PAGE);
    
    if(CheckUpdate() == FLASH_STATUS_SUCCESS)
    {                
        statusRes = FLASH_STATUS_SUCCESS;
        return statusRes;          
    }
           
    FMEEPROM_Erase(); 
    OperateStartAddr = s_eepromStartAddr;
    BackupStartAddr = s_eepromStartAddr - s_eepromPages * FLASH_PAGE_SIZE;
    CurWrAddress = s_eepromStartAddr;

    /* backup space标志写入最后一个word地址 */    
    if(FLASH_STATUS_SUCCESS != FL_EE_Program_Word(BackupStartAddr + s_eepromPages * FLASH_PAGE_SIZE - EEPROM_SPACE_FLAG_OFFSET, BACKUP_EEPROM_AREA_FLAG, WRITE_FLAG))
    {
        statusRes = FLASH_STATUS_CMD_INVALID;
        return statusRes;
    } 
    /* 拷贝完成标志写裙数第二个word地址 */  
    if(FLASH_STATUS_SUCCESS != FL_EE_Program_Word(BackupStartAddr + s_eepromPages * FLASH_PAGE_SIZE - EEPROM_COPY_FLAG_OFFSET, COPY_DATA_COMPLETE_FLAG, WRITE_FLAG))
    {
        statusRes = FLASH_STATUS_CMD_INVALID;
        return statusRes;
    } 
      
    return statusRes;

}

/* =============================================  EOF  ============================================== */





void IAP_Enable(void)
{

}
void IAP_Disable(void)
{

}
void IAP_TrigProgram(void)
{

}
//擦除扇区, 入口:DPTR = 扇区地址
void Sector_Erase(uint32_t eeprom_addr)
{
     FL_FLASH_PageErase(FLASH,eeprom_addr);
}

/*********************************************************************************************/
//字节编程，调用前需打开IAP 功能，入口:DPTR = 字节地址, A= 须编程字节的数据
void Memory_WriteByte(uint32_t eeprom_addr, uint8_t dat_temp)
{
       
}
void Memory_Write(uint32_t eeprom_addr ,uint8_t *data_buf ,uint16_t num)
{
	    
	  uint32_t eeprom_data;
	  uint32_t Address;
	  uint16_t  i,len;
	  (num%4)?(len=num/4+1):(len=num/4);
	  Address = eeprom_addr;
	
	  for( i = 0; i < len; i++)
    {
		    eeprom_data = (data_buf[3+(4*i)]<<24)|(data_buf[2+(4*i)]<<16)|(data_buf[1+(4*i)]<<8)|(data_buf[(4*i)+0]<<0);
		    FL_FLASH_Program_Word(FLASH,Address,eeprom_data);
			  eeprom_data = 0;
			  Address = Address + 4;  
		}
		
}

//读4字节
uint32_t flash_read_word(uint32_t addr)
{
		uint32_t data = 0;
		if(addr%4 != 0) return 0xffffffff;
		data = *((uint32_t*)addr);
		return data;
}


void Memory_Read(uint32_t eeprom_addr ,uint8_t *data_buf , uint16_t num)
{

     uint32_t  eeprom_data;
		 uint32_t Address;
	   uint16_t  i,len;	
	   Address = eeprom_addr;
	   (num%4)?(len=num/4+1):(len=num/4);	
	   for(i = 0; i < len; i++)
	   {
         eeprom_data = flash_read_word(Address);
			   if(i==(num/4))
				 {
					 switch(num%4)
					 {
						 case 0:
						 { 
								data_buf[3+(4*i)] = (eeprom_data>>24)&0xff;
								data_buf[2+(4*i)] = (eeprom_data>>16)&0xff;
								data_buf[1+(4*i)] = (eeprom_data>>8)&0xff;
								data_buf[0+(4*i)] = (eeprom_data>>0)&0xff;						      
						 }break;							 
						 case 1:
						 { 
								data_buf[0+(4*i)] = (eeprom_data>>0)&0xff;						      
						 }break;
						 case 2:
						 { 
								data_buf[1+(4*i)] = (eeprom_data>>8)&0xff;
								data_buf[0+(4*i)] = (eeprom_data>>0)&0xff;							      
						 }break;
						 case 3:
						 { 
								data_buf[2+(4*i)] = (eeprom_data>>16)&0xff;
								data_buf[1+(4*i)] = (eeprom_data>>8)&0xff;
								data_buf[0+(4*i)] = (eeprom_data>>0)&0xff;							      
						 }break;							 
						 default : break ;
					 }					 
				 }
				 else
				 {
				    data_buf[3+(4*i)] = (eeprom_data>>24)&0xff;
				    data_buf[2+(4*i)] = (eeprom_data>>16)&0xff;
					  data_buf[1+(4*i)] = (eeprom_data>>8)&0xff;
					  data_buf[0+(4*i)] = (eeprom_data>>0)&0xff;
				 }
			   eeprom_data = 0;
			   Address = Address + 4; 				 
				 
		 }
}

