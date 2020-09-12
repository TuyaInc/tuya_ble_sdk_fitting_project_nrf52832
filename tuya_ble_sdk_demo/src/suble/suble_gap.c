#include "suble_common.h"




/*********************************************************************
 * LOCAL CONSTANT
 */
#define SUBLE_BLE_OBSERVER_PRIO               3
#define SUBLE_DEVICE_NAME                     "Demo_name"

/*********************************************************************
 * LOCAL STRUCT
 */

/*********************************************************************
 * LOCAL VARIABLE
 */
NRF_BLE_GATT_DEF(m_gatt);

//0-slave, 1-master
conn_info_t g_conn_info[2] = {
    {
        .conn_handle = BLE_CONN_HANDLE_INVALID,
        .role = BLE_GAP_ROLE_INVALID,
    },
    {
        .conn_handle = BLE_CONN_HANDLE_INVALID,
        .role = BLE_GAP_ROLE_INVALID,
    },
};
static volatile uint16_t m_ble_max_data_len = BLE_GATT_ATT_MTU_DEFAULT - 3;

static suble_connect_result_handler_t suble_connect_result_handler;

static uint32_t s_reconnect_count = 0;

/*********************************************************************
 * VARIABLE
 */

/*********************************************************************
 * LOCAL FUNCTION
 */
static void suble_ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context);
static void suble_set_defConn_param(void);
static void gatt_evt_handler(nrf_ble_gatt_t* p_gatt, nrf_ble_gatt_evt_t const* p_evt);




/*********************************************************
FN: Function for handling BLE events.
PM: p_ble_evt   Bluetooth stack event.
    p_context   Unused.
*/
static void suble_ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
{
    ret_code_t err_code = NRF_SUCCESS;

    ble_gap_evt_t const * p_gap_evt = &p_ble_evt->evt.gap_evt;
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED: {
            if(p_gap_evt->params.connected.role == BLE_GAP_ROLE_PERIPH) {
                SUBLE_PRINTF("BLE_GAP_ROLE_PERIPH Connected");
                g_conn_info[0].role = BLE_GAP_ROLE_PERIPH;
                g_conn_info[0].conn_handle = p_gap_evt->conn_handle;
                memcpy(&g_conn_info[0].mac, &p_gap_evt->params.connected.peer_addr, sizeof(ble_gap_addr_t));
                
                suble_gap_conn_handler();
                
                suble_adv_state_update(false);
            }
            else {
                suble_gap_conn_param_update(p_gap_evt->conn_handle, 8, 8, 0, 5000);
                
                SUBLE_PRINTF("BLE_GAP_ROLE_CENTRAL Connected");
                g_conn_info[1].role = BLE_GAP_ROLE_CENTRAL;
                g_conn_info[1].conn_handle = p_gap_evt->conn_handle;
                memcpy(&g_conn_info[1].mac, &p_gap_evt->params.connected.peer_addr, sizeof(ble_gap_addr_t));
                
                suble_svc_c_handle_assign(p_gap_evt->conn_handle);
                
                suble_connect_result_handler(SUBLE_GAP_EVT_CONNECTED, NULL, 0);
                
                g_open_with_master_finish = false;
            }
        } break;

        case BLE_GAP_EVT_DISCONNECTED: {
            if(p_gap_evt->conn_handle == g_conn_info[0].conn_handle) {
                SUBLE_PRINTF("BLE_GAP_ROLE_PERIPH Disconnected: 0x%02x", p_gap_evt->params.disconnected.reason);
                g_conn_info[0].role = BLE_GAP_ROLE_INVALID;
                g_conn_info[0].conn_handle = BLE_CONN_HANDLE_INVALID;
                memset(&g_conn_info[0].mac, 0, sizeof(ble_gap_addr_t));
                
                suble_gap_disconn_handler();
                
                suble_adv_start();
            }
            else {
                SUBLE_PRINTF("BLE_GAP_ROLE_CENTRAL Disconnected: 0x%02x", p_gap_evt->params.disconnected.reason);
                
                ble_gap_addr_t tmpMac = g_conn_info[1].mac;
                
                g_conn_info[1].role = BLE_GAP_ROLE_INVALID;
                g_conn_info[1].conn_handle = BLE_CONN_HANDLE_INVALID;
                memset(&g_conn_info[1].mac, 0, sizeof(ble_gap_addr_t));
                
                suble_connect_result_handler(SUBLE_GAP_EVT_DISCONNECTED, NULL, 0);
                
                if(p_gap_evt->params.disconnected.reason == 0x3E) {
                    s_reconnect_count++;
                    if((s_reconnect_count <= 1) && (g_open_with_master_finish == false)) {
                        suble_gap_reconnect(&tmpMac);
                    }
                }
            }
        } break;

        case BLE_GAP_EVT_TIMEOUT: {
            //连接超时
            if(p_gap_evt->params.timeout.src == BLE_GAP_TIMEOUT_SRC_CONN) {
                suble_connect_result_handler(SUBLE_GAP_EVT_CONNECT_TIMEOUT, NULL, 0);
            }
//            if(p_gap_evt->params.timeout.src == BLE_GAP_TIMEOUT_SRC_SCAN) {
//                SUBLE_PRINTF("BLE_GAP_EVT_TIMEOUT-BLE_GAP_TIMEOUT_SRC_SCAN");
//            }
        } break;

        case BLE_GAP_EVT_PHY_UPDATE_REQUEST: {
            SUBLE_PRINTF("PHY update request.");
            ble_gap_phys_t const phys =
            {
                .rx_phys = BLE_GAP_PHY_AUTO,
                .tx_phys = BLE_GAP_PHY_AUTO,
            };
            err_code = sd_ble_gap_phy_update(p_gap_evt->conn_handle, &phys);
            APP_ERROR_CHECK(err_code);
        } break;

        case BLE_GAP_EVT_SEC_PARAMS_REQUEST: {
            SUBLE_PRINTF("sec param request.");
            // Pairing not supported
            err_code = sd_ble_gap_sec_params_reply(p_gap_evt->conn_handle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL);
            APP_ERROR_CHECK(err_code);
        } break;

        case BLE_GATTS_EVT_SYS_ATTR_MISSING: {
            SUBLE_PRINTF("sttr missing request.");
            // No system attributes have been stored.
            err_code = sd_ble_gatts_sys_attr_set(p_gap_evt->conn_handle, NULL, 0, 0);
            APP_ERROR_CHECK(err_code);
        } break;

        case BLE_GATTC_EVT_TIMEOUT: {
            // Disconnect on GATT Client timeout event.
            SUBLE_PRINTF("GATT Client Timeout.");
            err_code = sd_ble_gap_disconnect(p_gap_evt->conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
        } break;

        case BLE_GATTS_EVT_TIMEOUT: {
            //连接超时
            if(p_gap_evt->params.timeout.src == BLE_GAP_TIMEOUT_SRC_CONN) {
                suble_connect_result_handler(SUBLE_GAP_EVT_CONNECT_TIMEOUT, NULL, 0);
            }
        } break;

        case BLE_GAP_EVT_CONN_PARAM_UPDATE: {
            SUBLE_PRINTF("BLE_GAP_EVT_CONN_PARAM_UPDATE: min-%dms, max-%dms, latency-%d, timeout-%dms", \
                (uint16_t)(p_gap_evt->params.conn_param_update.conn_params.min_conn_interval*1.25),     \
                (uint16_t)(p_gap_evt->params.conn_param_update.conn_params.max_conn_interval*1.25),     \
                (uint16_t)(p_gap_evt->params.conn_param_update.conn_params.slave_latency),              \
                (uint16_t)(p_gap_evt->params.conn_param_update.conn_params.conn_sup_timeout*10) );
        } break;

        case BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST: {
            SUBLE_PRINTF("BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST");
            // Accepting parameters requested by peer.
            err_code = sd_ble_gap_conn_param_update(p_gap_evt->conn_handle,
                                                    &p_gap_evt->params.conn_param_update_request.conn_params);
//            APP_ERROR_CHECK(err_code);
        } break;

        default: {
        } break;
    }
}

/*********************************************************
FN: init SoftDevice、BLE Stack， register BLE event callback
*/
void suble_ble_stack_init(void)
{
    ret_code_t err_code;

    //使能SoftDevice
    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    //设置默认的Stack配置，获取APP程序RAM的起始地址
    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_default_cfg_set(SUBLE_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    // 使能 BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    // 注册BLE Event通知函数
    NRF_SDH_BLE_OBSERVER(m_ble_observer, SUBLE_BLE_OBSERVER_PRIO, suble_ble_evt_handler, NULL);
}

/*********************************************************
FN: 初始化GAP（设备名称、外观、期望的连接参数）
*/
void suble_gap_params_init(void)
{
    ret_code_t              err_code;
    ble_gap_conn_sec_mode_t sec_mode;

    //安全模式
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
    //设备名称
    err_code = sd_ble_gap_device_name_set(&sec_mode, (const uint8_t *)SUBLE_DEVICE_NAME, strlen(SUBLE_DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    //设备外观
    err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_UNKNOWN);
    APP_ERROR_CHECK(err_code);

    //期望的连接参数
    suble_set_defConn_param();
}

/*********************************************************
FN: GATT module初始化（MTU和DLE）
*/
void suble_gatt_init(void)
{
    ret_code_t err_code;
    err_code = nrf_ble_gatt_init(&m_gatt, gatt_evt_handler);
    APP_ERROR_CHECK(err_code);

//    //设置MTU
//    err_code = nrf_ble_gatt_att_mtu_periph_set(&m_gatt, 128);
//    APP_ERROR_CHECK(err_code);
}

/*********************************************************
FN: Function for handling events from the GATT library.
*/
static void gatt_evt_handler(nrf_ble_gatt_t* p_gatt, nrf_ble_gatt_evt_t const* p_evt)
{
    if((g_conn_info[0].conn_handle == p_evt->conn_handle) && (p_evt->evt_id == NRF_BLE_GATT_EVT_ATT_MTU_UPDATED))
    {
        m_ble_max_data_len = p_evt->params.att_mtu_effective - OPCODE_LENGTH - HANDLE_LENGTH;
        SUBLE_PRINTF("Data len is set to 0x%X(%d)", m_ble_max_data_len, m_ble_max_data_len);
    }
    SUBLE_PRINTF("ATT MTU exchange completed. central 0x%x peripheral 0x%x", p_gatt->att_mtu_desired_central, p_gatt->att_mtu_desired_periph);
}




/*********************************************************
FN: 发起连接
*/
uint32_t suble_gap_connect(ble_gap_addr_t* pMac, suble_connect_result_handler_t handler)
{
    ble_gap_scan_params_t scan_param;
    memset(&scan_param, 0, sizeof(ble_gap_scan_params_t));
    scan_param.interval       = MSEC_TO_UNITS(g_scan_param.interval, UNIT_0_625_MS);
    scan_param.window         = MSEC_TO_UNITS(g_scan_param.window, UNIT_0_625_MS);
    scan_param.timeout        = MSEC_TO_UNITS(g_scan_param.timeout, UNIT_10_MS);
    scan_param.active         = g_scan_param.isActive;
    scan_param.filter_policy  = g_scan_param.filter_policy;
    scan_param.scan_phys      = BLE_GAP_PHY_1MBPS;
    
    ble_gap_conn_params_t conn_params;
    memset(&conn_params, 0, sizeof(ble_gap_conn_params_t));
    conn_params.min_conn_interval = MSEC_TO_UNITS(SUBLE_CONN_INTERVAL_MIN, UNIT_1_25_MS);
    conn_params.max_conn_interval = MSEC_TO_UNITS(SUBLE_CONN_INTERVAL_MAX, UNIT_1_25_MS);
    conn_params.slave_latency     = SUBLE_SLAVE_LATENCY;
    conn_params.conn_sup_timeout  = MSEC_TO_UNITS(SUBLE_CONN_SUP_TIMEOUT, UNIT_10_MS);
    
    ret_code_t err_code;
    err_code = sd_ble_gap_connect(pMac, &scan_param, &conn_params, SUBLE_BLE_CONN_CFG_TAG);
    if(err_code == NRF_SUCCESS) {
        suble_connect_result_handler = handler;
        s_reconnect_count = 0;
    }
    APP_ERROR_CHECK(err_code);
//    SUBLE_PRINTF("sd_ble_gap_connect err_code: %d", err_code);
    return err_code;
}

/*********************************************************
FN: 重新发起连接
*/
uint32_t suble_gap_reconnect(ble_gap_addr_t* pMac)
{
    ble_gap_scan_params_t scan_param;
    memset(&scan_param, 0, sizeof(ble_gap_scan_params_t));
    scan_param.interval       = MSEC_TO_UNITS(g_scan_param.interval, UNIT_0_625_MS);
    scan_param.window         = MSEC_TO_UNITS(g_scan_param.window, UNIT_0_625_MS);
    scan_param.timeout        = MSEC_TO_UNITS(g_scan_param.timeout, UNIT_10_MS);
    scan_param.active         = g_scan_param.isActive;
    scan_param.filter_policy  = g_scan_param.filter_policy;
    scan_param.scan_phys      = BLE_GAP_PHY_1MBPS;
    
    ble_gap_conn_params_t conn_params;
    memset(&conn_params, 0, sizeof(ble_gap_conn_params_t));
    conn_params.min_conn_interval = MSEC_TO_UNITS(SUBLE_CONN_INTERVAL_MIN, UNIT_1_25_MS);
    conn_params.max_conn_interval = MSEC_TO_UNITS(SUBLE_CONN_INTERVAL_MAX, UNIT_1_25_MS);
    conn_params.slave_latency     = SUBLE_SLAVE_LATENCY;
    conn_params.conn_sup_timeout  = MSEC_TO_UNITS(SUBLE_CONN_SUP_TIMEOUT, UNIT_10_MS);
    
    ret_code_t err_code;
    err_code = sd_ble_gap_connect(pMac, &scan_param, &conn_params, SUBLE_BLE_CONN_CFG_TAG);
    APP_ERROR_CHECK(err_code);
    return err_code;
}

/*********************************************************
FN: 断开连接
*/
void suble_gap_disconnect(uint16_t conn_handle, uint8_t hci_status_code)
{
    sd_ble_gap_disconnect(conn_handle, hci_status_code);
}

/*********************************************************
FN: 
*/
void suble_gap_disconnect_for_tuya_ble_sdk(void)
{
    SUBLE_PRINTF("suble_gap_disconnect_for_tuya_ble_sdk");
    sd_ble_gap_disconnect(g_conn_info[0].conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
}

/*********************************************************
FN: 
*/
uint16_t suble_get_m_ble_max_data_len(void)
{
    return m_ble_max_data_len;
}

/*********************************************************
FN: 设置默认连接参数
*/
static void suble_set_defConn_param(void)
{
    //期望的连接参数
    ble_gap_conn_params_t gap_conn_params;
    memset(&gap_conn_params, 0, sizeof(gap_conn_params));
    gap_conn_params.min_conn_interval = MSEC_TO_UNITS(SUBLE_CONN_INTERVAL_MIN, UNIT_1_25_MS);
    gap_conn_params.max_conn_interval = MSEC_TO_UNITS(SUBLE_CONN_INTERVAL_MAX, UNIT_1_25_MS);
    gap_conn_params.slave_latency     = SUBLE_SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = MSEC_TO_UNITS(SUBLE_CONN_SUP_TIMEOUT, UNIT_10_MS);
    
    ret_code_t err_code;
    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}

/*********************************************************
FN: 
*/
void suble_gap_conn_param_update(uint16_t conn_handle, uint16_t cMin, uint16_t cMax, uint16_t latency, uint16_t timeout)
{
    //当前的连接参数
    ble_gap_conn_params_t gap_conn_params;
    memset(&gap_conn_params, 0, sizeof(gap_conn_params));
    gap_conn_params.min_conn_interval = MSEC_TO_UNITS(cMin, UNIT_1_25_MS);
    gap_conn_params.max_conn_interval = MSEC_TO_UNITS(cMax, UNIT_1_25_MS);
    gap_conn_params.slave_latency     = latency;
    gap_conn_params.conn_sup_timeout  = MSEC_TO_UNITS(timeout, UNIT_10_MS);
    
    ret_code_t err_code;
    err_code = sd_ble_gap_conn_param_update(conn_handle, &gap_conn_params);
//    SUBLE_PRINTF("sd_ble_gap_conn_param_update err_code: %x", err_code);
//    APP_ERROR_CHECK(err_code);
}

/*********************************************************
FN: 
*/
void suble_gap_init_bt_mac(void)
{
    uint8_t auth = 0;
    //set bt addr
    uint8_t tmp_mac_str[SUBLE_BT_MAC_STR_LEN] = TUYA_DEVICE_MAC;
    uint8_t mac[SUBLE_BT_MAC_LEN];
    uint8_t tmp_mac_str_in_flash[SUBLE_BT_MAC_STR_LEN];
    uint8_t tmp_mac_invalid[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    
    suble_flash_read(SUBLE_FLASH_BLE_MAC_ADDR, tmp_mac_str_in_flash, SUBLE_BT_MAC_STR_LEN);
    if(memcmp(tmp_mac_str_in_flash, tmp_mac_invalid, SUBLE_BT_MAC_STR_LEN) != 0) {
        memcpy(tmp_mac_str, tmp_mac_str_in_flash, SUBLE_BT_MAC_STR_LEN);
        auth = 1;
    }
        
    if(suble_util_str_hexstr2hexarray(tmp_mac_str, SUBLE_BT_MAC_STR_LEN, mac) == SUBLE_SUCCESS)
    {
        if(!auth) {
            suble_util_reverse_byte(mac, SUBLE_BT_MAC_LEN);
        }
        SUBLE_HEXDUMP("Mac", mac, SUBLE_BT_MAC_LEN);
        suble_gap_set_bt_mac(mac);
    }
}

/*********************************************************
FN: 
*/
void suble_gap_set_bt_mac(uint8_t *pMac)
{
    bool adv_flag = false;

    if(g_isAdvtising) {
        adv_flag = true;
        //停止广播
        suble_adv_stop();
    }

    ble_gap_addr_t p_addr;
    memset(&p_addr, 0, sizeof(ble_gap_addr_t));
    p_addr.addr_type = BLE_GAP_ADDR_TYPE_PUBLIC;
    memcpy(p_addr.addr, pMac, SUBLE_BT_MAC_LEN);
    
    ret_code_t err_code;
    err_code = sd_ble_gap_addr_set(&p_addr);
    APP_ERROR_CHECK(err_code);

    if(adv_flag) {
        //启动广播
        suble_adv_start();
    }
}

/*********************************************************
FN: 
*/
void suble_gap_get_bt_mac(uint8_t *pMac, uint32_t size)
{
    ble_gap_addr_t mac;
    
    ret_code_t err_code;
    err_code = sd_ble_gap_addr_get(&mac);
    APP_ERROR_CHECK(err_code);
    
    memcpy(pMac, mac.addr, BLE_GAP_ADDR_LEN);
}

/*********************************************************
FN: 
*/
void suble_gap_conn_handler(void)
{
#ifdef TUYA_BLE_SDK_IN_SUBLE
    tuya_ble_app_evt_send(APP_EVT_CONNECTED);
#endif
}

/*********************************************************
FN: 
*/
void suble_gap_disconn_handler(void)
{
#ifdef TUYA_BLE_SDK_IN_SUBLE
    tuya_ble_app_evt_send(APP_EVT_DISCONNECTED);
#endif
}
















