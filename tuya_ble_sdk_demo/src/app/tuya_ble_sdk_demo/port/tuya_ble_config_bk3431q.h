/**
****************************************************************************
* @file      tuya_ble_config_bk3431q.h
* @brief     tuya_ble_config_bk3431q
* @author    suding
* @version   V1.0.0
* @date      2020-04
* @note
******************************************************************************
* @attention
*
* <h2><center>&copy; COPYRIGHT 2020 Tuya </center></h2>
*/


#ifndef __TUYA_BLE_CONFIG_BK3431Q_H__
#define __TUYA_BLE_CONFIG_BK3431Q_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDE
 */
#include "tuya_ble_type.h"

/*********************************************************************
 * CONSTANT
 */
#define TUYA_BLE_PORT_PLATFORM_HEADER_FILE                  "tuya_ble_port_bk3431q.h"
#define CUSTOMIZED_TUYA_BLE_APP_PRODUCT_TEST_HEADER_FILE    "tuya_ble_app_product_test.h"
#define CUSTOMIZED_TUYA_BLE_APP_UART_COMMON_HEADER_FILE     "tuya_ble_app_uart_module_handler.h"

#define TUYA_BLE_USE_OS 0

#if TUYA_BLE_USE_OS

#define TUYA_BLE_SELF_BUILT_TASK  0

#if TUYA_BLE_SELF_BUILT_TASK

#define TUYA_BLE_TASK_PRIORITY  1

#define TUYA_BLE_TASK_STACK_SIZE  256 * 10   //!<  Task stack size ，application do not change this default value

#endif

#endif

/*
 * TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_BLE                   ble normal
 * TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_REGISTER_FROM_BLE     device register from ble 
 * TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_MESH                  ble mesh
 * TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_WIFI_24G              wifi_2.4g
 * TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_WIFI_5G               wifi_5g 
 * TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_ZIGBEE                zigbee
 * TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_NB                    nb-iot
 * @note: 
 * for example (TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_BLE|TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_MESH|TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_WIFI_24G)
 */
//#define TUYA_BLE_DEVICE_COMMUNICATION_ABILITY  (TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_BLE|TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_WIFI_24G|TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_WIFI_5G)

#define TUYA_BLE_DEVICE_COMMUNICATION_ABILITY  (TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_BLE|TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_REGISTER_FROM_BLE)

/*
 * If TUYA_BLE_WIFI_DEVICE_REGISTER_MODE is 1,Mobile app must first sends instructions to query network status after sending the pairing request .
 */
#define TUYA_BLE_WIFI_DEVICE_REGISTER_MODE  1

/*
 * if 1 ,tuya ble sdk authorization self-management
 */
#define  TUYA_BLE_DEVICE_AUTH_SELF_MANAGEMENT   1

 /*
  * TUYA_BLE_SECURE_CONNECTION_WITH_AUTH_KEY     encrypt with auth key
  * TUYA_BLE_SECURE_CONNECTION_WITH_ECC          encrypt with ECDH
  * TUYA_BLE_SECURE_CONNECTION_WTIH_PASSTHROUGH  no encrypt 
  * @note : only choose one 
 */
#define  TUYA_BLE_SECURE_CONNECTION_TYPE  TUYA_BLE_SECURE_CONNECTION_WITH_AUTH_KEY
 
 
/*
 * if 1 ,ble sdk will update mac address with with the address of the authorization information.
 */ 
#define  TUYA_BLE_DEVICE_MAC_UPDATE         1
 
/*
 * if 1 ,after update mac will reset.
 */
#define  TUYA_BLE_DEVICE_MAC_UPDATE_RESET    0 

#define  TUYA_BLE_USE_PLATFORM_MEMORY_HEAP   0 

#define TUYA_BLE_GATT_SEND_DATA_QUEUE_SIZE   20



/*
 * gatt mtu max sizes 
 */
#define TUYA_BLE_DATA_MTU_MAX  20


/*
 * if defined ,enable sdk log output
 */
#define TUYA_BLE_LOG_ENABLE         0

#define TUYA_BLE_LOG_COLORS_ENABLE  0

#define TUYA_BLE_LOG_LEVEL  TUYA_BLE_LOG_LEVEL_DEBUG

/*
 * if defined ,enable app log output
 */
#define TUYA_APP_LOG_ENABLE         1 

#define TUYA_APP_LOG_COLORS_ENABLE  0

#define TUYA_APP_LOG_LEVEL  TUYA_APP_LOG_LEVEL_DEBUG

/*
 * MACRO for advanced encryption,if 1 will use user rand check.
 */
#define TUYA_BLE_ADVANCED_ENCRYPTION_DEVICE 0




//nv
/* The minimum size of flash erasure. May be a flash sector size. */
#define TUYA_NV_ERASE_MIN_SIZE         (0x1000)

/* the flash write granularity, unit: byte*/
#define TUYA_NV_WRITE_GRAN             (4)

/* start address */
#define TUYA_NV_START_ADDR              SUBLE_FLASH_OTA_END_ADDR

/* area size. */
#define TUYA_NV_AREA_SIZE              (4*TUYA_NV_ERASE_MIN_SIZE)


#define TUYA_BLE_APP_VERSION_STRING         TUYA_DEVICE_FVER_STR

#define TUYA_BLE_APP_BUILD_FIRMNAME_STRING  TUYA_DEVICE_FIR_NAME

/*********************************************************************
 * STRUCT
 */

/*********************************************************************
 * EXTERNAL VARIABLE
 */

/*********************************************************************
 * EXTERNAL FUNCTION
 */

#ifdef __cplusplus
}
#endif

#endif //__TUYA_BLE_CONFIG_BK3431Q_H__



