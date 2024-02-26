#include <stdio.h>
#include "arch.h"
#include "PN102Series.h"
#include "peripherals.h"

#include "dbg.h"
#include "panip.h"
#include "lld_evt.h"
#include "reg_ble_em_et.h"

#include "ke_env.h"
#include "gapm_int.h"
#include "gattm_int.h"
#include "l2cm_int.h"
#include "hci_int.h"
#include "lld_evt.h"
#include "lld.h"
#include "llm.h"
#include "prf.h"
#include "app_task.h"
#include "gapc_int.h"
#include "gapm_int.h"
#include "gattc_int.h"
#include "l2cc_int.h"
#include "llm_task.h"
#include "lld_sleep.h"
#include "app.h"
#include "panip_config.h"
#include "rf.h"
#include "panble.h"
#include "app_temp_adc.h"
#include "app_flash.h"
#include "dbg_sys.h"
#include "stack_svc_api.h"

#if (USER_PROJ_TEMPLATE)
#include "proj_template.h"
#endif

#if(PROJ_OTA)
#include "ota_server_task.h"
#include "ota_server.h"
#endif

#include "mcu_hal.h"
#include "led_app.h"
#include "key_app.h"
#include "rev_app.h"

const app_info_t app_info __attribute__((at(APP_INFO_ADDR)))=
{
	.co_default_bdname = "encoder",
	.default_use_ext_32k = USE_EXT_32K,
	.default_hz32000 = HZ32000,
	.default_en_calib_rc = EN_CALIB_RC,
	.default_calib_slot = CALIB_SLOTS,
	.default_calib_32k_int = 96000,
};

//wakeup init
void ble_sleep_wakeup_init()
{
	periph_init();																//System peripheral initialization
	/* 注意：mcu_gpio_user_init()放在periph_init()内部的最后一行io保持不可以，放在periph_init()执行完出来的下一行才行！*/
	mcu_gpio_user_init();	// 初始化gpio
	
	GLOBAL_INT_STOP();
	
	ble_master_tgsoft_rst_setf(1);
	while(ble_master_tgsoft_rst_getf());
	((lld_sleep_init_handler)SVC_lld_sleep_init)(0,0,0);                     		//Link layer sleep initialization
	((rf_pa_ramp_init_handler)SVC_rf_pa_ramp_init)();							//tx power init
	((ble_int_cfg_handler)SVC_ble_int_cfg)();                        			//Protocol stack interrupt configuration
	((lld_enable_cscnt_slp_int_handler)SVC_lld_enable_cscnt_slp_int)();       	//Enable interrupt configuration
	GLOBAL_INT_START();                   										//Enable global interrupt

	((ke_event_set_handler)SVC_ke_event_set)(KE_EVENT_BLE_EVT_START);			//Start ble event after wake up
	#if(EXT_WAKEUP)
	wakeup_cnt++;
	// printf("wk\n");
	#endif
}

__asm void stack_sp_restore(void)
{
	LDR     R6 ,=0x20003ffc
	LDR     R7 ,[R6,#0]
	MSR     MSP, R7
	NOP
	BX          LR   
}

//POWER UP init
void ble_normal_reset_init()
{
	sys_clear_global_var();
	periph_init();
	/* 注意：mcu_gpio_user_init()放在periph_init()内部的最后一行io保持不可以，放在periph_init()执行完出来的下一行才行！*/
	mcu_gpio_user_init();	// 初始化gpio
	mcu_gpio_en_pow(FALSE);	// 首次复位拉低POW_EN，需要长按1s后开机

	printf("CPU @ %dHz,%s\n", SystemCoreClock, app_info.co_default_bdname);
	#if(PROJ_OTA)
	bootloader_start();
	#endif
	
	((rc_internal_init_handler)SVC_rc_internal_init)();
	((bleip_init_handler)SVC_bleip_init)();						//RF and stack initialization
	
	#if(TEMP_CHANGE_CALIB)
	temperature_init();
	app_flash_init();
	#endif
	
	GLOBAL_INT_START();

	proj_template_ini();
	user_code_start(); 											//start ble stack
}

//init finish,ble stack process
void ble_stack_process()
{
	while (1)
	{
		#if(EXT_WAKEUP)
		// if(P52 == 1)				//检测P52高电平 
		// {
		// 	printf("P52 1\n");
		// }
		// else						//检测P52低电平 
		// {
		// 	if(wakeup_cnt >= 100)	//int 100ms.mcu work 10s，then into deep sleep
		// 	{
		// 		wakeup_cnt = 0;
		// 		printf("P52 0\n");
		// 		sys_power_flag = 1;
		// 	}
		// }
		#endif
		((bleip_schedule_handler)SVC_bleip_schedule)();
		((rc_check_period_calib_handler)SVC_rc_check_period_calib)();
		key_monitorPress();
		
		#if(TEMP_CHANGE_CALIB)
		rf_calibration_process();
		#endif
		
		if (app_var.default_sleep_en && !revArg.revFlag)
		{
			#if (GPIO_RETAIN_EN)
			GPIO_Store();
			#endif
			GLOBAL_INT_DISABLE();
			
			// Check if the processor clock can be gated
			if(((bleip_sleep_handler)SVC_bleip_sleep)())
			{
				CLK_PowerDown();
				#if(GPIO_RETAIN_EN)
				if(sys_sleep_flag == 1)
				{
					sys_sleep_flag = 0;
					((ke_event_set_handler)SVC_ke_event_set)(KE_EVENT_BLE_EVT_START);
					#if(EXT_WAKEUP)
					// wakeup_cnt++;
					printf("wk\n");
					#endif
				}
				#endif
			}
			// Checks for sleep have to be done with interrupt disabled
			GLOBAL_INT_RESTORE();
		}
	}
}



//system init
void ble_init(void)
{
	if(ble_deep_sleep_stat_getf())
	{
		ble_sleep_wakeup_init();
	}
	else
	{   
		ble_normal_reset_init();
	}
}

int main(void)
{
#if (GPIO_RETAIN_EN)
	GPIO_Retract();
#endif
	stack_sp_restore();
	ble_init();
	ble_stack_process();
}

/*** (C) COPYRIGHT 2016   Shanghai Panchip Microelectronics Co., Ltd.   ***/
