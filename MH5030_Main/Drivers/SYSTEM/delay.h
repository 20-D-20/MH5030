#ifndef _DELAY_H
#define _DELAY_H

#include "main.h"



void delay_init(u16 SYSCLK);
void delay_ms(u32 nms);
u32 delay_us(u32 nus);

#endif

