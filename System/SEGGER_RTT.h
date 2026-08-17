/*********************************************************************
*                    SEGGER Microcontroller GmbH                     *
*                        The Embedded Experts                        *
**********************************************************************
*                                                                    *
*            (c) 1995 - 2019 SEGGER Microcontroller GmbH             *
*                                                                    *
*       www.segger.com     Support: support@segger.com               *
*                                                                    *
**********************************************************************
*                                                                    *
*       SEGGER RTT * Real Time Transfer for embedded targets         *
*                                                                    *
**********************************************************************
*/

#ifndef SEGGER_RTT_H
#define SEGGER_RTT_H

#include "SEGGER_RTT_Conf.h"

#ifdef __cplusplus
  extern "C" {
#endif

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/

#define SEGGER_RTT_MODE_NO_BLOCK_SKIP         (0)
#define SEGGER_RTT_MODE_NO_BLOCK_TRIM         (1)
#define SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL    (2)
#define SEGGER_RTT_MODE_MASK                  (3)

#define RTT_CTRL_RESET                "\x1B[0m"
#define RTT_CTRL_CLEAR                "\x1B[2J"

#define RTT_CTRL_TEXT_BLACK           "\x1B[2;30m"
#define RTT_CTRL_TEXT_RED             "\x1B[2;31m"
#define RTT_CTRL_TEXT_GREEN           "\x1B[2;32m"
#define RTT_CTRL_TEXT_YELLOW          "\x1B[2;33m"
#define RTT_CTRL_TEXT_BLUE            "\x1B[2;34m"
#define RTT_CTRL_TEXT_MAGENTA         "\x1B[2;35m"
#define RTT_CTRL_TEXT_CYAN            "\x1B[2;36m"
#define RTT_CTRL_TEXT_WHITE           "\x1B[2;37m"

#define RTT_CTRL_TEXT_BRIGHT_BLACK    "\x1B[1;30m"
#define RTT_CTRL_TEXT_BRIGHT_RED      "\x1B[1;31m"
#define RTT_CTRL_TEXT_BRIGHT_GREEN    "\x1B[1;32m"
#define RTT_CTRL_TEXT_BRIGHT_YELLOW   "\x1B[1;33m"
#define RTT_CTRL_TEXT_BRIGHT_BLUE     "\x1B[1;34m"
#define RTT_CTRL_TEXT_BRIGHT_MAGENTA  "\x1B[1;35m"
#define RTT_CTRL_TEXT_BRIGHT_CYAN     "\x1B[1;36m"
#define RTT_CTRL_TEXT_BRIGHT_WHITE    "\x1B[1;37m"

#define RTT_CTRL_BG_BLACK             "\x1B[24;40m"
#define RTT_CTRL_BG_RED               "\x1B[24;41m"
#define RTT_CTRL_BG_GREEN             "\x1B[24;42m"
#define RTT_CTRL_BG_YELLOW            "\x1B[24;43m"
#define RTT_CTRL_BG_BLUE              "\x1B[24;44m"
#define RTT_CTRL_BG_MAGENTA           "\x1B[24;45m"
#define RTT_CTRL_BG_CYAN              "\x1B[24;46m"
#define RTT_CTRL_BG_WHITE             "\x1B[24;47m"

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

typedef struct {
  const char*    sName;         // Optional name. Must be != NULL to be considered valid
  char*          pBuffer;       // Pointer to start of buffer
  unsigned       SizeOfBuffer;  // Buffer size in bytes. Note that one byte is lost, as this implementation does not fill up the buffer in order to avoid the problem of being unable to distinguish between full and empty.
  unsigned       WrOff;         // Position of next item to be written by either host.
  volatile unsigned RdOff;      // Position of next item to be read by target.
  unsigned       Flags;         // Contains configuration flags
} SEGGER_RTT_BUFFER_UP;

typedef struct {
  const char*    sName;         // Optional name. Must be != NULL to be considered valid
  char*          pBuffer;       // Pointer to start of buffer
  unsigned       SizeOfBuffer;  // Buffer size in bytes. Note that one byte is lost, as this implementation does not fill up the buffer in order to avoid the problem of being unable to distinguish between full and empty.
  volatile unsigned WrOff;      // Position of next item to be written by host.
  unsigned       RdOff;         // Position of next item to be read by target (down-buffer).
  unsigned       Flags;         // Contains configuration flags
} SEGGER_RTT_BUFFER_DOWN;

typedef struct {
  char                    acID[16];                                 // Initialized to "SEGGER RTT"
  int                     MaxNumUpBuffers;                          // Initialized to SEGGER_RTT_MAX_NUM_UP_BUFFERS (type. 2)
  int                     MaxNumDownBuffers;                        // Initialized to SEGGER_RTT_MAX_NUM_DOWN_BUFFERS (type. 2)
  SEGGER_RTT_BUFFER_UP    aUp[SEGGER_RTT_MAX_NUM_UP_BUFFERS];       // Up buffers, transferring information up from target via debug probe to host
  SEGGER_RTT_BUFFER_DOWN  aDown[SEGGER_RTT_MAX_NUM_DOWN_BUFFERS];   // Down buffers, transferring information down from host via debug probe to target
} SEGGER_RTT_CB;

extern SEGGER_RTT_CB _SEGGER_RTT;

/*********************************************************************
*
*       RTT API functions
*
**********************************************************************
*/

void     SEGGER_RTT_Init                (void);
unsigned SEGGER_RTT_Read                (unsigned BufferIndex, void* pData, unsigned BufferSize);
unsigned SEGGER_RTT_Write               (unsigned BufferIndex, const void* pBuffer, unsigned NumBytes);
unsigned SEGGER_RTT_WriteString         (unsigned BufferIndex, const char* s);
unsigned SEGGER_RTT_PutChar             (unsigned BufferIndex, char c);
int      SEGGER_RTT_GetKey              (void);
int      SEGGER_RTT_WaitKey             (void);
int      SEGGER_RTT_HasKey              (void);
int      SEGGER_RTT_ConfigUpBuffer      (unsigned BufferIndex, const char* sName, void* pBuffer, unsigned BufferSize, unsigned Flags);
int      SEGGER_RTT_ConfigDownBuffer    (unsigned BufferIndex, const char* sName, void* pBuffer, unsigned BufferSize, unsigned Flags);
int      SEGGER_RTT_SetNameUpBuffer     (unsigned BufferIndex, const char* sName);
int      SEGGER_RTT_SetNameDownBuffer   (unsigned BufferIndex, const char* sName);
int      SEGGER_RTT_SetFlagsUpBuffer    (unsigned BufferIndex, unsigned Flags);
int      SEGGER_RTT_SetFlagsDownBuffer  (unsigned BufferIndex, unsigned Flags);
int      SEGGER_RTT_printf              (unsigned BufferIndex, const char * sFormat, ...);

#ifdef __cplusplus
  }
#endif

#endif
