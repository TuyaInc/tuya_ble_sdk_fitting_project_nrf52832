/**
****************************************************************************
* @file      tuya_ble_master.h
* @brief     tuya_ble_master
* @author    suding
* @version   V1.0.0
* @date      2020-04
* @note
******************************************************************************
* @attention
*
* <h2><center>&copy; COPYRIGHT 2020 Tuya </center></h2>
*/


#ifndef __TUYA_BLE_MASTER_H__
#define __TUYA_BLE_MASTER_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDE
 */
#include "app_common.h"

/*********************************************************************
 * CONSTANT
 */
typedef enum
{
    TUYA_BLE_MASTER_EVT_TIMEOUT = 0x00,
    TUYA_BLE_MASTER_EVT_SLAVEID_INVALID,
    TUYA_BLE_MASTER_EVT_SCAN_TIMEOUT,
    TUYA_BLE_MASTER_EVT_CONNECT_TIMEOUT,
    TUYA_BLE_MASTER_EVT_BONDING,
    TUYA_BLE_MASTER_EVT_OPEN_WITH_MASTER_SUCCESS,
    TUYA_BLE_MASTER_EVT_OPEN_WITH_MASTER_FAILURE,
    TUYA_BLE_MASTER_EVT_DISCONNECT,
} tuya_ble_master_evt_t;

typedef enum
{
    TUYA_BLE_MASTER_OPERATION_CLOSE = 0x00,
    TUYA_BLE_MASTER_OPERATION_OPEN,
} tuya_ble_master_operation_t;

typedef enum
{
    TUYA_BLE_MASTER_METH_PHONE = 0x00,
    TUYA_BLE_MASTER_METH_KEY,
} tuya_ble_master_open_close_meth_t;

#define TUYA_BLE_MASTER_METH_INFO_DEFAULT_VALUE 0x00

/*********************************************************************
 * STRUCT
 */
#pragma pack(1)
typedef struct
{
    uint16_t id;
    uint8_t  mac[6];
    uint8_t  device_id[DEVICE_ID_LEN];
    uint8_t  login_key[LOGIN_KEY_LEN];
} slave_info_t;

typedef struct
{
    uint16_t slaveid;
    uint32_t timestamp;
} open_with_master_record_report_info_t;
#pragma pack()

typedef void (*tuya_ble_master_evt_handler_t)(uint32_t evt, uint8_t* buf, uint32_t size);

/*********************************************************************
 * EXTERNAL VARIABLE
 */

/*********************************************************************
 * EXTERNAL FUNCTION
 */
tuya_ble_status_t tuya_ble_master_info_init(slave_info_t* info, uint8_t slave_max_num);

void tuya_ble_master_data_passthrough_with_phone(void* buf, uint32_t size);

tuya_ble_status_t tuya_ble_master_scan_start(int32_t slaveid, tuya_ble_master_evt_handler_t handler);
tuya_ble_status_t tuya_ble_master_send_data(void* buf, uint32_t size);
tuya_ble_status_t tuya_ble_master_open_with_master(uint8_t operation, uint8_t open_meth, void* open_meth_info, uint8_t open_meth_info_size);


#ifdef __cplusplus
}
#endif

#endif //__TUYA_BLE_MASTER_H__
