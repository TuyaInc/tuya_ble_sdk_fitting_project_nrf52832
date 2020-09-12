/**
****************************************************************************
* @file      lock_common.h
* @brief     lock_common
* @author    suding
* @version   V1.0.0
* @date      2019-09-11
* @note
******************************************************************************
* @attention
*
* <h2><center>&copy; COPYRIGHT 2019 Tuya </center></h2>
*/


#ifndef __LOCK_COMMON_H__
#define __LOCK_COMMON_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "app_common.h"

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
void lock_common_init(void);
void lock_factory_handler(void);


#ifdef __cplusplus
}
#endif

#endif //__LOCK_COMMON_H__
