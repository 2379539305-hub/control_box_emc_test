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
#include <stdarg.h>

#define FORMAT_FLAG_LEFT_JUSTIFY   (1u << 0)
#define FORMAT_FLAG_PAD_ZERO       (1u << 1)
#define FORMAT_FLAG_PRINT_SIGN     (1u << 2)
#define FORMAT_FLAG_ALTERNATE      (1u << 3)

typedef struct {
  char*     pBuffer;
  unsigned  BufferSize;
  unsigned  Cnt;
  unsigned  RTTBufferIndex;
  int       ReturnValue;
} SEGGER_RTT_PRINTF_DESC;

static void _StoreChar(SEGGER_RTT_PRINTF_DESC * p, char c) {
  unsigned Cnt;
  Cnt = p->Cnt;
  if ((Cnt + 1u) <= p->BufferSize) {
    *(p->pBuffer + Cnt) = c;
    p->Cnt = Cnt + 1u;
    p->ReturnValue++;
  }
  if (p->Cnt == p->BufferSize) {
    if (SEGGER_RTT_Write(p->RTTBufferIndex, p->pBuffer, p->Cnt) != p->Cnt) {
      p->ReturnValue = -1;
    } else {
      p->Cnt = 0u;
    }
  }
}

static void _PrintUnsigned(SEGGER_RTT_PRINTF_DESC * pBufferDesc, unsigned v, unsigned Base, unsigned NumDigits, unsigned FieldWidth, unsigned FormatFlags) {
  static const char _aV2C[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
  unsigned Div;
  unsigned Digit;
  unsigned Number;
  unsigned Width;
  char c;

  Number = v;
  Digit = 1u;
  while (Number >= Base) {
    Number /= Base;
    Digit++;
  }
  Width = (NumDigits > Digit) ? NumDigits : Digit;
  if ((FormatFlags & FORMAT_FLAG_LEFT_JUSTIFY) == 0u) {
    if (FieldWidth > Width) {
      FieldWidth -= Width;
      c = ' ';
      if ((FormatFlags & FORMAT_FLAG_PAD_ZERO) != 0u) {
        c = '0';
      }
      do {
        _StoreChar(pBufferDesc, c);
      } while (--FieldWidth);
    }
  }
  Div = 1u;
  while (Digit > 1u) {
    Div *= Base;
    Digit--;
  }
  while (NumDigits > (Digit + 1u)) {
    _StoreChar(pBufferDesc, '0');
    NumDigits--;
  }
  do {
    _StoreChar(pBufferDesc, _aV2C[v / Div]);
    v %= Div;
    Div /= Base;
  } while (Div);
  if ((FormatFlags & FORMAT_FLAG_LEFT_JUSTIFY) != 0u) {
    if (FieldWidth > Width) {
      FieldWidth -= Width;
      do {
        _StoreChar(pBufferDesc, ' ');
      } while (--FieldWidth);
    }
  }
}

static void _PrintInt(SEGGER_RTT_PRINTF_DESC * pBufferDesc, int v, unsigned Base, unsigned NumDigits, unsigned FieldWidth, unsigned FormatFlags) {
  unsigned Number;

  if (v < 0) {
    v = -v;
    Number = (unsigned)v;
    _StoreChar(pBufferDesc, '-');
    if (FieldWidth) {
      FieldWidth--;
    }
  } else {
    Number = (unsigned)v;
    if ((FormatFlags & FORMAT_FLAG_PRINT_SIGN) != 0u) {
      _StoreChar(pBufferDesc, '+');
      if (FieldWidth) {
        FieldWidth--;
      }
    }
  }
  _PrintUnsigned(pBufferDesc, Number, Base, NumDigits, FieldWidth, FormatFlags);
}

static int _vPrint(SEGGER_RTT_PRINTF_DESC * pBufferDesc, const char * sFormat, va_list * pParamList) {
  char c;
  char * s;
  int v;
  unsigned NumDigits;
  unsigned FieldWidth;
  unsigned FormatFlags;

  do {
    c = *sFormat++;
    if (c == 0u) {
      break;
    }
    if (c == '%') {
      FormatFlags = 0u;
      v = 1;
      do {
        c = *sFormat++;
        switch (c) {
        case '-': FormatFlags |= FORMAT_FLAG_LEFT_JUSTIFY; break;
        case '0': FormatFlags |= FORMAT_FLAG_PAD_ZERO; break;
        case '+': FormatFlags |= FORMAT_FLAG_PRINT_SIGN; break;
        case '#': FormatFlags |= FORMAT_FLAG_ALTERNATE; break;
        default:  v = 0; break;
        }
      } while (v);
      FieldWidth = 0u;
      while ((c >= '0') && (c <= '9')) {
        FieldWidth = (FieldWidth * 10u) + (unsigned)(c - '0');
        c = *sFormat++;
      }
      NumDigits = 0u;
      if (c == '.') {
        c = *sFormat++;
        while ((c >= '0') && (c <= '9')) {
          NumDigits = (NumDigits * 10u) + (unsigned)(c - '0');
          c = *sFormat++;
        }
      }
      switch (c) {
      case 'c':
        v = va_arg(*pParamList, int);
        _StoreChar(pBufferDesc, (char)v);
        break;
      case 'd':
      case 'i':
        v = va_arg(*pParamList, int);
        _PrintInt(pBufferDesc, v, 10u, NumDigits, FieldWidth, FormatFlags);
        break;
      case 'u':
        v = va_arg(*pParamList, int);
        _PrintUnsigned(pBufferDesc, (unsigned)v, 10u, NumDigits, FieldWidth, FormatFlags);
        break;
      case 'x':
      case 'X':
        v = va_arg(*pParamList, int);
        _PrintUnsigned(pBufferDesc, (unsigned)v, 16u, NumDigits, FieldWidth, FormatFlags);
        break;
      case 's':
        s = va_arg(*pParamList, char *);
        while (*s) {
          _StoreChar(pBufferDesc, *s++);
        }
        break;
      case '%':
        _StoreChar(pBufferDesc, '%');
        break;
      default:
        _StoreChar(pBufferDesc, c);
        break;
      }
    } else {
      _StoreChar(pBufferDesc, c);
    }
  } while (pBufferDesc->ReturnValue >= 0);
  return pBufferDesc->ReturnValue;
}

int SEGGER_RTT_vprintf(unsigned BufferIndex, const char * sFormat, va_list * pParamList) {
  char acBuffer[SEGGER_RTT_PRINTF_BUFFER_SIZE];
  SEGGER_RTT_PRINTF_DESC BufferDesc;

  BufferDesc.pBuffer        = acBuffer;
  BufferDesc.BufferSize     = sizeof(acBuffer);
  BufferDesc.Cnt            = 0u;
  BufferDesc.RTTBufferIndex = BufferIndex;
  BufferDesc.ReturnValue    = 0;

  _vPrint(&BufferDesc, sFormat, pParamList);
  if (BufferDesc.ReturnValue >= 0) {
    if (BufferDesc.Cnt > 0u) {
      if (SEGGER_RTT_Write(BufferIndex, acBuffer, BufferDesc.Cnt) != BufferDesc.Cnt) {
        BufferDesc.ReturnValue = -1;
      }
    }
  }
  return BufferDesc.ReturnValue;
}

int SEGGER_RTT_printf(unsigned BufferIndex, const char * sFormat, ...) {
  va_list ParamList;
  int r;

  va_start(ParamList, sFormat);
  r = SEGGER_RTT_vprintf(BufferIndex, sFormat, &ParamList);
  va_end(ParamList);
  return r;
}
