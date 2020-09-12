#include "app_port.h"




/*********************************************************************
 * LOCAL CONSTANTS
 */

/*********************************************************************
 * LOCAL STRUCT
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

/*********************************************************************
 * LOCAL FUNCTION
 */

/*********************************************************************
 * VARIABLES
 */




/*********************************************************  Storage  *********************************************************/

/*********************************************************
FN: 
*/
void *app_port_malloc(uint32_t size)
{
    return tuya_ble_malloc(size);
}

/*********************************************************
FN: 
*/
uint32_t app_port_free(void* buf)
{
    return tuya_ble_free(buf);
}

/*********************************************************
FN: 
*/
uint32_t app_port_nv_init(void)
{
    sf_nv_init(SF_AREA_0);
    sf_nv_init(SF_AREA_1);
    sf_nv_init(SF_AREA_2);
    sf_nv_init(SF_AREA_3);
    sf_nv_init(SF_AREA_4);
    return APP_PORT_SUCCESS;
}

/*********************************************************
FN: 
*/
uint32_t app_port_nv_set(uint32_t area_id, uint16_t id, void *buf, uint8_t size)
{
    return sf_nv_write(area_id, id, buf, size);
}

/*********************************************************
FN: 
*/
uint32_t app_port_nv_get(uint32_t area_id, uint16_t id, void *buf, uint8_t size)
{
    return sf_nv_read(area_id, id, buf, size);
}

/*********************************************************
FN: 
*/
uint32_t app_port_nv_del(uint32_t area_id, uint16_t id)
{
    return sf_nv_delete(area_id, id);
}

/*********************************************************
FN: 
*/
uint32_t app_port_nv_set_default(void)
{
    sf_port_flash_erase(SF_AREA0_BASE, 2);
    sf_port_flash_erase(SF_AREA1_BASE, 2);
    sf_port_flash_erase(SF_AREA2_BASE, 2);
    sf_port_flash_erase(SF_AREA3_BASE, 2);
    sf_port_flash_erase(SF_AREA4_BASE, 2);
    return APP_PORT_SUCCESS;
}

/*********************************************************
FN: 
*/
uint32_t app_port_nv_write(uint32_t addr, const uint8_t* p_data, uint32_t size)
{
    return tuya_ble_nv_write(addr, p_data, size);
}

/*********************************************************
FN: 
*/
uint32_t app_port_nv_read(uint32_t addr, uint8_t* p_data, uint32_t size)
{
    return tuya_ble_nv_read(addr, p_data, size);
}

/*********************************************************
FN: 
*/
uint32_t app_port_nv_erase(uint32_t addr, uint32_t size)
{
    return tuya_ble_nv_erase(addr, size);
}


/*********************************************************  timer  *********************************************************/

/*********************************************************
FN: 
*/
uint32_t app_port_timer_create(void** p_timer_id, uint32_t timeout_value_ms, uint8_t mode, suble_timer_handler_t timeout_handler)
{
    return suble_timer_create(p_timer_id, timeout_value_ms, (suble_timer_mode_t)mode, timeout_handler);
}

/*********************************************************
FN: 
*/
uint32_t app_port_timer_delete(void* timer_id)
{
    return suble_timer_delete(timer_id);
}

/*********************************************************
FN: 
*/
uint32_t app_port_timer_start(void* timer_id)
{
    return suble_timer_start(timer_id);
}

/*********************************************************
FN: 
*/
uint32_t app_port_timer_stop(void* timer_id)
{
    return suble_timer_stop(timer_id);
}

/*********************************************************
FN: 
*/
uint32_t app_port_update_timestamp(uint32_t app_timestamp)
{
    suble_update_timestamp(app_timestamp);
    return APP_PORT_SUCCESS;
}

/*********************************************************
FN: if timestamp is not updated from poweron, return value is local timestamp
*/
uint32_t app_port_get_timestamp(void)
{
    return suble_get_timestamp();
}

/*********************************************************
FN: 
*/
uint32_t app_port_get_old_timestamp(uint32_t old_local_timestamp)
{
    return suble_get_old_timestamp(old_local_timestamp);
}

/*********************************************************
FN: 
*/
uint32_t app_port_delay_ms(uint32_t ms)
{
    suble_delay_ms(ms);
    return APP_PORT_SUCCESS;
}

/*********************************************************  ble  *********************************************************/

/*********************************************************
FN: 
*/
uint32_t app_port_dp_data_report(uint8_t *buf, uint32_t size)
{
    if(app_port_get_connect_status() == BONDING_CONN)
    {
        TUYA_APP_LOG_HEXDUMP_INFO("dp_rsp", buf, size);
        return tuya_ble_dp_data_report(buf, size);
    } else {
        TUYA_APP_LOG_HEXDUMP_INFO("dp_rsp_unconn", buf, size);
        return APP_PORT_ERROR_COMMON;
    }
}

/*********************************************************
FN: 
*/
uint32_t app_port_dp_data_with_time_report(uint32_t timestamp, uint8_t *buf, uint32_t size)
{
    if(app_port_get_connect_status() == BONDING_CONN)
    {
        TUYA_APP_LOG_INFO("timestamp: %d", timestamp);
        TUYA_APP_LOG_HEXDUMP_INFO("dp_rsp_timestamp", buf, size);
        return tuya_ble_dp_data_with_time_report(timestamp, buf, size);
    } else {
        TUYA_APP_LOG_INFO("timestamp: %d", timestamp);
        TUYA_APP_LOG_HEXDUMP_INFO("dp_rsp_timestamp_unconn", buf, size);
        return APP_PORT_ERROR_COMMON;
    }
}

/*********************************************************
FN: 
*/
uint32_t app_port_ota_rsp(tuya_ble_ota_response_t *rsp)
{
    if(rsp->type != TUYA_BLE_OTA_DATA)
    {
        TUYA_APP_LOG_INFO("ota_rsp_type: %d", rsp->type);
        TUYA_APP_LOG_HEXDUMP_INFO("ota_rsp_data", rsp->p_data, rsp->data_len);
    }
    return tuya_ble_ota_response(rsp);
}

/*********************************************************
FN: 
*/
tuya_ble_connect_status_t app_port_get_connect_status(void)
{
    return tuya_ble_connect_status_get();
}

/*********************************************************
FN: 
*/
void app_port_set_bt_mac_addr(uint8_t* addr)
{
    suble_gap_set_bt_mac(addr);
}

/*********************************************************
FN: 
*/
void app_port_get_bt_mac_addr(uint8_t* addr)
{
    if(addr == NULL) {
        return;
    }
    
    suble_gap_get_bt_mac(addr, 6);
}

/*********************************************************
FN: 
*/
uint32_t app_port_conn_param_update(uint16_t cMin, uint16_t cMax, uint16_t latency, uint16_t timeout)
{
    suble_gap_conn_param_update(g_conn_info[0].conn_handle, cMin, cMax, latency, timeout);
    return APP_PORT_SUCCESS;
}

/*********************************************************
FN: 
*/
uint32_t app_port_ble_gap_disconnect(void)
{
    suble_gap_disconnect(g_conn_info[0].conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
    return APP_PORT_SUCCESS;
}

/*********************************************************
FN: 
*/
uint32_t app_port_ble_conn_evt_ext(void)
{
    uint32_t err_code;
    ble_opt_t  opt;
    memset(&opt, 0x00, sizeof(opt));
    opt.common_opt.conn_evt_ext.enable = true;
    err_code = sd_ble_opt_set(BLE_COMMON_OPT_CONN_EVT_EXT, &opt);
    APP_ERROR_CHECK(err_code);
    return err_code;
}




/*********************************************************  device  *********************************************************/

/*********************************************************
FN: 
*/
uint32_t app_port_device_reset(void)
{
    TUYA_APP_LOG_INFO("device reset");
    tuya_ble_device_reset();
    return APP_PORT_SUCCESS;
}

/*********************************************************
FN: 
*/
uint32_t app_port_local_clock_start(void)
{
//    suble_local_timer_start(); //main�������Ѿ�����
    return APP_PORT_SUCCESS;
}

/*********************************************************
FN: 
*/
uint32_t app_port_ble_sdk_init(tuya_ble_device_param_t * param_data)
{
    return tuya_ble_sdk_init(param_data);
}

/*********************************************************
FN: 
*/
uint32_t app_port_ble_callback_queue_register(tuya_ble_callback_t cb)
{
    return tuya_ble_callback_queue_register(cb);
}

/*********************************************************
FN: 
*/
uint8_t app_port_ble_custom_event_send(tuya_ble_custom_evt_t evt)
{
    return tuya_ble_custom_event_send(evt);
}

/*********************************************************
FN: 
*/
uint32_t app_port_uart_send_data(const uint8_t* buf,uint16_t size)
{
    suble_uart1_send(buf, size);
    return APP_PORT_SUCCESS;
}

/*********************************************************
FN: 
*/
void app_port_factory_test_process(uint8_t* p_in_data, uint16_t in_len, uint8_t* p_out_data, uint16_t* out_len)
{
    factory_test_process(p_in_data, in_len, p_out_data, out_len);
}

/*********************************************************
FN: 
*/
uint32_t app_port_dfu_settings_init(bool sd_irq_initialized)
{
    return nrf_dfu_settings_init(sd_irq_initialized);
}

/*********************************************************
FN: 
*/
uint32_t app_port_dfu_settings_write_and_backup(nrf_dfu_flash_callback_t callback)
{
    return nrf_dfu_settings_write_and_backup(callback);
}

/*********************************************************
FN: 
*/
uint32_t app_port_tuya_ble_device_factory_reset(void)
{
    return tuya_ble_device_factory_reset();
}




/*********************************************************  check  *********************************************************/

/*********************************************************
FN: 
*/
uint8_t app_port_check_sum(uint8_t *buf, uint32_t size)
{
    return suble_util_check_sum8(buf, size);
}

/*********************************************************
FN: 
*/
uint16_t app_port_check_sum_16(uint8_t *buf, uint32_t size)
{
    return suble_util_check_sum16(buf, size);
}

/*********************************************************
FN: crc16
*/
uint16_t app_port_crc16_compute(uint8_t* buf, uint16_t size, uint16_t* p_crc)
{
    return suble_util_crc16(buf, size, p_crc);
}

/*********************************************************
FN: crc32
*/
uint32_t app_port_crc32_compute(uint8_t* buf, uint32_t size, uint32_t* p_crc)
{
    return suble_util_crc32(buf, size, p_crc);
}

/*********************************************************
FN: 
*/
void app_port_reverse_byte(void* buf, uint32_t size)
{
    suble_util_reverse_byte(buf, size);
}

/*********************************************************
FN: 
*/
uint32_t app_port_num_array_2_int(uint8_t *num_array, uint32_t start_idx, uint32_t size)
{
    return suble_util_numarray2int(num_array, start_idx, size);
}

/*********************************************************
FN: 
*/
bool app_port_aes128_cbc_encrypt(uint8_t *key,uint8_t *iv,uint8_t *input,uint16_t input_len,uint8_t *output)
{
    return tuya_ble_aes128_cbc_encrypt(key, iv, input, input_len, output);
}




/*********************************************************  string  *********************************************************/

/*********************************************************
FN: string handle
*/
uint8_t app_port_string_op_hexstr2hex(uint8_t *hexstr, int len, uint8_t* hex)
{
	return suble_util_str_hexstr2hexarray(hexstr, len, hex);
}

uint8_t app_port_string_op_hex2hexstr(uint8_t *hex, int len, uint8_t* hexstr)
{
	return suble_util_str_hexarray2hexstr(hex, len, hexstr);
}

uint8_t app_port_string_op_intstr2int(uint8_t *hex, int len, int* sum)
{
	return suble_util_str_intstr2int(hex, len, sum);
}









