#include "suble_common.h"




/*********************************************************************
 * LOCAL CONSTANT
 */

/*********************************************************************
 * LOCAL STRUCT
 */
typedef struct
{
    uint8_t is_occupy;
    app_timer_t data;
    uint32_t ms;
} suble_timer_item_t;

/*********************************************************************
 * LOCAL VARIABLE
 */
static suble_timer_item_t m_timer_pool[SUBLE_TIMER_MAX_NUM] = {0};

/*********************************************************************
 * VARIABLE
 */

/*********************************************************************
 * LOCAL FUNCTION
 */




/*********************************************************  suble_timer app  *********************************************************/

/*********************************************************
FN: 
*/
static app_timer_id_t acquire_timer(uint32_t ms)
{
    for(uint8_t i=0; i<SUBLE_TIMER_MAX_NUM; i++) {
        if (m_timer_pool[i].is_occupy == 0) {
            m_timer_pool[i].is_occupy = 1;
            m_timer_pool[i].ms = ms;
            return (void*)&m_timer_pool[i].data;
        }
    }
    return NULL;
}

/*********************************************************
FN: 
*/
static int32_t release_timer(void* timer_id)
{
    for(uint8_t i=0; i<SUBLE_TIMER_MAX_NUM; i++) {
        if (timer_id == &m_timer_pool[i].data) {
            m_timer_pool[i].is_occupy = 0;
            return i;
        }
    }
    return -1;
}

/*********************************************************
FN: 
*/
static int32_t find_timer_ms(void* timer_id, uint32_t *ms)
{
    for(uint8_t i=0; i<SUBLE_TIMER_MAX_NUM; i++) {
        if (timer_id == &m_timer_pool[i].data) {
            *ms = m_timer_pool[i].ms;
            return i;
        }
    }
    return -1;
}

/*********************************************************
FN: 
*/
uint32_t suble_timer_create(void** p_timer_id, uint32_t timeout_value_ms, 
    suble_timer_mode_t mode, suble_timer_handler_t timeout_handler)
{
    ret_code_t err_code;
    
    app_timer_id_t id = acquire_timer(timeout_value_ms);
    if(id == NULL)
    {
        SUBLE_PRINTF("Error: App timer is used up");
        return NRF_ERROR_NO_MEM;
    }

    suble_timer_handler_t handler = timeout_handler;
    err_code = app_timer_create(&id, (app_timer_mode_t)mode, handler);
    APP_ERROR_CHECK(err_code);
    
    *p_timer_id = id;
    
    return err_code;
}

/*********************************************************
FN: 
*/
uint32_t suble_timer_delete(void* timer_id)
{
    ret_code_t err_code;
    
    int32_t id = release_timer(timer_id);
    if(id == -1)
    {
        SUBLE_PRINTF("Error: Timer id is not valid");
        return NRF_ERROR_INVALID_PARAM;
    }

    err_code = app_timer_stop(timer_id);
    APP_ERROR_CHECK(err_code);
    return err_code;
}

/*********************************************************
FN: 
*/
uint32_t suble_timer_start(void* timer_id)
{
    ret_code_t err_code;
    uint32_t ms;
    
    if(find_timer_ms(timer_id, &ms) >= 0)
    {
        err_code = app_timer_start(timer_id, APP_TIMER_TICKS(ms), NULL);
        APP_ERROR_CHECK(err_code);
        return err_code;
    }
    else
    {
        SUBLE_PRINTF("Error: Timer id is not found");
        return NRF_ERROR_NOT_FOUND;
    }
}

/*********************************************************
FN: 
*/
uint32_t suble_timer_stop(void* timer_id)
{
    ret_code_t err_code;
    err_code = app_timer_stop(timer_id);
    APP_ERROR_CHECK(err_code);
    return err_code;
}

/*********************************************************
FN: 
*/
uint32_t suble_timer_restart(void* timer_id, uint32_t timeout_value_ms)
{
    ret_code_t err_code;
    uint32_t ms;
    
    if(find_timer_ms(timer_id, &ms) >= 0) {
        err_code = app_timer_stop((app_timer_id_t)timer_id);
        APP_ERROR_CHECK(err_code);
        err_code = app_timer_start((app_timer_id_t)timer_id, APP_TIMER_TICKS(timeout_value_ms), NULL);
        APP_ERROR_CHECK(err_code);
        return err_code;
    }
    else {
        SUBLE_PRINTF("suble_timer_restart: not find_timer_ms");
        return SUBLE_ERROR_COMMON;
    }
}




/*********************************************************  RTC  *********************************************************/

/* Declaring an instance of nrf_drv_rtc for RTC2. */
const nrf_drv_rtc_t rtc2 = NRF_DRV_RTC_INSTANCE(2);
static uint32_t s_local_timestamp = 0;
static uint32_t s_local_timestamp_when_update = 0;
static uint32_t s_app_timestamp_when_update = 0;

/*********************************************************
FN: 
*/
static void suble_rtc_handler(nrf_drv_rtc_int_type_t int_type)
{
    static uint16_t count = 0;
    if(int_type == NRF_DRV_RTC_INT_TICK)
    {
        count++;
        if(count == 8)
        {
            count=0;
            s_local_timestamp++;
        }
    }
}

/*********************************************************
FN: 启动本地时间戳
AT: 1 is used by the ap timer, and 0 is used by the SoftDevice
*/
static void suble_rtc_start(void)
{
    uint32_t err_code;

    //Initialize RTC instance
    nrf_drv_rtc_config_t config = NRF_DRV_RTC_DEFAULT_CONFIG;
    config.prescaler = 4095;
    err_code = nrf_drv_rtc_init(&rtc2, &config, suble_rtc_handler);
    APP_ERROR_CHECK(err_code);

    //Enable tick event & interrupt
    nrf_drv_rtc_tick_enable(&rtc2, true);

    //Power on RTC instance
    nrf_drv_rtc_enable(&rtc2);
}

/*********************************************************
FN: 启动本地时间戳
*/
void suble_local_timer_start(void)
{
    suble_rtc_start();
}

/*********************************************************
FN: 更新时间戳
*/
void suble_update_timestamp(uint32_t app_timestamp)
{
    s_local_timestamp_when_update = s_local_timestamp;
    s_app_timestamp_when_update = app_timestamp;
}

/*********************************************************
FN: 获取更新时的app时间戳
*/
uint32_t suble_get_app_timestamp_when_update(void)
{
    return s_app_timestamp_when_update;
}

/*********************************************************
FN: 获取本地时间戳
*/
uint32_t suble_get_local_timestamp(void)
{
    return s_local_timestamp;
}

/*********************************************************
FN: 获取当前时间戳（如果没有更新过，即为本地时间戳）
*/
uint32_t suble_get_timestamp(void)
{
    return (s_app_timestamp_when_update + (s_local_timestamp - s_local_timestamp_when_update));
}

/*********************************************************
FN: 获取过去的时间戳（必须在更新时间戳之后使用，否则返回 old_local_timestamp）
*/
uint32_t suble_get_old_timestamp(uint32_t old_local_timestamp)
{
    return (suble_get_timestamp() - (s_local_timestamp - old_local_timestamp));
}

uint32_t suble_get_rtc2_counter(void)
{
    return nrf_drv_rtc_counter_get(&rtc2);
}



/*********************************************************  delay  *********************************************************/

/*********************************************************
FN: 
*/
void suble_delay_ms(uint32_t ms)
{
    nrf_delay_ms(ms);
}

/*********************************************************
FN: 
*/
void suble_delay_us(uint32_t us)
{
    nrf_delay_us(us);
}












