#include "tuya_ble_master.h"
#include "tuya_ble_stdlib.h"
#include "tuya_ble_type.h"
#include "tuya_ble_heap.h"
#include "tuya_ble_mem.h"
#include "tuya_ble_api.h"
#include "tuya_ble_port.h"
#include "tuya_ble_main.h"
#include "tuya_ble_internal_config.h"
#include "tuya_ble_data_handler.h"
#include "tuya_ble_mutli_tsf_protocol.h"
#include "tuya_ble_utils.h"
#include "tuya_ble_secure.h"
#include "tuya_ble_main.h"
#include "tuya_ble_storage.h"
#include "tuya_ble_unix_time.h"
#include "tuya_ble_log.h"
#include "tuya_ble_gatt_send_queue.h"
#include "tuya_ble_master_port.h"




/*********************************************************************
 * LOCAL CONSTANT
 */

/*********************************************************************
 * LOCAL STRUCT
 */
typedef struct{
    uint32_t send_len;
    uint8_t  *send_data;
    uint32_t encrypt_data_buf_len;
    uint8_t  *encrypt_data_buf;
}tuya_ble_master_r_air_send_packet;

typedef struct{
    uint32_t recv_len;
    uint32_t recv_len_max;
    uint8_t  *recv_data;
    uint32_t decrypt_buf_len;
    uint8_t  *de_encrypt_buf;
}tuya_ble_master_r_air_recv_packet;

/*********************************************************************
 * LOCAL VARIABLE
 */
static tuya_ble_master_r_air_recv_packet air_recv_packet;

static frm_trsmitr_proc_s ty_trsmitr_proc;
static frm_trsmitr_proc_s ty_trsmitr_proc_send;

static uint32_t tuya_ble_receive_sn = 0;
static uint32_t tuya_ble_send_sn = 1;

static uint8_t service_rand[16] = {0};
static uint8_t tuya_ble_master_pair_rand[6] = {0};

/*********************************************************************
 * VARIABLE
 */

/*********************************************************************
 * LOCAL FUNCTION
 */




/*********************************************************************
 * LOCAL CONSTANT
 */
#define MASTER_RANDOM_NUM_LEN         8

enum {
    NV_ID_MASTER_INFO = 0xFFFE,
};

/*********************************************************************
 * LOCAL STRUCT
 */
#pragma pack(1)
typedef struct
{
    uint8_t  operation;
    uint16_t masterid;
    uint8_t  random[MASTER_RANDOM_NUM_LEN];
    uint16_t slaveid;
    uint8_t  device_id[DEVICE_ID_LEN];
    uint8_t  login_key[16];
} data_passthrough_t;

typedef struct
{
    uint8_t  operation;
    uint16_t masterid;
    uint8_t  random[MASTER_RANDOM_NUM_LEN];
    uint16_t slaveid;
    uint8_t  result;
} data_passthrough_result_t;

typedef struct
{
    uint32_t sn;
    uint32_t ack_sn;
    uint16_t id;
    uint16_t len;
    uint8_t  value[];
} tuya_ble_protocol_t;

typedef struct
{
    uint16_t firmware_version;
    uint16_t protocol_version;
    uint8_t  flag;
    uint8_t  bond;
    uint8_t  srand[PAIR_RANDOM_LEN];
    uint16_t hardware_version;
    uint8_t  auth_key[AUTH_KEY_LEN];
    uint8_t  firmware_version2[3];
    uint8_t  hardware_version2[3];
    uint16_t communication_type;
    uint8_t  reserved;
    uint8_t  device_virtual_id[DEVICE_VIRTUAL_ID_LEN];
} device_info_result_t;

typedef struct
{
    uint8_t  device_id[DEVICE_ID_LEN];
    uint8_t  login_key[LOGIN_KEY_LEN];
    uint8_t  device_virtual_id[DEVICE_VIRTUAL_ID_LEN];
} bond_request_t;

typedef struct
{
    uint16_t masterid;
    uint16_t slaveid;
    uint8_t  srand[8];
    uint8_t  operation;
    uint32_t timestamp;
    uint8_t  open_meth;
    uint8_t  open_meth_info[256];
} open_with_master_t;

typedef struct
{
    uint16_t masterid;
    uint16_t slaveid;
    uint8_t  srand[8];
    uint8_t  operation;
    uint32_t timestamp;
    uint8_t  open_meth;
    uint8_t  result;
} open_with_master_result_t;

//dp point
typedef struct
{
    uint8_t dp_id;
    uint8_t dp_type;
    uint8_t dp_data_len;
    union
    {
        uint8_t dp_data[256];
        open_with_master_t open_with_master;
        open_with_master_result_t open_with_master_result;
    };
} master_dp_t;

typedef struct
{
    uint16_t id;
    uint8_t  srand[MASTER_RANDOM_NUM_LEN];
} master_info_t;
#pragma pack()

/*********************************************************************
 * LOCAL VARIABLE
 */
static bool s_master_bonding_connecting = false;

static int32_t s_current_slave_index = -1;
static int32_t s_slaveid_tobe_scan = -1;
static int32_t s_last_slave_index = -1;

static master_dp_t s_master_cmd = {0};

static uint32_t s_slave_nvid_base = 0;
static uint32_t s_slave_nvid_max_num = 0;

static slave_info_t* p_slave_info = NULL;
static master_info_t s_master_info = {0};

static tuya_ble_master_evt_handler_t s_tuya_ble_master_evt_handler;

/*********************************************************************
 * VARIABLE
 */

/*********************************************************************
 * LOCAL FUNCTION
 */
static void suble_scan_result_handler(uint32_t evt, uint8_t* buf, uint32_t size);
static void suble_connect_result_handler(uint32_t evt, uint8_t* buf, uint32_t size);
static void suble_svc_result_handler(uint32_t evt, uint8_t* buf, uint32_t size);
static void suble_master_evt_handler(uint32_t evt, uint8_t* buf, uint32_t size);
static void tuya_ble_master_receive_data_from_slave_decrypt(void* buf, uint32_t size);

static void tuya_ble_master_connect_monitor_timer_init(void);
static void tuya_ble_master_connect_monitor_timer_start(void);
static void tuya_ble_master_connect_monitor_timer_stop(void);




/*********************************************************
FN: 
*/
static void tuya_ble_air_recv_packet_free(void)
{
    if(air_recv_packet.recv_data)
    {
        tuya_ble_free(air_recv_packet.recv_data);
        air_recv_packet.recv_data = NULL;
        air_recv_packet.recv_len_max = 0;
        air_recv_packet.recv_len = 0;
    }
}

/*********************************************************
FN: 
*/
static uint32_t get_ble_send_sn(void)
{
    uint32_t sn;
    tuya_ble_device_enter_critical();
    sn = tuya_ble_send_sn++;
    tuya_ble_device_exit_critical();
    return sn;
}

/*********************************************************
FN: 
*/
static void set_ble_receive_sn(uint32_t sn)
{
    tuya_ble_device_enter_critical();
    tuya_ble_receive_sn = sn;
    tuya_ble_device_exit_critical();
}

/*********************************************************
FN: 
*/
static uint8_t tuya_ble_master_cmd_data_crc_check(uint8_t *input,uint16_t len)
{
    uint16_t data_len = 0;
    uint16_t crc16 = 0xFFFF;
    uint16_t crc16_cal = 0;

    data_len = (input[10]<<8)|input[11];

    if((13+data_len)>=TUYA_BLE_AIR_FRAME_MAX)
    {
        return 1;
    }

    crc16_cal = tuya_ble_crc16_compute(input,12+data_len, &crc16);

    TUYA_BLE_LOG_DEBUG("crc16_cal[0x%04x]",crc16_cal);
    crc16 = (input[12+data_len]<<8)|input[13+data_len];
    TUYA_BLE_LOG_DEBUG("crc16[0x%04x]",crc16);
    if(crc16==crc16_cal)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

/*********************************************************
FN: 
*/
static uint32_t ble_data_unpack(uint8_t *buf,uint32_t len)
{
    static uint32_t offset = 0;
    mtp_ret ret;

    ret = trsmitr_recv_pkg_decode(&ty_trsmitr_proc, buf, len);
    if(MTP_OK != ret && MTP_TRSMITR_CONTINUE != ret)
    {
        air_recv_packet.recv_len_max = 0;
        air_recv_packet.recv_len = 0;
        if(air_recv_packet.recv_data)
        {
            tuya_ble_free(air_recv_packet.recv_data);
            air_recv_packet.recv_data = NULL;
        }
        
        return 1;
    }

    if(FRM_PKG_FIRST == ty_trsmitr_proc.pkg_desc)
    {
        if(air_recv_packet.recv_data)
        {
            tuya_ble_free(air_recv_packet.recv_data);
            air_recv_packet.recv_data = NULL;
        }
        air_recv_packet.recv_len_max = get_trsmitr_frame_total_len(&ty_trsmitr_proc);
        if((air_recv_packet.recv_len_max>TUYA_BLE_AIR_FRAME_MAX)||(air_recv_packet.recv_len_max==0))
        {
            air_recv_packet.recv_len_max = 0;
            air_recv_packet.recv_len = 0;
            TUYA_BLE_LOG_ERROR("ble_data_unpack total size [%d ]error.",air_recv_packet.recv_len_max);
            return 2;
        }
        air_recv_packet.recv_len = 0;
        air_recv_packet.recv_data = tuya_ble_malloc(air_recv_packet.recv_len_max);
        if(air_recv_packet.recv_data==NULL)
        {
            TUYA_BLE_LOG_ERROR("ble_data_unpack malloc failed.");
            return 2;
        }
        memset(air_recv_packet.recv_data,0,air_recv_packet.recv_len_max);
        offset = 0;
    }
    if((offset+get_trsmitr_subpkg_len(&ty_trsmitr_proc))<=air_recv_packet.recv_len_max)
    {
        if(air_recv_packet.recv_data)
        {
            memcpy(air_recv_packet.recv_data+offset,get_trsmitr_subpkg(&ty_trsmitr_proc),get_trsmitr_subpkg_len(&ty_trsmitr_proc));
            offset += get_trsmitr_subpkg_len(&ty_trsmitr_proc);
            air_recv_packet.recv_len = offset;
        }
        else
        {
            TUYA_BLE_LOG_ERROR("ble_data_unpack error.");
            air_recv_packet.recv_len_max = 0;
            air_recv_packet.recv_len = 0;
            return 2;
        }
    }
    else
    {
        ret = MTP_INVALID_PARAM;
        TUYA_BLE_LOG_ERROR("ble_data_unpack[%d] error:MTP_INVALID_PARAM");
        tuya_ble_air_recv_packet_free();
    }

    if(ret == MTP_OK)
    {
        offset=0;
        TUYA_BLE_LOG_DEBUG("ble_data_unpack[%d]",air_recv_packet.recv_len);

        return 0;
    }
    else
    {
        return 2;
    }
}

/*********************************************************
FN: 
*/
#define MASTER_KEY_IN_BUFFER_LEN_MAX 64
static bool tuya_ble_generate_key1(uint8_t *input,uint16_t input_len,uint8_t *output,tuya_ble_parameters_settings_t *current_para)
{
    //
    uint8_t key[16];
    uint8_t iv[16];

    if(input_len%16)
    {
        return false;
    }

    memcpy(key,current_para->auth_settings.auth_key,16);
    memset(iv,0,16);

    tuya_ble_aes128_cbc_encrypt(key,iv,input,input_len,output);
    //
    return true;
}

/*********************************************************
FN: 
*/
static bool tuya_ble_key_generate(uint8_t mode,uint8_t *key_out,tuya_ble_parameters_settings_t *current_para,uint8_t *dev_rand)
{
    uint16_t len = 0;
    static uint8_t key_out_hex[16] = {0};
    static uint8_t key_out_key1[48] = {0};
    static uint8_t key_in_buffer[MASTER_KEY_IN_BUFFER_LEN_MAX] = {0};

    if(mode>=ENCRYPTION_MODE_MAX)
    {
        return false;
    }
    if(mode==ENCRYPTION_MODE_NONE)
    {
        return true;
    }
    
    memset(key_in_buffer,0,sizeof(key_in_buffer));
    memset(key_out_key1,0,sizeof(key_out_key1));
    
    len = 0;
    memcpy(key_in_buffer+len,current_para->auth_settings.auth_key,AUTH_KEY_LEN);
    len += AUTH_KEY_LEN;
    
    if((mode==ENCRYPTION_MODE_KEY_1)||(mode==ENCRYPTION_MODE_KEY_2)||(mode==ENCRYPTION_MODE_KEY_3))
    {
        memcpy(key_in_buffer+len,service_rand,16);
        len += 16;
        if(!tuya_ble_generate_key1(key_in_buffer,AUTH_KEY_LEN+16,key_out_key1,current_para))
        {
            TUYA_BLE_LOG_ERROR("tuya_aes_generate_key1 failed.");
            return false;
        }
        
        memcpy(key_out_hex,key_out_key1+32,16);
        
        if(mode==ENCRYPTION_MODE_KEY_1)
        {
            memcpy(key_out,key_out_hex,16);
            return true;
        }

    }

    memset(key_in_buffer,0,sizeof(key_in_buffer));
    if((mode!=ENCRYPTION_MODE_KEY_1)&&(mode!=ENCRYPTION_MODE_KEY_4))
    {
        if(memcmp(dev_rand,key_in_buffer,PAIR_RANDOM_LEN)==0)
        {
            return false;
        }
    }
    if(mode==ENCRYPTION_MODE_KEY_3)
    {
        if(memcmp(current_para->sys_settings.user_rand,key_in_buffer,PAIR_RANDOM_LEN)==0)  //没有 user_rand
        {
            return false;
        }
    }

    if((16+PAIR_RANDOM_LEN*2)>MASTER_KEY_IN_BUFFER_LEN_MAX)
    {
        return false;
    }
    
    len = 0;
    switch(mode)
    {
    case ENCRYPTION_MODE_KEY_2:
        memcpy(key_in_buffer,key_out_hex,16);
        len += 16;
        memcpy(key_in_buffer+len,dev_rand,PAIR_RANDOM_LEN);
        len += PAIR_RANDOM_LEN;
        break;
    case ENCRYPTION_MODE_KEY_3:
        memcpy(key_in_buffer,key_out_hex,16);
        len += 16;
        memcpy(key_in_buffer+len,dev_rand,PAIR_RANDOM_LEN);
        len += PAIR_RANDOM_LEN;
        memcpy(key_in_buffer+len,current_para->sys_settings.user_rand,PAIR_RANDOM_LEN);
        len += PAIR_RANDOM_LEN;
        break;
    case ENCRYPTION_MODE_KEY_4:
        memcpy(key_in_buffer+len,p_slave_info[s_current_slave_index].login_key,LOGIN_KEY_LEN);
        len += LOGIN_KEY_LEN;
        break;
    case ENCRYPTION_MODE_SESSION_KEY:
        memcpy(key_in_buffer+len,p_slave_info[s_current_slave_index].login_key,LOGIN_KEY_LEN);
        len += LOGIN_KEY_LEN;
        memcpy(key_in_buffer+len,dev_rand,PAIR_RANDOM_LEN);
        len += PAIR_RANDOM_LEN;
        break;
    default:
        break;
    };
    if(len==0)
    {
        return false;
    }

    memset(key_out_hex,0,sizeof(key_out_hex));

    tuya_ble_md5_crypt(key_in_buffer,len,key_out_hex);

    memcpy(key_out,key_out_hex,16);

    return true;
}

/*********************************************************
FN: 
*/
static uint16_t tuya_ble_Add_Pkcs(uint8_t *p, uint16_t len)
{
    uint8_t pkcs[16];
    uint16_t out_len;

    if(len%16==0)
    {
        out_len = len;
    }
    else
    {
        uint16_t i = 0, cz = 16-len%16;
        memset(pkcs, 0, 16);
        for( i=0; i<cz; i++ ) {
            pkcs[i]=cz;
        }
        memcpy(p + len, pkcs, cz);
        out_len = len + cz;
    }
    return (out_len);
}

/*********************************************************
FN: 
*/
static uint8_t tuya_ble_master_encryption(uint8_t encryption_mode,uint8_t *iv,uint8_t *in_buf,uint32_t in_len,uint32_t *out_len,uint8_t *out_buf,
tuya_ble_parameters_settings_t *current_para_data,uint8_t *dev_rand)
{
    uint16_t len = 0;
    uint8_t key[16];
    bool key_ok = false;

    if(encryption_mode>=ENCRYPTION_MODE_MAX)
    {
        return 2;//
    }

    if(encryption_mode==ENCRYPTION_MODE_NONE)
    {
        memcpy(out_buf,in_buf,in_len);
        *out_len = in_len;
        return 0;
    }
    else
    {
        len = tuya_ble_Add_Pkcs(in_buf,in_len);
    }

    memset(key,0,sizeof(key));
    key_ok = tuya_ble_key_generate(encryption_mode,key,current_para_data,dev_rand);

    if(key_ok)
    {

        if(tuya_ble_aes128_cbc_encrypt(key,iv,in_buf,len,out_buf)==true)
        {
            *out_len = len;
            //  
            return 0;//
        }
        else
        {
            return 3;//
        }
    }
    else
    {
        TUYA_BLE_LOG_ERROR("key error!");
        return 4; //key failed
    }

}

/*********************************************************
FN: 
*/
static uint8_t tuya_ble_master_decryption(uint8_t const *in_buf,uint32_t in_len,uint32_t *out_len,uint8_t *out_buf,
    tuya_ble_parameters_settings_t *current_para_data,uint8_t *dev_rand)
{
    uint16_t len = 0;
    uint8_t key[16];
    uint8_t IV[16];
    uint8_t mode = 0;
    if(in_len<17)
    {
        return 1;  //
    }
    if(in_buf[0]>=ENCRYPTION_MODE_MAX)
    {
        return 2;//
    }

    if(in_buf[0]==ENCRYPTION_MODE_NONE)
    {
        len = in_len - 1;
        memcpy(out_buf,in_buf+1,len);
        *out_len = len;
        return 0;
    }

    len = in_len-17;

    memset(key,0,sizeof(key));
    memset(IV,0,sizeof(IV));
    mode = in_buf[0];

    if(mode==ENCRYPTION_MODE_KEY_1)
    {
        memcpy(service_rand,in_buf+1,16); //iv==rand
    }
    if(tuya_ble_key_generate(mode,key,current_para_data,dev_rand))
    {

        memcpy(IV,in_buf+1,16);

        if(tuya_ble_aes128_cbc_decrypt(key,IV,(uint8_t *)(in_buf+17),len,out_buf))
        {
 
            *out_len = len;
            return 0;//
        }
        else
        {
            return 3;//
        }
    }
    else
    {

        return 4; //
    }

}

/*********************************************************
FN: 
*/
static bool tuya_ble_device_id_key_generate(uint8_t *key_in, uint16_t key_len, uint8_t *key_out)
{
    uint16_t len = 0;

    uint8_t device_id_key_in_buffer[64] = {0};

    memset(device_id_key_in_buffer,0,sizeof(device_id_key_in_buffer));
    len = 0;

    memcpy(device_id_key_in_buffer+len,key_in,key_len);
    len += key_len;

    tuya_ble_md5_crypt(device_id_key_in_buffer,len,key_out);

    return true;
}

/*********************************************************
FN: 
*/
void tuya_ble_master_reset_ble_sn(void)
{
    tuya_ble_device_enter_critical();
    tuya_ble_receive_sn = 0;
    tuya_ble_send_sn = 1;
    tuya_ble_device_exit_critical();
}

/*********************************************************
FN: 
*/
uint8_t tuya_ble_master_commData_send(uint16_t cmd,uint32_t ack_sn,uint8_t *data,uint16_t len,uint8_t encryption_mode)
{
    TUYA_APP_LOG_INFO("current_cmd_tx: %04x", cmd);
//    TUYA_APP_LOG_HEXDUMP_INFO("tuya_ble_master_commData_send", data, len);
    mtp_ret ret;
    uint8_t send_len = 0;
    uint8_t p_buf[20];
    uint32_t err = 0;
    int8_t retries_cnt = 0;
    uint8_t iv[16];
    uint16_t rand_value = 0,i=0;
    uint16_t crc16 = 0;
    uint16_t en_len  = 0;
    uint32_t out_len = 0;
    uint32_t temp_len = 0;
	uint32_t package_number = 0;
    tuya_ble_master_r_air_send_packet air_send_packet;

    memset(&air_send_packet,0,sizeof(air_send_packet));

//    tuya_ble_connect_status_t currnet_connect_status = tuya_ble_connect_status_get();
// 
//    if((currnet_connect_status == BONDING_UNCONN)||(currnet_connect_status== UNBONDING_UNCONN))
//    {
//        TUYA_APP_LOG_INFO("tuya ble commData_send failed,because ble not in connect status.");
//        return 2;
//    }

    if((encryption_mode>=ENCRYPTION_MODE_MAX)||(len>(TUYA_BLE_AIR_FRAME_MAX-29)))
    {
        return 1;
    }

    //生成随机IV
    if(encryption_mode != ENCRYPTION_MODE_NONE)
    {
        for(i=0; i<16; i+=2)
        {
            rand_value = rand();
            iv[i+0] = rand_value>>8;
            iv[i+1] = rand_value;
        }
        en_len = 17;
    }
    else
    {
        en_len = 1;
        memset(iv,0,sizeof(iv));
    }
    
    air_send_packet.send_len = 14+len;

    if(air_send_packet.send_len%16==0)
    {
        temp_len = 0;
    }
    else
    {
        temp_len = 16 - air_send_packet.send_len%16;
    }

    temp_len += air_send_packet.send_len;

    if(temp_len>(TUYA_BLE_AIR_FRAME_MAX-en_len))
    {
        TUYA_APP_LOG_INFO("The length of the send to ble exceeds the maximum length.");
        air_send_packet.send_len = 0;
        return 1; //加密后数据加上加密头超过AIR_FRAME_MAX
    }
      
    air_send_packet.send_data = NULL;
       
    air_send_packet.send_data = (uint8_t *)tuya_ble_malloc(temp_len); //must temp_len
    
    if(air_send_packet.send_data==NULL)
    {
        TUYA_APP_LOG_INFO("air_send_packet.send_data malloc failed return 3.");
        air_send_packet.send_len = 0;
        return 3;
    }
    else
    {
        memset(air_send_packet.send_data,0,temp_len);
    }


    uint32_t send_sn = get_ble_send_sn();
    //利用send_data buffer缓存明文指令数据
    air_send_packet.send_data[0] = send_sn>>24;
    air_send_packet.send_data[1] = send_sn>>16;
    air_send_packet.send_data[2] = send_sn>>8;
    air_send_packet.send_data[3] = send_sn;

    air_send_packet.send_data[4] = ack_sn>>24;
    air_send_packet.send_data[5] = ack_sn>>16;
    air_send_packet.send_data[6] = ack_sn>>8;
    air_send_packet.send_data[7] = ack_sn;

    air_send_packet.send_data[8] = cmd>>8;
    air_send_packet.send_data[9] = cmd;

    air_send_packet.send_data[10] = len>>8;
    air_send_packet.send_data[11] = len;

    memcpy(&air_send_packet.send_data[12],data,len);

    crc16 = suble_util_crc16(air_send_packet.send_data,12+len, NULL);

    air_send_packet.send_data[12+len] = crc16>>8;
    air_send_packet.send_data[13+len] = crc16;
    

//    SUBLE_HEXDUMP("ble_commData_send plain data",(uint8_t*)air_send_packet.send_data,air_send_packet.send_len);//
  
    air_send_packet.encrypt_data_buf = NULL;
    
    air_send_packet.encrypt_data_buf = (uint8_t *)tuya_ble_malloc(temp_len+en_len);
    
    if(air_send_packet.encrypt_data_buf==NULL)
    {
        TUYA_APP_LOG_INFO("air_send_packet.encrypt_data_buf malloc failed.");
        tuya_ble_free(air_send_packet.send_data);
        return 3;
    }
    else
    {
        air_send_packet.encrypt_data_buf_len = 0;
        memset(air_send_packet.encrypt_data_buf,0,temp_len+en_len);
    }

         
    air_send_packet.encrypt_data_buf[0] = encryption_mode;

    if(encryption_mode != ENCRYPTION_MODE_NONE)
    {
        memcpy(&air_send_packet.encrypt_data_buf[1],iv,16);
    }

    if(tuya_ble_master_encryption(encryption_mode,iv,(uint8_t *)air_send_packet.send_data,air_send_packet.send_len,&out_len,
        (uint8_t *)(air_send_packet.encrypt_data_buf+en_len),&tuya_ble_current_para,tuya_ble_master_pair_rand)==0)
    {
        if(out_len!=temp_len)
        {
            TUYA_APP_LOG_ERROR("ble_commData_send encryed error."); 
            tuya_ble_free(air_send_packet.send_data);        
            tuya_ble_free(air_send_packet.encrypt_data_buf);        
            return 1;
        }

        air_send_packet.encrypt_data_buf_len = en_len + out_len;

//        TUYA_APP_LOG_HEXDUMP_INFO("ble_commData_send encryped data",(u8*)air_send_packet.encrypt_data_buf,air_send_packet.encrypt_data_buf_len);//
    }
    else
    {
        TUYA_APP_LOG_ERROR("ble_commData_send encryed fail."); 
        tuya_ble_free(air_send_packet.send_data);        
        tuya_ble_free(air_send_packet.encrypt_data_buf);        
        return 1;
    }
    
    tuya_ble_free(air_send_packet.send_data);
    package_number = 0;
    trsmitr_init(&ty_trsmitr_proc_send);
    do
    {
        ret = trsmitr_send_pkg_encode(&ty_trsmitr_proc_send,TUYA_BLE_PROTOCOL_VERSION_HIGN,(uint8_t *)(air_send_packet.encrypt_data_buf), air_send_packet.encrypt_data_buf_len);
        if (MTP_OK != ret && MTP_TRSMITR_CONTINUE != ret)
        {
            tuya_ble_free(air_send_packet.encrypt_data_buf);
            return 1;
        }
        send_len = get_trsmitr_subpkg_len(&ty_trsmitr_proc_send);
        memcpy(p_buf,get_trsmitr_subpkg(&ty_trsmitr_proc_send),send_len);
		package_number++;
		suble_svc_c_send_data(g_conn_info[1].conn_handle,p_buf,send_len);
    } while (ret == MTP_TRSMITR_CONTINUE);

//    TUYA_APP_LOG_INFO("ble_commData_send len = %d , package_number = %d , protocol version : 0x%02x , error code : 0x%02x",air_send_packet.encrypt_data_buf_len,package_number,TUYA_BLE_PROTOCOL_VERSION_HIGN,err);

    tuya_ble_free(air_send_packet.encrypt_data_buf);

    return 0;
}

/*********************************************************
FN: 
*/
void tuya_ble_master_commonData_rx_proc(uint8_t *buf, uint16_t len, uint8_t conn_id)
{
    uint8_t temp;
    uint32_t current_sn = 0;
    uint16_t current_cmd = 0;
    tuya_ble_evt_param_t evt;
    uint8_t *ble_evt_buffer=NULL;
    uint8_t current_encry_mode = 0;

    if(ble_data_unpack(buf,len))
    {
        return;      //
    }

    if(air_recv_packet.recv_len>TUYA_BLE_AIR_FRAME_MAX)
    {
        TUYA_APP_LOG_INFO("air_recv_packet.recv_len bigger than TUYA_BLE_AIR_FRAME_MAX.");
        tuya_ble_air_recv_packet_free();
        return;
    }

    if(ty_trsmitr_proc.version<2)  //协议主版本号低于2，不解析，返回。
    {
        TUYA_APP_LOG_INFO("ty_ble_rx_proc version not compatibility!");
        tuya_ble_air_recv_packet_free();
        return;
    }


    if(tuya_ble_current_para.sys_settings.bound_flag==1)//当前已绑定状态
    {
        if(ENCRYPTION_MODE_NONE==air_recv_packet.recv_data[0])
        {
            TUYA_APP_LOG_INFO("ty_ble_rx_proc data encryption mode error since bound_flag = 1.");
            tuya_ble_air_recv_packet_free();
            return;
        }
    }

    current_encry_mode = air_recv_packet.recv_data[0];

//    SUBLE_HEXDUMP("received encry data",(uint8_t*)air_recv_packet.recv_data,air_recv_packet.recv_len);//

    air_recv_packet.de_encrypt_buf = NULL;
    
    air_recv_packet.de_encrypt_buf = (uint8_t*)tuya_ble_malloc(air_recv_packet.recv_len);
    
    if(air_recv_packet.de_encrypt_buf==NULL)
    {
        TUYA_APP_LOG_INFO("air_recv_packet.de_encrypt_buf malloc failed.");
        tuya_ble_air_recv_packet_free();
        return;
    }
    else
    {
        air_recv_packet.decrypt_buf_len = 0;
        temp = tuya_ble_master_decryption((uint8_t *)air_recv_packet.recv_data,air_recv_packet.recv_len,&air_recv_packet.decrypt_buf_len,
        (uint8_t *)air_recv_packet.de_encrypt_buf,&tuya_ble_current_para,tuya_ble_master_pair_rand);
        tuya_ble_air_recv_packet_free();
    }
    
   
    if(temp != 0) //解密失败
    {
        TUYA_APP_LOG_INFO("ble receive data decryption error code = %d",temp);
        tuya_ble_free(air_recv_packet.de_encrypt_buf);
        return;
    }

//    SUBLE_HEXDUMP("decryped data",(uint8_t*)air_recv_packet.de_encrypt_buf,air_recv_packet.decrypt_buf_len);//解密数据
    //指令数据crc验证
    if(tuya_ble_master_cmd_data_crc_check((uint8_t *)air_recv_packet.de_encrypt_buf,air_recv_packet.decrypt_buf_len)!=0)
    {
        TUYA_APP_LOG_INFO("ble receive data crc check error!");
        tuya_ble_free(air_recv_packet.de_encrypt_buf);
        return;
    }

    //SN验证
    current_sn  = air_recv_packet.de_encrypt_buf[0]<<24;
    current_sn += air_recv_packet.de_encrypt_buf[1]<<16;
    current_sn += air_recv_packet.de_encrypt_buf[2]<<8;
    current_sn += air_recv_packet.de_encrypt_buf[3];

    if(current_sn<=tuya_ble_receive_sn)
    {
        TUYA_APP_LOG_INFO("ble receive SN error!");
        tuya_ble_gap_disconnect();//SN错误，断开蓝牙连接
        tuya_ble_free(air_recv_packet.de_encrypt_buf);
        return;
    }
    else
    {
        set_ble_receive_sn(current_sn);
    }

    current_cmd = ((air_recv_packet.de_encrypt_buf[8]<<8)|air_recv_packet.de_encrypt_buf[9]);


    if((s_master_bonding_connecting == false)
        &&(FRM_QRY_DEV_INFO_REQ != current_cmd)
        &&(PAIR_REQ != current_cmd)
        &&(FRM_LOGIN_KEY_REQ != current_cmd)
        &&(FRM_FACTORY_TEST_CMD != current_cmd)
        &&(FRM_NET_CONFIG_INFO_REQ != current_cmd)
        &&(FRM_ANOMALY_UNBONDING_REQ != current_cmd))
    {   //绑定前，不响应其它命令
        tuya_ble_free(air_recv_packet.de_encrypt_buf);
        TUYA_APP_LOG_INFO("ble receive cmd error on current bond state!");
        return;
    }


    tuya_ble_ota_status_t tuya_ble_ota_status_get(void);
    if(tuya_ble_ota_status_get()!=TUYA_BLE_OTA_STATUS_NONE)
    {   //OTA状态下，不处理其它事件
        if(!((current_cmd>=FRM_OTA_START_REQ)&&(current_cmd<=FRM_OTA_END_REQ)))
        {
            tuya_ble_free(air_recv_packet.de_encrypt_buf);
            TUYA_APP_LOG_INFO("ble receive cmd error on ota state!");
            return;
        }
    }

    ble_evt_buffer=(uint8_t*)tuya_ble_malloc(air_recv_packet.decrypt_buf_len+3);
    if(ble_evt_buffer==NULL)
    {
        TUYA_APP_LOG_INFO("ty_ble_rx_proc no mem.");
        tuya_ble_free(air_recv_packet.de_encrypt_buf);
        return;
    }
    else
    {
        memset(ble_evt_buffer,0,air_recv_packet.decrypt_buf_len+1);
    }
    
    ble_evt_buffer[0] = current_encry_mode;     //首字节拷贝加密方式，便于后续使用
    ble_evt_buffer[1] = current_cmd>>8;
    ble_evt_buffer[2] = current_cmd;
    memcpy(ble_evt_buffer+3,(uint8_t *)air_recv_packet.de_encrypt_buf,air_recv_packet.decrypt_buf_len);
    
//    tuya_ble_app_evt_send_with_data(APP_EVT_MASTER_RX_FROM_SLAVE_DECRYPT, ble_evt_buffer, air_recv_packet.decrypt_buf_len+3);
    tuya_ble_master_receive_data_from_slave_decrypt(ble_evt_buffer, air_recv_packet.decrypt_buf_len+3);
    
//    evt.hdr.event = TUYA_BLE_EVT_BLE_CMD;
//    evt.ble_cmd_data.cmd = current_cmd;
//    evt.ble_cmd_data.p_data = ble_evt_buffer;
//    evt.ble_cmd_data.data_len = air_recv_packet.decrypt_buf_len+1;
//    TUYA_APP_LOG_INFO("BLE EVENT SEND-CMD:0x%02x - LEN:0x%02x",current_cmd,air_recv_packet.decrypt_buf_len+1);

//    if(tuya_ble_event_send(&evt)!=0)
//    {
//        TUYA_APP_LOG_INFO("ble event send fail!");
//        tuya_ble_free(ble_evt_buffer);
//    }
    
    
    tuya_ble_free(ble_evt_buffer);
    tuya_ble_free(air_recv_packet.de_encrypt_buf);
}




/*********************************************************
FN: 
*/
static uint32_t lock_slave_info_save(slave_info_t* slave)
{
//    TUYA_APP_LOG_INFO("s_slave_nvid_max_num: %d", s_slave_nvid_max_num);
    //masterid存在，更新
    for(uint32_t idx=0; idx<s_slave_nvid_max_num; idx++) {
        if(p_slave_info[idx].id == slave->id) {
            memcpy(&p_slave_info[idx], slave, sizeof(slave_info_t));
            return tuya_ble_master_port_nv_write(SF_AREA_4, s_slave_nvid_base+idx, &p_slave_info[idx], sizeof(slave_info_t));
        }
    }
    
    //masterid不存在，新建
    for(uint32_t idx=0; idx<s_slave_nvid_max_num; idx++) {
        if(p_slave_info[idx].id == 0x00) {
            memcpy(&p_slave_info[idx], slave, sizeof(slave_info_t));
            return tuya_ble_master_port_nv_write(SF_AREA_4, s_slave_nvid_base+idx, &p_slave_info[idx], sizeof(slave_info_t));
        }
    }
    
    //存储满
	return APP_PORT_ERROR_COMMON;
}

/*********************************************************
FN: 
*/
static uint32_t lock_slave_info_load(void)
{
    for(uint32_t idx=0; idx<s_slave_nvid_max_num; idx++) {
        tuya_ble_master_port_nv_read(SF_AREA_4, s_slave_nvid_base+idx, &p_slave_info[idx], sizeof(slave_info_t));
    }
	return APP_PORT_SUCCESS;
}

/*********************************************************
FN: 
*/
static uint32_t lock_slave_info_delete(uint16_t slaveid)
{
    for(uint32_t idx=0; idx<s_slave_nvid_max_num; idx++) {
        if(p_slave_info[idx].id == slaveid) {
            memset(&p_slave_info[idx], 0, sizeof(slave_info_t));
            return tuya_ble_master_port_nv_delete(SF_AREA_4, s_slave_nvid_base+idx);
        }
    }
    
    //未找到
	return APP_PORT_ERROR_COMMON;
}

/*********************************************************
FN: 
*/
static int32_t lock_slave_info_find_index_by_device_id(void* device_id)
{
    for(uint32_t idx=0; idx<s_slave_nvid_max_num; idx++) {
        if(memcmp(p_slave_info[idx].device_id, device_id, DEVICE_ID_LEN) == 0) {
            return idx;
        }
    }
    return -1;
}

/*********************************************************
FN: 
*/
static int32_t lock_slave_info_find_index_by_slaveid(uint32_t slaveid)
{
    for(uint32_t idx=0; idx<s_slave_nvid_max_num; idx++) {
        if(p_slave_info[idx].id == slaveid) {
            return idx;
        }
    }
    return -1;
}

/*********************************************************
FN: 
*/
static uint32_t lock_master_info_save(void)
{
	return tuya_ble_master_port_nv_write(SF_AREA_4, NV_ID_MASTER_INFO, &s_master_info, sizeof(master_info_t));
}

/*********************************************************
FN: 
*/
static uint32_t lock_master_info_load(void)
{
	return tuya_ble_master_port_nv_read(SF_AREA_4, NV_ID_MASTER_INFO, &s_master_info, sizeof(master_info_t));
}

/*********************************************************
FN: 
*/
tuya_ble_status_t tuya_ble_master_info_init(slave_info_t* info, uint8_t slave_max_num)
{
    if(info == NULL) {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }
    
    if((slave_max_num == 0) || (slave_max_num >100)) {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }
    
    p_slave_info                  = info;
    s_slave_nvid_max_num          = slave_max_num;
    
    lock_master_info_load();
    lock_slave_info_load();
    
    tuya_ble_master_connect_monitor_timer_init();
    
    return TUYA_BLE_SUCCESS;
}




/*********************************************************
FN: 0. 配件和手机的透传通道，用于配件获取锁的信息
*/
void tuya_ble_master_data_passthrough_with_phone(void* buf, uint32_t size)
{
    data_passthrough_t* cmd = buf;
    TUYA_APP_LOG_HEXDUMP_INFO("data passthrough", (void*)cmd, size);
    
    data_passthrough_result_t rsp;
    memcpy(&rsp, cmd, sizeof(data_passthrough_result_t));
    rsp.result = 0x00;
    
    suble_util_reverse_byte(&cmd->masterid, 2);
    suble_util_reverse_byte(&cmd->slaveid, 2);
    
    //参数检查
    if(cmd->operation > 1) {
        rsp.result = 0x02;
        goto DATA_PASS_THROUGH;
    }
    
    if(cmd->operation == 0) {
        //解密锁的login key
        uint8_t key[16] = {0};
        uint8_t iv[16] = {0};
        uint8_t decrypt_login_key[16] = {0};
        memcpy(key, tuya_ble_current_para.sys_settings.login_key, LOGIN_KEY_LEN);
        tuya_ble_aes128_cbc_decrypt(key, iv, cmd->login_key, 16, decrypt_login_key);
//        TUYA_APP_LOG_HEXDUMP_INFO("decrypt_login_key", decrypt_login_key, 16);
        
        //存储从机（锁）的信息
        slave_info_t slave;
        slave.id = cmd->slaveid;
        memcpy(slave.device_id, cmd->device_id, DEVICE_ID_LEN);
        memcpy(slave.login_key, decrypt_login_key, LOGIN_KEY_LEN);
        if(lock_slave_info_save(&slave) != APP_PORT_SUCCESS) {
            rsp.result = 0x03;  //存储满
        }
        
        //存储主机（配件）信息
        s_master_info.id = cmd->masterid;
        memcpy(s_master_info.srand, cmd->random, MASTER_RANDOM_NUM_LEN);
        lock_master_info_save();
    }
    else {
        if(lock_slave_info_delete(cmd->slaveid) != APP_PORT_SUCCESS) {
            rsp.result = 0x07; //数据不存在
        }
        else {
            //删除从机相关信息
            tuya_ble_master_port_delete_slave_info(cmd->slaveid);
        }
    }
    
DATA_PASS_THROUGH:
    if(rsp.result != 0x00) {
        TUYA_APP_LOG_INFO("set_master_srand_num result: %d", rsp.result);
    }
    tuya_ble_data_passthrough((void*)&rsp, sizeof(data_passthrough_result_t));
    TUYA_APP_LOG_HEXDUMP_INFO("rsp", (void*)&rsp, sizeof(data_passthrough_result_t));
}

/*********************************************************
FN: 1. 开始扫描
*/
tuya_ble_status_t tuya_ble_master_scan_start(int32_t slaveid, tuya_ble_master_evt_handler_t handler)
{
    if(handler == NULL) {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }
    
    s_slaveid_tobe_scan = slaveid;
    s_tuya_ble_master_evt_handler = handler;
    
    tuya_ble_master_port_evt_register(suble_master_evt_handler);
    tuya_ble_master_port_scan_start(suble_scan_result_handler);
    
    tuya_ble_master_connect_monitor_timer_start();
    
    return TUYA_BLE_SUCCESS;
}

/*********************************************************
FN: 2. 扫描到目标设备，发起连接
*/
static uint8_t s_adv_mac[6];
static uint8_t s_adv_data[31];
static uint8_t key[16];
static uint8_t device_id_scan[16];
static uint8_t device_id_scan_invalid[16] = {0x00};
static void tuya_ble_master_scan_finish_handler(ble_gap_evt_adv_report_t const* adv)
{
    //adv data
    if((adv->data.p_data[5] == 0x01) && (adv->data.p_data[6] == 0xA2))
    {
        memcpy(s_adv_mac, adv->peer_addr.addr, 6);
        memcpy(s_adv_data, adv->data.p_data, 31);
    }
    
    //scan rsp
    if(((adv->data.p_data[6] == 0xD0) && (adv->data.p_data[7] == 0x07)))
    {
        //地址一致
        if(memcmp(s_adv_mac, adv->peer_addr.addr, 6) == 0)
        {
            //解密device_id（加解密方式和APP端商定）
            tuya_ble_device_id_key_generate(&s_adv_data[12], 8, key);
            tuya_ble_aes128_cbc_decrypt(key, key, &adv->data.p_data[14], DEVICE_ID_LEN, device_id_scan);
            
            if(memcmp(device_id_scan, device_id_scan_invalid, 16) == 0) {
                return;
            }
            
            //扫描到的从机存储序号
            s_current_slave_index = lock_slave_info_find_index_by_device_id(device_id_scan);
            if(s_current_slave_index >= 0) {
                int32_t slave_index = lock_slave_info_find_index_by_slaveid(s_slaveid_tobe_scan);
                if((s_slaveid_tobe_scan < 0)
                    || ((slave_index >= 0) && (slave_index == s_current_slave_index))) {
                    //记录 mac 地址
                    tuya_ble_app_evt_send_with_data(APP_EVT_MASTER_SAVE_SLAVE_MAC, (void*)adv->peer_addr.addr, 6);
                    TUYA_APP_LOG_HEXDUMP_INFO("mac", (void*)adv->peer_addr.addr, 6);
                    TUYA_APP_LOG_INFO("s_current_slave_index: %d", s_current_slave_index);
                    
                    //停止扫描并发起连接
                    tuya_ble_master_port_scan_stop();
                    tuya_ble_master_port_connect((void*)&adv->peer_addr, suble_connect_result_handler);
                }
                else {
                    s_tuya_ble_master_evt_handler(TUYA_BLE_MASTER_EVT_SLAVEID_INVALID, NULL, 0);
                    tuya_ble_master_connect_monitor_timer_stop();
                }
            }
        }
    }
}

/*********************************************************
FN: 3. 发现目标服务后，向从机发送数据（获取设备信息）
*/
static void tuya_ble_master_discovery_svc_complete_handler(void)
{
    tuya_ble_master_commData_send(FRM_QRY_DEV_INFO_REQ, 0, NULL, 0, ENCRYPTION_MODE_KEY_4);
}

/*********************************************************
FN: 4. 收到从机返回的数据（加密）
*/
static void tuya_ble_master_receive_data_from_slave(void* buf, uint32_t size)
{
    tuya_ble_master_commonData_rx_proc(buf, size, 0);
}

/*********************************************************
FN: 4. 收到从机返回的数据（解密）
*/
static void tuya_ble_master_receive_data_from_slave_decrypt(void* buf, uint32_t size)
{
    uint8_t* data = buf;
    uint8_t current_encry_mode = data[0];
    uint16_t current_cmd = (data[1]<<8) + data[2];
    
    tuya_ble_protocol_t* protocol = (void*)&data[3];
    
    TUYA_APP_LOG_INFO("current_cmd_rx: %04x", current_cmd);
    switch(current_cmd)
    {
        case FRM_QRY_DEV_INFO_RESP: {
            device_info_result_t* device_info = (void*)protocol->value;
            
            //暂存配对随机数
            memcpy(tuya_ble_master_pair_rand, device_info->srand, PAIR_RANDOM_LEN);
            
            //配对请求
            bond_request_t bond_request;
            memcpy(bond_request.device_id, p_slave_info[s_current_slave_index].device_id, DEVICE_ID_LEN);
            memcpy(bond_request.login_key, p_slave_info[s_current_slave_index].login_key, LOGIN_KEY_LEN);
            memcpy(bond_request.device_virtual_id, device_info->device_virtual_id, DEVICE_VIRTUAL_ID_LEN);
            tuya_ble_master_commData_send(PAIR_REQ, 0, (void*)&bond_request, sizeof(bond_request_t), ENCRYPTION_MODE_SESSION_KEY);
        } break;
        
        //配对成功
        case PAIR_RESP: {
            if(protocol->value[0] == 0x02) {
                s_master_bonding_connecting = true;
                
                s_tuya_ble_master_evt_handler(TUYA_BLE_MASTER_EVT_BONDING, NULL, 0);
            }
        } break;
        
        case FRM_CMD_RESP: {
            //dp指令下发成功
        } break;
        
        case FRM_STATE_QUERY_RESP: {
        } break;
        
        case FRM_ANOMALY_UNBONDING_RESP: {
        } break;
        
        case FRM_STAT_REPORT: {
            master_dp_t* rsp = (void*)protocol->value;
            
            open_with_master_record_report_info_t info;
            info.slaveid = p_slave_info[s_current_slave_index].id;
            suble_util_reverse_byte(&rsp->open_with_master_result.timestamp, sizeof(uint32_t));
            info.timestamp = rsp->open_with_master_result.timestamp;
            
            if(rsp->open_with_master_result.result == 0x00) {
                s_tuya_ble_master_evt_handler(TUYA_BLE_MASTER_EVT_OPEN_WITH_MASTER_SUCCESS, (void*)&info, sizeof(open_with_master_record_report_info_t));
            }
            else {
                s_tuya_ble_master_evt_handler(TUYA_BLE_MASTER_EVT_OPEN_WITH_MASTER_FAILURE, (void*)&info, sizeof(open_with_master_record_report_info_t));
            }
            TUYA_APP_LOG_HEXDUMP_INFO("dp_rsp", (void*)rsp, rsp->dp_data_len+3);
        } break;
        
        case FRM_STAT_WITH_TIME_REPORT: {
            TUYA_APP_LOG_HEXDUMP_INFO("dp_rsp_timestamp", (void*)&data[3+12+5], data[3+12+5+2]+3);
        } break;
        
        case FRM_GET_UNIX_TIME_CHAR_MS_REQ: {
            //请求时间戳不处理
        } break;
        
        default: {
        } break;
    }
}

/*********************************************************
FN: 5. 向从机发送普通dp信息
*/
tuya_ble_status_t tuya_ble_master_send_data(void* buf, uint32_t size)
{
    TUYA_APP_LOG_HEXDUMP_INFO("dp_cmd", buf, size);
    return tuya_ble_master_commData_send(FRM_CMD_SEND, 0, buf, size, ENCRYPTION_MODE_SESSION_KEY);
}

/*********************************************************
FN: 6. 向从机发送开门指令（对5的封装）
*/
tuya_ble_status_t tuya_ble_master_open_with_master(uint8_t operation, uint8_t open_meth, void* open_meth_info, uint8_t open_meth_info_size)
{
    s_master_cmd.dp_id = 71;
    s_master_cmd.dp_type = APP_PORT_DT_RAW;
    s_master_cmd.dp_data_len = 2+2+8+1+4+1+open_meth_info_size;
    
    s_master_cmd.open_with_master.masterid = s_master_info.id;
    s_master_cmd.open_with_master.slaveid = p_slave_info[s_current_slave_index].id;
    suble_util_reverse_byte(&s_master_cmd.open_with_master.masterid, 2);
    suble_util_reverse_byte(&s_master_cmd.open_with_master.slaveid, 2);
    memcpy(s_master_cmd.open_with_master.srand, s_master_info.srand, MASTER_RANDOM_NUM_LEN);
    s_master_cmd.open_with_master.operation = operation;
    s_master_cmd.open_with_master.timestamp = app_port_get_timestamp();
    suble_util_reverse_byte(&s_master_cmd.open_with_master.timestamp, 4);
    s_master_cmd.open_with_master.open_meth = open_meth;
    memcpy(s_master_cmd.open_with_master.open_meth_info, open_meth_info, open_meth_info_size);
    
    return tuya_ble_master_send_data((void*)&s_master_cmd, (3 + s_master_cmd.dp_data_len));
}




/*********************************************************
FN: 
*/
static void suble_scan_result_handler(uint32_t evt, uint8_t* buf, uint32_t size)
{
    switch (evt)
    {
        case SUBLE_SCAN_EVT_ADV_REPORT: {
            tuya_ble_master_scan_finish_handler((void*)buf);
        } break;
        
        case SUBLE_SCAN_EVT_SCAN_TIMEOUT: {
            s_tuya_ble_master_evt_handler(TUYA_BLE_MASTER_EVT_SCAN_TIMEOUT, NULL, 0);
            tuya_ble_master_connect_monitor_timer_stop();
        } break;
        
        default: {
        } break;
    }
}

/*********************************************************
FN: 
*/
static void suble_connect_result_handler(uint32_t evt, uint8_t* buf, uint32_t size)
{
    switch (evt)
    {
        case SUBLE_GAP_EVT_CONNECTED: {
            tuya_ble_master_reset_ble_sn();
            
            tuya_ble_master_port_db_discovery_start(suble_svc_result_handler);
//            if(s_current_slave_index != s_last_slave_index) {
//                tuya_ble_master_port_db_discovery_start(suble_svc_result_handler);
//            }
//            else {
//                tuya_ble_master_port_notify_enable();
//                tuya_ble_master_discovery_svc_complete_handler();
//            }
//            s_last_slave_index = s_current_slave_index;
        } break;
        
        case SUBLE_GAP_EVT_DISCONNECTED: {
            s_master_bonding_connecting = false;
            tuya_ble_master_reset_ble_sn();
            tuya_ble_air_recv_packet_free();
            
            s_tuya_ble_master_evt_handler(TUYA_BLE_MASTER_EVT_DISCONNECT, NULL, 0);
            tuya_ble_master_connect_monitor_timer_stop();
            
        } break;
        
        case SUBLE_GAP_EVT_CONNECT_TIMEOUT: {
            s_tuya_ble_master_evt_handler(TUYA_BLE_MASTER_EVT_CONNECT_TIMEOUT, NULL, 0);
            tuya_ble_master_connect_monitor_timer_stop();
        } break;
        
        default: {
        } break;
    }
}

/*********************************************************
FN: 
*/
static void suble_svc_result_handler(uint32_t evt, uint8_t* buf, uint32_t size)
{
    switch (evt)
    {
        case SUBLE_SVC_C_EVT_DISCOVERY_COMPLETE: {
            tuya_ble_master_discovery_svc_complete_handler();
        } break;
        
        case SUBLE_SVC_C_EVT_RECEIVE_DATA_FROM_SLAVE: {
            tuya_ble_master_receive_data_from_slave(buf, size);
        } break;
        
        default: {
        } break;
    }
}

/*********************************************************
FN: 
*/
static void suble_master_evt_handler(uint32_t evt, uint8_t* buf, uint32_t size)
{
    switch (evt)
    {
        case TUYA_BLE_APP_EVT_MASTER_BLE_CMD: {
//            tuya_ble_master_receive_data_from_slave_decrypt(buf, size);
        } break;
        
        case TUYA_BLE_APP_EVT_MASTER_SAVE_SLAVE_MAC: {
            if(memcmp(p_slave_info[s_current_slave_index].mac, buf, 6) != 0) {
                memcpy(p_slave_info[s_current_slave_index].mac, buf, 6);
                lock_slave_info_save(&p_slave_info[s_current_slave_index]);
            }
        } break;
        
        default: {
        } break;
    }
}




tuya_ble_timer_t tuya_ble_master_timer_connect_monitor;

/*********************************************************
FN: 
*/
static void tuya_ble_master_timer_conncet_monitor_callback(tuya_ble_timer_t timer)
{
    tuya_ble_master_port_disconnect();
    s_tuya_ble_master_evt_handler(TUYA_BLE_MASTER_EVT_TIMEOUT, NULL, 0);
}

/*********************************************************
FN: 
*/
static void tuya_ble_master_connect_monitor_timer_init(void)
{
    if(tuya_ble_timer_create(&tuya_ble_master_timer_connect_monitor, 15000, TUYA_BLE_TIMER_SINGLE_SHOT, tuya_ble_master_timer_conncet_monitor_callback) != TUYA_BLE_SUCCESS) {
        TUYA_BLE_LOG_ERROR("tuya_ble_master_timer_connect_monitor creat failed");
    }
}

/*********************************************************
FN: 
*/
static void tuya_ble_master_connect_monitor_timer_start(void)
{
    if(tuya_ble_timer_start(tuya_ble_master_timer_connect_monitor) != TUYA_BLE_SUCCESS) {
        TUYA_BLE_LOG_ERROR("tuya_ble_master_timer_connect_monitor start failed");
    }
}

/*********************************************************
FN: 
*/
static void tuya_ble_master_connect_monitor_timer_stop(void)
{
    if(tuya_ble_timer_stop(tuya_ble_master_timer_connect_monitor) != TUYA_BLE_SUCCESS) {
        TUYA_BLE_LOG_ERROR("tuya_ble_master_timer_connect_monitor stop failed");
    }
}


























