#include "upgrade_app.h"
#include "HwConfig.h"
#include "FlashMemoryMap.h"

#if OTA_UPGRADE_ENABLE

#if defined(APP_SLOT_B_BUILD)
#define DF_APP_IMAGE_BASE_ADDR     DF_APP_B_START_ADDRESS
#else
#define DF_APP_IMAGE_BASE_ADDR     DF_APP_A_START_ADDRESS
#endif

static uint8_t s_u8MetaLoaded;
static volatile uint8_t s_u8MetaFlashPending;

void UpgradeApp_MarkMetaDirty(void)
{
    s_u8MetaFlashPending = 1U;
}

void UpgradeApp_ClearMetaDirty(void)
{
    s_u8MetaFlashPending = 0U;
}

static void sys_nvic_set_vector_table(uint32_t baseaddr, uint32_t offset)
{
    SCB->VTOR = baseaddr | (offset & (uint32_t)0xFFFFFE00U);
}

static uint8_t AppVtorInAppSlotRange(uint32_t u32Vtor)
{
    if (u32Vtor >= DF_APP_A_START_ADDRESS && u32Vtor < DF_APP_A_END_ADDRESS)
    {
        return 1U;
    }
    if (u32Vtor >= DF_APP_B_START_ADDRESS && u32Vtor < DF_APP_B_END_ADDRESS)
    {
        return 1U;
    }
    return 0U;
}

static void AppEnsureVectorTable(void)
{
    uint32_t u32Vtor = SCB->VTOR;

    if (AppVtorInAppSlotRange(u32Vtor) != 0U)
    {
        return;
    }
    if (u32Vtor != DF_APP_IMAGE_BASE_ADDR)
    {
        sys_nvic_set_vector_table(FLASH_BASE, (DF_APP_IMAGE_BASE_ADDR - FLASH_BASE));
    }
}

static uint8_t AppGetCurrentSlot(void)
{
    uint32_t u32VtorAddr = SCB->VTOR;
    if (u32VtorAddr >= DF_APP_B_START_ADDRESS && u32VtorAddr < DF_APP_B_END_ADDRESS)
    {
        return DF_CHS_BLOCK_B;
    }
    return DF_CHS_BLOCK_A;
}

/**
 * 数字电源约束：仅当功率级未使能时才允许片内擦写。
 * Rise/Run/Derate 或 PFC_ok=1 时禁止写，避免快环黑窗。
 */
static uint8_t AppMeta_IsPowerStageIdle(void)
{
    if (DataFlowFace.PFC_ok != 0U)
    {
        return 0U;
    }
    if (DataFlowFace.PwmSta != 0U)
    {
        return 0U;
    }
    if (DataFlowFace.RunState == Rise
        || DataFlowFace.RunState == Run
        || DataFlowFace.RunState == Derate)
    {
        return 0U;
    }
    /* Init / Wait / Err 且驱动未开：可短时擦写元数据 */
    return 1U;
}

static void AppLoadMetaOnce(void)
{
    uint16_t u16Ok;

    if (s_u8MetaLoaded != 0U)
    {
        return;
    }

    u16Ok = ReadWriteUpgradeFileMsg(&StMasterProgStatus, DF_MASTET_FIRMWARE_MSG_START_ADDRESS);
    s_u8MetaLoaded = 1U;
    if (u16Ok == 0U)
    {
        /* CRC 失败只恢复 RAM，运行中不写 Flash */
        UpgradeApp_MarkMetaDirty();
        UPGRADE_DBG("[APP_META] CRC fail -> RAM default, wait power-idle flush\r\n");
    }
}

static void AppSyncFlashUserBlockFromVtor(void)
{
    uint8_t u8CurSlot = AppGetCurrentSlot();
    uint8_t u8Pending;

    AppLoadMetaOnce();
    u8Pending = StMasterProgStatus.StProgStatusMsg.u8PendingBootSlot;

    if (StMasterProgStatus.StProgStatusMsg.u8BootConfirmState == DF_BOOT_CONFIRM_STATE_PENDING
        && (u8Pending == DF_CHS_BLOCK_A || u8Pending == DF_CHS_BLOCK_B))
    {
        if (u8CurSlot == u8Pending)
        {
            if (StMasterProgStatus.StProgStatusMsg.u8FlashUserBlock != u8CurSlot)
            {
                StMasterProgStatus.StProgStatusMsg.u8FlashUserBlock = u8CurSlot;
                UpgradeApp_MarkMetaDirty();
            }
        }
        return;
    }

    if (StMasterProgStatus.StProgStatusMsg.u8FlashUserBlock != u8CurSlot)
    {
        StMasterProgStatus.StProgStatusMsg.u8FlashUserBlock = u8CurSlot;
        UpgradeApp_MarkMetaDirty();
    }
}

static void AppBootConfirmIfNeeded(void)
{
    uint8_t u8CurSlot = AppGetCurrentSlot();

    AppLoadMetaOnce();
    if (StMasterProgStatus.StProgStatusMsg.u8BootConfirmState == DF_BOOT_CONFIRM_STATE_PENDING
        && StMasterProgStatus.StProgStatusMsg.u8PendingBootSlot == u8CurSlot)
    {
        StMasterProgStatus.StProgStatusMsg.u8BootConfirmState = DF_BOOT_CONFIRM_STATE_CONFIRMED;
        StMasterProgStatus.StProgStatusMsg.u8PendingBootSlot = DF_UPBLOCK_INVALID;
        StMasterProgStatus.StProgStatusMsg.u8BootRetryCount = 0;
        StMasterProgStatus.StProgStatusMsg.u16LastErrorCode = 0x0000;
        UpgradeApp_MarkMetaDirty();
        UPGRADE_DBG("[APP_META] boot confirm RAM only, wait power-idle flush\r\n");
    }
}

static void SysDevMsgInit(void)
{
    StMasterDevMsg.StDevMsg.u16DevOnlineStatus = 0x0001;
    StMasterDevMsg.StDevMsg.u16DevType = Df_UPGRADE_DEV_SLAVE1;
    StMasterDevMsg.StDevMsg.u16HardwareVersion = 76;
    StMasterDevMsg.StDevMsg.u16SoftVersion = 103;
    StMasterDevMsg.StDevMsg.u16SysRunTime = 0;
}

void UpgradeAppEarlyInit(void)
{
    AppEnsureVectorTable();
}

void UpgradeAppInit(void)
{
    AppEnsureVectorTable();
    UpgradeDebugLog_Init();
    /* 与可用基线一致：先开串口；元数据只读/标 dirty，启动不写 Flash */
    UartPort_Init();
    SysDevMsgInit();
    s_u8MetaLoaded = 0U;
    s_u8MetaFlashPending = 0U;
    AppSyncFlashUserBlockFromVtor();
    AppBootConfirmIfNeeded();
}

/**
 * 仅在功率级空闲时落盘元数据（BootConfirm / CRC 默认等）。
 * 禁止定时盲写；Rise/Run 期间绝不擦写。INTO 路径仍自行 SaveWrite。
 */
void UpgradeApp_DeferredMetaFlush(void)
{
    if (s_u8MetaFlashPending == 0U)
    {
        return;
    }
    if (AppIntoUpgrade_IsBusy() != 0U)
    {
        return;
    }
    if (AppMeta_IsPowerStageIdle() == 0U)
    {
        return;
    }

    s_u8MetaFlashPending = 0U;
    UPGRADE_DBG("[APP_META] power-idle flush run=%u pfc_ok=%u pwm=%u\r\n",
           (unsigned)DataFlowFace.RunState,
           (unsigned)DataFlowFace.PFC_ok,
           (unsigned)DataFlowFace.PwmSta);
    SaveWriteUpgradeFileMsg(&StMasterProgStatus, DF_MASTET_FIRMWARE_MSG_START_ADDRESS);
}

#else /* OTA_UPGRADE_ENABLE */

#if defined(APP_SLOT_B_BUILD)
#define DF_APP_IMAGE_BASE_ADDR     DF_APP_B_START_ADDRESS
#else
#define DF_APP_IMAGE_BASE_ADDR     DF_APP_A_START_ADDRESS
#endif

static void AppEnsureVectorTableDisabled(void)
{
    uint32_t u32Vtor = SCB->VTOR;

    if (u32Vtor >= DF_APP_A_START_ADDRESS && u32Vtor < DF_APP_A_END_ADDRESS)
    {
        return;
    }
    if (u32Vtor >= DF_APP_B_START_ADDRESS && u32Vtor < DF_APP_B_END_ADDRESS)
    {
        return;
    }
    if (u32Vtor != DF_APP_IMAGE_BASE_ADDR)
    {
        SCB->VTOR = DF_APP_IMAGE_BASE_ADDR;
    }
}

void UpgradeAppEarlyInit(void)
{
#if defined(APP_SLOT_A_BUILD) || defined(APP_SLOT_B_BUILD)
    AppEnsureVectorTableDisabled();
#endif
}

void UpgradeAppInit(void) {}

#endif /* OTA_UPGRADE_ENABLE */
