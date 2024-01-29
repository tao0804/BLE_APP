#ifndef __TEMPERATURE_H_
#define __TEMPERATURE_H_

#include "panble.h"
#include "stack_svc_api.h"


// �ɼ��¶����ڣ���λ(1min)
#define SAMPLE_TEMPER_PERIOD        1
// TemperTable�����С
#define TEMPER_TABLE_MAX_LEN		500
// TemperValue 0ֵ�¶� �����϶ȣ�
#define ZERO_TEMPER_VALUE_C			37.0f
// TemperValue ���� �����϶ȣ�
#define PRECISION_TEMPER_VALUE_C	0.05f
// TemperValue ת ��ʵ�¶� �����϶ȣ�
#define TEMPER_VALUE_TO_C(value)	(ZERO_TEMPER_VALUE_C + PRECISION_TEMPER_VALUE_C * value)
// �Ƿ���VrefУ׼adc�ɼ��¶�
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
	uint16 startCnt;	// �ӵڼ��β�����ʼ��ȡ
	uint16 readLen;		// ��ȡ�ĳ���
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
