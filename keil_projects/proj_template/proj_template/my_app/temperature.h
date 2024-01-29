#ifndef __TEMPERATURE_H_
#define __TEMPERATURE_H_

#include "panble.h"
#include "stack_svc_api.h"


// 采集温度周期，单位(1min)
#define SAMPLE_TEMPER_PERIOD        1
// TemperTable数组大小
#define TEMPER_TABLE_MAX_LEN		500
// TemperValue 0值温度 （摄氏度）
#define ZERO_TEMPER_VALUE_C			37.0f
// TemperValue 精度 （摄氏度）
#define PRECISION_TEMPER_VALUE_C	0.05f
// TemperValue 转 真实温度 （摄氏度）
#define TEMPER_VALUE_TO_C(value)	(ZERO_TEMPER_VALUE_C + PRECISION_TEMPER_VALUE_C * value)
// 是否开启Vref校准adc采集温度
#define ADC_VREF_CALIBRATION_EN 	(0)

#pragma pack(1)
typedef struct TemperCfg
{
	uint16 zeroTemperValue;
	uint16 precisionTemperValue;
	uint16 temperTableMaxLen;
	uint8 sampleTemperPeriod;
	float vccVoltage;
	float currentTemp;
}TemperCfg_t;

typedef struct TemperReadCfg
{
	uint16 startCnt;	// 从第几次采样开始读取
	uint16 readLen;		// 读取的长度
}TemperReadCfg_t;
#pragma pack()

static void connected_data_periodicTimerOff(void)
{
	if (((ke_timer_active_handler)SVC_ke_timer_active)(CONNECTED_DATA_PERIODIC_TIMER, TASK_APP)) {
		((ke_timer_clear_handler)SVC_ke_timer_clear)(CONNECTED_DATA_PERIODIC_TIMER, TASK_APP);
	}
}

static void connected_data_periodicTimerOn(uint16 time)
{
	connected_data_periodicTimerOff();
	((ke_timer_set_handler)SVC_ke_timer_set)(CONNECTED_DATA_PERIODIC_TIMER, TASK_APP, time);
}

extern const TemperCfg_t g_temperCfg;
extern TemperCfg_t temperCfgStructure;
extern TemperReadCfg_t g_temperReadCfg;

int8 temper_getTemperValue(uint16 cnt);
uint16 temper_getTemperCnt(void);
void temper_sampleTemperTimerCb(void);
void temper_sampleTemper(void);

void period_send_data(void);
void temper_resetInit(void);

#endif //__TEMPERATURE_H_
