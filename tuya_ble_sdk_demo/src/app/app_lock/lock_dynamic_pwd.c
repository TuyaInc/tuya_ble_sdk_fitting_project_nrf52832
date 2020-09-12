#include "lock_dynamic_pwd.h"




/*********************************************************************
 * LOCAL CONSTANTS
 */

/*********************************************************************
 * LOCAL STRUCT
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
//static uint32_t s_last_dynamic_pwd = 0;

/*********************************************************************
 * LOCAL FUNCTION
 */

/*********************************************************************
 * VARIABLES
 */




/*********************************************************
FN: timeseq = (timestamp / DYNAMIC_PWD_TIME_STEP)
*/
static int get_timer_string(unsigned int timeseq, int str_len, char *time_str)
{
    if ( (timeseq == 0) || (str_len == 0) || (time_str == NULL) ) {
        TUYA_APP_LOG_ERROR("get_timer_string paras err");
        return -1;
    }

    snprintf(time_str, str_len, "%X", timeseq);
    if (strlen(time_str) < DYNAMIC_PWD_TOKEN_SIZE) {
        snprintf(time_str, str_len, "%08X", timeseq);
    }

    TUYA_APP_LOG_INFO(" get pass timer is %s", time_str);
    return 0;
}

/*********************************************************
FN: 
*/
static uint32_t TruncateSHA1(uint8_t *digest)
{
	uint32_t offset = digest[20 - 3] & 0x0F;
    
    uint32_t ret = ((digest[offset] & 0x7F) << 24)
                 | ((digest[offset + 1] & 0xFF) << 16)
                 | ((digest[offset + 2] & 0xFF) << 8) 
                 | (digest[offset + 3] & 0xFF);
	return ret;
}

/*********************************************************
FN: 
*/
static uint32_t TOTP(uint8_t *key, unsigned int key_len, uint8_t *msg, unsigned int msg_len, unsigned int digits) 
{
	uint8_t digest[20];
	uint32_t dt;

	HMAC_SHA1(key, key_len, msg, msg_len, digest);
	dt = TruncateSHA1(digest);
	return dt % (int)pow(10.0, digits);
}

/*********************************************************
FN: 
*/
static uint32_t pwd_number_convert_to_integer(uint8_t *pwd_num, uint8_t pwd_len)
{
	uint32_t input_pwd = 0;

    if (pwd_len > DYNAMIC_PWD_TOKEN_SIZE)
        pwd_len = DYNAMIC_PWD_TOKEN_SIZE;

        
	for (int pos=0; pos<pwd_len; pos++) {
		input_pwd = (input_pwd*10) + pwd_num[pos];
	}
	
	return input_pwd;
}

/*********************************************************
FN: 
*/
static int verify_dynamic_pwd_token(uint8_t *key, unsigned int key_len, uint8_t *pwd, uint8_t pwd_len, uint32_t ts)
{
	uint32_t timestamp, timeseq;
	uint32_t input_pwd, cal_dynamicpwd;

	if ((key == NULL) || (pwd == NULL) || (pwd_len < DYNAMIC_PWD_TOKEN_SIZE) || (ts == 0))
		return DYNAMIC_PWD_ERR_PARAS;

	/* Convert pwd number to integer, Is dynamic pwd is used */
	input_pwd = pwd_number_convert_to_integer(pwd, pwd_len);
//	if (input_pwd == s_last_dynamic_pwd) {
//		TUYA_APP_LOG_INFO("dynamic pwd already used\r\n");
//		return DYNAMIC_PWD_ERR_USED;
//	}
    
	/* Calc dynamic pwd */
    timestamp = ts;
    for (int verify_cnt=0; verify_cnt<3; verify_cnt++) {
        if (verify_cnt == 0)
            timeseq = (timestamp / DYNAMIC_PWD_TIME_STEP);
        else if (verify_cnt == 1)
            timeseq = ((timestamp + DYNAMIC_PWD_TIME_WINDOW) / DYNAMIC_PWD_TIME_STEP);
        else if (verify_cnt == 2)
            timeseq = ((timestamp - DYNAMIC_PWD_TIME_WINDOW) / DYNAMIC_PWD_TIME_STEP);
            
        char timeseq_str[DYNAMIC_PWD_TOKEN_SIZE+1] = {0};
        memset(timeseq_str, 0, sizeof(timeseq_str));
        if (get_timer_string(timeseq, sizeof(timeseq_str), timeseq_str))
            return DYNAMIC_PWD_ERR_PARAS;

    	cal_dynamicpwd = TOTP(key, key_len, (uint8_t *)&timeseq_str, strlen(timeseq_str), 8);
    	TUYA_APP_LOG_INFO("calc dynamic pwd->[%d]-[%08d]", verify_cnt, cal_dynamicpwd);
    	if (input_pwd == cal_dynamicpwd) {
//    		s_last_dynamic_pwd = cal_dynamicpwd;
    		TUYA_APP_LOG_INFO("dynamic pwd verify pass");
    		return DYNAMIC_PWD_VERIFY_SUCCESS;
    	} 
    }
    
    TUYA_APP_LOG_INFO("dynamic pwd verify failed");
    return DYNAMIC_PWD_ERR_TIMESEQ;
}

/*********************************************************
FN: 
*/
int lock_dynamic_pwd_verify(uint8_t *pwd, uint8_t size)
{
    return verify_dynamic_pwd_token(tuya_ble_current_para.sys_settings.login_key, LOGIN_KEY_LEN, pwd, size, app_port_get_timestamp());
}





