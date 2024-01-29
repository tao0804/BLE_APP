
#ifndef PANIP_CONFIG_H_
#define PANIP_CONFIG_H_

/*
 * DEFINES
 ****************************************************************************************
 */

/******************************************************************************************/
/* --------------------------   GENERAL SETUP       --------------------------------------*/
/******************************************************************************************/

#define	HOST_TO_OLD             (0)
#define	SDK_USELESS_CODE        (0)
#define	KEEP_ENC_CODE           (1)
//Customer marco
#define USER_PROJ_TEMPLATE      (1)
#define PROJ_OTA                (0)
#define PROJ_TEMPER             (1)

// DSP Log enable
#define DBG_SYS_EN              (0)

#define ENABLE_UART0			(0)
#define ENABLE_UART1			(0)
#define USE_EXT_32K 	        (0) //macro of use external 32k 		
#define SLEEP_EN		        (1) //maro of whether to use low power
#define PN102B			        (1)
#define SET_CUSTOME_ADDR		(0)
#if(SLEEP_EN == 1)
#define EXT_WAKEUP				(0)	//sleep mode,P52 wakeup 
#else
#define EXT_WAKEUP				(0)	//sleep mode,P52 wakeup 
#endif
#define XN297_MODE_EN			(0) //2.4g mode with xn297
#define RF_CLOSE_EN				(0)	//使能后，mcu工作关闭rf。
#define GPIO_RETAIN_EN			(0) //使能GPIO保持功能
#define SPROM_DEC				(0)	//在PAN1020Tool给SPROM加密后，需要使能宏。保证flash程序的安全
#define TEMP_CHANGE_CALIB		(0)	//Detect temperature change calibration two-point


#define	CFG_SLEEP
#define	CFG_BLE
#define	CFG_EMB
#define	CFG_HOST
#define	CFG_APP

#define	CFG_DBG
#define CFG_ALLROLES 1
#define CFG_CON      1

#define CFG_SECURITY_ON     1  
#define CFG_ATTC 
#define CFG_ATTS  
#define CFG_BLECORE_11

//#define CFG_CHNL_ASSESS
#define CFG_DEPRECATED_API
/* BLE Security  */
#define CFG_APP_SEC		1


/// Flag indicating if stack is compiled in dual or single mode
#if defined(CFG_BLE)
	#define BT_DUAL_MODE                     0
	#define BT_STD_MODE                      0
	#define BLE_STD_MODE                     1
#endif // CFG_BT


/// Flag indicating if Dual mode is supported
#define RW_DM_SUPPORT         BT_DUAL_MODE

/// Flag indicating if BLE handles main parts of the stack
#define RW_BLE_SUPPORT        BLE_STD_MODE

/// Flag indicating if stack is compiled for BLE1.0 HW or later
#if defined (CFG_BLECORE_11)
	#define BLE11_HW                    1
	#define BLE12_HW                    0
#else // defined (CFG_BLECORE_11)
	#define BLE11_HW                    0
	#define BLE12_HW                    1
#endif // defined (CFG_BLECORE_11)
/******************************************************************************************/
/* -------------------------   STACK PARTITIONING      -----------------------------------*/
/******************************************************************************************/

#define BT_EMB_PRESENT              0
#define HCI_PRESENT 				1

#if defined(CFG_EMB)
		#define BLE_EMB_PRESENT         1
#else
		#define BLE_EMB_PRESENT         0
#endif //CFG_EMB
#if defined(CFG_HOST)
		#define BLE_HOST_PRESENT        1
#else
		#define BLE_HOST_PRESENT        0
#endif //CFG_HOST
#if defined(CFG_APP)
		#define BLE_APP_PRESENT         1
#else
		#define BLE_APP_PRESENT         0
#endif //CFG_APP

/// Security Application
#if(CFG_APP_SEC)
#define BLE_APP_SEC          1
#else // defined(CFG_APP_SEC)
#define BLE_APP_SEC          0
#endif // defined(CFG_APP_SEC)

/******************************************************************************************/
/* -------------------------   INTERFACES DEFINITIONS      -------------------------------*/
/******************************************************************************************/

/// Generic Transport Layer
#if defined(CFG_GTL)
#define GTL_ITF           1
#else // defined(CFG_GTL)
#define GTL_ITF           0
#endif // defined(CFG_GTL)




/// Host Controller Interface (Controller side)
#define HCIC_ITF        (HCI_PRESENT)		//(!BLE_HOST_PRESENT)
/// Host Controller Interface (Host side)
#define HCIH_ITF        (BLE_HOST_PRESENT && HCI_PRESENT)	//!BLE_EMB_PRESENT)

#define TL_TASK_SIZE     0 //GTL_ITF + HCIC_ITF + HCIH_ITF


/******************************************************************************************/
/* --------------------------   BLE COMMON DEFINITIONS      ------------------------------*/
/******************************************************************************************/
/// Kernel Heap memory sized reserved for allocate dynamically connection environment
#define KE_HEAP_MEM_RESERVED        (4)

#if defined(CFG_BLE)
/// Application role definitions
#define BLE_BROADCASTER   (defined(CFG_BROADCASTER) || defined(CFG_ALLROLES))
#define BLE_OBSERVER      (defined(CFG_OBSERVER)    || defined(CFG_ALLROLES))
#define BLE_PERIPHERAL    (defined(CFG_PERIPHERAL)  || defined(CFG_ALLROLES))
#define BLE_CENTRAL       (defined(CFG_CENTRAL)     || defined(CFG_ALLROLES))

#if (!BLE_BROADCASTER) && (!BLE_OBSERVER) && (!BLE_PERIPHERAL) && (!BLE_CENTRAL)
	#error "No application role defined"
#endif /* #if (!BLE_BROADCASTER) && (!BLE_OBSERVER) && (!BLE_PERIPHERAL) && (!BLE_CENTRAL) */


/// Maximum number of simultaneous BLE activities (scan, connection, advertising, initiating)
#define BLE_ACTIVITY_MAX          (CFG_CON)


/// Maximum number of simultaneous connections
#if (BLE_CENTRAL)
	#define BLE_CONNECTION_MAX          CFG_CON
#elif (BLE_PERIPHERAL)
	#define BLE_CONNECTION_MAX          1
#else
	#define BLE_CONNECTION_MAX          1
#endif /* #if (BLE_CENTRAL) */

/// Support of Legacy Air Operations
#if defined(CFG_DEPRECATED_API)
#define BLE_DEPRECATED_API      (1)
#else //defined(CFG_DEPRECATED_API)
#define BLE_DEPRECATED_API      (0)
#endif //defined(CFG_DEPRECATED_API)


/// Number of tx data buffers
#if (BLE_CONNECTION_MAX == 1)
#if (BLE_CENTRAL || BLE_PERIPHERAL)
#define BLE_TX_BUFFER_DATA      8//5
#else
#define BLE_TX_BUFFER_DATA      0
#endif // (BLE_CENTRAL || BLE_PERIPHERAL)
#else
#define BLE_TX_BUFFER_DATA     (BLE_CONNECTION_MAX * 3)
#endif //  BLE_CONNECTION_MAX == 1


#if (BLE_CENTRAL || BLE_PERIPHERAL)
/// Number of tx advertising buffers
#define BLE_TX_BUFFER_ADV      	3
/// Number of tx control buffers
#define BLE_TX_BUFFER_CNTL      BLE_CONNECTION_MAX
#else
#if (BLE_BROADCASTER)
/// Number of tx advertising buffers
#define BLE_TX_BUFFER_ADV       2
/// Number of tx control buffers
#define BLE_TX_BUFFER_CNTL      0
#else
/// Number of tx advertising buffers
#define BLE_TX_BUFFER_ADV       1
/// Number of tx control buffers
#define BLE_TX_BUFFER_CNTL      0
#endif // BLE_BROADCASTER
#endif // (BLE_CENTRAL || BLE_PERIPHERAL)



/// Total number of elements in the TX buffer pool
#define BLE_TX_BUFFER_CNT      (BLE_TX_BUFFER_DATA + BLE_TX_BUFFER_CNTL + BLE_TX_BUFFER_ADV)

/// Number of receive buffers in the RX ring. This number defines the interrupt
/// rate during a connection event. An interrupt is asserted every BLE_RX_BUFFER_CNT/2
/// reception. This number has an impact on the size of the exchange memory. This number
/// may have to be increased when CPU is very slow to free the received data, in order not
/// to overflow the RX ring of buffers.
#if (BLE_CENTRAL || BLE_PERIPHERAL)
	#define BLE_RX_BUFFER_CNT           4//7
#elif (BLE_BROADCASTER)
	#define BLE_RX_BUFFER_CNT           1
#else
	#define BLE_RX_BUFFER_CNT           4
#endif // BLE_CENTRAL || BLE_PERIPHERAL


/// Max advertising reports before sending the info to the host
#define BLE_ADV_REPORTS_MAX             1

/// Use of security manager block
#define RW_BLE_USE_CRYPT  (defined(CFG_SECURITY_ON)) // TODO [modularity] [KE]
#endif //defined(CFG_BLE)



/// Accelerometer Application
#define BLE_APP_TASK_SIZE    BLE_APP_PRESENT


/******************************************************************************************/
/* -------------------------   DEEP SLEEP SETUP      -------------------------------------*/
/******************************************************************************************/

/// DEEP SLEEP enable
#if defined(CFG_SLEEP) && (BLE_EMB_PRESENT || BT_EMB_PRESENT)
	#define DEEP_SLEEP                              1
#else
	#define DEEP_SLEEP                              0
#endif /* CFG_SLEEP */

/// Use 32K Hz Clock if set to 1 else 32,768k is used
#if USE_EXT_32K
#define HZ32000                                     0
#else
#define HZ32000                                     1
#endif

#if SLEEP_EN
	#if (!USE_EXT_32K)
	#define EN_CALIB_RC                             1
	#define CALIB_SLOTS                             3
	#else
	#define EN_CALIB_RC                             0
	#define CALIB_SLOTS                             0
	#endif
#else
	#define EN_CALIB_RC                             0
	#define CALIB_SLOTS                             0
#endif


/******************************************************************************************/
/* -------------------------   CHANNEL ASSESSMENT SETUP      -----------------------------*/
/******************************************************************************************/

/// Channel Assessment
#if defined(CFG_BLE)
#if (defined(CFG_CHNL_ASSESS) && BLE_CENTRAL)
	#define BLE_CHNL_ASSESS        (1)
#else
	#define BLE_CHNL_ASSESS        (0)
#endif //(defined(CFG_CHNL_ASSESS) && BLE_CENTRAL)
#endif //defined(CFG_BLE)




/******************************************************************************************/
/* --------------------------   DEBUG SETUP       ----------------------------------------*/
/******************************************************************************************/

/// Number of tasks for DEBUG module
#define DEBUG_TASK_SIZE                 0

/// Flag indicating if tester emulator is available or not
#if defined(CFG_TESTER)
	#define RW_TESTER                   1
#else
	#define RW_TESTER                   0
#endif // defined (CFG_TESTER)

/// Flag indicating if debug mode is activated or not
#if defined(CFG_DBG)
	#define RW_DEBUG                        1
#if (BLE_EMB_PRESENT || BT_EMB_PRESENT)
	#define RW_SWDIAG                      0	// 1
#else
	#define RW_SWDIAG                       0
#endif
	#define KE_PROFILING                    1
	
#else
	#define RW_DEBUG                        0
	#define RW_SWDIAG                       0
	#define KE_PROFILING                    0
#endif /* CFG_DBG */

/// Flag indicating if Read/Write memory commands are supported or not
#if defined(CFG_DBG_MEM)
	#define RW_DEBUG_MEM               1
#else //CFG_DBG_MEM
	#define RW_DEBUG_MEM               0
#endif //CFG_DBG_MEM

/// Flag indicating if Flash debug commands are supported or not
#if defined(CFG_DBG_FLASH)
	#define RW_DEBUG_FLASH                  1
#else //CFG_DBG_FLASH
	#define RW_DEBUG_FLASH                  0
#endif //CFG_DBG_FLASH

/// Flag indicating if NVDS feature is supported or not
#if defined(CFG_DBG_NVDS)
	#define RW_DEBUG_NVDS                   1
#else //CFG_DBG_NVDS
	#define RW_DEBUG_NVDS                   0
#endif //CFG_DBG_NVDS

/// Flag indicating if CPU stack profiling commands are supported or not
#if defined(CFG_DBG_STACK_PROF)
	#define RW_DEBUG_STACK_PROF             1
#else
	#define RW_DEBUG_STACK_PROF             0
#endif // defined (CFG_DBG_STACK_PROF)

/// Debug printing
#if (RW_DEBUG)
	#define WARNING(P)                      dbg_warning P
#else
	#define WARNING(P)
#endif //RW_DEBUG

/// Modem back to back setup
#define MODEM2MODEM                          0
/// Special clock testing
#define CLK_WRAPPING                         0

/******************************************************************************************/
/* --------------------------      NVDS SETUP       --------------------------------------*/
/******************************************************************************************/

/// Flag indicating if NVDS feature is supported or not
#if defined(CFG_NVDS)
	#define NVDS_SUPPORT                    1
#else //CFG_DBG_NVDS
	#define NVDS_SUPPORT                    0
#endif //CFG_DBG_NVDS


/******************************************************************************************/
/* --------------------------      MISC SETUP       --------------------------------------*/
/******************************************************************************************/
/// Manufacturer: RivieraWaves SAS
#define RW_COMP_ID                           0x0060


/******************************************************************************************/
/* -------------------------   BT / BLE / BLE HL CONFIG    -------------------------------*/
/******************************************************************************************/

#if (BLE_EMB_PRESENT) || (BLE_HOST_PRESENT)
#include "panble_config.h"   // ble stack configuration
#endif //BLE_EMB_PRESENT

#if (BLE_HOST_PRESENT)
#include "panble_hl_config.h"  // ble Host stack configuration
#endif //BLE_HOST_PRESENT

#if defined(CFG_APP)
#include "panapp_config.h"     // Audio Mode 0 configuration
#endif // defined(CFG_APP)


/******************************************************************************************/
/* -------------------------   KERNEL SETUP          -------------------------------------*/
/******************************************************************************************/

/// Flag indicating Kernel is supported
//#define KE_SUPPORT  (BLE_EMB_PRESENT || BT_EMB_PRESENT || BLE_HOST_PRESENT || BLE_APP_PRESENT)
#define KE_SUPPORT  1


/// Event types definition
enum KE_EVENT_TYPE
{
	#if BLE_EMB_PRESENT
	KE_EVENT_BLE_CRYPT       ,  // 0x00, 0x00000001
	#endif //BLE_EMB_PRESENT

	KE_EVENT_KE_MESSAGE      ,  // 0x01, 0x00000002
	KE_EVENT_KE_TIMER        ,  // 0x02, 0x00000004

	#if (GTL_ITF)
	KE_EVENT_GTL_TX_DONE     ,
	#endif //(GTL_ITF)


	#if HCIC_ITF || HCIH_ITF
	//KE_EVENT_HCI_TX_DONE     ,
	#endif //HCIC_ITF || HCIH_ITF

	#if BLE_EMB_PRESENT
	KE_EVENT_BLE_EVT_END     ,  // 0x03, 0x00000008
	KE_EVENT_BLE_RX          ,  // 0x04, 0x00000010
	KE_EVENT_BLE_EVT_START   ,  // 0x05, 0x00000020
	#endif //BLE_EMB_PRESENT

#if (BLE_HOST_PRESENT)
#if (BLE_L2CC)
	KE_EVENT_L2CAP_TX        ,  // 0x06, 0x00000040
#endif //(BLE_L2CC)
#endif// (BLE_HOST_PRESENT)



	KE_EVENT_MAX             ,
};

/// Tasks types definition, change with TASK_ID_TYPE in rwip_task.h
enum KE_TASK_TYPE
{
	// Link Layer Tasks
	TASK_LLM          = 0   ,
	TASK_LLC          = 1   ,
	TASK_LLD          = 2   ,
	TASK_DBG          = 3   ,
	
	TASK_SMPM		  = 6	,
	TASK_SMPC		  = 7	,
	TASK_HCI		  = 8	,
	TASK_DISPLAY	  = 9	,


	TASK_L2CC 	  	  = 10,
	TASK_GATTM        = 11  ,   // Generic Attribute Profile Manager Task
	TASK_GATTC        = 12  ,   // Generic Attribute Profile Controller Task
	TASK_GAPM         = 13  ,   // Generic Access Profile Manager
	TASK_GAPC         = 14  ,   // Generic Access Profile Controller

	 // allocate a certain number of profiles task
	TASK_PRF_MAX = (TASK_GAPC + BLE_NB_PROFILES + 1),   // no use 
	
	
	TASK_APP		  = 26,		//for general GAP operation
	TASK_GTL		 	,

	TASK_MAX 		   = 30,
	TASK_NONE = 0xFF,
};


/// Kernel memory heaps types.
enum
{
	/// Memory allocated for environment variables
	KE_MEM_ENV,
	#if (BLE_HOST_PRESENT)
	/// Memory allocated for Attribute database
	KE_MEM_ATT_DB,
	#endif // (BLE_HOST_PRESENT)
	/// Memory allocated for kernel messages
	KE_MEM_KE_MSG,
	/// Non Retention memory block
	KE_MEM_NON_RETENTION,
	KE_MEM_BLOCK_MAX,
};


#define BT_TASK_SIZE_          0
#define BT_HEAP_MSG_SIZE_      0
#define BT_HEAP_ENV_SIZE_      0


#define BLE_TASK_SIZE_         BLE_TASK_SIZE		// 3
#define BLE_HEAP_MSG_SIZE_     BLE_HEAP_MSG_SIZE
#define BLE_HEAP_ENV_SIZE_     BLE_HEAP_ENV_SIZE

#if (BLE_HOST_PRESENT)
#define BLEHL_TASK_SIZE_       BLEHL_TASK_SIZE
#define BLEHL_HEAP_MSG_SIZE_   BLEHL_HEAP_MSG_SIZE
#define BLEHL_HEAP_ENV_SIZE_   BLEHL_HEAP_ENV_SIZE
#define BLEHL_HEAP_DB_SIZE_    BLEHL_HEAP_DB_SIZE
#else
#define BLEHL_TASK_SIZE_       0
#define BLEHL_HEAP_MSG_SIZE_   0
#define BLEHL_HEAP_ENV_SIZE_   0
#define BLEHL_HEAP_DB_SIZE_    0
#endif //BLE_HOST_PRESENT
//DYC_ADD
#define APP_TASK_SIZE_          15



/// Number of Kernel tasks
#define KE_TASK_SIZE             (  DEBUG_TASK_SIZE       + \
									TL_TASK_SIZE          + \
									BLE_APP_TASK_SIZE     + \
									BT_TASK_SIZE_         + \
									BLE_TASK_SIZE_        + \
									APP_TASK_SIZE_        + \
									BLEHL_TASK_SIZE_         )

//Add later

  /// Kernel Message Heap
#define RWIP_HEAP_MSG_SIZE         (  BT_HEAP_MSG_SIZE_      + \
									BLE_HEAP_MSG_SIZE_     + \
									BLEHL_HEAP_MSG_SIZE_      )
								  
/// Number of link in kernel environment TODO [FBE] add a define in scons build
#define KE_NB_LINK_IN_HEAP_ENV   4

/// Size of Environment heap
#define RWIP_HEAP_ENV_SIZE         (( BT_HEAP_ENV_SIZE_         + \
									  BLE_HEAP_ENV_SIZE_        + \
									  BLEHL_HEAP_ENV_SIZE_       )\
									  * KE_NB_LINK_IN_HEAP_ENV)

/// Size of Attribute database heap
#define RWIP_HEAP_DB_SIZE         (  BLEHL_HEAP_DB_SIZE_  )

/// Size of non retention heap - 1024 bytes per ble link should be sufficient and can be tuned
#if (BLE_EMB_PRESENT)
#define RWIP_HEAP_NON_RET_SIZE    ( 1024 * BLE_CONNECTION_MAX )
#else
#define RWIP_HEAP_NON_RET_SIZE    ( 1024 )
#endif


/// @} BT Stack Configuration
/// @} ROOT


// Debug Config
#define DBG_GATTC_EN                0
#define DBG_GATTC_TASK_EN           0



#endif //PANIP_CONFIG_H_
