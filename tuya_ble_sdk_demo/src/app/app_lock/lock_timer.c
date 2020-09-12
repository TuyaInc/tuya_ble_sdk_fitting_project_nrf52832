#include "lock_timer.h"




/*********************************************************************
 * LOCAL CONSTANTS
 */

/*********************************************************************
 * LOCAL STRUCT
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
static void* lock_timer[LOCK_TUMER_MAX];

/*********************************************************************
 * LOCAL FUNCTION
 */

/*********************************************************************
 * VARIABLES
 */




/*********************************************************
FN: 
*/
void conn_param_update_outtime_cb_handler(void)
{
    app_port_conn_param_update(SUBLE_CONN_INTERVAL_MIN, SUBLE_CONN_INTERVAL_MAX, SUBLE_SLAVE_LATENCY, SUBLE_CONN_SUP_TIMEOUT);
}
static void conn_param_update_outtime_cb(void* timer)
{
    tuya_ble_app_evt_send(APP_EVT_TIMER_0);
}

/*********************************************************
FN: 
*/
void delay_report_outtime_cb_handler(void)
{
    delay_report_outtime_handler();
}
static void delay_report_outtime_cb(void* timer)
{
    tuya_ble_app_evt_send(APP_EVT_TIMER_1);
}

/*********************************************************
FN: 
*/
void bonding_conn_outtime_cb_handler(void)
{
    lock_offline_evt_report(0xFF);
}
static void bonding_conn_outtime_cb(void* timer)
{
    tuya_ble_app_evt_send(APP_EVT_TIMER_2);
}

/*********************************************************
FN: 
*/
void reset_with_disconn_outtime_cb_handler(void)
{
    app_port_ble_gap_disconnect();
    lock_timer_start(LOCK_TIMER_RESET_WITH_DISCONN2);
}
static void reset_with_disconn_outtime_cb(void* timer)
{
    tuya_ble_app_evt_send(APP_EVT_TIMER_3);
}

/*********************************************************
FN: 
*/
void app_test_outtime_cb_handler(void)
{
    app_test_outtime_handler();
}
static void app_test_outtime_cb(void* timer)
{
    tuya_ble_app_evt_send(APP_EVT_TIMER_5);
}

/*********************************************************
FN: 
*/
void app_test_reset_outtime_cb_handler(void)
{
    app_port_device_reset();
}
static void app_test_reset_outtime_cb(void* timer)
{
    tuya_ble_app_evt_send(APP_EVT_TIMER_6);
}

/*********************************************************
FN: 
*/
void app_active_report_outtime_cb_handler(void)
{
    app_active_report_stop(ACTICE_REPORT_STOP_STATE_OUTTIME);
}
static void app_active_report_outtime_cb(void* timer)
{
    tuya_ble_app_evt_send(APP_EVT_TIMER_7);
}

/*********************************************************
FN: 
*/
void reset_with_disconn2_outtime_cb_handler(void)
{
    app_port_device_reset();
}
static void reset_with_disconn2_outtime_cb(void* timer)
{
    tuya_ble_app_evt_send(APP_EVT_TIMER_8);
}

/*********************************************************
FN: 
*/
void communication_monitor_outtime_cb_handler(void)
{
    TUYA_APP_LOG_INFO("ble disconncet because communication monitor timer timeout.");
    app_port_ble_gap_disconnect();
}
static void communication_monitor_outtime_cb(tuya_ble_timer_t timer)
{
    tuya_ble_app_evt_send(APP_EVT_TIMER_9);
}

/*********************************************************
FN: 
*/
uint32_t lock_timer_creat(void)
{
    uint32_t ret = 0;
    ret += app_port_timer_create(&lock_timer[LOCK_TIMER_CONN_PARAM_UPDATE], 1000, SUBLE_TIMER_SINGLE_SHOT, conn_param_update_outtime_cb);
    ret += app_port_timer_create(&lock_timer[LOCK_TIMER_DELAY_REPORT], 200, SUBLE_TIMER_SINGLE_SHOT, delay_report_outtime_cb);
    ret += app_port_timer_create(&lock_timer[LOCK_TIMER_BONDING_CONN], 1000, SUBLE_TIMER_SINGLE_SHOT, bonding_conn_outtime_cb);
    ret += app_port_timer_create(&lock_timer[LOCK_TIMER_RESET_WITH_DISCONN], 1000, SUBLE_TIMER_SINGLE_SHOT, reset_with_disconn_outtime_cb);
    ret += app_port_timer_create(&lock_timer[LOCK_TIMER_APP_TEST_OUTTIME], APP_TEST_MODE_ENTER_OUTTIME_MS, SUBLE_TIMER_SINGLE_SHOT, app_test_outtime_cb);
    ret += app_port_timer_create(&lock_timer[LOCK_TIMER_APP_TEST_RESET_OUTTIME], 500, SUBLE_TIMER_SINGLE_SHOT, app_test_reset_outtime_cb);
    ret += app_port_timer_create(&lock_timer[LOCK_TIMER_ACTIVE_REPORT], 30000, SUBLE_TIMER_SINGLE_SHOT, app_active_report_outtime_cb);
    ret += app_port_timer_create(&lock_timer[LOCK_TIMER_RESET_WITH_DISCONN2], 1000, SUBLE_TIMER_SINGLE_SHOT, reset_with_disconn2_outtime_cb);
    ret += app_port_timer_create(&lock_timer[LOCK_TIMER_COMMUNICATION_MONITOR], 120000, TUYA_BLE_TIMER_SINGLE_SHOT, communication_monitor_outtime_cb);
    //tuya_ble_xtimer_connect_monitor
    return ret;
}

/*********************************************************
FN: 
*/
uint32_t lock_timer_delete(lock_timer_t p_timer)
{
    if(p_timer >= LOCK_TUMER_MAX)
    {
        return APP_PORT_ERROR_COMMON;
    }
    
    return app_port_timer_delete(lock_timer[p_timer]);
}

/*********************************************************
FN: 
*/
uint32_t lock_timer_start(lock_timer_t p_timer)
{
    if(p_timer >= LOCK_TUMER_MAX)
    {
        return APP_PORT_ERROR_COMMON;
    }
    
    return app_port_timer_start(lock_timer[p_timer]);
}

/*********************************************************
FN: 
*/
uint32_t lock_timer_stop(lock_timer_t p_timer)
{
    if(p_timer >= LOCK_TUMER_MAX)
    {
        return APP_PORT_ERROR_COMMON;
    }
    
    return app_port_timer_stop(lock_timer[p_timer]);
}




























