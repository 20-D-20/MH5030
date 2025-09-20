#ifndef ___UI_MANAGER_H__
#define ___UI_MANAGER_H__

#include <stdlib.h>
#include "key.h"
#include "tps02r.h"
#include <stdio.h>
#include "ssd1305.h"
#include "FM24CXX.h"

#ifdef __cplusplus
extern "C" {
#endif// __cplusplus

/**
 * @brief      ϵͳ״̬ö��
 * @retval     None
 */
typedef enum 
{
    MODE_BROWSE,                                                   /* ���ģʽ */
    MODE_EDIT                                                      /* �༭ģʽ */
} SystemMode_e;

/**
 * @brief      ҳ��IDö��
 * @retval     None
 */
typedef enum 
{
    PAGE_STARTUP = 0,                                              /* ����ҳ�� */
    PAGE_TEMP_DISPLAY,                                             /* �¶���ʾҳ�� */
    PAGE_GUN_SETTING,                                              /* ǹ���¶�����ҳ�� */
    PAGE_CAVITY_SETTING,                                           /* ǻ���¶�����ҳ�� */
    PAGE_MAX                                                       /* ҳ������ */
} PageID_e;

/**
 * @brief      ��Ϣ����ö��
 * @retval     None
 */
typedef enum 
{
    MSG_PAGE_CHANGE,                                               /* ҳ���л� */
    MSG_VALUE_UPDATE,                                              /* ��ֵ���� */
    MSG_MODE_CHANGE,                                               /* ģʽ�л� */
    MSG_REALTIME_UPDATE                                            /* ʵʱ���ݸ��� */
} MsgType_e;

/**
 * @brief      ��Ϣ�ṹ��
 * @retval     None
 */
typedef struct 
{
    MsgType_e msg_type;                                            /* ��Ϣ���� */
    uint8_t page_id;                                               /* ҳ��ID */
    int16_t new_value;                                             /* ����ֵ */
    uint8_t refresh_type;                                          /* ˢ�����ͣ�0-��ֵ��1-ȫˢ�� */
} UIMessage_t;

/**
 * @brief      ҳ�����ݽṹ
 * @retval     None
 */
typedef struct 
{
    uint8_t page_id;                                               /* ҳ��ID */
    bool is_editable;                                              /* �Ƿ�ɱ༭ */
    int16_t current_value;                                         /* ��ǰֵ */
    int16_t min_value;                                             /* ��Сֵ */
    int16_t max_value;                                             /* ���ֵ */
    int16_t step;                                                  /* ����ֵ */
} PageData_t;

extern SystemMode_e g_current_mode;
extern uint8_t g_current_page_id;
extern PageData_t g_pages[PAGE_MAX];
extern int16_t g_gun_temp_measured ;                              // ǹ�ܲ����¶�
extern int16_t g_cavity_temp_measured;                            // ǻ������¶�


void init_page_data(void);
void get_test_temperatures(int16_t *gun_temp, int16_t *cavity_temp);


#ifdef __cplusplus
}
#endif 

#endif
