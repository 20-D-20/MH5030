#ifndef _DS18B20_H
#define _DS18B20_H

#include "onewire.h"
#include <stdbool.h>
#include "delay.h"

/**
 * @brief      DS18B20 设备句柄
 *
 * 保存单个 DS18B20 设备的 64-bit ROM 地址、最近一次读取的温度以及数据有效标志。
 */
typedef struct
{
    uint8_t Address[8];   /* 64-bit ROM 地址，LSB 在前 */
    float   Temperature;  /* 最近一次读取的温度值（℃） */
    bool    DataIsValid;  /* 最近一次读取的数据是否有效 */
} Ds18b20Sensor_t;

/* -------------------------------------------------------------------------------------- */
/* 外部句柄声明                                                                            */
/* -------------------------------------------------------------------------------------- */

/**
 * @brief      外部 DS18B20 设备句柄（示例：两路）
 */
extern Ds18b20Sensor_t ds18b20_1;
//extern Ds18b20Sensor_t ds18b20_2;

/**
 * @brief      外部 OneWire 总线句柄（示例：两路）
 */
extern OneWire_t OneWire_1;
//extern OneWire_t OneWire_2;

/* -------------------------------------------------------------------------------------- */
/* 常量与命令定义                                                                          */
/* -------------------------------------------------------------------------------------- */

/**
 * @brief      DS18B20 家族码（ROM 第 1 字节）
 */
#define DS18B20_FAMILY_CODE                     0x28

/**
 * @brief      报警搜索命令
 */
#define DS18B20_CMD_ALARMSEARCH                 0xEC

/**
 * @brief      触发温度转换命令（Convert T）
 */
#define DS18B20_CMD_CONVERTTEMP                 0x44  /* Convert temperature */

/* 分辨率下的小数步进（℃） */
#define DS18B20_DECIMAL_STEPS_12BIT             0.0625f
#define DS18B20_DECIMAL_STEPS_11BIT             0.125f
#define DS18B20_DECIMAL_STEPS_10BIT             0.25f
#define DS18B20_DECIMAL_STEPS_9BIT              0.5f

/**
 * @brief      转换完成等待的超时与轮询间隔（微秒）
 *
 * 12 位分辨率典型转换时间 750ms，此处取 1s 作为裕量；轮询间隔 1ms。
 */
#define DS18B20_CONVERT_TIMEOUT_US              (1000000u)   /* 最大等待 1 s */
#define DS18B20_POLL_INTERVAL_US                (1000u)      /* 每 1 ms 轮询一次 */

/* 配置寄存器中分辨率位位置（R1、R0） */
#define DS18B20_RESOLUTION_R1                   6
#define DS18B20_RESOLUTION_R0                   5

/* 读长度：启用 CRC 时为 9 字节，否则 2 字节（仅温度寄存器） */
#ifdef DS18B20_USE_CRC
#define DS18B20_DATA_LEN                        9
#else
#define DS18B20_DATA_LEN                        2
#endif

/* -------------------------------------------------------------------------------------- */
/* 分辨率枚举                                                                              */
/* -------------------------------------------------------------------------------------- */

/**
 * @brief      DS18B20 分辨率设置
 */
typedef enum
{
    DS18B20_Resolution_9bits  = 9,   /*!< 9  位分辨率，步进 0.5 ℃ */
    DS18B20_Resolution_10bits = 10,  /*!< 10 位分辨率，步进 0.25 ℃ */
    DS18B20_Resolution_11bits = 11,  /*!< 11 位分辨率，步进 0.125 ℃ */
    DS18B20_Resolution_12bits = 12   /*!< 12 位分辨率，步进 0.0625 ℃ */
} DS18B20_Resolution_t;

/* -------------------------------------------------------------------------------------- */
/* API 声明（Doxygen 风格头部注释）                                                        */
/* -------------------------------------------------------------------------------------- */

/**
 * @brief      手动触发并读取一次温度（单设备/已选定）
 * @param      ds18b20    目标传感器句柄指针（内部会写 Temperature、DataIsValid）
 * @param      OneWire    OneWire 总线句柄指针
 * @retval     true       转换完成且读取成功，Temperature 为有效摄氏度
 * @retval     false      超时或读取失败，DataIsValid 置为 false
 */
bool Ds18b20_ManualConvert(Ds18b20Sensor_t *ds18b20, OneWire_t *OneWire);

/**
 * @brief      触发温度转换（Convert T）
 * @param      OneWireStruct   OneWire 总线句柄指针
 * @retval     1               设备在位（复位有响应）且已下发转换命令
 * @retval     0               设备无响应（未在位）
 */
uint8_t DS18B20_Start(OneWire_t *OneWireStruct);

/**
 * @brief      读取温度（读取 Scratchpad 并换算为摄氏度）
 * @param      OneWireStruct   OneWire 总线句柄指针
 * @param      destination     输出温度指针（单位：℃）
 * @retval     true            读取成功且 CRC 校验通过（若启用 CRC）
 * @retval     false           转换未完成或 CRC 校验失败/读取错误
 */
bool DS18B20_Read(OneWire_t *OneWireStruct, float *destination);

/**
 * @brief      获取设备分辨率
 * @param      OneWireStruct   OneWire 总线句柄指针
 * @retval     9/10/11/12      当前分辨率位对应的位数
 * @retval     0               读取失败
 */
uint8_t DS18B20_GetResolution(OneWire_t *OneWireStruct);

/**
 * @brief      设置设备分辨率
 * @param      OneWireStruct   OneWire 总线句柄指针
 * @param      resolution      目标分辨率（9/10/11/12 位）
 * @retval     1               设置成功并写入 EEPROM（如实现）
 * @retval     0               设置失败
 */
uint8_t DS18B20_SetResolution(OneWire_t *OneWireStruct, DS18B20_Resolution_t resolution);

/**
 * @brief      判断转换是否完成（总线释放即完成）
 * @param      OneWireStruct   OneWire 总线句柄指针
 * @retval     1               转换完成（可读取）
 * @retval     0               转换未完成
 */
uint8_t DS18B20_AllDone(OneWire_t *OneWireStruct);

#endif /* _DS18B20_H */
