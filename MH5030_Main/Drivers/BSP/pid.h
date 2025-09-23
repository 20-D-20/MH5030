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

/* ---------------------- PID模式选择 ---------------------- */

//#define Test                              /* 自整定测量pid参数 */
#define Run                                 /* 已知pid参数，开始工作模式 */

/* ---------------------- 滤波器参数配置 ---------------------- */

#define LAG_PHASE    10                   /* 迟滞时间 */
#define PI           3.1415
#define SLIDING_MAX   10                  /* 原 N=10 */
#define MEDIAN_MAX     5                  /* 原 MEDIAN_N=5（建议 3~7，需 ≤ MEDIAN_MAX 且最好为奇数） */

typedef struct {
    /* 配置参数 */
    uint8_t N;                            /* 滑动平均窗口长度, 1..SLIDING_MAX */
    uint8_t MEDIAN_N;                     /* 中值窗口长度, 1..MEDIAN_MAX（奇数更合适） */
    float   limit_step;                   /* 振幅限速步长 (°C) */
    float   init_min;                     /* 首次有效值判定下界 */
    float   init_max;                     /* 首次有效值判定上界 */
                                        
    /* 状态量与缓存 */                   
    float   last_value;                   /* 上一次输出值 */
    int     tp_flag;                      /* 是否完成首次有效初始化 */
    float   sliding_window[SLIDING_MAX];  /* 滑动平均缓存 */
    uint8_t index1;                       /* 滑动平均索引 */
    float   median_buffer[MEDIAN_MAX];    /* 中值缓存 */
    uint8_t median_idx;                   /* 中值索引 */
} FilterCtx;                            

/* ---------------------- pid自增定结构体定义 ---------------------- */

/**
 * @brief    PID控制器参数结构体
 * @note     用于保存全部控制与运行参数、状态、历史量等
 */
typedef struct Pid
{
    float Sv;                             /* 用户设定值（Set Value） */
    float Pv;                             /* 当前测量值（Process Value） */
                
    float temp_old;                       /* 上次测量值 */
                
    float Kp;                             /* 比例系数 */
    int   T;                              /* PID计算/采样周期(ms) */
    float Ti;                             /* 积分时间常数 */
    float Td;                             /* 微分时间常数 */
                
    float Ek;                             /* 当前偏差（本次） */
    float Ek_1;                           /* 上一次偏差 */
    float SEk;                            /* 历史偏差和（积分） */
                
    float Iout;                           /* 积分分量输出 */
    float Pout;                           /* 比例分量输出 */
    float Dout;                           /* 微分分量输出 */
                
    float OUT0;                           /* 输出下限（可用作防死区） */
    float OUT;                            /* 实际控制输出值 */
                
    int   C1ms;                           /* 1ms定时计数（外部计时器辅助用） */
    int   pwmcycle;                       /* PWM周期（定时器分辨率） */
                
    int   times;                          /* 执行次数计数 */
                
    float deadzone;                       /* 死区阈值，偏差小于此值时输出不变 */
    float last_output;                    /* 上一次输出值 */

} PidType;



typedef struct TuneObject {
    uint8_t tuneEnable:2;                 //整定与PID控制开关，0：PID控制；1：参数整定；2：整定失败
    uint8_t preEnable:2;                  //预处理使能，在开始整定前置位
    uint8_t initialStatus:1;              //记录开始整定前偏差的初始状态
    uint8_t outputStatus:1;               //记录输出的初始状态，0允许上升过零计数；1允许下降过零计数
    uint8_t controllerType:1;             //控制器类型：0，不超调；1，佩森积分法则PID；2，PID控制器
                                          
    uint8_t zeroAcrossCounter;            //过零点计数器，每次输出改变加1，比实际过零次数多1
    uint8_t riseLagCounter;               //上升迟滞时间计数器
    uint8_t fallLagCounter;               //下降迟滞时间计数器
                                          
    uint16_t tunePeriod;                  //整定采样周期
    uint32_t tuneTimer;                   //整定计时器
    uint32_t startTime;                   //记录波形周期起始时间
    uint32_t endTime;                     //记录波形周期结束时间
                                          
    float outputStep;                     //输出阶跃d
    float maxPV;                          //振荡波形中测量值的最大值
    float minPV;                          //振荡波形中测量值的最小值
}TuneObjectType;


/* ---------------------- 全局变量声明区域 ---------------------- */

extern PidType g_stPidFront;;                       /* PID控制器全局实例 */
extern TuneObjectType g_stPidFrontAuto;             /* PID自整定对象全局实例 */
extern FilterCtx g_stFilterFront;                   /* 前枪管滤波器结构体 */
extern PidType g_stPidRear;;                        /* PID控制器全局实例 */
extern TuneObjectType g_stPidRearAuto;              /* PID自整定对象全局实例 */       
extern FilterCtx g_stFilterRear;                    /* 腔体滤波器结构体 */

/* ---------------------- 函数声明区域 ---------------------- */

void pid_front_init(PidType *vPID , TuneObjectType *tune, FilterCtx *filter);              //PID初始化
void pid_rear_init(PidType *vPID ,TuneObjectType *tune, FilterCtx *filter);                 //PID初始化
float median_filter(FilterCtx *ctx, float new_value);
void filter_init(FilterCtx *ctx,uint8_t N,uint8_t MEDIAN_N,float limit_step);
float sliding_average_filter(FilterCtx *ctx, float new_value);
float combined_filter(FilterCtx *ctx, float new_value);
float amplitude_limit_filter(FilterCtx *ctx, float new_value);

float PID_Calc(PidType *vPID,TuneObjectType *tune);                                    //PID计算
void TunePretreatment(PidType *vPID,TuneObjectType *tune);                      //自整定参数预处理
void RelayFeedbackAutoTuning(PidType *vPID,TuneObjectType *tune);                      //自整定
void RUN(PidType *vPID,TuneObjectType *tune);
static void CalculationParameters(PidType *vPID,TuneObjectType *tune);                 //参数计算

#endif

