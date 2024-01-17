/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "prf.h"
#include "prf_utils.h"
#include "attm.h"
#include "ke_mem.h"

#include "proj_template_server_task.h"
#include "proj_template_server.h"
#include "stack_svc_api.h"


const struct attm_desc proj_template_server_att_db[PROJ_TEMPLATE_IDX_NB] =
{
	[PROJ_TEMPLATE_IDX_SVC]				= { ATT_DECL_PRIMARY_SERVICE, PERM(RD, ENABLE), 0, 0 },

	[PROJ_TEMPLATE_IDX_TEMP_CHAR]		= { ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0 },
	[PROJ_TEMPLATE_IDX_TEMP_VAL]		= { TRX_CHAR_TEMP_UUID, PERM(RD, ENABLE), PERM(RI, ENABLE), PROJ_TEMPLATE_SERVER_PACKET_SIZE },

	[PROJ_TEMPLATE_IDX_VBATT_CHAR]		= { ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0 },
	[PROJ_TEMPLATE_IDX_VBATT_VAL]		= { TRX_CHAR_VBATT_UUID, PERM(RD, ENABLE), PERM(RI, ENABLE), PROJ_TEMPLATE_SERVER_PACKET_SIZE },
	
	[PROJ_TEMPLATE_IDX_REVPERMINUTE_CHAR]= { ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0 },
	[PROJ_TEMPLATE_IDX_REVPERMINUTE_VAL]= { TRX_CHAR_REVPERMINUTE_UUID, PERM(RD, ENABLE), PERM(RI, ENABLE), PROJ_TEMPLATE_SERVER_PACKET_SIZE },

	[PROJ_TEMPLATE_IDX_S2C_CHAR]        = { ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0 },
	[PROJ_TEMPLATE_IDX_S2C_VAL]         = { TRX_CHAR_S2C_UUID, PERM(NTF, ENABLE), PERM(RI, ENABLE), PROJ_TEMPLATE_SERVER_PACKET_SIZE },
	[PROJ_TEMPLATE_IDX_S2C_USER_DESC]   = { ATT_DESC_CHAR_USER_DESCRIPTION, PERM(RD, ENABLE), PERM(RI, ENABLE), PROJ_TEMPLATE_S2C_USER_DESC_VAL_LEN },
	[PROJ_TEMPLATE_IDX_S2C_CLIENT_CFG]  = { ATT_DESC_CLIENT_CHAR_CFG, PERM(RD, ENABLE) | PERM(WRITE_REQ, ENABLE), 0, 0 },
	
	[PROJ_TEMPLATE_IDX_C2S_CHAR]        = { ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0 },
	[PROJ_TEMPLATE_IDX_C2S_VAL]         = { TRX_CHAR_C2S_UUID, PERM(WRITE_COMMAND, ENABLE), PERM(RI, ENABLE), PROJ_TEMPLATE_SERVER_PACKET_SIZE },    
	[PROJ_TEMPLATE_IDX_C2S_USER_DESC]   = { ATT_DESC_CHAR_USER_DESCRIPTION, PERM(RD, ENABLE), PERM(RI, ENABLE), PROJ_TEMPLATE_C2S_USER_DESC_VAL_LEN },
};


/**
 ****************************************************************************************
 * @brief       Init of the SPS device
 * @param[in] Void
 * @return      None
 ****************************************************************************************
 */
static uint8_t proj_template_server_init (struct prf_task_env* env, uint16_t* start_hdl, uint16_t app_task, uint8_t sec_lvl, struct proj_template_server_db_cfg* params)
{
	uint16_t shdl;
	struct proj_template_server_env_tag *proj_template_server_env = NULL;
	uint8_t status = GAP_ERR_NO_ERROR;
	
	// Service content flag
	uint32_t cfg_flag = params->features;

	shdl = *start_hdl;
	status = attm_svc_create_db(&shdl, 
								TRX_SVC_UUID, 
								(uint8_t *)&cfg_flag,
								PROJ_TEMPLATE_IDX_NB, 
								NULL, 
								env->task, 
								proj_template_server_att_db,
								(sec_lvl & (PERM_MASK_SVC_DIS | PERM_MASK_SVC_AUTH | PERM_MASK_SVC_EKS)));
	
	if (status == ATT_ERR_NO_ERROR)
	{
		proj_template_server_env = (struct proj_template_server_env_tag* )((ke_malloc_handler)SVC_ke_malloc)(sizeof(struct proj_template_server_env_tag), KE_MEM_ATT_DB);
		memset(proj_template_server_env, 0 , sizeof(struct proj_template_server_env_tag));

		//set prf_env.prf[i]
		env->env = (prf_env_t*)proj_template_server_env;
		env->id = TASK_ID_PROJ_TEMPLATE_SERVER; 
		
		*start_hdl = shdl;
		proj_template_server_env->start_hdl = *start_hdl;
		proj_template_server_env->prf_env.app_task = app_task;
		//| (PERM_GET(sec_lvl, SVC_MI) ? PERM(PRF_MI, ENABLE) : PERM(PRF_MI, DISABLE));
		proj_template_server_env->prf_env.prf_task = env->task; 
				//| PERM(PRF_MI, DISABLE);

		proj_template_server_task_init(&(env->desc));
		// service is ready, go into an Idle state
		((ke_state_set_handler)SVC_ke_state_set)(env->task, PROJ_TEMPLATE_SVS_IDLE);
	}

	return status;
}

static void proj_template_server_destroy(struct prf_task_env* env)
{
	struct proj_template_server_env_tag* proj_template_server_env = (struct proj_template_server_env_tag*) env->env;

	// clear on-going operation
	if(proj_template_server_env->operation != NULL)
	{
	   ((ke_free_handler)SVC_ke_free)(proj_template_server_env->operation);
	}

	// free profile environment variables
	env->env = NULL;
	((ke_free_handler)SVC_ke_free)(proj_template_server_env);
}

/**
 ****************************************************************************************
 * @brief Handles Connection creation
 *
 * @param[in|out]    env        Collector or Service allocated environment data.
 * @param[in]        conidx     Connection index
 ****************************************************************************************
 */
static void proj_template_server_create(struct prf_task_env* env, uint8_t conidx)
{
	struct proj_template_server_env_tag* proj_template_server_env = (struct proj_template_server_env_tag*) env->env;
	ASSERT_ERR(conidx < BLE_CONNECTION_MAX);
	proj_template_server_env->client_cfg_s2c[conidx]  = PRF_CLI_STOP_NTFIND;
}

/**
 ****************************************************************************************
 * @brief Handles Disconnection
 *
 * @param[in|out]    env        Collector or Service allocated environment data.
 * @param[in]        conidx     Connection index
 * @param[in]        reason     Detach reason
 ****************************************************************************************
 */
static void proj_template_server_cleanup(struct prf_task_env* env, uint8_t conidx, uint8_t reason)
{
	struct proj_template_server_env_tag* proj_template_server_env = (struct proj_template_server_env_tag*) env->env;

	ASSERT_ERR(conidx < BLE_CONNECTION_MAX);
	// force notification config to zero when peer device is disconnected
	proj_template_server_env->client_cfg_s2c[conidx]  = PRF_CLI_STOP_NTFIND;
}

/// BASS Task interface required by profile manager
const struct prf_task_cbs proj_template_sev_itf =
{
	(prf_init_fnct) proj_template_server_init,
	proj_template_server_destroy,
	proj_template_server_create,
	proj_template_server_cleanup,
};

const struct prf_task_cbs* proj_template_server_prf_itf_get(void)
{
	return &proj_template_sev_itf;
}

uint16_t proj_template_server_get_att_handle(uint8_t svc_idx, uint8_t att_idx)
{
	struct proj_template_server_env_tag* proj_template_server_env = PRF_ENV_GET(PROJ_TEMPLATE_SERVER, proj_template_server);
	uint16_t handle = ATT_INVALID_HDL;

	handle = proj_template_server_env->start_hdl;
	if(att_idx < PROJ_TEMPLATE_IDX_NB)
	{
	   handle += att_idx;
	}
	else
	{
	   handle = ATT_INVALID_HDL;
	}

	return handle;
}

uint8_t proj_template_server_get_att_idx(uint16_t handle, uint8_t *svc_idx, uint8_t *att_idx)
{
	struct proj_template_server_env_tag* proj_template_server_env = PRF_ENV_GET(PROJ_TEMPLATE_SERVER, proj_template_server);
	uint16_t hdl_cursor = proj_template_server_env->start_hdl;
	uint8_t status = PRF_APP_ERROR;

	if (handle < (hdl_cursor + PROJ_TEMPLATE_IDX_NB))
	{
		*att_idx = handle - hdl_cursor;
		status = GAP_ERR_NO_ERROR;
	}

	return (status);
}
