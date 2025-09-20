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
 * @brief      系统状态枚举
 * @retval     None
 */
typedef enum 
{
    MODE_BROWSE,                                                   /* 浏览模式 */
    MODE_EDIT                                                      /* 编辑模式 */
} SystemMode_e;

/**
 * @brief      页面ID枚举
 * @retval     None
 */
typedef enum 
{
    PAGE_STARTUP = 0,                                              /* 启动页面 */
    PAGE_TEMP_DISPLAY,                                             /* 温度显示页面 */
    PAGE_GUN_SETTING,                                              /* 枪管温度设置页面 */
    PAGE_CAVITY_SETTING,                                           /* 腔体温度设置页面 */
    PAGE_MAX                                                       /* 页面数量 */
} PageID_e;

/**
 * @brief      消息类型枚举
 * @retval     None
 */
typedef enum 
{
    MSG_PAGE_CHANGE,                                               /* 页面切换 */
    MSG_VALUE_UPDATE,                                              /* 数值更新 */
    MSG_MODE_CHANGE,                                               /* 模式切换 */
    MSG_REALTIME_UPDATE                                            /* 实时数据更新 */
} MsgType_e;

/**
 * @brief      消息结构体
 * @retval     None
 */
typedef struct 
{
    MsgType_e msg_type;                                            /* 消息类型 */
    uint8_t page_id;                                               /* 页面ID */
    int16_t new_value;                                             /* 新数值 */
    uint8_t refresh_type;                                          /* 刷新类型：0-仅值，1-全刷新 */
} UIMessage_t;

/**
 * @brief      页面数据结构
 * @retval     None
 */
typedef struct 
{
    uint8_t page_id;                                               /* 页面ID */
    bool is_editable;                                              /* 是否可编辑 */
    int16_t current_value;                                         /* 当前值 */
    int16_t min_value;                                             /* 最小值 */
    int16_t max_value;                                             /* 最大值 */
    int16_t step;                                                  /* 步进值 */
} PageData_t;

extern SystemMode_e g_current_mode;
extern uint8_t g_current_page_id;
extern PageData_t g_pages[PAGE_MAX];
extern int16_t g_gun_temp_measured ;                              // 枪管测量温度
extern int16_t g_cavity_temp_measured;                            // 腔体测量温度


void init_page_data(void);
void get_test_temperatures(int16_t *gun_temp, int16_t *cavity_temp);


#ifdef __cplusplus
}
#endif 

#endif
