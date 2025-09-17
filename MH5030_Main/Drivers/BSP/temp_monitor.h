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

/* 系统参数定义 */
#define TEMP_RANGE_MIN      0.0f        // 温度范围最小值
#define TEMP_RANGE_MAX      220.0f      // 温度范围最大值
#define TEMP_STEP           0.1f        // 温度调节步进
#define SAVE_DISPLAY_TIME   2000        // 保存成功提示显示时间(ms)

/* 双缓冲 */
typedef struct {
    uint8_t need_full_refresh;  // 需要全屏刷新标志
    uint8_t last_page_type;     // 上次的页面类型
} DisplayBuffer_t;


/* 页面类型定义 */
typedef enum {
    PAGE_FRONT_TEMP = 0,                // 前枪管温度显示页
    PAGE_REAR_TEMP,                     // 后腔体温度显示页  
    PAGE_FRONT_SET,                     // 前枪管温度设置页
    PAGE_REAR_SET,                      // 后腔体温度设置页
    PAGE_MAX_NUM
} PageType_t;

/* 系统状态定义 */
typedef enum {
    STATE_DISPLAY = 0,                  // 显示模式
    STATE_SETTING                       // 设置模式
} SystemState_t;

/* 温度数据结构 */
typedef struct {
    float current;                      // 当前温度
    float target;                       // 目标温度
    float temp_set;                     // 临时设置值
} TempData_t;

/* 系统控制结构 */
typedef struct {
    PageType_t current_page;            // 当前页面
    SystemState_t state;                // 系统状态
    TempData_t front_gun;               // 前枪管温度
    TempData_t rear_chamber;            // 后腔体温度
    uint8_t refresh_flag;               // 刷新标志
    uint8_t save_flag;                  // 保存标志
    uint32_t save_timer;                // 保存提示计时器
    uint32_t update_counter;            // 更新计数器
} TempMonitor_t;

/* 函数声明 */
void TempMonitor_Init(void);
void TempMonitor_Process(void);
void TempMonitor_UpdateDisplay(void);
void TempMonitor_TimerHandler(void);

/* 温度接口函数 - 需要用户实现 */
float GetFrontGunTemp(void);
float GetRearChamberTemp(void);
void SetFrontGunTargetTemp(float temp);
void SetRearChamberTargetTemp(float temp);

#ifdef __cplusplus
}
#endif 

#endif
