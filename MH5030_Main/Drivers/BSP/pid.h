#ifndef _PID_H
#define _PID_H

#include "main.h"
#include "adc.h"
#include "gpio.h"
#include "ntc.h"
#include "stdio.h"
#include <math.h>
#include <string.h> 
#include "tps02r.h"

/* ---------------------- PIDģʽѡ�� ---------------------- */

//#define Test                              /* ����������pid���� */
#define Run                                 /* ��֪pid��������ʼ����ģʽ */

/* ---------------------- �˲����������� ---------------------- */

#define LAG_PHASE    10                   /* ����ʱ�� */
#define PI           3.1415
#define SLIDING_MAX   10                  /* ԭ N=10 */
#define MEDIAN_MAX     5                  /* ԭ MEDIAN_N=5������ 3~7���� �� MEDIAN_MAX �����Ϊ������ */

typedef struct {
    /* ���ò��� */
    uint8_t N;                            /* ����ƽ�����ڳ���, 1..SLIDING_MAX */
    uint8_t MEDIAN_N;                     /* ��ֵ���ڳ���, 1..MEDIAN_MAX�����������ʣ� */
    float   limit_step;                   /* ������ٲ��� (��C) */
    float   init_min;                     /* �״���Чֵ�ж��½� */
    float   init_max;                     /* �״���Чֵ�ж��Ͻ� */
                                        
    /* ״̬���뻺�� */                   
    float   last_value;                   /* ��һ�����ֵ */
    int     tp_flag;                      /* �Ƿ�����״���Ч��ʼ�� */
    float   sliding_window[SLIDING_MAX];  /* ����ƽ������ */
    uint8_t index1;                       /* ����ƽ������ */
    float   median_buffer[MEDIAN_MAX];    /* ��ֵ���� */
    uint8_t median_idx;                   /* ��ֵ���� */
} FilterCtx;                            

/* ---------------------- pid�������ṹ�嶨�� ---------------------- */

/**
 * @brief    PID�����������ṹ��
 * @note     ���ڱ���ȫ�����������в�����״̬����ʷ����
 */
typedef struct Pid
{
    float Sv;                             /* �û��趨ֵ��Set Value�� */
    float Pv;                             /* ��ǰ����ֵ��Process Value�� */
                
    float temp_old;                       /* �ϴβ���ֵ */
                
    float Kp;                             /* ����ϵ�� */
    int   T;                              /* PID����/��������(ms) */
    float Ti;                             /* ����ʱ�䳣�� */
    float Td;                             /* ΢��ʱ�䳣�� */
                
    float Ek;                             /* ��ǰƫ����Σ� */
    float Ek_1;                           /* ��һ��ƫ�� */
    float SEk;                            /* ��ʷƫ��ͣ����֣� */
                
    float Iout;                           /* ���ַ������ */
    float Pout;                           /* ����������� */
    float Dout;                           /* ΢�ַ������ */
                
    float OUT0;                           /* ������ޣ��������������� */
    float OUT;                            /* ʵ�ʿ������ֵ */
                
    int   C1ms;                           /* 1ms��ʱ�������ⲿ��ʱ�������ã� */
    int   pwmcycle;                       /* PWM���ڣ���ʱ���ֱ��ʣ� */
                
    int   times;                          /* ִ�д������� */
                
    float deadzone;                       /* ������ֵ��ƫ��С�ڴ�ֵʱ������� */
    float last_output;                    /* ��һ�����ֵ */

} PidType;



typedef struct TuneObject {
    uint8_t tuneEnable:2;                 //������PID���ƿ��أ�0��PID���ƣ�1������������2������ʧ��
    uint8_t preEnable:2;                  //Ԥ����ʹ�ܣ��ڿ�ʼ����ǰ��λ
    uint8_t initialStatus:1;              //��¼��ʼ����ǰƫ��ĳ�ʼ״̬
    uint8_t outputStatus:1;               //��¼����ĳ�ʼ״̬��0�����������������1�����½��������
    uint8_t controllerType:1;             //���������ͣ�0����������1����ɭ���ַ���PID��2��PID������
                                          
    uint8_t zeroAcrossCounter;            //������������ÿ������ı��1����ʵ�ʹ��������1
    uint8_t riseLagCounter;               //��������ʱ�������
    uint8_t fallLagCounter;               //�½�����ʱ�������
                                          
    uint16_t tunePeriod;                  //������������
    uint32_t tuneTimer;                   //������ʱ��
    uint32_t startTime;                   //��¼����������ʼʱ��
    uint32_t endTime;                     //��¼�������ڽ���ʱ��
                                          
    float outputStep;                     //�����Ծd
    float maxPV;                          //�񵴲����в���ֵ�����ֵ
    float minPV;                          //�񵴲����в���ֵ����Сֵ
}TuneObjectType;


/* ---------------------- ȫ�ֱ����������� ---------------------- */

extern PidType g_stPidFront;;                       /* PID������ȫ��ʵ�� */
extern TuneObjectType g_stPidFrontAuto;             /* PID����������ȫ��ʵ�� */
extern FilterCtx g_stFilterFront;                   /* ǰǹ���˲����ṹ�� */
extern PidType g_stPidRear;;                        /* PID������ȫ��ʵ�� */
extern TuneObjectType g_stPidRearAuto;              /* PID����������ȫ��ʵ�� */       
extern FilterCtx g_stFilterRear;                    /* ǻ���˲����ṹ�� */

/* ---------------------- ������������ ---------------------- */

void pid_front_init(PidType *vPID , TuneObjectType *tune, FilterCtx *filter);              //PID��ʼ��
void pid_rear_init(PidType *vPID ,TuneObjectType *tune, FilterCtx *filter);                 //PID��ʼ��
float median_filter(FilterCtx *ctx, float new_value);
void filter_init(FilterCtx *ctx,uint8_t N,uint8_t MEDIAN_N,float limit_step);
float sliding_average_filter(FilterCtx *ctx, float new_value);
float combined_filter(FilterCtx *ctx, float new_value);
float amplitude_limit_filter(FilterCtx *ctx, float new_value);

float PID_Calc(PidType *vPID,TuneObjectType *tune);                                    //PID����
void TunePretreatment(PidType *vPID,TuneObjectType *tune);                      //����������Ԥ����
void RelayFeedbackAutoTuning(PidType *vPID,TuneObjectType *tune);                      //������
void RUN(PidType *vPID,TuneObjectType *tune);
static void CalculationParameters(PidType *vPID,TuneObjectType *tune);                 //��������

#endif

