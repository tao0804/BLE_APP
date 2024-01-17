
/**
 ****************************************************************************************
 * @addtogroup APP
 * @{
 ****************************************************************************************
 */

#include "panip_config.h"     // SW configuration

#include "app_proj_template.h"
#include "proj_template_server.h"
#include "proj_template_server_task.h"
#include "ota_server_task.h"

#include "gapm_task.h" 
#include "att.h" 
#include "co_utils.h" 
#include "gattc_task.h"
#include "prf_types.h" 
#include "string.h"
#include "app.h"
#include "stack_svc_api.h"

#include "temperature.h"
#include "led_app.h"
#include "key_app.h"
#include "rev_app.h"

static struct app_proj_template_env_tag app_proj_template_env;
void app_proj_template_init(void)
{
	memset(&app_proj_template_env, 0, sizeof(app_proj_template_env));
	
	temper_resetInit();
	rev_resetInit();
	/* 注意：配置定时器放在此函数里没问题，放在appm_init()外部有问题，会导致无法进定时回调！
			函数调用关系：ble_normal_reset_init() -> user_code_start() -> appm_init() -> app_init_ind_func()
			设置定时器放在appm_init()内部没问题，但是放在appm_init()外部就不行，
			甚至把设置定时器放在appm_init()内部的最后一行可以，放在appm_init()执行完出来的下一行不行！*/
	((ke_timer_set_handler)SVC_ke_timer_set)(APP_SAMPLE_TEMPER_TIMER, TASK_APP, 60*100);

	led_resetInit();
	key_resetInit();
}


void app_proj_template_add_server(void)
{

	struct proj_template_server_db_cfg* db_cfg;
	
  // Allocate the BASS_CREATE_DB_REQ
  struct gapm_profile_task_add_cmd *req = KE_MSG_ALLOC_DYN(GAPM_PROFILE_TASK_ADD_CMD,
												TASK_GAPM, TASK_APP,
												gapm_profile_task_add_cmd,sizeof(struct proj_template_server_db_cfg));
	// Fill message
	req->operation	 = GAPM_PROFILE_TASK_ADD;
	req->sec_lvl	 = (PERM(SVC_AUTH, DISABLE)| PERM(SVC_UUID_LEN, UUID_16)| PERM(SVC_EKS, DISABLE)|PERM(SVC_DIS, DISABLE));

	req->prf_task_id = TASK_ID_PROJ_TEMPLATE_SERVER;
	req->app_task	 = TASK_APP;
	req->start_hdl	 = 0;

	// Set parameters
	db_cfg = (struct proj_template_server_db_cfg* ) req->param;
	db_cfg->features = 0xffffffff;	// 注意：配置了gatt数据库，要配置这个掩码使能，32位都是1可以使能32个数据库配置

	// Send the message
	((ke_msg_send_handler)SVC_ke_msg_send)(req);
}


void app_proj_template_enable_server_prf(uint8_t conidx)
{
	app_proj_template_env.conidx = conidx;

  // Allocate the message
  struct proj_template_server_enable_req * req = KE_MSG_ALLOC(PROJ_TEMPLATE_SERVER_ENABLE_REQ,
											  prf_get_task_from_id(TASK_ID_PROJ_TEMPLATE_SERVER),
											  TASK_APP,
											  proj_template_server_enable_req);
  // Fill in the parameter structure
  req->conidx             = conidx;

  // NTF initial status - Disabled
  req->ntf_cfg           = PRF_CLI_STOP_NTFIND;

  // Send the message
  ((ke_msg_send_handler)SVC_ke_msg_send)(req);
}

void app_proj_template_send_value(uint8_t att_idx,uint8_t *buf,uint8_t len)
{
	// Allocate the message
	struct proj_template_server_write_cmd * cmd = KE_MSG_ALLOC_DYN(PROJ_TEMPLATE_SERVER_WRITE_CMD,
													  prf_get_task_from_id(TASK_ID_PROJ_TEMPLATE_SERVER),
													  TASK_APP,
													  proj_template_server_write_cmd,
													  len);
	
	cmd->conidx  = app_proj_template_env.conidx;
	//now fill report
	cmd->write_type = NOTIFICATION;		//NOTIFICATION
	cmd->att_idx = att_idx; 
	if(len > PROJ_TEMPLATE_SERVER_PACKET_SIZE)
	{
		len = PROJ_TEMPLATE_SERVER_PACKET_SIZE;
	}
	cmd->packet_size = len;
	memcpy(cmd->packet, buf, len);

	((ke_msg_send_handler)SVC_ke_msg_send)(cmd);
}

static int proj_template_server_peer_write_data_ind_handler(ke_msg_id_t const msgid,
											   struct proj_template_server_peer_write_data_ind const *param,
											   ke_task_id_t const dest_id,
											   ke_task_id_t const src_id)
{
#if 0
	printf("recv %d\n", param->packet_size);
#else
	uint8_t Sendata[PROJ_TEMPLATE_SERVER_PACKET_SIZE] = {0};
	show_reg3(param->packet,param->packet_size);
	
	if(0xf3 == param->packet[0])
	{
		float temper = TEMPER_VALUE_TO_C(temper_getTemperValue(temper_getTemperCnt()));
		uint8 len = sprintf((char*)Sendata, "temper = %.2f", temper);
		app_proj_template_send_value(PROJ_TEMPLATE_IDX_S2C_VAL, Sendata, len);
	}
#endif
	return (KE_MSG_CONSUMED);
}

static int proj_template_server_enable_rsp_handler(ke_msg_id_t const msgid,
									struct proj_template_server_enable_rsp const *param,
									ke_task_id_t const dest_id,
									ke_task_id_t const src_id)
{
	return (KE_MSG_CONSUMED);
}

static int app_proj_template_msg_dflt_handler(ke_msg_id_t const msgid,
									 void const *param,
									 ke_task_id_t const dest_id,
									 ke_task_id_t const src_id)
{
	// Drop the message
	return (KE_MSG_CONSUMED);
}

static int app_sample_temper_handler(ke_msg_id_t const msgid,
                                     void const *param,
                                     ke_task_id_t const dest_id,
                                     ke_task_id_t const src_id)
{
	// todo:休眠前操作
	rev_resetInit();	// 上下顺序很重要,adc采样完成有阻塞
	// temper_sampleTemperTimerCb();	// 1分钟定时器回调函数,处理定时采温度任务?事实上我不需要周期采和存了
	temper_sampleTemper();
	((ke_timer_set_handler)SVC_ke_timer_set)(APP_SAMPLE_TEMPER_TIMER, TASK_APP, 60*100);	// 开启下一次定时
	return (KE_MSG_CONSUMED);
}

static int app_led_blink_handler(ke_msg_id_t const msgid,
                                     void const *param,
                                     ke_task_id_t const dest_id,
                                     ke_task_id_t const src_id)
{
	led_ledBlinkTimerCb();
	return (KE_MSG_CONSUMED);
}

static int app_key_scan_handler(ke_msg_id_t const msgid,
                                     void const *param,
                                     ke_task_id_t const dest_id,
                                     ke_task_id_t const src_id)
{
	key_scanTimerCB();
	return (KE_MSG_CONSUMED);
}

/// Default State handlers definition
const struct ke_msg_handler app_proj_template_msg_handler_list[] =
{
	// Note: first message is latest message checked by kernel so default is put on top.
	{KE_MSG_DEFAULT_HANDLER,        (ke_msg_func_t)app_proj_template_msg_dflt_handler},
	{PROJ_TEMPLATE_SERVER_PEER_WRITE_DATA_IND,   (ke_msg_func_t)proj_template_server_peer_write_data_ind_handler},
	{PROJ_TEMPLATE_SERVER_ENABLE_RSP,       (ke_msg_func_t)proj_template_server_enable_rsp_handler},
	{APP_SAMPLE_TEMPER_TIMER,	(ke_msg_func_t)app_sample_temper_handler},
	{APP_LED_BLINK_TIMER,		(ke_msg_func_t)app_led_blink_handler},
	{APP_KEY_SCAN_TIMER,		(ke_msg_func_t)app_key_scan_handler},
};

const struct ke_state_handler app_proj_template_table_handler =
	{&app_proj_template_msg_handler_list[0], (sizeof(app_proj_template_msg_handler_list)/sizeof(struct ke_msg_handler))};

