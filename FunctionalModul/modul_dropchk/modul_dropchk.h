#ifndef __MODUL_DROPCHK_H
#define __MODUL_DROPCHK_H

#include "main.h"
#include "system.h"

typedef struct
{
	unsigned char RGB_Light_Online;
	unsigned char Music_Msg_Online;
	unsigned char Sleep_Sensor_Online;
	
}DROP_TTL_STRUCT;

extern DROP_TTL_STRUCT Drop_Ttl_Stu;

unsigned char DropCheck_Run(void);
void TTL_Check_OnLine(void);


#endif






