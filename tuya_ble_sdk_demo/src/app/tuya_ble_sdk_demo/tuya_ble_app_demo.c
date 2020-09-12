#include "tuya_ble_app_demo.h"
#include "tuya_ble_stdlib.h"
#include "tuya_ble_type.h"
#include "tuya_ble_heap.h"
#include "tuya_ble_mem.h"
#include "tuya_ble_api.h"
#include "tuya_ble_port.h"
#include "tuya_ble_main.h"
#include "tuya_ble_secure.h"
#include "tuya_ble_data_handler.h"
#include "tuya_ble_storage.h"
#include "tuya_ble_sdk_version.h"
#include "tuya_ble_utils.h"
#include "tuya_ble_event.h"
#include "tuya_ble_log.h"
#include "app_port.h"




/*********************************************************************
 * LOCAL CONSTANT
 */

/*********************************************************************
 * LOCAL STRUCT
 */

/*********************************************************************
 * LOCAL VARIABLE
 */
static const char auth_key_test[]  = TUYA_DEVICE_AUTH_KEY;
static const char device_id_test[] = TUYA_DEVICE_DID; //DC234D08C8EF

//tuya_ble_sdk init param
static tuya_ble_device_param_t tuya_ble_device_param = 
{
    .device_id_len    = DEVICE_ID_LEN,
    .p_type           = TUYA_BLE_PRODUCT_ID_TYPE_PID,
    .product_id_len   = 8,
    .bound_flag       = 0,
    .firmware_version = TUYA_DEVICE_FVER_NUM,
    .hardware_version = TUYA_DEVICE_HVER_NUM,
};

tuya_ble_app_master_result_handler_t tuya_ble_app_master_result_handler;

/*********************************************************************
 * VARIABLE
 */
demo_dp_t g_demo_cmd;
demo_dp_t g_demo_rsp;

/*********************************************************************
 * LOCAL FUNCTION
 */
static void tuya_ble_app_data_process(int32_t evt_id,void *data);
static void tuya_ble_sdk_callback(tuya_ble_cb_evt_param_t* event);




/*********************************************************
FN: 
*/
void tuya_ble_app_init(void)
{
    lock_common_init();
    
    //tuya_ble_sdk init
    memcpy(tuya_ble_device_param.product_id, TUYA_DEVICE_PID, 8);
    memcpy(tuya_ble_device_param.device_id,  device_id_test,  DEVICE_ID_LEN);
    memcpy(tuya_ble_device_param.auth_key,   auth_key_test,   AUTH_KEY_LEN);
    tuya_ble_sdk_init(&tuya_ble_device_param);
    
    //register tuya_ble_sdk callback
    tuya_ble_callback_queue_register(tuya_ble_sdk_callback);
    
    TUYA_APP_LOG_HEXDUMP_INFO("auth key", tuya_ble_current_para.auth_settings.auth_key, AUTH_KEY_LEN);
    TUYA_APP_LOG_HEXDUMP_INFO("device id", tuya_ble_current_para.auth_settings.device_id, DEVICE_ID_LEN);
}

/*********************************************************
FN: 
*/
static void tuya_ble_sdk_callback(tuya_ble_cb_evt_param_t* event)
{
//    TUYA_APP_LOG_INFO("callback: %02x", event->evt);
    switch(event->evt)
    {
        //connect status
        case TUYA_BLE_CB_EVT_CONNECTE_STATUS: {
            if(event->connect_status == BONDING_CONN) {
                TUYA_APP_LOG_INFO("bonding and connecting");
                
                app_active_report_stop(ACTICE_REPORT_STOP_STATE_BONDING);
                
                lock_timer_start(LOCK_TIMER_CONN_PARAM_UPDATE);
                lock_timer_start(LOCK_TIMER_COMMUNICATION_MONITOR);
            }
        } break;
        
        //dp parser
        case TUYA_BLE_CB_EVT_DP_WRITE: {
            if(event->dp_write_data.p_data[2] == (event->dp_write_data.data_len - 3))
            {
                lock_dp_parser_handler(event->dp_write_data.p_data);
            }
            lock_timer_stop(LOCK_TIMER_COMMUNICATION_MONITOR);
            lock_timer_start(LOCK_TIMER_COMMUNICATION_MONITOR);
        } break;
        
        //response - dp report
        case TUYA_BLE_CB_EVT_DP_DATA_REPORT_RESPONSE: {
            if(g_sync_new.flag == 1)
            {
                lock_open_meth_sync_new_report(event->dp_response_data.status);
            }
            lock_timer_stop(LOCK_TIMER_COMMUNICATION_MONITOR);
            lock_timer_start(LOCK_TIMER_COMMUNICATION_MONITOR);
        } break;
        
        //response - dp report with timestamp
        case TUYA_BLE_CB_EVT_DP_DATA_WTTH_TIME_REPORT_RESPONSE: {
            if(event->dp_with_time_response_data.status != 0xFF)
            {
                lock_offline_evt_report(event->dp_with_time_response_data.status);
            }
            lock_timer_stop(LOCK_TIMER_COMMUNICATION_MONITOR);
            lock_timer_start(LOCK_TIMER_COMMUNICATION_MONITOR);
        } break;
        
        //timestamp synch
        case TUYA_BLE_CB_EVT_TIME_STAMP: {
            uint32_t timestamp_s = 0;
            uint32_t timestamp_ms = 0;
            uint64_t timestamp = 0;
            suble_util_str_intstr2int(event->timestamp_data.timestamp_string, 10, (void*)&timestamp_s);
            suble_util_str_intstr2int(event->timestamp_data.timestamp_string+10, 3, (void*)&timestamp_ms);
            timestamp = timestamp_s*1000 + timestamp_ms;
            
            TUYA_APP_LOG_INFO("TUYA_BLE_CB_EVT_TIME_STAMP - time_zone: %d", event->timestamp_data.time_zone);
            TUYA_APP_LOG_INFO("TUYA_BLE_CB_EVT_TIME_STAMP - timestamp: %d", timestamp_s);
            app_port_update_timestamp(timestamp_s);
            lock_timer_start(LOCK_TIMER_BONDING_CONN);
        } break;
        
        //unbond
        case TUYA_BLE_CB_EVT_UNBOUND:
        //unexpected unbond /restore factory setting
        case TUYA_BLE_CB_EVT_ANOMALY_UNBOUND: {
            lock_factory_handler();
            lock_timer_start(LOCK_TIMER_RESET_WITH_DISCONN);
        } break;
        
        //restore factory setting
        case TUYA_BLE_CB_EVT_DEVICE_RESET: {
            lock_factory_handler();
            lock_timer_start(LOCK_TIMER_RESET_WITH_DISCONN);
        } break;
        
        case TUYA_BLE_CB_EVT_DP_QUERY: {
//            TUYA_APP_LOG_HEXDUMP_DEBUG("TUYA_BLE_CB_EVT_DP_QUERY", event->dp_query_data.p_data, event->dp_query_data.data_len);
        } break;
        
        //ota
        case TUYA_BLE_CB_EVT_OTA_DATA: {
            app_ota_handler(&event->ota_data);
            lock_timer_stop(LOCK_TIMER_COMMUNICATION_MONITOR);
            lock_timer_start(LOCK_TIMER_COMMUNICATION_MONITOR);
        } break;
        
        //data passthrough
        case TUYA_BLE_CB_EVT_DATA_PASSTHROUGH: {
            tuya_ble_master_data_passthrough_with_phone((void*)event->ble_passthrough_data.p_data, event->ble_passthrough_data.data_len);
        } break;
        
        //data passthrough
        case TUYA_BLE_CB_EVT_UPDATE_LOGIN_KEY_VID: {
//            TUYA_APP_LOG_HEXDUMP_DEBUG("TUYA_BLE_CB_EVT_UPDATE_LOGIN_KEY_VID", event->dp_query_data.p_data, event->dp_query_data.data_len);
        } break;
        
        default: {
            TUYA_APP_LOG_INFO("tuya_ble_sdk_callback unknown event type 0x%04x", event->evt);
        } break;
    }
}

/*********************************************************
FN: 
*/
static void tuya_ble_app_data_process(int32_t evt_id, void *data)
{
    custom_evt_data_t* custom_data = data;
    
    switch (evt_id)
    {
        case APP_EVT_START_REG_CARD: {
            lock_hard_doorcard_start_reg(custom_data->value, custom_data->len);
        } break;
        
        case APP_EVT_CANCEL_REG_CARD: {
            lock_hard_doorcard_cancel_reg();
        } break;
        
        case APP_EVT_START_REG_FINGER: {
            lock_hard_finger_start_reg(custom_data->value, custom_data->len);
        } break;
        
        case APP_EVT_CANCEL_REG_FINGER: {
            lock_hard_finger_cancel_reg();
        } break;
        
        case APP_EVT_START_REG_FACE: {
            lock_hard_face_start_reg(custom_data->value, custom_data->len);
        } break;
        
        case APP_EVT_CANCEL_REG_FACE: {
            lock_hard_face_cancel_reg();
        } break;
        
        case APP_EVT_CONNECTED: {
            tuya_ble_connected_handler();
        } break;
        
        case APP_EVT_DISCONNECTED: {
            tuya_ble_disconnected_handler();
            lock_timer_stop(LOCK_TIMER_COMMUNICATION_MONITOR);
            
            app_ota_disconn_handler();
            app_active_report_finished_and_disconnect_handler();
        } break;
        
        case APP_EVT_MASTER_SAVE_SLAVE_MAC: {
            tuya_ble_app_master_result_handler(TUYA_BLE_APP_EVT_MASTER_SAVE_SLAVE_MAC, custom_data->value, custom_data->len);
        } break;
        
        case APP_EVT_OPEN_WITH_MASTER_RECORD: {
            open_with_master_success_handler(custom_data->value, custom_data->len);
        } break;
        
        case APP_EVT_TIMER_0: {
            conn_param_update_outtime_cb_handler();
        } break;
        
        case APP_EVT_TIMER_1: {
            delay_report_outtime_cb_handler();
        } break;
        
        case APP_EVT_TIMER_2: {
            bonding_conn_outtime_cb_handler();
        } break;
        
        case APP_EVT_TIMER_3: {
            reset_with_disconn_outtime_cb_handler();
        } break;
        
        case APP_EVT_TIMER_4: {
        } break;
        
        case APP_EVT_TIMER_5: {
            app_test_outtime_cb_handler();
        } break;
        
        case APP_EVT_TIMER_6: {
            app_test_reset_outtime_cb_handler();
        } break;
        
        case APP_EVT_TIMER_7: {
            app_active_report_outtime_cb_handler();
        } break;
        
        case APP_EVT_TIMER_8: {
            reset_with_disconn2_outtime_cb_handler();
        } break;
        
        case APP_EVT_TIMER_9: {
            communication_monitor_outtime_cb_handler();
        } break;
        
        case APP_EVT_TIMER_10: {
        } break;
        
        case APP_EVT_TIMER_11: {
        } break;
        
        case APP_EVT_TIMER_12: {
        } break;
        
        case APP_EVT_TIMER_13: {
        } break;
        
        case APP_EVT_TIMER_14: {
        } break;
        
        case APP_EVT_TIMER_15: {
        } break;
        
        case APP_EVT_TIMER_16: {
        } break;
        
        case APP_EVT_TIMER_17: {
        } break;
        
        case APP_EVT_TIMER_18: {
        } break;
        
        case APP_EVT_TIMER_19: {
        } break;
        
        default: {
        } break;
    }
    
    if(custom_data != NULL)
    {
        tuya_ble_free((void*)custom_data);
    }
}

/*********************************************************
FN: no data
*/
void tuya_ble_app_evt_send(custom_evtid_t evtid)
{
    tuya_ble_custom_evt_t custom_evt;
    
    custom_evt.evt_id = evtid;
    custom_evt.data = NULL;
    custom_evt.custom_event_handler = tuya_ble_app_data_process;
    
    tuya_ble_custom_event_send(custom_evt);
}

/*********************************************************
FN: 
*/
void tuya_ble_app_evt_send_with_data(custom_evtid_t evtid, void* buf, uint32_t size)
{
    custom_evt_data_t* custom_data = tuya_ble_malloc(sizeof(custom_evt_data_t) + size);
    if(custom_data)
    {
        tuya_ble_custom_evt_t custom_evt;
        
        custom_data->len = size;
        memcpy(custom_data->value, buf, size);
        
        custom_evt.evt_id = evtid;
        custom_evt.data = custom_data;
        custom_evt.custom_event_handler = tuya_ble_app_data_process;
        
        tuya_ble_custom_event_send(custom_evt);
    }
    else {
        TUYA_APP_LOG_ERROR("tuya_ble_app_evt_send_with_data: malloc failed");
    }
}


/*********************************************************
FN: 
*/
void tuya_ble_app_master_evt_register(tuya_ble_app_master_result_handler_t handler)
{
    tuya_ble_app_master_result_handler = handler;
}












