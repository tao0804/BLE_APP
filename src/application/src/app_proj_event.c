/**
 ****************************************************************************************
 * @file    app_proj_event.c
 * @brief   
 
 * @version 0.01
 * @date    2018/10/15
 * @history 
 * @note	   
 * detailed description 
 
 ****************************************************************************************
 * @attention
 *
 * Copyright (C) 2016 Shanghai Panchip Microelectronics Co., Ltd. All rights reserved.
 ****************************************************************************************
 */


/**
 ****************************************************************************************
 * @addtogroup APP
 * @{
 ****************************************************************************************
 */
#include "panip_config.h"             // SW configuration
#include "string.h"
#if (BLE_APP_PRESENT)

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "app_sec.h"
#include "app_proj_event.h"
#include "gattc_task.h"
#include "co_math.h"                 // Common Maths Definition
#if(USER_PROJ_TEMPLATE || USER_PROJ_WECHAT || PROJ_THROUGHPUT || PROJ_MULTIROLE)
#include "proj_template.h" 
#include "app_proj_template.h" 
#if(USER_PROJ_WECHAT)
#include "mpbledemo2.h"
#endif
#endif

#if(PROJ_HID_KEYBOARD)
#include "app_hid_keyboard.h"
#include "proj_hid_keyboard.h"
#include "app_batt.h"
#include "app_dis.h"
#endif // (PROJ_HID_KEYBOARD)
#if(PROJ_HID_SELFIE)
#include "app_hid_selfie.h"
#include "proj_hid_selfie.h"
#include "app_batt.h"
#include "app_dis.h"
#endif // (PROJ_HID_SELFIE)
#if(USER_RGB_LIGHT)
#include "proj_rgb.h"  
#include "app_rgb_light1.h"
#include "app_rgb_light2.h"
#endif
#if(USER_PROJ_CENTRAL)
#include "tmp_client_task.h"   
#include "app_tmp.h"
#endif 
#if(PROJ_OTA)
#include "app_ota.h"
#endif
#include "ke_timer.h"
#include "app_task.h"
#include "stack_svc_api.h"

#include "temperature.h"

#if PROJ_TEMPER
#include "led_app.h"
#endif

#define Conn_Param_Update           1
#define DBG_APP_PROJ_EVENT(x)       //printf x

struct gap_bdaddr connect_bdaddr;
/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
*/

 /**
 ****************************************************************************************
 * @brief app_api function. Project's actions start from here
 *
 * @return void.
 ****************************************************************************************
*/

void app_init_ind_func(void)
{	 
	#if(USER_PROJ_TEMPLATE || USER_PROJ_WECHAT || PROJ_THROUGHPUT || PROJ_MULTIROLE)
	app_proj_template_init();
	#endif
	
	#if(PROJ_HID_KEYBOARD)
	app_hid_init();
    app_batt_init();
    app_dis_init();
	#endif // (PROJ_HID_KEYBOARD)
	
	#if(PROJ_HID_SELFIE)
    hid_selfie_ini();
	app_hid_init();
    app_batt_init();
    app_dis_init();
	#endif // (PROJ_HID_SELFIE)
	
	#if(BLE_APP_SEC)
    app_sec_init();
    #endif // (BLE_APP_SEC)
	
	#if(USER_RGB_LIGHT)
	app_rgb1_init();
	app_rgb2_init();
	#endif
	
	#if(USER_PROJ_CENTRAL)
	app_tmp_init();
	#endif
	
	#if(PROJ_OTA)
    app_ota_init();
    #endif
}


void app_reset_complete_ind_func(void)
{
	printf("reset_complete\n");
	appm_set_dev_configuration(NULL);
}

/**
 ****************************************************************************************
 * @brief app_api function. Called upon device's configuration completion
 *
 * @return void.
 ****************************************************************************************
*/

void app_set_dev_config_complete_ind_func(void)
{
/**************************************************
Handle device configuration complete event. Start required Profiles' Database procedure. 
If no profiles supported or database initialization is completed starts advertising. 
***************************************************/    
	printf("dev_cfg complete\n");

    // Add the first required service in the database
    if (appm_add_svc())
    {
        printf("svc added\n");
    }
    return;
}

/**
 ****************************************************************************************
 * @brief app_api function. Handles Database creation. Start application.
 *
 * @return void.
 ****************************************************************************************
*/

void app_db_init_complete_ind_func(void)
{
/**************************************************
Database created. Ready to start Application i.e. start advertise
***************************************************/        
	printf("CDB complete\n");
	if(app_env.func[APP_CB_ID_CREATE_DB_COMPLETED] !=NULL)
		app_env.func[APP_CB_ID_CREATE_DB_COMPLETED]();

#if(USER_PROJ_TEMPLATE || USER_PROJ_WECHAT || PROJ_THROUGHPUT || PROJ_MULTIROLE)
    proj_template_start_advertising();
    #if(USER_PROJ_WECHAT)
    mpbledemo2_init();
	#endif
#endif	
	
#if(PROJ_HID_KEYBOARD)
    hid_kbd_start_advertising();
#endif // (PROJ_HID_KEYBOARD)
	
#if(PROJ_HID_SELFIE)
    hid_selfie_start_advertising();
#endif // (PROJ_HID_SELFIE)
	
#if(USER_RGB_LIGHT)
	rgb_start_advertising();
#endif	
	
#if(USER_PROJ_CENTRAL)
	app_start_scan();
#endif	

    return;
}

//For slave mode ,recvd  con_req
void app_connection_req_ind_func ( uint8_t conidx, struct gapc_connection_req_ind const* param )
{
	printf("Slave Role Connected\n");

	if(app_env.func[APP_CB_ID_SLAVER_CONNECTED] !=NULL)
		app_env.func[APP_CB_ID_SLAVER_CONNECTED]();
	
	#if (PROJ_HID_KEYBOARD | PROJ_HID_SELFIE)
	//app_sec_send_security_req(conidx);
    app_hid_enable_prf(conidx);
	app_batt_enable_prf(conidx);
	#endif
    
	#if(PROJ_MULTIROLE)
    proj_template_connected();
    #endif
	
	#if Conn_Param_Update
	((ke_timer_set_handler)SVC_ke_timer_set)(APP_CONN_UPDATE_TIMER, TASK_APP, 500);
	#endif
    sys_ble_conn_flag = 1;
	// 连上后即开始发送数据
	period_send_data();
	connected_data_periodicTimerOn(100);	// 5s

	#if PROJ_TEMPER
	if(ble_has_been_connected == FALSE)
	{
		ble_has_been_connected = TRUE;	// 已被连接过
		// led_setMode(LED_MODE_OFF);	// 小灯5s一闪
	}
	#endif

    return;
}

//For slave mode , encryed.
void app_connection_encrypted_ind_func(uint8_t conidx)
{
	printf("Slave Role encrypted\n");
}


/**
 ****************************************************************************************
 * @brief app_api function. Project's actions in app_disconnect
 *
 * @param[in] taskid     App task's id.
 * @param[in] param      Received gapc_disconnect_ind msg.
 *
 * @return void.            
 ****************************************************************************************
*/
void app_disconnect_ind_func(ke_task_id_t task_id, struct gapc_disconnect_ind const *param)
{    
	printf("DisConnected, reason 0x%02X\n", param->reason);

	if(app_env.func[APP_CB_ID_DISCONNECTED] !=NULL)
		app_env.func[APP_CB_ID_DISCONNECTED]();
    
#if(PROJ_MULTIROLE)
    proj_template_disconnect();
#endif
#if(USER_PROJ_TEMPLATE || USER_PROJ_WECHAT || PROJ_THROUGHPUT)
	#if(USER_PROJ_WECHAT)
	mpbledemo2_init();
	#endif
	
    #if(APP_WHITE_LIST)
    if(app_sec_env.bonded == TRUE)
    {
        proj_template_start_advertising_direct_host();
    }
    else
    #endif
    {
        proj_template_start_advertising();
    }
#endif	
#if(PROJ_HID_KEYBOARD)
    hid_kbd_start_advertising();
#endif // (PROJ_HID_KEYBOARD)
#if(PROJ_HID_SELFIE)
    hid_selfie_start_advertising();
#endif // (PROJ_HID_SELFIE)
	
#if(USER_RGB_LIGHT)
	rgb_start_advertising();
#endif	
#if(USER_PROJ_CENTRAL)
	app_start_scan();
#endif	
#if(PROJ_OTA)
    app_ota_check_status();
#endif
    sys_ble_conn_flag = 0;
    
}    


/**
 ****************************************************************************************
 * @brief app_api function. Called upon connection param's update rejection
 *
 * @param[in] status        Error code
 *
 * @return void.
 ****************************************************************************************
*/

void app_update_params_rejected_ind_func(uint8_t status)
{
	ASSERT_INFO(0, status, APP_PARAM_UPD);
    return;
}

/**
 ****************************************************************************************
 * @brief app_api function. Called upon connection param's update completion
 *
 * @return void.
 ****************************************************************************************
*/

void app_update_params_complete_ind_func(void)
{

/**************************************************
Handle connection parameters update event 
***************************************************/    
	printf("app_update_params_completed \n");

#if(PROJ_THROUGHPUT)
	struct gattc_exc_mtu_cmd *cmd =  KE_MSG_ALLOC(GATTC_EXC_MTU_CMD,
			KE_BUILD_ID(TASK_GATTC,app_env.conidx ), TASK_APP,
			gattc_exc_mtu_cmd);
    
	cmd->operation = GATTC_MTU_EXCH;

	((ke_msg_send_handler)SVC_ke_msg_send)(cmd);
#endif

    return;
}

/**
 ****************************************************************************************
 * @brief app_api function. Handles undirect advertising completion.
 *
 * @param[in] status        Command complete message status
 *
 * @return void.
 ****************************************************************************************
*/

void app_adv_undirect_complete_ind_func(uint8_t status)
{
    
/**************************************************
Handle undirected advirtising complete event
***************************************************/    
    printf("adv end[%d]\n",status);
	#if(PROJ_MULTIROLE)
    proj_template_gap_end();
    #endif
	
    return;
}

/**
 ****************************************************************************************
 * @brief app_api function. Handles direct advertising completion.
 *
 * @param[in] status        Command complete message status
 *
 * @return void.
 ****************************************************************************************
*/

void app_adv_direct_complete_ind_func(uint8_t status)
{

/**************************************************
Handle directed advirtising complete event
***************************************************/   
    return;
}


/**
 ****************************************************************************************
 * @brief app_api function. Handles scan procedure completion.
 *
 * @return void.
 ****************************************************************************************
*/

void app_scanning_completed_ind_func(uint8_t status)
{
	printf("scan done:%d\n",status);
	
	#if(PROJ_MULTIROLE)
    proj_template_gap_end();
	#endif
}

/**
 ****************************************************************************************
 * @brief app_api function. Handles reception of gapm_adv_report_ind msg.
 *
 * @param[in] param     gapm_adv_report_ind message
 *
 * @return void.
 ****************************************************************************************
*/

void app_adv_report_ind_func(struct gapm_adv_report_ind *param)
{
#if(USER_PROJ_CENTRAL)
	if((param->report.adv_addr.addr[0] == 0x06) && (param->report.adv_addr.addr[1] == 0x05))
	{
		memcpy(&connect_bdaddr.addr,param->report.adv_addr.addr, BD_ADDR_LEN);
		connect_bdaddr.addr_type = param->report.adv_addr_type;
		appm_stop_scanning();
	}
	printf("the scan mac 0x");
	for(uint8 i = 0;i<6;i++)
	{
		printf("%02x",param->report.adv_addr.addr[i]);
	}
	printf("\n");
#endif
}

/**
 ****************************************************************************************
 * @brief Handles connection request failure.
 *
 * @return void.
 ****************************************************************************************
*/
void app_get_info_ind_func(void *param,uint8_t operation)
{
	switch(operation)
	{
		case GAPC_GET_PEER_FEATURES:
		{
			 //struct gapc_peer_features_ind *data = (struct gapc_peer_features_ind *)param;
			 //show_reg2((uint8_t *)data,sizeof(struct gapc_peer_features_ind));
		}
		break;
		default:
			break;
	}
}

void app_connection_success_ind_func(uint8_t conidx)
{
	printf("Master Role Connected\n");
	if(app_env.func[APP_CB_ID_MASTER_CONNECTED] !=NULL)
		app_env.func[APP_CB_ID_MASTER_CONNECTED]();

	//appm_get_peer_info(conidx,GAPC_GET_PEER_FEATURES);
	#if (USER_PROJ_CENTRAL)
	app_tmp_enable_client_prf(conidx);
	#endif
	
//	struct gattc_exc_mtu_cmd *cmd =  KE_MSG_ALLOC(GATTC_EXC_MTU_CMD,
//			KE_BUILD_ID(TASK_GATTC,app_env.conidx ), TASK_APP,
//			gattc_exc_mtu_cmd);
//    
//	cmd->operation = GATTC_MTU_EXCH;

//	((ke_msg_send_handler)SVC_ke_msg_send)(cmd);
}

void app_connect_failed_ind_func(void)
{
	printf("Master Role Connect fail:\n");

    //appm_start_scanning(NULL);
}


void app_connection_encrypt_completed_ind_func(uint8_t conidx)
{
	printf("Master Role encrypted\n");

}
#endif  //BLE_APP_PRESENT
/// @} APP
