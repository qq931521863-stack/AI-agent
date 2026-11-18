#include "UpgradeFeatureConfig.h"
#if OTA_UPGRADE_ENABLE

#include "UpgradeDebugLog.h"
#include "usart3_debug.h"

void UpgradeDebugLog_Init(void)
{
#if UPGRADE_DBG_ENABLE
    USART3_DebugInit();
#endif
}

#else /* OTA_UPGRADE_ENABLE */

#include "UpgradeDebugLog.h"

void UpgradeDebugLog_Init(void) {}

#endif /* OTA_UPGRADE_ENABLE */
