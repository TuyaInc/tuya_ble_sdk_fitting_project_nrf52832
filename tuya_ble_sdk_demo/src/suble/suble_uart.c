#include "suble_common.h"




/*********************************************************************
 * LOCAL CONSTANT
 */
#define SUBLE_UART_TX_BUF_SIZE                512
#define SUBLE_UART_RX_BUF_SIZE                512

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
static void uart1_rx_handler(app_uart_evt_t * p_event);
//static void uart2_rx_handler(app_uart_evt_t * p_event);




/*********************************************************
FN: 
*/
static void uart1_rx_handler(app_uart_evt_t * p_event)
{
    uint8_t rx_char;

    switch (p_event->evt_type)
    {
        case APP_UART_DATA_READY: {
            UNUSED_VARIABLE(app_uart_get(&rx_char));
            
//            suble_uart1_send(&rx_char, 1);
#ifdef TUYA_BLE_SDK_IN_SUBLE
            tuya_ble_common_uart_receive_data(&rx_char, 1);
#endif
        } break;

        case APP_UART_TX_EMPTY: {
            SUBLE_PRINTF("uart tx finish");
        } break;

        case APP_UART_COMMUNICATION_ERROR: {
            app_uart_flush();
            app_uart_close();
            suble_uart1_init();
//            APP_ERROR_HANDLER(p_event->data.error_communication);
        } break;

        case APP_UART_FIFO_ERROR: {
//            APP_ERROR_HANDLER(p_event->data.error_code);
        } break;

        default: {
        } break;
    }
}

/*********************************************************
FN: 
*/
//static void uart2_rx_handler(uint8_t *buf, uint8_t len)
//{
//    //
//}

/*********************************************************
FN: 
*/
void suble_uart1_init(void)
{
    ret_code_t err_code;
    app_uart_comm_params_t const comm_params =
    {
        .rx_pin_no    = RX_PIN_NUMBER,
        .tx_pin_no    = TX_PIN_NUMBER,
        .rts_pin_no   = RTS_PIN_NUMBER,
        .cts_pin_no   = CTS_PIN_NUMBER,
        .flow_control = APP_UART_FLOW_CONTROL_DISABLED,
        .use_parity   = false,
        .baud_rate    = NRF_UART_BAUDRATE_115200
    };

    APP_UART_FIFO_INIT(&comm_params,
                       SUBLE_UART_RX_BUF_SIZE,
                       SUBLE_UART_TX_BUF_SIZE,
                       uart1_rx_handler,
                       APP_IRQ_PRIORITY_LOWEST,
                       err_code);
    APP_ERROR_CHECK(err_code);
}

/*********************************************************
FN: 
*/
void suble_uart2_init(void)
{
	//
}

/*********************************************************
FN: 
*/
void suble_uart1_send(const uint8_t* buf, uint32_t size)
{
    SUBLE_HEXDUMP("TX", (void*)buf, size);
    ret_code_t err_code;
    for(uint32_t idx=0; idx<size; idx++)
    {
        err_code = app_uart_put(buf[idx]);
        APP_ERROR_CHECK(err_code);
        if(err_code != NRF_SUCCESS)
        {
            break;
        }
    }
}

/*********************************************************
FN: 
*/
void suble_uart2_send(const uint8_t* buf, uint32_t size)
{
    //
}

















