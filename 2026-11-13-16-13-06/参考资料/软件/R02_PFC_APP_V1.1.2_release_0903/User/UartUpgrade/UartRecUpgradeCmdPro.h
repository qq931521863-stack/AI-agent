/******************************************************************************
    UartRecUpgradeCmdPro.h — APP/Boot 共用升级帧结构体与常量
******************************************************************************/
#ifndef _UartRecUpgradeCmdPro_H
#define _UartRecUpgradeCmdPro_H

#include "stm32g4xx_hal.h"
#include "UpgradeMsgSaveFunctionDf.h"

#pragma pack(1)
typedef struct
{
    uint16_t u16IntoUpgradeMode;
    uint16_t u16UpgradeDevType;
}_StIntoUpgradeModeMsg;
#pragma pack()

typedef union
{
    uint8_t u8Tab[4];
    _StIntoUpgradeModeMsg StIntoUpgradeModeMsg;
}_uInitUpgradeModeMsg;

#pragma pack(1)
typedef struct
{
    uint16_t u16StartAddr;
    uint16_t u16Len;
}_StIGetStartAddrMsg;
#pragma pack()

typedef union
{
    uint8_t u8Tab[4];
    _StIGetStartAddrMsg StIGetStartAddrMsg;
}_uGetStartAddrMsg;

#pragma pack(1)
typedef struct
{
    uint8_t u8ID;
    uint8_t u8FunctionCode;
    uint16_t u16StartAddr;
    uint16_t u16Len;
    uint16_t u16DevType;
    uint16_t u16UpgradeStatus;
    uint16_t u16UpgradeSchedule;
    uint16_t u16Reson;
    uint16_t u16Crc16;
}_StReturnUpgradeScheduleMsg;
#pragma pack()

typedef union
{
    uint8_t u8Tab[14];
    _StReturnUpgradeScheduleMsg StReturnUpgradeScheduleMsg;
}_uReturnUpgradeScheduleMsg;

#pragma pack(1)
typedef struct
{
    uint8_t u8ID;
    uint8_t u8FunctionCode;
    uint16_t u16StartAddr;
    uint16_t u16Len;
    uint16_t u16ReturnData;
    uint16_t u16FrameMaxByte;
    uint16_t u16Crc16;
}_StReturnUpgradeStartMsg;
#pragma pack()

typedef union
{
    uint8_t u8Tab[12];
    _StReturnUpgradeStartMsg StReturnUpgradeStartMsg;
}_uReturnUpgradeStartMsg;

#pragma pack(1)
typedef struct
{
    uint8_t u8ID;
    uint8_t u8FunctionCode;
    uint16_t u16StartAddr;
    uint16_t u16Len;
    uint16_t u16ReturnData;
    uint16_t u16Crc16;
}_StReturnIntoUpgradeModeMsg;
#pragma pack()

typedef union
{
    uint8_t u8Tab[10];
    _StReturnIntoUpgradeModeMsg StReturnIntoUpgradeModeMsg;
}_uReturnIntoUpgradeModeMsg;

#define DF_UART_UPGRADE_FUNCTION_CODE 0x10
#define DF_INTO_UPGRADEMODE_START_ADDR 0x8500
#define DF_START_UPGRADE_START_ADDR 0x8305
#define DF_UPGRADE_FILE_TRANSFER_START_ADDR 0x8320
#define DF_UPGRADE_FILE_TRANSFER_END_START_ADDR 0x9520
#define DF_UPGRADE_SCHEDULE_INQUIRE_START_ADDR 0x9530

#define DF_INTO_UPGRADEMODE_MODE_DATA_LEN 0x0004
#define DF_INTO_UPGRADEMODE_MODE_CMD 0x55AA
#define DF_START_UPGRADE_CMD_DATA_LEN 0x000C
#define DF_UPGRADE_FILE_TRANSFER_END_DATA_LEN 0x0002
#define DF_UPGRADE_FILE_TRANSFER_END_CMD 0x55AA
#define DF_UPGRADE_SCHEDULE_INQUIRE_DATA_LEN 0x0002

#define DF_RETURN_INTO_UPGRADEMODE_MODE_DATA_LEN 0x0002
#define DF_RETURN_START_UPGRADE_DATA_LEN 0x0004
#define DF_RETURN_UPGRADE_FILE_TRANSFER_DATA_LEN 0x0002
#define DF_RETURN_UPGRADE_FILE_TRANSFER_END_DATA_LEN 0x0002
#define DF_RETURN_UPGRADE_SCHEDULE_INQUIRE_DATA_LEN 0x0008

#define DF_UPGRADE_FILE_TRANSFER_PACK_DATA_MAC_LEN 1024
#define DF_UART1_REC_ADDR 0x01

#endif
