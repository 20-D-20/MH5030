#ifndef ___UI_MANAGER_H__
#define ___UI_MANAGER_H__

#include <stdlib.h>
#include "key.h"
#include "tps02r.h"
#include <stdio.h>
#include "ssd1305.h"
#include "FM24CXX.h"
#include "pid_manager.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 系统状态枚举 */
typedef enum {
    MODE_BROWSE,
    MODE_EDIT
} SystemMode_e;

/* 页面ID枚举 */
typedef enum {
    PAGE_STARTUP = 0,           // 启动页面
    PAGE_TEMP_DISPLAY,          // 温度显示页面
    PAGE_GUN_SETTING,           // 枪管温度设置页面
    PAGE_CAVITY_SETTING,        // 腔体温度设置页面
    PAGE_PID_CONTROL,           // 智能温控调节页面
    PAGE_AUTOTUNE_PROGRESS,     // 自整定进度页面
    PAGE_STATUS_DISPLAY,        // 状态显示页面
    PAGE_MAX
} PageID_e;

/* EEPROM地址定义 */
#define EEPROM_GUN_TEMP_ADDR     0x0100
#define EEPROM_CAVITY_TEMP_ADDR   0x0102

/* 消息类型枚举 */
typedef enum {
    MSG_PAGE_CHANGE,
    MSG_VALUE_UPDATE,
    MSG_MODE_CHANGE,
    MSG_REALTIME_UPDATE,
    MSG_AUTOTUNE_UPDATE
} MsgType_e;

/* 消息结构体 */
typedef struct {
    MsgType_e msg_type;
    uint8_t page_id;
    int16_t new_value;
    uint8_t refresh_type;
} UIMessage_t;

/* 页面数据结构 */
typedef struct {
    uint8_t page_id;
    bool is_editable;
    int16_t current_value;
    int16_t min_value;
    int16_t max_value;
    int16_t step;
} PageData_t;

/* 全局变量声明 */
extern SystemMode_e g_current_mode;
extern uint8_t g_current_page_id;
extern PageData_t g_pages[PAGE_MAX];
extern int16_t g_gun_temp_measured;
extern int16_t g_cavity_temp_measured;

/* 函数声明 */
void init_page_data(void);
void get_test_temperatures(int16_t *gun_temp, int16_t *cavity_temp);
void Display_Page(uint8_t page_id);
void Display_Startup_Page(void);
void Display_Temp_Page(void);
void Display_Gun_Setting_Page(void);
void Display_Cavity_Setting_Page(void);
void Display_PID_Control_Page(void);
void Display_Autotune_Progress_Page(void);
void Display_Status_Page(void);
void Update_Value_Display(uint8_t page_id, int16_t value, bool highlight);
void Update_Realtime_Data(void);

#ifdef __cplusplus
}
#endif 

#endif


