/**
****************************************************************************
* @file      app_common.h
* @brief     app_common
* @author    suding
* @version   V1.0.0
* @date      2019-09-11
* @note
******************************************************************************
* @attention
*
* <h2><center>&copy; COPYRIGHT 2019 Tuya </center></h2>
*/


#ifndef __APP_COMMON_H__
#define __APP_COMMON_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
//c_lib
#include "stdio.h"
#include "stdlib.h"
#include "stdint.h"
#include "string.h"
#include "math.h"
//app common
#include "app_port.h"
#include "app_flash.h"
#include "app_ota.h"
#include "app_active_report.h"
#include "app_test.h"
//app lock
#include "lock_dp_parser.h"
#include "lock_dp_report.h"
#include "lock_timer.h"
#include "lock_hard.h"
#include "lock_dynamic_pwd.h"
#include "lock_offline_pwd.h"
#include "lock_test.h"

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * STRUCT
 */

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */
void app_common_init(void);


#ifdef __cplusplus
}
#endif

#endif //__APP_COMMON_H__
