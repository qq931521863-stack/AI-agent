#ifndef UART_PORT_H
#define UART_PORT_H

#include "stm32g4xx_hal.h"

#define UART_PORT_RX_DMA_LEN   256U

extern UART_HandleTypeDef huart1;
extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart1_tx;

void UartPort_Init(void);
void UartPort_ProcessIdleIrq(void);
void UartPort_HandleIdleRx(const uint8_t *pu8Data, uint16_t u16Len);
void Uart1DMASendFunction(uint8_t *u8Buf, uint16_t u16len);
void UartPort_RestartRxDma(void);

#endif /* UART_PORT_H */
