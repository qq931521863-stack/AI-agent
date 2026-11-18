/******************************************************************************
    ???:   UpgradeMsgSaveFunctionDf.h
******************************************************************************/
#ifndef _UpgradeMsgSaveFunctionDf_H
#define _UpgradeMsgSaveFunctionDf_H

#include "stm32g4xx_hal.h"
#include "FlashMemoryMap.h"

#define SWAP16(value) (uint16_t)(((value) << 8) | (((value) >> 8) & 0xFF))
#define SWAP32(value) (uint32_t)((((uint8_t*)&(value))[0] << 24) | (((uint8_t*)&(value))[1] << 16) | (((uint8_t*)&(value))[2] << 8) | ((uint8_t*)&(value))[3])

#define DF_CMD_START_ADDR_OFFSET                                        2
#define DF_CMD_DATA_OFFSET                                              6

#define Df_UPGRADE_DEV_MASTET                                           0x0001
#define Df_UPGRADE_DEV_SLAVE1                                           0x0002
#define Df_UPGRADE_DEV_MERGED_PACK                                      0x0003

#define DF_UART_UPGRADE_ACK_INIT                                        0xFFFF

#define DF_INTO_UPGRADE_MODE_SUCCEED_ACK                                0x0000
#define DF_INTO_UPGRADE_MODE_FAIL_ACK                                   0x0001
#define DF_INTO_UPGRADE_MODE_FAIL_SAFE_TIMEOUT                          0x0002
#define DF_INTO_UPGRADE_MODE_FAIL_FAULT_LOCKED                          0x0003

#define DF_START_UPGRADE_SUCCEED_ACK                                    0x0000
#define DF_START_UPGRADE_PACK_DATA_NUM_OVERSIZE_ACK                     0x0001
#define DF_START_UPGRADE_BIN_FILE_OVERSIZE_ACK                          0x0002
#define DF_START_UPGRADE_OTHER_ERROR_ACK                                0x0004

#define DF_FILE_TRANSFER_FRAME_DATA_WRITE_FLASH_SUCCEED_ACK             0x0000
#define DF_FILE_TRANSFER_FRAME_DATA_WRITE_FLASH_ERROR_ACK               0x0001
#define DF_FILE_TRANSFER_FRAME_DATA_LEN_ERROR_ACK                       0x0002
#define DF_FRAME_DATA_HAVE_BEEN_WRITE_FLASH_ACK                         0x0004
#define DF_FRAME_DATA_SER_NUM_ERROR_ACK                                 0x0008
#define DF_FILE_TRANSFER_FRAME_DATA_OTHER_ERROR_ACK                     0x0010

#define DF_FILE_TRANSFER_END_BIN_FILE_REC_SUCCEED_ACK                   0x0000
#define DF_FILE_TRANSFER_END_FILE_TRANSFER_END_BIN_LEN_ERORR_ACK        0x0002
#define DF_FILE_TRANSFER_END_BIN_CRC_ERROR_ACK                          0x0001

#define DF_INTEGER_K_SIZE_VALUE                         1024
#define DF_INTEGER_2K_SIZE_VALUE                        2048

#define DF_INQUIRE_UPGRADE_UPGRADING_STATUS_ACK                         0x0001
#define DF_INQUIRE_UPGRADE_UPGRADE_SUCCEED_ACK                          0x0000
#define DF_INQUIRE_UPGRADE_UPGRADE_FAIL_ACK                             0x0002

#define DF_INTO_UPGRADEMODE_STEP                                        0x01
#define DF_START_UPGRADE_STEP                                           0x02
#define DF_UPGRADE_FILE_TRANSFER_STEP                                   0x03
#define DF_UPGRADE_FILE_TRANSFER_END_STEP                               0x04
#define DF_UPGRADE_SCHEDULE_INQUIRE_STEP                                0x05

#define DF_WAIT_DOWNLOAD_UPGRADR_FILE_STATUS                            0x5AB1
#define DF_WAIT_INTO_UPGRADR_STATUS                                     0x5AB2
#define DF_UPGRADR_FINISH_STATUS                                        0x5AB3
#define DF_UPGRADE_FAIL_STATUS                                          0x5AB4
#define DF_NOT_UPGRADE_STATUS                                           0x5AB5

#define DF_ERASE_FLASH_WAIT_INTO_ERASE_STATUS                          0x00
#define DF_ERASE_FLASH_ERASING_STATUS                                  0x01
#define DF_ERASE_FLASH_ERASE_FINISH_STATUS                             0x02

#define DF_INTO_UPGRADE_MODE_CMD                                        0x55AA
#define DF_UPGRADE_START_CMD                                            0x55AA

#define DF_UPGRADE_FILE_VAILE_STATUS                                    0xAA
#define DF_UPGRADE_FILE_INVAILE_STATUS                                  0x55

#define DF_CHS_BLOCK_A                                                  0x55
#define DF_CHS_BLOCK_B                                                  0xAA
#define DF_UPBLOCK_INVALID                                              0xFF

#define DF_SYS_NOT_UPGRADE_STATUS                                       0xAA
#define DF_SYS_WAIT_DOWNLOAD_FIRMWARE_STATUS                            0x01
#define DF_SYS_DOWNLOAD_FIRMWARE_FINISH_STATUS                          0x02
#define DF_SYS_DOWNLOAD_FIRMWARE_FILL_STATUS                            0x03
#define DF_SYS_VERIFY_NEW_FIRMWARE_FILL_STATUS                          0x04
#define DF_SYS_WRITE_NEW_FIRMWARE_FINISH_STATUS                         0x05
#define DF_SYS_WRITE_NEW_FIRMWARE_FILL_STATUS                           0x06

#define DF_BOOT_CONFIRM_STATE_NONE                                     0x00
#define DF_BOOT_CONFIRM_STATE_PENDING                                  0x01
#define DF_BOOT_CONFIRM_STATE_CONFIRMED                                0x02
#define DF_BOOT_CONFIRM_STATE_TIMEOUT                                  0x03

#define DF_BOOT_CONFIRM_RETRY_MAX                                      3

#pragma pack(1)
typedef struct
{
    uint8_t  u8UpgradeFlag;
    uint8_t u8BlockABinFileValidFlag;
    uint8_t u8BlockBBinFileValidFlag;
    uint8_t  u8FlashUPtoBlock;
    uint8_t  u8FlashUserBlock;
    uint16_t u16UpgradeState;
    uint32_t u32BlockABinFileLength;
    uint32_t u32BlockBBinFileLength;
    uint16_t u16BlockABinFlieCrc16;
    uint16_t u16BlockBBinFlieCrc16;
    uint32_t u32BlockABinFlieCrc32;
    uint32_t u32BlockBBinFlieCrc32;
    uint16_t u16BlockAHardwareVersion;
    uint16_t u16BlockAFirmwareVersion;
    uint16_t u16BlockBHardwareVersion;
    uint16_t u16BlockBFirmwareVersion;
    uint8_t  u8IntoUpgradFlag;
    uint8_t u8PendingBootSlot;
    uint8_t u8BootConfirmState;
    uint8_t u8RollbackCount;
    uint8_t u8BootRetryCount;
    uint16_t u16LastErrorCode;
    uint16_t u16ProgMsgCRC16Value;
}_StProgStatusMsg;
#pragma pack()

typedef union
{
    _StProgStatusMsg  StProgStatusMsg;
    uint32_t  u32DataTab[11];
    uint8_t u8Buf[44];
}_StProgStatus;

#pragma pack(1)
typedef struct
{
    uint8_t  u8SysStateFlag;
    uint16_t u16SysUpgradeState;
    uint8_t u8ReservedBuf[11];
    uint16_t u16CRC16Value;
}_StSysStatus;
#pragma pack()

typedef struct
{
    uint32_t  u32DataTab[4];
}_StSysStatusBuf;

typedef union
{
    _StSysStatusBuf  StSysStatusBuf;
    _StSysStatus    StSysStatus;
    uint8_t u8Buf[16];
    uint16_t u16Buf[8];
}_utSysStatusMsg;

extern _utSysStatusMsg utSysStatusMsg;
extern _StProgStatus  StMasterProgStatus;

extern void SaveWriteUpgradeFileMsg(_StProgStatus *Prog_Status, uint32_t u32Addr);
extern uint16_t ReadWriteUpgradeFileMsg(_StProgStatus *Prog_Status, uint32_t u32Addr);
extern void Sys_software_reset(void);
extern void DisenabledInterrupt(void);
extern void EnabledInterrupt(void);
extern void SaveSysStatusMsg(uint16_t u16SaveStatus);
extern void GetSysStatusMsg(void);
extern uint8_t ProgStatus_ApplyFlashScan(_StProgStatus *Prog_Status);

#endif
