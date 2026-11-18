
#include "controlAlgorithm.h"
#include "HwConfig.h"

/*
** ===================================================================
**     Funtion Name : HandleFast(void)
**     Description :  电流环计算
       
** ===================================================================
*/
void HandleFast(void)
{	
	uint8_t pfc_en = 0;
//	float adc_temp= 0;
	if(HAL_GPIO_ReadPin(LLC_OK_GPIO_Port, LLC_OK_Pin) == 0)DataFlowFace.LLC_en = 0;
	if(PFCtempDataStr.iLoopEN)
	{
		if(ADSample_Info.curInduc1_abs > FENGZHI_CUR_MAX)
		{
			PFCtempDataStr.iLoopEN = 0;
			iParamPid.inte = 0;			
			fault_Pre.bit.ACin_oc = 1;
		}		
		if(ADSample_Info.vBus_adc - PFCtempDataStr.HwVbusHip >= 10.0f)
		{
			PFCtempDataStr.iLoopEN = 0;
			iParamPid.inte = 0;			
			fault_Pre.bit.Vbus_ov = 1;
		}
	}	
	HwOcp();
	
	if((DataFlowFace.ACinFlv <= 30)||(DataFlowFace.ACinFlv >= 70))
	{
		DataFlowFace.ACinVolRMS = 0;
		DataFlowFace.ACinVolRmsFir = 0;
		DataFlowFace.ACinCurRMS = 0;
		DataFlowFace.ACinCurRmsFir = 0;
		DataFlowFace.ACinFlv = 0.0f;
		DataFlowFace.ACinFreFir = 0;
		pfc_en = 0;
	}else pfc_en = 1;
	
	if((DataFlowFace.ACinVolRMS <= 60.0f) || (DataFlowFace.ACinVolRMS >= 300.0f)) pfc_en = 0;
	else pfc_en = 1;	

	if(DataFlowFace.FaultSta.all)
	{
		PFCokDisable();
		DataFlowFace.RunState = Err;
		RelayOff();		
		PFCtempDataStr.iLoopEN = 0;
		Delay1ms.delay_star = 0;
		DataFlowFace.PwmSta = 0;
		if(ADSample_Info.delayEn > 1)ADSample_Info.delayEn = 1;
		ADSample_Info.starSoftNum = 0;
    Notch_Filter.filter_out = 0;
    Notch_Filter.filter_W0 = 0;
    Notch_Filter.filter_W1 = 0;
    Notch_Filter.filter_W2 = 0;			
	}else if((DataFlowFace.LLC_en == 0)|| fault_Pre.all || (pfc_en == 0))
	{		
		PFCokDisable();
		RelayOff();
		PFCtempDataStr.iLoopEN = 0;
		Delay1ms.delay_star = 0;
		DataFlowFace.PwmSta = 0;		
		if(ADSample_Info.delayEn > 1)ADSample_Info.delayEn = 1;
		ADSample_Info.starSoftNum = 0;		
		DataFlowFace.RunState = Init;		
    Notch_Filter.filter_out = 0;
    Notch_Filter.filter_W0 = 0;
    Notch_Filter.filter_W1 = 0;
    Notch_Filter.filter_W2 = 0;		
	}	
	
	if(PFCtempDataStr.iLoopEN)
	{		
		iLoop_Control();
	}	
	Syn_Signal_Calc();
	if(PFCtempDataStr.iLoopEN == 0)  
	{
		DriverPwm.DrvH = 0;
		DriverPwm.DrvL = 0;
		DriverPwm.SlowDrvL = 0;
		DriverPwm.SlowDrvH = 0;				
	}			
	SHRTIMERdrive();
	DataFlowFace.VbusVolt = DataFlowFace.VbusVolt * 0.95f + ADSample_Info.vBus_adc * 0.05f; 
	ADSample_Info.vBus_AD_FIR = SecondOrderForm(DataFlowFace.VbusVolt,&Notch_Filter);
	
	
}

/*
** ===================================================================
**     Funtion Name :void  HwOcp(void)
**     Description : 硬件过流保护
**     Parameters  :
**     Returns     :
** ===================================================================
*/
void HwOcp(void)
{
	//硬件周期封波次数计数器
	static int OpcCnt=0;
	//10MS硬件封波计时器
	static long OcpTimeCnt=0;
  uint32_t itflag   = htim1.Instance->SR;	
  if ((itflag & TIM_FLAG_BREAK) == TIM_FLAG_BREAK)
  {
		__HAL_TIM_CLEAR_FLAG(&htim1, TIM_FLAG_BREAK);	
		//封波次数计数器累加
		OpcCnt++;			
	}

	//10ms内连续封波次数大于100次，则发生严重过流故障
	if(OpcCnt > 100)
	{
		//计数器清0
		OpcCnt=0;		
	  //硬件过压标志位置位
		fault_Pre.bit.ACin_oc = 1;
	}
	//当发生封波时
	if(OpcCnt >0)
	{
		//10ms计时器计时
		OcpTimeCnt++;
		//10ms计时结束
		if(OcpTimeCnt>680)
		{
			//封波计数器清0
			OpcCnt =0;
			//10ms计时器清0
			OcpTimeCnt=0;
		}
	}else OcpTimeCnt = 0;
}


