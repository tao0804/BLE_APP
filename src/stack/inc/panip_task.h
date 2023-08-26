
#ifndef PANIP_TASK_H_
#define PANIP_TASK_H_

/**
 ****************************************************************************************
 * @addtogroup ROOT
 * @{
 *
 *  Information about RW SW TASK
 *
 * @name RW TASK Configuration
 * @{
 ****************************************************************************************
 */

#include <stdint.h>
#include "panip_config.h"

/*
 * DEFINES
 ****************************************************************************************
 */
#include "ke_task.h"
#include "ke_msg.h"
	//For RWIP903
#define TASK_FIRST_MSG	KE_FIRST_MSG
#define TASK_BUILD		KE_BUILD_ID
#define TASK_TYPE_GET	KE_TYPE_GET
#define TASK_IDX_GET	KE_IDX_GET


/*
/// Build the first message ID of a task. (in fact a ke_msg_id_t)
#define TASK_FIRST_MSG(task) ((uint16_t)((task) << 8))

/// Builds the task identifier from the type and the index of that task.
#define TASK_BUILD(type, index) ((uint16_t)(((index) << 8)|(type)) )

/// Retrieves task type from task id.
#define TASK_TYPE_GET(ke_task_id) ((uint16_t) & 0xFF)

/// Retrieves task index number from task id.
#define TASK_IDX_GET(ke_task_id) (((uint16_t) >> 8) & 0xFF)
*/

/// Tasks types definition, this value shall be in [0-254] range
enum TASK_API_ID
{
    // Link Layer Tasks
    TASK_ID_LLM          = 0,
    TASK_ID_LLC          = 1,
    TASK_ID_LLD          = 2,
    TASK_ID_DBG          = 3,

    // BT Controller Tasks
    TASK_ID_LM           = 4,
    TASK_ID_LC           = 5,
    TASK_ID_LB           = 6,
    TASK_ID_LD           = 7,

    TASK_ID_HCI          = 8,
    TASK_ID_DISPLAY      = 9,

    // -----------------------------------------------------------------------------------
    // --------------------- BLE HL TASK API Identifiers ---------------------------------
    // -----------------------------------------------------------------------------------

    TASK_ID_L2CC         = 10,   // L2CAP Controller Task
    TASK_ID_GATTM        = 11,   // Generic Attribute Profile Manager Task
    TASK_ID_GATTC        = 12,   // Generic Attribute Profile Controller Task
    TASK_ID_GAPM         = 13,   // Generic Access Profile Manager
    TASK_ID_GAPC         = 14,   // Generic Access Profile Controller

    TASK_ID_APP          = 15,   // Application API
    
	TASK_ID_TEST0		  = 16	,		//Just for test
	TASK_ID_TEST1		  = 17	,		//Just for test
	TASK_ID_UART_TEST	  = 18	,	//Just for test
	TASK_ID_UART1	  	= 19	,	//uart1_task


    // -----------------------------------------------------------------------------------
    // --------------------- BLE Profile TASK API Identifiers ----------------------------
    // -----------------------------------------------------------------------------------
    TASK_ID_DISS         = 20,   // Device Information Service Server Task
    TASK_ID_DISC         = 21,   // Device Information Service Client Task

    TASK_ID_PROXM        = 22,   // Proximity Monitor Task
    TASK_ID_PROXR        = 23,   // Proximity Reporter Task

    TASK_ID_FINDL        = 24,   // Find Me Locator Task
    TASK_ID_FINDT        = 25,   // Find Me Target Task

    TASK_ID_HTPC         = 26,   // Health Thermometer Collector Task
    TASK_ID_HTPT         = 27,   // Health Thermometer Sensor Task

    TASK_ID_BLPS         = 28,   // Blood Pressure Sensor Task
    TASK_ID_BLPC         = 29,   // Blood Pressure Collector Task

    TASK_ID_HRPS         = 30,   // Heart Rate Sensor Task
    TASK_ID_HRPC         = 31,   // Heart Rate Collector Task

    TASK_ID_TIPS         = 32,   // Time Server Task
    TASK_ID_TIPC         = 33,   // Time Client Task

    TASK_ID_SCPPS        = 34,   // Scan Parameter Profile Server Task
    TASK_ID_SCPPC        = 35,   // Scan Parameter Profile Client Task

    TASK_ID_BASS         = 36,   // Battery Service Server Task
    TASK_ID_BASC         = 37,   // Battery Service Client Task

    TASK_ID_HOGPD        = 38,   // HID Device Task
    TASK_ID_HOGPBH       = 39,   // HID Boot Host Task
    TASK_ID_HOGPRH       = 40,   // HID Report Host Task

    TASK_ID_GLPS         = 41,   // Glucose Profile Sensor Task
    TASK_ID_GLPC         = 42,   // Glucose Profile Collector Task

    TASK_ID_RSCPS        = 43,   // Running Speed and Cadence Profile Server Task
    TASK_ID_RSCPC        = 44,   // Running Speed and Cadence Profile Collector Task

    TASK_ID_CSCPS        = 45,   // Cycling Speed and Cadence Profile Server Task
    TASK_ID_CSCPC        = 46,   // Cycling Speed and Cadence Profile Client Task

    TASK_ID_ANPS         = 47,   // Alert Notification Profile Server Task
    TASK_ID_ANPC         = 48,   // Alert Notification Profile Client Task

    TASK_ID_PASPS        = 49,   // Phone Alert Status Profile Server Task
    TASK_ID_PASPC        = 50,   // Phone Alert Status Profile Client Task

    TASK_ID_CPPS         = 51,   // Cycling Power Profile Server Task
    TASK_ID_CPPC         = 52,   // Cycling Power Profile Client Task

    TASK_ID_LANS         = 53,   // Location and Navigation Profile Server Task
    TASK_ID_LANC         = 54,   // Location and Navigation Profile Client Task

    TASK_ID_IPSS         = 55,   // Internet Protocol Support Profile Server Task
    TASK_ID_IPSC         = 56,   // Internet Protocol Support Profile Client Task

    TASK_ID_ENVS         = 57,   // Environmental Sensing Profile Server Task
    TASK_ID_ENVC         = 58,   // Environmental Sensing Profile Client Task

    TASK_ID_WSCS         = 59,   // Weight Scale Profile Server Task
    TASK_ID_WSCC         = 60,   // Weight Scale Profile Client Task

    TASK_ID_UDSS         = 61,   // User Data Service Server Task
    TASK_ID_UDSC         = 62,   // User Data Service Client Task

    TASK_ID_BCSS         = 63,   // Body Composition Server Task
    TASK_ID_BCSC         = 64,   // Body Composition Client Task
	TASK_ID_AHI 		 = 65,	 // Application Host Interface

	TASK_ID_SPS_SERVER	  = 66,
	TASK_ID_TMP_CLIENT 	  = 67,

	TASK_ID_HQTANK_SERVER	  = 68,
    TASK_ID_RGB_SERVER1   =69,
    TASK_ID_RGB_SERVER2   =70,
    TASK_ID_GUN_PLAY_SERVER =71,
	TASK_ID_PROJ_TEMPLATE_SERVER =72,
	TASK_ID_OTA          = 75,
    /* 240 -> 241 reserved for Audio Mode 0 */
    TASK_ID_AM0          = 240,  // BLE Audio Mode 0 Task
    TASK_ID_AM0_HAS      = 241,  // BLE Audio Mode 0 Hearing Aid Service Task

    TASK_ID_INVALID      = 0xFF, // Invalid Task Identifier
};

/// @} BT Stack Configuration
/// @} ROOT



#endif //PANIP_CONFIG_H_