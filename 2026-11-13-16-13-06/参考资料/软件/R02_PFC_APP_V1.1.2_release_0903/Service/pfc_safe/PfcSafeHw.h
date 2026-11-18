#ifndef PFC_SAFE_HW_H
#define PFC_SAFE_HW_H

#include "stm32g4xx_hal.h"

/* 图腾柱 PFC：FastMOS/SlowMos 低电平关断 */
void PfcHw_EarlySafePinInit(void);
void PfcHw_SafeShutdown(void);

#endif
