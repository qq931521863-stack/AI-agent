#ifndef UPGRADE_APP_H
#define UPGRADE_APP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "UpgradeFeatureConfig.h"
#include "stm32g4xx_hal.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#if OTA_UPGRADE_ENABLE

#include "UpgradeDebugLog.h"
#include "Timer_pro.h"
#include "crc16.h"
#include "UpgradeMsgSaveFunctionDf.h"
#include "flash_internal.h"
#include "FlashMemoryMap.h"
#include "UartRecUpgradeCmdPro.h"
#include "InquireDevMsg.h"
#include "Uart_Rec_Analysis.h"
#include "UartRecUpgradeCmdRetuenDf.h"
#include "AppSlotValidate.h"
#include "UartPort.h"

#define DF_UART1_COM        0x00

void UpgradeAppEarlyInit(void);
void UpgradeAppInit(void);
void UpgradeApp_DeferredMetaFlush(void);
void UpgradeApp_MarkMetaDirty(void);
void UpgradeApp_ClearMetaDirty(void);

#else /* OTA_UPGRADE_ENABLE */

void UpgradeAppEarlyInit(void);
void UpgradeAppInit(void);
static inline void UpgradeApp_DeferredMetaFlush(void) {}
static inline void UpgradeApp_MarkMetaDirty(void) {}
static inline void UpgradeApp_ClearMetaDirty(void) {}

#endif /* OTA_UPGRADE_ENABLE */

#ifdef __cplusplus
}
#endif

#endif
