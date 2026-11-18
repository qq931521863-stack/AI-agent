/******************************************************************************
    FlashMemoryMap.h ? ?? STM32G431 ?? Flash ???Boot / APP_A / APP_B?
    Boot ? xpPFC431 APP ????????MDK scatter ? OCR_RVCT4 ?????????
    ??????????(boot+APPupdate)/User/UpgradeMsgSaveFunctionDf.h ? DF_SLAVE_INTERNAL_*

    ???????Boot ? APP_A ? APP_B ? ?????

    | Keil Target       | ???           | scatter     | ????   | ??   | ?? hex           | ?? |
    | xpPFC431_48_A     | APP_SLOT_A_BUILD | APP_A.sct   | 0x08008000 | 0xB000 | APP_A.hex          | APP A ??Boot ????? |
    | xpPFC431_48_B     | APP_SLOT_B_BUILD | APP_B.sct   | 0x08013000 | 0xB000 | APP_B.hex          | APP B ??OTA ????? |
    | IAP Bootloader    | (?)             | Boot.sct    | 0x08000000 | 0x7800 | IAP Bootloader.hex | Boot 30KB |

    A/B ???????????? APP_SLOT_*_BUILD ???Boot ??????????

    | ??                     | ????   | ??/??  | ?? |
    | Boot                     | 0x08000000 | 0x7800     | Bootloader |
    | ???????           | 0x08007800 | 2KB        | SysStatus |
    | APP_A                    | 0x08008000 | 0xB000     | ??? A |
    | APP_B                    | 0x08013000 | 0xB000     | ??? B |
    | ?????               | 0x0801F000 | 44B        | ProgStatus |
******************************************************************************/
#ifndef _FlashMemoryMap_H_
#define _FlashMemoryMap_H_

#include "stdint.h"

/************************** BootLoader (30KB) **************************/
#define DF_SYS_BOOT_LOADER_START_ADDR                   ((uint32_t)0x08000000U)
#define DF_SYS_BOOT_LOADER_END_ADDR                     ((uint32_t)0x08007800U)
#define DF_SYS_BOOT_LOADER_SIZE_BYTES                   ((uint32_t)0x00007800U)
#define DF_SYS_BOOT_LOADER_BLANKS                       FLASH_BANK_1
#define DF_SYS_BOOT_LOADER_PAGE_NUM                     15U

/************************** ??????? (2KB) **************************/
#define DF_SYS_STATE_MSG_SAVE_START_ADDR                ((uint32_t)0x08007800U)
#define DF_SYS_STATE_MSG_SAVE_END_ADDR                  ((uint32_t)0x08008000U)
#define DF_SYS_STATE_MSG_SAVE_BLANKS                    FLASH_BANK_1
#define DF_SYS_STATE_MSG_SAVE_PAGE_NUM                  1U

/************************** ? APP ? (44K + 44K) **************************/
#define DF_INTERNAL_FLASH_PAGE_SIZE                     ((uint32_t)0x800U)
#define DF_FLASH_END_ADDRESS                            ((uint32_t)0x08020000U)

#define DF_APP_A_START_ADDRESS                          ((uint32_t)0x08008000U)
#define DF_APP_A_END_ADDRESS                            ((uint32_t)0x08013000U)
#define DF_APP_A_SIZE_BYTES                             ((uint32_t)0x0000B000U)
#define DF_APP_A_PAGE_NUM                               22U

#define DF_APP_B_START_ADDRESS                          ((uint32_t)0x08013000U)
#define DF_APP_B_END_ADDRESS                            ((uint32_t)0x0801E000U)
#define DF_APP_B_SIZE_BYTES                             ((uint32_t)0x0000B000U)
#define DF_APP_B_PAGE_NUM                               22U

#define DF_APP_SLOT_SIZE_BYTES                          DF_APP_A_SIZE_BYTES

/* xpPFC431 完整 APP 链接后 bin 约 41KB；低于此值多为截断/错误 scatter，跳转必 HardFault */
#define DF_APP_MIN_BIN_LENGTH_BYTES                     ((uint32_t)34000U)

/************************** ????? / ??? **************************/
#define DF_APP_A_FIRMWARE_MSG_START_ADDRESS             ((uint32_t)0x0801E000U)
#define DF_APP_B_FIRMWARE_MSG_START_ADDRESS             ((uint32_t)0x0801E800U)
#define DF_UPGRADE_TRACK_START_ADDRESS                  ((uint32_t)0x0801F000U)
#define DF_EXCEPTION_LOG_START_ADDRESS                  ((uint32_t)0x0801F800U)
#define DF_RING_LOG_START_ADDRESS                       DF_EXCEPTION_LOG_START_ADDRESS

/************************** ???? **************************/
#define DF_APPLICATION_START_ADDRESS                    DF_APP_A_START_ADDRESS
#define DF_APPLICATION_END_ADDRESS                      DF_APP_A_END_ADDRESS
#define DF_APPLICATION_BLANKS                           FLASH_BANK_1
#define DF_APPLICATION_PAGE_NUM                         DF_APP_A_PAGE_NUM

#define DF_MASTET_A_FIRMWARE_START_ADDRESS              DF_APP_A_START_ADDRESS
#define DF_MASTET_B_FIRMWARE_START_ADDRESS              DF_APP_B_START_ADDRESS
#define DF_MASTER_FIRMWARE_PAGE_NUM                     DF_APP_B_PAGE_NUM
#define DF_MASTET_FIRMWARE_MSG_START_ADDRESS            DF_UPGRADE_TRACK_START_ADDRESS

#endif

