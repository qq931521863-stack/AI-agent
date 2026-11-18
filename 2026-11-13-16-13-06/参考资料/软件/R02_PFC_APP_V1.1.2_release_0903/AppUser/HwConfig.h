
#ifndef HWCONFIG_H
#define HWCONFIG_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32g4xx_hal.h"
#include "math.h"
#include "arm_math.h"
#include <stdio.h>
#include "string.h"
#include "main.h"
#include "MODBUS_SLAVE.h"
#include "ProtectionPFC.h"

#define INPUT_POW_MAX           1760.0f
#define INPUT_CUR_MAX           8.0f
#define FENGZHI_CUR_MAX         20.0f

#define HW_VBUS_OVP             450.0f
#define HW_VBUS_HIP             400.0f
#define HW_VBUS_SET             360.0f
#define HW_VBUS_LIP             340.0f

#define HW_VBUS_UVP             260.0f

#define SYSTEM_PLV              170000000.0f
#define SHRTIMER_PLV            68000.0f
#define SHRTIMER_PREIOD         2500.0f 
#define SHRTIMER_HALF_PREIOD    1250.0f
#define SHRTIMER_DEADTIME       17 //100ns

#define PI_FLOAT                3.141592653f
#define COM_VREF_K1             0.998f
#define COM_VREF_K2             0.002f

#define HW_ADC_REF              3.3f

#define HW_CUR_INDUC_GAIN       22.7273f   //待修正
#define HW_ACIN_VOL_GAIN        400.0f
#define HW_VBUS_GAIN            194.12f

#define HW_AUX_GAIN             4.9f

#define HW_DAC_OCP              15.0f  //24

#define HW_DAC_LCUR             (uint16_t)((HW_DAC_OCP * 4095)/(HW_ADC_REF * HW_CUR_INDUC_GAIN)) 
#define HW_DAC_VBUS             (uint16_t)((HW_VBUS_OVP * 4095)/(HW_ADC_REF * HW_VBUS_GAIN))


#define COM_VBUS_BASE           ((float)(HW_ADC_REF * HW_VBUS_GAIN))/4095.0f
#define COM_ACIN_VOL_BASE       ((float)(HW_ADC_REF * HW_ACIN_VOL_GAIN))/4095.0f
#define COM_CUR_INDUC_BASE      ((float)(HW_ADC_REF * HW_CUR_INDUC_GAIN))/4095.0f	
#define COM_CUR_FAN_BASE        ((float)(HW_ADC_REF * HW_FAN_GAIN ))/4095.0f
#define COM_AUX_BASE            ((float)(HW_ADC_REF * HW_AUX_GAIN ))/4095.0f
	
#define DrvL_On()  {GPIOB->MODER &= 0XF3FFFFFF; GPIOB->MODER |= 0X08000000;}
#define DrvL_Off()  {GPIOB->MODER &= 0XF3FFFFFF; GPIOB->MODER |= 0X04000000;GPIOB->BRR = (uint32_t)FastMOS_B_Pin;}
#define DrvH_On()   {GPIOA->MODER &= 0XFFFCFFFF; GPIOA->MODER |= 0X00020000;}
#define DrvH_Off()  {GPIOA->MODER &= 0XFFFCFFFF; GPIOA->MODER |= 0X00010000;GPIOA->BRR = (uint32_t)FastMOS_A_Pin;}
typedef struct{
   uint32_t ACin_ov   : 1 ;
   uint32_t ACin_uv   : 1 ;
   uint32_t ACin_oc   : 1 ;
   uint32_t ACin_of   : 1 ;
   uint32_t ACin_uf   : 1 ;
   uint32_t Vbus_ov   : 1 ;
   uint32_t Vbus_uv   : 1 ;
   uint32_t Out_ov    : 1 ;
   uint32_t Out_uv    : 1 ;
   uint32_t Out_oc    : 1 ;
   uint32_t PFC_Err   : 1 ;
   uint32_t LLC_Err   : 1 ;
   uint32_t Out_short : 1 ;
   uint32_t PFC_otp   : 1 ;
   uint32_t LLC_otp   : 1 ;
   uint32_t BAT_revs  : 1 ;
   uint32_t inRLY_Err : 1 ;	
	 uint32_t outRLY_Err : 1 ;	
   uint32_t FAN_Err   : 1 ;
   uint32_t CAN_Err   : 1 ;
   uint32_t INS_Err   : 1 ;
   uint32_t SCI_Err   : 1 ;
   uint32_t res       : 10 ;	
}FAULT_BIT_T;

//小电流模式
typedef union {
	uint32_t     all;
	FAULT_BIT_T  bit;
}FAULT_STA_T;




typedef struct 
{      
	float  CUR_INDUC_GAIN;       
  float  ACIN_VOL_GAIN; 
	float  VBUS_GAIN;
	float  AUX_GAIN;	
	float  DAC_OCP;
} Calibration_st;
extern Calibration_st calib_Data;
typedef struct 
{      
	float  AC_Vol_adc;        
  float  AC_Vol_abs; 
	float  curInduc1_adc;
	float  curInduc1_abs;	
	float  AuxVolt;
	float  AC_mid_volt;
  float  vBus_adc;
  float  vBus_Hold;
  float  vBus_AD_FIR; 
  float  MidCur_abs;
  uint8_t  temp_NTC1;
	uint8_t  temp_NTC2;
	uint8_t  temp_NTC3;
	uint8_t  delayEn;	
	uint8_t  starSoftNum;
	uint16_t adc_time;
	
} ADSample_Var_t;
extern ADSample_Var_t ADSample_Info;

typedef struct 
{      
	uint32_t delay_LED;
	uint32_t delay_Slow;
	uint32_t delay_ini;	
	uint32_t delay_star;
} DelayMS_t;
extern DelayMS_t  Delay1ms;

//状态机枚举量
typedef enum
{
  Init,  //初始化
  Wait,  //空闲等待
  Rise,  //软启
	Derate,//限流状态
  Run,   //正常运行 
  Err    //故障
}STATE_M;


typedef struct 
{      
	STATE_M      RunState;    //运行状态机
	FAULT_STA_T  FaultSta;	  //故障标志
	uint8_t      PfcState;    //PFC状态
	uint8_t      RelaySta;	  //继电器状态
	uint8_t      PwmSta;      //驱动允许信号
	uint8_t      ssFinsh;     //软启动
	uint8_t      Zero;        //过零
	uint8_t      VinDcFlag;   //直流标志
	uint8_t      Mrise;       //启动信号
	uint8_t      PFC_ok;      //pfc工作正常
	uint8_t      LLC_en;      //LLC允许信号
	uint8_t      Negative;    //负半波
	float        OutVolt;     //LLC输出电压
	float        VbusSetVol;  //母线设置电压
	float        Vbus_ref;    //母线参考电压
	float        VbusVolt;    //母线实际电压
	float        ACinFlv;     //交流频率
	float        ACinFreFir;  //滤波后的频率	
	float        ACinVolRMS;  //交流电压有效值
	float        ACinVolRmsFir;  //交流电压有效值
	float        ACinCurRMS;  //交流电流有效值
	float        ACinCurRmsFir;  //交流电流有效值
	float        ACinPower;   //交流输入功率	
	float        ACinMaxPow;  //最大交流输入功率 用于过温降额
	float		     pfcAux;
	float 		   pfcInDcComp; 
	
	float        pfcTransformerTemp;     //PFC 变压器温度
	float		     pfcHeatSinkTemp;        //散热器温度
	float		     pfcInductanceTemp;      //电感温度
} DataFlow_t;
extern DataFlow_t  DataFlowFace;
typedef struct
{
	float    Duty;          	
	uint8_t  DrvH;          //快管驱动
	uint8_t  DrvL;          //快管驱动
	uint8_t  SlowDrvH;      //慢管驱动
	uint8_t  SlowDrvL;      //慢管驱动
}Driver_t;
extern FAULT_STA_T  fault_Pre;	  //故障标志
extern uint16_t MiddleCurrend;
extern Driver_t DriverPwm;
extern float testData[600][6];
extern void sys_intx_enable(void);
extern void sys_intx_disable(void);
extern void IncTick(void);
extern void gpio_config(void);
extern void fwdg_init(void);
extern void CrcIni(void);
extern uint32_t CrcCompute(uint32_t* data,uint32_t num);
extern uint16_t ADC0_Buffer[8],ADC2_Buffer[8];

extern void StateMan(void);
extern void StateMInit(void);
extern void StateMWait(void);
extern void StateMRise(void);
extern void StateMRun(void);
extern void StateMErr(void);
extern void ADC0_Sample(void);
extern void ADC2_Sample(void);
extern void interrupt_ADC1(void);
extern void interrupt_ADC2(void);
extern void PwmCtrl(void);
extern void PwmClose(void);
extern void SHRTIMERdrive(void);
extern void PFCokDisable(void);
extern void PFCokEnable(void);
extern void RelayOn(void);
extern void RelayOff(void);
extern void starLowIni(void);
extern void notch_config(float inflv);
extern void VIacRmsCal(void);
extern void HandleFast(void);
extern void HandleSlow(void);
extern void starLoopIni(void);
extern void runLoopIni(void);
extern void HwOcp(void);
extern void LEDShow(void);
extern void DerateRun(void);
extern void ACinProtect(void);
#ifdef __cplusplus
}
#endif

#endif /* GD32E503V_EVAL_H */
