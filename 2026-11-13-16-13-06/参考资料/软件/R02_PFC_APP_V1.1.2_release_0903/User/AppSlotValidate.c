#include "UpgradeFeatureConfig.h"
#if OTA_UPGRADE_ENABLE

#include "upgrade_app.h"
#include "AppSlotValidate.h"

uint8_t AppSlot_GetBaseFromBlock(uint8_t u8Block, uint32_t *pBase)
{
    if (pBase == NULL)
    {
        return 1U;
    }
    if (u8Block == DF_CHS_BLOCK_B)
    {
        *pBase = DF_APP_B_START_ADDRESS;
        return 0U;
    }
    if (u8Block == DF_CHS_BLOCK_A)
    {
        *pBase = DF_APP_A_START_ADDRESS;
        return 0U;
    }
    return 1U;
}

uint8_t AppSlot_ImageValidateAtAddr(uint32_t u32SlotBase, uint32_t u32ImageAddr)
{
    uint32_t u32Sp;
    uint32_t u32Reset;

    u32Sp = *(__IO uint32_t *)u32ImageAddr;
    u32Reset = *(__IO uint32_t *)(u32ImageAddr + 4U);
    if ((u32Sp & 0x2FFE0000U) != 0x20000000U)
    {
        return 1U;
    }
    if (u32Reset < u32SlotBase || u32Reset >= (u32SlotBase + DF_APP_SLOT_SIZE_BYTES))
    {
        return 1U;
    }
    return 0U;
}

#else /* OTA_UPGRADE_ENABLE */

#include "AppSlotValidate.h"
#include "FlashMemoryMap.h"

uint8_t AppSlot_GetBaseFromBlock(uint8_t u8Block, uint32_t *pBase)
{
    (void)u8Block;
    (void)pBase;
    return 1U;
}
uint8_t AppSlot_ImageValidateAtAddr(uint32_t u32SlotBase, uint32_t u32ImageAddr)
{
    (void)u32SlotBase;
    (void)u32ImageAddr;
    return 1U;
}

#endif /* OTA_UPGRADE_ENABLE */
