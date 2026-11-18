
#include "HwConfig.h"

ADSample_Var_t  ADSample_Info;
Calibration_st calib_Data;
uint16_t MiddleCurrend = 0;
uint16_t ADC0_Buffer[8],ADC2_Buffer[8];
const float CalibraData[10] = {HW_CUR_INDUC_GAIN,HW_ACIN_VOL_GAIN,HW_VBUS_GAIN,HW_AUX_GAIN,HW_DAC_OCP};
const uint16_t ZM3973NTC[166]=
{
  3968,3959,3951,3941,3932,3922,3911,3900,3888,3876,3863,3850,3836,3821,3805,3789,
	3772,3755,3736,3717,3697,3676,3654,3631,3607,3583,3558,3532,3505,3477,3448,3419,
	3388,3357,3325,3293,3259,3225,3190,3154,3117,3080,3042,3003,2964,2924,2883,2842,
  2801,2758,2716,2673,2629,2585,2541,2497,2452,2407,2362,2317,2272,2227,2182,2137,
  2092,2048,2003,1959,1915,1872,1829,1786,1744,1703,1662,1621,1581,1542,1503,1465,
  1428,1391,1355,1319,1284,1250,1217,1184,1153,1121,1091,1061,1032,1003, 976, 949,
  922,896,871,847,823,800,777,755,734,713,693,673,654,636,617,600,
  583,566,550,535,519,505,490,476,463,450,437,425,413,401,390,379,
  369,358,348,339,329,320,312,303,295,287,279,271,264,257,250,243,
  237,231,224,218,213,207,202,196,191,186,182,177,172,168,164,159,
  155,152,148,144,140,137
};

static uint8_t tsp_BinaryTableSearch( uint16_t adc_val )
{
	uint16_t start = 0U, end = 165, mid = 0U, retout = 0;
	if( adc_val <= ZM3973NTC[165])retout = 165;
	else if( adc_val >= ZM3973NTC[0])retout = 0;
	else
	{
		while ( start <= end )
		{
			mid = (start + end) >> 1; 
			if( adc_val ==  ZM3973NTC[mid] )
			{
				break;
			}
			if( ( adc_val < ZM3973NTC[mid] ) && ( adc_val > ZM3973NTC[mid+1U] ) )
			{
				break;
			}
			if( adc_val < ZM3973NTC[mid] )
			{
				start = mid + 1U; 
			}
			else if( adc_val > ZM3973NTC[mid] )
			{
				end = mid - 1U;
			}
		}
		start = ZM3973NTC[mid] - adc_val;
		end = (ZM3973NTC[mid]-ZM3973NTC[mid+1])>>1;
		if(start >= end)retout = mid+1;
		else retout = mid;
	}
	return retout;
}
void ADC0_Sample(void)	
{
	static float midCur = 0;
	static uint32_t delay = 0;
  float adc_temp;
	adc_temp = (float)ADC0_Buffer[0];
	if(delay < 50000)
	{
		delay++;
		midCur = midCur * 0.999f + adc_temp * 0.001f;
		ADSample_Info.curInduc1_adc = 0;
		ADSample_Info.AC_Vol_adc = 0;
		ADSample_Info.delayEn = 0;
		ADSample_Info.adc_time = 0;
		MiddleCurrend = (uint16_t)(midCur);
	}else
	{		
		if(ADSample_Info.delayEn == 0)ADSample_Info.delayEn = 1;
    		
		ADSample_Info.curInduc1_adc = (adc_temp - midCur) * COM_CUR_INDUC_BASE;
		//电感电流
		if(ADSample_Info.curInduc1_adc < 0)ADSample_Info.curInduc1_abs = -ADSample_Info.curInduc1_adc;
		else ADSample_Info.curInduc1_abs = ADSample_Info.curInduc1_adc;		
		//电压输入
		adc_temp = (float)ADC0_Buffer[2];
		ADSample_Info.AC_Vol_adc = (float)adc_temp * COM_ACIN_VOL_BASE - ADSample_Info.AC_mid_volt;	
		if(ADSample_Info.AC_Vol_adc < 0)ADSample_Info.AC_Vol_abs = -ADSample_Info.AC_Vol_adc;
		else ADSample_Info.AC_Vol_abs = ADSample_Info.AC_Vol_adc;
    DataFlowFace.pfcInDcComp = 	DataFlowFace.pfcInDcComp * 0.999902f + ADSample_Info.AC_Vol_adc * 0.000098f;	
	}
	
	
  //vbus
	adc_temp = (float)ADC0_Buffer[3];
	ADSample_Info.vBus_adc = (float)adc_temp * COM_VBUS_BASE;	
	
//	cont++;
//	ADSample_Info.AC_Vol_adc = 140.0f * sinf( cont *  PI_FLOAT / 680.0f );
//  if(ADSample_Info.AC_Vol_adc > 0) ADSample_Info.AC_Vol_abs = ADSample_Info.AC_Vol_adc;
//	else ADSample_Info.AC_Vol_abs = - ADSample_Info.AC_Vol_adc;
//	ADSample_Info.vBus_adc = 140.0f;
}	
void ADC2_Sample(void)	
{
  float adc_temp;
	uint16_t temp;

  // 12V辅源
	adc_temp = (float)ADC2_Buffer[0] * COM_AUX_BASE;
	ADSample_Info.AuxVolt= ADSample_Info.AuxVolt * 0.9372f + adc_temp *0.0628f;
	DataFlowFace.pfcAux = ADSample_Info.AuxVolt;
		
	//中点电压
	adc_temp = (float)ADC2_Buffer[1] * COM_ACIN_VOL_BASE; 
	ADSample_Info.AC_mid_volt = ADSample_Info.AC_mid_volt * 0.9372f + adc_temp *0.0628f;

  //TEMP_PFC  MOS
	temp = ADC2_Buffer[2]; 
	ADSample_Info.temp_NTC1 = tsp_BinaryTableSearch(temp);	
	DataFlowFace.pfcHeatSinkTemp = DataFlowFace.pfcHeatSinkTemp * 0.999f + ADSample_Info.temp_NTC1 * 0.001f;
  //TEMP_PFC  变压器
	temp = ADC2_Buffer[3];
	ADSample_Info.temp_NTC2 = tsp_BinaryTableSearch(temp);
	DataFlowFace.pfcTransformerTemp = DataFlowFace.pfcTransformerTemp * 0.999f + ADSample_Info.temp_NTC2 * 0.001f;
  //TEMP_PFC  电感
	temp = ADC2_Buffer[4];
	ADSample_Info.temp_NTC3 = tsp_BinaryTableSearch(temp);
  DataFlowFace.pfcInductanceTemp = DataFlowFace.pfcInductanceTemp * 0.999f + ADSample_Info.temp_NTC3 * 0.001f;
}	





