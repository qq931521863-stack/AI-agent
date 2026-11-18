/******************************************************************************
    Uart_Rec_Analysis.h
******************************************************************************/
#ifndef _Uart_Rec_Analysis_H
#define _Uart_Rec_Analysis_H

#include "stm32g4xx_hal.h"

#pragma pack(1)
typedef struct
{
    uint16_t u16DataLen;
    uint8_t u8DataTable[1048];
    uint8_t u8RecFinshFlag;
    uint16_t u16GetDataLen;
}_StUartRxMsg;
#pragma pack()

extern _StUartRxMsg StUart1RxMsg;
void UartRxMsgProTask_Function(_StUartRxMsg *StRxMsg, uint8_t u8UartCom);
void AppIntoUpgrade_MainLoopTask(void);
uint8_t AppIntoUpgrade_IsBusy(void);

#endif
