#include <stdio.h>
#include <string.h>
#include "panip_config.h"
#include "panip.h"

#include "PN102Series.h"
#include "arch.h"	
#include "global_io.h"	
#include "peripherals.h"

#include "app.h"	
#include "app_task.h"
#include "app_sec.h"

#include "att.h"
#include "proj_template.h"
#include "proj_template_server.h"
#include "stack_svc_api.h"

#include "temperature.h"

#define STORE_INFO_FLASH_ADDR 0x0003C000

struct proj_template_env_tag proj_template_env;

void proj_template_ini(void)
{
	memset(&proj_template_env,0x00,sizeof(proj_template_env));
	
	proj_template_get_flash_cmd();
}

void proj_template_start_advertising(void)
{				
	struct gapm_start_advertise_cmd msg;
	uint8_t *pos ;
	uint16_t uuid_value;

	msg.op.code	 = GAPM_ADV_UNDIRECT;
	msg.op.addr_src = GAPM_STATIC_ADDR;
	msg.intv_min	 =	160;			//APP_ADV_INT_MIN;				
	msg.intv_max	 =	160;			//100ms
	msg.channel_map = APP_ADV_CHMAP;

	msg.info.host.mode = GAP_GEN_DISCOVERABLE;
	msg.info.host.adv_filt_policy = ADV_ALLOW_SCAN_ANY_CON_ANY;


	pos = msg.info.host.scan_rsp_data;
	*pos++ = strlen(app_info.co_default_bdname) + 1;
	*pos++	= '\x09';
	memcpy(pos, app_info.co_default_bdname, strlen(app_info.co_default_bdname));
	pos += strlen(app_info.co_default_bdname);
	msg.info.host.scan_rsp_data_len = ((uint32_t)pos - (uint32_t)(msg.info.host.scan_rsp_data));


	msg.info.host.adv_data[0] = 0x2;
	msg.info.host.adv_data[1] = 0x1;
	msg.info.host.adv_data[2] = 0x6;
	pos = &msg.info.host.adv_data[3];

	uuid_value = 0x0180;
	*pos++ = sizeof(uuid_value) + 1;
	*pos++	= '\x19';
	memcpy(pos, (uint8_t *)&uuid_value, sizeof(uuid_value));
	pos += sizeof(uuid_value);
	
	uuid_value = 0x1812;
	*pos++ = sizeof(uuid_value) + 1;
	*pos++	= '\x02';
	memcpy(pos, (uint8_t *)&uuid_value, sizeof(uuid_value));
	pos += sizeof(uuid_value);
	
	msg.info.host.adv_data_len = ((uint32_t)pos - (uint32_t)(msg.info.host.adv_data));
	
	appm_start_advertising(&msg);
}

#if(APP_WHITE_LIST)
void proj_template_start_advertising_direct_host(void)
{				
	struct gapm_start_advertise_cmd msg;
	uint8_t *pos ;
	uint16_t uuid_value;

	msg.op.code	 = GAPM_ADV_UNDIRECT;
	msg.op.addr_src = GAPM_STATIC_ADDR;
	msg.intv_min	 =	160;			//APP_ADV_INT_MIN;				
	msg.intv_max	 =	160;			//100ms
	msg.channel_map = APP_ADV_CHMAP;

	msg.info.host.mode = GAP_GEN_DISCOVERABLE;
	msg.info.host.adv_filt_policy = ADV_ALLOW_SCAN_WLST_CON_WLST;


	pos = msg.info.host.scan_rsp_data;
	*pos++ = strlen(app_info.co_default_bdname) + 1;
	*pos++	= '\x09';
	memcpy(pos, app_info.co_default_bdname, strlen(app_info.co_default_bdname));
	pos += strlen(app_info.co_default_bdname);
	msg.info.host.scan_rsp_data_len = ((uint32_t)pos - (uint32_t)(msg.info.host.scan_rsp_data));


	msg.info.host.adv_data[0] = 0x2;
	msg.info.host.adv_data[1] = 0x1;
	msg.info.host.adv_data[2] = 0x6;
	pos = &msg.info.host.adv_data[3];

	uuid_value = 0x0180;
	*pos++ = sizeof(uuid_value) + 1;
	*pos++	= '\x19';
	memcpy(pos, (uint8_t *)&uuid_value, sizeof(uuid_value));
	pos += sizeof(uuid_value);
	
	uuid_value = 0x1812;
	*pos++ = sizeof(uuid_value) + 1;
	*pos++	= '\x02';
	memcpy(pos, (uint8_t *)&uuid_value, sizeof(uuid_value));
	pos += sizeof(uuid_value);
	
	msg.info.host.adv_data_len = ((uint32_t)pos - (uint32_t)(msg.info.host.adv_data));
	
	appm_start_advertising(&msg);
}
#endif

void proj_template_store_info(void *buf, uint8_t store_type)
{    
	switch (store_type)
	{
		case PROJ_TEMPLATE_STORE_TYPE_LTK:
		{
			
			memcpy(proj_template_env.info.stored_ltk.ltk.key, buf, sizeof(struct gapc_ltk) );
		}
		break;
		
		case PROJ_TEMPLATE_STORE_TYPE_NTF:
		{            
			memcpy(&proj_template_env.info.ntf_cfg, buf, 2);
		}
		break;
		
		case PROJ_TEMPLATE_STORE_TYPE_BONDED:
		{            
			proj_template_env.info.bonded = app_sec_env.bonded;
		}
		break;
		
		case PROJ_TEMPLATE_STORE_TYPE_PEER_BD_ADDRESS:
		{            
			memcpy(&proj_template_env.info.stored_peer_addr, buf, sizeof(struct gap_bdaddr));
			#if(APP_WHITE_LIST)
			appm_add_white_list((struct gap_bdaddr*)&proj_template_env.info.stored_peer_addr);
			#endif
		}
		break;
		
		default:
		{
			
		}
		break;
	}
}

void *proj_template_get_store_info(uint8_t store_type)
{
	if(store_type == PROJ_TEMPLATE_STORE_TYPE_LTK)
	{
		return &proj_template_env.info.stored_ltk;
	}
	else if(store_type == PROJ_TEMPLATE_STORE_TYPE_NTF)
	{
		return &proj_template_env.info.ntf_cfg;
	}
	else if(store_type == PROJ_TEMPLATE_STORE_TYPE_BONDED)
	{
		return &proj_template_env.info.bonded;
	}
	else if(store_type == PROJ_TEMPLATE_STORE_TYPE_PEER_BD_ADDRESS)
	{
		return &proj_template_env.info.stored_peer_addr;
	}
	else
	{
		return NULL;
	}
}

void proj_template_remove_bond(void)
{
	app_sec_remove_bond();
}

void proj_template_clr_store_info(void)
{
	SYS_UnlockReg();
	FMC_Open();
	FMC_ENABLE_AP_UPDATE();
	FMC_Erase(STORE_INFO_FLASH_ADDR);
	memset(&proj_template_env, 0x00, sizeof(proj_template_env));
	FMC_Close();
	SYS_LockReg();
}

void proj_template_get_flash_cmd(void)
{
	uint8 proj_template_data[100] = {0};

	SYS_UnlockReg();
	FMC_Open();
	FMC_ENABLE_AP_UPDATE();

	for(int i = 0;i<sizeof(struct proj_template_env_tag);i++)
	{
		proj_template_data[i] = FMC_Read(STORE_INFO_FLASH_ADDR + 4 * i);
	}
	memcpy(&proj_template_env,proj_template_data,sizeof(struct proj_template_env_tag));
	FMC_Close();
	SYS_LockReg();
}

void proj_template_store_flash_data(void)
{
	uint8 *proj_template_data = (uint8 *)&proj_template_env;

	SYS_UnlockReg();
	FMC_Open();
	FMC_ENABLE_AP_UPDATE();
	
	FMC_Erase(STORE_INFO_FLASH_ADDR);
	for(int i = 0;i<sizeof(struct proj_template_env_tag);i++)
	{
		FMC_Write(STORE_INFO_FLASH_ADDR + i * 4,proj_template_data[i]);
	}

	FMC_Close();
	SYS_LockReg();
}


