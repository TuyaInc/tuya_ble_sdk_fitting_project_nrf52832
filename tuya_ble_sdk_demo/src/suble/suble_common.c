#include "suble_common.h"
#include "app_port.h"
#include "nrf_drv_wdt.h"





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




/*********************************************************  system  *********************************************************/

/*********************************************************
FN: 
*/
void suble_init_func(uint8_t location)
{
    switch(location)
    {
        case 0: {
            suble_gpio_init();
        } break;
        
        case 1: {
            lock_timer_creat();
        } break;
        
        case 2: {
            suble_test_func();
#ifdef TUYA_BLE_SDK_IN_SUBLE
            tuya_ble_app_init();
#endif
            suble_adv_start();
        } break;
        
        default: {
        } break;
    }
}

/*********************************************************
FN: 
*/
void suble_mainloop(void)
{
#ifdef TUYA_BLE_SDK_IN_SUBLE
    tuya_ble_main_tasks_exec();
#endif
}

/*********************************************************
FN: 
*/
void suble_system_reset(void)
{
    NRF_LOG_FINAL_FLUSH();
    NVIC_SystemReset();
}

/*********************************************************
FN: 
*/
static uint8_t __CR_NESTED = 0;
void suble_enter_critical(void)
{
   app_util_critical_region_enter(&__CR_NESTED);
}
void suble_exit_critical(void)
{
   app_util_critical_region_exit(__CR_NESTED);
}




static nrf_drv_wdt_channel_id m_channel_id;

/*********************************************************
FN: 
*/
static void wdt_event_handler(void)
{
    SUBLE_PRINTF("123456789");
    //NOTE: The max amount of time we can spend in WDT interrupt is two cycles of 32768[Hz] clock - after that, reset occurs
}

/*********************************************************
FN: 
*/
void suble_wdt_init(void)
{
    uint32_t err_code = NRF_SUCCESS;
    //Configure WDT.
    nrf_drv_wdt_config_t config = NRF_DRV_WDT_DEAFULT_CONFIG;
    err_code = nrf_drv_wdt_init(&config, wdt_event_handler);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_wdt_channel_alloc(&m_channel_id);
    APP_ERROR_CHECK(err_code);
    nrf_drv_wdt_enable();
}

/*********************************************************
FN: 
*/
void suble_wdt_feed(void)
{
    nrf_drv_wdt_channel_feed(m_channel_id);
}




/*********************************************************  log  *********************************************************/

/*********************************************************
FN: 
*/
void suble_log_init(void)
{
    elog_init();
//    elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_LVL);
    elog_start();
}

/*********************************************************
FN: 
*/
void suble_log_hexdump(const char *name, uint8_t *buf, uint16_t size)
{
    elog_hexdump(name, 8, buf, size);
}

/*********************************************************
FN: 
*/
void suble_log_hexdump_for_tuya_ble_sdk(const char *name, uint8_t width, uint8_t *buf, uint16_t size)
{
    elog_hexdump(name, width, buf, size);
}

/*********************************************************
FN: 
*/
void suble_log_hexdump_empty(const char *name, uint8_t *buf, uint16_t size)
{
    //empty
}




















