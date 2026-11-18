/******************************************************************************

    文件�?   InquireDevMsg.c

******************************************************************************/

#include "UpgradeFeatureConfig.h"
#if OTA_UPGRADE_ENABLE

#include "upgrade_app.h"



_uStDevMsg StMasterDevMsg;



uint16_t DevMsg_GetActiveSlotFromVtor(void)

{

    uint32_t u32VtorAddr = SCB->VTOR;



    if (u32VtorAddr >= DF_APP_B_START_ADDRESS && u32VtorAddr < DF_APP_B_END_ADDRESS)

    {

        return (uint16_t)DF_CHS_BLOCK_B;

    }

    return (uint16_t)DF_CHS_BLOCK_A;

}



static uint16_t DevMsg_NormalizeSlotVer(uint16_t u16Ver, uint8_t u8Valid, uint16_t u16Fallback)

{

    if (u16Ver == 0U || u16Ver == 0xFFFFU)

    {

        if (u8Valid == DF_UPGRADE_FILE_VAILE_STATUS)

        {

            return u16Fallback;

        }

        return 0U;

    }

    return u16Ver;

}



void DevMsg_FillExtInfo(_StDevExtMsg *pExt, uint16_t u16RunMode, uint16_t u16ActiveSlot)

{

    uint16_t u16SlotA;

    uint16_t u16SlotB;

    uint16_t u16Soft;



    if (pExt == NULL)

    {

        return;

    }

    ReadWriteUpgradeFileMsg(&StMasterProgStatus, DF_MASTET_FIRMWARE_MSG_START_ADDRESS);

    u16Soft = StMasterDevMsg.StDevMsg.u16SoftVersion;

    pExt->u16DevType         = StMasterDevMsg.StDevMsg.u16DevType;

    pExt->u16HardwareVersion = StMasterDevMsg.StDevMsg.u16HardwareVersion;

    pExt->u16SoftVersion     = u16Soft;

    pExt->u16SysRunTime      = StMasterDevMsg.StDevMsg.u16SysRunTime;

    pExt->u16DevOnlineStatus = StMasterDevMsg.StDevMsg.u16DevOnlineStatus;

    pExt->u16RunMode         = u16RunMode;

    pExt->u16ActiveSlot      = u16ActiveSlot;



    u16SlotA = DevMsg_NormalizeSlotVer(StMasterProgStatus.StProgStatusMsg.u16BlockAFirmwareVersion,

                                       StMasterProgStatus.StProgStatusMsg.u8BlockABinFileValidFlag,

                                       StMasterProgStatus.StProgStatusMsg.u16BlockABinFlieCrc16);

    u16SlotB = DevMsg_NormalizeSlotVer(StMasterProgStatus.StProgStatusMsg.u16BlockBFirmwareVersion,

                                       StMasterProgStatus.StProgStatusMsg.u8BlockBBinFileValidFlag,

                                       StMasterProgStatus.StProgStatusMsg.u16BlockBBinFlieCrc16);

    if (u16ActiveSlot == (uint16_t)DF_CHS_BLOCK_A && u16SlotA == 0U)

    {

        u16SlotA = u16Soft;

    }

    else if (u16ActiveSlot == (uint16_t)DF_CHS_BLOCK_B && u16SlotB == 0U)

    {

        u16SlotB = u16Soft;

    }

    pExt->u16SlotAFwVer = u16SlotA;

    pExt->u16SlotBFwVer = u16SlotB;

    pExt->u16PowerState = DevMsg_GetPowerState();

}



uint8_t DevMsg_ShouldIgnoreReadDuringUpgrade(uint16_t u16RunMode)

{

    (void)u16RunMode;

    if (StMasterProgStatus.StProgStatusMsg.u8IntoUpgradFlag == DF_SYS_WAIT_DOWNLOAD_FIRMWARE_STATUS)

    {

        return 1U;

    }

    return 0U;

}



void ReturnCheckVersionExtInquireCmd(uint8_t u8UartCom, uint16_t u16RunMode, uint16_t u16ActiveSlot)

{

    _uReturnCheckDeviceExtMsg uRet;

    _StDevExtMsg ext;



    DevMsg_FillExtInfo(&ext, u16RunMode, u16ActiveSlot);

    uRet.StReturnCheckDeviceExtMsg.u8ID = DF_UART1_REC_ADDR;

    uRet.StReturnCheckDeviceExtMsg.u8FunctionCode = DF_UART_READ_DEV_MSG_FUNCTION_CODE;

    uRet.StReturnCheckDeviceExtMsg.u16StartAddr = SWAP16(DF_UART_READ_DEV_MSG_START_ADDR);

    uRet.StReturnCheckDeviceExtMsg.u16Len = SWAP16(DF_UART_READ_DEV_MSG_DATA_LEN);

    uRet.StReturnCheckDeviceExtMsg.StDevExtMsg.u16DevType         = SWAP16(ext.u16DevType);

    uRet.StReturnCheckDeviceExtMsg.StDevExtMsg.u16HardwareVersion = SWAP16(ext.u16HardwareVersion);

    uRet.StReturnCheckDeviceExtMsg.StDevExtMsg.u16SoftVersion     = SWAP16(ext.u16SoftVersion);

    uRet.StReturnCheckDeviceExtMsg.StDevExtMsg.u16SysRunTime      = SWAP16(ext.u16SysRunTime);

    uRet.StReturnCheckDeviceExtMsg.StDevExtMsg.u16DevOnlineStatus = SWAP16(ext.u16DevOnlineStatus);

    uRet.StReturnCheckDeviceExtMsg.StDevExtMsg.u16RunMode         = SWAP16(ext.u16RunMode);

    uRet.StReturnCheckDeviceExtMsg.StDevExtMsg.u16ActiveSlot      = SWAP16(ext.u16ActiveSlot);

    uRet.StReturnCheckDeviceExtMsg.StDevExtMsg.u16SlotAFwVer      = SWAP16(ext.u16SlotAFwVer);

    uRet.StReturnCheckDeviceExtMsg.StDevExtMsg.u16SlotBFwVer      = SWAP16(ext.u16SlotBFwVer);

    uRet.StReturnCheckDeviceExtMsg.StDevExtMsg.u16PowerState     = SWAP16(ext.u16PowerState);

    uRet.StReturnCheckDeviceExtMsg.u16Crc16 = Modbus_CalCRC(uRet.u8Tab, sizeof(_uReturnCheckDeviceExtMsg) - 2U);

    _StUartDmaSendTab[u8UartCom].UartSendMsg(uRet.u8Tab, sizeof(_uReturnCheckDeviceExtMsg));

}



static uint16_t DevMsg_ParseRequestDevType(const uint8_t *u8Buf, uint16_t u16Len)

{

    if (u8Buf == NULL || u16Len < 10U)

    {

        return 0U;

    }

    return (uint16_t)((uint16_t)u8Buf[6] << 8) | (uint16_t)u8Buf[7];

}



static uint8_t DevMsg_ShouldRespondToQuery(const uint8_t *u8Buf, uint16_t u16Len)

{

    uint16_t u16ReqDevType;

    uint16_t u16MyDevType;



    if (u8Buf == NULL || u8Buf[0] != DF_UART1_REC_ADDR || u16Len < 8U)

    {

        return 0U;

    }

    u16ReqDevType = DevMsg_ParseRequestDevType(u8Buf, u16Len);

    if (u16ReqDevType == 0U)

    {

        return 1U;

    }

    u16MyDevType = StMasterDevMsg.StDevMsg.u16DevType;

    return (u16ReqDevType == u16MyDevType) ? 1U : 0U;

}



void ProcessReadDevMsgRequest(const uint8_t *u8Buf, uint16_t u16Len, uint8_t u8UartCom, uint16_t u16RunMode, uint16_t u16ActiveSlot)

{

    if (u8Buf == NULL || u16Len < 8U)

    {

        return;

    }

    if (DevMsg_ShouldIgnoreReadDuringUpgrade(u16RunMode) != 0U)

    {

        UPGRADE_DBG("[APP_RX_DROP] read_dev_busy into=0x%02x\r\n",

               (unsigned)StMasterProgStatus.StProgStatusMsg.u8IntoUpgradFlag);

        return;

    }

    if (DevMsg_ShouldRespondToQuery(u8Buf, u16Len) == 0U)

    {

        return;

    }

    ReturnCheckVersionExtInquireCmd(u8UartCom, u16RunMode, u16ActiveSlot);

}

#else /* OTA_UPGRADE_ENABLE */

#include "InquireDevMsg.h"

_uStDevMsg StMasterDevMsg;

uint16_t DevMsg_GetActiveSlotFromVtor(void) { return 0U; }
void DevMsg_FillExtInfo(_StDevExtMsg *pExt, uint16_t u16RunMode, uint16_t u16ActiveSlot)
{
    (void)pExt;
    (void)u16RunMode;
    (void)u16ActiveSlot;
}
uint8_t DevMsg_ShouldIgnoreReadDuringUpgrade(uint16_t u16RunMode) { (void)u16RunMode; return 0U; }
void ReturnCheckVersionExtInquireCmd(uint8_t u8UartCom, uint16_t u16RunMode, uint16_t u16ActiveSlot)
{
    (void)u8UartCom;
    (void)u16RunMode;
    (void)u16ActiveSlot;
}
void ProcessReadDevMsgRequest(const uint8_t *u8Buf, uint16_t u16Len, uint8_t u8UartCom,
                              uint16_t u16RunMode, uint16_t u16ActiveSlot)
{
    (void)u8Buf;
    (void)u16Len;
    (void)u8UartCom;
    (void)u16RunMode;
    (void)u16ActiveSlot;
}

#endif /* OTA_UPGRADE_ENABLE */

