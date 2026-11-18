#ifndef PFC_SAFE_APP_BRIDGE_H
#define PFC_SAFE_APP_BRIDGE_H

#include "stm32g4xx_hal.h"
#include "UpgradeFeatureConfig.h"

uint8_t PfcApp_IsPowerSafe(void);
uint8_t PfcApp_IsFaultLocked(void);
void PfcApp_RequestSafeShutdown(void);
#if OTA_UPGRADE_ENABLE
void PfcApp_PrepareForOta(void);
#endif
uint8_t PfcApp_WaitForSafeState(uint32_t u32TimeoutMs);

#endif
