#include "lock_dp_parser.h"




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
static uint32_t open_meth_creat_handler(void* cmd_dp_data, void* rsp_dp_data, uint8_t* rsp_dp_data_len);
static uint32_t open_meth_delete_handler(void* cmd_dp_data, void* rsp_dp_data, uint8_t* rsp_dp_data_len);
static uint32_t open_meth_modify_handler(void* cmd_dp_data, void* rsp_dp_data, uint8_t* rsp_dp_data_len);
static uint32_t temp_pw_creat_handler(void* cmd_dp_data, void* rsp_dp_data, uint8_t* rsp_dp_data_len);
static uint32_t temp_pw_delete_handler(void* cmd_dp_data, void* rsp_dp_data, uint8_t* rsp_dp_data_len);
static uint32_t temp_pw_modify_handler(void* cmd_dp_data, void* rsp_dp_data, uint8_t* rsp_dp_data_len);
static uint32_t open_meth_sync_new_handler(uint8_t cmd_dp_data_len, void* cmd_dp_data, void* rsp_dp_data, uint8_t* rsp_dp_data_len);
static uint32_t guide_page_handler(void* cmd_dp_data, void* rsp_dp_data, uint8_t* rsp_dp_data_len);
static uint32_t offline_pwd_set_T0_handler(void* cmd_dp_data, void* rsp_dp_data, uint8_t* rsp_dp_data_len);

/*********************************************************************
 * VARIABLES
 */
lock_dp_t g_cmd;
lock_dp_t g_rsp;




/*********************************************************
FN: 
*/
uint32_t lock_dp_parser_handler(void* dp_data)
{
    uint8_t rsp_flag = 1;
    
    //init cmd and rsp
    memcpy(&g_cmd, dp_data, sizeof(lock_dp_t));
    memcpy(&g_rsp, dp_data, sizeof(lock_dp_t));
    TUYA_APP_LOG_HEXDUMP_INFO("dp_cmd", (void*)&g_cmd, g_cmd.dp_data_len+3);
    
    switch(g_cmd.dp_id)
    {
        case WR_BSC_OPEN_METH_CREATE: {
            open_meth_creat_handler(g_cmd.dp_data, g_rsp.dp_data, &g_rsp.dp_data_len);
        } break;
        
        case WR_BSC_OPEN_METH_DELETE: {
            open_meth_delete_handler(g_cmd.dp_data, g_rsp.dp_data, &g_rsp.dp_data_len);
        } break;
        
        case WR_BSC_OPEN_METH_MODIFY: {
            open_meth_modify_handler(g_cmd.dp_data, g_rsp.dp_data, &g_rsp.dp_data_len);
        } break;
        
        case WR_BSC_TEMP_PW_CREAT: {
            temp_pw_creat_handler(g_cmd.dp_data, g_rsp.dp_data, &g_rsp.dp_data_len);
        } break;
        
        case WR_BSC_TEMP_PW_DELETE: {
            temp_pw_delete_handler(g_cmd.dp_data, g_rsp.dp_data, &g_rsp.dp_data_len);
        } break;
        
        case WR_BSC_TEMP_PW_MODIFY: {
            temp_pw_modify_handler(g_cmd.dp_data, g_rsp.dp_data, &g_rsp.dp_data_len);
        } break;
        
        case WR_BSC_OPEN_METH_SYNC_NEW: {
            TUYA_APP_LOG_INFO("OPEN_METH sync new start");
            open_meth_sync_new_handler(g_cmd.dp_data_len, g_cmd.dp_data, g_rsp.dp_data, &g_rsp.dp_data_len);
        } break;
        
        case WR_BSC_GUIDE_PAGE: {
            guide_page_handler(g_cmd.dp_data, g_rsp.dp_data, &g_rsp.dp_data_len);
        } break;
        
        
        
        
        case WR_SET_LOCK_BELL: {
            if((g_cmd.dp_data_len == 1) && (g_cmd.dp_data[0] <= 0x0A))
            {
                lock_settings.lock_bell = g_cmd.dp_data[0];
                if(lock_settings_save() == APP_PORT_SUCCESS) {
                    TUYA_APP_LOG_INFO("WR_SET_LOCK_BELL SUCCESS");
                }
            }
        } break;
        
        case WR_SET_LOCK_VOLUME: {
            if((g_cmd.dp_data_len == 1) && (g_cmd.dp_data[0] <= 0x03))
            {
                lock_settings.lock_volume = g_cmd.dp_data[0];
                if(lock_settings_save() == APP_PORT_SUCCESS) {
                    TUYA_APP_LOG_INFO("WR_SET_LOCK_VOLUME SUCCESS");
                }
            }
        } break;
        
        case WR_SET_KEY_VOLUME: {
            if((g_cmd.dp_data_len == 1) && (g_cmd.dp_data[0] <= 0x03))
            {
                lock_settings.key_volume = g_cmd.dp_data[0];
                if(lock_settings_save() == APP_PORT_SUCCESS) {
                    TUYA_APP_LOG_INFO("WR_SET_KEY_VOLUME SUCCESS");
                }
            }
        } break;
        
        case WR_SET_NAVIGATE_VOLUME: {
            if((g_cmd.dp_data_len == 1) && (g_cmd.dp_data[0] <= 0x03))
            {
                lock_settings.navigation_volume = g_cmd.dp_data[0];
                if(lock_settings_save() == APP_PORT_SUCCESS) {
                    TUYA_APP_LOG_INFO("WR_SET_NAVIGATE_VOLUME SUCCESS");
                }
            }
        } break;
        
        case WR_SET_LOCK_LANGUAGE: {
            if((g_cmd.dp_data_len == 1) && (g_cmd.dp_data[0] <= 0x0A))
            {
                lock_settings.lock_language = g_cmd.dp_data[0];
                if(lock_settings_save() == APP_PORT_SUCCESS) {
                    TUYA_APP_LOG_INFO("WR_SET_LOCK_LANGUAGE SUCCESS");
                }
            }
        } break;
        
        case WR_SET_WELCOME_WORDS: {
            if((g_cmd.dp_data_len > 0) && (g_cmd.dp_data_len <= HARD_WELCOME_WORDS_MAX_LEN))
            {
                memcpy(lock_settings.welcome_words, g_cmd.dp_data, g_cmd.dp_data_len);
                if(lock_settings_save() == APP_PORT_SUCCESS) {
                    TUYA_APP_LOG_INFO("WR_SET_WELCOME_WORDS SUCCESS");
                }
            }
        } break;
        
        case WR_SET_COMBINE_UNLOCK: {
            if((g_cmd.dp_data_len == 1) && (g_cmd.dp_data[0] <= 0x06))
            {
                lock_settings.combine_lock_switch = g_cmd.dp_data[0];
                if(lock_settings_save() == APP_PORT_SUCCESS) {
                    TUYA_APP_LOG_INFO("WR_SET_COMBINE_UNLOCK SUCCESS");
                }
            }
        } break;
        
        case WR_SET_LOCK_CHECK_SWITCH: {
            if((g_cmd.dp_data_len == 1) && (g_cmd.dp_data[0] <= 0x01))
            {
                lock_settings.lock_check_switch = g_cmd.dp_data[0];
                if(lock_settings_save() == APP_PORT_SUCCESS) {
                    TUYA_APP_LOG_INFO("WR_SET_LOCK_CHECK_SWITCH SUCCESS");
                }
            }
        } break;
        
        case WR_SET_SINGLE_MULTI_SWITCH: {
            if((g_cmd.dp_data_len == 1) && (g_cmd.dp_data[0] <= 0x01))
            {
                lock_settings.single_multi_switch = g_cmd.dp_data[0];
                if(lock_settings_save() == APP_PORT_SUCCESS) {
                    TUYA_APP_LOG_INFO("WR_SET_SINGLE_MULTI_SWITCH SUCCESS");
                }
            }
        } break;
        
        case WR_SET_SPECIAL_FUNCTION: {
            if(g_cmd.dp_data_len == 1)
            {
                lock_settings.special_function = g_cmd.dp_data[0];
                if(lock_settings_save() == APP_PORT_SUCCESS) {
                    TUYA_APP_LOG_INFO("WR_SET_SPECIAL_FUNCTION SUCCESS");
                }
            }
        } break;
        
        
        
        
        case WR_BSC_OFFLINE_PW_SET_T0: {
            offline_pwd_set_T0_handler(g_cmd.dp_data, g_rsp.dp_data, &g_rsp.dp_data_len);
        } break;
        
        default: {
            rsp_flag = 0;
        } break;
    }
    
    if(rsp_flag && (g_rsp.dp_data_len > 0))
    {
        app_port_dp_data_report((void*)&g_rsp, (3 + g_rsp.dp_data_len));
    }

    return 0;
}

/*********************************************************
FN: create open method
*/
static uint32_t open_meth_creat_handler(void* cmd_dp_data, void* rsp_dp_data, uint8_t* rsp_dp_data_len)
{
    open_meth_creat_t* cmd = cmd_dp_data;
    open_meth_creat_result_t* rsp = rsp_dp_data;
    uint8_t* rsp_len = rsp_dp_data_len;
    
    //调整主机id和从机id位置
    app_port_reverse_byte(&cmd->slaveid, 2);
    rsp->slaveid = cmd->slaveid;
    app_port_reverse_byte(&rsp->slaveid, 2);
    
    switch(cmd->meth)
    {
        case OPEN_METH_PASSWORD: {
            if(cmd->stage == REG_STAGE_STSRT)
            {
                if(lock_get_hardid(cmd->meth) != HARD_ID_INVALID)
                {
                    TUYA_APP_LOG_INFO("OPEN_METH_PASSWORD creat start");
                    if(g_auto_switch.creat_pw_flag == 0) {
                        lock_hard_creat_sub_report_with_delay(cmd->meth, REG_STAGE_COMPLETE, lock_get_hardid(cmd->meth), REG_NOUSE_DEFAULT_VALUE, REG_NOUSE_DEFAULT_VALUE);
                        lock_hard_save_in_local_flash(cmd->meth);
                    }
                }
                else
                {//hardid is used up
                    TUYA_APP_LOG_INFO("OPEN_METH_PASSWORD hardid is used up");
                    lock_hard_creat_sub_report_with_delay(cmd->meth, REG_STAGE_FAILED, lock_get_hardid(cmd->meth), REG_STAGE_STSRT, REG_FAILD_NO_HARDID);
                }
                rsp->reg_num = 0x00; //password doesn't need this
            }
            else if(cmd->stage == REG_STAGE_CANCEL)
            {
                TUYA_APP_LOG_INFO("OPEN_METH_PASSWORD REG_STAGE_CANCEL");
//                tuya_ble_app_evt_send(APP_EVT_CANCEL_REG_CARD);
                rsp->reg_num = REG_NOUSE_DEFAULT_VALUE;
            }
            rsp->result = REG_NOUSE_DEFAULT_VALUE; //default, no sence
        } break;
        
        case OPEN_METH_DOORCARD: {
            if(cmd->stage == REG_STAGE_STSRT)
            {
                if(lock_get_hardid(cmd->meth) != HARD_ID_INVALID)
                {
                    TUYA_APP_LOG_INFO("OPEN_METH_DOORCARD creat start");
                    tuya_ble_app_evt_send_with_data(APP_EVT_START_REG_CARD, cmd, sizeof(open_meth_creat_t));
                }
                else
                {//hardid is used up
                    TUYA_APP_LOG_INFO("OPEN_METH_DOORCARD hardid is used up");
                    lock_hard_creat_sub_report_with_delay(cmd->meth, REG_STAGE_FAILED, lock_get_hardid(cmd->meth), REG_STAGE_STSRT, REG_FAILD_NO_HARDID);
                }

                rsp->reg_num = 0x01; //doorcard only need 1 time
            }
            else if(cmd->stage == REG_STAGE_CANCEL)
            {
                TUYA_APP_LOG_INFO("OPEN_METH_DOORCARD REG_STAGE_CANCEL");
                tuya_ble_app_evt_send(APP_EVT_CANCEL_REG_CARD);
                rsp->reg_num = REG_NOUSE_DEFAULT_VALUE;
            }
            rsp->result = REG_NOUSE_DEFAULT_VALUE;
        } break;
        
        default: {
        } break;
    }
    
    *rsp_len = sizeof(open_meth_creat_result_t);
    
    return APP_PORT_SUCCESS;
}

/*********************************************************
FN: delete open method
*/
static uint32_t open_meth_delete_handler(void* cmd_dp_data, void* rsp_dp_data, uint8_t* rsp_dp_data_len)
{
    open_meth_delete_t* cmd = cmd_dp_data;
    open_meth_delete_result_t* rsp = rsp_dp_data;
    uint8_t* rsp_len = rsp_dp_data_len;
    uint8_t ret = APP_PORT_SUCCESS;
    
    //调整主机id和从机id位置
    app_port_reverse_byte(&cmd->slaveid, 2);
    rsp->slaveid = cmd->slaveid;
    app_port_reverse_byte(&rsp->slaveid, 2);
    
    switch(cmd->meth)
    {
        case OPEN_METH_BASE: {
            ret += lock_hard_delete_all_by_memberid(cmd->memberid);
            if(ret == APP_PORT_SUCCESS) {
                TUYA_APP_LOG_INFO("OPEN_METH_BASE delete start"); }
        } break;
        
        case OPEN_METH_PASSWORD: {
            ret += lock_hard_delete(cmd->hardid);
            if(ret == APP_PORT_SUCCESS) {
                TUYA_APP_LOG_INFO("OPEN_METH_PASSWORD delete start"); }
        } break;
        
        case OPEN_METH_DOORCARD: {
            ret += lock_hard_delete(cmd->hardid);
            ret += lock_hard_doorcard_delete(cmd->hardid);
            if(ret == APP_PORT_SUCCESS) {
                TUYA_APP_LOG_INFO("OPEN_METH_DOORCARD delete start"); }
        } break;
        
        default: {
        } break;
    }
    
    if(ret == APP_PORT_SUCCESS) {
        rsp->result = 0xFF; //delete success
    } else {
        rsp->result = 0x00; //delete fail
    }
    *rsp_len = sizeof(open_meth_delete_result_t);
    
    return APP_PORT_SUCCESS;
}

/*********************************************************
FN: modify open method
*/
static uint32_t open_meth_modify_handler(void* cmd_dp_data, void* rsp_dp_data, uint8_t* rsp_dp_data_len)
{
    open_meth_modify_t* cmd = cmd_dp_data;
    open_meth_modify_result_t* rsp = rsp_dp_data;
    uint8_t* rsp_len = rsp_dp_data_len;
    uint8_t ret = APP_PORT_SUCCESS;
    
    //调整主机id和从机id位置
    app_port_reverse_byte(&cmd->slaveid, 2);
    rsp->slaveid = cmd->slaveid;
    app_port_reverse_byte(&rsp->slaveid, 2);
    
    switch(cmd->meth)
    {
        case OPEN_METH_BASE: {
            ret = lock_hard_modify_all_by_memberid(cmd->memberid, cmd->time);
            if(ret == APP_PORT_SUCCESS) {
                TUYA_APP_LOG_INFO("OPEN_METH_BASE modify start"); }
        } break;
        
        case OPEN_METH_PASSWORD: {
            ret = lock_hard_modify_in_local_flash(OPEN_METH_PASSWORD);
            if(ret == APP_PORT_SUCCESS) {
                TUYA_APP_LOG_INFO("OPEN_METH_PASSWORD modify start"); }
        } break;
        
        case OPEN_METH_DOORCARD: {
            ret = lock_hard_modify_in_local_flash(OPEN_METH_PASSWORD);
            if(ret == APP_PORT_SUCCESS) {
                TUYA_APP_LOG_INFO("OPEN_METH_DOORCARD modify start"); }
        } break;
        
        default: {
        } break;
    }
    
    rsp->cycle = cmd->cycle;
    if(ret == APP_PORT_SUCCESS) {
        rsp->result = 0xFF; //modify success
    } else {
        rsp->result = 0x00; //modify fail
    }
    *rsp_len = sizeof(open_meth_modify_result_t);
    
    return APP_PORT_SUCCESS;
}

/*********************************************************
FN: creat temp password
*/
static uint32_t temp_pw_creat_handler(void* cmd_dp_data, void* rsp_dp_data, uint8_t* rsp_dp_data_len)
{
    temp_pw_creat_t* cmd = cmd_dp_data;
    temp_pw_creat_result_t* rsp = rsp_dp_data;
    uint8_t* rsp_len = rsp_dp_data_len;
    uint8_t ret = APP_PORT_SUCCESS;
    
    //调整主机id和从机id位置
    app_port_reverse_byte(&cmd->slaveid, 2);
    rsp->slaveid = cmd->slaveid;
    app_port_reverse_byte(&rsp->slaveid, 2);
    
    rsp->hardid = lock_get_hardid(OPEN_METH_TEMP_PW);
    if(lock_get_hardid(OPEN_METH_TEMP_PW) != HARD_ID_INVALID)
    {
        TUYA_APP_LOG_INFO("OPEN_METH_TEMP_PW creat start");
        ret += lock_hard_save_in_local_flash(OPEN_METH_TEMP_PW);
        
        switch(cmd->type)
        {
            case 0: {
            } break;
            
            case 1: {
            } break;
            
            default: {
            } break;
        }
        
        if(ret == APP_PORT_SUCCESS) {
            rsp->result = 0x00;
        } else {
            rsp->result = 0x01;
        }
    }
    else
    {//hardid is used up
        TUYA_APP_LOG_INFO("OPEN_METH_TEMP_PW hardid is used up");
        rsp->result = 0x02;
    }
    
    *rsp_len = sizeof(temp_pw_creat_result_t);
    
    return APP_PORT_SUCCESS;
}

/*********************************************************
FN: delete temp password
*/
static uint32_t temp_pw_delete_handler(void* cmd_dp_data, void* rsp_dp_data, uint8_t* rsp_dp_data_len)
{
    temp_pw_delete_t* cmd = cmd_dp_data;
    temp_pw_delete_result_t* rsp = rsp_dp_data;
    uint8_t* rsp_len = rsp_dp_data_len;
    uint8_t ret = APP_PORT_SUCCESS;
    
    //调整主机id和从机id位置
    app_port_reverse_byte(&cmd->slaveid, 2);
    rsp->slaveid = cmd->slaveid;
    app_port_reverse_byte(&rsp->slaveid, 2);
    
    rsp->hardid = cmd->hardid;
    ret = lock_hard_delete(cmd->hardid);
    
    if(ret == APP_PORT_SUCCESS) {
        TUYA_APP_LOG_INFO("OPEN_METH_TEMP_PW delete success");
        rsp->result = 0x00;
    } else {
        TUYA_APP_LOG_INFO("OPEN_METH_TEMP_PW delete fail");
        rsp->result = 0x01;
    }
    *rsp_len = sizeof(temp_pw_delete_result_t);
    
    return APP_PORT_SUCCESS;
}

/*********************************************************
FN: modify temp password
*/
static uint32_t temp_pw_modify_handler(void* cmd_dp_data, void* rsp_dp_data, uint8_t* rsp_dp_data_len)
{
    temp_pw_modify_t* cmd = cmd_dp_data;
    temp_pw_modify_result_t* rsp = rsp_dp_data;
    uint8_t* rsp_len = rsp_dp_data_len;
    uint8_t ret = APP_PORT_SUCCESS;
    
    //调整主机id和从机id位置
    app_port_reverse_byte(&cmd->slaveid, 2);
    rsp->slaveid = cmd->slaveid;
    app_port_reverse_byte(&rsp->slaveid, 2);
    
    rsp->hardid = cmd->hardid;
    ret = lock_hard_modify_in_local_flash(OPEN_METH_TEMP_PW);
    
    switch(cmd->type)
    {
        case 0: {
        } break;
        
        case 1: {
        } break;
        
        default: {
        } break;
    }

    if(ret == APP_PORT_SUCCESS) {
        TUYA_APP_LOG_INFO("OPEN_METH_TEMP_PW modify success");
        rsp->result = 0x00;
    } else {
        TUYA_APP_LOG_INFO("OPEN_METH_TEMP_PW modify fail");
        rsp->result = 0x01;
    }
    *rsp_len = sizeof(temp_pw_modify_result_t);

    return APP_PORT_SUCCESS;
}

/*********************************************************
FN: sync open method new
*/
static uint32_t open_meth_sync_new_handler(uint8_t cmd_dp_data_len, void* cmd_dp_data, void* rsp_dp_data, uint8_t* rsp_dp_data_len)
{
    open_meth_sync_new_last_result_t* rsp = rsp_dp_data;
    uint8_t* rsp_len = rsp_dp_data_len;
    uint8_t ret = APP_PORT_SUCCESS;
    
    //检查硬件类型枚举的长度
    bool just_rsp_flag = false;
    if((cmd_dp_data_len == 0) || (cmd_dp_data_len >= OPEN_METH_MAX)) {
        just_rsp_flag = true;
    }
    
    //检查硬件类型枚举
    for(uint8_t idx=0; idx<cmd_dp_data_len; idx++) {
        uint8_t* hard_type = cmd_dp_data;
        if((hard_type[idx] >= OPEN_METH_MAX) || (hard_type[idx] == OPEN_METH_BASE)) {
            just_rsp_flag = true;
            break;
        }
    }
    
    if(!just_rsp_flag)
    {
        uint8_t* hard_type = cmd_dp_data;
        
        memset((void*)&g_sync_new, 0x00, sizeof(open_meth_sync_new_t));
        g_sync_new.flag = 1;
        g_sync_new.hard_type_len = cmd_dp_data_len;
        for(uint8_t idx=0; idx<cmd_dp_data_len; idx++) {
            g_sync_new.hard[idx].type = hard_type[idx];
            g_sync_new.hard[idx].idx = 0;
        }
        lock_open_meth_sync_new_report(0x00);
        
        *rsp_len = 0x00;
    } else {
        rsp->stage = 0x01; //sync finish
        rsp->pkgs = 0x00;
        *rsp_len = sizeof(open_meth_sync_new_last_result_t);
    }
    return ret;
}

/*********************************************************
FN: 
*/
static uint32_t guide_page_handler(void* cmd_dp_data, void* rsp_dp_data, uint8_t* rsp_dp_data_len)
{
    guide_page_t* cmd = cmd_dp_data;
    guide_page_result_t* rsp = rsp_dp_data;
    uint8_t* rsp_len = rsp_dp_data_len;
    uint8_t ret = APP_PORT_SUCCESS;
    
    rsp->func = cmd->func;
    rsp->result = 0x00;
    
    *rsp_len = sizeof(guide_page_result_t);

    return APP_PORT_SUCCESS;
}

/*********************************************************
FN: 
*/
static uint32_t offline_pwd_set_T0_handler(void* cmd_dp_data, void* rsp_dp_data, uint8_t* rsp_dp_data_len)
{
    uint32_t T0_tmp = 0;
    app_port_string_op_intstr2int(cmd_dp_data, 10, (void*)&T0_tmp);
    lock_offline_pwd_set_T0(T0_tmp);
    
    return APP_PORT_SUCCESS;
}

















