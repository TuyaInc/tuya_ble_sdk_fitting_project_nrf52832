/**
****************************************************************************
* @file      suble_common.h
* @brief     suble_common
* @author    suding
* @version   V1.0.0
* @date      2020-04
* @note
******************************************************************************
* @attention
*
* <h2><center>&copy; COPYRIGHT 2020 Tuya </center></h2>
*/


#ifndef __SUBLE_COMMON_H__
#define __SUBLE_COMMON_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDE
 */
#include "sdk_config.h"
#include "nordic_common.h"
#include "nrf.h"
#include "bsp_btn_ble.h"
#include "nrfx_rtc.h"
#include "nrf_sdh.h"
#include "nrf_sdm.h"
#include "nrf_sdh_soc.h"
#include "nrf_drv_clock.h"
#include "nrf_drv_rtc.h"
#include "nrf_fstorage.h"
#include "nrf_fstorage_sd.h"
#include "nrf_sdh_ble.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_delay.h"
#include "nrf_uart.h"
#include "nrf_ble_qwr.h"
#include "nrf_ble_scan.h"
#include "nrf_ble_gatt.h"
#include "nrf_dfu_settings.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
//nrf_ble
#include "ble.h"
#include "ble_hci.h"
#include "ble_gap.h"
#include "ble_nus.h"
#include "ble_nus_c.h"
#include "ble_advdata.h"
#include "peer_manager.h"
#include "ble_srv_common.h"
#include "ble_advertising.h"
#include "ble_conn_state.h"
#include "ble_conn_params.h"
#include "ble_db_discovery.h"
#include "peer_manager_handler.h"
//nrf_lib
#include "fds.h"
#include "SEGGER_RTT.h"
//nrf_app
#include "app_timer.h"
#include "app_uart.h"
#include "app_error.h"

//cpt
#include "elog.h"
#include "sf_port.h"
//tuya_ble_sdk
#include "tuya_ble_log.h"
#include "tuya_ble_api.h"
#include "tuya_ble_app_demo.h"




/*********************************************************************
 * CONSTANT
 */
/* suble_common
 **************************************************/
#define SUBLE_DEBUG_EN 1

#if (SUBLE_DEBUG_EN)
    #define SUBLE_PRINTF(...)                   TUYA_APP_LOG_INFO(__VA_ARGS__)
    #define SUBLE_HEXDUMP(...)                  TUYA_APP_LOG_HEXDUMP_INFO(__VA_ARGS__)
#else
    #define SUBLE_PRINTF(...)
    #define SUBLE_HEXDUMP(...)
#endif

typedef enum {
    SUBLE_SUCCESS = 0x00,
    SUBLE_ERROR_COMMON,
} suble_status_t;

/* suble_adv_scan
 **************************************************/
#define  SUBLE_ADV_DATA_MAX_LEN                (31)
#define  SUBLE_ADV_INTERVAL_MIN                (TUYA_ADV_INTERVAL)
#define  SUBLE_ADV_INTERVAL_MAX                (TUYA_ADV_INTERVAL)

typedef enum {
    SUBLE_SCAN_EVT_ADV_REPORT = 0x00,
    SUBLE_SCAN_EVT_SCAN_TIMEOUT,
} suble_scan_result_t;

/* suble_gap
 **************************************************/
#define SUBLE_BLE_CONN_CFG_TAG                 (1)

#define SUBLE_BT_MAC_LEN                       (BLE_GAP_ADDR_LEN)
#define SUBLE_BT_MAC_STR_LEN                   (SUBLE_BT_MAC_LEN*2)

#define SUBLE_CONN_INTERVAL_MIN                (TUYA_CONN_INTERVAL_MIN) //最小可接受的连接间隔
#define SUBLE_CONN_INTERVAL_MAX                (TUYA_CONN_INTERVAL_MAX)
#define SUBLE_SLAVE_LATENCY                    (0)
#define SUBLE_CONN_SUP_TIMEOUT                 (5000)

typedef enum {
    SUBLE_GAP_EVT_CONNECTED = 0x00,
    SUBLE_GAP_EVT_DISCONNECTED,
    SUBLE_GAP_EVT_CONNECT_TIMEOUT,
} suble_gap_result_t;

/* suble_svc
 **************************************************/
typedef enum {
    SUBLE_SVC_C_EVT_DISCOVERY_COMPLETE = 0x00,
    SUBLE_SVC_C_EVT_RECEIVE_DATA_FROM_SLAVE,
} suble_svc_result_t;

/* suble_gpio
 **************************************************/
#define SUBLE_OUT_PIN_0                        0x12
#define SUBLE_OUT_PIN_1                        0x13
#define SUBLE_IN_PIN_0                         0x10
#define SUBLE_IN_PIN_1                         0x11

enum
{
    SUBLE_LEVEL_INVALID = 0xFF,
    SUBLE_LEVEL_LOW  = 0,
    SUBLE_LEVEL_HIGH = 1,
};

/* suble_uart
 **************************************************/

/* suble_flash
 **************************************************/
#define SUBLE_FLASH_START_ADDR                 0x46000
#define SUBLE_FLASH_END_ADDR                   0x78000

//ota
#define SUBLE_FLASH_OTA_START_ADDR             SUBLE_FLASH_START_ADDR
#define SUBLE_FLASH_OTA_END_ADDR               0x66000
//mac
#define SUBLE_FLASH_BLE_MAC_ADDR               0x77000

/* suble_timer
 **************************************************/
#define SUBLE_TIMER_MAX_NUM                    20

typedef enum {
    SUBLE_TIMER_SINGLE_SHOT,
    SUBLE_TIMER_REPEATED,
} suble_timer_mode_t;

/* suble_util
 **************************************************/

/* suble_test
 **************************************************/




/*********************************************************************
 * STRUCT
 */
#pragma pack(1)
/* suble_common
 **************************************************/

/* suble_adv_scan
 **************************************************/
typedef struct
{
    double  adv_interval_min; //ms
    double  adv_interval_max; //ms
    uint8_t adv_type;
    uint8_t adv_power;
    uint8_t adv_channal_map;
} adv_param_t;

typedef struct
{
    uint16_t interval;
    uint16_t window;
    uint16_t timeout;
    uint8_t  isActive;
    uint8_t  filter_policy;
    uint8_t  isFiltDuplicated;
    int8_t   minRssi;
    uint8_t  maxRes;
} scan_param_t;

typedef struct
{
    uint32_t len;
    uint8_t  value[SUBLE_ADV_DATA_MAX_LEN];
} adv_data_t;

typedef void (*suble_scan_result_handler_t)(uint32_t evt, uint8_t* buf, uint32_t size);

/* suble_gap
 **************************************************/
typedef struct
{
    uint16_t conn_handle;
    uint8_t  role;
    ble_gap_addr_t mac;
} conn_info_t;

typedef void (*suble_connect_result_handler_t)(uint32_t evt, uint8_t* buf, uint32_t size);

/* suble_svc
 **************************************************/
typedef void (*suble_svc_result_handler_t)(uint32_t evt, uint8_t* buf, uint32_t size);

/* suble_gpio
 **************************************************/
typedef struct
{
    uint8_t pin;
    uint8_t level;
} suble_out_pin_t;

/* suble_uart
 **************************************************/

/* suble_flash
 **************************************************/

/* suble_timer
 **************************************************/
typedef void (*suble_timer_handler_t)(void*);

/* suble_util
 **************************************************/

/* suble_test
 **************************************************/

#pragma pack()




/*********************************************************************
 * EXTERNAL VARIABLE
 */
/* suble_common
 **************************************************/
extern volatile bool   g_system_sleep;

/* suble_adv_scan
 **************************************************/
extern adv_data_t      g_adv_data;
extern adv_data_t      g_scan_rsp;
extern adv_param_t     g_adv_param;
extern bool            g_isAdvtising;

extern scan_param_t    g_scan_param;
extern bool            g_is_scanning;

/* suble_gap
 **************************************************/
extern conn_info_t     g_conn_info[];

/* suble_svc
 **************************************************/

/* suble_gpio
 **************************************************/
extern volatile bool g_open_with_master_finish;

/* suble_uart
 **************************************************/

/* suble_flash
 **************************************************/

/* suble_timer
 **************************************************/

/* suble_util
 **************************************************/

/* suble_test
 **************************************************/




/*********************************************************************
 * EXTERNAL FUNCTION
 */
/* suble_common
 **************************************************/
void suble_init_func(uint8_t location);
void suble_mainloop(void);
void suble_system_reset(void);
void suble_enter_critical(void);
void suble_exit_critical(void);

void suble_log_init(void);
void suble_log_hexdump(const char *name, uint8_t *buf, uint16_t size);
void suble_log_hexdump_for_tuya_ble_sdk(const char *name, uint8_t width, uint8_t *buf, uint16_t size);
void suble_log_hexdump_empty(const char *name, uint8_t *buf, uint16_t size);

void suble_wdt_init(void);
void suble_wdt_feed(void);

/* suble_adv_scan
 **************************************************/
void suble_adv_init(void);
void suble_adv_start(void);
void suble_adv_stop(void);
void suble_adv_state_update(bool adv_state);
void suble_adv_update_advDataAndScanRsp(void);
void suble_adv_param_set(void);

void suble_scan_init(void);
void suble_scan_start(suble_scan_result_handler_t handler);
void suble_scan_stop(void);
void suble_scan_param_set(void);

/* suble_gap
 **************************************************/
void suble_ble_stack_init(void);
void suble_gap_params_init(void);
void suble_gatt_init(void);
uint32_t suble_gap_connect(ble_gap_addr_t* pMac, suble_connect_result_handler_t handler);
uint32_t suble_gap_reconnect(ble_gap_addr_t* pMac);
void suble_gap_disconnect(uint16_t conn_handle, uint8_t hci_status_code);
void suble_gap_disconnect_for_tuya_ble_sdk(void);
uint16_t suble_get_m_ble_max_data_len(void);
void suble_gap_conn_param_update(uint16_t conn_handle, uint16_t cMin, uint16_t cMax, uint16_t latency, uint16_t timeout);
void suble_gap_init_bt_mac(void);
void suble_gap_set_bt_mac(uint8_t *pMac);
void suble_gap_get_bt_mac(uint8_t *pMac, uint32_t size);

void suble_gap_conn_handler(void);
void suble_gap_disconn_handler(void);

/* suble_svc
 **************************************************/
void suble_svc_init(void);
void suble_svc_receive_data(ble_nus_evt_t * p_evt);
void suble_svc_send_data(uint8_t* buf, uint32_t size);
void suble_svc_notify(uint8_t* buf, uint32_t size);
void suble_svc_notify_handler(void);

void suble_svc_c_init(void);
void suble_svc_c_handle_assign(uint16_t conn_handle);
void suble_db_discovery_init(void);
void suble_db_discovery_start(suble_svc_result_handler_t handler);
uint32_t suble_svc_c_send_data(uint16_t connHandle, uint8_t* pBuf, uint16_t len);

/* suble_gpio
 **************************************************/
void suble_gpio_init(void);
void suble_gpio_open_with_common_pwd(uint8_t hardid, uint16_t slaveid);
void suble_gpio_open_with_tmp_pwd(uint8_t hardid, uint16_t slaveid);
void open_with_master_success_handler(void* buf, uint32_t size);

/* suble_uart
 **************************************************/
void suble_uart1_init(void);
void suble_uart2_init(void);
void suble_uart1_send(const uint8_t* buf, uint32_t size);
void suble_uart2_send(const uint8_t* buf, uint32_t size);

/* suble_flash
 **************************************************/
void suble_flash_init(void);
void suble_flash_read(uint32_t addr, uint8_t *buf, uint32_t size);
void suble_flash_write(uint32_t addr, uint8_t *buf, uint32_t size);
void suble_flash_erase(uint32_t addr, uint32_t num);

/* suble_timer
 **************************************************/
uint32_t suble_timer_create(void** p_timer_id, uint32_t timeout_value_ms, suble_timer_mode_t mode, suble_timer_handler_t timeout_handler);
uint32_t suble_timer_delete(void* timer_id);
uint32_t suble_timer_start(void* timer_id);
uint32_t suble_timer_stop(void* timer_id);
uint32_t suble_timer_restart(void* timer_id, uint32_t timeout_value_ms);

void suble_local_timer_start(void);
void suble_update_timestamp(uint32_t app_timestamp);
uint32_t suble_get_app_timestamp_when_update(void);
uint32_t suble_get_local_timestamp(void);
uint32_t suble_get_timestamp(void);
uint32_t suble_get_old_timestamp(uint32_t old_local_timestamp);
uint32_t suble_get_rtc2_counter(void);

void suble_delay_ms(uint32_t ms);
void suble_delay_us(uint32_t us);

/* suble_util
 **************************************************/
uint8_t  suble_util_check_sum8(uint8_t *buf, uint32_t size);
uint16_t suble_util_check_sum16(uint8_t *buf, uint32_t size);
uint16_t suble_util_crc16(uint8_t* buf, uint32_t size, uint16_t* p_crc);
uint32_t suble_util_crc32(uint8_t* buf, uint32_t size, uint32_t* p_crc);
void     suble_util_reverse_byte(void* buf, uint32_t size);
uint32_t suble_util_numarray2int(uint8_t *num_array, uint32_t start_idx, uint32_t size);

uint8_t  suble_util_str_hexchar2int(uint8_t hexchar);
uint8_t  suble_util_str_int2hexchar(uint8_t int_num);
uint32_t suble_util_str_hexstr2int(uint8_t* hexstr, uint32_t size, int* sum);
uint32_t suble_util_str_intstr2int(uint8_t* intstr, uint32_t size, int* sum);
uint32_t suble_util_str_hexstr2hexarray(uint8_t* hexstr, uint32_t size, uint8_t* hexarray);
uint32_t suble_util_str_hexarray2hexstr(uint8_t* hexarray, uint32_t size, uint8_t* hexstr);

/* suble_test
 **************************************************/
void suble_test_func(void);


#ifdef __cplusplus
}
#endif

#endif //__SUBLE_COMMON_H__
