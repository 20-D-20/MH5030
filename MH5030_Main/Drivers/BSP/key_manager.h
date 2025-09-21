#ifndef __KEY_MANAGER_H
#define __KEY_MANAGER_H

#include "main.h"
#include "key.h"
#include "ui_manager.h"
#include "pid_manager.h"
#include "cmsis_os.h"
#include "queue.h"

/* ����������� */
#define OK_KEY_COUNT_MAX    7       // OK��������������������
#define OK_KEY_TIMEOUT      2000    // ������ʱʱ��(ms)

/* �����������ṹ�� */
typedef struct {
    uint8_t ok_press_count;        // OK���������´���
    uint32_t ok_first_press_tick;  // ��һ�ΰ��µ�ʱ���
    uint8_t last_key;               // ��һ�ΰ���ֵ
    uint8_t key_count;              // ������������
} KeyManager_t;

/* ȫ�ֱ������� */
extern KeyManager_t g_key_manager;
extern QueueHandle_t UI_Queue;
extern SemaphoreHandle_t Data_Mutex;

/* �������� */
void Key_Manager_Init(void);
void Process_Browse_Mode_Key(uint8_t key);
void Process_Edit_Mode_Key(uint8_t key);
void Check_OK_Multiple_Press(void);
void Send_UI_Message(MsgType_e type, uint8_t page_id, int16_t value, uint8_t refresh_type);
void Handle_Page_Navigation(uint8_t key);
void Handle_Value_Adjustment(uint8_t key);
void Save_Temperature_Setting(uint8_t page_id, int16_t value);

#endif /* __KEY_MANAGER_H */

