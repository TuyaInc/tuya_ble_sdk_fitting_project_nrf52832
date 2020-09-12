#include "suble_common.h"
#include "app_port.h"




/*********************************************************************
 * LOCAL CONSTANT
 */
//广播和扫描响应数据
#define  DEFAULT_ADV_DATA                                                   \
            {                                                               \
                3,                                                          \
                {                                                           \
                    0x02,                                                   \
                    BLE_GAP_AD_TYPE_FLAGS,                                  \
                    BLE_GAP_ADV_FLAG_LE_GENERAL_DISC_MODE|BLE_GAP_ADV_FLAG_BR_EDR_NOT_SUPPORTED, \
                },                                                          \
            }
            
#define  DEFAULT_SCAN_RSP                                                   \
            {                                                               \
                6,                                                          \
                {                                                           \
                    0x05,                                                   \
                    BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME,                    \
                    'D', 'e', 'm', 'o',                                     \
                },                                                          \
            }

//广播和扫描参数
#define  DEFAULT_ADV_PARAM                                                  \
            {                                                               \
                .adv_interval_min = SUBLE_ADV_INTERVAL_MIN,                 \
                .adv_interval_max = SUBLE_ADV_INTERVAL_MAX,                 \
                .adv_type         = BLE_GAP_ADV_TYPE_CONNECTABLE_SCANNABLE_UNDIRECTED, \
                .adv_power        = 0x00,                                   \
                .adv_channal_map  = 0x07,                                   \
            }

#define     DEFAULT_SCAN_PARAM                                              \
            {                                                               \
                .interval         = 5,                                     \
                .window           = 5,                                     \
                .timeout          = 10000,                                   \
                .isActive         = 1,                                      \
                .filter_policy    = BLE_GAP_SCAN_FP_ACCEPT_ALL,             \
                .isFiltDuplicated = 1,                                      \
                .minRssi          = -50,                                    \
                .maxRes           = 10,                                     \
            }

/*********************************************************************
 * LOCAL STRUCT
 */

/*********************************************************************
 * VARIABLE
 */
//Adv
adv_data_t      g_adv_data      = DEFAULT_ADV_DATA;
adv_data_t      g_scan_rsp      = DEFAULT_SCAN_RSP;
adv_param_t     g_adv_param     = DEFAULT_ADV_PARAM;
bool            g_isAdvtising   = false;

//Scan
scan_param_t    g_scan_param    = DEFAULT_SCAN_PARAM;
bool            g_is_scanning   = false;

/*********************************************************************
 * LOCAL VARIABLE
 */
//Adv
static uint8_t m_adv_handle                 = BLE_GAP_ADV_SET_HANDLE_NOT_SET;
static ble_gap_adv_params_t m_adv_params    = {0};
static ble_gap_adv_data_t   m_adv_data      = {0};

//Scan
NRF_BLE_SCAN_DEF(m_scan);
static ble_gap_scan_params_t m_scan_param   = {0};
static suble_scan_result_handler_t suble_scan_result_handler;

/*********************************************************************
 * LOCAL FUNCTION
 */




/*********************************************************  adv  *********************************************************/

/*********************************************************
FN: 初始化广播
*/
void suble_adv_init(void)
{
    m_adv_params.interval           = MSEC_TO_UNITS(g_adv_param.adv_interval_max, UNIT_0_625_MS);
    m_adv_params.duration           = MSEC_TO_UNITS(BLE_GAP_ADV_TIMEOUT_GENERAL_UNLIMITED, UNIT_10_MS);
    m_adv_params.properties.type    = g_adv_param.adv_type;
    m_adv_params.primary_phy        = 0x01;
    m_adv_params.secondary_phy      = 0x01;
    m_adv_params.filter_policy      = BLE_GAP_ADV_FP_ANY;

    m_adv_data.adv_data.len         = g_adv_data.len;
    m_adv_data.adv_data.p_data      = g_adv_data.value;
    m_adv_data.scan_rsp_data.len    = g_scan_rsp.len;
    m_adv_data.scan_rsp_data.p_data = g_scan_rsp.value;

    ret_code_t err_code;
    err_code = sd_ble_gap_adv_set_configure(&m_adv_handle, &m_adv_data, &m_adv_params);
    APP_ERROR_CHECK(err_code);
}

/*********************************************************
FN: 启动广播
*/
void suble_adv_start(void)
{
    ret_code_t err_code;
    if(g_isAdvtising == false) {
        err_code = sd_ble_gap_adv_start(m_adv_handle, SUBLE_BLE_CONN_CFG_TAG);
        APP_ERROR_CHECK(err_code);
        if(err_code == NRF_SUCCESS) {
            g_isAdvtising = true;
            SUBLE_PRINTF("adv start");
        }
    }
    else {
//        SUBLE_PRINTF("Error: Device is advertising");
    }
}

/*********************************************************
FN: 停止广播
*/
void suble_adv_stop(void)
{
    ret_code_t err_code;
    if(g_isAdvtising == true) {
        err_code = sd_ble_gap_adv_stop(m_adv_handle);
        APP_ERROR_CHECK(err_code);
        if(err_code == NRF_SUCCESS) {
            g_isAdvtising = false;
            SUBLE_PRINTF("adv stop");
        }
    }
    else {
//        SUBLE_PRINTF("Error: Device is not advertising");
    }
}

/*********************************************************
FN: 更新广播状态
*/
void suble_adv_state_update(bool adv_state)
{
    g_isAdvtising = adv_state;
}

/*********************************************************
FN: 更新广播和扫描响应数据
*/
void suble_adv_update_advDataAndScanRsp(void)
{
    bool adv_flag = false;

    if(g_isAdvtising) {
        adv_flag = true;
        //停止广播
        suble_adv_stop();
    }

    m_adv_data.adv_data.len         = g_adv_data.len;
    m_adv_data.adv_data.p_data      = g_adv_data.value;
    m_adv_data.scan_rsp_data.len    = g_scan_rsp.len;
    m_adv_data.scan_rsp_data.p_data = g_scan_rsp.value;

    ret_code_t err_code;
    err_code = sd_ble_gap_adv_set_configure(&m_adv_handle, &m_adv_data, NULL);
    APP_ERROR_CHECK(err_code);

    if(adv_flag) {
        //启动广播
        suble_adv_start();
    }
}

/*********************************************************
FN: 设置广播参数，结合 g_adv_param 使用
*/
void suble_adv_param_set(void)
{
    bool adv_flag = false;

    if(g_isAdvtising) {
        adv_flag = true;
        //停止广播
        suble_adv_stop();
    }

    m_adv_params.interval           = MSEC_TO_UNITS(g_adv_param.adv_interval_max, UNIT_0_625_MS);
    m_adv_params.duration           = MSEC_TO_UNITS(BLE_GAP_ADV_TIMEOUT_GENERAL_UNLIMITED, UNIT_10_MS);
    m_adv_params.properties.type    = g_adv_param.adv_type;
    m_adv_params.primary_phy        = 0x01;
    m_adv_params.secondary_phy      = 0x01;
    m_adv_params.filter_policy      = BLE_GAP_ADV_FP_ANY;

    ret_code_t err_code;
    err_code = sd_ble_gap_adv_set_configure(&m_adv_handle, NULL, &m_adv_params);
    APP_ERROR_CHECK(err_code);

    if(adv_flag) {
        //启动广播
        suble_adv_start();
    }
}




/*********************************************************  scan  *********************************************************/

/*********************************************************
FN: 处理扫描事件
*/
uint32_t g_scan_count = 0;
static void suble_scan_evt_handler(scan_evt_t const * p_scan_evt)
{
    ret_code_t err_code;

    switch(p_scan_evt->scan_evt_id)
    {
        case NRF_BLE_SCAN_EVT_FILTER_MATCH: {
            SUBLE_PRINTF("NRF_BLE_SCAN_EVT_FILTER_MATCH");
        } break;

        case NRF_BLE_SCAN_EVT_WHITELIST_REQUEST: {
            SUBLE_PRINTF("NRF_BLE_SCAN_EVT_WHITELIST_REQUEST");
        } break;

        case NRF_BLE_SCAN_EVT_WHITELIST_ADV_REPORT: {
            SUBLE_PRINTF("NRF_BLE_SCAN_EVT_WHITELIST_ADV_REPORT");
        } break;

        case NRF_BLE_SCAN_EVT_NOT_FOUND: {
            ble_gap_evt_adv_report_t const* adv = p_scan_evt->params.p_not_found;
            suble_scan_result_handler(SUBLE_SCAN_EVT_ADV_REPORT, (void*)adv, 0);
//            elog_hexdump("1", 20, (void*)adv->peer_addr.addr, 6);
//            g_scan_count++;
//            SUBLE_PRINTF("g_scan_count: %d", g_scan_count);
        } break;

        case NRF_BLE_SCAN_EVT_SCAN_TIMEOUT: {
            suble_scan_result_handler(SUBLE_SCAN_EVT_SCAN_TIMEOUT, NULL, 0);
//            SUBLE_PRINTF("NRF_BLE_SCAN_EVT_SCAN_TIMEOUT");
        } break;

        case NRF_BLE_SCAN_EVT_SCAN_REQ_REPORT: {
            SUBLE_PRINTF("NRF_BLE_SCAN_EVT_SCAN_REQ_REPORT");
        } break;

        case NRF_BLE_SCAN_EVT_CONNECTING_ERROR: {
            SUBLE_PRINTF("NRF_BLE_SCAN_EVT_CONNECTING_ERROR");
        } break;

        default: {
        } break;
    }
}

/*********************************************************
FN: 扫描初始化
*/
void suble_scan_init(void)
{
    nrf_ble_scan_init_t init_scan;
    memset(&init_scan, 0, sizeof(init_scan));
    init_scan.conn_cfg_tag = SUBLE_BLE_CONN_CFG_TAG;
    
    ret_code_t err_code;
    err_code = nrf_ble_scan_init(&m_scan, &init_scan, suble_scan_evt_handler);
    APP_ERROR_CHECK(err_code);
    
    suble_scan_param_set();
}

/*********************************************************
FN: 启动扫描
*/
void suble_scan_start(suble_scan_result_handler_t handler)
{
    g_scan_count = 0;
    ret_code_t err_code;
    err_code = nrf_ble_scan_start(&m_scan);
    APP_ERROR_CHECK(err_code);
    if(err_code == NRF_SUCCESS) {
        g_is_scanning = true;
        
        suble_scan_result_handler = handler;
        
        SUBLE_PRINTF("scan start");
    }
}

/*********************************************************
FN: 停止扫描
*/
void suble_scan_stop(void)
{
    nrf_ble_scan_stop();
    g_is_scanning = false;
    SUBLE_PRINTF("scan stop");
}

/*********************************************************
FN: 设置扫描参数
*/
void suble_scan_param_set(void)
{
    m_scan_param.interval       = MSEC_TO_UNITS(g_scan_param.interval, UNIT_0_625_MS);
    m_scan_param.window         = MSEC_TO_UNITS(g_scan_param.window, UNIT_0_625_MS);
    m_scan_param.timeout        = MSEC_TO_UNITS(g_scan_param.timeout, UNIT_10_MS);
    m_scan_param.active         = g_scan_param.isActive;
    m_scan_param.filter_policy  = g_scan_param.filter_policy;
    m_scan_param.scan_phys      = BLE_GAP_PHY_1MBPS;
    
    ret_code_t err_code;
    err_code = nrf_ble_scan_params_set(&m_scan, &m_scan_param);
    APP_ERROR_CHECK(err_code);
}



























