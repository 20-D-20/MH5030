#ifndef __PID_MANAGER_H
#define __PID_MANAGER_H

#include "main.h"
#include "pid.h"
#include "FM24CXX.h"
#include "tps02r.h"
#include <string.h>

/* ---------------------- 宏定义 ---------------------- */
#define PID_MAGIC_WORD      0x5AA55AA5      /* EEPROM魔术字 */
#define PID_VERSION         0x01            /* 参数版本号 */
#define EEPROM_BASE_ADDR    0x0000          /* EEPROM基地址 */

/* 温度分组边界定义 */
#define TEMP_GROUP0_MAX     140.0f          /* ≤140℃使用组0 */
#define TEMP_GROUP1_MAX     180.0f          /* 141-180℃使用组1 */

/* 参数组索引 */
#define PARAM_GROUP_120     0               /* 120℃参数组 */
#define PARAM_GROUP_160     1               /* 160℃参数组 */  
#define PARAM_GROUP_220     2               /* 220℃参数组 */
#define PARAM_GROUP_MAX     3               /* 最大组数 */

/* 系统运行模式 */
#define PID_MODE_STOP       0                   /* 停止模式 */
#define PID_MODE_RUN        1                   /* 运行模式 */
#define PID_MODE_AUTOTUNE   2               /* 自整定模式 */

/* 自整定完成标志 */
#define AUTOTUNE_COMPLETE   5               /* 穿越5次完成 */

/* ---------------------- 数据结构定义 ---------------------- */

/* PID参数组结构 */
typedef struct 
{
    float front_Kp;                        /* 前枪管比例系数 */
    float front_Ti;                        /* 前枪管积分时间 */
    float front_Td;                        /* 前枪管微分时间 */
    float rear_Kp;                         /* 腔体比例系数 */
    float rear_Ti;                         /* 腔体积分时间 */
    float rear_Td;                         /* 腔体微分时间 */
    uint8_t valid;                         /* 有效标志 */
    uint8_t reserved[3];                   /* 对齐保留 */
} PID_Param_Group_t;

/* EEPROM存储结构 */
typedef struct 
{
    uint32_t magic;                        /* 魔术字 */
    uint8_t version;                       /* 版本号 */
    uint8_t reserved[3];                   /* 保留字节 */
    
    PID_Param_Group_t group[PARAM_GROUP_MAX]; /* 三个参数组 */
    
    uint16_t checksum;                     /* 校验和 */
} PID_Storage_t;

/* 系统状态结构体 */
typedef struct 
{
    /* 温度数据 */
    float front_temp_pv;                   /* 前枪管当前温度 */
    float front_temp_sv;                   /* 前枪管设定温度 */
    float rear_temp_pv;                    /* 腔体当前温度 */  
    float rear_temp_sv;                    /* 腔体设定温度 */
    
    /* 运行状态 */
    uint8_t mode;                          /* 运行模式 */
    uint8_t autotune_request;              /* 自整定请求标志 */
    uint8_t param_group;                   /* 当前参数组 */
    uint8_t param_loaded;                  /* 参数加载标志 */
    
    /* 自整定进度 */
    uint8_t front_cross_cnt;              /* 前枪管穿越次数 */
    uint8_t rear_cross_cnt;               /* 腔体穿越次数 */
    uint8_t autotune_complete;            /* 自整定完成标志 */
    
    /* 错误状态 */
    uint8_t error_code;                   /* 错误代码 */
    uint8_t temp_error;                   /* 温度错误标志 */
} System_Status_t;

/* ---------------------- 全局变量声明 ---------------------- */
extern System_Status_t g_system_status;   /* 全局系统状态 */
extern PID_Storage_t g_pid_storage;       /* PID存储结构 */

/* ---------------------- 函数声明 ---------------------- */

/* 参数组管理 */
uint8_t Get_Param_Group(float temp_sv);
void Check_And_Switch_Group(void);
bool Load_Parameters_From_Group(uint8_t group);
void Save_Parameters_To_Group(uint8_t group);

/* EEPROM操作 */
bool Load_All_Parameters(void);
void Save_All_Parameters(void);
bool Verify_Parameters(PID_Storage_t *storage);
uint16_t Calculate_Checksum(PID_Storage_t *storage);

/* 参数初始化 */
void Init_PID_Manager(void);
void Set_Default_Parameters(uint8_t group);
void Apply_Parameters_To_PID(uint8_t group);

/* 自整定管理 */
void Start_Autotune(void);
void Stop_Autotune(void);
void Check_Autotune_Complete(void);
void Save_Autotune_Results(void);

/* 状态管理 */
void Update_System_Status(void);
void Clear_Error_Status(void);

#endif /* __PID_MANAGER_H */

