/**
****************************************************************************
* @file      app_port.h
* @brief     app_port
* @author    suding
* @version   V1.0.0
* @date      2019-09-11
* @note
******************************************************************************
* @attention
*
* <h2><center>&copy; COPYRIGHT 2019 Tuya </center></h2>
*/


#ifndef __APP_PORT_H__
#define __APP_PORT_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
//cpt
#include "hmac.h"
#include "sf_port.h"
#include "hmac-sha1.h"
//tuya_ble_sdk
#include "tuya_ble_api.h"
#include "tuya_ble_log.h"
#include "tuya_ble_mem.h"
#include "tuya_ble_mutli_tsf_protocol.h"
#include "tuya_ble_master.h"
#include "tuya_ble_main.h" //tuya_ble_current_para
#include "tuya_ble_unix_time.h" //tuya_ble_time_struct_data_t
#include "tuya_ble_app_demo.h" //tuya_ble_time_struct_data_t
//suble
#include "suble_common.h"

/*********************************************************************
 * CONSTANTS
 */
/*********************************************************  app_port  *********************************************************/
//addr len
#define APP_PORT_BLE_ADDR_LEN           (SUBLE_BT_MAC_LEN)
#define APP_PORT_BLE_ADDR_STR_LEN       (APP_PORT_BLE_ADDR_LEN*2)

//return value
typedef enum {
    APP_PORT_SUCCESS  = 0x00,
    APP_PORT_ERROR_COMMON  = 0x01,
} app_port_status_t;

/*********************************************************  tuya_ble_sdk  *********************************************************/
//pid len
#define APP_PORT_PID_LEN                8

//tuya_ble_sdk data type
#define APP_PORT_DT_RAW                 DT_RAW    
#define APP_PORT_DT_BOOL                DT_BOOL   
#define APP_PORT_DT_VALUE               DT_VALUE  
#define APP_PORT_DT_INT                 DT_INT    
#define APP_PORT_DT_STRING              DT_STRING 
#define APP_PORT_DT_ENUM                DT_ENUM   
#define APP_PORT_DT_BITMAP              DT_BITMAP 
#define APP_PORT_DT_CHAR                DT_CHAR   
#define APP_PORT_DT_UCHAR               DT_UCHAR  
#define APP_PORT_DT_SHORT               DT_SHORT  
#define APP_PORT_DT_USHORT              DT_USHORT 
#define APP_PORT_DT_LMT                 DT_LMT    
//tuya_ble_sdk data type len
#define APP_PORT_DT_VALUE_LEN           DT_VALUE_LEN
#define APP_PORT_DT_BOOL_LEN            DT_BOOL_LEN
#define APP_PORT_DT_ENUM_LEN            DT_ENUM_LEN
#define APP_PORT_DT_BITMAP_MAX          DT_BITMAP_MAX
#define APP_PORT_DT_STR_MAX             DT_STR_MAX
#define APP_PORT_DT_RAW_MAX             DT_RAW_MAX
#define APP_PORT_DT_INT_LEN             DT_INT_LEN

/*********************************************************************
 * STRUCT
 */

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */
/*********************************************************  Storage  *********************************************************/
void *app_port_malloc(uint32_t size);
uint32_t app_port_free(void* buf);
uint32_t app_port_nv_init(void);
uint32_t app_port_nv_set(uint32_t area_id, uint16_t id, void *buf, uint8_t size);
uint32_t app_port_nv_get(uint32_t area_id, uint16_t id, void *buf, uint8_t size);
uint32_t app_port_nv_del(uint32_t area_id, uint16_t id);
uint32_t app_port_nv_set_default(void);
uint32_t app_port_nv_write(uint32_t addr, const uint8_t* p_data, uint32_t size);
uint32_t app_port_nv_read(uint32_t addr, uint8_t* p_data, uint32_t size);
uint32_t app_port_nv_erase(uint32_t addr, uint32_t size);

/*********************************************************  timer  *********************************************************/
uint32_t app_port_timer_create(void** p_timer_id, uint32_t timeout_value_ms, uint8_t mode, suble_timer_handler_t timeout_handler);
uint32_t app_port_timer_delete(void* timer_id);
uint32_t app_port_timer_start(void* timer_id);
uint32_t app_port_timer_stop(void* timer_id);
uint32_t app_port_update_timestamp(uint32_t app_timestamp);
uint32_t app_port_get_timestamp(void);
uint32_t app_port_get_old_timestamp(uint32_t old_local_timestamp);
uint32_t app_port_delay_ms(uint32_t ms);

/*********************************************************  ble  *********************************************************/
uint32_t app_port_dp_data_report(uint8_t *buf, uint32_t size);
uint32_t app_port_dp_data_with_time_report(uint32_t timestamp, uint8_t *buf, uint32_t size);
uint32_t app_port_ota_rsp(tuya_ble_ota_response_t *rsp);
tuya_ble_connect_status_t app_port_get_connect_status(void);
void app_port_set_bt_mac_addr(uint8_t* addr);
void app_port_get_bt_mac_addr(uint8_t* addr);
uint32_t app_port_conn_param_update(uint16_t cMin, uint16_t cMax, uint16_t latency, uint16_t timeout);
uint32_t app_port_ble_gap_disconnect(void);
uint32_t app_port_ble_conn_evt_ext(void);

/*********************************************************  device  *********************************************************/
uint32_t app_port_device_reset(void);
uint32_t app_port_local_clock_start(void);
uint32_t app_port_ble_sdk_init(tuya_ble_device_param_t * param_data);
uint32_t app_port_ble_callback_queue_register(tuya_ble_callback_t cb);
uint8_t app_port_ble_custom_event_send(tuya_ble_custom_evt_t evt);
uint32_t app_port_uart_send_data(const uint8_t* buf,uint16_t size);
void app_port_factory_test_process(uint8_t* p_in_data, uint16_t in_len, uint8_t* p_out_data, uint16_t* out_len);
uint32_t app_port_dfu_settings_init(bool sd_irq_initialized);
uint32_t app_port_dfu_settings_write_and_backup(nrf_dfu_flash_callback_t callback);
uint32_t app_port_tuya_ble_device_factory_reset(void);

/*********************************************************  check  *********************************************************/
uint8_t app_port_check_sum(uint8_t *buf, uint32_t size);
uint16_t app_port_check_sum_16(uint8_t *buf, uint32_t size);
uint16_t app_port_crc16_compute(uint8_t* buf, uint16_t size, uint16_t* p_crc);
uint32_t app_port_crc32_compute(uint8_t* buf, uint32_t size, uint32_t* p_crc);
void app_port_reverse_byte(void* buf, uint32_t size);
uint32_t app_port_num_array_2_int(uint8_t *num_array, uint32_t start_idx, uint32_t size);
bool app_port_aes128_cbc_encrypt(uint8_t *key,uint8_t *iv,uint8_t *input,uint16_t input_len,uint8_t *output);

/*********************************************************  string  *********************************************************/
uint8_t app_port_string_op_hexstr2hex(uint8_t *hexstr, int len, uint8_t* hex);
uint8_t app_port_string_op_hex2hexstr(uint8_t *hex, int len, uint8_t* hexstr);
uint8_t app_port_string_op_intstr2int(uint8_t *hex, int len, int* sum);


#ifdef __cplusplus
}
#endif

#endif //__APP_PORT_H__
