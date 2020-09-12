/**
****************************************************************************
* @file      tuya_ble_port_bk3431q.h
* @brief     tuya_ble_port_bk3431q
* @author    suding
* @version   V1.0.0
* @date      2020-04
* @note
******************************************************************************
* @attention
*
* <h2><center>&copy; COPYRIGHT 2020 Tuya </center></h2>
*/


#ifndef __TUYA_BLE_PORT_BK3431Q_H__
#define __TUYA_BLE_PORT_BK3431Q_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDE
 */
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "stdint.h"
#include "aes.h"
#include "md5.h"
#include "hmac.h"
#include "tuya_ble_config.h"
#include "tuya_ble_type.h"

/*********************************************************************
 * CONSTANT
 */
#if (TUYA_BLE_LOG_ENABLE||TUYA_APP_LOG_ENABLE)
    #define TUYA_BLE_PRINTF(...)            log_d(__VA_ARGS__)
    #define TUYA_BLE_HEXDUMP(...)           suble_log_hexdump_for_tuya_ble_sdk("", 8, __VA_ARGS__)
#else
    #define TUYA_BLE_PRINTF(...)
    #define TUYA_BLE_HEXDUMP(...)
#endif

#include "suble_common.h"
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

#endif //__TUYA_BLE_PORT_BK3431Q_H__



