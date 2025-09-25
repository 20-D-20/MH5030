#ifndef __KEY_MANAGER_H__
#define __KEY_MANAGER_H__

#include "main.h"
#include "key.h"
#include "ui_manager.h"
#include "pid_manager.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "cmsis_os.h"

#ifdef __cplusplus
extern "C" {
#endif
    
/* ����������� */
#define OK_KEY_COUNT_MAX    7       /* OK�������������������� */
#define OK_KEY_TIMEOUT      10000   /* ������ʱʱ��(ms) - 10s */

#define ESC_KEY_COUNT_MAX   2       /* OK�������������������� */
#define ESC_KEY_TIMEOUT     1000    /* ������ʱʱ��(ms)- 1s */
    
/* �������������� */
typedef struct {
    uint8_t  ok_press_count;        /* OK���������¼��� */
    uint32_t last_press_time;       /* �ϴΰ���ʱ�� */
    uint8_t  autotune_triggered;    /* �������Ѵ�����־ */
    uint8_t  esc_press_count;       /* ESC���������¼��� */        
} KeyPressCounter_t;

/* ȫ�ֱ������� */
extern QueueHandle_t UI_Queue;
extern SemaphoreHandle_t Data_Mutex;
extern KeyPressCounter_t g_stOkCntrAutotune;
extern KeyPressCounter_t g_stEscCntrGun;

/* �������� */
void Key_Manager_Init(void);
void Process_Browse_Mode_Key(uint8_t key);
void Process_Edit_Mode_Key(uint8_t key);
void Process_Autotune_Key(uint8_t key);
void Process_Gun_Select_Mode_Key(uint8_t key);
void Handle_Page_Navigation(uint8_t key);
void Handle_Value_Adjustment(uint8_t key);
void Handle_Gun_Select(uint8_t key);
void Check_Autotune_Trigger(void);
void Check_GunSelect_Trigger(void);
void Reset_Key_Counter(KeyPressCounter_t *key_counter);
void Send_UI_Message(MsgType_e type, uint8_t page_id, int16_t value, uint8_t refresh);

#ifdef __cplusplus
}
#endif

#endif /* __KEY_MANAGER_H__ */


