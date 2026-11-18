
#include "HwConfig.h"
#include "controlAlgorithm.h"
#include "UpgradeFeatureConfig.h"
#if OTA_UPGRADE_ENABLE
#include "upgrade_app.h"
#endif
DelayMS_t   Delay1ms;
DataFlow_t  DataFlowFace;
Driver_t    DriverPwm;
void HAL_IncTick(void)
{
	static uint8_t SysResTime = 0;
#if OTA_UPGRADE_ENABLE
	extern __IO uint32_t uwTick;

	uwTick++;
	Timer1msProFunction();
#endif
	Delay1ms.delay_Slow++;
	if(Delay1ms.delay_Slow >= 5)
	{
		Delay1ms.delay_Slow = 0;
		StateMan();
	}else if(Delay1ms.delay_Slow == 1)
	{
		//状态显示灯函数
		LEDShow();		
		SysResTime++;
		if(SysResTime >= 20)
		{
			SysResTime = 0;
			SysReset();
		}
	}else if(Delay1ms.delay_Slow == 2)
	{
		VIacRmsCal();  
	}else if(Delay1ms.delay_Slow == 3)
	{
    ACinProtect();
	}
	overUartTime();
}
void interrupt_ADC1(void)
{
	ADC0_Sample();
	HandleFast();
}
void interrupt_ADC2(void)
{
	ADC2_Sample();
	HandleSlow();	
}

void LEDShow(void)
{
	static uint16_t DelayCnt = 0;
	if(DataFlowFace.RunState == Err)
	{
		DelayCnt++;
		if(DelayCnt >= 250)
		{
			DelayCnt = 0;
			HAL_GPIO_TogglePin(LED_out_GPIO_Port, LED_out_Pin);
		}
  }else
	{
		DelayCnt++;
		if(DelayCnt >= 1000)
		{
			DelayCnt = 0;
			HAL_GPIO_TogglePin(LED_out_GPIO_Port, LED_out_Pin);
		}
	}
}
void StateMan(void)
{
	if(DataFlowFace.FaultSta.all)
	{
		DataFlowFace.RunState = Err;
	}else if(ADSample_Info.delayEn < 2)
	{
		DataFlowFace.RunState = Init;
	}
	//状态机
	switch(DataFlowFace.RunState)
	{
		//初始化状态
		case  Init :StateMInit();
		break;
		//等待状态
		case  Wait :StateMWait();
		break;
		//软启动状态
		case  Rise :StateMRise();
		break;
		//运行状态
		case  Derate :DerateRun();
		break;		
		//运行状态
		case  Run :StateMRun();
		break;
		//故障状态
		case  Err :StateMErr();
		break;
	}
	DataFlowFace.PfcState = DataFlowFace.RunState;
}
uint16_t CapChargedCnt=0;
void StateMInit(void)
{
	RelayOff();
	PFCokDisable();	
  Delay1ms.delay_star++;	
	if(Delay1ms.delay_star > 200)
	{
		Delay1ms.delay_star = 200;
		if(ADSample_Info.delayEn >= 2)
		{
			DataFlowFace.RunState  = Wait;			
			CapChargedCnt=0;
		}		
	}else
	{			
		PFCtempDataStr.iLoopEN = 0;
		DataFlowFace.PwmSta = 0;	
//    if(Delay1ms.delay_star == 1)
//		{
//			HAL_DAC_SetValue(&hdac3,DAC_CHANNEL_2,DAC_ALIGN_12B_R,MiddleCurrend + 100);
//			HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_1,DAC_ALIGN_12B_R,500);			
//	
//		}else if(Delay1ms.delay_star == 196)htim1.Instance->BDTR |= TIM_BDTR_MOE;		
  }
}

void StateMWait(void)
{
//	static float VbusAvgPre = 0;
		
	DataFlowFace.PwmSta = 0;    
//	if((ADSample_Info.vBus_AD_FIR >(VbusAvgPre + 2.0f)) || (ADSample_Info.vBus_AD_FIR <(VbusAvgPre - 2.0f)))CapChargedCnt=0;
//	if(ADSample_Info.vBus_AD_FIR <(VbusAvgPre - 2.0f))CapChargedCnt=0;
//	else 
	{
		CapChargedCnt++;
		if(DataFlowFace.RelaySta)
		{
			if(CapChargedCnt > 50)
			{
				DataFlowFace.RunState  = Rise;
				DataFlowFace.Mrise = 0;
				CapChargedCnt = 0;
			}
		}else
		{
			if(CapChargedCnt > 200)
			{
				CapChargedCnt=0;
				RelayOn();
			}
		}	
	}

//	VbusAvgPre = ADSample_Info.vBus_AD_FIR;
}

void StateMRise(void)
{
	static uint16_t delayTime = 0;
	if(DataFlowFace.Mrise ==0)
	{			
		delayTime = 0;
		DataFlowFace.ssFinsh = 0;
		DataFlowFace.Mrise =1;
		iParamPid.inte = 0;	
		vParamPid.inte = 100.0f;
		vParamPid.Out = 100.0f;		
		DataFlowFace.PwmSta = 1;
		DataFlowFace.Vbus_ref = ADSample_Info.vBus_AD_FIR + 2.0f;
	}else
	{
		if(ADSample_Info.vBus_AD_FIR > (DataFlowFace.VbusSetVol - 4.0f))
		{
			DataFlowFace.RunState  = Run;
			PFCokEnable();
			DataFlowFace.PFC_ok = 1;
			DataFlowFace.ssFinsh =1;
			DataFlowFace.Mrise =0;
			delayTime = 0;
		}		
//		delayTime++;
//		if(delayTime > 2000)
//		{
//			DataFlowFace.FaultSta.bit.Vbus_uv = 1;
//		}
	}
}
void DerateRun(void)
{	
	if(DataFlowFace.FaultSta.all)
	{
		DataFlowFace.RunState = Err;
		DataFlowFace.PwmSta = 0;
	}
}
void StateMRun(void)
{	
	if(DataFlowFace.FaultSta.all)
	{
		DataFlowFace.RunState = Err;
		DataFlowFace.PwmSta = 0;
	}
}
void StateMErr(void)
{
  static uint16_t MerrCnt = 0;
	DataFlowFace.PwmSta = 0;
	DataFlowFace.ssFinsh=0;
	PFCokDisable();
  DataFlowFace.PFC_ok = 0;
	RelayOff();
	DataFlowFace.RelaySta = 0;
	if(DataFlowFace.FaultSta.all == 0)
	{	
		if(ADSample_Info.vBus_AD_FIR < 340.0f)
		{
			MerrCnt++;
			if(MerrCnt >= 200)
			{
				//跳转至空闲等待状态,重新启动
				DataFlowFace.RunState = Init;
				ADSample_Info.starSoftNum = 0;
				Delay1ms.delay_star = 0;				
				if(ADSample_Info.delayEn > 1)ADSample_Info.delayEn = 1;
				MerrCnt = 0;
			}
	  }else MerrCnt = 0;
  }else MerrCnt = 0;
}

void starLowIni(void)
{
	memset((uint8_t*)&DataFlowFace,0,sizeof(DataFlow_t));	
	memset((uint8_t*)&DriverPwm,0,sizeof(Driver_t));	
	memset((uint8_t*)&ADSample_Info,0,sizeof(ADSample_Var_t));
	memset((uint8_t*)&Delay1ms,0,sizeof(DelayMS_t));
	PwmClose();
}

void PwmClose(void)
{
	DriverPwm.Duty = 0;
	DriverPwm.DrvH = 0;
	DriverPwm.DrvL = 0;
	DriverPwm.SlowDrvH = 0;
	DriverPwm.SlowDrvL = 0;	
}

void vBusContrl(void)
{
	float out,temp = 0;
	static float peak = 0;
	peak = peak * 0.98f + PFCtempDataStr.Peak * 0.02f;
	peak = PFCtempDataStr.Peak;	
	out = peak + 30.0f;
	if(out > 405.0f)out = 405.0f;
	else if(out < 360.0f)out = 360.0f;
	
	PFCtempDataStr.HwVbusSet = out;
	temp = out + 30.0f; 	
	if(temp < 400.0f)temp = 400.0f;
	if(temp > 435.0f)temp = 435.0f;
	PFCtempDataStr.HwVbusHip = temp;  
	temp = peak + 15.0f;
	if(temp < 320.0f)temp = 320.0f;
	PFCtempDataStr.HwVbusLip = temp;
	temp = peak;
	if(temp < 320.0f)temp = 320.0f;
  PFCtempDataStr.HwVbusUvp = temp;

}

//void SHRTIMERdrive(void)
//{
//	uint16_t temp = (uint16_t) (0.5f * SHRTIMER_HALF_PREIOD);	
//	__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1,temp);	
////	if(Delay1ms.delay_star > 190)
//	{
//		DrvH_On();	
//		DrvL_On();	
//	}
//}
void SHRTIMERdrive(void)
{
	uint16_t temp = (uint16_t) (DriverPwm.Duty * SHRTIMER_HALF_PREIOD);	
	__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1,temp);
	
	if(DriverPwm.DrvH){DrvH_On();}
	else {DrvH_Off();}
	if(DriverPwm.DrvL){DrvL_On();}
	else {DrvL_Off();}
	
	if(DriverPwm.SlowDrvH)HAL_GPIO_WritePin(SlowMos_A_GPIO_Port, SlowMos_A_Pin, GPIO_PIN_SET);
	else HAL_GPIO_WritePin(SlowMos_A_GPIO_Port, SlowMos_A_Pin, GPIO_PIN_RESET);
	if(DriverPwm.SlowDrvL)HAL_GPIO_WritePin(SlowMos_B_GPIO_Port, SlowMos_B_Pin, GPIO_PIN_SET);
	else HAL_GPIO_WritePin(SlowMos_B_GPIO_Port, SlowMos_B_Pin, GPIO_PIN_RESET);		
}
void PFCokDisable(void)
{
	HAL_GPIO_WritePin(PFC_OK_GPIO_Port, PFC_OK_Pin, GPIO_PIN_RESET);
	DataFlowFace.PFC_ok = 0;
}
void PFCokEnable(void)
{
	HAL_GPIO_WritePin(PFC_OK_GPIO_Port, PFC_OK_Pin, GPIO_PIN_SET);
	DataFlowFace.PFC_ok = 1;
}
void RelayOn(void)
{
	HAL_GPIO_WritePin(RELAY_CTRL_GPIO_Port, RELAY_CTRL_Pin, GPIO_PIN_SET);
	DataFlowFace.RelaySta = 1;
}
void RelayOff(void)
{
	HAL_GPIO_WritePin(RELAY_CTRL_GPIO_Port, RELAY_CTRL_Pin, GPIO_PIN_RESET);
	DataFlowFace.RelaySta = 0;
}

