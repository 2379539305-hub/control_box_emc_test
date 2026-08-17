#ifndef __APP_MESH_H__
#define __APP_MESH_H__

#include "main.h"



extern unsigned char mesh_start_pair_flag;


void mesh_start_pair(void);
void mesh_stop_pair(void);
void mesh_uart_process(unsigned char data);
void mesh_control(void);
void Mesh_TimeManagerTask(void);

unsigned char Get_Mesh_Connect(void);
void Set_Mesh_Connect_Flag(unsigned char flag);
#endif


