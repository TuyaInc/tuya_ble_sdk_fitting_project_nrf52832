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


#ifndef __TUYA_BLE_MASTER_PORT_H__
#define __TUYA_BLE_MASTER_PORT_H__

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

/*********************************************************************
 * STRUCT
 */

/*********************************************************************
 * EXTERNAL VARIABLE
 */

/*********************************************************************
 * EXTERNAL FUNCTION
 */
void tuya_ble_master_port_scan_start(suble_scan_result_handler_t handler);
void tuya_ble_master_port_scan_stop(void);
void tuya_ble_master_port_connect(ble_gap_addr_t* pMac, suble_connect_result_handler_t handler);
void tuya_ble_master_port_disconnect(void);
void tuya_ble_master_port_db_discovery_start(suble_svc_result_handler_t handler);
void tuya_ble_master_port_notify_enable(void);
void tuya_ble_master_port_evt_register(tuya_ble_app_master_result_handler_t handler);
uint32_t tuya_ble_master_port_nv_write(u32 area_id, u16 id, void *buf, u8 size);
uint32_t tuya_ble_master_port_nv_read(u32 area_id, u16 id, void *buf, u8 size);
uint32_t tuya_ble_master_port_nv_delete(u32 area_id, u16 id);
void tuya_ble_master_port_reverse_byte(void* buf, uint32_t size);
void tuya_ble_master_port_delete_slave_info(uint16_t slaveid);


#ifdef __cplusplus
}
#endif

#endif //__TUYA_BLE_MASTER_PORT_H__
