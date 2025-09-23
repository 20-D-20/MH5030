#include "pid.h"

/* PID��������ؽṹ����� */
PidType g_stPidFront = {0};                   /* ǰǹ��PID�������ṹ�� */
TuneObjectType g_stPidFrontAuto = {0};        /* ǰǹ��PID����������ṹ�� */
FilterCtx g_stFilterFront = {0};              /* ǰǹ���˲����ṹ�� */
PidType g_stPidRear = {0};                    /* ǻ��PID�������ṹ�� */
TuneObjectType g_stPidRearAuto = {0};         /* ǻ��PID����������ṹ�� */
FilterCtx g_stFilterRear = {0};               /* ǻ���˲����ṹ�� */

/* ����/���� ���� */
unsigned int full_flag = 0;                   /* ����־ */
unsigned int num_test = 0;                    /* ���Ա���1 */

/**
 * @brief     PID1������ʼ��/ǰǹ���¶ȼ���PID��������
 * @param     vPID   PID����������ָ��
 * @param     tune   �������������ָ��
 * @param     ctx    �˲�������
 * @note      ��ѡ��Test(������)��Run(��ͨPID)���ֹ���ģʽ
 * @retval    void
 */
void pid_front_init(PidType *vPID , TuneObjectType *tune, FilterCtx *filter)
{
    vPID->Sv = 120;                            /* �趨ֵ */
    vPID->T  = 200;                            /* ��������/�������� */
    vPID->deadzone = 0.02;                     /* ����������С */          
    vPID->pwmcycle = 1000;                     /* PWM���� */
    vPID->OUT0 = 1;                            /* �����ֵ */
    vPID->C1ms = 0;                            /* 1ms������ */
    
#ifdef Test                                    /* ������ģʽ */
    vPID->Kp = 0;                              /* ����ϵ�� */
    vPID->Ti = 0;                              /* ����ʱ�� */
    vPID->Td = 0;                              /* ΢��ʱ�� */
#endif       
             
#ifdef Run                                     /* ��ͨPIDģʽ */
         
//    /* �������õ���pid���� */      
//    pid->Kp = 0;                             /* ����ϵ�������뼶 */
//    pid->Ti = 0;                             /* ����ʱ�䣬���뼶 */
//    pid->Td = 0;                             /* ΢��ʱ�䣬���뼶 */
             
#endif
    
   /* Ԥ����ʹ�ܡ�������ʹ�ܾ�Ϊһ��PID������ģʽ�ſ�ʹ�� */
    tune->preEnable        = 1;                /* Ԥ����ʹ����һ */
    tune->tuneEnable       = 1;                /* ������ʹ����һ */ 
                                               
   filter_init(filter, /*N=*/10, /*MEDIAN_N=*/5, /*limit_step=*/0.5f);    /* �˲����������� */
    
}

/**
 * @brief     PID2������ʼ��/ǻ���¶ȼ���PID��������
 * @param     vPID   PID����������ָ��
 * @param     tune   �������������ָ��
 * @param     ctx    �˲�������
 * @note      ��ѡ��Test(������)��Run(��ͨPID)���ֹ���ģʽ
 * @retval    void
 */
void pid_rear_init(PidType *vPID ,TuneObjectType *tune, FilterCtx *filter)
{
    vPID->Sv = 120;                            /* �趨ֵ */
    vPID->T  = 200;                            /* ��������/�������� */
    vPID->deadzone = 0.02;                     /* ����������С */          
    vPID->pwmcycle = 1000;                     /* PWM���� */
    vPID->OUT0 = 1;                            /* �����ֵ */
    vPID->C1ms = 0;                            /* 1ms������ */
    
#ifdef Test                                    /* ������ģʽ */
    vPID->Kp = 0;                              /* ����ϵ�� */
    vPID->Ti = 0;                              /* ����ʱ�� */
    vPID->Td = 0;                              /* ΢��ʱ�� */
#endif       
             
#ifdef Run                                     /* ��ͨPIDģʽ */
         
//    /* �������õ���pid���� */      
//    pid->Kp = 0;                             /* ����ϵ�������뼶 */
//    pid->Ti = 0;                             /* ����ʱ�䣬���뼶 */
//    pid->Td = 0;                             /* ΢��ʱ�䣬���뼶 */
             
#endif

    /* Ԥ����ʹ�ܡ�������ʹ�ܾ�Ϊһ��PID������ģʽ�ſ�ʹ�� */
   tune->preEnable        = 1;                /* Ԥ����ʹ����һ */
   tune->tuneEnable       = 1;                /* ������ʹ����һ */ 
   filter_init(filter, /*N=*/10, /*MEDIAN_N=*/5, /*limit_step=*/0.5f);    /* �˲����������� */
    
}

/**
 * @brief     PID������Ԥ�����ʼ��
 * @param     vPID   PID����������ָ��
 * @param     tune   �������������ָ��
 * @retval    void
 * @note      ��ʼ��TuneObjectType����Ա�����ݵ�ǰ�趨/����״̬����ֵ
 */
void TunePretreatment(PidType *vPID, TuneObjectType *tune)
{
    tune->maxPV            = 0;                /* ������������ */
    tune->minPV            = 0;                /* ��С���������� */
    tune->tuneTimer        = 0;                /* ��������ʱ�� */
    tune->startTime        = 0;                /* ��ʼʱ�� */
    tune->endTime          = 0;                /* ����ʱ�� */
    tune->outputStep       = 1000;             /* ���������ֵ/�����Ծd */
    tune->tunePeriod       = 200;              /* �������� */
    tune->controllerType   = 1;                /* �������ͣ�Ĭ��1 */
                                               
    if (vPID->Sv >= vPID->Pv)                  /* �����趨/����ֵ��ʼ��״̬ */
    {                                          
        tune->initialStatus  = 1;              /* ��ʼ״̬Ϊ1 */
        tune->outputStatus   = 0;              /* ���״̬Ϊ0 */
    }                                          
    else                                       
    {                                          
        tune->initialStatus  = 0;              
        tune->outputStatus   = 1;              
    }                                          
    tune->preEnable         = 0;               /* Ԥ����ʹ������ */                                           
    tune->zeroAcrossCounter = 0;               /* ��㴩Խ�������� */
    tune->riseLagCounter    = 0;               /* �����ͺ�������� */
    tune->fallLagCounter    = 0;               /* �½��ͺ�������� */
}                                              

/**
 * @brief  ��ʼ���˲������ģ����㻺�������������ô��ڳ����������
 * @param  ctx        �˲�������
 * @param  N          ����ƽ�����ڳ��ȣ�<= SLIDING_MAX��
 * @param  MEDIAN_N   ��ֵ���ڳ��ȣ�<= MEDIAN_MAX������������
 * @param  limit_step �����������仯���� (��C)
 */
void filter_init(FilterCtx *ctx,uint8_t N,uint8_t MEDIAN_N,float limit_step)
{
    if (!ctx) 
    {
        return;
    }
    
    if (N == 0 || N > SLIDING_MAX)
    {
        N = SLIDING_MAX;                         /* Խ����ǯ�Ƶ���� */
    } 

    if (MEDIAN_N == 0 || MEDIAN_N > MEDIAN_MAX)
    {
        MEDIAN_N = MEDIAN_MAX;                   /* Խ����ǯ�Ƶ���� */
    }

    ctx->N          = N;
    ctx->MEDIAN_N   = MEDIAN_N;
    ctx->limit_step = limit_step;
    ctx->init_min   = 0.0f;                      /* ԭ�߼���5~180 Ϊ���ŷ�Χ */
    ctx->init_max   = 230.0f;

    /* ���㻺����״̬ */
    memset(ctx->sliding_window, 0, sizeof(ctx->sliding_window));
    memset(ctx->median_buffer,  0, sizeof(ctx->median_buffer));
    ctx->index1     = 0;
    ctx->median_idx = 0;
    ctx->tp_flag    = 0;
    ctx->last_value = 0.0f;
}

/**
 * @brief  ��������˲�������ֹͻ�䣩
 * @param  ctx        �˲�������
 * @param  new_value  �²ɼ�ֵ
 * @return �˲�������
 */
float amplitude_limit_filter(FilterCtx *ctx, float new_value)
{
    if (!ctx) return new_value;

    /* �׸�����ֵ����ʼ�� last�����ø�ֵԤ����������ڣ�����������ƫ�� */
    if (ctx->tp_flag < 1 && new_value > ctx->init_min && new_value < ctx->init_max) 
   {
        ctx->last_value = new_value;
        for (uint8_t i = 0; i < ctx->N; i++)
        {
            ctx->sliding_window[i] = new_value;           /* Ԥ��们������ */
        }

        for (uint8_t i = 0; i < ctx->MEDIAN_N; i++)  
        {
            ctx->median_buffer[i]  = new_value;           /* Ԥ�����ֵ���� */
        }
        ctx->index1     = 0;
        ctx->median_idx = 0;
        ctx->tp_flag    = 1;
        return new_value;
    }

    /* ��δ��ʼ����ֱ�������ϴ�ֵ��Ĭ�� 0�� */
    if (ctx->tp_flag < 1) 
    {
        return ctx->last_value;
    }

    float diff = new_value - ctx->last_value;

    if (diff >  ctx->limit_step)
    {
        new_value = ctx->last_value + ctx->limit_step;    /* �������� */
    }
    else if (diff < -ctx->limit_step) 
    {
        new_value = ctx->last_value - ctx->limit_step;    /* �������� */
    }
    /* ����ֱ��ʹ�� new_value */

    ctx->last_value = new_value;
    return new_value;
}

/**
 * @brief  ��ֵ�˲�
 * @param  ctx        �˲�������
 * @param  new_value  �²ɼ�ֵ
 * @return ��ֵ
 */
float median_filter(FilterCtx *ctx, float new_value)
{
    if (!ctx) return new_value;

    ctx->median_buffer[ctx->median_idx] = new_value;                           /* д�뻷�λ��� */
    ctx->median_idx = (uint8_t)((ctx->median_idx + 1) % ctx->MEDIAN_N);        /* �������� */

    float temp[MEDIAN_MAX];                                                    /* ��ʱ�������� */
    for (uint8_t i = 0; i < ctx->MEDIAN_N; i++)
    {
        temp[i] = ctx->median_buffer[i];
    }

    /* ��ѡ������MEDIAN_N ��С���׶����㹻�죩 */
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
    return temp[ctx->MEDIAN_N / 2];                                            /* ȡ��λ�� */
}

/**
 * @brief  ����ƽ���˲�
 * @param  ctx        �˲�������
 * @param  new_value  �²ɼ�ֵ
 * @return ƽ��ֵ
 */
float sliding_average_filter(FilterCtx *ctx, float new_value)
{
    if (!ctx) 
   {
        return new_value;
    }

    ctx->sliding_window[ctx->index1] = new_value;                              /* д�뻷�λ��� */
    ctx->index1 = (uint8_t)((ctx->index1 + 1) % ctx->N);                       /* �������� */

    float sum = 0.0f;
    for (uint8_t i = 0; i < ctx->N; i++) 
    {
        sum += ctx->sliding_window[i];                                         /* ֱ���ۼ� */
    }
    return sum / (float)ctx->N;
}

/**
 * @brief  �ۺ��˲��������� -> ��ֵ -> ����ƽ��
 * @param  ctx        �˲�������
 * @param  new_value  �²ɼ�ֵ
 * @return �༶�˲����
 */
float combined_filter(FilterCtx *ctx, float new_value)
{                                                           
    float v1 = amplitude_limit_filter(ctx, new_value);                         /* ���� */
    float v2 = median_filter(ctx, v1);                                         /* ��ֵ */
    float v3 = sliding_average_filter(ctx, v2);                                /* ����ƽ�� */
    return v3;                                              
}

/**
 * @brief     PID����������
 * @param     PidType *vPID,TuneObjectType *tune
 * @return    PID���ֵ
 * @note      �����������֡�΢�ֻ��ڣ���������Ч���ⶶ��
 */
float PID_Calc(PidType *vPID,TuneObjectType *tune)
{
    if (tune->tuneEnable == 0)                                     /* ����ͨPIDģʽ��Ч */
    {
        num_test++;                                                /* ���Լ������� */

        vPID->Ek = vPID->Sv - vPID->Pv;                            /* ������ */

        if (fabsf(vPID->Ek) <= vPID->deadzone)                     /* �������� */
        {
            return vPID->last_output;                              /* �����ڣ�������� */
        }

        vPID->Pout = vPID->Kp * vPID->Ek;                          /* ������� */
        vPID->SEk += vPID->Ek;                                     /* ����ۼƻ��� */

        float ki = vPID->Kp * (vPID->T / vPID->Ti);                /* Ki = Kp * T / Ti */
        vPID->Iout = ki * vPID->SEk;                               /* ������� */

        float DelEk = vPID->Ek - vPID->Ek_1;                       /* ���仯�� */
        float kd = vPID->Kp * (vPID->Td / vPID->T);                /* Kd = Kp * Td / T */
        vPID->Dout = kd * DelEk;                                   /* ΢����� */

        float out = vPID->Pout + vPID->Iout + vPID->Dout;          /* PID������� */

        if (out > vPID->pwmcycle)
        {
            out = vPID->pwmcycle;                                   /* ������� */
        }
        else if (out <= 0)
        {
            out = vPID->OUT0;                                       /* ������ޣ���Ϊ0����ޣ� */
        }
        else
        {
            vPID->OUT = out;                                        /* ������� */
        }

        vPID->Ek_1 = vPID->Ek;                                      /* �����ϴ���� */
        vPID->OUT = out;                                            /* ��¼��� */
        vPID->last_output = out;                                    /* ��¼��ʷ��� */

        return out;                                                 /* ���ر������ */
    }
    return vPID->last_output;                                       /* �������ͨģʽ��������һ����� */
}

/**
 * @brief     PID���������̵�����������
 * @param     vPID   PID����������ָ��
 * @param     tune   �������������ָ��
 * @retval    void
 * @note      LAG_PHASE��full_flag�����ⲿ����
 */
void RelayFeedbackAutoTuning(PidType *vPID, TuneObjectType *tune)
{
    if (tune->tuneEnable == 1)                                       /* ��������ʹ�ܣ���ʼ���� */
    {
        uint32_t tuneDuration = 0;

        if (tune->preEnable == 1)                                    /* �״ν���������ã����в���Ԥ���� */
        {
            TunePretreatment(vPID, tune);                            /* ������������������/״̬ */
        }

        tune->tuneTimer++;                                           /* ��������ʱ��������λ���� */
        tuneDuration = (tune->tuneTimer * tune->tunePeriod) / 1000;  /* �����������ѳ��������� */

        if (tuneDuration > 3600)                                     /* �������������ʱ����ǿ���˳� */
        {
            tune->tuneEnable = 2;                                    /* 2��ʾ������ʧ�ܻ�ʱ */
            tune->preEnable = 1;                                     /* Ԥ�����־��λ��Ϊ�´ν�����׼�� */
            return;
        }

        if (vPID->Sv >= vPID->Pv)                                    /* ��ǰ�趨ֵ���ڹ��������ж�Ϊ������/����ߡ��׶� */
        {
            tune->riseLagCounter++;                                  /* �����ӳټ������� */
            tune->fallLagCounter = 0;                                /* �½��ӳټ������� */

            if (tune->riseLagCounter > LAG_PHASE)                    /* �����׶γ����㹻�������л����״̬ */
            {
                vPID->OUT = 1000;                                    /* �����Ϊ�ߵ�ƽ/����״̬ */

                if (tune->outputStatus == 0)                         /* ����״̬�л�˲���¼ */
                {
                    tune->outputStatus = 1;                          /* ��¼�����״̬ */
                    tune->zeroAcrossCounter++;                       /* ��㴩Խ������1 */

                    if (tune->zeroAcrossCounter == 3)                /* �ﵽ�ؼ���3�δ�Խʱ��¼��ʼʱ�� */
                    {
                        tune->startTime = tune->tuneTimer;
                    }
                }
            }
        }
        else                                                        /* ��ǰ�趨ֵС�ڹ��������ж�Ϊ������/����͡��׶� */
        {
            tune->riseLagCounter = 0;                               /* �����ӳټ������� */
            tune->fallLagCounter++;                                 /* �½��ӳټ������� */

            if (tune->fallLagCounter > LAG_PHASE)                   /* �½��׶γ����㹻�������л����״̬ */
            {
                vPID->OUT = 0;                                      /* �����Ϊ�͵�ƽ/�Ͽ�״̬ */

                if (tune->outputStatus == 1)                        /* ����״̬�л�˲���¼ */
                {
                    tune->outputStatus = 0;
                    tune->zeroAcrossCounter++;                      /* ��㴩Խ������1 */
                    full_flag = 1;                                  /* ��������������Ѿ���һ�������������� */

                    if (tune->zeroAcrossCounter == 3)               /* �ﵽ��3�δ�Խʱ��¼��������ʼʱ�� */
                    {
                        tune->startTime = tune->tuneTimer;
                    }
                }
            }
        }

        vPID->temp_old = vPID->Pv;                                  /* ��¼�������ϴ�ֵ�����ں������㣩 */

        if (tune->zeroAcrossCounter == 3)                           /* �ؼ��㣺��3����㴩Խ֮��ͳ�Ʋ���/���� */
        {
            tune->preEnable = 0;                                    /* �ر�Ԥ����ֻ��һ�Σ� */

            if (tune->initialStatus == 1)                           /* ����ʼ״̬Ϊ����/���� */
            {
                if (vPID->Pv < tune->minPV)
                {
                    tune->minPV = vPID->Pv;                         /* ������С�����������ȣ� */
                }
            }
            else if (tune->initialStatus == 0)                      /* ����ʼ״̬Ϊ����/�Ͽ� */
            {
                if (vPID->Pv > tune->maxPV)
                {
                    tune->maxPV = vPID->Pv;                         /* �����������������壩 */
                }
            }
        }
        else if (tune->zeroAcrossCounter == 4)                      /* �ؼ��㣺��4����㴩Խ��ͳ����һ����ֵ */
        {
            if (tune->initialStatus == 1)
            {
                if (vPID->Pv > tune->maxPV)
                {
                    tune->maxPV = vPID->Pv;                         /* �����������������壩 */
                }
            }
            else if (tune->initialStatus == 0)
            {
                if (vPID->Pv < tune->minPV)
                {
                    tune->minPV = vPID->Pv;                         /* ������С�����������ȣ� */
                }
            }
        }
        else if (tune->zeroAcrossCounter == 5)                      /* �ﵽ5�δ�Խ����Ϊ�����������ʶ���� */
        {
            CalculationParameters(vPID, tune);                      /* ���ݼ�ֵ����PID���� */

            tune->tuneEnable = 0;                                   /* �ر����������� */
            tune->preEnable = 0;                                    /* �ر�Ԥ���� */
            tune->endTime = tune->tuneTimer;                        /* ��¼����������ʱ�� */
        }
    }
}

/********************************����ģʽ*********************************************/
//Ԥ��
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
    else                           	//�趨ֵС�ڵ�ǰֵ
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
 * @brief     �����������������PID������Ziegler-Nichols����
 * @param     vPID   PID�����ṹ��ָ��
 * @param     tune   �������ṹ��ָ��
 * @retval    void
 * @note      �Զ������ٽ����������ڣ����ݿ��������͸�ֵKp��Ti��Td
 */
static void CalculationParameters(PidType *vPID, TuneObjectType *tune)
{
    float kc = 0.0;                                        /* �ٽ����� */
    float tc = 0.0;                                        /* �ٽ����� */
    float zn[3][3] =                                       /* Z-N���������0:P, 1:PI, 2:PID�� */
    {
        {0.35, 0.8, 0.8},
        {0.9, 0.081, 0},
        {0.6, 0.5, 0.125}
    };

    tc = (tune->endTime - tune->startTime) * tune->tunePeriod / 1000.0;   /* ��������������(s) */
    kc = (8.0 * tune->outputStep) / (PI * (tune->maxPV - tune->minPV));   /* �����������ٽ����� */

    //#if PID_PARAMETER_STYLE > (0)
    //    *vPID->pKp=zn[tune->controllerType][0]*kc;                                   
    //    *vPID->pKi=*vPID->pKp*tune->tunePeriod/(zn[tune->controllerType][1]*tc*);     
    //    *vPID->pKd=*vPID->pKp*zn[tune->controllerType][2]*tc/tune->tunePeriod;       
    //#else
    ////    vPID->Kp=(100/(zn[tune->controllerType][0]*kc))/8;                             
    ////    vPID->Ti=(zn[tune->controllerType][1]*tc/10)*1.5;                             
    ////    vPID->Td=(zn[tune->controllerType][2]*tc/2000)*0.6;                           

    vPID->Kp = ((zn[tune->controllerType][0] * kc * 10));                      /* �����������Kp */
    vPID->Ti = (zn[tune->controllerType][1] * tc);                             /* �������ʱ��Ti */
    vPID->Td = (zn[tune->controllerType][2] * tc);                             /* ����΢��ʱ��Td */
    //#endif

    printf("KP:%4.2f, TI:%4.2f, TD:%4.2f \r\n",vPID->Kp,vPID->Ti,vPID->Td);

}



////pid����
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
//        // ����������ڣ�������һ������������»�����

//        return pid.last_output;  //��������һ�����
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

//        float out = pid.Pout + pid.Iout +pid.Dout;           // �����PID������ pid.Dout
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

