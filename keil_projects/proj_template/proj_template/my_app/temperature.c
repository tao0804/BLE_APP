#include "temperature.h"
#include "mcu_hal.h"
#include "panip_config.h"
#include <string.h>
#include <math.h>

const TemperCfg_t g_temperCfg = {
	.zeroTemperValue = ZERO_TEMPER_VALUE_C * 1000,
	.precisionTemperValue = PRECISION_TEMPER_VALUE_C * 1000,
	.temperTableMaxLen = TEMPER_TABLE_MAX_LEN,
	.sampleTemperPeriod = SAMPLE_TEMPER_PERIOD,
};

TemperReadCfg_t g_temperReadCfg;

static int8 temperTable[TEMPER_TABLE_MAX_LEN];	// �����ʷ�¶�����
static uint16 temperCnt;	// ���¶���ɴ���
static uint8 temperTimerCnt;	// ��ʱ���ص��������

// �����¶�ֵ��temperTable,ͬʱ����temperCnt
static void saveTemperValue(int8 value)
{
	temperTable[temperCnt % TEMPER_TABLE_MAX_LEN] = value;
	temperCnt++;
}

// �¶Ȳ�����ΧӦ��Ӳ��������ѹ���蹫ʽ�ж�,û���˷�,��û������
#define TEMPER_VALUE_MAX_CFG	200
#define TEMPER_VALUE_MIN_CFG	200
#define TEMPER_VALUE_OVER_MAX   127
#define TEMPER_VALUE_LESS_MIN   -128
// ��ʵ�¶�(���϶�)תint8 TemperValue
static inline int8 C_TO_TEMPER_VALUE(float value)
{
	int32 t;
	t = (int32)round((value - ZERO_TEMPER_VALUE_C) / PRECISION_TEMPER_VALUE_C);
	if(t > TEMPER_VALUE_MAX_CFG)
		t = TEMPER_VALUE_OVER_MAX;
	else if(t < TEMPER_VALUE_MIN_CFG)
		t = TEMPER_VALUE_LESS_MIN;
	return (int8)t;
}

// ntcBֵ��25�ȶ�Ӧ����ֵ
#define NTC_B_VALUE_CFG 4250
#define NTC_25_C_RES_CFG 100000

// adc��ѹֵ��int8�¶�ֵ��ת��
static int8 adcVoltageToTemperValue(float voltage)
{
	float t;	// ���϶�(��C)
	float r;	// ĳ�¶�ʱntcʵ����ֵ(Ohm)
	int8 temperValue;

	r = 1000 * (1500 - 200 * voltage) / (8.75 + 3 * voltage);	// �˷ŷ����ɼ��ĵ�ѹ��ntc��ֵ��ϵ(����)
	t = 1.0 / ( 1.0 / ( 273.15 + 25 ) + 1.0 / NTC_B_VALUE_CFG * log( r / ( NTC_25_C_RES_CFG ) ) ) - 273.15;
	temperValue = C_TO_TEMPER_VALUE(t);
	return temperValue;
}

static float adcVoltageToFloatTemperValue(float Vcc, float Vtemp)
{
	float t;
	float vcc = Vcc * 2;
	float r = Vtemp * 100000 / (vcc - Vtemp);
	t = 1.0 / ( 1.0 / ( 273.15 + 25 ) + 1.0 / NTC_B_VALUE_CFG * log( r / ( NTC_25_C_RES_CFG ) ) ) - 273.15;
	return t;
}


#define TEMPER_VALUE_ERROR -127

// ��ȡint8 �¶�ֵ,0��ʾZERO_TEMPER_VALUE_C��,����TEMPER_VALUE_ERROR��ʾ����
// ��ʵ�¶ȵ���ZERO_TEMPER_VALUE_C+����ֵ*PRECISION_TEMPER_VALUE_C
int8 temper_getTemperValue(uint16 cnt)
{
	// �ж��Ƿ��TEMPER_TABLE_MAX_LEN��
	if(cnt > temperCnt || cnt == 0 ||
	   (temperCnt > TEMPER_TABLE_MAX_LEN && temperCnt - cnt >= TEMPER_TABLE_MAX_LEN))
		return TEMPER_VALUE_ERROR;
	return temperTable[(cnt - 1) % TEMPER_TABLE_MAX_LEN];
}

// ��ȡ������ɴ���
uint16 temper_getTemperCnt(void)
{
	return temperCnt;
}

#define VREF_VOLTAGE 1.25	/*��һ��adcͨ���ɼ�һ���ο���ѹ,����У׼��Ư���µ�adc���,ǰ��1.25��׼*/

// ÿ����SAMPLE_TEMPER_PERIOD��,������һ���¶�,��洢
void temper_sampleTemperTimerCb(void)
{
	float v;
	int8 t;
	
	if(++temperTimerCnt >= SAMPLE_TEMPER_PERIOD)
		temperTimerCnt = 0;	// ÿ��SAMPLE_TEMPER_PERIOD����������һ��
	else
		return;
	
	mcu_gpio_en_ldo(TRUE);
	mcu_adc_user_init();
	while(!mcu_adc_main());	// �������������
	mcu_gpio_en_ldo(FALSE);
	v = mcu_adc_get_voltage(MCU_P12_ADC_CH2);
#if ADC_VREF_CALIBRATION_EN
	float vref;
	vref = mcu_adc_get_voltage(MCU_P13_ADC_CH3);
	v = v * VREF_VOLTAGE / vref;	// У׼��Ư���v
#endif
	t = adcVoltageToTemperValue(v);
	saveTemperValue(t);
}

// ������һ���¶�,���洢
float temper_sampleTemper(void)
{
	float v1,v2;
	int8 t;

	mcu_gpio_en_ldo(TRUE);
	mcu_adc_user_init();
	while(!mcu_adc_main());	// �������������
	mcu_gpio_en_ldo(FALSE);
	v1 = mcu_adc_get_voltage(MCU_P31_ADC_CH7);
	v2 = mcu_adc_get_voltage(MCU_P14_ADC_CH4);
#if ADC_VREF_CALIBRATION_EN
	float vref;
	vref = mcu_adc_get_voltage(MCU_P13_ADC_CH3);
	v = v * VREF_VOLTAGE / vref;	// У׼��Ư���v
#endif
	t = adcVoltageToFloatTemperValue(v1, v2);
	return t;
}

// �����ϵ�ram��ʼ��,���Ѳ���Ҫ����
void temper_resetInit(void)
{
	temperCnt = 0;
	temperTimerCnt = 0;
	memset(&g_temperReadCfg, 0, sizeof(g_temperReadCfg));
	memset(temperTable, 0, sizeof(temperTable));
	// temper_sampleTemperTimerCb();	// �����ɼ�һ���¶�
}
