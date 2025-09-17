#ifndef __TEMP_MONITOR_H__
#define __TEMP_MONITOR_H__
#ifdef __cplusplus
extern "C" {
#endif// __cplusplus

#include "ssd1305.h"
#include "key.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

/* ϵͳ�������� */
#define TEMP_RANGE_MIN      0.0f        // �¶ȷ�Χ��Сֵ
#define TEMP_RANGE_MAX      220.0f      // �¶ȷ�Χ���ֵ
#define TEMP_STEP           0.1f        // �¶ȵ��ڲ���
#define SAVE_DISPLAY_TIME   2000        // ����ɹ���ʾ��ʾʱ��(ms)

/* ˫���� */
typedef struct {
    uint8_t need_full_refresh;  // ��Ҫȫ��ˢ�±�־
    uint8_t last_page_type;     // �ϴε�ҳ������
} DisplayBuffer_t;


/* ҳ�����Ͷ��� */
typedef enum {
    PAGE_FRONT_TEMP = 0,                // ǰǹ���¶���ʾҳ
    PAGE_REAR_TEMP,                     // ��ǻ���¶���ʾҳ  
    PAGE_FRONT_SET,                     // ǰǹ���¶�����ҳ
    PAGE_REAR_SET,                      // ��ǻ���¶�����ҳ
    PAGE_MAX_NUM
} PageType_t;

/* ϵͳ״̬���� */
typedef enum {
    STATE_DISPLAY = 0,                  // ��ʾģʽ
    STATE_SETTING                       // ����ģʽ
} SystemState_t;

/* �¶����ݽṹ */
typedef struct {
    float current;                      // ��ǰ�¶�
    float target;                       // Ŀ���¶�
    float temp_set;                     // ��ʱ����ֵ
} TempData_t;

/* ϵͳ���ƽṹ */
typedef struct {
    PageType_t current_page;            // ��ǰҳ��
    SystemState_t state;                // ϵͳ״̬
    TempData_t front_gun;               // ǰǹ���¶�
    TempData_t rear_chamber;            // ��ǻ���¶�
    uint8_t refresh_flag;               // ˢ�±�־
    uint8_t save_flag;                  // �����־
    uint32_t save_timer;                // ������ʾ��ʱ��
    uint32_t update_counter;            // ���¼�����
} TempMonitor_t;

/* �������� */
void TempMonitor_Init(void);
void TempMonitor_Process(void);
void TempMonitor_UpdateDisplay(void);
void TempMonitor_TimerHandler(void);

/* �¶Ƚӿں��� - ��Ҫ�û�ʵ�� */
float GetFrontGunTemp(void);
float GetRearChamberTemp(void);
void SetFrontGunTargetTemp(float temp);
void SetRearChamberTargetTemp(float temp);

#ifdef __cplusplus
}
#endif 

#endif
