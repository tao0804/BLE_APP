#ifndef __MCU_HAL_H_
#define __MCU_HAL_H_

#include "panble.h"


////////////////////////////////////////////gpio_user/////////////////////////////////////////////
void mcu_gpio_user_init(void);
void mcu_gpio_en_ldo(bool en);
void mcu_gpio_light_led(bool light);
void mcu_gpio_en_pow(bool en);
bool mcu_gpio_key_pressed(void);


////////////////////////////////////////////adc_driver/////////////////////////////////////////////
typedef enum{
	ADC_CH00 = 0,
	ADC_CH01,
	ADC_CH02,
	ADC_CH03,
	ADC_CH04,
	ADC_CH05,
	ADC_CH06,
	ADC_CH07,
}ADC_CHANNEL;
#pragma pack(1)
typedef struct{
	ADC_CHANNEL		adcChannel;
	uint16			filterLen;
	float			convRate;		// 转换率，adcCode * convRate = voltage
	GPIO_T			*gpio;
	uint32			pinMask;
	__IO uint32_t	*mfpReg;
	uint32			mfpMask;
	uint32			mfpAdcCh;
	//无需配置
	uint16			adcCode;
	uint32			adcCodeSum;
	uint16			filterCnt;
	uint16			rollCount;
	float			voltage;
}MCU_ADC_TAB;
#pragma pack()
void mcu_adc_init(MCU_ADC_TAB *p_table, uint8 tableNum);
void mcu_adc_deinit(void);
float mcu_adc_get_voltage(uint8 index);
int8 mcu_adc_main(void);
void mcu_adc_isr(void);

////////////////////////////////////////////adc_user/////////////////////////////////////////////
enum {
	MCU_P14_ADC_CH4 = 0,
	MCU_P31_ADC_CH7,
	MCU_P12_ADC_CH2,
	MCU_P13_ADC_CH3,
	MCU_ADC_NUM,
};
void mcu_adc_user_init(void);

#endif //__MCU_HAL_H_
