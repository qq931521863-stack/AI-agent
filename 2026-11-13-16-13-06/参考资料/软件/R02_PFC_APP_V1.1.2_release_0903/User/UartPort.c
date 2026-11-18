#include "UartPort.h"
#include "MODBUS_SLAVE.h"
#include "Uart_Rec_Analysis.h"
#include "UpgradeFeatureConfig.h"
#if OTA_UPGRADE_ENABLE
#include "UartRecUpgradeCmdPro.h"
#include "InquireDevMsg.h"
#endif
#include "UartFrameHelper.h"
#include "PfcSafeAppBridge.h"
#include <string.h>

static uint8_t s_uartRxDmaBuf[UART_PORT_RX_DMA_LEN];
static uint8_t s_uartTxDmaBuf[256];

#define UART_TX_WAIT_LOOPS 100000U

static void UartPort_RxDmaStart(uint8_t *pu8Buf, uint16_t u16Len)
{
	huart1.Instance->CR3 &= 0xFFFFFFBF;
	__HAL_DMA_DISABLE(&hdma_usart1_rx);
	hdma_usart1_rx.Instance->CNDTR = u16Len;
	hdma_usart1_rx.Instance->CPAR = (uint32_t)&huart1.Instance->RDR;
	hdma_usart1_rx.Instance->CMAR = (uint32_t)pu8Buf;
	__HAL_DMA_ENABLE(&hdma_usart1_rx);
	huart1.Instance->CR3 |= 0x40U;
}

static void UartPort_WaitTxComplete(void)
{
    uint32_t u32Wait = 0U;

    while (huart1.gState != HAL_UART_STATE_READY)
    {
        if (++u32Wait > UART_TX_WAIT_LOOPS)
        {
            (void)HAL_UART_DMAStop(&huart1);
            huart1.gState = HAL_UART_STATE_READY;
            break;
        }
    }
    u32Wait = 0U;
    while (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TC) == RESET)
    {
        if (++u32Wait > UART_TX_WAIT_LOOPS)
        {
            break;
        }
    }
    __HAL_UART_CLEAR_FLAG(&huart1, UART_FLAG_TC);
}

void UartPort_RestartRxDma(void)
{
    UartPort_RxDmaStart(s_uartRxDmaBuf, UART_PORT_RX_DMA_LEN);
}

void UartPort_Init(void)
{
    memset(s_uartRxDmaBuf, 0, sizeof(s_uartRxDmaBuf));
    __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);
    UartPort_RxDmaStart(s_uartRxDmaBuf, UART_PORT_RX_DMA_LEN);
}

static uint8_t UartPort_IsR02ModbusTraffic(const uint8_t *pu8Data, uint16_t u16Len)
{
    if (pu8Data == NULL || u16Len < 2U)
    {
        return 0U;
    }
    /* LLC<->PFC 业务帧：llc_id + status + float V/I + CRC */
    if (pu8Data[0] == llc_id && u16Len >= 12U)
    {
        return 1U;
    }
    return 0U;
}

static uint8_t UartPort_IsIapTraffic(const uint8_t *pu8Data, uint16_t u16Len)
{
#if !OTA_UPGRADE_ENABLE
    (void)pu8Data;
    (void)u16Len;
    return 0U;
#else
    if (pu8Data == NULL || u16Len < 2U)
    {
        return 0U;
    }
    if (pu8Data[0] != DF_UART1_REC_ADDR)
    {
        return 0U;
    }
    if (pu8Data[1] == DF_UART_UPGRADE_FUNCTION_CODE
        || pu8Data[1] == DF_UART_READ_DEV_MSG_FUNCTION_CODE)
    {
        return 1U;
    }
    return 0U;
#endif
}

static void UartPort_HandleIapChunk(const uint8_t *pu8Data, uint16_t u16Len)
{
#if !OTA_UPGRADE_ENABLE
    (void)pu8Data;
    (void)u16Len;
    return;
#else
    if (pu8Data == NULL || u16Len == 0U)
    {
        return;
    }
    if (AppIntoUpgrade_IsBusy() != 0U)
    {
        return;
    }
    if (StUart1RxMsg.u8RecFinshFlag != 0U)
    {
        return;
    }

    if (StUart1RxMsg.u16DataLen == 0U
        && (pu8Data[0] != DF_UART1_REC_ADDR))
    {
        return;
    }

    if ((StUart1RxMsg.u16DataLen + u16Len) >= sizeof(StUart1RxMsg.u8DataTable))
    {
        StUart1RxMsg.u8RecFinshFlag = 0U;
        StUart1RxMsg.u16DataLen = 0U;
        return;
    }

    memcpy(StUart1RxMsg.u8DataTable + StUart1RxMsg.u16DataLen, pu8Data, u16Len);
    StUart1RxMsg.u16DataLen = (uint16_t)(StUart1RxMsg.u16DataLen + u16Len);

    if (UartFrame_IsComplete(StUart1RxMsg.u8DataTable, StUart1RxMsg.u16DataLen))
    {
        StUart1RxMsg.u8RecFinshFlag = 0x01U;
    }
    else if (StUart1RxMsg.u16DataLen > 0U
             && UartFrame_GetExpectedLen(StUart1RxMsg.u8DataTable, StUart1RxMsg.u16DataLen) == 0U)
    {
        StUart1RxMsg.u8RecFinshFlag = 0U;
        StUart1RxMsg.u16DataLen = 0U;
    }
#endif
}

void UartPort_HandleIdleRx(const uint8_t *pu8Data, uint16_t u16Len)
{
    if (pu8Data == NULL || u16Len == 0U)
    {
        return;
    }

    if (UartPort_IsR02ModbusTraffic(pu8Data, u16Len))
    {
        Modbus_Slave_Rx((uint8_t *)pu8Data, u16Len);
        return;
    }

#if OTA_UPGRADE_ENABLE
    if (UartPort_IsIapTraffic(pu8Data, u16Len) || StUart1RxMsg.u16DataLen > 0U)
    {
        UartPort_HandleIapChunk(pu8Data, u16Len);
    }
#endif
}

void UartPort_ProcessIdleIrq(void)
{
    uint16_t u16Len;
    uint32_t temp;

    temp = hdma_usart1_rx.Instance->CNDTR;
    huart1.Instance->CR3 &= 0xFFFFFFBF;
    __HAL_DMA_DISABLE(&hdma_usart1_rx);
    u16Len = (uint16_t)(UART_PORT_RX_DMA_LEN - temp);
    if (u16Len > 0U)
    {
        UartPort_HandleIdleRx(s_uartRxDmaBuf, u16Len);
    }
    UartPort_RestartRxDma();
}

void Uart1DMASendFunction(uint8_t *u8Buf, uint16_t u16len)
{
    if (u8Buf == NULL || u16len == 0U || u16len > sizeof(s_uartTxDmaBuf))
    {
        return;
    }

    UartPort_WaitTxComplete();
    memcpy(s_uartTxDmaBuf, u8Buf, u16len);
    SendData_API(s_uartTxDmaBuf, u16len);
    UartPort_WaitTxComplete();
}
