#include "suble_common.h"
#include "app_port.h"




/*********************************************************************
 * LOCAL CONSTANT
 */

/*********************************************************************
 * LOCAL STRUCT
 */
#pragma pack(1)
typedef struct
{
    uint8_t  open_meth;
    uint8_t  hardid;
    uint8_t  open_meth_info_len;
    uint8_t  open_meth_info[64];
} open_record_info_t;
#pragma pack()

/*********************************************************************
 * LOCAL VARIABLE
 */
static open_record_info_t s_open_record_info;
static volatile bool s_master_scan_is_running = false;

/*********************************************************************
 * VARIABLE
 */
volatile bool g_open_with_master_finish = false;

/*********************************************************************
 * LOCAL FUNCTION
 */
static void bsp_event_handler(bsp_event_t event);




/*********************************************************
FN: 
*/
void suble_gpio_init(void)
{
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);
    err_code = bsp_init(BSP_INIT_BUTTONS, bsp_event_handler);
    APP_ERROR_CHECK(err_code);
}

/*********************************************************
FN: 
*/
void open_with_master_success_handler(void* buf, uint32_t size)
{
    open_with_master_record_report_info_t* info = (void*)buf;
    lock_open_record_report(info->timestamp, s_open_record_info.open_meth, s_open_record_info.hardid, info->slaveid);
}

/*********************************************************
FN: 
*/
void master_bonding_slave_handler(uint32_t evt, uint8_t* buf, uint32_t size)
{
    switch (evt)
    {
        case TUYA_BLE_MASTER_EVT_TIMEOUT: {
            SUBLE_PRINTF("TUYA_BLE_MASTER_EVT_TIMEOUT");
            s_master_scan_is_running = false;
        } break;
        
        case TUYA_BLE_MASTER_EVT_SLAVEID_INVALID: {
            SUBLE_PRINTF("TUYA_BLE_MASTER_EVT_SLAVEID_INVALID");
            s_master_scan_is_running = false;
        } break;
        
        case TUYA_BLE_MASTER_EVT_SCAN_TIMEOUT: {
            SUBLE_PRINTF("TUYA_BLE_MASTER_EVT_SCAN_TIMEOUT");
            s_master_scan_is_running = false;
        } break;
        
        case TUYA_BLE_MASTER_EVT_CONNECT_TIMEOUT: {
            SUBLE_PRINTF("TUYA_BLE_MASTER_EVT_CONNECT_TIMEOUT");
            s_master_scan_is_running = false;
        } break;
        
        case TUYA_BLE_MASTER_EVT_BONDING: {
            //开门
            tuya_ble_master_open_with_master(TUYA_BLE_MASTER_OPERATION_OPEN, 
                                                s_open_record_info.open_meth, 
                                                s_open_record_info.open_meth_info, 
                                                s_open_record_info.open_meth_info_len);
        } break;
        
        case TUYA_BLE_MASTER_EVT_OPEN_WITH_MASTER_SUCCESS: {
            //实际上会断开连接，此处上报只为存储开锁记录，待手机连接后再上报
            if(s_open_record_info.open_meth == OR_LOG_OPEN_WITH_KEY) {
                TUYA_APP_LOG_INFO("open with key success");
            }
            else if(s_open_record_info.open_meth == OR_LOG_OPEN_WITH_PW) {
                TUYA_APP_LOG_INFO("open with common password success");
            }
            else if(s_open_record_info.open_meth == OR_LOG_OPEN_WITH_TMP_PWD) {
                TUYA_APP_LOG_INFO("open with temp password success");
            }
            
            g_open_with_master_finish = true;
            
            tuya_ble_app_evt_send_with_data(APP_EVT_OPEN_WITH_MASTER_RECORD, buf, size);
            
            suble_gap_disconnect(g_conn_info[1].conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
        } break;
        
        case TUYA_BLE_MASTER_EVT_OPEN_WITH_MASTER_FAILURE: {
            suble_gap_disconnect(g_conn_info[1].conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            
            g_open_with_master_finish = true;
        } break;
        
        case TUYA_BLE_MASTER_EVT_DISCONNECT: {
            SUBLE_PRINTF("TUYA_BLE_MASTER_EVT_DISCONNECT");
            s_master_scan_is_running = false;
        } break;
        
        default: {
        } break;
    }
}

/*********************************************************
FN: 
*/
static void bsp_event_handler(bsp_event_t event)
{
    SUBLE_PRINTF("button: %d", event-BSP_EVENT_KEY_0);
    switch (event)
    {
        case BSP_EVENT_KEY_0: { //14
            s_open_record_info.open_meth = OR_LOG_OPEN_WITH_KEY;
            s_open_record_info.hardid = 0x00;
            s_open_record_info.open_meth_info_len = 6;
            s_open_record_info.open_meth_info[0] = 0;
            s_open_record_info.open_meth_info[1] = 0;
            s_open_record_info.open_meth_info[2] = 0;
            s_open_record_info.open_meth_info[3] = 0;
            s_open_record_info.open_meth_info[4] = 0;
            s_open_record_info.open_meth_info[5] = 0;
    
            if(!s_master_scan_is_running) {
                s_master_scan_is_running = true;
                tuya_ble_master_scan_start(-1, master_bonding_slave_handler);
            }
            else {
                SUBLE_PRINTF("__________________s_master_scan_is_running");
            }
        } break;

        case BSP_EVENT_KEY_1: {
        } break;

        case BSP_EVENT_KEY_2: {
        } break;

        case BSP_EVENT_KEY_3: {
        } break;
        
        default: {
        } break;
    }
}

/*********************************************************
FN: 
*/
void suble_gpio_open_with_common_pwd(uint8_t hardid, uint16_t slaveid)
{
    s_open_record_info.open_meth = OR_LOG_OPEN_WITH_PW;
    s_open_record_info.hardid = hardid;
    s_open_record_info.open_meth_info_len = 6;
    s_open_record_info.open_meth_info[0] = 0;
    s_open_record_info.open_meth_info[1] = 0;
    s_open_record_info.open_meth_info[2] = 0;
    s_open_record_info.open_meth_info[3] = hardid;
    s_open_record_info.open_meth_info[4] = slaveid>>8;
    s_open_record_info.open_meth_info[5] = slaveid&0xFF;
    
    if(!s_master_scan_is_running) {
        s_master_scan_is_running = true;
        tuya_ble_master_scan_start(slaveid, master_bonding_slave_handler);
    }
    else {
        SUBLE_PRINTF("__________________s_master_scan_is_running");
    }
}

/*********************************************************
FN: 
*/
void suble_gpio_open_with_tmp_pwd(uint8_t hardid, uint16_t slaveid)
{
    s_open_record_info.open_meth = OR_LOG_OPEN_WITH_TMP_PWD;
    s_open_record_info.hardid = hardid;
    s_open_record_info.open_meth_info_len = 6;
    s_open_record_info.open_meth_info[0] = 0;
    s_open_record_info.open_meth_info[1] = 0;
    s_open_record_info.open_meth_info[2] = 0;
    s_open_record_info.open_meth_info[3] = hardid;
    s_open_record_info.open_meth_info[4] = slaveid>>8;
    s_open_record_info.open_meth_info[5] = slaveid&0xFF;
    
    if(!s_master_scan_is_running) {
        s_master_scan_is_running = true;
        tuya_ble_master_scan_start(slaveid, master_bonding_slave_handler);
    }
    else {
        SUBLE_PRINTF("__________________s_master_scan_is_running");
    }
}
















