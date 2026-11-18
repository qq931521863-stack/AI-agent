#include "UpgradeFeatureConfig.h"
#if OTA_UPGRADE_ENABLE

#include "upgrade_app.h"

_StvoidFunctionUartDmaSend _StUartDmaSendTab[2] =
{
    Uart1DMASendFunction,
    0,
};

void REturnIntoUpgradeModeCmd(uint16_t u16returnResult, uint8_t u8UartCom)
{
    _uReturnIntoUpgradeModeMsg uReturnIntoUpgradeModeMsg;
    uReturnIntoUpgradeModeMsg.StReturnIntoUpgradeModeMsg.u8ID = DF_UART1_REC_ADDR;
    uReturnIntoUpgradeModeMsg.StReturnIntoUpgradeModeMsg.u8FunctionCode = DF_UART_UPGRADE_FUNCTION_CODE;
    uReturnIntoUpgradeModeMsg.StReturnIntoUpgradeModeMsg.u16StartAddr = SWAP16(DF_INTO_UPGRADEMODE_START_ADDR);
    uReturnIntoUpgradeModeMsg.StReturnIntoUpgradeModeMsg.u16Len = SWAP16(DF_RETURN_INTO_UPGRADEMODE_MODE_DATA_LEN);
    uReturnIntoUpgradeModeMsg.StReturnIntoUpgradeModeMsg.u16ReturnData = SWAP16(u16returnResult);
    uReturnIntoUpgradeModeMsg.StReturnIntoUpgradeModeMsg.u16Crc16 = Modbus_CalCRC(uReturnIntoUpgradeModeMsg.u8Tab, sizeof(_uReturnIntoUpgradeModeMsg) - 2);
    _StUartDmaSendTab[u8UartCom].UartSendMsg(uReturnIntoUpgradeModeMsg.u8Tab, sizeof(_uReturnIntoUpgradeModeMsg));
}

void REturnUpgradeStartCmd(uint16_t u16returnResult, uint8_t u8UartCom)
{
    _uReturnUpgradeStartMsg uReturnUpgradeStartMsg;
    uReturnUpgradeStartMsg.StReturnUpgradeStartMsg.u8ID = DF_UART1_REC_ADDR;
    uReturnUpgradeStartMsg.StReturnUpgradeStartMsg.u8FunctionCode = DF_UART_UPGRADE_FUNCTION_CODE;
    uReturnUpgradeStartMsg.StReturnUpgradeStartMsg.u16StartAddr = SWAP16(DF_START_UPGRADE_START_ADDR);
    uReturnUpgradeStartMsg.StReturnUpgradeStartMsg.u16Len = SWAP16(DF_RETURN_START_UPGRADE_DATA_LEN);
    uReturnUpgradeStartMsg.StReturnUpgradeStartMsg.u16ReturnData = SWAP16(u16returnResult);
    uReturnUpgradeStartMsg.StReturnUpgradeStartMsg.u16FrameMaxByte = SWAP16(DF_UPGRADE_FILE_TRANSFER_PACK_DATA_MAC_LEN);
    uReturnUpgradeStartMsg.StReturnUpgradeStartMsg.u16Crc16 = Modbus_CalCRC(uReturnUpgradeStartMsg.u8Tab, sizeof(_uReturnUpgradeStartMsg) - 2);
    _StUartDmaSendTab[u8UartCom].UartSendMsg(uReturnUpgradeStartMsg.u8Tab, sizeof(_uReturnUpgradeStartMsg));
}

void REturnUpgradeFileTransferCmd(uint16_t u16returnResult, uint8_t u8UartCom)
{
    _uReturnIntoUpgradeModeMsg uReturnIntoUpgradeModeMsg;
    uReturnIntoUpgradeModeMsg.StReturnIntoUpgradeModeMsg.u8ID = DF_UART1_REC_ADDR;
    uReturnIntoUpgradeModeMsg.StReturnIntoUpgradeModeMsg.u8FunctionCode = DF_UART_UPGRADE_FUNCTION_CODE;
    uReturnIntoUpgradeModeMsg.StReturnIntoUpgradeModeMsg.u16StartAddr = SWAP16(DF_UPGRADE_FILE_TRANSFER_START_ADDR);
    uReturnIntoUpgradeModeMsg.StReturnIntoUpgradeModeMsg.u16Len = SWAP16(DF_RETURN_UPGRADE_FILE_TRANSFER_DATA_LEN);
    uReturnIntoUpgradeModeMsg.StReturnIntoUpgradeModeMsg.u16ReturnData = SWAP16(u16returnResult);
    uReturnIntoUpgradeModeMsg.StReturnIntoUpgradeModeMsg.u16Crc16 = Modbus_CalCRC(uReturnIntoUpgradeModeMsg.u8Tab, sizeof(_uReturnIntoUpgradeModeMsg) - 2);
    _StUartDmaSendTab[u8UartCom].UartSendMsg(uReturnIntoUpgradeModeMsg.u8Tab, sizeof(_uReturnIntoUpgradeModeMsg));
}

void REturnUpgradeFileTransferEndCmd(uint16_t u16returnResult, uint8_t u8UartCom)
{
    _uReturnIntoUpgradeModeMsg uReturnIntoUpgradeModeMsg;
    uReturnIntoUpgradeModeMsg.StReturnIntoUpgradeModeMsg.u8ID = DF_UART1_REC_ADDR;
    uReturnIntoUpgradeModeMsg.StReturnIntoUpgradeModeMsg.u8FunctionCode = DF_UART_UPGRADE_FUNCTION_CODE;
    uReturnIntoUpgradeModeMsg.StReturnIntoUpgradeModeMsg.u16StartAddr = SWAP16(DF_UPGRADE_FILE_TRANSFER_END_START_ADDR);
    uReturnIntoUpgradeModeMsg.StReturnIntoUpgradeModeMsg.u16Len = SWAP16(DF_RETURN_UPGRADE_FILE_TRANSFER_END_DATA_LEN);
    uReturnIntoUpgradeModeMsg.StReturnIntoUpgradeModeMsg.u16ReturnData = SWAP16(u16returnResult);
    uReturnIntoUpgradeModeMsg.StReturnIntoUpgradeModeMsg.u16Crc16 = Modbus_CalCRC(uReturnIntoUpgradeModeMsg.u8Tab, sizeof(_uReturnIntoUpgradeModeMsg) - 2);
    _StUartDmaSendTab[u8UartCom].UartSendMsg(uReturnIntoUpgradeModeMsg.u8Tab, sizeof(_uReturnIntoUpgradeModeMsg));
}

void REturnUpgradeStatusInquireCmd(uint16_t u16DevTypeCode, uint16_t u16UpgradeStatusCode, uint16_t u16UpgradeScheduleCode, uint8_t u8UartCom)
{
    _uReturnUpgradeScheduleMsg uReturnUpgradeScheduleMsg;
    uReturnUpgradeScheduleMsg.StReturnUpgradeScheduleMsg.u8ID = DF_UART1_REC_ADDR;
    uReturnUpgradeScheduleMsg.StReturnUpgradeScheduleMsg.u8FunctionCode = DF_UART_UPGRADE_FUNCTION_CODE;
    uReturnUpgradeScheduleMsg.StReturnUpgradeScheduleMsg.u16StartAddr = SWAP16(DF_UPGRADE_SCHEDULE_INQUIRE_START_ADDR);
    uReturnUpgradeScheduleMsg.StReturnUpgradeScheduleMsg.u16Len = SWAP16(DF_RETURN_UPGRADE_SCHEDULE_INQUIRE_DATA_LEN);
    uReturnUpgradeScheduleMsg.StReturnUpgradeScheduleMsg.u16DevType = SWAP16(u16DevTypeCode);
    uReturnUpgradeScheduleMsg.StReturnUpgradeScheduleMsg.u16UpgradeStatus = SWAP16(u16UpgradeStatusCode);
    uReturnUpgradeScheduleMsg.StReturnUpgradeScheduleMsg.u16UpgradeSchedule = SWAP16(u16UpgradeScheduleCode);
    uReturnUpgradeScheduleMsg.StReturnUpgradeScheduleMsg.u16Reson = SWAP16(0x0000);
    uReturnUpgradeScheduleMsg.StReturnUpgradeScheduleMsg.u16Crc16 = Modbus_CalCRC(uReturnUpgradeScheduleMsg.u8Tab, sizeof(_uReturnUpgradeScheduleMsg) - 2);
    _StUartDmaSendTab[u8UartCom].UartSendMsg(uReturnUpgradeScheduleMsg.u8Tab, sizeof(_uReturnUpgradeScheduleMsg));
}

#endif /* OTA_UPGRADE_ENABLE */
