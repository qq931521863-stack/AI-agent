/******************************************************************************
    ???:   UartRecUpgradeCmdRetuenDf.h
******************************************************************************/
#ifndef _UartRecUpgradeCmdRetuenDf_H
#define _UartRecUpgradeCmdRetuenDf_H

#include "stm32g4xx_hal.h"

#pragma pack(1)
typedef struct
{
    void (*UartSendMsg)(uint8_t *u8Buf, uint16_t u16len);
}_StvoidFunctionUartDmaSend;
#pragma pack()

extern _StvoidFunctionUartDmaSend _StUartDmaSendTab[2];

void REturnIntoUpgradeModeCmd(uint16_t u16returnResult, uint8_t u8UartCom);
void REturnUpgradeStartCmd(uint16_t u16returnResult, uint8_t u8UartCom);
void REturnUpgradeFileTransferCmd(uint16_t u16returnResult, uint8_t u8UartCom);
void REturnUpgradeFileTransferEndCmd(uint16_t u16returnResult, uint8_t u8UartCom);
void REturnUpgradeStatusInquireCmd(uint16_t u16DevTypeCode, uint16_t u16UpgradeStatusCode,
                                   uint16_t u16UpgradeScheduleCode, uint8_t u8UartCom);

#endif
