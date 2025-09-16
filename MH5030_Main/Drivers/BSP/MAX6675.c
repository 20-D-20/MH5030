#include "max6675.h"
#include <stdio.h>

/* 全局变量定义 */
MAX6675_Handle_t g_stMax6675;
float temperatures[MAX6675_CHANNELS];

/* 私有函数声明 */
static void MAX6675_GPIO_Init(MAX6675_Handle_t* handle, const MAX6675_Config_t* config);
static void MAX6675_SetCS(MAX6675_Handle_t* handle, uint8_t channel, GPIO_PinState state);
static void MAX6675_SetSCK(MAX6675_Handle_t* handle, GPIO_PinState state);
static GPIO_PinState MAX6675_ReadSO(MAX6675_Handle_t* handle);
static uint16_t MAX6675_ReadRawData(MAX6675_Handle_t* handle, uint8_t channel);
static float MAX6675_RawToTemperature(uint16_t raw_data);
static float MAX6675_ApplyFilter(MAX6675_Handle_t* handle, uint8_t channel, float new_temp);
static void MAX6675_Delay_us(uint32_t us);

/**
* @brief  温度监控任务
* @note   该函数主要用于测试
* @param  None
* @retval None
*/
void Temperature_Monitor_Task(void)
{
   MAX6675_Error_e result;
   float temp;
   static uint32_t cycle_count = 0;
   
   printf("--- Cycle %lu ---\r\n", ++cycle_count);
   
   /* 方法1: 单独读取每个通道 */
   for(uint8_t i = 0; i < MAX6675_CHANNELS; i++) 
   {
       /* 检查通道是否准备好 */
       if(!MAX6675_IsChannelReady(&g_stMax6675, i)) 
       {
           printf("CH%d: Not ready\r\n", i + 1);
           continue;
       }
       
       /* 读取带滤波的温度 */
       result = MAX6675_ReadTemperatureFiltered(&g_stMax6675, i, &temp);
       
       /* 根据结果处理 */
       switch(result) {
           case MAX6675_OK:
               printf("CH%d: %.2f °C", i + 1, temp);
               
               /* 温度报警检查 */
               if(temp > 100.0f) 
               {
                   printf(" [HIGH TEMP WARNING!]");
               } else if(temp < 10.0f) {
                   printf(" [LOW TEMP WARNING!]");
               }
               printf("\r\n");
               break;
               
           case MAX6675_ERROR_OPEN:
               printf("CH%d: Thermocouple OPEN!\r\n", i + 1);
               break;
               
           case MAX6675_ERROR_COMM:
               printf("CH%d: Communication ERROR!\r\n", i + 1);
               break;
               
           case MAX6675_ERROR_RANGE:
               printf("CH%d: Out of RANGE!\r\n", i + 1);
               break;
               
           case MAX6675_ERROR_TIMEOUT:
               printf("CH%d: Read TIMEOUT!\r\n", i + 1);
               break;
               
           default:
               printf("CH%d: Unknown error (%d)\r\n", i + 1, result);
               break;
       }
       
   }
   
   /* 每10个周期打印一次统计信息 */
   if(cycle_count % 10 == 0) {
       printf("\r\n");
       MAX6675_PrintStatus(&g_stMax6675);
   }
}

/**
 * @brief  MAX6675初始化配置
 * @param  None
 * @retval None
 */
 void MAX6675_Setup(void)
{
    MAX6675_Config_t config = {0};
    
    /* 配置SCK引脚 - PA12 */
    config.sck_port = MAX6675_SCK_GPIO_Port;
    config.sck_pin = MAX6675_SCK_Pin;
    
    /* 配置SO引脚 - PA10 */
    config.so_port = MAX6675_SO_A_GPIO_Port;
    config.so_pin = MAX6675_SO_A_Pin;
    
    /* 配置CS引脚 -    PA11 */
    config.cs_ports[0] = MAX6675__CS_GPIO_Port;
    config.cs_pins[0] = MAX6675__CS_Pin; 
    
#if 0   /*根据实际应用需要去配置其他通道*/
    config.cs_ports[1] = GPIOA;
    config.cs_pins[1] = GPIO_PIN_3;  /* CS2 - PA3 */
    
    config.cs_ports[2] = GPIOA;
    config.cs_pins[2] = GPIO_PIN_2;  /* CS3 - PA2 */
    
    config.cs_ports[3] = GPIOA;
    config.cs_pins[3] = GPIO_PIN_1;  /* CS4 - PA1 */
#endif

    /* 使能滤波 */
    config.enable_filter = true;
    config.timeout_ms = 100;
    
    /* 初始化MAX6675 */
    MAX6675_Init(&g_stMax6675, &config);

}

/**
 * @brief  微秒级延时函数
 * @param  us: 延时微秒数
 * @retval None
 */
static void MAX6675_Delay_us(uint32_t us)
{
    uint32_t ticks = us * (SystemCoreClock / 1000000) / 5;
    while(ticks--) {
        __NOP();
    }
}

/**
 * @brief  初始化MAX6675
 * @param  handle: MAX6675句柄
 * @param  config: 配置参数
 * @retval HAL状态
 */
HAL_StatusTypeDef MAX6675_Init(MAX6675_Handle_t* handle, const MAX6675_Config_t* config)
{
    if(handle == NULL || config == NULL) {
        return HAL_ERROR;
    }
    
    /* 清空句柄 */
    memset(handle, 0, sizeof(MAX6675_Handle_t));
    
    /* 保存配置 */
    handle->use_filter = config->enable_filter;
    handle->read_timeout = config->timeout_ms;
    
    /* 初始化GPIO */
    MAX6675_GPIO_Init(handle, config);
    
    /* 初始化通道状态 */
    for(uint8_t i = 0; i < MAX6675_CHANNELS; i++) 
    {
        handle->channels[i].temperature = 0.0f;
        handle->channels[i].buffer_index = 0;
        handle->channels[i].is_initialized = false;
        handle->channels[i].is_open = false;
        handle->channels[i].comm_error = false;
        handle->channels[i].last_read_tick = 0;
        handle->channels[i].error_count = 0;
        
        /* 清空滤波缓冲区 */
        for(uint8_t j = 0; j < MAX6675_FILTER_SIZE; j++) 
        {
            handle->channels[i].temp_buffer[j] = 0.0f;
        }
        
        /* CS拉高 */
        MAX6675_SetCS(handle, i, GPIO_PIN_SET);
    }
    
    /* SCK拉低 */
    MAX6675_SetSCK(handle, GPIO_PIN_RESET);
     
    return HAL_OK;
}

/**
 * @brief  GPIO初始化
 * @param  handle: MAX6675句柄
 * @param  config: 配置参数
 * @retval None
 */
static void MAX6675_GPIO_Init(MAX6675_Handle_t* handle, const MAX6675_Config_t* config)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* 保存GPIO配置 */
    handle->sck.port = config->sck_port;
    handle->sck.pin = config->sck_pin;
    handle->so.port = config->so_port;
    handle->so.pin = config->so_pin;
    
    for(uint8_t i = 0; i < MAX6675_CHANNELS; i++) 
    {
        handle->cs[i].port = config->cs_ports[i];
        handle->cs[i].pin = config->cs_pins[i];
    }
    
    /* 使能GPIO时钟 - 根据实际使用的端口修改 */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    /* 配置SCK引脚 - 推挽输出 */
    GPIO_InitStruct.Pin = handle->sck.pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(handle->sck.port, &GPIO_InitStruct);
    
    /* 配置CS引脚 - 推挽输出 */
    for(uint8_t i = 0; i < MAX6675_CHANNELS; i++) 
    {
        GPIO_InitStruct.Pin = handle->cs[i].pin;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(handle->cs[i].port, &GPIO_InitStruct);
    }
    
    /* 配置SO引脚 - 输入上拉 */
    GPIO_InitStruct.Pin = handle->so.pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(handle->so.port, &GPIO_InitStruct);
}

/**
 * @brief  设置CS引脚状态
 * @param  handle: MAX6675句柄
 * @param  channel: 通道号
 * @param  state: 引脚状态
 * @retval None
 */
static void MAX6675_SetCS(MAX6675_Handle_t* handle, uint8_t channel, GPIO_PinState state)
{
    if(channel < MAX6675_CHANNELS) 
    {
        HAL_GPIO_WritePin(handle->cs[channel].port, handle->cs[channel].pin, state);
    }
}

/**
 * @brief  设置SCK引脚状态
 * @param  handle: MAX6675句柄
 * @param  state: 引脚状态
 * @retval None
 */
static void MAX6675_SetSCK(MAX6675_Handle_t* handle, GPIO_PinState state)
{
    HAL_GPIO_WritePin(handle->sck.port, handle->sck.pin, state);
}

/**
 * @brief  读取SO引脚状态
 * @param  handle: MAX6675句柄
 * @retval 引脚状态
 */
static GPIO_PinState MAX6675_ReadSO(MAX6675_Handle_t* handle)
{
    return HAL_GPIO_ReadPin(handle->so.port, handle->so.pin);
}

/**
 * @brief  读取原始数据
 * @param  handle: MAX6675句柄
 * @param  channel: 通道号
 * @retval 16位原始数据
 */
static uint16_t MAX6675_ReadRawData(MAX6675_Handle_t* handle, uint8_t channel)
{
    uint16_t data = 0;
    uint32_t timeout_start;
    
    if(channel >= MAX6675_CHANNELS) 
    {
        return 0xFFFF;
    }
    
    /* 记录开始时间 */
    timeout_start = HAL_GetTick();
    
    /* CS拉低，开始SPI通信 */
    MAX6675_SetCS(handle, channel, GPIO_PIN_RESET);
    MAX6675_Delay_us(1);
    
    /* 读取16位数据 */
    for(uint8_t i = 0; i < 16; i++) 
   {
        /* 超时检查 */
        if((HAL_GetTick() - timeout_start) > handle->read_timeout) 
        {
            MAX6675_SetCS(handle, channel, GPIO_PIN_SET);
            return 0xFFFF;
        }
        
        /* SCK上升沿 */
        MAX6675_SetSCK(handle, GPIO_PIN_SET);
        MAX6675_Delay_us(1);
        
        /* 读取数据位 */
        data <<= 1;
        if(MAX6675_ReadSO(handle) == GPIO_PIN_SET) 
        {
            data |= 0x0001;
        }
        
        /* SCK下降沿 */
        MAX6675_SetSCK(handle, GPIO_PIN_RESET);
        MAX6675_Delay_us(1);
    }
    
    /* CS拉高，结束通信 */
    MAX6675_SetCS(handle, channel, GPIO_PIN_SET);
    
    /* 更新统计 */
    handle->total_reads++;
    
    return data;
}

/**
 * @brief  原始数据转换为温度
 * @param  raw_data: 原始数据
 * @retval 温度值（摄氏度）
 */
static float MAX6675_RawToTemperature(uint16_t raw_data)
{
    /* 提取12位温度数据 (D14-D3) */
    uint16_t temp_data = (raw_data >> 3) & 0x0FFF;
    
    /* 转换为温度值，分辨率0.25°C */
    return (float)temp_data * 0.25f;
}

/**
 * @brief  应用滤波器
 * @param  handle: MAX6675句柄
 * @param  channel: 通道号
 * @param  new_temp: 新温度值
 * @retval 滤波后的温度
 */
static float MAX6675_ApplyFilter(MAX6675_Handle_t* handle, uint8_t channel, float new_temp)
{
    float sum = 0;
    uint8_t count = 0;
    MAX6675_ChannelStatus_t* ch_status;
    
    if(channel >= MAX6675_CHANNELS) 
    {
        return new_temp;
    }
    
    ch_status = &handle->channels[channel];
    
    /* 存储新值 */
    ch_status->temp_buffer[ch_status->buffer_index] = new_temp;
    ch_status->buffer_index = (ch_status->buffer_index + 1) % MAX6675_FILTER_SIZE;
    
    /* 如果是第一次读取，填充整个缓冲区 */
    if(!ch_status->is_initialized) {
        for(uint8_t i = 0; i < MAX6675_FILTER_SIZE; i++) 
        {
            ch_status->temp_buffer[i] = new_temp;
        }
        ch_status->is_initialized = true;
        return new_temp;
    }
    
    /* 计算平均值 */
    for(uint8_t i = 0; i < MAX6675_FILTER_SIZE; i++) 
    {
        if(ch_status->temp_buffer[i] > 0) 
        {
            sum += ch_status->temp_buffer[i];
            count++;
        }
    }
    
    return (count > 0) ? (sum / count) : new_temp;
}

/**
 * @brief  读取温度
 * @param  handle: MAX6675句柄
 * @param  channel: 通道号
 * @param  temperature: 温度值指针
 * @retval 错误码
 */
MAX6675_Error_e MAX6675_ReadTemperature(MAX6675_Handle_t* handle, uint8_t channel, float* temperature)
{
    uint16_t raw_data;
    float temp_value;
    
    if(handle == NULL || temperature == NULL) 
    {
        return MAX6675_ERROR_INVALID_CH;
    }
    
    if(channel >= MAX6675_CHANNELS) 
    {
        return MAX6675_ERROR_INVALID_CH;
    }
    
    /* 检查读取间隔 */
    uint32_t current_tick = HAL_GetTick();
    if((current_tick - handle->channels[channel].last_read_tick) < MAX6675_CONVERSION_TIME) 
    {
        /* 返回上次的值 */
        *temperature = handle->channels[channel].temperature;
        return MAX6675_OK;
    }
    
    /* 读取原始数据 */
    raw_data = MAX6675_ReadRawData(handle, channel);
    
    /* 检查超时 */
    if(raw_data == 0xFFFF) 
    {
        handle->channels[channel].error_count++;
        handle->total_errors++;
        return MAX6675_ERROR_TIMEOUT;
    }
    
    /* 检查D0位 - 应该为0 */
    if(raw_data & 0x0001) 
    {
        handle->channels[channel].comm_error = true;
        handle->channels[channel].error_count++;
        handle->total_errors++;
        return MAX6675_ERROR_COMM;
    }
    
    /* 检查D1位 - 设备ID，应该为0 */
    if(raw_data & 0x0002) {
        handle->channels[channel].comm_error = true;
        handle->channels[channel].error_count++;
        handle->total_errors++;
        return MAX6675_ERROR_COMM;
    }
    
    /* 检查D2位 - 热电偶开路检测 */
    if(raw_data & 0x0004) 
    {
        handle->channels[channel].is_open = true;
        handle->channels[channel].error_count++;
        handle->total_errors++;
        return MAX6675_ERROR_OPEN;
    }
    
    /* 清除错误标志 */
    handle->channels[channel].is_open = false;
    handle->channels[channel].comm_error = false;
    
    /* 转换温度 */
    temp_value = MAX6675_RawToTemperature(raw_data);
    
    /* 范围检查 */
    if(temp_value < TEMP_MIN || temp_value > TEMP_MAX) 
    {
        handle->channels[channel].error_count++;
        handle->total_errors++;
        return MAX6675_ERROR_RANGE;
    }
    
    /* 更新通道状态 */
    handle->channels[channel].temperature = temp_value;
    handle->channels[channel].last_read_tick = current_tick;
    
    *temperature = temp_value;
    
    return MAX6675_OK;
}

/**
 * @brief  读取带滤波的温度
 * @param  handle: MAX6675句柄
 * @param  channel: 通道号
 * @param  temperature: 温度值指针
 * @retval 错误码
 */
MAX6675_Error_e MAX6675_ReadTemperatureFiltered(MAX6675_Handle_t* handle, uint8_t channel, float* temperature)
{
    MAX6675_Error_e result;
    float temp_raw;
    
    if(handle == NULL || temperature == NULL) 
    {
        return MAX6675_ERROR_INVALID_CH;
    }
    
    /* 读取原始温度 */
    result = MAX6675_ReadTemperature(handle, channel, &temp_raw);
    
    if(result != MAX6675_OK) 
    {
        return result;
    }
    
    /* 应用滤波 */
    if(handle->use_filter) 
    {
        *temperature = MAX6675_ApplyFilter(handle, channel, temp_raw);
    } 
    else
    {
        *temperature = temp_raw;
    }
    
    return MAX6675_OK;
}

/**
 * @brief  读取所有通道
 * @param  handle: MAX6675句柄
 * @param  temperatures: 温度数组
 * @retval HAL状态
 */
HAL_StatusTypeDef MAX6675_ReadAllChannels(MAX6675_Handle_t* handle, float* temperatures)
{
    MAX6675_Error_e result;
    
    if(handle == NULL || temperatures == NULL) 
    {
        return HAL_ERROR;
    }
    
    for(uint8_t i = 0; i < MAX6675_CHANNELS; i++) 
    {
        if(handle->use_filter) 
        {
            result = MAX6675_ReadTemperatureFiltered(handle, i, &temperatures[i]);
        } 
        else 
        {
            result = MAX6675_ReadTemperature(handle, i, &temperatures[i]);
        }
        
        if(result != MAX6675_OK) 
        {
            temperatures[i] = TEMP_INVALID;
        }
        
        /* 通道间延时 */
        HAL_Delay(50);
    }
    
    return HAL_OK;
}

/**
 * @brief  获取通道状态
 * @param  handle: MAX6675句柄
 * @param  channel: 通道号
 * @param  status: 状态结构体指针
 * @retval HAL状态
 */
HAL_StatusTypeDef MAX6675_GetChannelStatus(MAX6675_Handle_t* handle, uint8_t channel, MAX6675_ChannelStatus_t* status)
{
    if(handle == NULL || status == NULL || channel >= MAX6675_CHANNELS) 
    {
        return HAL_ERROR;
    }
    
    memcpy(status, &handle->channels[channel], sizeof(MAX6675_ChannelStatus_t));
    
    return HAL_OK;
}

/**
 * @brief  清除错误
 * @param  handle: MAX6675句柄
 * @param  channel: 通道号
 * @retval HAL状态
 */
HAL_StatusTypeDef MAX6675_ClearErrors(MAX6675_Handle_t* handle, uint8_t channel)
{
    if(handle == NULL || channel >= MAX6675_CHANNELS) 
    {
        return HAL_ERROR;
    }
    
    handle->channels[channel].is_open = false;
    handle->channels[channel].comm_error = false;
    handle->channels[channel].error_count = 0;
    
    return HAL_OK;
}

/**
 * @brief  检查通道是否准备好
 * @param  handle: MAX6675句柄
 * @param  channel: 通道号
 * @retval true-准备好，false-未准备好
 */
bool MAX6675_IsChannelReady(MAX6675_Handle_t* handle, uint8_t channel)
{
    uint32_t current_tick;
    
    if(handle == NULL || channel >= MAX6675_CHANNELS) 
    {
        return false;
    }
    
    current_tick = HAL_GetTick();
    
    return (current_tick - handle->channels[channel].last_read_tick) >= MAX6675_CONVERSION_TIME;
}

/**
 * @brief  获取错误字符串
 * @param  error: 错误码
 * @retval 错误描述字符串
 */
const char* MAX6675_GetErrorString(MAX6675_Error_e error)
{
    switch(error) 
    {
        case MAX6675_OK:
            return "No error";
        case MAX6675_ERROR_OPEN:
            return "Thermocouple open";
        case MAX6675_ERROR_COMM:
            return "Communication error";
        case MAX6675_ERROR_RANGE:
            return "Temperature out of range";
        case MAX6675_ERROR_INVALID_CH:
            return "Invalid channel";
        case MAX6675_ERROR_TIMEOUT:
            return "Read timeout";
        default:
            return "Unknown error";
    }
}

/**
 * @brief  打印状态信息
 * @param  handle: MAX6675句柄
 * @retval None
 */
void MAX6675_PrintStatus(MAX6675_Handle_t* handle)
{
    if(handle == NULL) 
    {
        return;
    }
    
    printf("=== MAX6675 Status ===\n");
    printf("Total reads: %lu\n", handle->total_reads);
    printf("Total errors: %lu\n", handle->total_errors);
    printf("Filter: %s\n", handle->use_filter ? "Enabled" : "Disabled");
    
    for(uint8_t i = 0; i < MAX6675_CHANNELS; i++) 
    {
        printf("\nChannel %d:\n", i + 1);
        printf("  Temperature: %.2f°C\n", handle->channels[i].temperature);
        printf("  Open circuit: %s\n", handle->channels[i].is_open ? "Yes" : "No");
        printf("  Comm error: %s\n", handle->channels[i].comm_error ? "Yes" : "No");
        printf("  Error count: %lu\n", handle->channels[i].error_count);
    }
}

/**
 * @brief  反初始化
 * @param  handle: MAX6675句柄
 * @retval HAL状态
 */
HAL_StatusTypeDef MAX6675_DeInit(MAX6675_Handle_t* handle)
{
    if(handle == NULL) 
    {
        return HAL_ERROR;
    }
    
    /* 所有CS引脚拉高 */
    for(uint8_t i = 0; i < MAX6675_CHANNELS; i++) 
    {
        MAX6675_SetCS(handle, i, GPIO_PIN_SET);
    }
    
    /* 清空句柄 */
    memset(handle, 0, sizeof(MAX6675_Handle_t));
    
    return HAL_OK;
}


