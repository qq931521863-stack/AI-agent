#include "UpgradeFeatureConfig.h"
#if OTA_UPGRADE_ENABLE

#include "upgrade_app.h"

void Timer1msProFunction(void)
{
    static uint8_t s_u8SysRunMs = 0U;

    UartRxMsgProTask_Function(&StUart1RxMsg, DF_UART1_COM);

    if (++s_u8SysRunMs >= 50U)
    {
        s_u8SysRunMs = 0U;
        StMasterDevMsg.StDevMsg.u16SysRunTime++;
    }
}

#endif /* OTA_UPGRADE_ENABLE */
