#ifndef __REV_APP_H__
#define __REV_APP_H__

#include "panble.h"


typedef struct RevArg{
	int32	revTimerCnt;	// Timer register CNT value
	float	revTime;		// CNT value corresponding actualtime
	int16	revFlag;		// execute TIMER_Reset or TIMER_Start
	uint32	targetRPM;		// target Revolutions Per minute
}RevArg_t;


extern RevArg_t revArg;
void mcu_gpio_en_hall(bool en);
void mcu_rev_init(void);
void rev_start(void);
void rev_resetInit(void);

#endif


