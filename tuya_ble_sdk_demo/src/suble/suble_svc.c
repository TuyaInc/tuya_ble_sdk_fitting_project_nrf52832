#include "suble_common.h"
#include "app_port.h"




/*********************************************************************
 * LOCAL CONSTANT
 */
#define NOTIFY_QUEUE_MAX_NUM  64

/*********************************************************************
 * LOCAL STRUCT
 */
typedef struct
{
    uint32_t len;
    uint8_t  value[20];
} notify_data_t;

/*********************************************************************
 * LOCAL VARIABLE
 */
BLE_NUS_DEF(m_nus, NRF_SDH_BLE_TOTAL_LINK_COUNT);

static notify_data_t notify_data[NOTIFY_QUEUE_MAX_NUM];
static uint32_t s_start_idx = 0;
static uint32_t s_end_idx = 0;
static bool s_send_complete_flag = true;

/*********************************************************************
 * VARIABLE
 */

/*********************************************************************
 * LOCAL FUNCTION
 */
static uint32_t suble_next_idx(uint32_t index);
static uint32_t suble_queue_full(void);
static uint32_t suble_queue_empty(void);




/*********************************************************  svc  *********************************************************/

/*********************************************************
FN: 
*/
void suble_svc_init(void)
{
    // Initialize NUS.
    ble_nus_init_t     nus_init;
    memset(&nus_init, 0, sizeof(nus_init));
    nus_init.data_handler = suble_svc_receive_data;
    
    ret_code_t err_code;
    err_code = ble_nus_init(&m_nus, &nus_init);
    APP_ERROR_CHECK(err_code);
}

/*********************************************************
FN: 接收主机的数据
*/
void suble_svc_receive_data(ble_nus_evt_t * p_evt)
{
    if(p_evt->type == BLE_NUS_EVT_RX_DATA) {
#ifdef TUYA_BLE_SDK_IN_SUBLE
        tuya_ble_gatt_receive_data((void*)p_evt->params.rx_data.p_data, p_evt->params.rx_data.length);
#endif
//        SUBLE_HEXDUMP("Svc rx", (void*)p_evt->params.rx_data.p_data, p_evt->params.rx_data.length);
    }
}

/*********************************************************
FN: 作为从机向主机发送数据
*/
void suble_svc_send_data(uint8_t* buf, uint32_t size)
{
    ret_code_t err_code;
    uint32_t tmp_count = 100;
    do
    {
        err_code = ble_nus_data_send(&m_nus, buf, (void*)&size, g_conn_info[0].conn_handle);
        if ((err_code != NRF_ERROR_INVALID_STATE) &&
            (err_code != NRF_ERROR_RESOURCES) &&
            (err_code != NRF_ERROR_NOT_FOUND) &&
            (err_code != NRF_SUCCESS))
        {
            SUBLE_PRINTF("Error: ble nus send mtu error, err_code = %d\n", err_code);
            APP_ERROR_CHECK(err_code);
        }
        suble_delay_ms(5);
        tmp_count--;
    } while (err_code == NRF_ERROR_RESOURCES && tmp_count > 0);
}

/*********************************************************
FN: 发送完成
*/
void suble_svc_send_data_complete(void)
{
    s_send_complete_flag = true;
    s_start_idx = suble_next_idx(s_start_idx);
}




/*********************************************************
FN: 
*/
static uint32_t suble_next_idx(uint32_t index)
{
    return (index < NOTIFY_QUEUE_MAX_NUM-1) ? (index + 1) : 0;
}

static uint32_t suble_queue_full(void)
{
    uint32_t tmp = s_start_idx;
    return suble_next_idx(s_end_idx) == tmp;
}

static uint32_t suble_queue_empty(void)
{
    uint32_t tmp = s_start_idx;
    return s_end_idx == tmp;
}

/*********************************************************
FN: 
*/
void suble_svc_notify(uint8_t* buf, uint32_t size)
{
    if(!suble_queue_full()) {
        notify_data[s_end_idx].len = size;
        memcpy(notify_data[s_end_idx].value, buf, size);

        s_end_idx = suble_next_idx(s_end_idx);
    }
    else {
        SUBLE_PRINTF("suble_svc_notify: suble_queue is full");
    }
}

/*********************************************************
FN: 
*/
void suble_svc_notify_handler(void)
{
    if(!suble_queue_empty()) {
        if(s_send_complete_flag) {
            s_send_complete_flag = false;
            suble_svc_send_data(notify_data[s_start_idx].value, notify_data[s_start_idx].len);
        }
    }
}




/*********************************************************  svc_c  *********************************************************/

/*********************************************************************
 * LOCAL CONSTANT
 */

/*********************************************************************
 * LOCAL STRUCT
 */

/*********************************************************************
 * LOCAL VARIABLE
 */
BLE_NUS_C_DEF(m_ble_nus_c);
BLE_DB_DISCOVERY_DEF(m_db_disc);
static suble_svc_result_handler_t suble_svc_result_handler;

/*********************************************************************
 * VARIABLE
 */
void* p_m_ble_nus_c = &m_ble_nus_c;

/*********************************************************************
 * LOCAL FUNCTION
 */




/*********************************************************
FN: 
*/
static void ble_nus_c_evt_handler(ble_nus_c_t * p_ble_nus_c, ble_nus_c_evt_t const * p_ble_nus_evt)
{
    switch (p_ble_nus_evt->evt_type)
    {
        case BLE_NUS_C_EVT_DISCOVERY_COMPLETE: {
            SUBLE_PRINTF("BLE_NUS_C_EVT_DISCOVERY_COMPLETE");
            
            ret_code_t err_code;
            err_code = ble_nus_c_handles_assign(&m_ble_nus_c, p_ble_nus_evt->conn_handle, &p_ble_nus_evt->handles);
            APP_ERROR_CHECK(err_code);

            err_code = ble_nus_c_tx_notif_enable(&m_ble_nus_c);
            APP_ERROR_CHECK(err_code);
            
            suble_svc_result_handler(SUBLE_SVC_C_EVT_DISCOVERY_COMPLETE, NULL, 0);
        } break;

        //作为主机收到从机数据（Notify）
        case BLE_NUS_C_EVT_NUS_TX_EVT: {
//            SUBLE_HEXDUMP("BLE_NUS_C_EVT_NUS_TX_EVT", p_ble_nus_evt->p_data, p_ble_nus_evt->data_len);
            suble_svc_result_handler(SUBLE_SVC_C_EVT_RECEIVE_DATA_FROM_SLAVE, p_ble_nus_evt->p_data, p_ble_nus_evt->data_len);
        } break;

        default: {
        } break;
    }
}

/*********************************************************
FN: 
*/
static void db_disc_handler(ble_db_discovery_evt_t * p_evt)
{
    ble_nus_c_on_db_disc_evt(&m_ble_nus_c, p_evt);
}

/*********************************************************
FN: 
*/
void suble_svc_c_init(void)
{
    ret_code_t       err_code;
    ble_nus_c_init_t init;

    init.evt_handler = ble_nus_c_evt_handler;

    err_code = ble_nus_c_init(&m_ble_nus_c, &init);
    APP_ERROR_CHECK(err_code);
}

/*********************************************************
FN: 
*/
void suble_svc_c_handle_assign(uint16_t conn_handle)
{
    ret_code_t err_code;
    err_code = ble_nus_c_handles_assign(&m_ble_nus_c, conn_handle, NULL);
    APP_ERROR_CHECK(err_code);
}

/*********************************************************
FN: 
*/
void suble_db_discovery_init(void)
{
    ret_code_t err_code;
    err_code = ble_db_discovery_init(db_disc_handler);
    APP_ERROR_CHECK(err_code);
}

/*********************************************************
FN: 
*/
void suble_db_discovery_start(suble_svc_result_handler_t handler)
{
    //发现服务
    ret_code_t err_code = ble_db_discovery_start(&m_db_disc, g_conn_info[1].conn_handle);
    if (err_code != NRF_ERROR_BUSY) {
        APP_ERROR_CHECK(err_code);
    }
    if(err_code == NRF_SUCCESS) {
        suble_svc_result_handler = handler;
    }
}

/*********************************************************
FN: 
*/
uint32_t suble_svc_c_send_data(uint16_t connHandle, uint8_t* pBuf, uint16_t len)
{
    uint32_t ret_val;
    do
    {
        ret_val = ble_nus_c_string_send(&m_ble_nus_c, pBuf, len);
        if ( (ret_val != NRF_ERROR_INVALID_STATE) && (ret_val != NRF_ERROR_RESOURCES) )
        {
            APP_ERROR_CHECK(ret_val);
        }
    } while (ret_val == NRF_ERROR_RESOURCES);
    return ret_val;
}






















