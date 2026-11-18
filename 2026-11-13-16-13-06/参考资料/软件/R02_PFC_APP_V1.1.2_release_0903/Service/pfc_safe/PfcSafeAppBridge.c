#include "PfcSafeHw.h"
#include "HwConfig.h"
#include "UpgradeFeatureConfig.h"
#include "InquireDevMsg.h"
uint8_t PfcApp_WaitForSafeState(uint32_t u32TimeoutMs);
uint8_t PfcApp_IsPowerSafe(void)
{
    switch (DataFlowFace.RunState)
    {
    case Init:
    case Wait:
    case Err:
        return 1U;
    default:
        return 0U;
    }
}

uint8_t PfcApp_IsFaultLocked(void)
{
    if (DataFlowFace.RunState == Err && DataFlowFace.FaultSta.all != 0U)
    {
        return 1U;
    }
    return 0U;
}

void PfcApp_RequestSafeShutdown(void)
{
    PwmClose();
    RelayOff();
    PfcHw_SafeShutdown();
    DataFlowFace.RunState = Wait;
}

#if OTA_UPGRADE_ENABLE
void PfcApp_PrepareForOta(void)
{
    if (PfcApp_IsPowerSafe() == 0U)
    {
        (void)PfcApp_WaitForSafeState(3000U);
    }

    PwmClose();
    RelayOff();
    PfcHw_SafeShutdown();
    DataFlowFace.FaultSta.all = 0U;
    DataFlowFace.RunState = Wait;
}
#endif /* OTA_UPGRADE_ENABLE */

uint8_t PfcApp_WaitForSafeState(uint32_t u32TimeoutMs)
{
    uint32_t u32Start = HAL_GetTick();

    while ((uint32_t)(HAL_GetTick() - u32Start) < u32TimeoutMs)
    {
        if (PfcApp_IsPowerSafe() != 0U)
        {
            return 1U;
        }
        PwmClose();
        RelayOff();
        PfcHw_SafeShutdown();
        DataFlowFace.RunState = Wait;
    }

    return PfcApp_IsPowerSafe();
}

uint16_t DevMsg_GetPowerState(void)
{
    switch (DataFlowFace.RunState)
    {
    case Init:
        return DF_PWR_STATE_INIT;
    case Wait:
        return DF_PWR_STATE_WAIT;
    case Err:
        return DF_PWR_STATE_ERR;
    case Rise:
        return DF_PWR_STATE_RISE;
    case Derate:
        return DF_PWR_STATE_DERATE;
    case Run:
        return DF_PWR_STATE_RUN;
    default:
        return DF_PWR_STATE_WAIT;
    }
}
