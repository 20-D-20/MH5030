#include "ntc.h"

#define R		(unsigned int)10000//240
#define R_25		(unsigned int)10000
#define REF_VOLTAGE	3.3f
#define NTC_B_VALUE		(unsigned int)3950
#define SAMPLES 10 // ������������
#define MAX_VALUE 150 // ���ֵ
#define MIN_VALUE 0 // ��Сֵ
#define SAMPLES 10 // ������������
#define FILTER_SIZE 5 // �˲�����С

uint16_t samples[SAMPLES]; // ����������ݵ�����
uint16_t filteredValue; // ���˺������
uint16_t Sum=0;
uint16_t lastvalue=0;

//==============================================================================
// @brief    ������ֵ���мн��˲��������书�ܣ�
// @param    value   ����Ĵ��˲���ֵ
// @return   �˲������ֵ
//==============================================================================
double clampFilter(double value)
{
    static double lastvalue = 0;                                   /* ������һ����� */

    if (value > 150) {
        lastvalue = 150;
        return 150;                                                /* ������н���150 */
    }
    else if (value < 0) {
        return lastvalue;                                          /* С��0�򷵻��ϴ�ֵ */
    }
    else {
        lastvalue = value;
        return value;                                              /* �������䣬ֱ���������¼ */
    }
}

//==============================================================================
// @brief    ��������ADCֵת��Ϊ�¶�ֵ����C��
// @param    adc_value   ADC�ɼ�����ԭʼֵ
// @return   ת������¶�ֵ����C��
//==============================================================================
double NTC_calculate_temperature(unsigned int adc_value)
{
    double fTemp;
    fTemp = R * adc_value / (4095.0f - adc_value);                 /* �����ѹ���� */
    fTemp /= R_25;
    fTemp = log(fTemp);
    fTemp /= NTC_B_VALUE;
    fTemp += 1 / (273.15 + 25);
    fTemp = 1 / fTemp;
    fTemp -= 273.15;
    return fTemp;                                                  /* �����¶� */
}

//==============================================================================
// @brief    ��ADC����ֵת��Ϊ��ѹֵ
// @param    ad     ADCԭʼ����ֵ
// @return   ��ѹֵ����λ��V��
//==============================================================================
float NTC_calculate_voltage(unsigned short ad)
{
    return ((float)ad) * REF_VOLTAGE / 4095.0f;                    /* ADCת��ѹ */
}

