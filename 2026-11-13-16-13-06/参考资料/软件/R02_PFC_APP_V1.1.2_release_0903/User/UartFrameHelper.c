#include "UartFrameHelper.h"

#include "crc16.h"

#define DF_UART_READ_DEV_MSG_FUNCTION_CODE  0x15U
#define DF_UART_READ_DEV_MSG_REQ_MIN_LEN    8U
#define DF_UART_READ_DEV_MSG_REQ_FULL_LEN   10U

/**
 * 0x15 读设备信息「请求」定长识别：�?10B(�?DevType) �?8B，避�?8B CRC 误判截断 10B 帧�? */
static uint16_t ReadDevMsg_TryRequestLen(const uint8_t *pBuf, uint16_t u16RxLen)
{
    uint16_t u16Calc;
    uint16_t u16Recv;

    if (u16RxLen >= DF_UART_READ_DEV_MSG_REQ_FULL_LEN)
    {
        u16Calc = Modbus_CalCRC(pBuf, 8U);
        u16Recv = (uint16_t)pBuf[8] | ((uint16_t)pBuf[9] << 8);
        if (u16Calc == u16Recv)
        {
            return DF_UART_READ_DEV_MSG_REQ_FULL_LEN;
        }
    }

    if (u16RxLen >= DF_UART_READ_DEV_MSG_REQ_MIN_LEN)
    {
        u16Calc = Modbus_CalCRC(pBuf, 6U);
        u16Recv = (uint16_t)pBuf[6] | ((uint16_t)pBuf[7] << 8);
        if (u16Calc == u16Recv)
        {
            return DF_UART_READ_DEV_MSG_REQ_MIN_LEN;
        }
    }

    return 0U;
}

uint16_t UartFrame_GetExpectedLen(const uint8_t *pBuf, uint16_t u16RxLen)
{
    uint16_t u16PduLen;
    uint16_t u16ReqLen;

    if (pBuf == NULL || u16RxLen < 6U)
    {
        return 0U;
    }

    if (pBuf[1] == DF_UART_READ_DEV_MSG_FUNCTION_CODE)
    {
        u16ReqLen = ReadDevMsg_TryRequestLen(pBuf, u16RxLen);
        if (u16ReqLen != 0U)
        {
            return u16ReqLen;
        }
    }

    u16PduLen = (uint16_t)((uint16_t)pBuf[4] << 8) | (uint16_t)pBuf[5];
    return (uint16_t)(u16PduLen + 8U);
}

uint8_t UartFrame_IsComplete(const uint8_t *pBuf, uint16_t u16RxLen)
{
    uint16_t u16Expect;

    if (pBuf == NULL || u16RxLen < 6U)
    {
        return 0U;
    }

    u16Expect = UartFrame_GetExpectedLen(pBuf, u16RxLen);
    if (u16Expect == 0U)
    {
        return 0U;
    }

    return (u16RxLen >= u16Expect) ? 1U : 0U;
}
