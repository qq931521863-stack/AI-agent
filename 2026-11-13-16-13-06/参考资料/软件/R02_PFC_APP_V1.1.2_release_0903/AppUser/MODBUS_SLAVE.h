#ifndef MODBUS_SLAVE_H
#define MODBUS_SLAVE_H
#include "HwConfig.h"
#include <string.h>

#define LLC_MODBUS_OTA_SAFE_REQ   0x80U
#define PFC_LLC_UART_BAUD         115200U
#define USART1_RX_DMA_LEN         20U
#define BUFFER_SIZE               20U

#define pfc_id 0x01
#define llc_id 0xA1

typedef struct
{
	uint8_t Stuts;              //0:空闲 1：恒压 2：恒流
	float outVolt;              //0.25V	
	float outCurr;              //故障等级
}STATUS_LLC;

extern STATUS_LLC DataLLC;
extern void Modbus_Slave_Rx(uint8_t *RxData ,uint16_t Len);
extern uint8_t Rxbuffer_usat1[BUFFER_SIZE];
extern void Send_to_llc_info(void);
extern void overUartTime(void);
extern void overTime_llc(void);
extern void InitModbusData(void);
#endif
