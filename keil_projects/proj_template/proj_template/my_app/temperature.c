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

static int8 temperTable[TEMPER_TABLE_MAX_LEN];	// 存放历史温度数据
static uint16 temperCnt;	// 采温度完成次数
static uint8 temperTimerCnt;	// 定时器回调进入计数

/**
 * @brief 保存温度值到temperTable,同时更新temperCnt
 * @param value 温度值
 */
static void saveTemperValue(int8 value)
{
	temperTable[temperCnt % TEMPER_TABLE_MAX_LEN] = value;
	temperCnt++;
}

// 温度测量范围应由硬件给出电压电阻公式判断,没有运放,即没有限制
#define TEMPER_VALUE_MAX_CFG	200
#define TEMPER_VALUE_MIN_CFG	200
/**
 * @brief 真实温度 （摄氏度） 转 TemperValue，超过最大最小显示特殊值
 * @param value 真实温度
 * @return int8 返回TemperValue
 */
static inline int8 C_TO_TEMPER_VALUE(float value)
{
	int32 t;
	t = (int32)round((value - ZERO_TEMPER_VALUE_C) / PRECISION_TEMPER_VALUE_C);
	// if(t > TEMPER_VALUE_MAX_CFG)
	// 	t = TEMPER_VALUE_OVER_MAX;
	// else if(t < TEMPER_VALUE_MIN_CFG)
	// 	t = TEMPER_VALUE_LESS_MIN;
	return (int8)t;
}

// ntcB值和25度对应的阻值(待改)
#define NTC_B_VALUE_CFG 4250
#define NTC_25_C_RES_CFG 100000
/**
 * @brief adc电压值到温度值的转换
 * @attention 温度值不是真实温度，是int8类型的，真实温度等于ZERO_TEMPER_VALUE_C+温度值*PRECISION_TEMPER_VALUE_C
 * @param voltage adc电压值(V)
 * @return int8 返回温度值
 */
static int8 adcVoltageToTemperValue(float voltage)
{
	float t;	// 摄氏度(°C)
	float r;	// 某温度时ntc实际阻值(Ohm)
	int8 temperValue;

	r = 1000 * (1500 - 200 * voltage) / (8.75 + 3 * voltage);	// 运放分析采集的电压与ntc阻值关系(待改)
	t = 1.0 / ( 1.0 / ( 273.15 + 25 ) + 1.0 / NTC_B_VALUE_CFG * log( r / ( NTC_25_C_RES_CFG ) ) ) - 273.15;
	temperValue = C_TO_TEMPER_VALUE(t);
	return temperValue;
}

#define TEMPER_VALUE_ERROR -127
/**
 * @brief 获取温度值
 * @param cnt 第几次采样
 * @return int8 温度值，0表示ZERO_TEMPER_VALUE_C度，返回TEMPER_VALUE_ERROR表示错误
 * @note 真实温度等于ZERO_TEMPER_VALUE_C+返回值*PRECISION_TEMPER_VALUE_C
 */
int8 temper_getTemperValue(uint16 cnt)
{
	if(cnt > temperCnt || cnt == 0 ||
	   (temperCnt > TEMPER_TABLE_MAX_LEN && temperCnt - cnt >= TEMPER_TABLE_MAX_LEN))	// 判断是否近TEMPER_TABLE_MAX_LEN次
		return TEMPER_VALUE_ERROR;
	return temperTable[(cnt - 1) % TEMPER_TABLE_MAX_LEN];
}

/**
 * @brief 获取采样完成次数
 * @return uint16 返回采样次数
 */
uint16 temper_getTemperCnt(void)
{
	return temperCnt;
}

#define VREF_VOLTAGE 1.25	/*有一个adc通道采集一个参考电压，用于校准温漂导致的adc误差*/
/**
 * @brief 每调用SAMPLE_TEMPER_PERIOD次，阻塞采一次温度，后存储
 * @note 定时器回调每1min调一次此接口
 */
void temper_sampleTemperTimerCb(void)
{
	float v;
	int8 t;
	
	if(++temperTimerCnt >= SAMPLE_TEMPER_PERIOD)
		temperTimerCnt = 0;	// 每过SAMPLE_TEMPER_PERIOD次真正采样一次
	else
		return;
	
	mcu_gpio_en_ldo(TRUE);
	mcu_adc_user_init();
	while(!mcu_adc_main());	// 阻塞到采样完成
	mcu_gpio_en_ldo(FALSE);
	v = mcu_adc_get_voltage(MCU_P12_ADC_CH2);
#if ADC_VREF_CALIBRATION_EN
	float vref;
	vref = mcu_adc_get_voltage(MCU_P13_ADC_CH3);
	v = v * VREF_VOLTAGE / vref;	// 校准温漂后的v
#endif
	t = adcVoltageToTemperValue(v);
	saveTemperValue(t);
}

/**
 * @brief 阻塞采一次温度，不存储
 */
int8 temper_sampleTemper(void)
{
	float v;
	int8 t;

	mcu_gpio_en_ldo(TRUE);
	mcu_adc_user_init();
	while(!mcu_adc_main());	// 阻塞到采样完成
	mcu_gpio_en_ldo(FALSE);
	v = mcu_adc_get_voltage(MCU_P12_ADC_CH2);
#if ADC_VREF_CALIBRATION_EN
	float vref;
	vref = mcu_adc_get_voltage(MCU_P13_ADC_CH3);
	v = v * VREF_VOLTAGE / vref;	// 校准温漂后的v
#endif
	t = adcVoltageToTemperValue(v);
	return t;
}

/**
 * @brief 初次上电ram初始化，唤醒不需要调用
 */
void temper_resetInit(void)
{
	temperCnt = 0;
	temperTimerCnt = 0;
	memset(&g_temperReadCfg, 0, sizeof(g_temperReadCfg));
	memset(temperTable, 0, sizeof(temperTable));
	temper_sampleTemperTimerCb();	// 阻塞采集一次温度
}
