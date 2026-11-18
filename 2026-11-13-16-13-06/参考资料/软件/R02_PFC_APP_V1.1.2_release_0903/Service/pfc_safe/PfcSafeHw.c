#include "PfcSafeHw.h"

#define PFC_RELAY_PIN          GPIO_PIN_15
#define PFC_RELAY_PORT         GPIOC
#define PFC_SLOW_A_PIN         GPIO_PIN_15
#define PFC_SLOW_B_PIN         GPIO_PIN_14
#define PFC_SLOW_PORT          GPIOB
#define PFC_FAST_A_PIN         GPIO_PIN_8
#define PFC_FAST_A_PORT        GPIOA
#define PFC_FAST_B_PIN         GPIO_PIN_13
#define PFC_FAST_B_PORT        GPIOB
#define PFC_OK_PIN             GPIO_PIN_5
#define PFC_OK_PORT            GPIOB

static void PfcHw_StopTim1Outputs(void)
{
    if ((RCC->APB2ENR & RCC_APB2ENR_TIM1EN) == 0U)
    {
        RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
    }

    TIM1->BDTR &= ~(uint32_t)(TIM_BDTR_MOE | TIM_BDTR_AOE);
    TIM1->CCER = 0U;
    TIM1->CR1 &= ~(uint32_t)TIM_CR1_CEN;
}

static void PfcHw_ConfigOutputLow(GPIO_TypeDef *port, uint32_t pin)
{
    GPIO_InitTypeDef gpio = {0};

    HAL_GPIO_WritePin(port, (uint16_t)pin, GPIO_PIN_RESET);
    gpio.Pin = (uint16_t)pin;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(port, &gpio);
}

void PfcHw_EarlySafePinInit(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    PfcHw_StopTim1Outputs();

    PfcHw_ConfigOutputLow(PFC_RELAY_PORT, PFC_RELAY_PIN);
    PfcHw_ConfigOutputLow(PFC_SLOW_PORT, (uint32_t)PFC_SLOW_A_PIN | (uint32_t)PFC_SLOW_B_PIN);
    PfcHw_ConfigOutputLow(PFC_OK_PORT, PFC_OK_PIN);
    PfcHw_ConfigOutputLow(PFC_FAST_A_PORT, PFC_FAST_A_PIN);
    PfcHw_ConfigOutputLow(PFC_FAST_B_PORT, PFC_FAST_B_PIN);
}

void PfcHw_SafeShutdown(void)
{
    PfcHw_EarlySafePinInit();
}
