/******************************************************************************
    ???:   InquireDevMsg.h
******************************************************************************/
#ifndef _InquireDevMsg_H_
#define _InquireDevMsg_H_

#include "UpgradeFeatureConfig.h"
#include "upgrade_app.h"

#define CAN_ID_STD 0
#define CAN_ID_EXT 1
#define DF_CAN_FT_DATA 0
#define DF_CAN_FT_REMOT 1

#define DF_UART_READ_DEV_MSG_FUNCTION_CODE      0x15U
#define DF_UART_READ_DEV_MSG_START_ADDR         0x1400U
#define DF_UART_READ_DEV_MSG_REQ_FILTER_LEN     0x0002U
#define DF_UART_READ_DEV_MSG_DATA_LEN           0x0014U   /* 0x15 ?????0x1400~0x1409 */

#define DF_DEV_RUN_MODE_APP                      0x0000U
#define DF_DEV_RUN_MODE_BOOT                      0x0001U

#define DF_PWR_STATE_INIT                        0x0000U
#define DF_PWR_STATE_WAIT                        0x0001U
#define DF_PWR_STATE_ERR                         0x0002U
#define DF_PWR_STATE_RISE                        0x0003U
#define DF_PWR_STATE_DERATE                      0x0004U
#define DF_PWR_STATE_RUN                         0x0005U
#define DF_PWR_STATE_BOOT                        0x00FFU

#pragma pack(1)
typedef struct
{
    uint16_t u16DevType;
    uint16_t u16HardwareVersion;
    uint16_t u16SoftVersion;
    uint16_t u16SysRunTime;
    uint16_t u16DevOnlineStatus;
}_StDevMsg;
#pragma pack()

typedef union
{
    _StDevMsg StDevMsg;
    uint8_t u8Tab[10];
}_uStDevMsg;

#pragma pack(1)
typedef struct
{
    uint16_t u16DevType;
    uint16_t u16HardwareVersion;
    uint16_t u16SoftVersion;
    uint16_t u16SysRunTime;
    uint16_t u16DevOnlineStatus;
    uint16_t u16RunMode;
    uint16_t u16ActiveSlot;
    uint16_t u16SlotAFwVer;
    uint16_t u16SlotBFwVer;
    uint16_t u16PowerState;
}_StDevExtMsg;
#pragma pack()

typedef union
{
    _StDevExtMsg StDevExtMsg;
    uint8_t u8Tab[20];
}_uStDevExtMsg;

#pragma pack(1)
typedef struct
{
    uint8_t u8ID;
    uint8_t u8FunctionCode;
    uint16_t u16StartAddr;
    uint16_t u16Len;
    uint16_t u16DevType;
    uint16_t u16HardVersion;
    uint16_t u16SoftVersion;
    uint16_t u16RuingTimer;
    uint16_t u16DevOnlineStatus;
    uint16_t u16Crc16;
}_StReturnCheckDeviceMsg;
#pragma pack()

typedef union
{
    uint8_t u8Tab[18];
    _StReturnCheckDeviceMsg StReturnCheckDeviceMsg;
}_uReturnCheckDeviceMsg;

#pragma pack(1)
typedef struct
{
    uint8_t u8ID;
    uint8_t u8FunctionCode;
    uint16_t u16StartAddr;
    uint16_t u16Len;
    _StDevExtMsg StDevExtMsg;
    uint16_t u16Crc16;
}_StReturnCheckDeviceExtMsg;
#pragma pack()

typedef union
{
    uint8_t u8Tab[28];
    _StReturnCheckDeviceExtMsg StReturnCheckDeviceExtMsg;
}_uReturnCheckDeviceExtMsg;

extern _uStDevMsg StMasterDevMsg;

uint16_t DevMsg_GetActiveSlotFromVtor(void);
uint16_t DevMsg_GetPowerState(void);
void DevMsg_FillExtInfo(_StDevExtMsg *pExt, uint16_t u16RunMode, uint16_t u16ActiveSlot);
uint8_t DevMsg_ShouldIgnoreReadDuringUpgrade(uint16_t u16RunMode);
void ProcessReadDevMsgRequest(const uint8_t *u8Buf, uint16_t u16Len, uint8_t u8UartCom, uint16_t u16RunMode, uint16_t u16ActiveSlot);
void ReturnCheckVersionExtInquireCmd(uint8_t u8UartCom, uint16_t u16RunMode, uint16_t u16ActiveSlot);

#endif
