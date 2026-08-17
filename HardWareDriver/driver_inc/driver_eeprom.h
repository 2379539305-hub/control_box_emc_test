#ifndef __DRIVER_EEPROM_H
#define __DRIVER_EEPROM_H


#include "system.h"
#include "driver_config.h"
#include "stdint.h"
#include "stdio.h"

/* ===========================================  Define according to requirements=========================================== */
#define FLASH_BASE_ADDRESS                 (0x1E000UL)                                       /*芯片整个FLASH的基地址*/
#define FLASH_SIZE                         (0x00002000UL)                                       /*芯片整个FLASH空间大小，默认配合8K*/
#define FLASH_PAGE_SIZE                    (0x00000200UL)                                       /*页大小*/
#define FLASH_SECTOR_SIZE                  (0x00000800UL)                                       /*块大小  2K */
    
#define USER_NUM_OF_DATA                   (220)                                                 /*定义用户最大的存储字节数*/    

#define EE_SIZE_FIXED                      (1)                                                   /*固定长度为最大存储510字节数据*/ 
                                                                                                 /*数据格式：用一个word地址只存一个字节的数据，因此一个Sector只能存2048/4-512个word，然后减去空间标志的最后两个word，可用空间为510*/

/* ===========================================  Automatic Define =========================================================== */     
#define FLASH_END_ADDRESS                  (FLASH_BASE_ADDRESS+FLASH_SIZE)                      /*芯片整个FLASH的末地址*/
#define FLASH_PAGE_TOTAL                   (FLASH_SIZE/FLASH_PAGE_SIZE)                         /*芯片整个FLASH的页总数 */ 

#if(EE_SIZE_FIXED)                                                                              /* 使用一个sector作为基础单元，目的为了擦除的时候可以sector擦，减少擦除时间*/ 
#define FMEEPROM_SIZE                      2048                                                 /* 实际FLASH所需要的空间*///(可选择2048 or 4096)                                                                                                                                              
#define SECTOR_NUM                         (FMEEPROM_SIZE/2048)
                                                                                                 
#else
#define FMEEPROM_SIZE                      ((((USER_NUM_OF_DATA + 2) * 4) % 512) ? ((((USER_NUM_OF_DATA + 2) * 4) /512 + 1) * 512 ) : ((((USER_NUM_OF_DATA + 2) * 4)/512) * 512 )) 
#endif
#define FMEEPROM_PAGE_NUM                  (FMEEPROM_SIZE/512) 
#define FMEEPROM_START_PAGE                (FLASH_PAGE_TOTAL - (FMEEPROM_SIZE)/FLASH_PAGE_SIZE) /* 根据开辟EE空间，自动定位默认operate space的首页*/

#define M32(addr) (*((uint32_t*)(addr)))
#define M16(addr) (*((uint16_t*)(addr)))
#define M8(addr) (*((uint8_t*)(addr)))


typedef enum
{
    FLASH_STATUS_SUCCESS =1,                    /*正常状态 */
    FLASH_STATUS_CMD_INVALID,                   /*无效的命令操作*/
    FLASH_STATUS_PDATA_INVALID,                 /*无效的数据地址 */
    FLASH_STATUS_IDLEN_ERR,                     /*长度超限 */     
    FLASH_STATUS_SPACE_DEFINE_ERR,              /*空间定义错误*/   
    FLASH_STATUS_DATACHECK_ERR,                 /*数据校验错误*/ 
    FLASH_STATUS_INVALID_DATAFLASH_ADDRESS,     /*无效DataFlash地址访问*/
    FLASH_STATUS_INVALID_EE_ADDRESS             /*无效EE地址访问*/   
} FLASH_StatusType;                   

/* ====================================  Functions declaration  ===================================== */

void FMEEPROM_Init(uint16_t size, uint32_t pageIndex);
FLASH_StatusType FMEEPROM_Write(uint16_t writeID, const uint8_t *dataBuffer, uint16_t writeNum);
FLASH_StatusType FMEEPROM_Read(uint16_t ee_addr, uint8_t *dataBuffer,uint16_t datalen);
FLASH_StatusType FMEEPROM_Erase(void);
FLASH_StatusType FMEEPROM_SoftInit(void);







void Sector_Erase(uint32_t eeprom_addr);
uint8_t Memory_ReadByte(uint32_t eeprom_addr);
void Memory_WriteByte(uint32_t eeprom_addr, uint8_t data_temp);

uint32_t flash_read_word(uint32_t addr);
void Memory_Write(uint32_t eeprom_addr ,uint8_t *buf ,uint16_t num);
void Memory_Read(uint32_t eeprom_addr ,uint8_t *buf , uint16_t num);


#endif






