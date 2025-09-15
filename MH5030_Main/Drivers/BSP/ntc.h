#ifndef __NTC_H
#define __NTC_H

#include "main.h"
#include <math.h>
#include "adc.h"
#include "dma.h"

double NTC_calculate_temperature(unsigned int adc_value);
float NTC_calculate_voltage(unsigned short ad);
double clampFilter(double value);

#endif
