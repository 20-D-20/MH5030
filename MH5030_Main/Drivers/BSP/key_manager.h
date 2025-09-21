#ifndef __KEY_MANAGER_H
#define __KEY_MANAGER_H

#include "main.h"
#include "key.h"
#include "ui_manager.h"
#include "pid_manager.h"
#include "cmsis_os.h"
#include "queue.h"

/* 按键连击检测 */
#define OK_KEY_COUNT_MAX    7       // OK键连击次数触发自整定
#define OK_KEY_TIMEOUT      2000    // 连击超时时间(ms)

/* 按键管理器结构体 */
typedef struct {
    uint8_t ok_press_count;        // OK键连续按下次数
    uint32_t ok_first_press_tick;  // 第一次按下的时间戳
    uint8_t last_key;               // 上一次按键值
    uint8_t key_count;              // 按键消抖计数
} KeyManager_t;

/* 全局变量声明 */
extern KeyManager_t g_key_manager;
extern QueueHandle_t UI_Queue;
extern SemaphoreHandle_t Data_Mutex;

/* 函数声明 */
void Key_Manager_Init(void);
void Process_Browse_Mode_Key(uint8_t key);
void Process_Edit_Mode_Key(uint8_t key);
void Check_OK_Multiple_Press(void);
void Send_UI_Message(MsgType_e type, uint8_t page_id, int16_t value, uint8_t refresh_type);
void Handle_Page_Navigation(uint8_t key);
void Handle_Value_Adjustment(uint8_t key);
void Save_Temperature_Setting(uint8_t page_id, int16_t value);

#endif /* __KEY_MANAGER_H */

