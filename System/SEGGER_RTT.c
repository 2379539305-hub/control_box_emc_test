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

#include "SEGGER_RTT.h"
#include <string.h>

#define RTT_MEMCPY(pDest, pSrc, NumBytes)  memcpy((pDest), (pSrc), (NumBytes))

static char _acUpBuffer  [BUFFER_SIZE_UP];
static char _acDownBuffer[BUFFER_SIZE_DOWN];

SEGGER_RTT_CB _SEGGER_RTT = {
  "SEGGER RTT",
  SEGGER_RTT_MAX_NUM_UP_BUFFERS,
  SEGGER_RTT_MAX_NUM_DOWN_BUFFERS,
  {{ "Terminal", &_acUpBuffer[0],   sizeof(_acUpBuffer),   0, 0, SEGGER_RTT_MODE_DEFAULT }},
  {{ "Terminal", &_acDownBuffer[0], sizeof(_acDownBuffer), 0, 0, SEGGER_RTT_MODE_DEFAULT }}
};

void SEGGER_RTT_Init(void) {
  _SEGGER_RTT.aUp[0].sName         = "Terminal";
  _SEGGER_RTT.aUp[0].pBuffer       = &_acUpBuffer[0];
  _SEGGER_RTT.aUp[0].SizeOfBuffer  = sizeof(_acUpBuffer);
  _SEGGER_RTT.aUp[0].WrOff         = 0u;
  _SEGGER_RTT.aUp[0].RdOff         = 0u;
  _SEGGER_RTT.aUp[0].Flags         = SEGGER_RTT_MODE_DEFAULT;

  _SEGGER_RTT.aDown[0].sName       = "Terminal";
  _SEGGER_RTT.aDown[0].pBuffer     = &_acDownBuffer[0];
  _SEGGER_RTT.aDown[0].SizeOfBuffer= sizeof(_acDownBuffer);
  _SEGGER_RTT.aDown[0].WrOff       = 0u;
  _SEGGER_RTT.aDown[0].RdOff       = 0u;
  _SEGGER_RTT.aDown[0].Flags       = SEGGER_RTT_MODE_DEFAULT;

  _SEGGER_RTT.MaxNumUpBuffers      = SEGGER_RTT_MAX_NUM_UP_BUFFERS;
  _SEGGER_RTT.MaxNumDownBuffers    = SEGGER_RTT_MAX_NUM_DOWN_BUFFERS;
  strcpy(_SEGGER_RTT.acID, "SEGGER RTT");
}

static unsigned _WriteBlocking(SEGGER_RTT_BUFFER_UP* pRing, const char* pBuffer, unsigned NumBytes) {
  unsigned NumBytesToWrite;
  unsigned NumBytesWritten;
  unsigned RdOff;
  unsigned WrOff;

  NumBytesWritten = 0u;
  WrOff = pRing->WrOff;
  do {
    RdOff = pRing->RdOff;
    if (RdOff > WrOff) {
      NumBytesToWrite = RdOff - WrOff - 1u;
    } else {
      NumBytesToWrite = pRing->SizeOfBuffer - (WrOff - RdOff + 1u);
    }
    NumBytesToWrite = (NumBytesToWrite < (NumBytes - NumBytesWritten)) ? NumBytesToWrite : (NumBytes - NumBytesWritten);
    if (NumBytesToWrite > 0u) {
      if ((WrOff + NumBytesToWrite) > pRing->SizeOfBuffer) {
        unsigned NumBytesAtOnce = pRing->SizeOfBuffer - WrOff;
        RTT_MEMCPY(pRing->pBuffer + WrOff, pBuffer + NumBytesWritten, NumBytesAtOnce);
        NumBytesWritten += NumBytesAtOnce;
        WrOff = 0u;
        NumBytesToWrite -= NumBytesAtOnce;
      }
      RTT_MEMCPY(pRing->pBuffer + WrOff, pBuffer + NumBytesWritten, NumBytesToWrite);
      NumBytesWritten += NumBytesToWrite;
      WrOff += NumBytesToWrite;
      if (WrOff == pRing->SizeOfBuffer) {
        WrOff = 0u;
      }
      pRing->WrOff = WrOff;
    }
  } while (NumBytesWritten < NumBytes);
  return NumBytesWritten;
}

static unsigned _WriteNoCheck(SEGGER_RTT_BUFFER_UP* pRing, const char* pData, unsigned NumBytes) {
  unsigned NumBytesAtOnce;
  unsigned WrOff;
  unsigned Rem;

  WrOff = pRing->WrOff;
  Rem = pRing->SizeOfBuffer - WrOff;
  if (Rem > NumBytes) {
    RTT_MEMCPY(pRing->pBuffer + WrOff, pData, NumBytes);
    pRing->WrOff = WrOff + NumBytes;
  } else {
    NumBytesAtOnce = Rem;
    RTT_MEMCPY(pRing->pBuffer + WrOff, pData, NumBytesAtOnce);
    NumBytesAtOnce = NumBytes - Rem;
    RTT_MEMCPY(pRing->pBuffer, pData + Rem, NumBytesAtOnce);
    pRing->WrOff = NumBytesAtOnce;
  }
  return NumBytes;
}

unsigned SEGGER_RTT_Write(unsigned BufferIndex, const void* pBuffer, unsigned NumBytes) {
  unsigned NumBytesToWrite;
  unsigned NumBytesWritten;
  unsigned RdOff;
  unsigned WrOff;
  SEGGER_RTT_BUFFER_UP* pRing;

  if (BufferIndex >= (unsigned)_SEGGER_RTT.MaxNumUpBuffers) {
    return 0u;
  }
  pRing = &_SEGGER_RTT.aUp[BufferIndex];
  if (pRing->pBuffer == NULL) {
    return 0u;
  }
  SEGGER_RTT_LOCK();
  WrOff = pRing->WrOff;
  RdOff = pRing->RdOff;
  if (RdOff > WrOff) {
    NumBytesToWrite = RdOff - WrOff - 1u;
  } else {
    NumBytesToWrite = pRing->SizeOfBuffer - (WrOff - RdOff + 1u);
  }
  switch (pRing->Flags & SEGGER_RTT_MODE_MASK) {
  case SEGGER_RTT_MODE_NO_BLOCK_SKIP:
    if (NumBytesToWrite >= NumBytes) {
      NumBytesWritten = _WriteNoCheck(pRing, (const char*)pBuffer, NumBytes);
    } else {
      NumBytesWritten = 0u;
    }
    break;
  case SEGGER_RTT_MODE_NO_BLOCK_TRIM:
    NumBytes = (NumBytes < NumBytesToWrite) ? NumBytes : NumBytesToWrite;
    NumBytesWritten = _WriteNoCheck(pRing, (const char*)pBuffer, NumBytes);
    break;
  case SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL:
    NumBytesWritten = _WriteBlocking(pRing, (const char*)pBuffer, NumBytes);
    break;
  default:
    NumBytesWritten = 0u;
    break;
  }
  SEGGER_RTT_UNLOCK();
  return NumBytesWritten;
}

unsigned SEGGER_RTT_WriteString(unsigned BufferIndex, const char* s) {
  return SEGGER_RTT_Write(BufferIndex, s, strlen(s));
}

unsigned SEGGER_RTT_PutChar(unsigned BufferIndex, char c) {
  return SEGGER_RTT_Write(BufferIndex, &c, 1u);
}

unsigned SEGGER_RTT_Read(unsigned BufferIndex, void* pBuffer, unsigned BufferSize) {
  unsigned NumBytesRem;
  unsigned NumBytesRead;
  unsigned RdOff;
  unsigned WrOff;
  SEGGER_RTT_BUFFER_DOWN* pRing;

  if (BufferIndex >= (unsigned)_SEGGER_RTT.MaxNumDownBuffers) {
    return 0u;
  }
  pRing = &_SEGGER_RTT.aDown[BufferIndex];
  if (pRing->pBuffer == NULL) {
    return 0u;
  }
  SEGGER_RTT_LOCK();
  RdOff = pRing->RdOff;
  WrOff = pRing->WrOff;
  NumBytesRead = 0u;
  if (RdOff <= WrOff) {
    NumBytesRem = WrOff - RdOff;
  } else {
    NumBytesRem = pRing->SizeOfBuffer - (RdOff - WrOff);
  }
  NumBytesRem = (NumBytesRem < BufferSize) ? NumBytesRem : BufferSize;
  if (NumBytesRem > 0u) {
    if ((RdOff + NumBytesRem) > pRing->SizeOfBuffer) {
      unsigned NumBytesAtOnce = pRing->SizeOfBuffer - RdOff;
      RTT_MEMCPY((char*)pBuffer, pRing->pBuffer + RdOff, NumBytesAtOnce);
      NumBytesRead += NumBytesAtOnce;
      RdOff = 0u;
      NumBytesRem -= NumBytesAtOnce;
    }
    RTT_MEMCPY((char*)pBuffer + NumBytesRead, pRing->pBuffer + RdOff, NumBytesRem);
    NumBytesRead += NumBytesRem;
    RdOff += NumBytesRem;
    if (RdOff == pRing->SizeOfBuffer) {
      RdOff = 0u;
    }
    pRing->RdOff = RdOff;
  }
  SEGGER_RTT_UNLOCK();
  return NumBytesRead;
}

int SEGGER_RTT_ConfigUpBuffer(unsigned BufferIndex, const char* sName, void* pBuffer, unsigned BufferSize, unsigned Flags) {
  if (BufferIndex < (unsigned)_SEGGER_RTT.MaxNumUpBuffers) {
    SEGGER_RTT_LOCK();
    if (BufferIndex > 0u) {
      _SEGGER_RTT.aUp[BufferIndex].sName        = sName;
      _SEGGER_RTT.aUp[BufferIndex].pBuffer      = (char*)pBuffer;
      _SEGGER_RTT.aUp[BufferIndex].SizeOfBuffer = BufferSize;
      _SEGGER_RTT.aUp[BufferIndex].RdOff        = 0u;
      _SEGGER_RTT.aUp[BufferIndex].WrOff        = 0u;
    }
    _SEGGER_RTT.aUp[BufferIndex].Flags          = Flags;
    SEGGER_RTT_UNLOCK();
    return 0;
  }
  return -1;
}

int SEGGER_RTT_ConfigDownBuffer(unsigned BufferIndex, const char* sName, void* pBuffer, unsigned BufferSize, unsigned Flags) {
  if (BufferIndex < (unsigned)_SEGGER_RTT.MaxNumDownBuffers) {
    SEGGER_RTT_LOCK();
    if (BufferIndex > 0u) {
      _SEGGER_RTT.aDown[BufferIndex].sName        = sName;
      _SEGGER_RTT.aDown[BufferIndex].pBuffer      = (char*)pBuffer;
      _SEGGER_RTT.aDown[BufferIndex].SizeOfBuffer = BufferSize;
      _SEGGER_RTT.aDown[BufferIndex].RdOff        = 0u;
      _SEGGER_RTT.aDown[BufferIndex].WrOff        = 0u;
    }
    _SEGGER_RTT.aDown[BufferIndex].Flags          = Flags;
    SEGGER_RTT_UNLOCK();
    return 0;
  }
  return -1;
}

int SEGGER_RTT_GetKey(void) {
  char c;
  if (SEGGER_RTT_Read(0, &c, 1) == 1) {
    return (int)(unsigned char)c;
  }
  return -1;
}

int SEGGER_RTT_HasKey(void) {
  SEGGER_RTT_BUFFER_DOWN* pRing = &_SEGGER_RTT.aDown[0];
  return (pRing->RdOff != pRing->WrOff) ? 1 : 0;
}
