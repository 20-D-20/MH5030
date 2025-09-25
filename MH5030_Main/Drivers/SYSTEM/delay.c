#define DELAY_USE_SYSTICK   0   //0��ʹ�ö�ʱ����1��ʹ��systick
#define SYSTEM_SUPPORT_OS	1


#include "delay.h"
#if SYSTEM_SUPPORT_OS 						//�����Ҫ֧��OS.
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#endif

//////////////////////////////////////////////////////////////////////////////////


static u32 fac_us=72;						//us��ʱ������
#if SYSTEM_SUPPORT_OS 						//�����Ҫ֧��OS.
static u16 fac_ms=1;				        //ms��ʱ������,��os��,����ÿ�����ĵ�ms��
#endif

#if DELAY_USE_SYSTICK
    #define DELAY_COUNTUP   0                                       //����ģʽ
    #define DELAY_LOAD      (SysTick->LOAD)                         //����ֵ
    #define DELAY_TICKS(n)     (fac_us*(n))                                //ÿ΢�����
    #define DELAY_CNT       (SysTick->VAL)                          //����ֵ
#else
    #define DELAY_COUNTUP   1                                       //����ģʽ
    #define DELAY_LOAD      (DELAY_TIM->ARR)                        //����ֵ
    #define DELAY_TICKS(n)     (fac_us/(DELAY_TIM->PSC)*(n))               //ÿ΢��ļ�������
    #define DELAY_CNT       (DELAY_TIM->CNT)                        //����ֵ
#endif

//��ʼ���ӳٺ���
//��ʹ��ucos��ʱ��,�˺������ʼ��ucos��ʱ�ӽ���
//SYSTICK��ʱ�ӹ̶�ΪAHBʱ��
//SYSCLK:ϵͳʱ��Ƶ�ʣ�����ϵͳ����
void delay_init(u16 SYSCLK)
{
#if DELAY_USE_SYSTICK

#if SYSTEM_SUPPORT_OS 						//�����Ҫ֧��OS.
	u32 reload = SYSCLK;					    //ÿ���ӵļ������� ��λΪK
	reload *= 1000000UL/configTICK_RATE_HZ;	//���ݸ���configTICK_RATE_HZ�趨���ʱ��
											//reloadΪ24λ�Ĵ���,���ֵ:16777216,��180M��,Լ��0.745s����
	fac_ms = 1000/configTICK_RATE_HZ;		//����OS������ʱ�����ٵ�λ
	SysTick->LOAD = reload - 1; 					//ÿ1/OS_TICKS_PER_SEC���ж�һ��
	SysTick->VAL = 0;
	SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;//����SYSTICK�ж�
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk; //����SYSTICK
#else
    u32 ticks;
//    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);//SysTickƵ��ΪHCLK
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
	fac_us = SystemCoreClock/1000000UL;						//�����Ƿ�ʹ��OS, fac_us����Ҫʹ��

}

#define DELAY_TIM   (TIM2)
//��ʱnus
//nus:Ҫ��ʱ��us��.
//nus:0~190887435(���ֵ��2^32/fac_us@fac_us=22.5)
u32 delay_us(u32 nus)
{
	//ʹ�ö�ʱ��6����Ƶ71����ʱ��Ƶ��Ϊ72M/(71+1)=1M����1us
/*	u32 startCnt = __HAL_TIM_GET_COUNTER(DELAY_TIM);
	while ((__HAL_TIM_GET_COUNTER(DELAY_TIM) - startCnt) <= nus
        || (__HAL_TIM_GET_COUNTER(DELAY_TIM) + __HAL_TIM_GET_AUTORELOAD(DELAY_TIM) - startCnt) <= nus);
*/
	u32 ticks;
	u32 told,tnow,tcnt=0;
	u32 reload = DELAY_LOAD;				//LOAD��ֵ
	ticks = DELAY_TICKS(nus); 						//��Ҫ�Ľ�����
	told = DELAY_CNT;        				//�ս���ʱ�ļ�����ֵ
	while(1)
	{
		tnow = DELAY_CNT;
		if(tnow != told)
		{
#if DELAY_COUNTUP
            tcnt += (tnow > told) ? (tnow - told) : (reload + tnow - told); //����ע��TIM6��һ�������ļ�����
#else
			tcnt += (tnow < told) ? (told - tnow) : (reload + told - tnow);	//����ע��һ��SYSTICK��һ���ݼ��ļ������Ϳ�����.
#endif
			told = tnow;
			if(tcnt >= ticks)
            {
                break;          //ʱ�䳬��/����Ҫ�ӳٵ�ʱ��,���˳�.
			}
		}
	}
    return tcnt;
}

//��ʱnms,�������������
//nms:Ҫ��ʱ��ms ��
//nms:0~65535
void delay_ms(u32 nms)
{
	u32 i;
#if SYSTEM_SUPPORT_OS 						//�����Ҫ֧��OS.
	if(xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)//ϵͳ�Ѿ�����
	{
		if(nms >= fac_ms) //��ʱ��ʱ�����OS ������ʱ������
		{
			vTaskDelay(nms/fac_ms); //FreeRTOS ��ʱ
		}
		nms %= fac_ms; //OS �Ѿ��޷��ṩ��ôС����ʱ��,
		//������ͨ��ʽ��ʱ
	}
#endif
	for(i=0;i<nms;i++) delay_us(1000);//��ͨ��ʽ��ʱ
}


