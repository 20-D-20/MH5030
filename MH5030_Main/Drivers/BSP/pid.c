#include "pid.h"

/* PID自整定相关结构体变量 */
PidType g_stPidFront = {0};                   /* 前枪管PID控制器结构体 */
TuneObjectType g_stPidFrontAuto = {0};        /* 前枪管PID自整定对象结构体 */
FilterCtx g_stFilterFront = {0};              /* 前枪管滤波器结构体 */
PidType g_stPidRear = {0};                    /* 腔体PID控制器结构体 */
TuneObjectType g_stPidRearAuto = {0};         /* 腔体PID自整定对象结构体 */
FilterCtx g_stFilterRear = {0};               /* 腔体滤波器结构体 */

/* 测试/调试 变量 */
unsigned int full_flag = 0;                   /* 满标志 */
unsigned int num_test = 0;                    /* 测试变量1 */

/**
 * @brief     PID1参数初始化/前枪管温度加热PID参数配置
 * @param     vPID   PID控制器参数指针
 * @param     tune   自整定对象参数指针
 * @param     ctx    滤波上下文
 * @note      可选择Test(自整定)或Run(普通PID)两种工作模式
 * @retval    void
 */
void pid_front_init(PidType *vPID , TuneObjectType *tune, FilterCtx *filter)
{
    vPID->Sv = 120;                            /* 设定值 */
    vPID->T  = 200;                            /* 采样周期/积分周期 */
    vPID->deadzone = 0.02;                     /* 设置死区大小 */          
    vPID->pwmcycle = 1000;                     /* PWM周期 */
    vPID->OUT0 = 1;                            /* 输出初值 */
    vPID->C1ms = 0;                            /* 1ms计数器 */
    
#ifdef Test                                    /* 自整定模式 */
    vPID->Kp = 0;                              /* 比例系数 */
    vPID->Ti = 0;                              /* 积分时间 */
    vPID->Td = 0;                              /* 微分时间 */
#endif       
             
#ifdef Run                                     /* 普通PID模式 */
         
//    /* 自整定得到的pid参数 */      
//    pid->Kp = 0;                             /* 比例系数，毫秒级 */
//    pid->Ti = 0;                             /* 积分时间，毫秒级 */
//    pid->Td = 0;                             /* 微分时间，毫秒级 */
             
#endif
    
   /* 预处理使能、自整定使能均为一，PID自整定模式才可使用 */
    tune->preEnable        = 1;                /* 预处理使能置一 */
    tune->tuneEnable       = 1;                /* 自整定使能置一 */ 
                                               
   filter_init(filter, /*N=*/10, /*MEDIAN_N=*/5, /*limit_step=*/0.5f);    /* 滤波器参数配置 */
    
}

/**
 * @brief     PID2参数初始化/腔体温度加热PID参数配置
 * @param     vPID   PID控制器参数指针
 * @param     tune   自整定对象参数指针
 * @param     ctx    滤波上下文
 * @note      可选择Test(自整定)或Run(普通PID)两种工作模式
 * @retval    void
 */
void pid_rear_init(PidType *vPID ,TuneObjectType *tune, FilterCtx *filter)
{
    vPID->Sv = 120;                            /* 设定值 */
    vPID->T  = 200;                            /* 采样周期/积分周期 */
    vPID->deadzone = 0.02;                     /* 设置死区大小 */          
    vPID->pwmcycle = 1000;                     /* PWM周期 */
    vPID->OUT0 = 1;                            /* 输出初值 */
    vPID->C1ms = 0;                            /* 1ms计数器 */
    
#ifdef Test                                    /* 自整定模式 */
    vPID->Kp = 0;                              /* 比例系数 */
    vPID->Ti = 0;                              /* 积分时间 */
    vPID->Td = 0;                              /* 微分时间 */
#endif       
             
#ifdef Run                                     /* 普通PID模式 */
         
//    /* 自整定得到的pid参数 */      
//    pid->Kp = 0;                             /* 比例系数，毫秒级 */
//    pid->Ti = 0;                             /* 积分时间，毫秒级 */
//    pid->Td = 0;                             /* 微分时间，毫秒级 */
             
#endif

    /* 预处理使能、自整定使能均为一，PID自整定模式才可使用 */
   tune->preEnable        = 1;                /* 预处理使能置一 */
   tune->tuneEnable       = 1;                /* 自整定使能置一 */ 
   filter_init(filter, /*N=*/10, /*MEDIAN_N=*/5, /*limit_step=*/0.5f);    /* 滤波器参数配置 */
    
}

/**
 * @brief     PID自整定预处理初始化
 * @param     vPID   PID控制器参数指针
 * @param     tune   自整定对象参数指针
 * @retval    void
 * @note      初始化TuneObjectType各成员并根据当前设定/测量状态赋初值
 */
void TunePretreatment(PidType *vPID, TuneObjectType *tune)
{
    tune->maxPV            = 0;                /* 最大过程量清零 */
    tune->minPV            = 0;                /* 最小过程量清零 */
    tune->tuneTimer        = 0;                /* 自整定计时器 */
    tune->startTime        = 0;                /* 开始时间 */
    tune->endTime          = 0;                /* 结束时间 */
    tune->outputStep       = 1000;             /* 步进输出幅值/输出阶跃d */
    tune->tunePeriod       = 200;              /* 步进周期 */
    tune->controllerType   = 1;                /* 控制类型，默认1 */
                                               
    if (vPID->Sv >= vPID->Pv)                  /* 根据设定/过程值初始化状态 */
    {                                          
        tune->initialStatus  = 1;              /* 初始状态为1 */
        tune->outputStatus   = 0;              /* 输出状态为0 */
    }                                          
    else                                       
    {                                          
        tune->initialStatus  = 0;              
        tune->outputStatus   = 1;              
    }                                          
    tune->preEnable         = 0;               /* 预处理使能清零 */                                           
    tune->zeroAcrossCounter = 0;               /* 零点穿越计数清零 */
    tune->riseLagCounter    = 0;               /* 上升滞后计数清零 */
    tune->fallLagCounter    = 0;               /* 下降滞后计数清零 */
}                                              

/**
 * @brief  初始化滤波上下文（清零缓存与索引，设置窗口长度与参数）
 * @param  ctx        滤波上下文
 * @param  N          滑动平均窗口长度（<= SLIDING_MAX）
 * @param  MEDIAN_N   中值窗口长度（<= MEDIAN_MAX，建议奇数）
 * @param  limit_step 单步最大允许变化幅度 (°C)
 */
void filter_init(FilterCtx *ctx,uint8_t N,uint8_t MEDIAN_N,float limit_step)
{
    if (!ctx) 
    {
        return;
    }
    
    if (N == 0 || N > SLIDING_MAX)
    {
        N = SLIDING_MAX;                         /* 越界则钳制到最大 */
    } 

    if (MEDIAN_N == 0 || MEDIAN_N > MEDIAN_MAX)
    {
        MEDIAN_N = MEDIAN_MAX;                   /* 越界则钳制到最大 */
    }

    ctx->N          = N;
    ctx->MEDIAN_N   = MEDIAN_N;
    ctx->limit_step = limit_step;
    ctx->init_min   = 0.0f;                      /* 原逻辑：5~180 为可信范围 */
    ctx->init_max   = 230.0f;

    /* 清零缓存与状态 */
    memset(ctx->sliding_window, 0, sizeof(ctx->sliding_window));
    memset(ctx->median_buffer,  0, sizeof(ctx->median_buffer));
    ctx->index1     = 0;
    ctx->median_idx = 0;
    ctx->tp_flag    = 0;
    ctx->last_value = 0.0f;
}

/**
 * @brief  振幅限速滤波器（防止突变）
 * @param  ctx        滤波上下文
 * @param  new_value  新采集值
 * @return 滤波后的输出
 */
float amplitude_limit_filter(FilterCtx *ctx, float new_value)
{
    if (!ctx) return new_value;

    /* 首个可信值：初始化 last，并用该值预填充两个窗口，避免冷启动偏差 */
    if (ctx->tp_flag < 1 && new_value > ctx->init_min && new_value < ctx->init_max) 
   {
        ctx->last_value = new_value;
        for (uint8_t i = 0; i < ctx->N; i++)
        {
            ctx->sliding_window[i] = new_value;           /* 预填充滑动窗口 */
        }

        for (uint8_t i = 0; i < ctx->MEDIAN_N; i++)  
        {
            ctx->median_buffer[i]  = new_value;           /* 预填充中值窗口 */
        }
        ctx->index1     = 0;
        ctx->median_idx = 0;
        ctx->tp_flag    = 1;
        return new_value;
    }

    /* 尚未初始化：直接沿用上次值（默认 0） */
    if (ctx->tp_flag < 1) 
    {
        return ctx->last_value;
    }

    float diff = new_value - ctx->last_value;

    if (diff >  ctx->limit_step)
    {
        new_value = ctx->last_value + ctx->limit_step;    /* 上行限速 */
    }
    else if (diff < -ctx->limit_step) 
    {
        new_value = ctx->last_value - ctx->limit_step;    /* 下行限速 */
    }
    /* 否则直接使用 new_value */

    ctx->last_value = new_value;
    return new_value;
}

/**
 * @brief  中值滤波
 * @param  ctx        滤波上下文
 * @param  new_value  新采集值
 * @return 中值
 */
float median_filter(FilterCtx *ctx, float new_value)
{
    if (!ctx) return new_value;

    ctx->median_buffer[ctx->median_idx] = new_value;                           /* 写入环形缓冲 */
    ctx->median_idx = (uint8_t)((ctx->median_idx + 1) % ctx->MEDIAN_N);        /* 递增索引 */

    float temp[MEDIAN_MAX];                                                    /* 临时排序数组 */
    for (uint8_t i = 0; i < ctx->MEDIAN_N; i++)
    {
        temp[i] = ctx->median_buffer[i];
    }

    /* 简单选择排序（MEDIAN_N 较小，易读且足够快） */
    for (uint8_t i = 0; i < ctx->MEDIAN_N - 1; i++) 
    {
        uint8_t min_i = i;
        for (uint8_t j = i + 1; j < ctx->MEDIAN_N; j++) 
        {
            if (temp[j] < temp[min_i]) min_i = j;
        }
        if (min_i != i) 
        {
            float t = temp[i]; 
            temp[i] = temp[min_i]; 
            temp[min_i] = t;
        }
    }
    return temp[ctx->MEDIAN_N / 2];                                            /* 取中位数 */
}

/**
 * @brief  滑动平均滤波
 * @param  ctx        滤波上下文
 * @param  new_value  新采集值
 * @return 平均值
 */
float sliding_average_filter(FilterCtx *ctx, float new_value)
{
    if (!ctx) 
   {
        return new_value;
    }

    ctx->sliding_window[ctx->index1] = new_value;                              /* 写入环形缓冲 */
    ctx->index1 = (uint8_t)((ctx->index1 + 1) % ctx->N);                       /* 递增索引 */

    float sum = 0.0f;
    for (uint8_t i = 0; i < ctx->N; i++) 
    {
        sum += ctx->sliding_window[i];                                         /* 直接累加 */
    }
    return sum / (float)ctx->N;
}

/**
 * @brief  综合滤波器：限速 -> 中值 -> 滑动平均
 * @param  ctx        滤波上下文
 * @param  new_value  新采集值
 * @return 多级滤波结果
 */
float combined_filter(FilterCtx *ctx, float new_value)
{                                                           
    float v1 = amplitude_limit_filter(ctx, new_value);                         /* 限速 */
    float v2 = median_filter(ctx, v1);                                         /* 中值 */
    float v3 = sliding_average_filter(ctx, v2);                                /* 滑动平均 */
    return v3;                                              
}

/**
 * @brief     PID控制器计算
 * @param     PidType *vPID,TuneObjectType *tune
 * @return    PID输出值
 * @note      含比例、积分、微分环节，死区可有效避免抖动
 */
float PID_Calc(PidType *vPID,TuneObjectType *tune)
{
    if (tune->tuneEnable == 0)                                     /* 仅普通PID模式生效 */
    {
        num_test++;                                                /* 测试计数变量 */

        vPID->Ek = vPID->Sv - vPID->Pv;                            /* 误差计算 */

        if (fabsf(vPID->Ek) <= vPID->deadzone)                     /* 死区处理 */
        {
            return vPID->last_output;                              /* 死区内，输出不变 */
        }

        vPID->Pout = vPID->Kp * vPID->Ek;                          /* 比例输出 */
        vPID->SEk += vPID->Ek;                                     /* 误差累计积分 */

        float ki = vPID->Kp * (vPID->T / vPID->Ti);                /* Ki = Kp * T / Ti */
        vPID->Iout = ki * vPID->SEk;                               /* 积分输出 */

        float DelEk = vPID->Ek - vPID->Ek_1;                       /* 误差变化量 */
        float kd = vPID->Kp * (vPID->Td / vPID->T);                /* Kd = Kp * Td / T */
        vPID->Dout = kd * DelEk;                                   /* 微分输出 */

        float out = vPID->Pout + vPID->Iout + vPID->Dout;          /* PID三项求和 */

        if (out > vPID->pwmcycle)
        {
            out = vPID->pwmcycle;                                   /* 输出上限 */
        }
        else if (out <= 0)
        {
            out = vPID->OUT0;                                       /* 输出下限（可为0或底限） */
        }
        else
        {
            vPID->OUT = out;                                        /* 正常输出 */
        }

        vPID->Ek_1 = vPID->Ek;                                      /* 更新上次误差 */
        vPID->OUT = out;                                            /* 记录输出 */
        vPID->last_output = out;                                    /* 记录历史输出 */

        return out;                                                 /* 返回本次输出 */
    }
    return vPID->last_output;                                       /* 如果非普通模式，返回上一次输出 */
}

/**
 * @brief     PID自整定（继电器反馈法）
 * @param     vPID   PID控制器参数指针
 * @param     tune   自整定对象参数指针
 * @retval    void
 * @note      LAG_PHASE、full_flag等需外部定义
 */
void RelayFeedbackAutoTuning(PidType *vPID, TuneObjectType *tune)
{
    if (tune->tuneEnable == 1)                                       /* 若自整定使能，开始流程 */
    {
        uint32_t tuneDuration = 0;

        if (tune->preEnable == 1)                                    /* 首次进入或需重置，进行参数预处理 */
        {
            TunePretreatment(vPID, tune);                            /* 重置自整定辅助变量/状态 */
        }

        tune->tuneTimer++;                                           /* 自整定计时递增，单位周期 */
        tuneDuration = (tune->tuneTimer * tune->tunePeriod) / 1000;  /* 换算自整定已持续的秒数 */

        if (tuneDuration > 3600)                                     /* 超过最大自整定时长，强制退出 */
        {
            tune->tuneEnable = 2;                                    /* 2表示自整定失败或超时 */
            tune->preEnable = 1;                                     /* 预处理标志复位，为下次进入做准备 */
            return;
        }

        if (vPID->Sv >= vPID->Pv)                                    /* 当前设定值大于过程量，判断为“加热/输出高”阶段 */
        {
            tune->riseLagCounter++;                                  /* 上升延迟计数递增 */
            tune->fallLagCounter = 0;                                /* 下降延迟计数清零 */

            if (tune->riseLagCounter > LAG_PHASE)                    /* 上升阶段持续足够长，才切换输出状态 */
            {
                vPID->OUT = 1000;                                    /* 输出置为高电平/加热状态 */

                if (tune->outputStatus == 0)                         /* 仅在状态切换瞬间记录 */
                {
                    tune->outputStatus = 1;                          /* 记录输出高状态 */
                    tune->zeroAcrossCounter++;                       /* 零点穿越计数加1 */

                    if (tune->zeroAcrossCounter == 3)                /* 达到关键第3次穿越时记录开始时间 */
                    {
                        tune->startTime = tune->tuneTimer;
                    }
                }
            }
        }
        else                                                        /* 当前设定值小于过程量，判断为“降温/输出低”阶段 */
        {
            tune->riseLagCounter = 0;                               /* 上升延迟计数清零 */
            tune->fallLagCounter++;                                 /* 下降延迟计数递增 */

            if (tune->fallLagCounter > LAG_PHASE)                   /* 下降阶段持续足够长，才切换输出状态 */
            {
                vPID->OUT = 0;                                      /* 输出置为低电平/断开状态 */

                if (tune->outputStatus == 1)                        /* 仅在状态切换瞬间记录 */
                {
                    tune->outputStatus = 0;
                    tune->zeroAcrossCounter++;                      /* 零点穿越计数加1 */
                    full_flag = 1;                                  /* 标记自整定过程已经过一次完整正反过程 */

                    if (tune->zeroAcrossCounter == 3)               /* 达到第3次穿越时记录自整定开始时间 */
                    {
                        tune->startTime = tune->tuneTimer;
                    }
                }
            }
        }

        vPID->temp_old = vPID->Pv;                                  /* 记录过程量上次值（便于后续运算） */

        if (tune->zeroAcrossCounter == 3)                           /* 关键点：第3次零点穿越之后，统计波谷/波峰 */
        {
            tune->preEnable = 0;                                    /* 关闭预处理（只做一次） */

            if (tune->initialStatus == 1)                           /* 若初始状态为升温/加热 */
            {
                if (vPID->Pv < tune->minPV)
                {
                    tune->minPV = vPID->Pv;                         /* 更新最小过程量（波谷） */
                }
            }
            else if (tune->initialStatus == 0)                      /* 若初始状态为降温/断开 */
            {
                if (vPID->Pv > tune->maxPV)
                {
                    tune->maxPV = vPID->Pv;                         /* 更新最大过程量（波峰） */
                }
            }
        }
        else if (tune->zeroAcrossCounter == 4)                      /* 关键点：第4次零点穿越后，统计另一个极值 */
        {
            if (tune->initialStatus == 1)
            {
                if (vPID->Pv > tune->maxPV)
                {
                    tune->maxPV = vPID->Pv;                         /* 更新最大过程量（波峰） */
                }
            }
            else if (tune->initialStatus == 0)
            {
                if (vPID->Pv < tune->minPV)
                {
                    tune->minPV = vPID->Pv;                         /* 更新最小过程量（波谷） */
                }
            }
        }
        else if (tune->zeroAcrossCounter == 5)                      /* 达到5次穿越，认为完成了完整辨识周期 */
        {
            CalculationParameters(vPID, tune);                      /* 依据极值计算PID参数 */

            tune->tuneEnable = 0;                                   /* 关闭自整定功能 */
            tune->preEnable = 0;                                    /* 关闭预处理 */
            tune->endTime = tune->tuneTimer;                        /* 记录自整定结束时间 */
        }
    }
}

/********************************运行模式*********************************************/
//预热
void RUN(PidType *vPID,TuneObjectType *tune)
{
    if(tune->tuneEnable == 1)
    {
    if(  ((vPID->Sv) >=(vPID->Pv)) ) 
            {		
                
                tune->riseLagCounter++;
                tune->fallLagCounter=0;
            
        if(tune->riseLagCounter > LAG_PHASE)
        {
                    
                        vPID->OUT=1000;  
        
            if(tune->outputStatus==0)
            {
                tune->outputStatus=1;
                tune->zeroAcrossCounter++;
                
                if(tune->zeroAcrossCounter==3)
                {
                    tune->startTime=tune->tuneTimer;
                }
            }
        }
    }
    else                           	//设定值小于当前值
    {   
            
        tune->riseLagCounter=0;
            
        tune->fallLagCounter++;
        
        if(tune->fallLagCounter > LAG_PHASE)
        {
            vPID->OUT=0;
                            
            if(tune->outputStatus==1)
            {
                tune->outputStatus=0;
                tune->zeroAcrossCounter++;
                      full_flag=1;	
                                                            
                if(tune->zeroAcrossCounter==3)
                {tune->tuneEnable=0; 
                                        
                    tune->startTime = tune->tuneTimer;
                }
            }
        }
    }
        
    
    
    if(tune->zeroAcrossCounter==3)                  
    {
            tune->tuneEnable=0; 
    
    }
    
    }
}
/**
 * @brief     依据自整定结果计算PID参数（Ziegler-Nichols法）
 * @param     vPID   PID参数结构体指针
 * @param     tune   自整定结构体指针
 * @retval    void
 * @note      自动整定临界增益与周期，根据控制器类型赋值Kp、Ti、Td
 */
static void CalculationParameters(PidType *vPID, TuneObjectType *tune)
{
    float kc = 0.0;                                        /* 临界增益 */
    float tc = 0.0;                                        /* 临界周期 */
    float zn[3][3] =                                       /* Z-N经验参数表（0:P, 1:PI, 2:PID） */
    {
        {0.35, 0.8, 0.8},
        {0.9, 0.081, 0},
        {0.6, 0.5, 0.125}
    };

    tc = (tune->endTime - tune->startTime) * tune->tunePeriod / 1000.0;   /* 计算自整定周期(s) */
    kc = (8.0 * tune->outputStep) / (PI * (tune->maxPV - tune->minPV));   /* 计算自整定临界增益 */

    //#if PID_PARAMETER_STYLE > (0)
    //    *vPID->pKp=zn[tune->controllerType][0]*kc;                                   
    //    *vPID->pKi=*vPID->pKp*tune->tunePeriod/(zn[tune->controllerType][1]*tc*);     
    //    *vPID->pKd=*vPID->pKp*zn[tune->controllerType][2]*tc/tune->tunePeriod;       
    //#else
    ////    vPID->Kp=(100/(zn[tune->controllerType][0]*kc))/8;                             
    ////    vPID->Ti=(zn[tune->controllerType][1]*tc/10)*1.5;                             
    ////    vPID->Td=(zn[tune->controllerType][2]*tc/2000)*0.6;                           

    vPID->Kp = ((zn[tune->controllerType][0] * kc * 10));                      /* 计算比例增益Kp */
    vPID->Ti = (zn[tune->controllerType][1] * tc);                             /* 计算积分时间Ti */
    vPID->Td = (zn[tune->controllerType][2] * tc);                             /* 计算微分时间Td */
    //#endif

    printf("KP:%4.2f, TI:%4.2f, TD:%4.2f \r\n",vPID->Kp,vPID->Ti,vPID->Td);

}



////pid计算
//float PID_Calc(float deadzone)
//{
//pid.deadzone=	deadzone;
//	
//	if(PID_auto.tuneEnable == 0)
//	{
//		
//		
////		
////		if(pid.C1ms<(pid.T))  
//// 	  {
////    	 return pid.last_output ;
//// 	  }
////		
//		num_test++;
// 
//		pid.Ek=pid.Sv-pid.Pv;   
//		
//		if ( fabsf(pid.Ek)<= pid.deadzone) {
//        // 误差在死区内，保持上一次输出，不更新积分项

//        return pid.last_output;  //：保持上一次输出
//     
//    }
//		
//		
//		pid.Pout=pid.Kp*pid.Ek;     
// 
//		pid.SEk+=pid.Ek;        
// 
////		DelEk=pid.Ek-pid.Ek_1; 
//// 
////		ti=pid.T/pid.Ti;
////		ki=ti*pid.Kp;

////		pid.Iout=ki*pid.SEk;  

////		td=pid.Td/pid.T;		
//// 
////		kd=pid.Kp*td;
//// 
////		pid.Dout=kd*DelEk;   
//// 
//////	out= pid.Pout+ pid.Iout+ pid.Dout;
////	out= pid.Pout+ pid.Iout;
//			 float ki = pid.Kp * (pid.T / pid.Ti);      // Ki = Kp * T / Ti
//        pid.Iout = ki * pid.SEk;  

//        float DelEk = pid.Ek - pid.Ek_1;
//        float kd = pid.Kp * (pid.Td / pid.T);      // Kd = Kp * Td / T
//        pid.Dout = kd * DelEk;   

//        float out = pid.Pout + pid.Iout +pid.Dout;           // 如果用PID，加上 pid.Dout
//			 
//			 
//			 
//			 
//						if(out>pid.pwmcycle)
//						{
//							out=pid.pwmcycle;
//						}
//						else if(out<=0)
//						{
//                            out=pid.OUT0; 
//						}
//						else 
//						{
//							pid.OUT=out;
//						}
//						pid.Ek_1=pid.Ek;  
//						pid.OUT=out;
//						pid.last_output = out;
//					

//						
//}
//				
//}

