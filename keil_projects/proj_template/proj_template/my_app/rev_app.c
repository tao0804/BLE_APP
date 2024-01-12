#include "rev_app.h"
#include "gpio.h"
#include "timer.h"
#include "led_app.h"
#include "stack_svc_api.h"


// static void rev_freqMeasureTimer1_off(void)
// {
// 	if(TIMER_IS_ACTIVE(TIMER0))
// 	{
// 		TIMER_Close(TIMER0);	// TIMER_T *timer
// 	}
// }
// static void rev_freqMeasureTimer1_on(void)
// {
// 	rev_freqMeasureTimer1_off();
// 	TIMER_Open(TIMER0, TIMER_ONESHOT_MODE, 100);
// }

RevArg_t revArg;

static void mcu_rev_gpio_init(void)
{
	GPIO_PullUp(P1, BIT2, GPIO_PULLUP_DISABLE);
	GPIO_ENABLE_DIGITAL_PATH(P1, BIT2);
	SYS->P1_MFP &= ~(SYS_MFP_P12_Msk);
	SYS->P1_MFP |= SYS_MFP_P12_GPIO;
	GPIO_InitOutput(P1, BIT2, GPIO_LOW_LEVEL);
	P12 = 0;	// HALL_EN 默认不开

	GPIO_PullUp(P1, BIT3, GPIO_PULLUP_ENABLE);	// 可能会有漏电流
	GPIO_ENABLE_DIGITAL_PATH(P1, BIT3);
	SYS->P1_MFP &= ~(SYS_MFP_P13_Msk);
	SYS->P1_MFP |= SYS_MFP_P13_GPIO;
	GPIO_SetMode(P1, BIT3, GPIO_MODE_INPUT);
}

void mcu_gpio_en_hall(bool en)
{
	if(en)
		P12 = 1;	// HALL传感器开启
	else
		P12 = 0;	// HALL传感器不开
}

void mcu_gpio01_isr(void)
{
	// GPIO_GET_INT_FLAG(P1, BIT3) will return 0 or 8
	if(GPIO_GET_INT_FLAG(P1, BIT3))
	{
		led_setMode(LED_MODE_ON);
		GPIO_CLR_INT_FLAG(P1, BIT3);
		if(revArg.revFlag)
		{
			revArg.revTimerCnt = TIMER_GetCounter(TIMER0);
			revArg.revTime = revArg.revTimerCnt / (TIMER_GetModuleClock(TIMER0) / 1000);	// psc = 1
			revArg.targetRPM = 60 * 1000 / revArg.revTime;	// 目标每分钟转速
			TIMER_Reset(TIMER0);
		}
		else
		{
			TIMER_EnableInt(TIMER0);
			TIMER_Start(TIMER0);
		}
		revArg.revFlag = !revArg.revFlag;
	}
}

void mcu_TMR0_isr(void)
{
	// indicates Timer time-out interrupt occurred or not
	if(TIMER_GetIntFlag(TIMER0))
	{
		TIMER_Reset(TIMER0);
		TIMER_ClearIntFlag(TIMER0);
		revArg.revFlag = !revArg.revFlag;
	}
}

void mcu_rev_init(void)
{
	memset(&revArg, 0, sizeof(RevArg_t));
	mcu_rev_gpio_init();
	GPIO_EnableInt(P1, 3, GPIO_INT_FALLING);
    NVIC_DisableIRQ(GPIO01_IRQn);	// 检测开启后再去使能
	NVIC_SetPriority(GPIO01_IRQn, 2);
	((interrupt_register_handler)SVC_interrupt_register)(GPIO01_IRQ, mcu_gpio01_isr);

	CLK_EnableModuleClock(TMR0_MODULE);
	CLK_SetModuleClock(TMR0_MODULE, CLK_CLKSEL1_TMR0SEL_HCLK);
	TIMER_Reset(TIMER0);
	// Give a dummy target frequency here. Will over write capture resolution with macro
	TIMER_Open(TIMER0, TIMER_ONESHOT_MODE, 38000); 
	// Update prescale to set proper resolution.
	TIMER_SET_PRESCALE_VALUE(TIMER0, 0);
	// Set compare value as large as possible, so don't need to worry about counter overrun too frequently.
	TIMER_SET_CMP_VALUE(TIMER0, 0xFFFFFF);

	NVIC_EnableIRQ(TMR0_IRQn);
	((interrupt_register_handler)SVC_interrupt_register)(TMR0_IRQ, mcu_TMR0_isr);
}



void rev_start(void)
{
	mcu_gpio_en_hall(TRUE);
	NVIC_EnableIRQ(GPIO01_IRQn);
}

