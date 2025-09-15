#include "ds18b20.h"

/*Ds18b20全局变量*/

Ds18b20Sensor_t ds18b20_1;
//Ds18b20Sensor_t ds18b20_2;
OneWire_t OneWire_1;
//OneWire_t OneWire_2;

/**
 * @brief      手动触发并读取 DS18B20 温度（单设备场景）
 * @param      ds18b20   目标传感器的句柄指针（写入 Temperature 与 DataIsValid）
 * @param      OneWire   OneWire 总线句柄指针
 * @retval     true      读取成功，ds18b20->Temperature 为摄氏度，DataIsValid = true
 * @retval     false     超时或读取失败，DataIsValid = false（Temperature 不更新或保持上次值）
 *
 * 流程：
 * 1) 触发温度转换（常规 VDD 供电，无需强上拉）。
 * 2) 轮询等待转换完成或超时（微秒级延时，不依赖系统 tick）。
 * 3) 完成后读取温度；若失败或超时，标记为无效。
 *
 * 注意：
 * - 本函数假设总线上仅有一个 DS18B20（或已做过匹配）；若为多设备，请先 Match ROM。
 * - 轮询次数采用“向上取整”的方式计算，避免间隔较大时少轮询一次。
 */
bool Ds18b20_ManualConvert(Ds18b20Sensor_t *ds18b20, OneWire_t *OneWire)
{
    /* 触发单设备温度转换（内部应包含复位与 SKIP/MATCH ROM 逻辑） */
    DS18B20_Start(OneWire);                                              /* 开始转换 */

    /* 轮询等待完成或超时：向上取整，避免漏检一次 */
    uint32_t loops = (DS18B20_CONVERT_TIMEOUT_US + DS18B20_POLL_INTERVAL_US - 1)
                     / DS18B20_POLL_INTERVAL_US;                         /* 计算轮询次数 */
    bool done = false;                                                   /* 转换完成标志 */

    while (loops--)
    {
        if (DS18B20_AllDone(OneWire))                                    /* 转换完成？ */
        {
            done = true;
            break;
        }
        delay_us(DS18B20_POLL_INTERVAL_US);                              /* 微秒级轮询间隔 */
    }

    if (!done)
    {
        ds18b20->DataIsValid = false;                                    /* 超时：数据无效 */
        return false;
    }

    /* 转换完成后读取温度到本地变量，成功再写回句柄 */
    float t = 0.0f;
    bool ok = DS18B20_Read(OneWire, &t);                                 /* 读取温度（℃） */
    ds18b20->DataIsValid = ok;                                           /* 标记有效性 */
    if (ok)
    {
        ds18b20->Temperature = t;                                        /* 仅成功时更新 */
    }
    return ok;
}

/**
 * @brief      触发 DS18B20 温度转换（单设备/已选定场景）
 * @param      OneWire   OneWire 总线句柄指针
 * @retval     1         设备在位且已下发转换命令
 * @retval     0         未检测到设备在位（复位无响应）
 *
 * 流程：
 * 1) 复位并检测设备在位（presence pulse）。
 * 2) 发送 SKIP ROM（单设备或已提前匹配的场景）。
 * 3) 发送 CONVERT T 命令以启动温度转换。
 *
 * 说明：
 * - 本实现假设正常 VDD 供电，无需强上拉；若为寄生电源模式，请在转换期间提供强上拉。
 */
uint8_t DS18B20_Start(OneWire_t *OneWire)
{
    uint8_t present = OneWire_Reset(OneWire);                   /* 复位并检测在位 */
    OneWire_WriteByte(OneWire, ONEWIRE_CMD_SKIPROM);            /* 跳过 ROM（单设备/已选定） */
    OneWire_WriteByte(OneWire, DS18B20_CMD_CONVERTTEMP);        /* 启动温度转换 */
    return present;                                              /* 返回在位标志 */
}


/**
 * @brief      读取 DS18B20 温度（读取 Scratchpad 并换算为摄氏度）
 * @param      OneWire       OneWire 总线句柄指针
 * @param      destination   输出温度指针（单位：℃）
 * @retval     true          温度读取并校验成功，*destination 写入有效值
 * @retval     false         转换未完成、CRC 校验失败或其他读取失败
 *
 * 流程说明：
 * 1) 通过读一个 time slot 判断转换是否完成（总线释放为 1 表示完成）。
 * 2) 复位 → SKIP ROM → 读 SCRATCHPAD（9 字节）。
 * 3) 计算前 8 字节的 CRC 并与第 9 字节比对，失败则返回 false。
 * 4) 解析原始温度（低两字节），按配置寄存器解析分辨率（9~12bit），换算摄氏度。
 * 5) 处理负温（原始值为补码），成功则写入 *destination 并返回 true。
 *
 * 备注：
 * - 若使用寄生电源，发起转换后需在转换期间提供强上拉；此函数假设在调用前已完成转换触发。
 */
bool DS18B20_Read(OneWire_t *OneWire, float *destination)
{
    uint16_t temperature;                                                   /* 原始温度寄存器值 */
    uint8_t  resolution;                                                    /* 传感器分辨率 9~12bit */
    int8_t   digit, minus = 0;                                              /* 整数部分与符号标志 */
    float    decimal = 0.0f;                                                /* 小数部分 */
    uint8_t  i = 0;                                                         /* 循环计数 */
    uint8_t  data[9];                                                       /* Scratchpad 缓冲区 */
    uint8_t  crc;                                                           /* 计算得到的 CRC */

    if (!OneWire_ReadBit(OneWire))                                          /* 转换完成？1=完成 */
    {
        return false;                                                       /* 未完成则直接返回 */
    }

    OneWire_Reset(OneWire);                                                 /* 复位总线 */
    OneWire_WriteByte(OneWire, ONEWIRE_CMD_SKIPROM);                        /* 单设备：跳过 ROM */
    OneWire_WriteByte(OneWire, ONEWIRE_CMD_RSCRATCHPAD);                    /* 读取 Scratchpad */

    for (i = 0; i < 9; i++)                                                 /* 读取 9 字节 */
    {
        data[i] = OneWire_ReadByte(OneWire);                                /* 逐字节读取 */
    }

    crc = OneWire_CRC8(data, 8);                                            /* 计算前 8 字节 CRC */
    if (crc != data[8])                                                     /* 校验失败？ */
    {
        return false;                                                       /* CRC 无效 */
    }

    temperature = (uint16_t)(data[0] | (data[1] << 8));                     /* 温度低/高字节 */
    OneWire_Reset(OneWire);                                                 /* 读完复位以释放总线 */

    if (temperature & 0x8000)                                               /* 最高位=1 为负温 */
    {
        temperature = (uint16_t)(~temperature + 1U);                        /* 补码转正值 */
        minus = 1;                                                          /* 记录负号 */
    }

    resolution = (uint8_t)(((data[4] & 0x60) >> 5) + 9U);                   /* 解析分辨率 */

    digit  = (int8_t)(temperature >> 4);                                    /* 整数部分：高位 */
    digit |= (int8_t)(((temperature >> 8) & 0x07) << 4);                    /* 整数部分：补齐 */

    switch (resolution)                                                     /* 小数按分辨率计算 */
    {
        case 9:
            decimal = (float)((temperature >> 3) & 0x01) * (float)DS18B20_DECIMAL_STEPS_9BIT;
            break;
        case 10:
            decimal = (float)((temperature >> 2) & 0x03) * (float)DS18B20_DECIMAL_STEPS_10BIT;
            break;
        case 11:
            decimal = (float)((temperature >> 1) & 0x07) * (float)DS18B20_DECIMAL_STEPS_11BIT;
            break;
        case 12:
            decimal = (float)(temperature & 0x0F) * (float)DS18B20_DECIMAL_STEPS_12BIT;
            break;
        default:
            return false;                                                   /* 异常分辨率 */
    }

    decimal = (float)digit + decimal;                                       /* 合成小数+整数 */
    if (minus)                                                              /* 负温处理 */
    {
        decimal = -decimal;
    }

    *destination = decimal;                                                 /* 写回温度值 */
    return true;                                                            /* 成功 */
}

/**
 * @brief      读取 DS18B20 的分辨率设置（9/10/11/12 位）
 * @param      OneWire   OneWire 总线句柄指针
 * @retval     9/10/11/12    当前分辨率位数
 * @retval     0             读取失败（如设备无响应）
 *
 * 说明：
 * - 分辨率位位于配置寄存器（Scratchpad 第 5 字节，索引 data[4]）的 R1、R0 位（bit6、bit5）。
 * - 本函数通过 SKIP ROM 直接访问总线上的单个（或已选定的）器件。
 */
uint8_t DS18B20_GetResolution(OneWire_t *OneWire)
{
    uint8_t conf;                                                           /* 配置寄存器（含分辨率位） */
                                                                            
    if (!OneWire_Reset(OneWire))                                            /* 复位并检测在位；0 表示无响应 */
    {                                                                       
        return 0;                                                           /* 设备不在位，返回失败 */
    }                                                                       
                                                                            
    OneWire_WriteByte(OneWire, ONEWIRE_CMD_SKIPROM);                        /* 跳过 ROM（单设备/已选定） */
    OneWire_WriteByte(OneWire, ONEWIRE_CMD_RSCRATCHPAD);                    /* 读取 Scratchpad 命令 */
                                                                            
    (void)OneWire_ReadByte(OneWire);                                        /* 跳过温度 LSB */
    (void)OneWire_ReadByte(OneWire);                                        /* 跳过温度 MSB */
    (void)OneWire_ReadByte(OneWire);                                        /* 跳过高温报警阈值 TH */
    (void)OneWire_ReadByte(OneWire);                                        /* 跳过低温报警阈值 TL */
                                                                            
    conf = OneWire_ReadByte(OneWire);                                       /* 读取配置寄存器（包含 R1/R0） */
                                                                            
    /* 可选：读完后复位以释放总线；若上层另有流程要求，可移除 */               
    OneWire_Reset(OneWire);                                                 /* 释放总线 */
                                                                            
    return (uint8_t)(((conf & 0x60u) >> 5) + 9u);                           /* (R1:R0)→[00..11] 映射为 9..12 位 */
}                                                                           


/**
 * @brief      设置 DS18B20 的温度分辨率（9/10/11/12 位）
 * @param      OneWire        OneWire 总线句柄指针
 * @param      resolution     目标分辨率（DS18B20_Resolution_t）
 * @retval     1              设置成功（含复制到内部 EEPROM 完成）
 * @retval     0              设备无响应或过程失败
 *
 * 过程：
 * 1) 读 Scratchpad 以获取当前 TH、TL 与配置寄存器（只修改 R1/R0 两位，保持其余位不变）。
 * 2) 写回 TH、TL、配置寄存器到 Scratchpad。
 * 3) 执行 Copy Scratchpad 将配置写入内部 EEPROM（典型 ≤10ms）。若为寄生电源需强上拉。
 *
 * 提示：
 * - 若总线上有多个设备，请在外层进行 Match ROM；此处使用 SKIP ROM 仅适用于单设备或已选定设备。
 * - Copy Scratchpad 期间，器件会拉低总线直到完成，可轮询数据线释放来判断完成。
 */
uint8_t DS18B20_SetResolution(OneWire_t *OneWire, DS18B20_Resolution_t resolution)
{
    uint8_t th, tl, conf;

    if (!OneWire_Reset(OneWire))                                                /* 复位检测在位 */
    {
        return 0;
    }
    OneWire_WriteByte(OneWire, ONEWIRE_CMD_SKIPROM);                            /* 跳过 ROM */
    OneWire_WriteByte(OneWire, ONEWIRE_CMD_RSCRATCHPAD);                        /* 读 Scratchpad */

    (void)OneWire_ReadByte(OneWire);                                            /* 跳过 Temp LSB */
    (void)OneWire_ReadByte(OneWire);                                            /* 跳过 Temp MSB */
    th   = OneWire_ReadByte(OneWire);                                           /* 读 TH */
    tl   = OneWire_ReadByte(OneWire);                                           /* 读 TL */
    conf = OneWire_ReadByte(OneWire);                                           /* 读配置寄存器 */

    /* 仅修改分辨率位（R1/R0 分别为 bit6/bit5），其他位保持不变 */
    switch (resolution)
    {
        case DS18B20_Resolution_9bits:
            conf &= (uint8_t)~(1u << DS18B20_RESOLUTION_R1);
            conf &= (uint8_t)~(1u << DS18B20_RESOLUTION_R0);
            break;
        case DS18B20_Resolution_10bits:
            conf &= (uint8_t)~(1u << DS18B20_RESOLUTION_R1);
            conf |=  (uint8_t)(1u << DS18B20_RESOLUTION_R0);
            break;
        case DS18B20_Resolution_11bits:
            conf |=  (uint8_t)(1u << DS18B20_RESOLUTION_R1);
            conf &= (uint8_t)~(1u << DS18B20_RESOLUTION_R0);
            break;
        case DS18B20_Resolution_12bits:
            conf |=  (uint8_t)(1u << DS18B20_RESOLUTION_R1);
            conf |=  (uint8_t)(1u << DS18B20_RESOLUTION_R0);
            break;
        default:
            return 0;                                                           /* 非法入参 */
    }

    if (!OneWire_Reset(OneWire))                                                /* 再次复位 */
    {
        return 0;
    }
    OneWire_WriteByte(OneWire, ONEWIRE_CMD_SKIPROM);                            /* 跳过 ROM */
    OneWire_WriteByte(OneWire, ONEWIRE_CMD_WSCRATCHPAD);                        /* 写 Scratchpad */

    OneWire_WriteByte(OneWire, th);                                             /* 写 TH */
    OneWire_WriteByte(OneWire, tl);                                             /* 写 TL */
    OneWire_WriteByte(OneWire, conf);                                           /* 写配置寄存器 */

    if (!OneWire_Reset(OneWire))                                                /* 复位进入拷贝 */
    {
        return 0;
    }
    OneWire_WriteByte(OneWire, ONEWIRE_CMD_SKIPROM);                            /* 跳过 ROM */
    OneWire_WriteByte(OneWire, ONEWIRE_CMD_CPYSCRATCHPAD);                      /* 拷贝到 EEPROM */

    /* 典型 ≤10ms：通过读取一个 time slot 轮询完成（1=完成，0=忙） */
    {
        /* 简单轮询，最多等待 ~15ms */
        const uint32_t max_us = 15000u;                                         /* 最长等待 15ms */
        const uint32_t step_us = 500u;                                          /* 轮询步进 0.5ms */
        uint32_t waited = 0;

        while (!OneWire_ReadBit(OneWire))                                       /* 低电平=忙 */
        {
            delay_us(step_us);
            waited += step_us;
            if (waited >= max_us)
            {
                return 0;                                                       /* 超时失败 */
            }
        }
    }

    return 1;
}


uint8_t DS18B20_AllDone(OneWire_t* OneWire)
{
    /* If read bit is low, then device is not finished yet with calculation temperature */
    return OneWire_ReadBit(OneWire);
}


