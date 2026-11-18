#ifndef _UartFrameHelper_H_
#define _UartFrameHelper_H_
#include "stdint.h"

uint16_t UartFrame_GetExpectedLen(const uint8_t *pBuf, uint16_t u16RxLen);
uint8_t  UartFrame_IsComplete(const uint8_t *pBuf, uint16_t u16RxLen);

#endif
