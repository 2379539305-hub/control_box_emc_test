#ifndef __DRIVER_A7105_H
#define __DRIVER_A7105_H

#include "system.h"
#include "driver_config.h"


#define A7105_NEW_ID  0
#define A7105_OLD_ID  1


void A7105_Config(uint8_t a7105_config_flag);
void A7105_SetCH(uint8_t ch_temp);
void A7105_WriteID(uint8_t *id_buf_temp);
void A7105_RecvDataToBuf(uint8_t *databuf);
uint8_t A7105_Check_Online(void);

#endif






