
#ifndef PROJ_TEMPLATE_SERVER_TASK_H_
#define PROJ_TEMPLATE_SERVER_TASK_H_


/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdint.h>
#include "ke_task.h"
#include "panip_task.h"

#include "l2cc_pdu.h"

/*
 * INSTANCES
 ****************************************************************************************
 */
#define PROJ_TEMPLATE_SERVER_PACKET_SIZE (L2C_MIN_LE_MTUSIG - 3)

/*
 * MESSAGES
 ****************************************************************************************
 */
/// Message API of the SPS_SERVER task
enum
{
		/// Enables the Serial Port Service Device profile. The profile has to be enabled only
		/// once a connection has been established by the application
		PROJ_TEMPLATE_SERVER_ENABLE_REQ = KE_FIRST_MSG(TASK_ID_PROJ_TEMPLATE_SERVER),
		
		//Enable condirmation message
		PROJ_TEMPLATE_SERVER_ENABLE_RSP,
		
		//peer write data to att idx
		PROJ_TEMPLATE_SERVER_PEER_WRITE_DATA_IND,
	
		//NTF / Indication  data to peer
		PROJ_TEMPLATE_SERVER_WRITE_CMD,

		// 温度周期采样定时器
		APP_SAMPLE_TEMPER_TIMER,

		// led闪烁定时器
		APP_LED_BLINK_TIMER,

		// key扫描定时器
		APP_KEY_SCAN_TIMER,

		// temp, voltage, rpm周期发送数据定时器
		CONNECTED_DATA_PERIODIC_TIMER,
};


struct proj_template_server_db_cfg
{
		uint32_t features;
};

/// @ref PROJ_TEMPLATE_SERVER_ENABLE_REQ parameters structure description.
struct proj_template_server_enable_req
{
		/// connection index
		uint8_t  conidx;
		/// Notification Configuration
		uint8_t  ntf_cfg;
};

/// Parameters of the @ref BASS_ENABLE_RSP message
struct proj_template_server_enable_rsp
{
		/// connection index
		uint8_t conidx;
		///status
		uint8_t status;
};

struct proj_template_server_peer_write_data_ind
{
		uint8_t 	conidx;
		uint8_t		att_idx;
		uint8_t 	packet[PROJ_TEMPLATE_SERVER_PACKET_SIZE];
		uint8_t 	packet_size;
};

//enum ntf_type
//{
//		NOTIFICATION,
//		INDICATION,
//};

struct proj_template_server_write_cmd
{
		uint8_t 	conidx;
		uint8_t		att_idx;
		uint8_t 	write_type;
		uint8_t 	packet_size;
		uint8_t 	packet[];
};
#endif // TMP_SERVER_TASK_H_
