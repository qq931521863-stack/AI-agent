/******************************************************************************
    File name:   UpgradeMsgSaveFunctionDf.c
    Version :   v1.0
    Author  :   C7
    Date    :   20290608
    Technical Support: https://shop252056293.taobao.com/
******************************************************************************/
#include "UpgradeFeatureConfig.h"
#if OTA_UPGRADE_ENABLE

#include "upgrade_app.h"
#include "mx25L1606E.h"
_StProgStatus  StMasterProgStatus;                      // Master upgrade status structure
_utSysStatusMsg utSysStatusMsg;
//-----------------------------------------------------------
//Description    : Save upgrade info to Flash
//Input          : None
//Output         : None
//Author         : C7 20190612
//Note(s)        : None
//-----------------------------------------------------------
void SaveWriteUpgradeFileMsg(_StProgStatus *Prog_Status,uint32_t u32Addr)
{	
    _StProgStatus stVerify;
    UPGRADE_DBG("[SAVE_UP_MSG] addr=0x%08lx into=0x%02x upState=0x%04x upTo=0x%02x user=0x%02x pend=0x%02x cfm=%d rb=%d retry=%d err=0x%04x\r\n",
           (unsigned long)u32Addr,
           Prog_Status->StProgStatusMsg.u8IntoUpgradFlag,
           Prog_Status->StProgStatusMsg.u16UpgradeState,
           Prog_Status->StProgStatusMsg.u8FlashUPtoBlock,
           Prog_Status->StProgStatusMsg.u8FlashUserBlock,
           Prog_Status->StProgStatusMsg.u8PendingBootSlot,
           Prog_Status->StProgStatusMsg.u8BootConfirmState,
           Prog_Status->StProgStatusMsg.u8RollbackCount,
           Prog_Status->StProgStatusMsg.u8BootRetryCount,
           Prog_Status->StProgStatusMsg.u16LastErrorCode);
	FlashSector_Erase(u32Addr);   
	Prog_Status->StProgStatusMsg.u16ProgMsgCRC16Value = Modbus_CalCRC(Prog_Status->u8Buf,sizeof(_StProgStatus)-2);
    UPGRADE_DBG("[SAVE_UP_MSG] crc=0x%04x len=%d raw0..7=%02x %02x %02x %02x %02x %02x %02x %02x\r\n",
           Prog_Status->StProgStatusMsg.u16ProgMsgCRC16Value,
           (int)sizeof(_StProgStatus),
           Prog_Status->u8Buf[0], Prog_Status->u8Buf[1], Prog_Status->u8Buf[2], Prog_Status->u8Buf[3],
           Prog_Status->u8Buf[4], Prog_Status->u8Buf[5], Prog_Status->u8Buf[6], Prog_Status->u8Buf[7]);
    SPI_FLASH_BufferWrite(Prog_Status->u8Buf,u32Addr,sizeof(_StProgStatus));	                    // Write upgrade structure to flash
    FlashRead(stVerify.u8Buf, u32Addr, sizeof(_StProgStatus));
    UPGRADE_DBG("[SAVE_UP_MSG] verify raw0..7=%02x %02x %02x %02x %02x %02x %02x %02x into=0x%02x upState=0x%04x pend=0x%02x cfm=%d rb=%d retry=%d err=0x%04x crc=0x%04x\r\n",
           stVerify.u8Buf[0], stVerify.u8Buf[1], stVerify.u8Buf[2], stVerify.u8Buf[3],
           stVerify.u8Buf[4], stVerify.u8Buf[5], stVerify.u8Buf[6], stVerify.u8Buf[7],
           stVerify.StProgStatusMsg.u8IntoUpgradFlag,
           stVerify.StProgStatusMsg.u16UpgradeState,
           stVerify.StProgStatusMsg.u8PendingBootSlot,
           stVerify.StProgStatusMsg.u8BootConfirmState,
           stVerify.StProgStatusMsg.u8RollbackCount,
           stVerify.StProgStatusMsg.u8BootRetryCount,
           stVerify.StProgStatusMsg.u16LastErrorCode,
           stVerify.StProgStatusMsg.u16ProgMsgCRC16Value);
}

//-----------------------------------------------------------
//Description    : Read upgrade info from Flash
//Input          : None
//Output         : None
//Author         : C7 20190612
//Note(s)        : None
//-----------------------------------------------------------
uint16_t ReadWriteUpgradeFileMsg(_StProgStatus *Prog_Status,uint32_t u32Addr)
{	
	uint16_t u16CRC16 = 0;

    FlashRead(Prog_Status->u8Buf,u32Addr,sizeof(_StProgStatus));	                                                // Read upgrade structure from Flash
    u16CRC16 = Modbus_CalCRC(Prog_Status->u8Buf,sizeof(_StProgStatus)-2);                                          // Calculate CRC check value
    UPGRADE_DBG("[READ_UP_MSG] addr=0x%08lx raw0..7=%02x %02x %02x %02x %02x %02x %02x %02x into=0x%02x upState=0x%04x pend=0x%02x cfm=%d rb=%d retry=%d err=0x%04x storedCrc=0x%04x calcCrc=0x%04x\r\n",
           (unsigned long)u32Addr,
           Prog_Status->u8Buf[0], Prog_Status->u8Buf[1], Prog_Status->u8Buf[2], Prog_Status->u8Buf[3],
           Prog_Status->u8Buf[4], Prog_Status->u8Buf[5], Prog_Status->u8Buf[6], Prog_Status->u8Buf[7],
           Prog_Status->StProgStatusMsg.u8IntoUpgradFlag,
           Prog_Status->StProgStatusMsg.u16UpgradeState,
           Prog_Status->StProgStatusMsg.u8PendingBootSlot,
           Prog_Status->StProgStatusMsg.u8BootConfirmState,
           Prog_Status->StProgStatusMsg.u8RollbackCount,
           Prog_Status->StProgStatusMsg.u8BootRetryCount,
           Prog_Status->StProgStatusMsg.u16LastErrorCode,
           Prog_Status->StProgStatusMsg.u16ProgMsgCRC16Value,
           u16CRC16);
    if(u16CRC16 != Prog_Status->StProgStatusMsg.u16ProgMsgCRC16Value)                                               // CRC check failed, restore to A block default
    {
        UPGRADE_DBG("Upgrade info CRC check failed, restored to default status\r\n");
        Prog_Status->StProgStatusMsg.u8FlashUserBlock = DF_CHS_BLOCK_A;                                             // Switch to A block
        Prog_Status->StProgStatusMsg.u8FlashUPtoBlock = 0xFF;                                                       // No upgrade block specified
        Prog_Status->StProgStatusMsg.u8BlockABinFileValidFlag = DF_UPGRADE_FILE_INVAILE_STATUS;                     // A bin file invalid
        Prog_Status->StProgStatusMsg.u8BlockBBinFileValidFlag = DF_UPGRADE_FILE_INVAILE_STATUS;                     // B bin file invalid
        Prog_Status->StProgStatusMsg.u16UpgradeState = DF_WAIT_DOWNLOAD_UPGRADR_FILE_STATUS;                        // Waiting for upgrade file download
        Prog_Status->StProgStatusMsg.u8PendingBootSlot = DF_UPBLOCK_INVALID;
        Prog_Status->StProgStatusMsg.u8BootConfirmState = DF_BOOT_CONFIRM_STATE_NONE;
        Prog_Status->StProgStatusMsg.u8RollbackCount = 0;
        Prog_Status->StProgStatusMsg.u8BootRetryCount = 0;
        Prog_Status->StProgStatusMsg.u16LastErrorCode = 0x0000;
        /* 运行中禁止写 Flash。RAM 已默认；功率空闲或 INTO 安全关断后再落盘。 */
        return 0x00;
    }
    else                                                                                                            // CRC check pass, verify each state value
    {
          if(Prog_Status->StProgStatusMsg.u16UpgradeState != DF_WAIT_DOWNLOAD_UPGRADR_FILE_STATUS                   // State validity check
             && Prog_Status->StProgStatusMsg.u16UpgradeState != DF_WAIT_INTO_UPGRADR_STATUS
             && Prog_Status->StProgStatusMsg.u16UpgradeState != DF_UPGRADR_FINISH_STATUS
             && Prog_Status->StProgStatusMsg.u16UpgradeState != DF_UPGRADE_FAIL_STATUS
             && Prog_Status->StProgStatusMsg.u16UpgradeState != DF_NOT_UPGRADE_STATUS)
        {
            Prog_Status->StProgStatusMsg.u16UpgradeState = DF_WAIT_DOWNLOAD_UPGRADR_FILE_STATUS;
        }

        if(Prog_Status->StProgStatusMsg.u8BlockABinFileValidFlag != DF_UPGRADE_FILE_INVAILE_STATUS 
            && Prog_Status->StProgStatusMsg.u8BlockABinFileValidFlag != DF_UPGRADE_FILE_VAILE_STATUS)               // A block validity flag check
        {
            Prog_Status->StProgStatusMsg.u8BlockABinFileValidFlag = DF_UPGRADE_FILE_INVAILE_STATUS;
        }
         if(Prog_Status->StProgStatusMsg.u8BlockBBinFileValidFlag != DF_UPGRADE_FILE_INVAILE_STATUS 
             && Prog_Status->StProgStatusMsg.u8BlockBBinFileValidFlag != DF_UPGRADE_FILE_VAILE_STATUS)              // B block validity flag check
        {
            Prog_Status->StProgStatusMsg.u8BlockBBinFileValidFlag = DF_UPGRADE_FILE_INVAILE_STATUS;
        }
        
        if(Prog_Status->StProgStatusMsg.u32BlockABinFileLength == 0xFFFFFFFF || Prog_Status->StProgStatusMsg.u32BlockABinFileLength == 0x0) // A bin file length invalid
        {
            Prog_Status->StProgStatusMsg.u8BlockABinFileValidFlag = DF_UPGRADE_FILE_INVAILE_STATUS;
            Prog_Status->StProgStatusMsg.u32BlockABinFileLength = 0x0000;
        }
        if(Prog_Status->StProgStatusMsg.u32BlockBBinFileLength == 0xFFFFFFFF || Prog_Status->StProgStatusMsg.u32BlockBBinFileLength == 0x0) // B bin file length invalid
        {
            Prog_Status->StProgStatusMsg.u8BlockBBinFileValidFlag = DF_UPGRADE_FILE_INVAILE_STATUS;
            Prog_Status->StProgStatusMsg.u32BlockBBinFileLength = 0x0000;
        }
        
        if(Prog_Status->StProgStatusMsg.u8FlashUserBlock != DF_CHS_BLOCK_A && Prog_Status->StProgStatusMsg.u8FlashUserBlock != DF_CHS_BLOCK_B) // Should only use valid user block
        {
            Prog_Status->StProgStatusMsg.u8BlockABinFileValidFlag = DF_UPGRADE_FILE_INVAILE_STATUS;
            Prog_Status->StProgStatusMsg.u8BlockBBinFileValidFlag = DF_UPGRADE_FILE_INVAILE_STATUS;
            Prog_Status->StProgStatusMsg.u8FlashUserBlock = DF_CHS_BLOCK_A;
        }       
        if(Prog_Status->StProgStatusMsg.u8PendingBootSlot != DF_UPBLOCK_INVALID
            && Prog_Status->StProgStatusMsg.u8PendingBootSlot != DF_CHS_BLOCK_A
            && Prog_Status->StProgStatusMsg.u8PendingBootSlot != DF_CHS_BLOCK_B)
        {
            Prog_Status->StProgStatusMsg.u8PendingBootSlot = DF_UPBLOCK_INVALID;
        }
        if(Prog_Status->StProgStatusMsg.u8BootConfirmState > DF_BOOT_CONFIRM_STATE_TIMEOUT)
        {
            Prog_Status->StProgStatusMsg.u8BootConfirmState = DF_BOOT_CONFIRM_STATE_NONE;
        }
        UPGRADE_DBG("Upgrade info check passed\r\n");
        return 0x01;
    }
}
//----------------------------------------------------------------------
//Description    : Save system status
//Input          : None
//Output         : None
//Author         : C7
//Note(s)        : None
//---------------------------------------------------------------------- 
void SaveSysStatusMsg(uint16_t u16SaveStatus)
{
   utSysStatusMsg.StSysStatus.u16SysUpgradeState = u16SaveStatus;
   utSysStatusMsg.StSysStatus.u16CRC16Value = Modbus_CalCRC(utSysStatusMsg.u8Buf,sizeof(_utSysStatusMsg)-2);

    WriteSysStatusMsg(DF_SYS_STATE_MSG_SAVE_START_ADDR,&utSysStatusMsg,sizeof(_utSysStatusMsg) / 4);	
}
//----------------------------------------------------------------------
//Description    : Read system status
//Input          : None
//Output         : None
//Author         : C7
//Note(s)        : None
//---------------------------------------------------------------------- 
void GetSysStatusMsg(void)
{
    uint16_t u16CRC16;    
    ReadSysStatusMsg(DF_SYS_STATE_MSG_SAVE_START_ADDR,&utSysStatusMsg,sizeof(_utSysStatusMsg));             // Read system status info from Flash
    u16CRC16 = Modbus_CalCRC(utSysStatusMsg.u8Buf,sizeof(_utSysStatusMsg)-2);                               // Calculate CRC16
    if(u16CRC16 == utSysStatusMsg.StSysStatus.u16CRC16Value)                                                // CRC16 check success
    {   
        if(utSysStatusMsg.StSysStatus.u16SysUpgradeState != DF_WAIT_DOWNLOAD_UPGRADR_FILE_STATUS            // System upgrade state value check
            && utSysStatusMsg.StSysStatus.u16SysUpgradeState != DF_UPGRADR_FINISH_STATUS)
        {
            UPGRADE_DBG("Status exception, restored to default\r\n");
            utSysStatusMsg.StSysStatus.u16SysUpgradeState = DF_WAIT_DOWNLOAD_UPGRADR_FILE_STATUS;
            SaveSysStatusMsg(DF_WAIT_DOWNLOAD_UPGRADR_FILE_STATUS);
        }
        UPGRADE_DBG("System status info check passed\r\n");
    }
    else                                                                                                    // CRC16 check failed, restore to default
    {
        SaveSysStatusMsg(DF_WAIT_DOWNLOAD_UPGRADR_FILE_STATUS);
         UPGRADE_DBG("System status info CRC check failed, restored to default\r\n");
    }  
}
//-----------------------------------------------------------
//Description    : Software reset
//Input          : None
//Output         : None
//Author         : C7 20190612
//Note(s)        : None
//-----------------------------------------------------------
void Sys_software_reset(void)
{
   __set_PRIMASK(1); 
    NVIC_SystemReset();
}
//----------------------------------------------------------------------
//Description    : Disable interrupt
//Input          : None
//Output         : None
//Author         : any at 2023/07/20
//Note(s)        : None
//----------------------------------------------------------------------
void DisenabledInterrupt(void)
{
     __set_PRIMASK(1);
}
//----------------------------------------------------------------------
//Description    : Enable interrupt
//Input          : None
//Output         : None
//Author         : C7 20190612
//Note(s)        : None
//----------------------------------------------------------------------
void EnabledInterrupt(void)
{
     __set_PRIMASK(0);
}

#else /* OTA_UPGRADE_ENABLE */

#include "UpgradeMsgSaveFunctionDf.h"

_StProgStatus  StMasterProgStatus;
_utSysStatusMsg utSysStatusMsg;

void SaveWriteUpgradeFileMsg(_StProgStatus *Prog_Status, uint32_t u32Addr)
{
    (void)Prog_Status;
    (void)u32Addr;
}
uint16_t ReadWriteUpgradeFileMsg(_StProgStatus *Prog_Status, uint32_t u32Addr)
{
    (void)Prog_Status;
    (void)u32Addr;
    return 0U;
}
void SaveSysStatusMsg(uint16_t u16SaveStatus) { (void)u16SaveStatus; }
void Sys_software_reset(void) {}
void DisenabledInterrupt(void) {}
void EnabledInterrupt(void) {}

#endif /* OTA_UPGRADE_ENABLE */
