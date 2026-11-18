
#include "controlAlgorithm.h"
#include "HwConfig.h"
_filter_Deal_t Notch_Filter;
ParamPid_t vParamPid;
ParamPid_t iParamPid;
PFCtempData_t PFCtempDataStr;
filter_str iLoopFront;
void HandleSlow(void)
{
	static uint16_t cont = 0;
	float temp = 0;
	if(DataFlowFace.LLC_en == 0)
	{
	  if(HAL_GPIO_ReadPin(LLC_OK_GPIO_Port, LLC_OK_Pin))cont++;
		else cont = 0;
    if(cont >= 10)DataFlowFace.LLC_en = 1;
	}
	else
	{
	  if(HAL_GPIO_ReadPin(LLC_OK_GPIO_Port, LLC_OK_Pin) == 0)DataFlowFace.LLC_en = 0;
	}	
//	DataFlowFace.LLC_en = 0;
	vBusContrl();
  DataFlowFace.VbusSetVol = PFCtempDataStr.HwVbusSet;
	if((DataFlowFace.RunState >= Rise)&&(DataFlowFace.FaultSta.all == 0))
	{		
		if(PFCtempDataStr.iLoopEN)
		{			
			if(DataFlowFace.Vbus_ref < DataFlowFace.VbusSetVol - 0.05f)DataFlowFace.Vbus_ref  += 0.05f;
			else if( DataFlowFace.Vbus_ref > DataFlowFace.VbusSetVol + 0.05f)DataFlowFace.Vbus_ref  -= 0.05f;
			else DataFlowFace.Vbus_ref = DataFlowFace.VbusSetVol;
	 
		 //电压环函数
		 vParamPid.err = DataFlowFace.Vbus_ref - ADSample_Info.vBus_AD_FIR;                                          
		 temp = DataFlowFace.ACinVolRMS * 0.005f;
		 vParamPid.inte += temp * 0.2f * vParamPid.err;
		 vParamPid.Out = vParamPid.inte + temp * 10.0f * vParamPid.err;
//		 vParamPid.inte += temp * 0.1f * vParamPid.err;
//		 vParamPid.Out = vParamPid.inte + temp * 5.0f * vParamPid.err;				
		 if(DataFlowFace.RunState == Rise)temp = DataFlowFace.ACinPower + 200.0f;
		 else temp = DataFlowFace.ACinVolRMS * INPUT_CUR_MAX;

		 if(temp > INPUT_POW_MAX)temp = INPUT_POW_MAX;
		
		 if(vParamPid.inte > temp)vParamPid.inte = temp;
		 else if(vParamPid.inte < 0.0f)vParamPid.inte = 0.0f;		 
		 if(vParamPid.Out > temp)vParamPid.Out = temp;
		 else if(vParamPid.Out < 0.0f)vParamPid.Out = 0.0f;
		}else
		{
			vParamPid.Out = 100.0f;
		}
		 
		 PFCtempDataStr.vPidOut = vParamPid.Out ;
		 
		 DataFlowFace.PwmSta = 1;
//		if((DataLLC.outCurr >= 3.0f)&&(DataFlowFace.RunState == Run))
//		{
//			if(ADSample_Info.vBus_AD_FIR < PFCtempDataStr.HwVbusUvp)
//			{
//				DataFlowFace.FaultSta.bit.Vbus_uv = 1;
//			}
//		}		
	}else 
	{
		DataFlowFace.PwmSta = 0;
	}
}
void notch_config(float inflv)
{
	static uint8_t high_flv = 0;
	switch(high_flv)
	{
		case 0:
			if(inflv >= 55.0f)
			{
				notchRef(60.0f * NOTCH_PI,&Notch_Filter);
				high_flv = 2;
			}else
			{
				notchRef(50.0f * NOTCH_PI,&Notch_Filter);
				high_flv = 1;
			}
		  break;
		case 1:
			if(inflv >= 56.0f)
			{
				notchRef(60.0f * NOTCH_PI,&Notch_Filter);
				high_flv = 2;
			}			
			break;
		case 2:
			if(inflv < 54.0f)
			{
				notchRef(50.0f * NOTCH_PI,&Notch_Filter);
				high_flv = 1;
			}			
			break;
		default:
			break;
	}
}


/*
** ===================================================================
**     Funtion Name : void VloopKpKiCal(void)
**     Description : 电压环Kp和Ki变增益，当BUS电压过高或者过低时能够快速调节会参考值
**     Parameters  : none
**     Returns     : none
** ===================================================================
*/
void VloopKpKiCal(void)
{
	float temp,tempa;
	//计算参考电压
//	temp = DataFlowFace.OutVolt * 8.0f;
	temp = 120.0f;
	tempa = DataFlowFace.ACinVolRMS * 1.5f;	
	if(temp < tempa)temp = tempa;
	if(temp > 400.0f)temp = 400.0f;
//	else if(temp < 320.0f)temp = 320.0f;
	
	DataFlowFace.VbusSetVol = temp;
	temp = 400.0f/ADSample_Info.vBus_adc;
	vParamPid.Kp = vLoopPid.Kp * temp;
	vParamPid.Ki = vLoopPid.Ki * temp;		
}
/*
** ===================================================================
**     Funtion Name : void VrefCal(void)
**     Description : 输出参考电压计算，启动时参考电压慢慢增加
**     Parameters  : none
**     Returns     : none
** ===================================================================
*/
void VrefCal(void)
{
	if(PFCtempDataStr.iLoopEN > 0)
	{
		if(DataFlowFace.Vbus_ref < DataFlowFace.VbusSetVol - 0.05f)DataFlowFace.Vbus_ref  += 0.05f;
		else if( DataFlowFace.Vbus_ref > DataFlowFace.VbusSetVol + 0.05f)DataFlowFace.Vbus_ref  -= 0.05f;
		else DataFlowFace.Vbus_ref = DataFlowFace.VbusSetVol;
	}	
}
/*
** ===================================================================
**     Funtion Name : void VoltageLoop(void)
**     Description : 恒压环路
**     Parameters  : none
**     Returns     : none
** ===================================================================
*/
void VoltageLoop(void)
{	
  vParamPid.err = DataFlowFace.Vbus_ref - ADSample_Info.vBus_AD_FIR;
	vParamPid.Out = vParamPid.Kp * (vParamPid.err - vParamPid.inte) + vParamPid.Ki * vParamPid.err;
	vParamPid.inte = vParamPid.err;
}

/*
** ===================================================================
**     Funtion Name : void VbusUVP(void)
**     Description : 母线欠压保护，防止驱动损坏的情况下输出电压不能满足要求
**     Parameters  : none
**     Returns     : none
** ===================================================================
*/

//void VbusUVP(void)
//{
//	float temp;
//	temp = DataFlowFace.ACinVolRMS * 1.414f;
//	//只有在PFC运行状态下处理
//	if(DataFlowFace.RunState ==Run)
//	{
//		//当BUS电压< 线电压计算的BUS整流电压-50V时，发生欠压保护，PFC关闭PWM
//		if(ADSample_Info.vBus_adc < temp - 30.0f)
//		{
//			//标志位置位
//			DataFlowFace.FaultSta.bit.Vbus_uv = 1;
//		
//			//关闭PWM波
//			DataFlowFace.PwmSta = 0;
//		}
//	}
//	//当发生过压后
//	if(DataFlowFace.FaultSta.bit.Vbus_uv ==1)
//	{
//		//当出现过压后，电压恢复到理想值后复位
//		if(ADSample_Info.vBus_AD_FIR > temp + 5.0f)
//		{
//			DataFlowFace.FaultSta.bit.Vbus_uv = 0;
//		}
//	}			
//}

/*
** ===================================================================
**     Funtion Name : void VIacRmsCal(void)
**     Description : 输入电压，输入电流有效值计算
**     Parameters  : none
**     Returns     : none
** ===================================================================
*/

void VIacRmsCal(void)  
{
	float tmp,dk;
	if(PFCtempDataStr.RmsCalReady > 100)        
	{			
		dk = 1.0f / (float)PFCtempDataStr.RmsCalReady;
		DataFlowFace.ACinFlv = SHRTIMER_PLV * dk;
    DataFlowFace.ACinFreFir = DataFlowFace.ACinFreFir * 0.98f + DataFlowFace.ACinFlv * 0.02f;
		tmp = PFCtempDataStr.VacSum * dk;            
		DataFlowFace.ACinVolRMS = tmp * 1.11072f;
		DataFlowFace.ACinVolRmsFir = DataFlowFace.ACinVolRmsFir * 0.9f + DataFlowFace.ACinVolRMS * 0.1f;	
		if(DataFlowFace.ACinVolRMS < 10.0f)	DataFlowFace.ACinVolRMS = 10.0f;
		PFCtempDataStr.RmsVol2inv = 1.0f/(DataFlowFace.ACinVolRMS * DataFlowFace.ACinVolRMS);
		PFCtempDataStr.Peak = DataFlowFace.ACinVolRMS * 1.41421356f;
    PFCtempDataStr.VacPeakInv  = 1.0f/PFCtempDataStr.Peak;
//		PFCtempDataStr.threshold = PFCtempDataStr.Peak * 0.06f;
//		if(PFCtempDataStr.threshold < 8.0f)PFCtempDataStr.threshold = 8.0f;
//		else if(PFCtempDataStr.threshold > 12.0f)PFCtempDataStr.threshold = 12.0f;
		PFCtempDataStr.threshold = 20.0f;
		tmp = PFCtempDataStr.cur1Sum  * dk;
		DataFlowFace.ACinCurRMS =  tmp * 1.11072f;
		DataFlowFace.ACinCurRmsFir = DataFlowFace.ACinCurRmsFir * 0.9f + DataFlowFace.ACinCurRMS * 0.1f;
    PFCtempDataStr.RmsCalReady = 0;    
    //Asw_DCMFeedCal();	
    notch_config(DataFlowFace.ACinFlv);	
	
		DataFlowFace.ACinPower = DataFlowFace.ACinVolRMS * DataFlowFace.ACinCurRMS;     
		if(DataFlowFace.ACinPower < 1.0f)DataFlowFace.ACinPower = 1.0f;		
		
		
		if(PFCtempDataStr.LowPin == 1)
		{
			if(DataFlowFace.ACinPower > 50.0f)
			{
				PFCtempDataStr.LowPin = 0;			
			}
		}else
		{
			if(DataFlowFace.ACinPower < 40.0f)
			{
			 PFCtempDataStr.LowPin = 1;//标志位置1		 
			}	
		}			
	}
}

/*
** ===================================================================
**     Funtion Name : void DCMFeedCal(void)
**     Description : DCM模式下前馈系数计算
**     Parameters  : none
**     Returns     : none
** ===================================================================
*/
void Asw_DCMFeedCal(void)         
{
	static float VacPeakTemp=0;
	static float VacRmsTemp=0;
	DataFlowFace.ACinPower = DataFlowFace.ACinVolRMS * DataFlowFace.ACinCurRMS;     
	if(DataFlowFace.ACinPower < 1.0f)DataFlowFace.ACinPower = 1.0f;
	PFCtempDataStr.ACpowerSqrtf =  sqrtf(DataFlowFace.ACinPower);

	PFCtempDataStr.ACpowerSqrtf1fir  = PFCtempDataStr.ACpowerSqrtf1fir * 0.6875f + PFCtempDataStr.ACpowerSqrtf * 0.3125f;

	//计算电压峰值倒数限制，防止电压过小，倒数计算过大，限制170的峰值
//	if(PFCtempDataStr.Peak  < 80.0f)VacPeakTemp = 80.0f;
	if(PFCtempDataStr.Peak  < 127.0f)VacPeakTemp = 127.0f;
	else VacPeakTemp = PFCtempDataStr.Peak;
	PFCtempDataStr.VacPeakInv = 1 / VacPeakTemp;
	PFCtempDataStr.FeedcmK =  PFCtempDataStr.ACpowerSqrtf1fir * PFCtempDataStr.VacPeakInv * FEED_COEF;

	if(DataFlowFace.ACinVolRMS < 90.0f)VacRmsTemp = 90.0f;
	else VacRmsTemp = DataFlowFace.ACinVolRMS;
	PFCtempDataStr.RmsVol2inv = 1.0f / (VacRmsTemp * VacRmsTemp);
	//当输出功率较小时，取消前馈

	if(PFCtempDataStr.LowPin == 1)
	{
	   PFCtempDataStr.Feedssk = 0;
		if(DataFlowFace.ACinPower > 50.0f)
		{
			PFCtempDataStr.LowPin = 0;			
		}
	}else
	{
	  PFCtempDataStr.Feedssk = 1;//取消前馈
	  if(DataFlowFace.ACinPower < 40.0f)
	  {
		 PFCtempDataStr.LowPin = 1;//标志位置1		 
	  }	
	}
}


float SecondOrderForm(float input, _filter_Deal_t *p)
{
	 float vr;
	 vr = (input * p->coeff_B0) + p->filter_W1;
	 p->filter_W1 = (input * p->coeff_B1) + p->filter_W2 - (vr * p->coeff_A1);
	 p->filter_W2 = (input * p->coeff_B2) - (vr * p->coeff_A2);
	
	  
	 return vr;
}
 
void notchRef(float flv,_filter_Deal_t *p)
{
	float k1,k2,v1,v2,v3;
	if(flv < 40.0f)flv = 40.0f;
	v1 = NOTCH_BW2/(flv*flv);
	v2 = (sqrtf(v1 + 1.0f)) - 1.0f;
	k1 = sqrtf(v2/NOTCH_DEPTH2);
	k2 = NOTCH_DEPTH1*k1;

	v3 = flv * NOTCH_DELAY;	
	v2 = 4.0f * v3;
	v1 = v3 * v3; 
	
	p->coeff_A0 = v1 + k1*v2 + 4.0f;
	p->coeff_A1 = 2*v1 - 8;
	p->coeff_A2 = v1 - k1*v2 + 4.0f;
	
	p->coeff_B0 = v1 + k2*v2 + 4.0f;
	p->coeff_B1 = p->coeff_A1;
	p->coeff_B2 = v1 - k2*v2 + 4.0f;
	
	p->coeff_A1 /= p->coeff_A0;
	p->coeff_A2 /= p->coeff_A0;
	
	p->coeff_B0 /= p->coeff_A0;
	p->coeff_B1 /= p->coeff_A0;
	p->coeff_B2 /= p->coeff_A0;

}
void Compensator_Pid(ParamPid_t *p, float err)
{
	p->err = err;                                              //计算误差
  p->inte += p->Ki*p->err;
	p->Out = p->inte + p->Kp * p->err;
}
float oneOrderForm(float input, filter_str *p)
{
	 float vr;
	 vr = (input * p->coeff_B0) + p->filter_W1;
	 p->filter_W1 = (input * p->coeff_B1) - (vr * p->coeff_A1);
	 return vr;
}
void starLoopIni(void)
{	
	uint8_t i = 0;
	memset((uint8_t*)&PFCtempDataStr,0,sizeof(PFCtempData_t));
	
	for(i = 0;i<SYS_PROTECT_NUM;i++)
	{
	  memset((uint8_t*)&SysProtectData[i],0,sizeof(TSysProData));
	}
	fault_Pre.all = 0;
}

void runLoopIni(void)
{
	DataFlowFace.ACinMaxPow = INPUT_POW_MAX;
	memcpy(&vParamPid,&vLoopPid,sizeof(ParamPid_t));
	memcpy(&iParamPid,&iLoopPid,sizeof(ParamPid_t));
	memcpy(&iLoopFront,&loopFilter,sizeof(filter_str));	
}
const filter_str loopFilter = {0,0,0,0,33.096f,-33.7841f,1.0f,-0.6881f,0.0f};
const ParamPid_t vLoopPid = {15.5373f,0.393569f,0,0,0,0,0};
const ParamPid_t iLoopPid = {0.085f,0.0389f,0,0,0,0,0};
const float lingdian[10] = {0.05f,0.1f,0.15,0.2f,0.3f,0.4f,0.55f,0.7f,0.85f,0.7f};
