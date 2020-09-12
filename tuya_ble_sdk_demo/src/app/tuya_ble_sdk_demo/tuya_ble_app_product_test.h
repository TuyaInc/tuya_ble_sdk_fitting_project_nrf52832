/**
****************************************************************************
* @file      tuya_ble_app_product_test.h
* @brief     tuya_ble_app_product_test
* @author    suding
* @version   V1.0.0
* @date      2020-04
* @note
******************************************************************************
* @attention
*
* <h2><center>&copy; COPYRIGHT 2020 Tuya </center></h2>
*/


#ifndef __TUYA_BLE_APP_PRODUCT_TEST_H__
#define __TUYA_BLE_APP_PRODUCT_TEST_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDE
 */
#include "tuya_ble_port_bk3431q.h"

/*********************************************************************
 * CONSTANT
 */

/*********************************************************************
 * STRUCT
 */

/*********************************************************************
 * EXTERNAL VARIABLE
 */

/*********************************************************************
 * EXTERNAL FUNCTION
 */
tuya_ble_status_t tuya_ble_prod_beacon_scan_start(void);
tuya_ble_status_t tuya_ble_prod_beacon_scan_stop(void);
tuya_ble_status_t tuya_ble_prod_beacon_get_rssi_avg(int8_t *rssi);
tuya_ble_status_t tuya_ble_prod_gpio_test(void);
void tuya_ble_custom_app_production_test_process(uint8_t channel,uint8_t *p_in_data,uint16_t in_len);

#ifdef __cplusplus
}
#endif

#endif //__TUYA_BLE_APP_PRODUCT_TEST_H__

