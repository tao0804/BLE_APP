/**
 * @file key_app.c
 * @note 检测按键按下后开启定时器，每100ms检测一次按键是否还被按下，
 *       到按键松开2s内无按键按下关闭定时器，2s内继续按下算连按，
 *       能记录最后一次的按下时间，和总的按下次数，
 *       检测到特殊事件后能回调用户提供的处理函数。
 *       检测周期和连按最大间隔时间可由用户配置。
 */
#include "key_app.h"
#include "mcu_hal.h"
#include "stack_svc_api.h"

#define KEY_PRESSED     TRUE     // 按下
#define KEY_NOT_PRESSED FALSE    // 未按下

KeyInfo_t keyInfo;
static KeyCfg_t *pKeyCfg;

static void key_scanTimerOff(void)
{
	// 查是否开启，如果开启则关闭
	if (((ke_timer_active_handler)SVC_ke_timer_active)(APP_KEY_SCAN_TIMER, TASK_APP)) {
		((ke_timer_clear_handler)SVC_ke_timer_clear)(APP_KEY_SCAN_TIMER, TASK_APP);
	}
	keyInfo.taskStatus = FALSE;
}

static void key_scanTimerOn(uint16 time)
{
	key_scanTimerOff();    // 防止之前开着定时器，先尝试关
	((ke_timer_set_handler)SVC_ke_timer_set)(APP_KEY_SCAN_TIMER, TASK_APP, time);
	keyInfo.taskStatus = TRUE;
}

static void key_scan(void)
{
	if (mcu_gpio_key_pressed()) {
		keyInfo.keyStatus = KEY_PRESSED;
	} else {
		keyInfo.keyStatus = KEY_NOT_PRESSED;
	}
}

void key_init(KeyCfg_t *pCfg)
{
	pKeyCfg = pCfg;
}

/**
 * @brief 监测首次按下
 */
void key_monitorPress(void)
{
	// 如果按键周期扫描已开启，无需检测首次按下
	if (keyInfo.taskStatus)
		return;
	key_scan();
	if (keyInfo.keyStatus == KEY_PRESSED) {
		// 首次按下 开启扫描按键定时器
		key_scanTimerOn(pKeyCfg->timerPeriod);
		keyInfo.pressTime = 0;      // 按下时间清0
		keyInfo.releaseTime = 0;    // 释放时间清0
		keyInfo.pressCnt = 0;       // 连按次数清0
		pKeyCfg->keyEventCB(KEY_PRESS, keyInfo.pressCnt);
	}
}

void key_taskReset(void)
{
	key_scanTimerOff();
	memset(&keyInfo, 0, sizeof(KeyInfo_t));
}

void key_scanTimerCB(void)
{
	uint8 lastKeyStatus = keyInfo.keyStatus;
	uint16 lastPressTime = keyInfo.pressTime;
	uint16 lastReleaseTime = keyInfo.releaseTime;
	bool timerContinue = TRUE;

	key_scan();
	if (lastKeyStatus != keyInfo.keyStatus) {
		if (keyInfo.keyStatus == KEY_PRESSED) {
			// 检测到按下
			keyInfo.pressTime = 0;
			keyInfo.pressCnt++;
			pKeyCfg->keyEventCB(KEY_PRESS, keyInfo.pressCnt);
		} else {
			// 检测到释放
			keyInfo.releaseTime = 0;
			pKeyCfg->keyEventCB(KEY_RELEASE, keyInfo.pressCnt);
		}
	}

	if (keyInfo.keyStatus == KEY_PRESSED) {
		keyInfo.pressTime += pKeyCfg->timerPeriod;
		if (keyInfo.pressTime >= 100 && lastPressTime < 100) {
			pKeyCfg->keyEventCB(KEY_PRESS_1SECS, keyInfo.pressCnt);
		} else if (keyInfo.pressTime >= 300 && lastPressTime < 300) {
			pKeyCfg->keyEventCB(KEY_PRESS_3SECS, keyInfo.pressCnt);
		}
	} else {
		keyInfo.releaseTime += pKeyCfg->timerPeriod;
		if (keyInfo.releaseTime >= pKeyCfg->pressIntervalTime && lastReleaseTime < pKeyCfg->pressIntervalTime) {
			timerContinue = FALSE;
			pKeyCfg->keyEventCB(KEY_SCAN_END, keyInfo.pressCnt);
		}
	}

	if (timerContinue) {
		key_scanTimerOn(pKeyCfg->timerPeriod);
	} else {
		key_scanTimerOff();
	}
}

////////////////////////////////////////////key_user/////////////////////////////////////////////
typedef enum KeyUserFlag {
	KEY_NO_FLAG = 0,
	KEY_POWER_ON_FLAG,
	KEY_3SECS_UNLOCK_FLAG,
	KEY_POWER_OFF_FLAG,
} KeyUserFlag_t;

typedef struct KeyUserInfo {
	uint8 runMode;    // 0:关机 1:开机
	KeyUserFlag_t flag;
} KeyUserInfo_t;

KeyUserInfo_t keyUserInfo;
static KeyCfg_t keyCfg;

void key_eventCB(KeyEvents_t event, uint8 pressCnt)
{
	// if (event == KEY_PRESSED && pressCnt == 0) {
	// 	// 首次按下清标志位
	// 	keyUserInfo.flag = KEY_NO_FLAG;
	// }

	// if (keyUserInfo.runMode) {
	// 	if (event == KEY_PRESS_3SECS && pressCnt == 0) {
	// 		// 长按3s触发解锁按键
	// 		keyUserInfo.flag = KEY_3SECS_UNLOCK_FLAG;
	// 	} else if (event == KEY_PRESS && pressCnt == 1) {
	// 		if (keyUserInfo.flag == KEY_3SECS_UNLOCK_FLAG) {
	// 			// 解锁后再按下就让POW_EN拉低，在用户释放按键时完全断电
	// 			keyUserInfo.flag = KEY_POWER_OFF_FLAG;
	// 			mcu_gpio_en_pow(FALSE);
	// 		}
	// 	} else if (event == KEY_RELEASE) {
	// 		if (keyUserInfo.flag == KEY_NO_FLAG) {
	// 			// 未解锁按键，在释放按键时重置按键
	// 			key_taskReset();
	// 		}
	// 	}
	// } else {
	// 	if (event == KEY_PRESS_1SECS) {
	// 		// 长按1s开机
	// 		mcu_gpio_en_pow(TRUE);
	// 		keyUserInfo.flag = KEY_POWER_ON_FLAG;
	// 	} else if (event == KEY_RELEASE) {
	// 		if (keyUserInfo.flag == KEY_NO_FLAG) {
	// 			// 未长按1s在释放按键时下电
	// 			mcu_gpio_en_pow(FALSE);
	// 		}
	// 	} else if (event == KEY_SCAN_END) {
	// 		if (keyUserInfo.flag == KEY_POWER_ON_FLAG) {
	// 			// 等按键扫描任务结束再置runMode，防止开机后立马又被按下
	// 			keyUserInfo.runMode = TRUE;
	// 		}
	// 	}
	// }
	
	if (event == KEY_PRESS_1SECS) {
		// 长按3s置关机flag
		keyUserInfo.flag = KEY_POWER_ON_FLAG;
	} else if (event == KEY_PRESS_3SECS) {
		// 长按3s置关机flag
		keyUserInfo.flag = KEY_POWER_OFF_FLAG;
	} else if (event == KEY_RELEASE && keyUserInfo.flag == KEY_POWER_OFF_FLAG) {
		// 松开时关机，防止一直按着，一关机就被唤醒
		sys_power_flag = 1;
		app_var.ext_wakeup_enable = 1;
		key_taskReset();
	} else if (event == KEY_RELEASE && keyUserInfo.flag == KEY_PRESS_1SECS) {
		// 长按1s开机
		sys_power_flag = 0;
		app_var.ext_wakeup_enable = 0;
		key_taskReset();
	}
}

void key_resetInit(void)
{
	keyCfg.keyEventCB = key_eventCB;
	keyCfg.pressIntervalTime = 200;
	keyCfg.timerPeriod = 10;
	key_init(&keyCfg);
	memset(&keyInfo, 0, sizeof(KeyInfo_t));

	memset(&keyUserInfo, 0, sizeof(KeyUserInfo_t));
}
