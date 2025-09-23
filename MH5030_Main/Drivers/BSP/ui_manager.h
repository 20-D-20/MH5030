#ifndef __UI_MANAGER_H__
#define __UI_MANAGER_H__

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

/* 枪管类型枚举 */
typedef enum 
{
    POLL_H2SO4_MIST,            /* 硫酸雾 */
    POLL_DIOXIN,                /* 二噁英 */
    POLL_AMMONIA,               /* 氨 */    
    POLL_CODE_3091,             /* 3091 */  
    POLL_MERCURY,               /* 汞 */ 
    POLL_COUNT
} Gun_Setting_e;

/* 系统状态枚举 */
 typedef enum 
 {
     MODE_BROWSE = 0,           /* 浏览模式 */
     MODE_EDIT,                 /* 编辑模式 */
     MODE_AUTOTUNE,             /* 自整定模式 */
     MODE_SELECT                /* 选择模式 */
 } SystemMode_e;

/* 页面ID枚举 */
typedef enum 
{
    PAGE_STARTUP = 0,           /* 启动页面 */
    PAGE_TEMP_DISPLAY,          /* 温度显示页面 */
    PAGE_GUN_SETTING,           /* 枪管温度设置页面 */
    PAGE_CAVITY_SETTING,        /* 腔体温度设置页面 */
    PAGE_SMART_CONTROL,         /* 智能温控调节页面 */
    PAGE_DIOXIN_DISPLAY,        /* 二噁英3091温度显示 */
    PAGE_GUN_SELECT,            /* 枪管选择界面页面 */
    PAGE_MAX
} PageID_e;

/* 消息类型枚举 */
typedef enum 
{
    MSG_PAGE_CHANGE,
    MSG_VALUE_UPDATE,
    MSG_MODE_CHANGE,
    MSG_REALTIME_UPDATE,
    MSG_AUTOTUNE_PROGRESS       /* 自整定进度更新 */
} MsgType_e;

/* 消息结构体 */
typedef struct 
{
    MsgType_e msg_type;
    uint8_t page_id;
    int16_t new_value;
    uint8_t refresh_type;
} UIMessage_t;

/* 页面数据结构 */
typedef struct 
{
    uint8_t page_id;
    bool is_editable;
    int16_t current_value;
    int16_t min_value;
    int16_t max_value;
    int16_t step;
} PageData_t;

/* 全局变量声明 */
extern uint8_t g_current_gun_id;
extern SystemMode_e g_current_mode;
extern uint8_t g_current_page_id;
extern PageData_t g_pages[PAGE_MAX];
extern int16_t g_gun_temp_measured;
extern int16_t g_cavity_temp_measured;
extern int16_t g_dioxin_temp1;         /* 二噁英温度 */
extern int16_t g_dioxin_temp2;         /* 3091温度 */

/* 函数声明 */
void UI_Manager_Init(void);
void init_page_data(void);
void Display_Page(uint8_t page_id);
void Display_Startup_Page(void);
void Display_Temp_Page(void);
void Display_Gun_Select_Page(void);
void Display_Gun_Setting_Page(void);
void Display_Cavity_Setting_Page(void);
void Display_Smart_Control_Page(void);
void Display_Autotune_Progress_Page(void);
void Display_Dioxin_Page(void);
void Update_Value_Display(uint8_t page_id, int16_t value, bool highlight);
void get_test_temperatures(int16_t *gun_temp, int16_t *cavity_temp);
void get_dioxin_temperatures(int16_t *temp1, int16_t *temp2);

#ifdef __cplusplus
}
#endif 

#endif


