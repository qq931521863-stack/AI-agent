
#include "ProtectionPFC.h"
#include "HwConfig.h"

FAULT_STA_T fault_Pre;
HwProData  HwprotectData;
TSysProData SysProtectData[SYS_PROTECT_NUM];
uint8_t HwProtectEn = 0,HwOcOv = 0,HwOvBusNum = 0;
const TSysProPara  SysProPara[SYS_PROTECT_NUM] = {
	//ID 使能 比较 阀值A   阀值B   时间A  次数 重复次数  重启间隔  清除时间  清除使能  比较 阀值A   阀值B  时间A                          

	{ 0,   1,  1,  268.0f,  0,     100,   20,    1,     1000,    6000,     1 ,     0,  264.0f, 30.0f, 100 },  //1.AC输入过压
	{ 1,   1,  0,  86.0f,   0,     100,   20,    1,     1000,    6000,     1 ,     1,  90.0f,  0.0f,  100 },  //2.输入欠压
	{ 2,   1,  1,  8.5f,    0,     100,   20,    20,    1000,  200000,     0 ,     0,   0.5f,  0.0f,  100 },  //3.输入过流    20  Y  下个半波 
	{ 3,   1,  1,   0.0f,  0.0f,   100,   1,     20,    1000,  200000,     0 ,     0,   0.5f,  0.0f,  100 },  //4.输入过流    20  N
	{ 4,   1,  1,  67.0f, 10.0f,   100,   20,    1,     1000,    6000,     1 ,     0,  65.0f,  0.0f,  100 },  //5.输入过频
	{ 5,   1,  0,  43.0f, 10.0f,   100,   20,    1,     1000,    6000,     1 ,     1,  45.0f,  0.0f,  100 },  //6.输入欠频
	{ 6,   1,  1,  0.0f,  10.0f,   100,    1,    20,    1000,  200000,     0 ,     0,   5.0f,  0.0f,  100 },  //7.母线过压    20  N
	{ 7,   1,  0,  0.0f,   0.0f,   100,   20,    20,    1000,  200000,     0 ,     1,   5.0f,  0.0f,  100 },  //8.母线欠压    20  Y
	{ 8,   1,  2, 12.8f,  11.2f,   100,   20,    1,     1000,    6000,     1 ,     3,  12.5f,  11.5f, 100 },  //9.辅源故障    自恢复
};

uint8_t  Protect_comm(uint8_t fault, uint8_t faultID,float input)
{
	uint8_t i,sult = 0,faultbitA = 0,k = 0;
	uint32_t sum=0;
	faultbitA = fault;
	if(ADSample_Info.delayEn)
	{		
		SysProtectData[faultID].TimeCount ++;
		if(SysProtectData[faultID].TimeCount > 10000)SysProtectData[faultID].TimeCount = 10000;
		SysProtectData[faultID].RepeatCount ++;
		if(SysProtectData[faultID].RepeatCount > 10000)SysProtectData[faultID].RepeatCount = 10000;		
		if(faultbitA)
		{
			if(SysProPara[faultID].ClearEnable)
			{
				sult = 0;
				if(SysProPara[faultID].DirectionConfig == 0)
				{
					if(input < SysProPara[faultID].Threshold_C)sult = 1;
				}else if(SysProPara[faultID].DirectionConfig == 1)
				{
					if(input >= SysProPara[faultID].Threshold_C)sult = 1;
				}else if(SysProPara[faultID].DirectionConfig == 2)
				{
					if(input > SysProPara[faultID].Threshold_C)sult = 1;
					else if(input < SysProPara[faultID].Threshold_D)sult = 1;
				}else
				{
					if(input < SysProPara[faultID].Threshold_C)
					{
					  if(input > SysProPara[faultID].Threshold_D)sult = 1;
					}						
				}
				if(sult)
				{
					SysProtectData[faultID].ClearTime++;
 				  if(SysProtectData[faultID].ClearTime >= SysProPara[faultID].DelayTimeC)
					{
						//清除故障
						faultbitA = 0;
						k =1;
						SysProtectData[faultID].TimeCount = 0;	
					}
				}else
				{
					SysProtectData[faultID].ClearTime = 0;
				}				
			}
		}else
		{
			sult = 0;
			if(SysProPara[faultID].byDirectionConfig == 0)
			{
				if(input < SysProPara[faultID].Threshold_A)sult = 1;
			}else if(SysProPara[faultID].byDirectionConfig == 1)
			{
				if(input > SysProPara[faultID].Threshold_A)sult = 1;
			}else if(SysProPara[faultID].byDirectionConfig == 2)
			{
				if(input > SysProPara[faultID].Threshold_A)sult = 1;
				else if(input < SysProPara[faultID].Threshold_B)sult = 1;
			}else
			{
				if(input < SysProPara[faultID].Threshold_A)
				{
					if(input > SysProPara[faultID].Threshold_B)sult = 1;
				}						
			}
			if(sult)
			{
				k = 0;
				if(SysProtectData[faultID].DelayNum > 0)
				{
				  SysProtectData[faultID].Delay[SysProtectData[faultID].DelayNum - 1] = SysProtectData[faultID].TimeCount;
				}
				SysProtectData[faultID].TimeCount = 0;
				SysProtectData[faultID].DelayNum ++;
				if(SysProtectData[faultID].DelayNum >= SysProPara[faultID].ErrNum)
				{					
					SysProtectData[faultID].DelayNum = 0;
					SysProtectData[faultID].Delay[0] = 0;
					sum = 0;						
					for(i = 0;i < SysProPara[faultID].ErrNum;i++)sum += SysProtectData[faultID].Delay[i];
					if(sum < SysProPara[faultID].DelayTimeA)
					{							
						//故障发生						
						if(SysProPara[faultID].RepeatNum > 1)faultbitA = 1;
						else faultbitA = 0;
						k = 1;
					}
				}
				if(k)
				{
					if(SysProtectData[faultID].RepeatNum > 0)
					{
						SysProtectData[faultID].Repeat[SysProtectData[faultID].RepeatNum - 1] = SysProtectData[faultID].RepeatCount;
					}					
					SysProtectData[faultID].RepeatCount = 0;
					SysProtectData[faultID].RepeatNum ++;
					if(SysProtectData[faultID].RepeatNum >= SysProPara[faultID].RepeatNum)
					{
						SysProtectData[faultID].RepeatNum = 0;	
						sum = 0;						
						for(i = 0;i < SysProPara[faultID].RepeatNum;i++)sum += SysProtectData[faultID].Repeat[i];
						SysProtectData[faultID].Repeat[0] = 0;
						if(sum < SysProPara[faultID].ClearTime)
						{							
							//故障发生							
							faultbitA = 2;
							SysProtectData[faultID].ClearTime = 0;
						}						
					}
				
				}

			}
		}	
	}
  return faultbitA;
}

void InOvProtect(void)
{
	uint8_t fault = 0,sult = 0;
	float input = DataFlowFace.ACinVolRMS;
  if(DataFlowFace.FaultSta.bit.ACin_ov)fault = 2;
	else fault = 0;
  sult = Protect_comm(fault,0,input);
	if(sult==2)DataFlowFace.FaultSta.bit.ACin_ov = 1;
	else 
	{
		DataFlowFace.FaultSta.bit.ACin_ov = 0;
		fault_Pre.bit.ACin_ov = 0; 
	}
}

void InUvProtect(void)
{
	uint8_t fault = 0,sult = 0;
	float input = DataFlowFace.ACinVolRMS;
  if(DataFlowFace.FaultSta.bit.ACin_uv)fault = 2;
	else fault = 0;
  sult = Protect_comm(fault,1,input);
	if(sult==2)DataFlowFace.FaultSta.bit.ACin_uv  = 1;	
	else 
	{
		DataFlowFace.FaultSta.bit.ACin_uv = 0;
    fault_Pre.bit.ACin_uv = 0;
	}		
}

void InOcProtect_s(void)
{
	uint8_t fault = 0,sult = 0;
	float input = DataFlowFace.ACinCurRMS;
//	input = 5.0f;
  if(DataFlowFace.FaultSta.bit.ACin_oc)fault = 2;
	else fault = 0;
  sult = Protect_comm(fault,2,input);	
  if(sult==2)DataFlowFace.FaultSta.bit.ACin_oc  = 1;	
	else 
	{
		DataFlowFace.FaultSta.bit.ACin_oc = 0;
		fault_Pre.bit.ACin_oc = 0;
	}		
}

void InOcProtect_f(void)
{	
	uint8_t fault = 0,sult = 0;	
	float input = 0;
  if(DataFlowFace.FaultSta.bit.ACin_oc == 0)
	{
		fault = 0;
	  if(fault_Pre.bit.ACin_oc)input = 1.0f;	
    sult = Protect_comm(fault,3,input);	
		if(sult==2)DataFlowFace.FaultSta.bit.ACin_oc  = 1;	
		else 
		{
			DataFlowFace.FaultSta.bit.ACin_oc = 0;	
		  fault_Pre.bit.ACin_oc = 0;
		}
	}	
}

void InOfProtect(void)
{
	uint8_t fault = 0,sult = 0;
	float input = DataFlowFace.ACinFlv;
  if(DataFlowFace.FaultSta.bit.ACin_of)fault = 2;
	else fault = 0;
  sult = Protect_comm(fault,4,input);
	
	if(sult==2)DataFlowFace.FaultSta.bit.ACin_of  = 1;		
	else 
	{
		DataFlowFace.FaultSta.bit.ACin_of = 0;	
		fault_Pre.bit.ACin_of  = 0;
	}
}

void InUfProtect(void)
{
	uint8_t fault = 0,sult = 0;
	float input = DataFlowFace.ACinFlv;
  if(DataFlowFace.FaultSta.bit.ACin_uf)fault = 2;
	else fault = 0;
  sult = Protect_comm(fault,5,input);
	
	if(sult >= 1)DataFlowFace.FaultSta.bit.ACin_uf  = 1;		
	else 
	{
		DataFlowFace.FaultSta.bit.ACin_uf = 0;
    fault_Pre.bit.ACin_uf  = 0;	
	}		
}

void InBusOvProtect(void)
{
	uint8_t fault = 0,sult = 0;
	float input = 0;

  if(DataFlowFace.FaultSta.bit.Vbus_ov == 0)
	{
		fault = 0;
	  if(fault_Pre.bit.Vbus_ov)
		{
			input = 1.0f;		
		}
    sult = Protect_comm(fault,6,input);	
		if(sult==2)DataFlowFace.FaultSta.bit.Vbus_ov  = 1;		
		else 
		{
			DataFlowFace.FaultSta.bit.Vbus_ov = 0;
		  fault_Pre.bit.Vbus_ov = 0;
		}
	}	
}

void InBusUvProtect(void)
{
	uint8_t fault = 0,sult = 0;
	float input = 0;
  if(DataFlowFace.FaultSta.bit.Vbus_uv == 0)
	{
		fault_Pre.bit.Vbus_uv = 0;
		if(DataFlowFace.RunState == Run)
		{
//			input = ADSample_Info.vBus_AD_FIR - 340.0f;
			input = ADSample_Info.vBus_AD_FIR - PFCtempDataStr.HwVbusUvp;			
			sult = Protect_comm(fault,7,input);	
			if(sult>=2)DataFlowFace.FaultSta.bit.Vbus_uv  = 1;	
      else if(sult==1)fault_Pre.bit.Vbus_uv  = 1;			
			else 
			{
				fault_Pre.bit.Vbus_uv = 0;
				DataFlowFace.FaultSta.bit.Vbus_uv  = 0;
			}
	  }
	}		
}

void AuxProtect(void)
{
	uint8_t fault = 0,sult = 0;
	float input = ADSample_Info.AuxVolt;
  if(DataFlowFace.FaultSta.bit.PFC_Err)fault = 2;
	else fault = 0;
  sult = Protect_comm(fault,8,input);
	
	if(sult==1)fault_Pre.bit.PFC_Err  = 1;
	else if(sult==2)DataFlowFace.FaultSta.bit.PFC_Err  = 1;		
	else 
	{
		DataFlowFace.FaultSta.bit.PFC_Err = 0;	
		fault_Pre.bit.PFC_Err  = 0;
	}
}

void ACinProtect(void)
{
	if(ADSample_Info.delayEn >= 1)
	{
		InOvProtect();   
	  InUvProtect();
		InOfProtect();
		InUfProtect();	
		InOcProtect_s();   //过流保护
//    AuxProtect();		
	}
	InBusUvProtect();
	InBusOvProtect();
}

void SysReset(void)
{
	if(DataFlowFace.ACinVolRMS < 30.0f)
	{
    DataFlowFace.FaultSta.all = 0;	
    fault_Pre.all = 0;		
		Delay1ms.delay_ini = 0;
		PFCokDisable();
		RelayOff();
		DataFlowFace.RunState = Init;
		PFCtempDataStr.iLoopEN = 0;
		DataFlowFace.PwmSta = 0;
		if(ADSample_Info.delayEn > 1)ADSample_Info.delayEn = 1;
		ADSample_Info.starSoftNum = 0;	
	}
}

