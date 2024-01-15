
#ifndef PROJ_TEMPLATE_SERVER_H_
#define PROJ_TEMPLATE_SERVER_H_

#include "prf_types.h"
#include "prf.h"

#define PROJ_TEMPLATE_SERVER_IDX_MAX     0x01


enum proj_template_server_state
{
	/// Idle state
	PROJ_TEMPLATE_SVS_IDLE,
	/// busy state
	PROJ_TEMPLATE_SVS_BUSY,
	/// Number of defined states.
	PROJ_TEMPLATE_SVS_STATE_MAX
};

enum
{
	TRX_SVC_UUID			= 0xFFF0,
	// TRX_CHAR_TEMPER_UUID	= 0xFFF1,
	// TRX_CHAR_COUNT_UUID		= 0xFFF2,
	// TRX_CHAR_CONST_UUID		= 0xFFF3,
	// TRX_CHAR_HISTORY_UUID	= 0xFFF4,
	// TRX_CHAR_CONFIG_UUID	= 0xFFF5,
	// TRX_CHAR_VBATT_UUID		= 0xFFF6,
	// TRX_CHAR_BINDING_UUID	= 0xFFF7,
	// TRX_CHAR_DISCOVER_UUID	= 0xFFF8,
	TRX_CHAR_S2C_UUID		= 0xFFF9,
	TRX_CHAR_C2S_UUID		= 0xFFFA,
};

//attibute idex
enum
{
	PROJ_TEMPLATE_IDX_SVC, 
	
	// PROJ_TEMPLATE_IDX_TEMPER_CHAR,
	// PROJ_TEMPLATE_IDX_TEMPER_VAL,
	
	// PROJ_TEMPLATE_IDX_COUNT_CHAR,
	// PROJ_TEMPLATE_IDX_COUNT_VAL,
	
	// PROJ_TEMPLATE_IDX_CONST_CHAR,
	// PROJ_TEMPLATE_IDX_CONST_VAL,
	
	// PROJ_TEMPLATE_IDX_HISTORY_CHAR,
	// PROJ_TEMPLATE_IDX_HISTORY_VAL,

	// PROJ_TEMPLATE_IDX_CONFIG_CHAR,
	// PROJ_TEMPLATE_IDX_CONFIG_VAL,

	// PROJ_TEMPLATE_IDX_VBATT_CHAR,
	// PROJ_TEMPLATE_IDX_VBATT_VAL,
	
	// PROJ_TEMPLATE_IDX_BINDING_CHAR,
	// PROJ_TEMPLATE_IDX_BINDING_VAL,
	
	// PROJ_TEMPLATE_IDX_DISCOVER_CHAR,
	// PROJ_TEMPLATE_IDX_DISCOVER_VAL,
	
	PROJ_TEMPLATE_IDX_S2C_CHAR, 
	PROJ_TEMPLATE_IDX_S2C_VAL, 
	PROJ_TEMPLATE_IDX_S2C_USER_DESC, 
	PROJ_TEMPLATE_IDX_S2C_CLIENT_CFG, 
	
	PROJ_TEMPLATE_IDX_C2S_CHAR, 
	PROJ_TEMPLATE_IDX_C2S_VAL, 
	PROJ_TEMPLATE_IDX_C2S_USER_DESC, 

	PROJ_TEMPLATE_IDX_NB,
};


#define PROJ_TEMPLATE_S2C_USER_DESC_VAL         "Server To Client"
#define PROJ_TEMPLATE_S2C_USER_DESC_VAL_LEN     sizeof(PROJ_TEMPLATE_S2C_USER_DESC_VAL)
#define PROJ_TEMPLATE_S2C_CLIENT_CFG_LEN        sizeof(uint16_t)
#define PROJ_TEMPLATE_C2S_USER_DESC_VAL         "Client To Server"
#define PROJ_TEMPLATE_C2S_USER_DESC_VAL_LEN     sizeof(PROJ_TEMPLATE_C2S_USER_DESC_VAL)


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

struct proj_template_server_env_tag
{
	/// profile environment
	prf_env_t prf_env;
	/// On-going operation
	struct ke_msg * operation;
	/// Services Start Handle
	uint16_t   start_hdl;
	ke_state_t state[PROJ_TEMPLATE_SERVER_IDX_MAX];
	uint16_t   client_cfg_s2c[BLE_CONNECTION_MAX];
};

extern struct proj_template_server_env_tag proj_template_server_env;
const struct prf_task_cbs* proj_template_server_prf_itf_get(void);

void proj_template_server_task_init(struct ke_task_desc *task_desc);
void proj_template_server_exe_operation(void);

uint16_t proj_template_server_get_att_handle(uint8_t svc_idx, uint8_t att_idx);
uint8_t  proj_template_server_get_att_idx(uint16_t handle, uint8_t *svc_idx, uint8_t *att_idx);

#endif 
