#include "suble_common.h"




/*********************************************************************
 * LOCAL CONSTANTS
 */
//Value used as error code on stack dump, can be used to identify stack location on stack unwind.
#define DEAD_BEEF                       0xDEADBEEF




/*********************************************************
FN: 
*/
void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{
//    __disable_irq();

    switch (id)
    {
        case NRF_FAULT_ID_SD_ASSERT: {
            SUBLE_PRINTF("SOFTDEVICE: ASSERTION FAILED");
        } break;
        
        case NRF_FAULT_ID_APP_MEMACC: {
            SUBLE_PRINTF("SOFTDEVICE: INVALID MEMORY ACCESS");
        } break;
        
        case NRF_FAULT_ID_SDK_ASSERT: {
            assert_info_t * p_info = (assert_info_t *)info;
            SUBLE_PRINTF("ASSERTION FAILED at %s:%u",
                          p_info->p_file_name,
                          p_info->line_num);
            break;
        }
        
        case NRF_FAULT_ID_SDK_ERROR: {
            error_info_t * p_info = (error_info_t *)info;
            SUBLE_PRINTF("ERROR %u [%s] at %s:%u\r\nPC at: 0x%08x",
                          p_info->err_code,
                          nrf_strerror_get(p_info->err_code),
                          p_info->p_file_name,
                          p_info->line_num,
                          pc);
             SUBLE_PRINTF("End of error report");
            break;
        }
        
        default: {
            SUBLE_PRINTF("UNKNOWN FAULT at 0x%08X", pc);
        } break;
    }

//    SUBLE_PRINTF("System reset");
//    NRF_LOG_FINAL_FLUSH();
//    NVIC_SystemReset();
}

/*********************************************************
FN: Callback function for asserts in the SoftDevice.
    This function will be called in case of an assert in the SoftDevice.

    @warning This handler is an example only and does not fit a final product.
             You need to analyze how your product is supposed to react in case of Assert.
    @warning On assert from the SoftDevice, the system can only recover on reset.

PM: line_num   Line number of the failing ASSERT call.
    file_name  File name of the failing ASSERT call.
*/
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

/*********************************************************
FN: Function for initializing the nrf log module.
*/
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}

/*********************************************************
FN: Function for initializing power management.
*/
static void power_management_init(void)
{
    ret_code_t err_code;
    err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);
}

/*********************************************************
FN: Function for handling the idle state (main loop).
    If there is no pending log operation, then sleep until next the next event occurs.
*/
static void idle_state_handle(void)
{
    suble_mainloop();
    suble_wdt_feed();
    if (NRF_LOG_PROCESS() == false)
    {
        nrf_pwr_mgmt_run();
    }
}

/*********************************************************
FN: Function for application main entry.
*/
int main(void)
{
    suble_init_func(0);
    //log init
    log_init();
    suble_log_init();
    //peripheral init
    suble_flash_init();
    suble_uart1_init();
    suble_local_timer_start();
    suble_init_func(1);
    //power manage init
    power_management_init();

    //ble init
    suble_ble_stack_init();
    suble_gap_params_init();
    suble_gap_init_bt_mac();
    suble_gatt_init();
    //connect state init
    ble_conn_state_init();
    //adv init
    suble_adv_init();
    suble_svc_init();
    //scan init
    suble_scan_init();
    suble_db_discovery_init();
    suble_svc_c_init();
    
    suble_wdt_init();
    
    suble_init_func(2);
    //Enter main loop
    for (;;)
    {
        idle_state_handle();
    }
}
































