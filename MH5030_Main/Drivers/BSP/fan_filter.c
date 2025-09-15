/**
  ******************************************************************************
  * @file    fan_filter.c
  * @brief   风扇转速滤波算法集合
  * @author  
  * @date    2025-01-10
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <string.h>
#include <math.h>
#include <stdio.h>
#include "fan_control.h"

/* Private typedef -----------------------------------------------------------*/

/**
  * @brief  移动平均滤波器结构体
  */
typedef struct {
    uint16_t buffer[16];       // 数据缓冲区
    uint8_t  buffer_size;      // 缓冲区大小
    uint8_t  index;            // 当前索引
    uint8_t  count;            // 已填充数量
    uint32_t sum;              // 累计和
} MovingAvgFilter_t;

/**
  * @brief  中值滤波器结构体
  */
typedef struct {
    uint16_t buffer[9];        // 数据缓冲区（奇数个）
    uint8_t  buffer_size;      // 缓冲区大小
    uint8_t  index;            // 当前索引
    uint8_t  count;            // 已填充数量
} MedianFilter_t;

/**
  * @brief  卡尔曼滤波器结构体
  */
typedef struct {
    float Q;                   // 过程噪声协方差
    float R;                   // 测量噪声协方差
    float P;                   // 估计误差协方差
    float K;                   // 卡尔曼增益
    float X;                   // 状态估计值
} KalmanFilter_t;

/**
  * @brief  一阶低通滤波器结构体
  */
typedef struct {
    float alpha;               // 滤波系数 (0-1)
    float output;              // 输出值
    uint8_t initialized;       // 初始化标志
} LowPassFilter_t;

/**
  * @brief  滑动窗口滤波器（去除极值）
  */
typedef struct {
    uint16_t buffer[12];       // 数据缓冲区
    uint8_t  buffer_size;      // 缓冲区大小
    uint8_t  index;            // 当前索引
    uint8_t  count;            // 已填充数量
    uint8_t  trim_count;       // 去除的极值数量（两端各去除）
} TrimmedMeanFilter_t;

/* Private variables ---------------------------------------------------------*/
static MovingAvgFilter_t moving_avg_filter;
static MedianFilter_t median_filter;
static KalmanFilter_t kalman_filter;
static LowPassFilter_t lowpass_filter;
static TrimmedMeanFilter_t trimmed_filter;

/* Function implementations --------------------------------------------------*/

/**
  * @brief  初始化所有滤波器
  * @param  None
  * @retval None
  */
void Filter_Init_All(void)
{
    // 移动平均滤波器初始化
    memset(&moving_avg_filter, 0, sizeof(MovingAvgFilter_t));
    moving_avg_filter.buffer_size = 8;  // 使用8个采样点
    
    // 中值滤波器初始化
    memset(&median_filter, 0, sizeof(MedianFilter_t));
    median_filter.buffer_size = 7;  // 使用7个采样点（必须是奇数）
    
    // 卡尔曼滤波器初始化
    kalman_filter.Q = 0.01f;    // 过程噪声，值越小越信任模型
    kalman_filter.R = 15.0f;    // 测量噪声，根据实际波动调整（约±15 RPM）
    kalman_filter.P = 1.0f;     // 初始估计误差
    kalman_filter.X = 730.0f;   // 初始估计值（取中间值）
    
    // 一阶低通滤波器初始化
    lowpass_filter.alpha = 0.15f;  // 滤波系数，越小越平滑（0.1-0.3较好）
    lowpass_filter.initialized = 0;
    
    // 去极值平均滤波器初始化
    memset(&trimmed_filter, 0, sizeof(TrimmedMeanFilter_t));
    trimmed_filter.buffer_size = 10;
    trimmed_filter.trim_count = 2;  // 去除最大和最小各2个值
}

/**
  * @brief  移动平均滤波
  * @param  new_value: 新的采样值
  * @retval 滤波后的值
  */
uint16_t Filter_MovingAverage(uint16_t new_value)
{
    // 如果是第一个数据
    if (moving_avg_filter.count == 0) 
    {
        moving_avg_filter.sum = 0;
    }
    
    // 如果缓冲区已满，减去最旧的值
    if (moving_avg_filter.count >= moving_avg_filter.buffer_size) 
    {
        moving_avg_filter.sum -= moving_avg_filter.buffer[moving_avg_filter.index];
    } 
    else 
    {
        moving_avg_filter.count++;
    }
    
    // 添加新值
    moving_avg_filter.buffer[moving_avg_filter.index] = new_value;
    moving_avg_filter.sum += new_value;
    
    // 更新索引
    moving_avg_filter.index = (moving_avg_filter.index + 1) % moving_avg_filter.buffer_size;
    
    // 返回平均值
    return (uint16_t)(moving_avg_filter.sum / moving_avg_filter.count);
}

/**
  * @brief  中值滤波
  * @param  new_value: 新的采样值
  * @retval 滤波后的值
  */
uint16_t Filter_Median(uint16_t new_value)
{
    uint16_t temp_buffer[9];
    uint16_t temp;
    uint8_t i, j;
    
    // 添加新值到缓冲区
    median_filter.buffer[median_filter.index] = new_value;
    median_filter.index = (median_filter.index + 1) % median_filter.buffer_size;
    
    // 更新计数
    if (median_filter.count < median_filter.buffer_size) 
    {
        median_filter.count++;
    }
    
    // 复制数据到临时缓冲区
    memcpy(temp_buffer, median_filter.buffer, median_filter.count * sizeof(uint16_t));
    
    // 冒泡排序
    for (i = 0; i < median_filter.count - 1; i++) 
    {
        for (j = 0; j < median_filter.count - i - 1; j++) 
        {
            if (temp_buffer[j] > temp_buffer[j + 1]) 
            {
                temp = temp_buffer[j];
                temp_buffer[j] = temp_buffer[j + 1];
                temp_buffer[j + 1] = temp;
            }
        }
    }
    
    // 返回中值
    return temp_buffer[median_filter.count / 2];
}

/**
  * @brief  卡尔曼滤波
  * @param  new_value: 新的测量值
  * @retval 滤波后的值
  */
uint16_t Filter_Kalman(uint16_t new_value)
{
    // 预测步骤
    // X(k|k-1) = X(k-1|k-1)  (假设状态不变)
    // P(k|k-1) = P(k-1|k-1) + Q
    kalman_filter.P = kalman_filter.P + kalman_filter.Q;
    
    // 更新步骤
    // K(k) = P(k|k-1) / (P(k|k-1) + R)
    kalman_filter.K = kalman_filter.P / (kalman_filter.P + kalman_filter.R);
    
    // X(k|k) = X(k|k-1) + K(k) * (Z(k) - X(k|k-1))
    kalman_filter.X = kalman_filter.X + kalman_filter.K * ((float)new_value - kalman_filter.X);
    
    // P(k|k) = (1 - K(k)) * P(k|k-1)
    kalman_filter.P = (1.0f - kalman_filter.K) * kalman_filter.P;
    
    return (uint16_t)kalman_filter.X;
}

/**
  * @brief  一阶低通滤波（IIR滤波器）
  * @param  new_value: 新的采样值
  * @retval 滤波后的值
  */
uint16_t Filter_LowPass(uint16_t new_value)
{
    if (!lowpass_filter.initialized) 
    {
        lowpass_filter.output = (float)new_value;
        lowpass_filter.initialized = 1;
    } 
    else 
    {
        // Y(n) = α * X(n) + (1 - α) * Y(n-1)
        lowpass_filter.output = lowpass_filter.alpha * (float)new_value + 
                                (1.0f - lowpass_filter.alpha) * lowpass_filter.output;
    }
    
    return (uint16_t)lowpass_filter.output;
}

/**
  * @brief  去极值平均滤波
  * @param  new_value: 新的采样值
  * @retval 滤波后的值
  */
uint16_t Filter_TrimmedMean(uint16_t new_value)
{
    uint16_t temp_buffer[12];
    uint16_t temp;
    uint8_t i, j;
    uint32_t sum = 0;
    uint8_t valid_count;
    
    // 添加新值到缓冲区
    trimmed_filter.buffer[trimmed_filter.index] = new_value;
    trimmed_filter.index = (trimmed_filter.index + 1) % trimmed_filter.buffer_size;
    
    // 更新计数
    if (trimmed_filter.count < trimmed_filter.buffer_size) 
    {
        trimmed_filter.count++;
    }
    
    // 如果数据不够，返回简单平均
    if (trimmed_filter.count <= trimmed_filter.trim_count * 2) 
       {
        for (i = 0; i < trimmed_filter.count; i++) 
        {
            sum += trimmed_filter.buffer[i];
        }
        return (uint16_t)(sum / trimmed_filter.count);
    }
    
    // 复制并排序
    memcpy(temp_buffer, trimmed_filter.buffer, trimmed_filter.count * sizeof(uint16_t));
    
    // 冒泡排序
    for (i = 0; i < trimmed_filter.count - 1; i++) 
    {
        for (j = 0; j < trimmed_filter.count - i - 1; j++) 
        {
            if (temp_buffer[j] > temp_buffer[j + 1]) 
            {
                temp = temp_buffer[j];
                temp_buffer[j] = temp_buffer[j + 1];
                temp_buffer[j + 1] = temp;
            }
        }
    }
    
    // 去除极值后求平均
    valid_count = trimmed_filter.count - 2 * trimmed_filter.trim_count;
    for (i = trimmed_filter.trim_count; i < trimmed_filter.count - trimmed_filter.trim_count; i++) 
    {
        sum += temp_buffer[i];
    }
    
    return (uint16_t)(sum / valid_count);
}

/**
  * @brief  组合滤波器
  * @note   先用中值滤波去除脉冲干扰，再用卡尔曼滤波平滑
  * @param  new_value: 新的采样值
  * @retval 滤波后的值
  */
uint16_t Filter_Combined(uint16_t new_value)
{
    uint16_t median_output;
    uint16_t final_output;
    
    // 第一级：中值滤波去除异常值
    median_output = Filter_Median(new_value);
    
    // 第二级：卡尔曼滤波平滑输出
    final_output = Filter_Kalman(median_output);
    
    return final_output;
}

/**
  * @brief  自适应滤波器（根据波动自动调整）
  * @param  new_value: 新的采样值
  * @retval 滤波后的值
  */
uint16_t Filter_Adaptive(uint16_t new_value)
{
    static uint16_t last_value = 0;
    static float adaptive_alpha = 0.2f;
    uint16_t diff;
    uint16_t output;
    
    if (last_value == 0) 
    {
        last_value = new_value;
        return new_value;
    }
    
    // 计算变化量
    diff = (new_value > last_value) ? (new_value - last_value) : (last_value - new_value);
    
    // 根据变化量调整滤波系数
    if (diff > 50) 
    {
        // 大变化，可能是真实的转速突变
        adaptive_alpha = 0.5f;
    } 
    else if (diff > 20) 
    {
        // 中等变化
        adaptive_alpha = 0.3f;
    } 
    else
    {
        // 小变化，加强滤波
        adaptive_alpha = 0.1f;
    }
    
    // 应用低通滤波
    output = (uint16_t)(adaptive_alpha * new_value + (1.0f - adaptive_alpha) * last_value);
    last_value = output;
    
    return output;
}

/**
  * @brief  滤波器性能测试
  * @param  None
  * @retval None
  */
void Filter_Test(void)
{
    // 您的测试数据
    uint16_t test_data[] = 
    {
        747, 717, 748, 717, 748, 747, 718, 747, 748, 717,
        747, 748, 718, 747, 718, 748, 747, 717, 748, 718,
        747, 717, 748, 717, 748, 717, 747, 748, 718, 747,
        718, 748, 748, 717, 748, 718, 747, 747, 718, 747,
        718, 747, 717, 748, 718, 747, 717, 748, 748, 717,
        748, 718, 747, 717, 748, 747, 718, 747, 718, 747,
        717, 748, 747, 718, 748, 747
    };
    
    uint8_t data_count = sizeof(test_data) / sizeof(test_data[0]);
    uint8_t i;
    
    printf("\n=== 滤波器性能测试 ===\n");
    printf("原始数据波动范围: 717-748 RPM (±15 RPM)\n\n");
    
    // 初始化所有滤波器
    Filter_Init_All();
    
    printf("Index | Raw  | MovAvg | Median | Kalman | LowPass | Trimmed | Combined\n");
    printf("------|------|--------|--------|--------|---------|---------|----------\n");
    
    for (i = 0; i < data_count; i++) 
    {
        uint16_t raw = test_data[i];
        uint16_t mov_avg = Filter_MovingAverage(raw);
        uint16_t median = Filter_Median(raw);
        uint16_t kalman = Filter_Kalman(raw);
        uint16_t lowpass = Filter_LowPass(raw);
        uint16_t trimmed = Filter_TrimmedMean(raw);
        uint16_t combined = Filter_Combined(raw);
        
        printf("%5d | %4d | %6d | %6d | %6d | %7d | %7d | %8d\n",
               i, raw, mov_avg, median, kalman, lowpass, trimmed, combined);
    }
}

/**
  * @brief  获取滤波器统计信息
  * @param  None
  * @retval None
  */
void Filter_GetStatistics(void)
{
    printf("\n=== 滤波器配置参数 ===\n");
    printf("移动平均: 窗口大小 = %d\n", moving_avg_filter.buffer_size);
    printf("中值滤波: 窗口大小 = %d\n", median_filter.buffer_size);
    printf("卡尔曼滤波: Q=%.3f, R=%.1f\n", kalman_filter.Q, kalman_filter.R);
    printf("低通滤波: α = %.2f\n", lowpass_filter.alpha);
    printf("去极值平均: 窗口=%d, 去极值=%d\n", 
           trimmed_filter.buffer_size, trimmed_filter.trim_count);
}

