/******************************************************************************
    Uart_Rec_Analysis.c — APP 侧 UART 帧分析
******************************************************************************/
#include "UpgradeFeatureConfig.h"
#if OTA_UPGRADE_ENABLE

#include "upgrade_app.h"
#include "UartFrameHelper.h"
#include "PfcSafeAppBridge.h"
#include "UartPort.h"

_StUartRxMsg StUart1RxMsg;

static volatile uint8_t s_u8IntoPending;
static volatile uint8_t s_u8IntoBusy;
static uint8_t s_u8IntoUartCom;

uint8_t AppIntoUpgrade_IsBusy(void)
{
    if (s_u8IntoPending != 0U || s_u8IntoBusy != 0U)
    {
        return 1U;
    }
    return 0U;
}

static void AppIntoUpgrade_PauseUartRx(void)
{
    HAL_NVIC_DisableIRQ(USART1_IRQn);
    __HAL_UART_DISABLE_IT(&huart1, UART_IT_IDLE);
}

static uint8_t AppIntoUpgrade_TryQueue(_StUartRxMsg *StRxMsg, uint8_t u8UartCom)
{
    _uGetStartAddrMsg uGetStartAddrMsg;
    _uInitUpgradeModeMsg uIntoUpgradeModeMsg;

    if (s_u8IntoPending != 0U || s_u8IntoBusy != 0U)
    {
        UPGRADE_DBG("[APP_INTO_DROP] busy pending=%u busy=%u\r\n",
               (unsigned)s_u8IntoPending,
               (unsigned)s_u8IntoBusy);
        return 0U;
    }

    memcpy(uGetStartAddrMsg.u8Tab, StRxMsg->u8DataTable + DF_CMD_START_ADDR_OFFSET, sizeof(_uGetStartAddrMsg));
    uGetStartAddrMsg.StIGetStartAddrMsg.u16Len = SWAP16(uGetStartAddrMsg.StIGetStartAddrMsg.u16Len);
    uGetStartAddrMsg.StIGetStartAddrMsg.u16StartAddr = SWAP16(uGetStartAddrMsg.StIGetStartAddrMsg.u16StartAddr);

    if (StRxMsg->u16DataLen < (uGetStartAddrMsg.StIGetStartAddrMsg.u16Len + 7U))
    {
        UPGRADE_DBG("[APP_INTO_DROP] len_not_enough rx=%u expect=%u\r\n",
               StRxMsg->u16DataLen,
               (uint16_t)(uGetStartAddrMsg.StIGetStartAddrMsg.u16Len + 7U));
        return 0U;
    }
    if (uGetStartAddrMsg.StIGetStartAddrMsg.u16StartAddr != DF_INTO_UPGRADEMODE_START_ADDR)
    {
        UPGRADE_DBG("[APP_INTO_DROP] start_addr=0x%04x expect=0x%04x\r\n",
               uGetStartAddrMsg.StIGetStartAddrMsg.u16StartAddr,
               DF_INTO_UPGRADEMODE_START_ADDR);
        return 0U;
    }

    memcpy(uIntoUpgradeModeMsg.u8Tab, StRxMsg->u8DataTable + DF_CMD_DATA_OFFSET, sizeof(_uInitUpgradeModeMsg));
    uIntoUpgradeModeMsg.StIntoUpgradeModeMsg.u16IntoUpgradeMode =
        SWAP16(uIntoUpgradeModeMsg.StIntoUpgradeModeMsg.u16IntoUpgradeMode);
    uIntoUpgradeModeMsg.StIntoUpgradeModeMsg.u16UpgradeDevType =
        SWAP16(uIntoUpgradeModeMsg.StIntoUpgradeModeMsg.u16UpgradeDevType);

    if (uIntoUpgradeModeMsg.StIntoUpgradeModeMsg.u16UpgradeDevType != Df_UPGRADE_DEV_SLAVE1
        && uIntoUpgradeModeMsg.StIntoUpgradeModeMsg.u16UpgradeDevType != Df_UPGRADE_DEV_MASTET)
    {
        UPGRADE_DBG("[APP_INTO_DROP] devType=0x%04x expect SLAVE1=0x%04x or MASTET=0x%04x\r\n",
               uIntoUpgradeModeMsg.StIntoUpgradeModeMsg.u16UpgradeDevType,
               (unsigned)Df_UPGRADE_DEV_SLAVE1,
               (unsigned)Df_UPGRADE_DEV_MASTET);
        REturnIntoUpgradeModeCmd(DF_INTO_UPGRADE_MODE_FAIL_ACK, u8UartCom);
        return 0U;
    }

    s_u8IntoUartCom = u8UartCom;
    s_u8IntoPending = 1U;
    UPGRADE_DBG("[APP_INTO_UP] queued for main loop, pwr=0x%04x\r\n",
           (unsigned)DevMsg_GetPowerState());
    return 1U;
}

void AppIntoUpgrade_MainLoopTask(void)
{
    uint8_t u8UartCom;

    if (s_u8IntoPending == 0U)
    {
        return;
    }

    s_u8IntoPending = 0U;
    s_u8IntoBusy = 1U;
    u8UartCom = s_u8IntoUartCom;

    AppIntoUpgrade_PauseUartRx();

    UPGRADE_DBG("[APP_INTO_UP] main loop prepare OTA, pwr=0x%04x\r\n",
           (unsigned)DevMsg_GetPowerState());
    PfcApp_PrepareForOta();

    REturnIntoUpgradeModeCmd(DF_INTO_UPGRADE_MODE_SUCCEED_ACK, u8UartCom);

    ReadWriteUpgradeFileMsg(&StMasterProgStatus, DF_MASTET_FIRMWARE_MSG_START_ADDRESS);
    {
        uint32_t u32Vtor = SCB->VTOR;
        uint8_t u8CurSlot = DF_CHS_BLOCK_A;
        if (u32Vtor >= DF_APP_B_START_ADDRESS && u32Vtor < DF_APP_B_END_ADDRESS)
        {
            u8CurSlot = DF_CHS_BLOCK_B;
        }
        if (StMasterProgStatus.StProgStatusMsg.u8FlashUserBlock != u8CurSlot)
        {
            StMasterProgStatus.StProgStatusMsg.u8FlashUserBlock = u8CurSlot;
            UPGRADE_DBG("[APP_INTO_UP] sync FlashUserBlock=0x%02x from VTOR=0x%08lx\r\n",
                   (unsigned)u8CurSlot, (unsigned long)u32Vtor);
        }
    }
    UPGRADE_DBG("[APP_INTO_UP] before set addr=0x%08lx into=0x%02x upState=0x%04x upTo=0x%02x user=0x%02x\r\n",
           (unsigned long)DF_MASTET_FIRMWARE_MSG_START_ADDRESS,
           StMasterProgStatus.StProgStatusMsg.u8IntoUpgradFlag,
           StMasterProgStatus.StProgStatusMsg.u16UpgradeState,
           StMasterProgStatus.StProgStatusMsg.u8FlashUPtoBlock,
           StMasterProgStatus.StProgStatusMsg.u8FlashUserBlock);

    StMasterProgStatus.StProgStatusMsg.u16UpgradeState = DF_WAIT_DOWNLOAD_UPGRADR_FILE_STATUS;
    StMasterProgStatus.StProgStatusMsg.u8FlashUPtoBlock = 0xFF;
    StMasterProgStatus.StProgStatusMsg.u8IntoUpgradFlag = DF_SYS_WAIT_DOWNLOAD_FIRMWARE_STATUS;
    StMasterProgStatus.StProgStatusMsg.u8PendingBootSlot = DF_UPBLOCK_INVALID;
    StMasterProgStatus.StProgStatusMsg.u8BootConfirmState = DF_BOOT_CONFIRM_STATE_NONE;
    StMasterProgStatus.StProgStatusMsg.u8BootRetryCount = 0;
    StMasterProgStatus.StProgStatusMsg.u16LastErrorCode = 0x0000;
    UpgradeApp_ClearMetaDirty();
    SaveWriteUpgradeFileMsg(&StMasterProgStatus, DF_MASTET_FIRMWARE_MSG_START_ADDRESS);
    (void)ReadWriteUpgradeFileMsg(&StMasterProgStatus, DF_MASTET_FIRMWARE_MSG_START_ADDRESS);
    UPGRADE_DBG("[APP_INTO_UP] after save into=0x%02x upState=0x%04x upTo=0x%02x pend=0x%02x cfm=%d retry=%d err=0x%04x\r\n",
           StMasterProgStatus.StProgStatusMsg.u8IntoUpgradFlag,
           StMasterProgStatus.StProgStatusMsg.u16UpgradeState,
           StMasterProgStatus.StProgStatusMsg.u8FlashUPtoBlock,
           StMasterProgStatus.StProgStatusMsg.u8PendingBootSlot,
           StMasterProgStatus.StProgStatusMsg.u8BootConfirmState,
           StMasterProgStatus.StProgStatusMsg.u8BootRetryCount,
           StMasterProgStatus.StProgStatusMsg.u16LastErrorCode);
    SaveSysStatusMsg(DF_WAIT_DOWNLOAD_UPGRADR_FILE_STATUS);

    HAL_Delay(20);
    Sys_software_reset();
}

void UartRxMsgProTask_Function(_StUartRxMsg *StRxMsg, uint8_t u8UartCom)
{
    uint16_t u16CalculateCrc16 = 0;
    uint16_t u16GetCrc16 = 0;
    uint16_t u16FrameLen = 0;

    if (StRxMsg->u8RecFinshFlag == 0x00U)
    {
        return;
    }
    if (AppIntoUpgrade_IsBusy() != 0U)
    {
        return;
    }
    if (StRxMsg->u16DataLen < 0x05U || StRxMsg->u8DataTable[0] != DF_UART1_REC_ADDR)
    {
        UPGRADE_DBG("[APP_RX_DROP] invalid_header len=%u addr=0x%02x expect=0x%02x\r\n",
               StRxMsg->u16DataLen,
               StRxMsg->u8DataTable[0],
               DF_UART1_REC_ADDR);
        StRxMsg->u8RecFinshFlag = 0U;
        StRxMsg->u16DataLen = 0U;
        return;
    }

    u16FrameLen = UartFrame_GetExpectedLen(StRxMsg->u8DataTable, StRxMsg->u16DataLen);
    if (u16FrameLen == 0U || u16FrameLen > StRxMsg->u16DataLen)
    {
        UPGRADE_DBG("[APP_RX_DROP] len_overflow frameLen=%u rxLen=%u\r\n",
               u16FrameLen,
               StRxMsg->u16DataLen);
        StRxMsg->u8RecFinshFlag = 0U;
        StRxMsg->u16DataLen = 0U;
        return;
    }

    u16GetCrc16 = (uint16_t)StRxMsg->u8DataTable[u16FrameLen - 2U]
        + ((uint16_t)StRxMsg->u8DataTable[u16FrameLen - 1U] << 8);
    u16CalculateCrc16 = Modbus_CalCRC(StRxMsg->u8DataTable, u16FrameLen - 2U);
    if (u16CalculateCrc16 != u16GetCrc16)
    {
        UPGRADE_DBG("[APP_RX_DROP] crc_mismatch calc=0x%04x recv=0x%04x len=%u func=0x%02x\r\n",
               u16CalculateCrc16,
               u16GetCrc16,
               u16FrameLen,
               StRxMsg->u8DataTable[1]);
        StRxMsg->u8RecFinshFlag = 0U;
        StRxMsg->u16DataLen = 0U;
        return;
    }

    StRxMsg->u16GetDataLen = u16FrameLen;
    StRxMsg->u16DataLen = u16FrameLen;

    UPGRADE_DBG("[APP_RX] frame_ok func=0x%02x len=%u\r\n",
           StRxMsg->u8DataTable[1],
           StRxMsg->u16DataLen);

    switch (StRxMsg->u8DataTable[1])
    {
    case DF_UART_READ_DEV_MSG_FUNCTION_CODE:
        if (DevMsg_ShouldIgnoreReadDuringUpgrade(DF_DEV_RUN_MODE_APP) == 0U)
        {
            ProcessReadDevMsgRequest(StRxMsg->u8DataTable, StRxMsg->u16DataLen, u8UartCom,
                                     DF_DEV_RUN_MODE_APP, DevMsg_GetActiveSlotFromVtor());
        }
        break;
    case DF_UART_UPGRADE_FUNCTION_CODE:
        (void)AppIntoUpgrade_TryQueue(StRxMsg, u8UartCom);
        break;
    default:
        UPGRADE_DBG("[APP_RX_DROP] unknown_func func=0x%02x len=%u\r\n",
               StRxMsg->u8DataTable[1],
               StRxMsg->u16DataLen);
        break;
    }

    StRxMsg->u8RecFinshFlag = 0U;
    StRxMsg->u16DataLen = 0U;
}

#else /* OTA_UPGRADE_ENABLE */

#include "Uart_Rec_Analysis.h"

_StUartRxMsg StUart1RxMsg;

uint8_t AppIntoUpgrade_IsBusy(void) { return 0U; }
void AppIntoUpgrade_MainLoopTask(void) {}
void UartRxMsgProTask_Function(_StUartRxMsg *StRxMsg, uint8_t u8UartCom)
{
    (void)StRxMsg;
    (void)u8UartCom;
}

#endif /* OTA_UPGRADE_ENABLE */
