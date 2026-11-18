/******************************************************************************
    ?????:   mx25L1606E.c
    ??? :   v1.0
    ????  :   ?????? 
    ????????20290608
    ??????????https://shop252056293.taobao.com/
******************************************************************************/
#include "mx25L1606E.h"
#include "flash_internal.h"
#include "UpgradeDebugLog.h"
#include <string.h> 
#if OTA_UPGRADE_ENABLE
#define SPI_FLASH_PageSize              256
#define SPI_FLASH_PerWritePageSize      256

#define FLASH_CS_High()   	  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_6,GPIO_PIN_SET);
#define FLASH_CS_Low()     	  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_6,GPIO_PIN_RESET);
#define DUMMY_BYTE  0XFF
void FlashGet_Info(flashInfoTypedef * pFlashInfo);
void FlashGet_ElectronicInfo(flashInfoTypedef * pFlashInfo);
static HAL_StatusTypeDef InternalFlashWriteBytes(uint32_t write_addr, const uint8_t *buffer, uint32_t length);

uint8_t Flash_SPI_ReadWriteByte(uint8_t TxData)
{
    (void)TxData;
    return 0xFFU;
}

static HAL_StatusTypeDef InternalFlashWriteBytes(uint32_t write_addr, const uint8_t *buffer, uint32_t length)
{
    HAL_StatusTypeDef status = HAL_OK;

    if (length == 0U)
    {
        return HAL_OK;
    }

    HAL_FLASH_Unlock();
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS | FLASH_FLAG_EOP);
    while (length > 0U)
    {
        uint32_t aligned_addr = write_addr & ~0x7UL;
        uint32_t offset = write_addr - aligned_addr;
        uint32_t chunk = 8U - offset;
        uint8_t double_word_buf[8];
        uint64_t double_word_value = 0;
        uint32_t i;

        if (chunk > length)
        {
            chunk = length;
        }

        for (i = 0; i < 8U; i++)
        {
            double_word_buf[i] = *(__IO uint8_t *)(aligned_addr + i);
        }

        memcpy(&double_word_buf[offset], buffer, chunk);
        memcpy(&double_word_value, double_word_buf, sizeof(double_word_value));

        {
            uint32_t primask = __get_PRIMASK();
            __disable_irq();
        __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS | FLASH_FLAG_EOP);
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, aligned_addr, double_word_value);
            if (primask == 0U)
            {
                __enable_irq();
            }
        }
        if (status != HAL_OK)
        {
            break;
        }

        write_addr += chunk;
        buffer += chunk;
        length -= chunk;
    }

    HAL_FLASH_Lock();
    return status;
}


 /**
  * @brief  ??FLASH???????????????????????????????????????
  * @param	pBuffer???????????????
  * @param  WriteAddr????????
  * @param  NumByteToWrite?????????????
  * @retval ??
  */
void SPI_FLASH_BufferWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
    HAL_StatusTypeDef st = InternalFlashWriteBytes(WriteAddr, pBuffer, NumByteToWrite);
    UPGRADE_DBG("[IF_WRITE] addr=0x%08lx len=%u hal=%d err=0x%08lx\r\n",
           (unsigned long)WriteAddr,
           (unsigned int)NumByteToWrite,
           (int)st,
           (unsigned long)HAL_FLASH_GetError());
}

//------------------------------------------------------------------------------
//Description    : ????????
//Input          : None
//Output         : None
//Author         : C7
//Note(s)        : None
//------------------------------------------------------------------------------
void FlashSector_Erase(uint32_t sectorAddr)
{
    uint32_t ret;
    uint8_t b0, b1, b2, b3, b4, b5, b6, b7;
    ret = FLASH_If_Erase(sectorAddr, 1, FLASH_BANK_1);
    b0 = *(__IO uint8_t *)(sectorAddr + 0);
    b1 = *(__IO uint8_t *)(sectorAddr + 1);
    b2 = *(__IO uint8_t *)(sectorAddr + 2);
    b3 = *(__IO uint8_t *)(sectorAddr + 3);
    b4 = *(__IO uint8_t *)(sectorAddr + 4);
    b5 = *(__IO uint8_t *)(sectorAddr + 5);
    b6 = *(__IO uint8_t *)(sectorAddr + 6);
    b7 = *(__IO uint8_t *)(sectorAddr + 7);
    UPGRADE_DBG("[IF_ERASE] addr=0x%08lx ret=%lu err=0x%08lx raw0..7=%02x %02x %02x %02x %02x %02x %02x %02x\r\n",
           (unsigned long)sectorAddr,
           (unsigned long)ret,
           (unsigned long)HAL_FLASH_GetError(),
           b0, b1, b2, b3, b4, b5, b6, b7);
}
/*-------------------------------------------------------------------------------------
  * @brief  ???FLASH???? 
  * @param 	ptr?????????????????
  * @param   addr????????
  * @param   len????????????  (????????)
  * @retval ??
------------------------------------------------------------------------------------------*/
void FlashRead(uint8_t *ptr,uint32_t addr,uint32_t len)
{
    memcpy(ptr, (const void *)addr, len);
}
#endif


