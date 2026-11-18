/**
 * @file        MODBUS_SLAVE.c
 * @brief       Modbus Protocol Slave Framework Program
 * @details     
 * 
 * @author      KJW
 * @date        2026-02-09
 * @version     v1.0.0
 * 
 * @copyright   (c) 2024 Your Company
 * 
 * @note        Before using：
 *             
 * @warning     
 * 
 * @changlog:
 * - v1.0.0 2026-02-09
 *   - Initial version
 */
#include "MODBUS_SLAVE.h"
#include "PfcSafeAppBridge.h"
#include "UpgradeFeatureConfig.h"

STATUS_LLC DataLLC;
uint8_t Rxbuffer_usat1[BUFFER_SIZE] = {0};
uint8_t pfcData[86] = {0};

volatile uint8_t overU = 0;
volatile uint32_t lastRxPfctime = 0;
volatile uint32_t okRxPfctime = 0;

/*******************************************************************************
 * @fn        : void SendData_API(uint8_t *Data,uint16_t Len)
 * @brief     : Send Data API
 * @param     : Data - Pointer to the Data
 *              Len - Length of the Data
 * @return    : null
 * @example   : SendData_API(Data,Len);
 * @author    : KJW
 * @date      : 2026-02-09
 ******************************************************************************/
void SendData_API(uint8_t *Data,uint16_t Len){
	huart1.Instance->CR3 &= 0xFFFFFF7F;	
	__HAL_DMA_DISABLE(&hdma_usart1_tx);
	hdma_usart1_tx.Instance->CNDTR = Len;
	hdma_usart1_tx.Instance->CPAR = (uint32_t)&huart1.Instance->TDR;
	hdma_usart1_tx.Instance->CMAR = (uint32_t)Data;
	__HAL_DMA_ENABLE(&hdma_usart1_tx);
	huart1.Instance->CR3 |= 0x80U;
}


/*******************************************************************************
 * @fn        : uint16_t Modbus_CRC16(uint8_t *data, uint16_t length)
 * @brief     : Modbus CRC16
 * @param     : data - Pointer to the Data
 *              length - Length of the Data
 * @return    : 16bit CRC
 * @example   : Modbus_CRC16(data,length);
 * @author    : KJW
 * @date      : 2026-02-09
 ******************************************************************************/
uint16_t Modbus_CRC16(uint8_t *data, uint16_t length) {
    uint16_t crc = 0xFFFF;  // 初始值
    uint16_t i, j;
    
    for (i = 0; i < length; i++) {
        crc ^= data[i];  // 异或数据字节
        
        for (j = 0; j < 8; j++) {
            if (crc & 0x0001) {  // 如果最低位为1
                crc >>= 1;       // 右移一位
                crc ^= 0xA001;   // 异或多项式(反向)
            } else {
                crc >>= 1;       // 只右移一位
            }
        }
    }
    
    return crc;  // 返回CRC值
}

/*******************************************************************************
 * @fn        : void InitModbusData(void)
 * @brief     : Init Modbus Data
 * @param     : null
 * @return    : null
 * @example   : InitModbusData();
 * @date      : 2026-02-09
 ******************************************************************************/
void InitModbusData(void){
    memset(&DataLLC,0,sizeof(STATUS_LLC));

//#if !OTA_UPGRADE_ENABLE

	__HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);	
	__HAL_DMA_DISABLE(&hdma_usart1_rx);   	
	hdma_usart1_rx.Instance->CNDTR = BUFFER_SIZE;
	hdma_usart1_rx.Instance->CPAR = (uint32_t)&huart1.Instance->RDR;
	hdma_usart1_rx.Instance->CMAR = (uint32_t)&Rxbuffer_usat1;
	__HAL_DMA_ENABLE(&hdma_usart1_rx);
	huart1.Instance->CR3 |= 0x40;
//#endif
}


void rx_llc_data(uint8_t *rxdata,uint8_t len){
	
	float volt, curr;
	uint8_t *pVolt = (uint8_t*)&volt;
	uint8_t *pCurr = (uint8_t*)&curr;

	pVolt[0] = rxdata[2];
	pVolt[1] = rxdata[3];
	pVolt[2] = rxdata[4];
	pVolt[3] = rxdata[5];

	pCurr[0] = rxdata[6];
	pCurr[1] = rxdata[7];
	pCurr[2] = rxdata[8];
	pCurr[3] = rxdata[9];

	if ((rxdata[1] & LLC_MODBUS_OTA_SAFE_REQ) != 0U)
	{
#if OTA_UPGRADE_ENABLE
		PfcApp_RequestSafeShutdown();
#endif
	}
	DataLLC.Stuts = (uint8_t)(rxdata[1] & (uint8_t)(~LLC_MODBUS_OTA_SAFE_REQ));
	DataLLC.outVolt = volt;
	DataLLC.outCurr = curr;
	
}

/*******************************************************************************
 * @fn        : void Modbus_Slave_Rx(uint8_t *RxData ,uint16_t Len)
 * @brief     : Modbus Slave Rx
 * @param     : RxData - Pointer to the Rx Data
 *              Len - Length of the Rx Data
 * @return    : null
 * @example   : Modbus_Slave_Rx(RxData,Len);
 * @author    : KJW
 * @date      : 2026-02-09
 ******************************************************************************/
void Modbus_Slave_Rx(uint8_t *RxData ,uint16_t Len){
	uint16_t crc = 0;
	uint8_t id = 0;
	id = RxData[0];
    crc = (uint16_t)RxData[Len - 1] | (uint16_t)(RxData[Len - 2] << 8);
    if(crc == Modbus_CRC16(RxData,Len - 2) ){
        switch (id)
        {
         case llc_id:
		    rx_llc_data(RxData,Len);
				Send_to_llc_info();
				overU = 1;
				lastRxPfctime = okRxPfctime;
         break;
        default:
         break;
        }
    }   
}

typedef union {
    float f;
    uint8_t bytes[4];
} FloatBytes;


//发送回llc数据
void Send_to_llc_info(void){
	
	uint16_t crc = 0;
	uint8_t idx = 0;
	
	pfcData[idx++] = (uint8_t)pfc_id;
	pfcData[idx++] = (uint8_t)DataFlowFace.RunState;
	pfcData[idx++] = (uint8_t)(DataFlowFace.FaultSta.all >> 24);
	pfcData[idx++] = (uint8_t)(DataFlowFace.FaultSta.all >> 16);
	pfcData[idx++] = (uint8_t)(DataFlowFace.FaultSta.all >> 8);
	pfcData[idx++] = (uint8_t)(DataFlowFace.FaultSta.all & 0xFF);
	pfcData[idx++] = (uint8_t)DataFlowFace.PfcState;
	pfcData[idx++] = (uint8_t)DataFlowFace.RelaySta;
	pfcData[idx++] = (uint8_t)DataFlowFace.PwmSta;
	pfcData[idx++] = (uint8_t)DataFlowFace.ssFinsh;
	pfcData[idx++] = (uint8_t)DataFlowFace.Zero;
	pfcData[idx++] = (uint8_t)DataFlowFace.VinDcFlag;
	pfcData[idx++] = (uint8_t)DataFlowFace.Mrise;
	pfcData[idx++] = (uint8_t)DataFlowFace.PFC_ok;
	pfcData[idx++] = (uint8_t)DataFlowFace.LLC_en;
	pfcData[idx++] = (uint8_t)DataFlowFace.Negative;
	
    FloatBytes fb;
    DataFlowFace.OutVolt = DataLLC.outVolt;
    fb.f = DataFlowFace.OutVolt;
    pfcData[idx++] = fb.bytes[3]; 
    pfcData[idx++] = fb.bytes[2];
    pfcData[idx++] = fb.bytes[1];
    pfcData[idx++] = fb.bytes[0];  

    fb.f = DataFlowFace.VbusSetVol;
    pfcData[idx++] = fb.bytes[3];
    pfcData[idx++] = fb.bytes[2];
    pfcData[idx++] = fb.bytes[1];
    pfcData[idx++] = fb.bytes[0];

    fb.f = DataFlowFace.Vbus_ref;
    fb.f = ADSample_Info.vBus_adc;
    pfcData[idx++] = fb.bytes[3];
    pfcData[idx++] = fb.bytes[2];
    pfcData[idx++] = fb.bytes[1];
    pfcData[idx++] = fb.bytes[0];

    fb.f = ADSample_Info.vBus_AD_FIR;
		fb.f = ADSample_Info.vBus_adc;
    pfcData[idx++] = fb.bytes[3];
    pfcData[idx++] = fb.bytes[2];
    pfcData[idx++] = fb.bytes[1];
    pfcData[idx++] = fb.bytes[0];

    fb.f = DataFlowFace.ACinFlv;
    pfcData[idx++] = fb.bytes[3];
    pfcData[idx++] = fb.bytes[2];
    pfcData[idx++] = fb.bytes[1];
    pfcData[idx++] = fb.bytes[0];

    fb.f = DataFlowFace.ACinFreFir;
    pfcData[idx++] = fb.bytes[3];
    pfcData[idx++] = fb.bytes[2];
    pfcData[idx++] = fb.bytes[1];
    pfcData[idx++] = fb.bytes[0];

    fb.f = DataFlowFace.ACinVolRMS;
    pfcData[idx++] = fb.bytes[3];
    pfcData[idx++] = fb.bytes[2];
    pfcData[idx++] = fb.bytes[1];
    pfcData[idx++] = fb.bytes[0];

    fb.f = DataFlowFace.ACinVolRmsFir;
    pfcData[idx++] = fb.bytes[3];
    pfcData[idx++] = fb.bytes[2];
    pfcData[idx++] = fb.bytes[1];
    pfcData[idx++] = fb.bytes[0];

    fb.f = DataFlowFace.ACinCurRMS;
    pfcData[idx++] = fb.bytes[3];
    pfcData[idx++] = fb.bytes[2];
    pfcData[idx++] = fb.bytes[1];
    pfcData[idx++] = fb.bytes[0];

    fb.f = DataFlowFace.ACinCurRmsFir;
    pfcData[idx++] = fb.bytes[3];
    pfcData[idx++] = fb.bytes[2];
    pfcData[idx++] = fb.bytes[1];
    pfcData[idx++] = fb.bytes[0];

    fb.f = DataFlowFace.ACinPower;
    pfcData[idx++] = fb.bytes[3];
    pfcData[idx++] = fb.bytes[2];
    pfcData[idx++] = fb.bytes[1];
    pfcData[idx++] = fb.bytes[0];

    fb.f = DataFlowFace.ACinMaxPow;
    pfcData[idx++] = fb.bytes[3];
    pfcData[idx++] = fb.bytes[2];
    pfcData[idx++] = fb.bytes[1];
    pfcData[idx++] = fb.bytes[0];
	
	
	fb.f = DataFlowFace.pfcAux;
    pfcData[idx++] = fb.bytes[3];
    pfcData[idx++] = fb.bytes[2];
    pfcData[idx++] = fb.bytes[1];
    pfcData[idx++] = fb.bytes[0];
	
	fb.f = DataFlowFace.pfcInDcComp;
    pfcData[idx++] = fb.bytes[3];
    pfcData[idx++] = fb.bytes[2];
    pfcData[idx++] = fb.bytes[1];
    pfcData[idx++] = fb.bytes[0];		
 
 fb.f = DataFlowFace.pfcHeatSinkTemp;
    pfcData[idx++] = fb.bytes[3];
    pfcData[idx++] = fb.bytes[2];
    pfcData[idx++] = fb.bytes[1];
    pfcData[idx++] = fb.bytes[0];	
	fb.f = DataFlowFace.pfcTransformerTemp;
    pfcData[idx++] = fb.bytes[3];
    pfcData[idx++] = fb.bytes[2];
    pfcData[idx++] = fb.bytes[1];
    pfcData[idx++] = fb.bytes[0];
	
	fb.f = DataFlowFace.pfcInductanceTemp;
    pfcData[idx++] = fb.bytes[3];
    pfcData[idx++] = fb.bytes[2];
    pfcData[idx++] = fb.bytes[1];
    pfcData[idx++] = fb.bytes[0];
	
	crc = Modbus_CRC16(pfcData, idx);

	pfcData[idx++] = (uint8_t)((crc >> 8) & 0xFF);
    pfcData[idx++] = (uint8_t)(crc & 0xFF);

	SendData_API(pfcData, idx);
}

void overUartTime(void){
	okRxPfctime++;

}

void overTime_llc(void){

	if(overU != 1){
		if(okRxPfctime - lastRxPfctime > 3 * 1000){
			DataFlowFace.FaultSta.bit.SCI_Err = 1;
		}
	}else{
		DataFlowFace.FaultSta.bit.SCI_Err = 0;
	}

	overU = 0;
}
