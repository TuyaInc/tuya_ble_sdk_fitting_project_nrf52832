/**
****************************************************************************
* @file      lock_dp_parser.h
* @brief     lock_dp_parser
* @author    suding
* @version   V1.0.0
* @date      2019-09-11
* @note
******************************************************************************
* @attention
*
* <h2><center>&copy; COPYRIGHT 2019 Tuya </center></h2>
*/


#ifndef __LOCK_DP_PARSER_H__
#define __LOCK_DP_PARSER_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "lock_common.h"

/*********************************************************************
 * CONSTANTS
 */
//WR-write_report, OW-only_write, OR-only_report
//BSC-basic, STS-state_sync, SET-setting, RFU-remain_for_future_use
//PW-password, BT-blue_tooth
#define  WR_BSC_OPEN_METH_CREATE                  1  //��������-��ӿ�����ʽ
#define  WR_BSC_OPEN_METH_DELETE                  2  //��������-ɾ��������ʽ
#define  WR_BSC_OPEN_METH_MODIFY                  3  //��������-�޸Ŀ�����ʽ
#define  WR_BSC_TEMP_PW_CREAT                     4  //��������-�����ʱ����
#define  WR_BSC_TEMP_PW_DELETE                    5  //��������-ɾ����ʱ����
#define  WR_BSC_TEMP_PW_MODIFY                    6  //��������-�޸���ʱ����
#define  WR_BSC_OPEN_METH_SYNC_NEW                7  //��������-ͬ��������ʽ����������
#define  WR_BSC_GUIDE_PAGE                        8  //��������-��������

#define  WR_SET_LOCK_BELL                         20  //��������-�����Ч
#define  WR_SET_LOCK_VOLUME                       21  //��������-�������
#define  WR_SET_KEY_VOLUME                        22  //��������-��������
#define  WR_SET_NAVIGATE_VOLUME                   23  //��������-���ı��ص�������
#define  WR_SET_LOCK_LANGUAGE                     24  //��������-���������л�
#define  WR_SET_WELCOME_WORDS                     25  //��������-��ʾ����ӭ��
#define  WR_SET_COMBINE_UNLOCK                    26  //��������-��һ��������Ͽ����л�
#define  WR_SET_LOCK_CHECK_SWITCH                 27  //��������-����У�鿪��
#define  WR_SET_SINGLE_MULTI_SWITCH               28  //��������-�����࿪����
#define  WR_SET_SPECIAL_FUNCTION                  29  //��������-���⹦��

#define  OR_STS_BATTERY_PERCENT                   40  //״̬ͬ��-�����ٷֱ�
#define  OR_STS_BATTERY_POSITION                  41  //״̬ͬ��-������λ

#define  OR_LOG_OPEN_WITH_KEY                     50  //������¼-����
#define  OR_LOG_OPEN_WITH_PW                      51  //������¼-����
#define  OR_LOG_OPEN_WITH_CARD                    52  //������¼-�ſ�
#define  OR_LOG_OPEN_WITH_FINGER                  53  //������¼-ָ��
#define  OR_LOG_OPEN_WITH_FACE                    54  //������¼-����
#define  OR_LOG_OPEN_WITH_EYE                     55  //������¼-��Ĥ
#define  OR_LOG_OPEN_WITH_PALM_PRINT              56  //������¼-����
#define  OR_LOG_OPEN_WITH_FINGER_VEIN             57  //������¼-ָ����
#define  OR_LOG_OPEN_WITH_TMP_PWD                 58  //������¼-��ʱ����
#define  OR_LOG_OPEN_WITH_COMBINE                 59  //������¼-��Ͽ���
#define  OR_LOG_ALARM_REASON                      60  //������¼-����
#define  OR_LOG_COMMON_REPORT                     69  //������¼-�����¼�ϱ�

#define  WR_BSC_OFFLINE_PW_SET_T0                 70  //��������-��������-T0ʱ���·�
#define  OR_LOG_OFFLINE_PW_CLEAR_SINGLE_ALARM     71  //������¼-��������-�����������
#define  OR_LOG_OFFLINE_PW_CLEAR_ALL_ALARM        72  //������¼-��������-�����������
#define  OR_LOG_OFFLINE_PW_OPEN_WITH              73  //������¼-��������
#define  OR_LOG_DYNAMIC_PW_OPEN_WITH              74  //������¼-��̬����


//open method
typedef enum
{
    OPEN_METH_BASE = 0,
    OPEN_METH_PASSWORD,
    OPEN_METH_DOORCARD,
    OPEN_METH_FINGER,
    OPEN_METH_FACE,
    OPEN_METH_TEMP_PW, //ʵ�ʵ���ʱ����ֵΪ0xF0���˴�Ϊ����洢��ʾ��ȡֵ0x05
    OPEN_METH_MAX,
} open_meth_t;

//reg state
typedef enum
{
    REG_STAGE_STSRT    = 0x00,
    REG_STAGE_RUNNING  = 0xFC,
    REG_STAGE_FAILED   = 0xFD,
    REG_STAGE_CANCEL   = 0xFE,
    REG_STAGE_COMPLETE = 0xFF,
} reg_stage_t;

//reg failed reason
typedef enum
{
    REG_NOUSE_DEFAULT_VALUE  = 0x00,
    REG_FAILD_OUTTIME  = 0x00,
    REG_FAILD_FAILED   = 0x01,
    REG_FAILD_REPEAT   = 0x02,
    REG_FAILD_NO_HARDID  = 0x03,
    REG_FAILD_INVALID_PW  = 0x04,
    REG_FAILD_INVALID_PW_LEN  = 0x05,
    REG_FAILD_INVALID_OPEN_METH  = 0x06,
    REG_FAILD_FINGER_RUNNING  = 0x07,
    REG_FAILD_CARD_RUNNING  = 0x08,
    REG_FAILD_FACE_RUNNING  = 0x08,
    REG_FAILD_WONG_NUMBER  = 0xFE,
} reg_failed_reason_t;

//reg abnormal reason
typedef enum
{
    REG_ABNORMAL_NONE = 0x00,
    REG_ABNORMAL_FP_INCOMPLETE = 0x01,
} reg_abnormal_reason_t;

//alarm reason
typedef enum
{
    ALARM_WRONG_FINGER = 0,
    ALARM_WRONG_PASSWORD,
    ALARM_WRONG_CARD,
    ALARM_WRONG_FACE,
    ALARM_TONGUE_BAD,
    ALARM_TOO_HOT,          //0x05
    ALARM_UNLOCKED_OUTTIME,
    ALARM_TONGUE_NOT_OUT,
    ALARM_PRY,
    ALARM_KEY_IN,
    ALARM_LOW_BATTERY,      //0x0A
    ALARM_POWER_OFF,
    ALARM_SHOCK,
} alarm_reason_t;

//
typedef enum
{
    FREEZE_ON = 0,
    FREEZE_OFF = 1,
} lock_freeze_t;

#define OPEN_WITH_NOPWD_REMOTE_KEY "nopwd_remote"

/*********************************************************************
 * STRUCT
 */
#pragma pack(1)
//create open method
typedef struct
{
    uint16_t slaveid;
    open_meth_t meth;
    reg_stage_t stage;
    uint8_t admin_falg;
    uint8_t memberid;
    uint8_t hardid;
    uint8_t time[17];
    uint8_t valid_num;
    uint8_t password_len;
    uint8_t password[10];
} open_meth_creat_t;
typedef struct
{
    uint16_t slaveid;
    open_meth_t meth;
    reg_stage_t stage;
    uint8_t admin_falg;
    uint8_t memberid;
    uint8_t hardid;
    uint8_t reg_num;
    uint8_t result;
} open_meth_creat_result_t;

//delete open method
typedef struct
{
    uint16_t slaveid;
    open_meth_t meth;   
    reg_stage_t stage;  
    uint8_t admin_falg; 
    uint8_t memberid;   
    uint8_t hardid;     
    uint8_t delete_style;
} open_meth_delete_t;
typedef struct
{
    uint16_t slaveid;
    open_meth_t meth;   
    reg_stage_t stage;  
    uint8_t admin_falg; 
    uint8_t memberid;   
    uint8_t hardid;     
    uint8_t delete_style;
    uint8_t result;     
} open_meth_delete_result_t;

//modify open method
typedef struct
{
    uint16_t slaveid;
    open_meth_t meth;    
    reg_stage_t stage;   
    uint8_t admin_falg;  
    uint8_t memberid;    
    uint8_t hardid;      
    uint8_t time[17];    
    uint8_t cycle;       
    uint8_t password_len;
    uint8_t password[10];
} open_meth_modify_t;
typedef struct
{
    uint16_t slaveid;
    open_meth_t meth;
    reg_stage_t stage;
    uint8_t amin_falg;
    uint8_t memberid; 
    uint8_t hardid;   
    uint8_t cycle;    
    uint8_t result;   
} open_meth_modify_result_t;

//create temp pw
typedef struct
{
    uint16_t slaveid;
    uint8_t type;
    uint8_t time[17];
    uint8_t valid_num;
    uint8_t password_len;
    uint8_t password[10];
} temp_pw_creat_t;
typedef struct
{
    uint16_t slaveid;
    uint8_t hardid;
    uint8_t result;
} temp_pw_creat_result_t;

//delete temp pw
typedef struct
{
    uint16_t slaveid;
    uint8_t hardid;
} temp_pw_delete_t;
typedef struct
{
    uint16_t slaveid;
    uint8_t hardid;
    uint8_t result;
} temp_pw_delete_result_t;

//modify temp pw
typedef struct
{
    uint16_t slaveid;
    uint8_t hardid;
    uint8_t type;
    uint8_t time[17];
    uint8_t valid_num;
    uint8_t password_len;
    uint8_t password[10];
} temp_pw_modify_t;
typedef struct
{
    uint16_t slaveid;
    uint8_t hardid;
    uint8_t result;
} temp_pw_modify_result_t;

//sync open method new
typedef struct
{
    uint8_t memberid;
    uint8_t freeze_state;
} open_meth_sync_hard_attribute_t;
typedef struct
{
    uint8_t hardid;
    uint8_t hard_type;
    open_meth_sync_hard_attribute_t hard_attribute;
} open_meth_sync_node_new_t;
typedef struct
{
    uint8_t stage;
    uint8_t pkgs;
} open_meth_sync_new_last_result_t;
typedef struct
{
    uint8_t type;
    uint8_t idx;
    uint8_t count;
} open_meth_sync_new_hard_t;
typedef struct
{
    uint8_t flag;
    uint8_t hard_type_len;
    uint8_t idx;
    uint8_t pkg_count;
    open_meth_sync_new_hard_t hard[OPEN_METH_MAX];
} open_meth_sync_new_t;

//guide page
typedef struct
{
    uint8_t func;
} guide_page_t;
typedef struct
{
    uint8_t func;
    uint8_t result;
} guide_page_result_t;


//dp point
typedef struct
{
    uint8_t dp_id;
    uint8_t dp_type;
    uint8_t dp_data_len;
    union
    {
        uint8_t dp_data[256];
        open_meth_creat_t open_meth_creat;
        open_meth_creat_result_t open_meth_creat_result;
        open_meth_modify_t open_meth_modify;
        open_meth_modify_result_t open_meth_modify_result;
        open_meth_delete_t open_meth_delete;
        open_meth_delete_result_t open_meth_delete_result;
        temp_pw_creat_t temp_pw_creat;
        temp_pw_creat_result_t temp_pw_creat_result;
        temp_pw_delete_t temp_pw_delete;
        temp_pw_delete_result_t temp_pw_delete_result;
        temp_pw_modify_t temp_pw_modify;
        temp_pw_modify_result_t temp_pw_modify_result;
    };
} lock_dp_t;
#pragma pack()

/*********************************************************************
 * EXTERNAL VARIABLES
 */
extern lock_dp_t g_cmd;
extern lock_dp_t g_rsp;
extern volatile open_meth_sync_new_t g_sync_new;

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */
uint32_t lock_dp_parser_handler(void* dp_data);


#ifdef __cplusplus
}
#endif

#endif //__LOCK_DP_PARSER_H__
