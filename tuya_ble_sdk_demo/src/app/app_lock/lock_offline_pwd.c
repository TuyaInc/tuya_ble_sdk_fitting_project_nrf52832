#include "lock_offline_pwd.h"
#include "fpe_decrypt.h"




/*********************************************************************
 * LOCAL CONSTANTS
 */

/*********************************************************************
 * LOCAL STRUCT
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
static uint32_t T0 = 1589251799;
static volatile bool is_T0_updated = true;

/*********************************************************************
 * LOCAL FUNCTION
 */
static uint32_t lock_offline_pwd_save(int32_t pwdid, lock_offline_pwd_storage_t* pwd_storage);
static uint32_t lock_offline_pwd_load(int32_t pwdid, lock_offline_pwd_storage_t* pwd_storage);
static void lock_offline_pwd_calculate_T2_T3(lock_offline_pwd_t *pwd, uint32_t* pT2, uint32_t* pT3);
static void lock_offline_pwd_storage_calculate_T2_T3(lock_offline_pwd_storage_t* pwd_storage, uint32_t* pT2, uint32_t* pT3);
static int32_t lock_offline_pwd_find(uint32_t T_now);
static int32_t is_offline_pwd_exist(enum_offline_pwd_type_t type, uint32_t pwd_integer, lock_offline_pwd_storage_t *data);
static int32_t lock_offline_pwd_clear(enum_offline_pwd_type_t type, uint32_t pwd);

/*********************************************************************
 * VARIABLES
 */




/*********************************************************
FN: 
*/
static uint32_t lock_offline_pwd_save(int32_t pwdid, lock_offline_pwd_storage_t* pwd_storage)
{
    if(pwdid >= OFFLINE_PWD_MAX_NUM) {
        return APP_PORT_ERROR_COMMON;
    }
    
	uint32_t err_code = app_port_nv_set(SF_AREA_3, pwdid, pwd_storage, sizeof(lock_offline_pwd_storage_t));
	if(err_code == APP_PORT_SUCCESS) {
        return APP_PORT_SUCCESS;
	}
    return APP_PORT_ERROR_COMMON;
}

/*********************************************************
FN: 
*/
static uint32_t lock_offline_pwd_load(int32_t pwdid, lock_offline_pwd_storage_t* pwd_storage)
{
    if(pwdid >= OFFLINE_PWD_MAX_NUM) {
        return APP_PORT_ERROR_COMMON;
    }
    
	uint32_t err_code = app_port_nv_get(SF_AREA_3, pwdid, pwd_storage, sizeof(lock_offline_pwd_storage_t));
	if(err_code == APP_PORT_SUCCESS) {
        return APP_PORT_SUCCESS;
    }
	return APP_PORT_ERROR_COMMON;
}

/*********************************************************
FN: 
*/
uint32_t lock_offline_pwd_delete(int32_t pwdid)
{
    if(pwdid >= OFFLINE_PWD_MAX_NUM) {
        return APP_PORT_ERROR_COMMON;
    }
    
	uint32_t err_code = app_port_nv_del(SF_AREA_3, pwdid);
	if(err_code == APP_PORT_SUCCESS) {
        return APP_PORT_SUCCESS;
	}
    return APP_PORT_ERROR_COMMON;
}

/*********************************************************
FN: 
*/
uint32_t lock_offline_pwd_delete_all(void)
{
    for (int32_t idx=0; idx<OFFLINE_PWD_MAX_NUM; idx++)
    {
        lock_offline_pwd_delete(idx);
    }
    return APP_PORT_SUCCESS;
}

/*********************************************************
FN: 
*/
void lock_offline_pwd_set_T0(uint32_t T0_tmp)
{
    if (T0_tmp == lock_offline_pwd_get_T0()) {
        TUYA_APP_LOG_INFO("set T0, but same->[%d]", T0);
        return;
    }
    
	T0 = T0_tmp;
    app_port_nv_set(SF_AREA_0, NV_ID_T0_STORAGE, &T0, sizeof(uint32_t));
    TUYA_APP_LOG_INFO("set T0->[%d]", T0);
    
    is_T0_updated = true;
}

/*********************************************************
FN: 
*/
uint32_t lock_offline_pwd_get_T0(void)
{
    if (is_T0_updated) {
        app_port_nv_get(SF_AREA_0, NV_ID_T0_STORAGE, &T0, sizeof(uint32_t));
//        TUYA_APP_LOG_INFO("get T0->[%d]", T0);
        is_T0_updated = false;
    }
    return T0;
}

/*********************************************************
FN: 
*/
static void lock_offline_pwd_calculate_T2_T3(lock_offline_pwd_t *pwd, uint32_t* pT2, uint32_t* pT3)
{
	uint32_t num_1_5 = 0;
    uint32_t num_6_9 = 0;
	uint32_t T0_round;
    uint32_t T2 = 0;
    uint32_t T3 = 0;

    num_1_5 = app_port_num_array_2_int(pwd->num_1_5_array, 0, 5);
    num_6_9 = app_port_num_array_2_int(pwd->num_6_9_array, 0, 4);

    // T0 = T0 - (T0 % 3600)
    // num_1_5 = (T2 - T0)/3600      T2 = num_1_5*3600 + T0
	// num_6_9 = (T3 - T2)/3600      T3 = num_6_9*3600 + T2
    T0_round = T0 - (T0 % 3600);
    T2 = num_1_5*OFFLINE_PWD_TIME_ACCURACY + T0_round;
    if(pwd->type == PWD_TYPE_TIMELINESS || pwd->type == PWD_TYPE_CLEAR_SINGLE) {
        T3 = (num_6_9*OFFLINE_PWD_TIME_ACCURACY + T2);
    } else if (pwd->type == PWD_TYPE_SINGLE || pwd->type == PWD_TYPE_CLEAR_ALL) {
        T3 = num_6_9;
    }

	*pT2 = T2;
	*pT3 = T3;
    
    
    //��ӡ T2��T3����ǰʱ��
    tuya_ble_time_struct_data_t datatime;
    
    tuya_ble_utc_sec_2_mytime(T2, &datatime, 0);
	TUYA_APP_LOG_INFO("T2->[%02d-%02d-%02d %02d:xx:xx]", datatime.nYear, datatime.nMonth, datatime.nDay, datatime.nHour);
    
    if (pwd->type == PWD_TYPE_TIMELINESS || pwd->type == PWD_TYPE_CLEAR_SINGLE) {
        tuya_ble_utc_sec_2_mytime(T3, &datatime, 0);
        TUYA_APP_LOG_INFO("T3->[%02d-%02d-%02d %02d:xx:xx]", datatime.nYear, datatime.nMonth, datatime.nDay, datatime.nHour);
    } else {
        TUYA_APP_LOG_INFO("C3->[%d]", T3);
    }
    
    tuya_ble_utc_sec_2_mytime(app_port_get_timestamp(), &datatime, 0);
	TUYA_APP_LOG_INFO("NOW->[%02d-%02d-%02d %02d:xx:xx]", datatime.nYear, datatime.nMonth, datatime.nDay, datatime.nHour);
}

/*********************************************************
FN: 
*/
static void lock_offline_pwd_storage_calculate_T2_T3(lock_offline_pwd_storage_t* pwd_storage, uint32_t* pT2, uint32_t* pT3)
{
	uint32_t num_1_5 = 0;
    uint32_t num_6_9 = 0;
	uint32_t T0_round;
    uint32_t T2 = 0;
    uint32_t T3 = 0;
   
    num_1_5 = (pwd_storage->pwd / 10000);
    num_6_9 = (pwd_storage->pwd % 10000);
    
    // T0 = T0 - (T0 % 3600)
    // num_1_5 = (T2 - T0)/3600      T2 = num_1_5*3600 + T0
	// num_6_9 = (T3 - T2)/3600      T3 = num_6_9*3600 + T2
    T0_round = T0 - (T0 % 3600);
    T2 = num_1_5*OFFLINE_PWD_TIME_ACCURACY + T0_round;
    if (pwd_storage->type == PWD_TYPE_TIMELINESS) {
        T3 = (num_6_9*OFFLINE_PWD_TIME_ACCURACY + T2);
    } else if (pwd_storage->type == PWD_TYPE_SINGLE) {
        T3 = T2 + PWD_ACTIVE_PERIOD_SINGLE;
    } else if (pwd_storage->type == PWD_TYPE_CLEAR_ALL) {
        T3 = T2 + PWD_ACTIVE_PERIOD_CLEAR_ALL;
    }
    
	*pT2 = T2;
	*pT3 = T3;
}

/*********************************************************
FN: 
*/
static int32_t lock_offline_pwd_find(uint32_t T_now)
{
    uint32_t T2, T3;
	int32_t  find_pwdid = -1;
    bool find_flag = false;
    lock_offline_pwd_find_info_t info_invalid_pwd;
    lock_offline_pwd_find_info_t info_valid_pwd;
    lock_offline_pwd_find_info_t info_single_pwd;
    lock_offline_pwd_find_info_t info_clear_all;
    
    // ��������洢���ȼ�
    // 1.δʹ�õ�
    // 2.���ڵģ�T3 < T_now����ֱ�Ӹ���
    // 3.ʱЧ����������״̬Ϊ��Ч�ģ���ʧЧʱ�������
    // 4.ʱЧ����������״̬Ϊ��Ч�ģ�����Чʱ�������
    // 5.�綼�ǵ������룬��ʧЧʱ�������
    // 6.�綼������������룬��ʧЧʱ�������
    info_invalid_pwd.found = false;
    info_valid_pwd.found   = false;
    info_single_pwd.found  = false;
    info_clear_all.found   = false;
    
    for (int32_t idx=0; idx<OFFLINE_PWD_MAX_NUM; idx++)
    {
        lock_offline_pwd_storage_t pwd_storage = {0};
        lock_offline_pwd_load(idx, &pwd_storage);
        
        //δʹ�õ�
		if (pwd_storage.status == PWD_STATUS_UNUSED)
        {
			find_pwdid = idx;
			find_flag = true;
            TUYA_APP_LOG_INFO("find unused pwdid->[%d]", idx);
            break;
		}
        else
        {
            lock_offline_pwd_storage_calculate_T2_T3(&pwd_storage, &T2, &T3);
            
            //���ڵ�
            if (T3 < T_now) {
                find_pwdid = idx;
			    find_flag = true;
                TUYA_APP_LOG_INFO("find outtime(T3<T_now) pwdid->[%d]", idx);
                break;
            }
            //ʱЧ����
            else if (pwd_storage.type == PWD_TYPE_TIMELINESS) {
                //����״̬Ϊ��Ч�ģ���ʧЧʱ�������
                if (pwd_storage.status == PWD_STATUS_INVALID) {
                    if (!info_invalid_pwd.found) {
                        info_invalid_pwd.found = true;
                        info_invalid_pwd.min_T = T3;
                        info_invalid_pwd.pwdid = idx;
                    } else if (T3 < info_invalid_pwd.min_T) {
                        info_invalid_pwd.min_T = T3;
                        info_invalid_pwd.pwdid = idx;
                    }
                }
                //����״̬Ϊ��Ч�ģ�����Чʱ�������
                else {
                    if (!info_valid_pwd.found) {
                        info_valid_pwd.found = true;
                        info_valid_pwd.min_T = T2;
                        info_valid_pwd.pwdid = idx;
                    } else if (T2 < info_valid_pwd.min_T) {
                        info_valid_pwd.min_T = T2;
                        info_valid_pwd.pwdid = idx;
                    }
                }
            }
            //�綼�ǵ������룬��ʧЧʱ�������
            else if (pwd_storage.type == PWD_TYPE_SINGLE) {
                if (!info_single_pwd.found) {
                    info_single_pwd.found = true;
                    info_single_pwd.min_T = T3;
                    info_single_pwd.pwdid = idx;
                } else if (T3 < info_single_pwd.min_T) {
                    info_single_pwd.min_T = T3;
                    info_single_pwd.pwdid = idx;
                }
            }
            //�綼������������룬��ʧЧʱ�������
            else if (pwd_storage.type == PWD_TYPE_CLEAR_ALL) {
                if (!info_clear_all.found) {
                    info_clear_all.found = true;
                    info_clear_all.min_T = T3;
                    info_clear_all.pwdid = idx;
                } else if (T3 < info_clear_all.min_T) {
                    info_clear_all.min_T = T3;
                    info_clear_all.pwdid = idx;
                }
            }
        }
	}

    if (!find_flag) {
        if (info_invalid_pwd.found) {
            find_pwdid = info_invalid_pwd.pwdid;
        }
        else if (info_valid_pwd.found) {
            find_pwdid = info_valid_pwd.pwdid;
        }
        else if (info_single_pwd.found) {
            find_pwdid = info_single_pwd.pwdid;
        }
        else if (info_clear_all.found) {
            find_pwdid = info_clear_all.pwdid;
        }
        TUYA_APP_LOG_INFO("all pwdid full, cover id->[%d]", find_pwdid);
    }
	return find_pwdid;
}

/*********************************************************
FN: 
RT: >= 0 exist
    -1
    -2
*/
static int32_t is_offline_pwd_exist(enum_offline_pwd_type_t type, uint32_t pwd_int, lock_offline_pwd_storage_t *data)
{
	for (int32_t idx=0; idx<OFFLINE_PWD_MAX_NUM; idx++)
    {
        lock_offline_pwd_storage_t pwd_storage = {0};
        lock_offline_pwd_load(idx, &pwd_storage);
        
		if ((pwd_storage.status != PWD_STATUS_UNUSED)
            && (pwd_storage.type == type)
            && (pwd_storage.pwd == pwd_int)) {
            
			memcpy(data, &pwd_storage, sizeof(lock_offline_pwd_storage_t));
            return idx;
		}
	}
	return -1;
}

/*********************************************************
FN: 
*/
static int32_t lock_offline_pwd_clear(enum_offline_pwd_type_t type, uint32_t pwd)
{
    if ((type != PWD_TYPE_CLEAR_SINGLE) && (type != PWD_TYPE_CLEAR_ALL)) {
        TUYA_APP_LOG_INFO("Warning! clear offline pwd, but type err");
        return OFFLINE_PWD_ERR_PARAM;
    }
    
    bool clear_all_flag = false;
    
    for (int32_t idx=0; idx<OFFLINE_PWD_MAX_NUM; idx++)
    {
        lock_offline_pwd_storage_t pwd_storage = {0};
        lock_offline_pwd_load(idx, &pwd_storage);
        
        //�����������
        if(type == PWD_TYPE_CLEAR_SINGLE && pwd_storage.pwd == pwd)
        {
            if(pwd_storage.status == PWD_STATUS_VALID) {
                pwd_storage.status = PWD_STATUS_INVALID;
                lock_offline_pwd_save(idx, &pwd_storage);
                
                TUYA_APP_LOG_INFO("OFFLINE_PWD_CLEAR_SINGLE_SUCCESS");
                return OFFLINE_PWD_CLEAR_SINGLE_SUCCESS;
            }
            else if (pwd_storage.status == PWD_STATUS_INVALID) {
                TUYA_APP_LOG_INFO("OFFLINE_PWD_ERR_INVALID");
                return OFFLINE_PWD_ERR_INVALID;
            }
        }
        //�����������
        else if (type == PWD_TYPE_CLEAR_ALL)
        {
            if(pwd_storage.status == PWD_STATUS_VALID) {
                clear_all_flag = true;
                pwd_storage.status = PWD_STATUS_INVALID;
                lock_offline_pwd_save(idx, &pwd_storage);
            }
        }
	}
    
    if(type == PWD_TYPE_CLEAR_SINGLE) {
        TUYA_APP_LOG_INFO("OFFLINE_PWD_ERR_NO_EXIST");
        return OFFLINE_PWD_ERR_NO_EXIST;
    }
    else {
        if(clear_all_flag) {
            TUYA_APP_LOG_INFO("OFFLINE_PWD_CLEAR_ALL_SUCCESS");
            return OFFLINE_PWD_CLEAR_ALL_SUCCESS;
        }
        else {
            TUYA_APP_LOG_INFO("OFFLINE_PWD_ERR_NO_EXIST");
            return OFFLINE_PWD_ERR_NO_EXIST;
        }
    }
}

/*********************************************************
FN: 
PM: key - 16�ֽ��޷���������Ĭ������ַ���0�����ڽ�β��䡰tuya_ble_current_para.sys_settings.login_key����6�ֽڣ�������login_keyΪ��8f7d54������keyΪ��00000000008f7d54��
    key_len - 16�ֽ�
    encrypt_pwd - �������루�û����������룩
    encrypt_pwd_len - 10�ֽ�
    timestamp - ��ǰʱ���
    plain_pwd - ������������
    p_plain_pwd_len - 10�ֽ�
*/
int32_t lock_offline_pwd_verify(uint8_t *key, uint8_t key_len,
                                    uint8_t *encrypt_pwd, uint8_t encrypt_pwd_len,
                                    uint32_t timestamp,
                                    uint8_t *plain_pwd, uint8_t *p_plain_pwd_len)
{
	uint8_t decrypt_pwd[OFFLINE_PWD_LEN] = {0};
    uint8_t decrypt_pwd_len;
    
	uint32_t T2, T3, T_now = timestamp;
	uint32_t active_period;
    
    //����������
	if ((key == NULL)
            || (key_len != 16 && key_len != 24 && key_len != 32)
            || (encrypt_pwd == NULL)
            || (encrypt_pwd_len != OFFLINE_PWD_LEN)) {
		TUYA_APP_LOG_ERROR("OFFLINE_PWD_ERR_PARAM");
		return OFFLINE_PWD_ERR_PARAM;
	}

    //fpe����
	if (fpe_decrypt(key, key_len, encrypt_pwd, encrypt_pwd_len, decrypt_pwd, &decrypt_pwd_len)) {
		TUYA_APP_LOG_ERROR("OFFLINE_PWD_ERR_DECRYPT");
		return OFFLINE_PWD_ERR_DECRYPT;
	}
    //�������
    if (decrypt_pwd_len > OFFLINE_PWD_LEN) {
		TUYA_APP_LOG_ERROR("OFFLINE_PWD_ERR_UNKNOW");
        return OFFLINE_PWD_ERR_UNKNOW;
    }
    
    //��ȡ��������
    if (plain_pwd != NULL) {
        memcpy(plain_pwd, decrypt_pwd, decrypt_pwd_len);
        *p_plain_pwd_len = decrypt_pwd_len;
    }
    
    //�����������ͼ��
	lock_offline_pwd_t *pwd = (void*)decrypt_pwd;
    TUYA_APP_LOG_HEXDUMP_INFO("decrypt_pwd", (void*)pwd, sizeof(lock_offline_pwd_t));
	if (pwd->type != PWD_TYPE_TIMELINESS
            && pwd->type != PWD_TYPE_SINGLE
            && pwd->type != PWD_TYPE_CLEAR_SINGLE
            && pwd->type != PWD_TYPE_CLEAR_ALL) {
		TUYA_APP_LOG_ERROR("OFFLINE_PWD_ERR_TYPE");
		return OFFLINE_PWD_ERR_TYPE;
	}
	
    //��ȡT0
    lock_offline_pwd_get_T0();
    //����T2��T3
	lock_offline_pwd_calculate_T2_T3(pwd, &T2, &T3);
    
    
	uint32_t num_1_9 = app_port_num_array_2_int(decrypt_pwd, 1, 9);
	TUYA_APP_LOG_INFO("offline pwd info, type->[%d] num_1_9->[%09d]", pwd->type, num_1_9);
    //ʱЧ/��������
    if (pwd->type == PWD_TYPE_TIMELINESS || pwd->type == PWD_TYPE_SINGLE)
    {
        //ʱЧ����
        if(pwd->type == PWD_TYPE_TIMELINESS) {
            if (T2 > T_now) {
                TUYA_APP_LOG_INFO("OFFLINE_PWD_ERR_START_TIME");
        	    return OFFLINE_PWD_ERR_START_TIME;
            }
        	else if (T3 < T_now) {
                TUYA_APP_LOG_INFO("OFFLINE_PWD_ERR_END_TIME");
        		return OFFLINE_PWD_ERR_END_TIME;
            }
            active_period = PWD_ACTIVE_PERIOD_TIMELINESS;
        }
        //��������
        else if(pwd->type == PWD_TYPE_SINGLE) {
            if (T2 > T_now) {
                TUYA_APP_LOG_INFO("OFFLINE_PWD_ERR_START_TIME");
        		return OFFLINE_PWD_ERR_START_TIME;
            }
            else if (T3 > PWD_MAX_SN_PER_PERIOD) {
                TUYA_APP_LOG_INFO("OFFLINE_PWD_ERR_BSS_SN");
                return OFFLINE_PWD_ERR_BSS_SN;
            }
            active_period = PWD_ACTIVE_PERIOD_SINGLE;
        }

        int32_t exist_pwdid = 0;
        lock_offline_pwd_storage_t pwd_storage = {0};
        exist_pwdid = is_offline_pwd_exist((enum_offline_pwd_type_t)pwd->type, num_1_9, &pwd_storage);
        TUYA_APP_LOG_INFO("is_offline_pwd_exist, pwdid-->[%d]", exist_pwdid);
        //���������Ѿ�����
        if (exist_pwdid >= 0) {
            if(pwd_storage.status == PWD_STATUS_INVALID) {
                TUYA_APP_LOG_INFO("OFFLINE_PWD_ERR_INVALID");
                return OFFLINE_PWD_ERR_INVALID;
            } else{
                TUYA_APP_LOG_INFO("OFFLINE_PWD_VERIFY_SUCCESS");
                return OFFLINE_PWD_VERIFY_SUCCESS;
            }
        }
        //�������볬����������
		else if ((T_now - T2) > active_period) {
            TUYA_APP_LOG_INFO("OFFLINE_PWD_ERR_ACTIVE_TIME");
			return OFFLINE_PWD_ERR_ACTIVE_TIME;
        }
        //�洢��������
		else {
            TUYA_APP_LOG_INFO("lock_offline_pwd_find");
            int32_t find_pwdid = lock_offline_pwd_find(T_now);
			if (find_pwdid < 0) {
				TUYA_APP_LOG_INFO("OFFLINE_PWD_ERR_NO_SPACE");
				return OFFLINE_PWD_ERR_NO_SPACE;
			}
			
			pwd_storage.status = (pwd->type == PWD_TYPE_TIMELINESS) ? (PWD_STATUS_VALID) : (PWD_STATUS_INVALID); //��������ʹ��һ�κ��ʧЧ
			pwd_storage.type   = pwd->type;
			pwd_storage.pwd    = num_1_9;
            lock_offline_pwd_save(find_pwdid, &pwd_storage);
            
            TUYA_APP_LOG_INFO("OFFLINE_PWD_VERIFY_SUCCESS");
			return OFFLINE_PWD_VERIFY_SUCCESS;
        }
    }
    //�����������
    else if (pwd->type == PWD_TYPE_CLEAR_SINGLE)
    {
        //���ʱЧ��
        if (T2 > T_now) {
            TUYA_APP_LOG_INFO("OFFLINE_PWD_ERR_NO_EXIST");
            return OFFLINE_PWD_ERR_NO_EXIST;
        }
        else if (T3 < T_now) {
            TUYA_APP_LOG_INFO("OFFLINE_PWD_ERR_END_TIME");
            return OFFLINE_PWD_ERR_END_TIME;
        }
        
        return lock_offline_pwd_clear(PWD_TYPE_CLEAR_SINGLE, num_1_9);
    }
    //�����������
    else if (pwd->type == PWD_TYPE_CLEAR_ALL)
    {
        if (T2 > T_now) {
            TUYA_APP_LOG_INFO("OFFLINE_PWD_ERR_START_TIME");
			return OFFLINE_PWD_ERR_START_TIME;
        }
        //�������볬����������
		else if ((T_now - T2) > PWD_ACTIVE_PERIOD_CLEAR_ALL) {
            TUYA_APP_LOG_INFO("OFFLINE_PWD_ERR_ACTIVE_TIME");
			return OFFLINE_PWD_ERR_ACTIVE_TIME;
        }
		else if (T3 > PWD_MAX_SN_PER_PERIOD) {
            TUYA_APP_LOG_INFO("OFFLINE_PWD_ERR_BSS_SN");
            return OFFLINE_PWD_ERR_BSS_SN;
        }
        
        int32_t exist_pwdid = 0;
        lock_offline_pwd_storage_t pwd_storage = {0};
        exist_pwdid = is_offline_pwd_exist((enum_offline_pwd_type_t)pwd->type, num_1_9, &pwd_storage);
        TUYA_APP_LOG_INFO("is_offline_pwd_exist, pwdid-->[%d]", exist_pwdid);
        //���������Ѿ�����
        if (exist_pwdid >= 0) {
            TUYA_APP_LOG_INFO("OFFLINE_PWD_ERR_INVALID");
            return OFFLINE_PWD_ERR_INVALID;
        }
        //�洢��������
		else {
            int32_t find_pwdid = lock_offline_pwd_find(T_now);
			if (find_pwdid < 0) {
				TUYA_APP_LOG_INFO("OFFLINE_PWD_ERR_NO_SPACE");
				return OFFLINE_PWD_ERR_NO_SPACE;
			}
			
			pwd_storage.status = PWD_STATUS_INVALID; //�����������ʹ��һ�κ��ʧЧ
			pwd_storage.type   = pwd->type;
			pwd_storage.pwd    = num_1_9;
            lock_offline_pwd_save(find_pwdid, &pwd_storage);
            
            return lock_offline_pwd_clear(PWD_TYPE_CLEAR_ALL, 0);
        }
    }
    
    //Can't reach here
	return OFFLINE_PWD_ERR_UNKNOW;
}
