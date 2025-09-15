/**
  ******************************************************************************
  * @file    fan_filter.c
  * @brief   ����ת���˲��㷨����
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
  * @brief  �ƶ�ƽ���˲����ṹ��
  */
typedef struct {
    uint16_t buffer[16];       // ���ݻ�����
    uint8_t  buffer_size;      // ��������С
    uint8_t  index;            // ��ǰ����
    uint8_t  count;            // ���������
    uint32_t sum;              // �ۼƺ�
} MovingAvgFilter_t;

/**
  * @brief  ��ֵ�˲����ṹ��
  */
typedef struct {
    uint16_t buffer[9];        // ���ݻ���������������
    uint8_t  buffer_size;      // ��������С
    uint8_t  index;            // ��ǰ����
    uint8_t  count;            // ���������
} MedianFilter_t;

/**
  * @brief  �������˲����ṹ��
  */
typedef struct {
    float Q;                   // ��������Э����
    float R;                   // ��������Э����
    float P;                   // �������Э����
    float K;                   // ����������
    float X;                   // ״̬����ֵ
} KalmanFilter_t;

/**
  * @brief  һ�׵�ͨ�˲����ṹ��
  */
typedef struct {
    float alpha;               // �˲�ϵ�� (0-1)
    float output;              // ���ֵ
    uint8_t initialized;       // ��ʼ����־
} LowPassFilter_t;

/**
  * @brief  ���������˲�����ȥ����ֵ��
  */
typedef struct {
    uint16_t buffer[12];       // ���ݻ�����
    uint8_t  buffer_size;      // ��������С
    uint8_t  index;            // ��ǰ����
    uint8_t  count;            // ���������
    uint8_t  trim_count;       // ȥ���ļ�ֵ���������˸�ȥ����
} TrimmedMeanFilter_t;

/* Private variables ---------------------------------------------------------*/
static MovingAvgFilter_t moving_avg_filter;
static MedianFilter_t median_filter;
static KalmanFilter_t kalman_filter;
static LowPassFilter_t lowpass_filter;
static TrimmedMeanFilter_t trimmed_filter;

/* Function implementations --------------------------------------------------*/

/**
  * @brief  ��ʼ�������˲���
  * @param  None
  * @retval None
  */
void Filter_Init_All(void)
{
    // �ƶ�ƽ���˲�����ʼ��
    memset(&moving_avg_filter, 0, sizeof(MovingAvgFilter_t));
    moving_avg_filter.buffer_size = 8;  // ʹ��8��������
    
    // ��ֵ�˲�����ʼ��
    memset(&median_filter, 0, sizeof(MedianFilter_t));
    median_filter.buffer_size = 7;  // ʹ��7�������㣨������������
    
    // �������˲�����ʼ��
    kalman_filter.Q = 0.01f;    // ����������ֵԽСԽ����ģ��
    kalman_filter.R = 15.0f;    // ��������������ʵ�ʲ���������Լ��15 RPM��
    kalman_filter.P = 1.0f;     // ��ʼ�������
    kalman_filter.X = 730.0f;   // ��ʼ����ֵ��ȡ�м�ֵ��
    
    // һ�׵�ͨ�˲�����ʼ��
    lowpass_filter.alpha = 0.15f;  // �˲�ϵ����ԽСԽƽ����0.1-0.3�Ϻã�
    lowpass_filter.initialized = 0;
    
    // ȥ��ֵƽ���˲�����ʼ��
    memset(&trimmed_filter, 0, sizeof(TrimmedMeanFilter_t));
    trimmed_filter.buffer_size = 10;
    trimmed_filter.trim_count = 2;  // ȥ��������С��2��ֵ
}

/**
  * @brief  �ƶ�ƽ���˲�
  * @param  new_value: �µĲ���ֵ
  * @retval �˲����ֵ
  */
uint16_t Filter_MovingAverage(uint16_t new_value)
{
    // ����ǵ�һ������
    if (moving_avg_filter.count == 0) 
    {
        moving_avg_filter.sum = 0;
    }
    
    // �����������������ȥ��ɵ�ֵ
    if (moving_avg_filter.count >= moving_avg_filter.buffer_size) 
    {
        moving_avg_filter.sum -= moving_avg_filter.buffer[moving_avg_filter.index];
    } 
    else 
    {
        moving_avg_filter.count++;
    }
    
    // �����ֵ
    moving_avg_filter.buffer[moving_avg_filter.index] = new_value;
    moving_avg_filter.sum += new_value;
    
    // ��������
    moving_avg_filter.index = (moving_avg_filter.index + 1) % moving_avg_filter.buffer_size;
    
    // ����ƽ��ֵ
    return (uint16_t)(moving_avg_filter.sum / moving_avg_filter.count);
}

/**
  * @brief  ��ֵ�˲�
  * @param  new_value: �µĲ���ֵ
  * @retval �˲����ֵ
  */
uint16_t Filter_Median(uint16_t new_value)
{
    uint16_t temp_buffer[9];
    uint16_t temp;
    uint8_t i, j;
    
    // �����ֵ��������
    median_filter.buffer[median_filter.index] = new_value;
    median_filter.index = (median_filter.index + 1) % median_filter.buffer_size;
    
    // ���¼���
    if (median_filter.count < median_filter.buffer_size) 
    {
        median_filter.count++;
    }
    
    // �������ݵ���ʱ������
    memcpy(temp_buffer, median_filter.buffer, median_filter.count * sizeof(uint16_t));
    
    // ð������
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
    
    // ������ֵ
    return temp_buffer[median_filter.count / 2];
}

/**
  * @brief  �������˲�
  * @param  new_value: �µĲ���ֵ
  * @retval �˲����ֵ
  */
uint16_t Filter_Kalman(uint16_t new_value)
{
    // Ԥ�ⲽ��
    // X(k|k-1) = X(k-1|k-1)  (����״̬����)
    // P(k|k-1) = P(k-1|k-1) + Q
    kalman_filter.P = kalman_filter.P + kalman_filter.Q;
    
    // ���²���
    // K(k) = P(k|k-1) / (P(k|k-1) + R)
    kalman_filter.K = kalman_filter.P / (kalman_filter.P + kalman_filter.R);
    
    // X(k|k) = X(k|k-1) + K(k) * (Z(k) - X(k|k-1))
    kalman_filter.X = kalman_filter.X + kalman_filter.K * ((float)new_value - kalman_filter.X);
    
    // P(k|k) = (1 - K(k)) * P(k|k-1)
    kalman_filter.P = (1.0f - kalman_filter.K) * kalman_filter.P;
    
    return (uint16_t)kalman_filter.X;
}

/**
  * @brief  һ�׵�ͨ�˲���IIR�˲�����
  * @param  new_value: �µĲ���ֵ
  * @retval �˲����ֵ
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
        // Y(n) = �� * X(n) + (1 - ��) * Y(n-1)
        lowpass_filter.output = lowpass_filter.alpha * (float)new_value + 
                                (1.0f - lowpass_filter.alpha) * lowpass_filter.output;
    }
    
    return (uint16_t)lowpass_filter.output;
}

/**
  * @brief  ȥ��ֵƽ���˲�
  * @param  new_value: �µĲ���ֵ
  * @retval �˲����ֵ
  */
uint16_t Filter_TrimmedMean(uint16_t new_value)
{
    uint16_t temp_buffer[12];
    uint16_t temp;
    uint8_t i, j;
    uint32_t sum = 0;
    uint8_t valid_count;
    
    // �����ֵ��������
    trimmed_filter.buffer[trimmed_filter.index] = new_value;
    trimmed_filter.index = (trimmed_filter.index + 1) % trimmed_filter.buffer_size;
    
    // ���¼���
    if (trimmed_filter.count < trimmed_filter.buffer_size) 
    {
        trimmed_filter.count++;
    }
    
    // ������ݲ��������ؼ�ƽ��
    if (trimmed_filter.count <= trimmed_filter.trim_count * 2) 
       {
        for (i = 0; i < trimmed_filter.count; i++) 
        {
            sum += trimmed_filter.buffer[i];
        }
        return (uint16_t)(sum / trimmed_filter.count);
    }
    
    // ���Ʋ�����
    memcpy(temp_buffer, trimmed_filter.buffer, trimmed_filter.count * sizeof(uint16_t));
    
    // ð������
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
    
    // ȥ����ֵ����ƽ��
    valid_count = trimmed_filter.count - 2 * trimmed_filter.trim_count;
    for (i = trimmed_filter.trim_count; i < trimmed_filter.count - trimmed_filter.trim_count; i++) 
    {
        sum += temp_buffer[i];
    }
    
    return (uint16_t)(sum / valid_count);
}

/**
  * @brief  ����˲���
  * @note   ������ֵ�˲�ȥ��������ţ����ÿ������˲�ƽ��
  * @param  new_value: �µĲ���ֵ
  * @retval �˲����ֵ
  */
uint16_t Filter_Combined(uint16_t new_value)
{
    uint16_t median_output;
    uint16_t final_output;
    
    // ��һ������ֵ�˲�ȥ���쳣ֵ
    median_output = Filter_Median(new_value);
    
    // �ڶ������������˲�ƽ�����
    final_output = Filter_Kalman(median_output);
    
    return final_output;
}

/**
  * @brief  ����Ӧ�˲��������ݲ����Զ�������
  * @param  new_value: �µĲ���ֵ
  * @retval �˲����ֵ
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
    
    // ����仯��
    diff = (new_value > last_value) ? (new_value - last_value) : (last_value - new_value);
    
    // ���ݱ仯�������˲�ϵ��
    if (diff > 50) 
    {
        // ��仯����������ʵ��ת��ͻ��
        adaptive_alpha = 0.5f;
    } 
    else if (diff > 20) 
    {
        // �еȱ仯
        adaptive_alpha = 0.3f;
    } 
    else
    {
        // С�仯����ǿ�˲�
        adaptive_alpha = 0.1f;
    }
    
    // Ӧ�õ�ͨ�˲�
    output = (uint16_t)(adaptive_alpha * new_value + (1.0f - adaptive_alpha) * last_value);
    last_value = output;
    
    return output;
}

/**
  * @brief  �˲������ܲ���
  * @param  None
  * @retval None
  */
void Filter_Test(void)
{
    // ���Ĳ�������
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
    
    printf("\n=== �˲������ܲ��� ===\n");
    printf("ԭʼ���ݲ�����Χ: 717-748 RPM (��15 RPM)\n\n");
    
    // ��ʼ�������˲���
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
  * @brief  ��ȡ�˲���ͳ����Ϣ
  * @param  None
  * @retval None
  */
void Filter_GetStatistics(void)
{
    printf("\n=== �˲������ò��� ===\n");
    printf("�ƶ�ƽ��: ���ڴ�С = %d\n", moving_avg_filter.buffer_size);
    printf("��ֵ�˲�: ���ڴ�С = %d\n", median_filter.buffer_size);
    printf("�������˲�: Q=%.3f, R=%.1f\n", kalman_filter.Q, kalman_filter.R);
    printf("��ͨ�˲�: �� = %.2f\n", lowpass_filter.alpha);
    printf("ȥ��ֵƽ��: ����=%d, ȥ��ֵ=%d\n", 
           trimmed_filter.buffer_size, trimmed_filter.trim_count);
}

