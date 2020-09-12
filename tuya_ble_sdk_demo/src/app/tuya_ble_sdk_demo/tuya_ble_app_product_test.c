#include "tuya_ble_app_product_test.h"




tuya_ble_status_t tuya_ble_prod_beacon_scan_start(void)
{
    return TUYA_BLE_SUCCESS;
}

tuya_ble_status_t tuya_ble_prod_beacon_scan_stop(void)
{
    return TUYA_BLE_SUCCESS;
}

tuya_ble_status_t tuya_ble_prod_beacon_get_rssi_avg(int8_t *rssi)
{
    *rssi = -30;
    return TUYA_BLE_SUCCESS;
}

tuya_ble_status_t tuya_ble_prod_gpio_test(void)
{
    return TUYA_BLE_SUCCESS;
}

void tuya_ble_custom_app_production_test_process(uint8_t channel,uint8_t *p_in_data,uint16_t in_len)
{
    uint16_t cmd = 0;
    uint8_t *data_buffer = NULL;
    uint16_t data_len = ((p_in_data[4]<<8) + p_in_data[5]);
       
    if((p_in_data[6] != 3)||(data_len<3)){
        return;
    }
    
    cmd = (p_in_data[7]<<8) + p_in_data[8];
    data_len -= 3;
    if(data_len>0) {
        data_buffer = p_in_data+9;
        (void)data_buffer;
    }
    
    switch(cmd)
    {
        default: {
        } break;
    }
}





















