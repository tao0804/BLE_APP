#include "mcu_hal.h"
#include "adc.h"
#include "stack_svc_api.h"

#define ARRAY_NUM(arr)		(sizeof(arr)/sizeof((arr)[0]))


////////////////////////////////////////////gpio_user/////////////////////////////////////////////
void mcu_gpio_user_init(void)
{
// 	// gpio init
// 	GPIO_PullUp(P1, BIT0, GPIO_PULLUP_ENABLE);
// 	GPIO_ENABLE_DIGITAL_PATH(P1, BIT0);
// 	SYS->P1_MFP &= ~(SYS_MFP_P10_Msk);
// 	SYS->P1_MFP |= SYS_MFP_P10_GPIO;
// 	GPIO_InitOutput(P1, BIT0, GPIO_LOW_LEVEL);	// 需注意：原本库的GPIO_LOW_LEVEL有误，正确应该是 GPIO_LOW_LEVEL=0
// 	P10 = 0;	// ldo 默认不开

// 	GPIO_PullUp(P1, BIT4, GPIO_PULLUP_ENABLE);
// 	GPIO_ENABLE_DIGITAL_PATH(P1, BIT4);
// 	SYS->P1_MFP &= ~(SYS_MFP_P14_Msk);
// 	SYS->P1_MFP |= SYS_MFP_P14_GPIO;
// 	GPIO_SetMode(P1, BIT4, GPIO_MODE_OUTPUT);	// 需低功耗io保持
// //	GPIO_InitOutput(P1, BIT4, GPIO_HIGH_LEVEL);	// 需注意：原本库的GPIO_HIGH_LEVEL有误，正确应该是 GPIO_HIGH_LEVEL=1
// //	P14 = 1;	// led 默认不亮

// 	GPIO_PullUp(P1, BIT5, GPIO_PULLUP_ENABLE);
// 	GPIO_ENABLE_DIGITAL_PATH(P1, BIT5);
// 	SYS->P1_MFP &= ~(SYS_MFP_P15_Msk);
// 	SYS->P1_MFP |= SYS_MFP_P15_GPIO;
// 	GPIO_SetMode(P1, BIT5, GPIO_MODE_OUTPUT);	// 需低功耗io保持
// 	// GPIO_InitOutput(P1, BIT5, GPIO_HIGH_LEVEL);	// P15 是 POW_EN 开机后要一直拉高，关机时拉低

// 	GPIO_PullUp(P3, BIT0, GPIO_PULLUP_DISABLE);	// 外部下拉，关闭内部上拉防止漏电流
// 	GPIO_ENABLE_DIGITAL_PATH(P3, BIT0);
// 	SYS->P1_MFP &= ~(SYS_MFP_P30_Msk);
// 	SYS->P1_MFP |= SYS_MFP_P30_GPIO;
// 	GPIO_SetMode(P3, BIT0, GPIO_MODE_INPUT);	// P30 是 KEY 高电平为按下

	GPIO_PullUp(P1, BIT0, GPIO_PULLUP_ENABLE);
	GPIO_ENABLE_DIGITAL_PATH(P1, BIT0);
	SYS->P1_MFP &= ~(SYS_MFP_P10_Msk);
	SYS->P1_MFP |= SYS_MFP_P10_GPIO;
	GPIO_SetMode(P1, BIT0, GPIO_MODE_OUTPUT);	// 需低功耗io保持

}

void mcu_gpio_en_ldo(bool en)
{;
	// if(en)
	// 	P10 = 1;	// ldo en 拉高
	// else
	// 	P10 = 0;	// ldo en 拉低
}

void mcu_gpio_light_led(bool light)
{
	// // 注意：使用P14宏操作Pxn_PDIO寄存器低功耗IO保持才可以，使用GPIO_SetBits()和GPIO_ClearBits()接口操作DOUT寄存器会导致低功耗IO保持异常
	// if(light)
	// 	P14 = 0;	// led 亮
	// else
	// 	P14 = 1;	// led 灭
	if(light)
		P10 = 0;	// led 亮
	else
		P10 = 1;	// led 灭
}

void mcu_gpio_en_pow(bool en)
{
	if(en)
		P15 = 1;	// POW_EN 拉高保持供电
	else
		P15 = 0;	// POW_EN 拉低下电
}

bool mcu_gpio_key_pressed(void)
{
	if(GPIO_GET_IN_DATA(P3) & BIT0)
		return TRUE;	// 按下
	else
		return FALSE;	// 未按下
}

////////////////////////////////////////////adc_driver/////////////////////////////////////////////
static uint8 mcuAdcTableNum = 0;
static MCU_ADC_TAB * p_mcuAdcTable = NULL;
static volatile uint16 mcuAdcUserChIdx = 0;	// 当前采集点的Table索引信息，p_mcuAdcTable[mcuAdcUserChIdx]
static bool mcuAdcIsInited = FALSE;

/**
 * @brief 开启指定通道的单次转换
 * @param channel adc通道
 */
static void mcu_adc_start_channel_convert(ADC_CHANNEL channel)
{
    ADC_Open(ADC, 0, 0, 0x01 << channel);	// Enable channel
	ADC_START_CONV(ADC);
}

/**
 * @brief adc初始化
 * @param p_table adc配置表地址
 * @param tableNum adc配置表元素个数
 */
void mcu_adc_init(MCU_ADC_TAB *p_table, uint8 tableNum)
{
	uint8 i = 0;
	mcuAdcTableNum = tableNum;
	p_mcuAdcTable = p_table;

	// gpio init
	for(i = 0; i < mcuAdcTableNum; i++)
	{
		// 需要外部使能gpio的时钟
		GPIO_SetMode(p_table[i].gpio, p_table[i].pinMask, GPIO_MODE_INPUT);
		GPIO_PullUp(p_table[i].gpio, p_table[i].pinMask, GPIO_PULLUP_DISABLE);
		GPIO_DISABLE_DIGITAL_PATH(p_table[i].gpio, p_table[i].pinMask);
		*p_table[i].mfpReg &= ~p_table[i].mfpMask;
		*p_table[i].mfpReg |= p_table[i].mfpAdcCh;
	}
	
	// 注册中断处理函数到协议栈，否则中断会卡死。
	((interrupt_register_handler)SVC_interrupt_register)(ADC_IRQ, mcu_adc_isr);

	CLK_EnableModuleClock(ADC_MODULE);
    // Select ADC input range.1 means 0.4V~2.4V ;0 means 0.4V~1.4V.
    // 0.4V~2.4V & 0.4V~1.4V both are theoretical value,the real range is determined by bandgap voltage.
    ADC_SelInputRange(ADC_INPUTRANGE_HIGH);
	// Set Sample Time 16_ADC_CLOCK
	ADC_SetExtraSampleTime(ADC, 0, ADC_SAMPLE_CLOCK_16);
    // Power on ADC
    ADC_POWER_ON(ADC);
    // Enable ADC convert complete interrupt
    ADC_EnableInt(ADC, ADC_ADIF_INT);
    NVIC_EnableIRQ(ADC_IRQn);

	mcuAdcUserChIdx = 0;
	mcu_adc_start_channel_convert(p_mcuAdcTable[mcuAdcUserChIdx].adcChannel);
	mcuAdcIsInited = TRUE;
}

/**
 * @brief adc反初始化
 */
void mcu_adc_deinit(void)
{
	CLK_DisableModuleClock(ADC_MODULE);
    ADC_POWER_DOWN(ADC);
    NVIC_DisableIRQ(ADC_IRQn);
	mcuAdcIsInited = FALSE;
}

/**
 * @brief adc获取电压值
 * 
 * @param index adc配置表中的元素索引
 * @return float 电压值 unit:V
 */
float mcu_adc_get_voltage(uint8 index)
{
	if(NULL==p_mcuAdcTable || index>=mcuAdcTableNum)
		return 0;
	return p_mcuAdcTable[index].voltage;
}

#define ADC_SAMPLE_TIMES_CFG 20
/**
 * @brief adc采集一轮后求平均、算电压，开启下一轮
 * @return 返回0表示未结束，返回-1表示异常，返回1表示采样结束
 * @note 所有通道采完平均过ADC_SAMPLE_TIMES_CFG次后会关ADC，低功耗需求
 */
int8 mcu_adc_main(void)
{
	uint8 i = 0;
	bool adcReady = TRUE;
	
	if(NULL == p_mcuAdcTable)
		return -1;	// 未初始化
	if(FALSE == mcuAdcIsInited)
		return 1;	// 已关闭

	if(mcuAdcUserChIdx >= mcuAdcTableNum)
	{
		for(i = 0; i < mcuAdcTableNum; i++)
		{
			p_mcuAdcTable[i].filterCnt++;
			p_mcuAdcTable[i].adcCodeSum += p_mcuAdcTable[i].adcCode;
			if(p_mcuAdcTable[i].filterCnt >= p_mcuAdcTable[i].filterLen)	// 是否采够求平均的累加次数
			{
				p_mcuAdcTable[i].voltage = p_mcuAdcTable[i].adcCodeSum / p_mcuAdcTable[i].filterCnt * p_mcuAdcTable[i].convRate;
				p_mcuAdcTable[i].rollCount++;	// 每平均1次rc++
				p_mcuAdcTable[i].filterCnt = 0;
				p_mcuAdcTable[i].adcCodeSum = 0;
			}
		}
		mcuAdcUserChIdx = 0;
		for(i = 0; i < mcuAdcTableNum; i++)
			if(p_mcuAdcTable[i].rollCount < ADC_SAMPLE_TIMES_CFG)	// 采样不够配置的次数
				adcReady = FALSE;
		if(adcReady == FALSE)
			mcu_adc_start_channel_convert(p_mcuAdcTable[mcuAdcUserChIdx].adcChannel);
		else {
			mcu_adc_deinit();	// 所有通道已采完，关ADC
			return 1;
		}
	}
	return 0;
}

/**
 * @brief adc中断回调
 */
void mcu_adc_isr(void)
{
	uint16 adcCode = ADC_GET_CONVERSION_DATA(ADC, 0);	// 读取转换结果
	uint32 flag = ADC_GET_INT_FLAG(ADC, ADC_ADIF_INT);	// 读中断标志位
	ADC_CLR_INT_FLAG(ADC, flag);	// 清中断

	if(NULL==p_mcuAdcTable)
		return;
	
	p_mcuAdcTable[mcuAdcUserChIdx].adcCode = adcCode;
	mcuAdcUserChIdx++;
	if(mcuAdcUserChIdx < mcuAdcTableNum)
	{
		mcu_adc_start_channel_convert(p_mcuAdcTable[mcuAdcUserChIdx].adcChannel);
	}
}


////////////////////////////////////////////adc_user/////////////////////////////////////////////
#define MCU_AVDD_CFG 2.5f
const MCU_ADC_TAB adcCfgTable[] = {
	{ADC_CH02, 10, MCU_AVDD_CFG/4096, P1, BIT2, &SYS->P1_MFP, SYS_MFP_P12_Msk, SYS_MFP_P12_ADC_CH2},
	{ADC_CH03, 10, MCU_AVDD_CFG/4096, P1, BIT3, &SYS->P1_MFP, SYS_MFP_P13_Msk, SYS_MFP_P13_ADC_CH3},
};
MCU_ADC_TAB adcTable[ARRAY_NUM(adcCfgTable)];

void mcu_adc_user_init(void)
{
	// init global var
	memset(adcTable, 0, sizeof(adcTable));
	memcpy(adcTable, adcCfgTable, sizeof(adcTable));
	// adc init
	mcu_adc_init(adcTable, ARRAY_NUM(adcTable));
}
