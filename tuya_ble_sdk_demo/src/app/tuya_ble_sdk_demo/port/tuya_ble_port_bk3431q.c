#include "tuya_ble_port_bk3431q.h"




/*********************************************************
FN: 
*/
tuya_ble_status_t tuya_ble_gap_advertising_adv_data_update(uint8_t const* p_ad_data, uint8_t ad_len)
{
    g_adv_data.len = ad_len;
    memcpy(g_adv_data.value, p_ad_data, ad_len);
    suble_adv_update_advDataAndScanRsp();
    return TUYA_BLE_SUCCESS;
}

tuya_ble_status_t tuya_ble_gap_advertising_scan_rsp_data_update(uint8_t const *p_sr_data, uint8_t sr_len)
{
    g_scan_rsp.len = sr_len;
    memcpy(g_scan_rsp.value, p_sr_data, sr_len);
    suble_adv_update_advDataAndScanRsp();
    return TUYA_BLE_SUCCESS;
}

/*********************************************************
FN: 
*/
tuya_ble_status_t tuya_ble_gap_disconnect(void)
{
     suble_gap_disconnect_for_tuya_ble_sdk();
     return TUYA_BLE_SUCCESS;
}

/*********************************************************
FN: 
*/
tuya_ble_status_t tuya_ble_gap_addr_get(tuya_ble_gap_addr_t *p_addr)
{
    suble_gap_get_bt_mac((void*)p_addr, SUBLE_BT_MAC_LEN);
    return TUYA_BLE_SUCCESS;
}

tuya_ble_status_t tuya_ble_gap_addr_set(tuya_ble_gap_addr_t *p_addr)
{
    suble_gap_set_bt_mac((void*)p_addr);
    return TUYA_BLE_SUCCESS;
}

/*********************************************************
FN: 
*/
tuya_ble_status_t tuya_ble_gatt_send_data(const uint8_t *p_data,uint16_t len)
{
    uint8_t data_len = len;
    if(data_len > TUYA_BLE_DATA_MTU_MAX) {
        data_len = TUYA_BLE_DATA_MTU_MAX;
    }
    
    suble_svc_send_data((void*)p_data, len);
    return TUYA_BLE_SUCCESS;
}

/*********************************************************
FN: 
*/
tuya_ble_status_t tuya_ble_common_uart_init(void)
{    
    return TUYA_BLE_SUCCESS;
}

tuya_ble_status_t tuya_ble_common_uart_send_data(const uint8_t *p_data,uint16_t len)
{
    suble_uart1_send(p_data, len);
    return TUYA_BLE_SUCCESS;
}

/*********************************************************
FN: 
*/
tuya_ble_status_t tuya_ble_timer_create(void** p_timer_id,uint32_t timeout_value_ms, tuya_ble_timer_mode mode,tuya_ble_timer_handler_t timeout_handler)
{
    suble_timer_create(p_timer_id, timeout_value_ms, (suble_timer_mode_t)mode, timeout_handler);
    return TUYA_BLE_SUCCESS;
}


tuya_ble_status_t tuya_ble_timer_delete(void* timer_id)
{
    suble_timer_delete(timer_id);
    return TUYA_BLE_SUCCESS;
}

tuya_ble_status_t tuya_ble_timer_start(void* timer_id)
{
    suble_timer_start(timer_id);
    return TUYA_BLE_SUCCESS;
}

tuya_ble_status_t tuya_ble_timer_restart(void* timer_id,uint32_t timeout_value_ms)
{
    suble_timer_restart(timer_id, timeout_value_ms);
    return TUYA_BLE_SUCCESS;
}

tuya_ble_status_t tuya_ble_timer_stop(void* timer_id)
{
    suble_timer_stop(timer_id);
    return TUYA_BLE_SUCCESS;
}

/*********************************************************
FN: 
*/
void tuya_ble_device_delay_ms(uint32_t ms)
{
    suble_delay_ms(ms);
}

/*********************************************************
FN: 
*/
tuya_ble_status_t tuya_ble_rand_generator(uint8_t* p_buf, uint8_t len)
{
    uint32_t cnt=len/4;
    uint8_t  remain = len%4;
    int32_t temp;
    for(uint32_t i=0; i<cnt; i++)
    {
        temp = rand();
        memcpy(p_buf,(uint8_t *)&temp,4);
        p_buf += 4;
    }
    temp = rand();
    memcpy(p_buf,(uint8_t *)&temp,remain);

    return TUYA_BLE_SUCCESS;
}

/*********************************************************
FN: 
*/
tuya_ble_status_t tuya_ble_device_reset(void)
{
    suble_system_reset();
    return TUYA_BLE_SUCCESS;
}

/*********************************************************
FN: 
*/
void tuya_ble_device_enter_critical(void)
{
   suble_enter_critical();
}

void tuya_ble_device_exit_critical(void)
{
   suble_exit_critical();
}

/*********************************************************
FN: 
*/
tuya_ble_status_t tuya_ble_rtc_get_timestamp(uint32_t *timestamp,int32_t *timezone)
{
    *timestamp = 0;
    *timezone = 0;
    return TUYA_BLE_SUCCESS;
}

tuya_ble_status_t tuya_ble_rtc_set_timestamp(uint32_t timestamp,int32_t timezone)
{
    return TUYA_BLE_SUCCESS;
}

/*********************************************************
FN: 
*/
tuya_ble_status_t tuya_ble_nv_init(void)
{
    return TUYA_BLE_SUCCESS;
}

tuya_ble_status_t tuya_ble_nv_erase(uint32_t addr,uint32_t size)
{
    uint32_t erase_pages = size / TUYA_NV_ERASE_MIN_SIZE;
    if (size % TUYA_NV_ERASE_MIN_SIZE != 0) {
        erase_pages++;
    }
    
    suble_flash_erase(addr, erase_pages);
    return TUYA_BLE_SUCCESS;
}

tuya_ble_status_t tuya_ble_nv_write(uint32_t addr,const uint8_t *p_data, uint32_t size)
{  
    suble_flash_write(addr, (void*)p_data, size);
    return TUYA_BLE_SUCCESS;
}

tuya_ble_status_t tuya_ble_nv_read(uint32_t addr,uint8_t *p_data, uint32_t size)
{
    suble_flash_read(addr, (void*)p_data, size);
    return TUYA_BLE_SUCCESS;
}




#if TUYA_BLE_USE_OS

bool tuya_ble_os_task_create(void **pp_handle, const char *p_name, void (*p_routine)(void *),void *p_param, uint16_t stack_size, uint16_t priority)
{
    return os_task_create(pp_handle, p_name, p_routine,p_param, stack_size, priority);
}

bool tuya_ble_os_task_delete(void *p_handle)
{
    return os_task_delete(p_handle);
}

bool tuya_ble_os_task_suspend(void *p_handle)
{
    return os_task_suspend(p_handle);
}

bool tuya_ble_os_task_resume(void *p_handle)
{
    return os_task_resume(p_handle);
}

bool tuya_ble_os_msg_queue_create(void **pp_handle, uint32_t msg_num, uint32_t msg_size)
{
    return os_msg_queue_create(pp_handle, msg_num, msg_size);
}

bool tuya_ble_os_msg_queue_delete(void *p_handle)
{
    return os_msg_queue_delete(p_handle);
}

bool tuya_ble_os_msg_queue_peek(void *p_handle, uint32_t *p_msg_num)
{
    return os_msg_queue_peek(p_handle, p_msg_num);
}

bool tuya_ble_os_msg_queue_send(void *p_handle, void *p_msg, uint32_t wait_ms)
{
    return os_msg_send(p_handle, p_msg, wait_ms);
}

bool tuya_ble_os_msg_queue_recv(void *p_handle, void *p_msg, uint32_t wait_ms)
{
    return os_msg_recv(p_handle, p_msg, wait_ms);
}

#endif


bool tuya_ble_aes128_ecb_encrypt(uint8_t *key,uint8_t *input,uint16_t input_len,uint8_t *output)
{
    uint16_t length;
    mbedtls_aes_context aes_ctx;
    //
    if(input_len%16)
    {
        return false;
    }

    length = input_len;

    mbedtls_aes_init(&aes_ctx);

    mbedtls_aes_setkey_enc(&aes_ctx, key, 128);

    while( length > 0 )
    {
        mbedtls_aes_crypt_ecb(&aes_ctx, MBEDTLS_AES_ENCRYPT, input, output );
        input  += 16;
        output += 16;
        length -= 16;
    }

    mbedtls_aes_free(&aes_ctx);

    return true;
}

bool tuya_ble_aes128_ecb_decrypt(uint8_t *key,uint8_t *input,uint16_t input_len,uint8_t *output)
{
    uint16_t length;
    mbedtls_aes_context aes_ctx;
    //
    if(input_len%16)
    {
        return false;
    }

    length = input_len;

    mbedtls_aes_init(&aes_ctx);

    mbedtls_aes_setkey_dec(&aes_ctx, key, 128);

    while( length > 0 )
    {
        mbedtls_aes_crypt_ecb(&aes_ctx, MBEDTLS_AES_DECRYPT, input, output );
        input  += 16;
        output += 16;
        length -= 16;
    }

    mbedtls_aes_free(&aes_ctx);

    return true;
}

bool tuya_ble_aes128_cbc_encrypt(uint8_t *key,uint8_t *iv,uint8_t *input,uint16_t input_len,uint8_t *output)
{
    mbedtls_aes_context aes_ctx;
    //
    if(input_len%16)
    {
        return false;
    }

    mbedtls_aes_init(&aes_ctx);

    mbedtls_aes_setkey_enc(&aes_ctx, key, 128);
    
    mbedtls_aes_crypt_cbc(&aes_ctx,MBEDTLS_AES_ENCRYPT,input_len,iv,input,output);
    //
    mbedtls_aes_free(&aes_ctx);

    return true;
}

bool tuya_ble_aes128_cbc_decrypt(uint8_t *key,uint8_t *iv,uint8_t *input,uint16_t input_len,uint8_t *output)
{
    mbedtls_aes_context aes_ctx;
    //
    if(input_len%16)
    {
        return false;
    }

    mbedtls_aes_init(&aes_ctx);

    mbedtls_aes_setkey_dec(&aes_ctx, key, 128);
    
    mbedtls_aes_crypt_cbc(&aes_ctx,MBEDTLS_AES_DECRYPT,input_len,iv,input,output);
    //
    mbedtls_aes_free(&aes_ctx);

    return true;
}


bool tuya_ble_md5_crypt(uint8_t *input,uint16_t input_len,uint8_t *output)
{
    mbedtls_md5_context md5_ctx;
    mbedtls_md5_init(&md5_ctx);
    mbedtls_md5_starts(&md5_ctx);
    mbedtls_md5_update(&md5_ctx, input, input_len);
    mbedtls_md5_finish(&md5_ctx, output);
    mbedtls_md5_free(&md5_ctx);    
    
    return true;
}

bool tuya_ble_hmac_sha1_crypt(const uint8_t *key, uint32_t key_len, const uint8_t *input, uint32_t input_len, uint8_t *output)
{
	return true;
}

bool tuya_ble_hmac_sha256_crypt(const uint8_t *key, uint32_t key_len, const uint8_t *input, uint32_t input_len, uint8_t *output)
{
	return true;
}

