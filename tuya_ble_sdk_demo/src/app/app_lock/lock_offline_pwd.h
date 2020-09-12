/**
****************************************************************************
* @file      lock_offline_pwd.h
* @brief     lock_offline_pwd
* @author    suding
* @version   V1.0.0
* @date      2019-09-11
* @note
******************************************************************************
* @attention
*
* <h2><center>&copy; COPYRIGHT 2019 Tuya </center></h2>
*/


#ifndef __LOCK_OFFLINE_PWD_H__
#define __LOCK_OFFLINE_PWD_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "lock_common.h"

/*********************************************************************
 * CONSTANTS1
 */
#define OFFLINE_PWD_LEN			        (10)      //�������볤��
#define OFFLINE_PWD_MAX_NUM		        (200)     //���������������
#define OFFLINE_PWD_TIME_ACCURACY	    (3600u)   //��������ʱ�侫��-Сʱ

//ҵ����ز���
#define PWD_MAX_SN_PER_PERIOD           (10)      //������������/ÿ����
#define PWD_ACTIVE_PERIOD_SINGLE        (6*3600)  //�������뼤���     6Сʱ
#define PWD_ACTIVE_PERIOD_TIMELINESS    (24*3600) //ʱЧ���뼤���     24Сʱ
#define PWD_ACTIVE_PERIOD_CLEAR_SINGLE  (24*3600) //����������뼤��� 24Сʱ
#define PWD_ACTIVE_PERIOD_CLEAR_ALL     (24*3600) //����������뼤��� 24Сʱ

typedef enum {
	PWD_TYPE_TIMELINESS     = 0,                  //ʱЧ����
	PWD_TYPE_SINGLE         = 1,                  //��������
    PWD_TYPE_CLEAR_SINGLE   = 8,                  //�����������
	PWD_TYPE_CLEAR_ALL      = 9,                  //�����������
} enum_offline_pwd_type_t;

typedef enum {
	PWD_STATUS_UNUSED = 0,                        //δʹ��
	PWD_STATUS_INVALID,	                          //��Ч
	PWD_STATUS_VALID,		                      //��Ч
} enum_offline_pwd_status_t;

typedef enum {
	OFFLINE_PWD_VERIFY_SUCCESS       = (0),       //��֤�ɹ�
	OFFLINE_PWD_CLEAR_SINGLE_SUCCESS = (1),       //����ɹ�
	OFFLINE_PWD_CLEAR_ALL_SUCCESS    = (2),       //����ɹ�
	OFFLINE_PWD_ERR_PARAM            = (-1),      //��������
	OFFLINE_PWD_ERR_DECRYPT          = (-2),      //���ܴ���
	OFFLINE_PWD_ERR_TYPE             = (-3),      //�������ʹ���
	OFFLINE_PWD_ERR_INVALID          = (-4),      //����״̬ʧЧ
	OFFLINE_PWD_ERR_START_TIME       = (-5),      //��ǰʱ�� < ��Чʱ�䣬��û��ʼ��Ч
	OFFLINE_PWD_ERR_END_TIME         = (-6),      //��ǰʱ�� > ʧЧʱ�䣬�Ѿ�ʧЧ
	OFFLINE_PWD_ERR_ACTIVE_TIME      = (-7),      //δ�ڼ�����ڼ���
    OFFLINE_PWD_ERR_BSS_SN           = (-8),      //�����ҵ����ˮ��
	OFFLINE_PWD_ERR_NO_SPACE         = (-9),      //�޴洢�ռ�
	OFFLINE_PWD_ERR_NO_EXIST         = (-10),     //���벻����
	OFFLINE_PWD_ERR_UNKNOW           = (-11),     //δ֪����
} enum_offline_pwd_err_t;

/*********************************************************************
 * STRUCT
 */
#pragma pack(1)
typedef struct
{
	uint8_t type;			    // ������������
	uint8_t num_1_5_array[5];   // ��ʼʱ�� (T2-T0)/3600
	uint8_t num_6_9_array[4];   // ����ʱ�� (T3-T2)/3600
} lock_offline_pwd_t;

typedef struct
{
    uint8_t  status;
    uint8_t  type;
    uint32_t pwd;
} lock_offline_pwd_storage_t;

typedef struct
{
    bool found;
    uint32_t min_T;
    int pwdid;
} lock_offline_pwd_find_info_t;
#pragma pack()

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */
uint32_t lock_offline_pwd_delete(int32_t pwdid);
uint32_t lock_offline_pwd_delete_all(void);
void     lock_offline_pwd_set_T0(uint32_t T0_tmp);
uint32_t lock_offline_pwd_get_T0(void);
int32_t  lock_offline_pwd_verify(uint8_t *key, uint8_t key_len,
                                    uint8_t *encrypt_pwd, uint8_t encrypt_pwd_len,
                                    uint32_t timestamp,
                                    uint8_t *plain_pwd, uint8_t *p_plain_pwd_len);

#ifdef __cplusplus
}
#endif

#endif //__LOCK_OFFLINE_PWD_H__
