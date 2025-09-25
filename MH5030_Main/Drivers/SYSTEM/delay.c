#define DELAY_USE_SYSTICK   0   //0，使用定时器；1，使用systick
#define SYSTEM_SUPPORT_OS	1


#include "delay.h"
#if SYSTEM_SUPPORT_OS 						//如果需要支持OS.
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#endif

//////////////////////////////////////////////////////////////////////////////////


static u32 fac_us=72;						//us延时倍乘数
#if SYSTEM_SUPPORT_OS 						//如果需要支持OS.
static u16 fac_ms=1;				        //ms延时倍乘数,在os下,代表每个节拍的ms数
#endif

#if DELAY_USE_SYSTICK
    #define DELAY_COUNTUP   0                                       //计数模式
    #define DELAY_LOAD      (SysTick->LOAD)                         //重载值
    #define DELAY_TICKS(n)     (fac_us*(n))                                //每微秒计数
    #define DELAY_CNT       (SysTick->VAL)                          //计数值
#else
    #define DELAY_COUNTUP   1                                       //计数模式
    #define DELAY_LOAD      (DELAY_TIM->ARR)                        //重载值
    #define DELAY_TICKS(n)     (fac_us/(DELAY_TIM->PSC)*(n))               //每微秒的计数次数
    #define DELAY_CNT       (DELAY_TIM->CNT)                        //计数值
#endif

//初始化延迟函数
//当使用ucos的时候,此函数会初始化ucos的时钟节拍
//SYSTICK的时钟固定为AHB时钟
//SYSCLK:系统时钟频率，不开系统无用
void delay_init(u16 SYSCLK)
{
#if DELAY_USE_SYSTICK

#if SYSTEM_SUPPORT_OS 						//如果需要支持OS.
	u32 reload = SYSCLK;					    //每秒钟的计数次数 单位为K
	reload *= 1000000UL/configTICK_RATE_HZ;	//根据根据configTICK_RATE_HZ设定溢出时间
											//reload为24位寄存器,最大值:16777216,在180M下,约合0.745s左右
	fac_ms = 1000/configTICK_RATE_HZ;		//代表OS可以延时的最少单位
	SysTick->LOAD = reload - 1; 					//每1/OS_TICKS_PER_SEC秒中断一次
	SysTick->VAL = 0;
	SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;//开启SYSTICK中断
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk; //开启SYSTICK
#else
    u32 ticks;
//    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);//SysTick频率为HCLK
//	while (HAL_SYSTICK_Config(SystemCoreClock / (1000U / uwTickFreq)) > 0U);
    ticks = SystemCoreClock / (1000U / uwTickFreq);
    SysTick->LOAD  = (uint32_t)(ticks - 1UL);                         /* set reload register */
//    NVIC_SetPriority (SysTick_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL); /* set Priority for Systick Interrupt */
    SysTick->VAL   = 0UL;                                             /* Load the SysTick Counter Value */
    SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
//                   SysTick_CTRL_TICKINT_Msk   |
                   SysTick_CTRL_ENABLE_Msk;                         /* Enable SysTick IRQ and SysTick Timer */

#endif
#endif
	fac_us = SystemCoreClock/1000000UL;						//不论是否使用OS, fac_us都需要使用

}

#define DELAY_TIM   (TIM2)
//延时nus
//nus:要延时的us数.
//nus:0~190887435(最大值即2^32/fac_us@fac_us=22.5)
u32 delay_us(u32 nus)
{
	//使用定时器6，分频71，定时器频率为72M/(71+1)=1M，即1us
/*	u32 startCnt = __HAL_TIM_GET_COUNTER(DELAY_TIM);
	while ((__HAL_TIM_GET_COUNTER(DELAY_TIM) - startCnt) <= nus
        || (__HAL_TIM_GET_COUNTER(DELAY_TIM) + __HAL_TIM_GET_AUTORELOAD(DELAY_TIM) - startCnt) <= nus);
*/
	u32 ticks;
	u32 told,tnow,tcnt=0;
	u32 reload = DELAY_LOAD;				//LOAD的值
	ticks = DELAY_TICKS(nus); 						//需要的节拍数
	told = DELAY_CNT;        				//刚进入时的计数器值
	while(1)
	{
		tnow = DELAY_CNT;
		if(tnow != told)
		{
#if DELAY_COUNTUP
            tcnt += (tnow > told) ? (tnow - told) : (reload + tnow - told); //这里注意TIM6是一个递增的计数器
#else
			tcnt += (tnow < told) ? (told - tnow) : (reload + told - tnow);	//这里注意一下SYSTICK是一个递减的计数器就可以了.
#endif
			told = tnow;
			if(tcnt >= ticks)
            {
                break;          //时间超过/等于要延迟的时间,则退出.
			}
		}
	}
    return tcnt;
}

//延时nms,会引起任务调度
//nms:要延时的ms 数
//nms:0~65535
void delay_ms(u32 nms)
{
	u32 i;
#if SYSTEM_SUPPORT_OS 						//如果需要支持OS.
	if(xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)//系统已经运行
	{
		if(nms >= fac_ms) //延时的时间大于OS 的最少时间周期
		{
			vTaskDelay(nms/fac_ms); //FreeRTOS 延时
		}
		nms %= fac_ms; //OS 已经无法提供这么小的延时了,
		//采用普通方式延时
	}
#endif
	for(i=0;i<nms;i++) delay_us(1000);//普通方式延时
}


