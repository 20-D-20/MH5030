#ifndef __MAX6675_H
#define __MAX6675_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"
#include <stdbool.h>
#include <string.h>
#include "delay.h"
#include "main.h"

/* 配置参数 */
#define MAX6675_CHANNELS        1           /* 通道数量 */
#define MAX6675_FILTER_SIZE     4           /* 滤波器大小 */
#define MAX6675_CONVERSION_TIME 250         /* 转换时间(ms) */

/* 通道定义 */
typedef enum {
    MAX6675_CH1 = 0,
    MAX6675_CH2 = 1,
    MAX6675_CH3 = 2,
    MAX6675_CH4 = 3
} MAX6675_Channel_e;

/* 错误码定义 */
typedef enum {
    MAX6675_OK               = 0,
    MAX6675_ERROR_OPEN       = -1,
    MAX6675_ERROR_COMM       = -2,
    MAX6675_ERROR_RANGE      = -3,
    MAX6675_ERROR_INVALID_CH = -4,
    MAX6675_ERROR_TIMEOUT    = -5
} MAX6675_Error_e;

/* 特殊温度值 */
#define TEMP_INVALID            999.0f
#define TEMP_MIN                0.0f
#define TEMP_MAX                1024.0f

/* GPIO配置结构体 */
typedef struct {
    GPIO_TypeDef* port;
    uint16_t pin;
} MAX6675_GPIO_t;

/* 通道状态结构体 */
typedef struct {
    float temperature;                      /* 当前温度 */
    float temp_buffer[MAX6675_FILTER_SIZE]; /* 滤波缓冲区 */
    float temp_filter;                      /* 滤波后的温度 */
    uint8_t buffer_index;                   /* 缓冲区索引 */
    bool is_initialized;                    /* 初始化标志 */
    bool is_open;                           /* 开路标志 */
    bool comm_error;                        /* 通信错误标志 */
    uint32_t last_read_tick;                /* 上次读取时间 */
    uint32_t error_count;                   /* 错误计数 */
} MAX6675_ChannelStatus_t;

/* MAX6675主结构体 */
typedef struct {
    /* GPIO配置 */
    MAX6675_GPIO_t sck;                    /* 时钟引脚 */
    MAX6675_GPIO_t so;                     /* 数据输出引脚 */
    MAX6675_GPIO_t cs[MAX6675_CHANNELS];   /* 片选引脚数组 */
    
    /* 通道状态 */
    MAX6675_ChannelStatus_t channels[MAX6675_CHANNELS];
    
    /* 配置参数 */
    bool use_filter;                       /* 是否使用滤波 */
    uint32_t read_timeout;                 /* 读取超时时间 */
    
    /* 统计信息 */
    uint32_t total_reads;                  /* 总读取次数 */
    uint32_t total_errors;                 /* 总错误次数 */
} MAX6675_Handle_t;

/* 初始化配置结构体 */
typedef struct {
    /* SCK引脚 */
    GPIO_TypeDef* sck_port;
    uint16_t sck_pin;
    
    /* SO引脚 */
    GPIO_TypeDef* so_port;
    uint16_t so_pin;
    
    /* CS引脚数组 */
    GPIO_TypeDef* cs_ports[MAX6675_CHANNELS];
    uint16_t cs_pins[MAX6675_CHANNELS];
    
    /* 配置选项 */
    bool enable_filter;
    uint32_t timeout_ms;
} MAX6675_Config_t;

/* 全局变量声明 */

extern MAX6675_Handle_t g_stMax6675;


/* 公共函数声明 */
void    MAX6675_Setup(void);
void    Temperature_Monitor_Task(void);
bool    MAX6675_IsChannelReady(MAX6675_Handle_t* handle, uint8_t channel);
HAL_StatusTypeDef    MAX6675_Init(MAX6675_Handle_t* handle, const MAX6675_Config_t* config);
HAL_StatusTypeDef    MAX6675_DeInit(MAX6675_Handle_t* handle);
MAX6675_Error_e      MAX6675_ReadTemperature(MAX6675_Handle_t* handle, uint8_t channel, float* temperature);
MAX6675_Error_e      MAX6675_ReadTemperatureFiltered(MAX6675_Handle_t* handle, uint8_t channel, float* temperature);
HAL_StatusTypeDef    MAX6675_ReadAllChannels(MAX6675_Handle_t* handle, float* temperatures);
HAL_StatusTypeDef    MAX6675_GetChannelStatus(MAX6675_Handle_t* handle, uint8_t channel, MAX6675_ChannelStatus_t* status);
HAL_StatusTypeDef    MAX6675_ClearErrors(MAX6675_Handle_t* handle, uint8_t channel);

/* 工具函数 */
const char*     MAX6675_GetErrorString(MAX6675_Error_e error);
void    MAX6675_PrintStatus(MAX6675_Handle_t* handle);

#ifdef __cplusplus
}
#endif

#endif /* __MAX6675_H */

