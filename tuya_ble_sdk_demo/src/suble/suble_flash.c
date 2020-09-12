#include "suble_common.h"




/*********************************************************************
 * LOCAL CONSTANT
 */
#define FLASH_OP_INDEX_MAX       10
#define FLASH_CACHE_MAX_BYTES    256

/*********************************************************************
 * LOCAL STRUCT
 */
typedef struct
{
    uint32_t flag;
} flash_op_record_t;

/*********************************************************************
 * LOCAL VARIABLE
 */
static nrf_atomic_u32_t current_flash_op_index = 0;
static volatile flash_op_record_t flash_op_record[FLASH_OP_INDEX_MAX];
static __ALIGN(sizeof(uint32_t)) uint8_t nrfs_flash_cache[FLASH_CACHE_MAX_BYTES];

/*********************************************************************
 * VARIABLE
 */
static void fstorage_evt_handler(nrf_fstorage_evt_t* p_evt);
NRF_FSTORAGE_DEF(nrf_fstorage_t fstorage) =
{
    .evt_handler = fstorage_evt_handler,
    .start_addr = SUBLE_FLASH_START_ADDR,
    .end_addr   = SUBLE_FLASH_END_ADDR,
};

/*********************************************************************
 * LOCAL FUNCTION
 */
static uint32_t get_current_flash_op_index(void);




/*********************************************************
FN: 
*/
void suble_flash_init(void)
{
    //init param
    nrf_atomic_u32_store(&current_flash_op_index, 0);
    memset((void *)&flash_op_record[0], 0, sizeof(flash_op_record));
    
    //init fstorage
    nrf_fstorage_api_t* p_fs_api;
    p_fs_api = &nrf_fstorage_sd;
    
    ret_code_t err_code;
    err_code = nrf_fstorage_init(&fstorage, p_fs_api, NULL);
    APP_ERROR_CHECK(err_code);
}

/*********************************************************
FN: 
*/
static uint32_t get_current_flash_op_index(void)
{
    uint32_t index;
    index = nrf_atomic_u32_add(&current_flash_op_index, 1);
    
    if(index >= FLASH_OP_INDEX_MAX) {
        index = nrf_atomic_u32_store(&current_flash_op_index, 0);
    }
    return index;
}

/*********************************************************
FN: 
*/
static void fstorage_evt_handler(nrf_fstorage_evt_t* p_evt)
{
    flash_op_record_t* p_op;
    p_op = (void*)p_evt->p_param;
    
    if (p_evt->result != NRF_SUCCESS) {
        p_op->flag = 0xFFFFFFFF;
        SUBLE_PRINTF("--> Event received: ERROR while executing an fstorage operation. %d bytes at address 0x%x.", p_evt->len, p_evt->addr);
        return;
    }

    switch (p_evt->id)
    {
        case NRF_FSTORAGE_EVT_WRITE_RESULT: {
            p_op->flag = 1;
        } break;

        case NRF_FSTORAGE_EVT_ERASE_RESULT: {
            p_op->flag = 1;
        } break;

        default: {
        } break;
    }
}

/*********************************************************
FN: 
*/
void suble_flash_read(uint32_t addr, uint8_t *buf, uint32_t size)
{
    suble_wdt_feed();
    
    ret_code_t err_code;
    err_code = nrf_fstorage_read(&fstorage, addr, buf, size);
    APP_ERROR_CHECK(err_code);
}

/*********************************************************
FN: 
*/
void suble_flash_write(uint32_t addr, uint8_t *buf, uint32_t size)
{
    suble_wdt_feed();
    
    bool block = true;

    if(size == 0) {
//        SUBLE_PRINTF("Error: flash write size zero");
        return;
    }
    
    if(size > FLASH_CACHE_MAX_BYTES) {
        SUBLE_PRINTF("Error: flash write size error");
        return;
    }
    
    uint32_t op_index = get_current_flash_op_index();
    memset((void*)&flash_op_record[op_index], 0, sizeof(flash_op_record_t));
    memcpy(nrfs_flash_cache, buf, size);
    
    ret_code_t err_code = nrf_fstorage_write(&fstorage, addr, nrfs_flash_cache, size, (void*)&flash_op_record[op_index]);
    if (err_code == NRF_SUCCESS) {
        if(!block) {
            SUBLE_PRINTF("flash write success");
            return;
        }
    }
    else {
        SUBLE_PRINTF("Error: flash write err_code %d", err_code);
        return;
    }

    //等待操作完成
    uint32_t count = 0;
    while (flash_op_record[op_index].flag != 1)
    {
        nrf_delay_ms(2);
        if((count++) >= 1000) {
            SUBLE_PRINTF("Error: flash write timeout");
            err_code = NRF_ERROR_TIMEOUT;
            break;
        }
        else if(flash_op_record[op_index].flag == 0xFFFFFFFF) {
            err_code = NRF_ERROR_TIMEOUT;
            break;
        }
    }
    
    if(err_code == 0) {
//        NRFS_PRINTF("flash write success");
    }
}

/*********************************************************
FN: 
*/
void suble_flash_erase(uint32_t addr, uint32_t num)
{
    suble_wdt_feed();
    
    bool block = true;
    
    uint32_t op_index = get_current_flash_op_index();
    memset((void*)&flash_op_record[op_index], 0, sizeof(flash_op_record_t));

    ret_code_t err_code = nrf_fstorage_erase(&fstorage, addr, num, (void*)&flash_op_record[op_index]);
    if (err_code == NRF_SUCCESS) {
        if(!block) {
            SUBLE_PRINTF("flash erase success");
            return;
        }
    }
    else {
        SUBLE_PRINTF("Error: flash erase err_code %d", err_code);
        return;
    }
    
    //等待操作完成
    uint32_t count = 0;
    while(flash_op_record[op_index].flag != 1)
    {
        nrf_delay_ms(2);
        if((count++) >= 1000) {
            SUBLE_PRINTF("Error: flash erase timeout");
            err_code = NRF_ERROR_TIMEOUT;
            break;
        }
        else if(flash_op_record[op_index].flag == 0xFFFFFFFF) {
            err_code = NRF_ERROR_TIMEOUT;
            break;
        }
    }
    
    if(err_code == NRF_SUCCESS) {
        SUBLE_PRINTF("flash erase success");
    }
}




















