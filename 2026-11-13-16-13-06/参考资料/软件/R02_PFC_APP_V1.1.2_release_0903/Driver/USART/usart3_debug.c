/******************************************************************************
    usart3_debug.c — USART3 PB9 TX only，printf 重定向
    与 USART1(PB6/PB7) IAP/LLC 通信无关
******************************************************************************/
#include "UpgradeFeatureConfig.h"
#if OTA_UPGRADE_ENABLE

#include "usart3_debug.h"
#include "UpgradeDebugLog.h"
#include <stdio.h>

#if UPGRADE_DBG_ENABLE

#if (__ARMCC_VERSION >= 6010050)
__asm(".global __use_no_semihosting\n\t");
__asm(".global __ARM_use_no_argv \n\t");
#else
#pragma import(__use_no_semihosting)
struct __FILE
{
    int handle;
};
#endif

int _ttywrch(int ch)
{
    (void)ch;
    return ch;
}

void _sys_exit(int x)
{
    (void)x;
}

char *_sys_command_string(char *cmd, int len)
{
    (void)cmd;
    (void)len;
    return NULL;
}

FILE __stdout;

int fputc(int ch, FILE *f)
{
    (void)f;
    while ((USART3->ISR & USART_ISR_TXE_TXFNF) == 0U)
    {
    }
    USART3->TDR = (uint8_t)ch;
    return ch;
}

static UART_HandleTypeDef huart3_debug;

void USART3_DebugInit(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART3;
    PeriphClkInit.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
        return;
    }

    __HAL_RCC_USART3_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    huart3_debug.Instance = USART3;
    huart3_debug.Init.BaudRate = DF_USART3_DEBUG_BAUD;
    huart3_debug.Init.WordLength = UART_WORDLENGTH_8B;
    huart3_debug.Init.StopBits = UART_STOPBITS_1;
    huart3_debug.Init.Parity = UART_PARITY_NONE;
    huart3_debug.Init.Mode = UART_MODE_TX;
    huart3_debug.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart3_debug.Init.OverSampling = UART_OVERSAMPLING_16;
    huart3_debug.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart3_debug.Init.ClockPrescaler = UART_PRESCALER_DIV1;
    huart3_debug.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    (void)HAL_UART_Init(&huart3_debug);
}

#else /* UPGRADE_DBG_ENABLE */

void USART3_DebugInit(void)
{
}

#endif /* UPGRADE_DBG_ENABLE */

#endif /* OTA_UPGRADE_ENABLE */
