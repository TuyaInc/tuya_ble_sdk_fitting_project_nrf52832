#include "lock_common.h"




/*********************************************************************
 * LOCAL CONSTANTS
 */

/*********************************************************************
 * LOCAL STRUCT
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

/*********************************************************************
 * LOCAL FUNCTION
 */

/*********************************************************************
 * VARIABLES
 */




/*********************************************************
FN: 
*/
void lock_common_init(void)
{
    app_ota_init();
    
    app_port_local_clock_start();
    
    app_port_nv_init();
    
    app_common_init();
}

/*********************************************************
FN: 
*/
void lock_factory_handler(void)
{
    app_port_tuya_ble_device_factory_reset();
    
    //erase all lock falsh
    lock_flash_erease_all();
}















