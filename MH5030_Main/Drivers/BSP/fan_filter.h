#ifndef __FAN_FILTER_H__
#define __FAN_FILTER_H__

#ifdef __cplusplus
extern "C" {
#endif// __cplusplus

void Filter_Init_All(void);
uint16_t Filter_MovingAverage(uint16_t new_value);
uint16_t Filter_Median(uint16_t new_value);
uint16_t Filter_Kalman(uint16_t new_value);
uint16_t Filter_LowPass(uint16_t new_value);
uint16_t Filter_TrimmedMean(uint16_t new_value);
uint16_t Filter_Combined(uint16_t new_value);
uint16_t Filter_Adaptive(uint16_t new_value);
void Filter_Test(void);

#ifdef __cplusplus
}
#endif 

#endif

