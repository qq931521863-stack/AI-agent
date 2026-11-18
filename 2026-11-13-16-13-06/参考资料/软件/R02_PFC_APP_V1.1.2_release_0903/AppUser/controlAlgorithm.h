

#ifndef CONTROLALGORTHM_H
#define CONTROLALGORTHM_H

#include "HwConfig.h"

//#define Nmax  10
#define NOTCH_BW2      90000.0f   
#define NOTCH_DEPTH1   0.01f  
#define NOTCH_DEPTH2   1.9996f  
#define NOTCH_DELAY    1.0f/(float)SHRTIMER_PLV  
#define NOTCH_PI       6.2831853f

#define FEED_COEF      8.88144f  //2*sqrtf(L*f)
#define FEED_K         0.95f
#define ZERO_NUN       13
#define ZERO_K         0.025f

typedef struct
{
  float filter_out;
  float filter_W0;
  float filter_W1;
  float filter_W2;
    
  float coeff_B0;
  float coeff_B1;
  float coeff_B2;
	float coeff_A0;
  float coeff_A1;
  float coeff_A2;
	
}_filter_Deal_t;	

extern _filter_Deal_t Notch_Filter;

typedef struct
{
  float filter_out;
  float filter_W0;
  float filter_W1;
  float filter_W2;
    
  float coeff_B0;
  float coeff_B1;
  float coeff_B2;
  float coeff_A1;
  float coeff_A2;	
}filter_str;
extern const filter_str loopFilter;
extern filter_str iLoopFront;

typedef struct
{
	float Kp;                  //定义比例参数
	float Ki;                  //定义积分参数
	float err;                 //偏差
	float inte;
	float Out;                 //PID输出
	float MaxLimit;            //PID输出最大限幅
	float MinLimit;            //PID输出最小限幅
}ParamPid_t;

typedef struct
{
	float VacSum;       //电压累加，用以计算有效值用
	float cur1Sum;
	float Peak;         //峰值电压
	float VacPeakInv;
	float threshold;
	float Rmsover2;     //输入电压有效值倒数的平方	

	float ACpowerSqrtf;
	float ACpowerSqrtf1fir;
  float FeedcmK;		
	float RmsVol2inv;

  float CCM_Duty;

	float DCM_Duty;
	float Feedssk;
	float CurCorr;
	float Final_Duty;
	float PwmOutDuty;	
	
	float Iref;
	float PreIref;
	float vPidOut;

	float HwVbusHip;
	float HwVbusSet;
	float HwVbusLip;
	float HwVbusUvp;	
  	
	
	uint16_t RmsCalReady;
	uint8_t LowPin;
	uint8_t SoftStar;
	uint8_t Disable;
	uint8_t Fault;
	uint8_t iLoopEN;
	uint8_t Driveroff;
	uint8_t CCMode;
	uint8_t DrivOff;
	uint8_t N_OnTime;
	uint8_t P_OnTime;
	uint16_t zeroTime;	
	uint16_t ZeroNum;

}PFCtempData_t;

extern ParamPid_t vParamPid;
extern ParamPid_t iParamPid;

extern PFCtempData_t PFCtempDataStr;
extern const ParamPid_t vLoopPid;
extern const ParamPid_t iLoopPid;
extern void Asw_LoopCtrl_BurstCtrl(void);
extern void Cur_LoopCtrl_FeedCal(void);
extern void Asw_LoopCtrl_IrefCal(void);
extern void Asw_Samping_VinRmsCalc(char channel);
extern void Asw_LoopCtrl_DCMFeedCal(void) ;


extern void notchRef(float flv,_filter_Deal_t *p);
extern float SecondOrderForm(float input, _filter_Deal_t *p);
extern void Compensator_Pid(ParamPid_t *p, float err);
extern void Cur_LoopCtrl_FeedCal(void);
extern void VbusLimit(void);
extern void VbusUVP(void);
extern void Syn_Signal_Calc(void);

extern void Asw_DCMFeedCal(void);
extern void VloopKpKiCal(void);
extern void VrefCal(void);
extern void VoltageLoop(void);
extern float oneOrderForm(float input, filter_str *p);
extern const float lingdian[10];
extern void iLoop_Control(void);
extern void vBusContrl(void);

#endif 
