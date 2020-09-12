#include "tuya_ble_master.h"
#include "tuya_ble_stdlib.h"
#include "tuya_ble_type.h"
#include "tuya_ble_heap.h"
#include "tuya_ble_mem.h"
#include "tuya_ble_api.h"
#include "tuya_ble_port.h"
#include "tuya_ble_main.h"
#include "tuya_ble_internal_config.h"
#include "tuya_ble_data_handler.h"
#include "tuya_ble_mutli_tsf_protocol.h"
#include "tuya_ble_utils.h"
#include "tuya_ble_secure.h"
#include "tuya_ble_main.h"
#include "tuya_ble_storage.h"
#include "tuya_ble_unix_time.h"
#include "tuya_ble_log.h"
#include "tuya_ble_gatt_send_queue.h"




/*********************************************************************
 * LOCAL CONSTANT
 */

/*********************************************************************
 * LOCAL STRUCT
 */

/*********************************************************************
 * LOCAL VARIABLE
 */

/*********************************************************************
 * VARIABLE
 */

/*********************************************************************
 * LOCAL FUNCTION
 */




/*********************************************************
FN: 
*/
void tuya_ble_master_port_scan_start(suble_scan_result_handler_t handler)
{
    suble_scan_start(handler);
}

/*********************************************************
FN: 
*/
void tuya_ble_master_port_scan_stop(void)
{
    suble_scan_stop();
}

/*********************************************************
FN: 
*/
void tuya_ble_master_port_connect(ble_gap_addr_t* pMac, suble_connect_result_handler_t handler)
{
    suble_gap_connect(pMac, handler);
}

/*********************************************************
FN: 
*/
void tuya_ble_master_port_disconnect(void)
{
    suble_gap_disconnect(g_conn_info[1].conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
}

/*********************************************************
FN: 
*/
void tuya_ble_master_port_db_discovery_start(suble_svc_result_handler_t handler)
{
    suble_db_discovery_start(handler);
}

/*********************************************************
FN: 
*/
void tuya_ble_master_port_notify_enable(void)
{
    extern void* p_m_ble_nus_c;
    ret_code_t err_code = ble_nus_c_tx_notif_enable(p_m_ble_nus_c);
    APP_ERROR_CHECK(err_code);
}

/*********************************************************
FN: 
*/
void tuya_ble_master_port_evt_register(tuya_ble_app_master_result_handler_t handler)
{
    tuya_ble_app_master_evt_register(handler);
}

/*********************************************************
FN: 
*/
uint32_t tuya_ble_master_port_nv_write(u32 area_id, u16 id, void *buf, u8 size)
{
    return sf_nv_write(area_id, id, buf, size);
}

/*********************************************************
FN: 
*/
uint32_t tuya_ble_master_port_nv_read(u32 area_id, u16 id, void *buf, u8 size)
{
    return sf_nv_read(area_id, id, buf, size);
}

/*********************************************************
FN: 
*/
uint32_t tuya_ble_master_port_nv_delete(u32 area_id, u16 id)
{
    return sf_nv_delete(area_id, id);
}

/*********************************************************
FN: 
*/
void tuya_ble_master_port_reverse_byte(void* buf, uint32_t size)
{
    suble_util_reverse_byte(buf, size);
}

/*********************************************************
FN: 
*/
void tuya_ble_master_port_delete_slave_info(uint16_t slaveid)
{
    lock_hard_delete_all_by_slaveid(slaveid);
}























