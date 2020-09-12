#include "app_active_report.h"

/*
�ļ�˵����
    Ϊ������ֲ�����ļ������ݲ�����app_port���ϲ�ķ�װ��׼��
ʹ�ó�����
    ���豸��������״̬����Ҫ�����ϱ�����ʱ������app_active_report_start()�������������豸���豸���ߺ󼴿ɽ��������ϱ���
    �豸�ϱ���ɺ����app_active_report_finished_and_disconnect()�Ͽ����ӡ�
��ֲ���̣�
    1. ��lock_timer.c/h�ļ��д�����LOCK_TIMER_ACTIVE_REPORT����ʱ�������ڳ�ʱ��ָ��㲥�����
    2. ��case��TUYA_BLE_CB_EVT_CONNECTE_STATUS���е���app_active_report_stop(ACTICE_REPORT_STOP_STATE_BONDING)�����ڰ󶨺�رն�ʱ��;
    3. ��case��APP_EVT_BLE_GAP_EVT_DISCONNECTED���е���app_active_report_finished_and_disconnect_handler()�����������ϱ���ɺ�ָ��ֳ���
*/


/*********************************************************************
 * LOCAL CONSTANTS
 */

/*********************************************************************
 * LOCAL STRUCT
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
static volatile bool s_active_report_running_flag = false;
static volatile bool s_active_report_bonding_flag = false;

/*********************************************************************
 * LOCAL FUNCTION
 */

/*********************************************************************
 * VARIABLES
 */




/*********************************************************
FN: 
*/
void app_active_report_start(void)
{
    if(g_isAdvtising) {
        //�ı�㲥���
        g_adv_param.adv_interval_max = APP_ACTIVE_REPORT_ADV_INTERVAL;
        g_adv_param.adv_interval_min = APP_ACTIVE_REPORT_ADV_INTERVAL;
        suble_adv_param_set();
        
        //�ı��־λ
        g_scan_rsp.value[8] |= 0x01;
        suble_adv_update_advDataAndScanRsp();
        
        //������ʱ��
        lock_timer_start(LOCK_TIMER_ACTIVE_REPORT);
        
        s_active_report_running_flag = true;
        TUYA_APP_LOG_INFO("app_active_report_start");
    }
    else {
        TUYA_APP_LOG_INFO("device is not advertising");
    }
}

/*********************************************************
FN: 
*/
void app_active_report_stop(uint8_t state)
{
    if(s_active_report_running_flag) {
        if(state == ACTICE_REPORT_STOP_STATE_OUTTIME) {
            if(g_isAdvtising)
            {
                //�ָ��㲥���
                g_adv_param.adv_interval_max = (uint16_t)TUYA_ADV_INTERVAL;
                g_adv_param.adv_interval_min = (uint16_t)TUYA_ADV_INTERVAL;
                suble_adv_param_set();
                
                TUYA_APP_LOG_INFO("app_active_report_stop: 30s is timeout");
            }
            else {
                TUYA_APP_LOG_INFO("device is not advertising");
            }
        }
        else if(state == ACTICE_REPORT_STOP_STATE_BONDING) {
            //�رն�ʱ��
            lock_timer_stop(LOCK_TIMER_ACTIVE_REPORT);
            
            s_active_report_bonding_flag = true;
            TUYA_APP_LOG_INFO("app_active_report_stop: device is bonding");
        }
    }
    else {
//        TUYA_APP_LOG_INFO("app_active_report is not running");
    }
}

/*********************************************************
FN: 
*/
void app_active_report_finished_and_disconnect(void)
{
    app_port_ble_gap_disconnect();
}

/*********************************************************
FN: 
*/
void app_active_report_finished_and_disconnect_handler(void)
{
    if(s_active_report_running_flag && s_active_report_bonding_flag) {
        //�ָ��㲥���
        g_adv_param.adv_interval_max = (uint16_t)TUYA_ADV_INTERVAL;
        g_adv_param.adv_interval_min = (uint16_t)TUYA_ADV_INTERVAL;
        suble_adv_param_set();
        
        //�ָ���־λ
        g_scan_rsp.value[8] &= 0xFE;
        suble_adv_update_advDataAndScanRsp();
        
        s_active_report_running_flag = false;
        s_active_report_bonding_flag = false;
        TUYA_APP_LOG_INFO("app_active_report_finished_and_disconnect_handler");
    }
    else {
//        TUYA_APP_LOG_INFO("app_active_report is not running or device is not bonding");
    }
}


















