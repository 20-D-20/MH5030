#include "ntc.h"

#define R		(unsigned int)10000//240
#define R_25		(unsigned int)10000
#define REF_VOLTAGE	3.3f
#define NTC_B_VALUE		(unsigned int)3950
#define SAMPLES 10 // 采样数据数量
#define MAX_VALUE 150 // 最大值
#define MIN_VALUE 0 // 最小值
#define SAMPLES 10 // 采样数据数量
#define FILTER_SIZE 5 // 滤波器大小

uint16_t samples[SAMPLES]; // 保存采样数据的数组
uint16_t filteredValue; // 过滤后的数据
uint16_t Sum=0;
uint16_t lastvalue=0;

//==============================================================================
// @brief    对输入值进行夹紧滤波（带记忆功能）
// @param    value   输入的待滤波数值
// @return   滤波后的数值
//==============================================================================
double clampFilter(double value)
{
    static double lastvalue = 0;                                   /* 记忆上一次输出 */

    if (value > 150) {
        lastvalue = 150;
        return 150;                                                /* 超限则夹紧到150 */
    }
    else if (value < 0) {
        return lastvalue;                                          /* 小于0则返回上次值 */
    }
    else {
        lastvalue = value;
        return value;                                              /* 正常区间，直接输出并记录 */
    }
}

//==============================================================================
// @brief    热敏电阻ADC值转换为温度值（°C）
// @param    adc_value   ADC采集到的原始值
// @return   转换后的温度值（°C）
//==============================================================================
double NTC_calculate_temperature(unsigned int adc_value)
{
    double fTemp;
    fTemp = R * adc_value / (4095.0f - adc_value);                 /* 电阻分压计算 */
    fTemp /= R_25;
    fTemp = log(fTemp);
    fTemp /= NTC_B_VALUE;
    fTemp += 1 / (273.15 + 25);
    fTemp = 1 / fTemp;
    fTemp -= 273.15;
    return fTemp;                                                  /* 返回温度 */
}

//==============================================================================
// @brief    将ADC采样值转换为电压值
// @param    ad     ADC原始采样值
// @return   电压值（单位：V）
//==============================================================================
float NTC_calculate_voltage(unsigned short ad)
{
    return ((float)ad) * REF_VOLTAGE / 4095.0f;                    /* ADC转电压 */
}

