#include <string.h>
#include "I2c.h"
#include "delay.h"

/***************************************************************************************************/

/**
 * @brief      使能指定GPIO端口的时钟（基于HAL库实现）
 * @param[in]  pGPIO   指向GPIO端口的指针（如GPIOA, GPIOB等）
 * @retval     u8      操作结果
 *                    - 0: 时钟使能成功
 *                    - 1: 无效GPIO端口或使能失败
 * @note       通过判断GPIO基地址调用HAL库时钟使能宏
 */
u8 hal_gpio_rcc_clk_en(GPIO_TypeDef *pGPIO)
{
    switch ((u32)pGPIO)
    {
    case GPIOA_BASE:
        __HAL_RCC_GPIOA_CLK_ENABLE();                  /* 使能GPIOA时钟 */
        break;
    case GPIOB_BASE:
        __HAL_RCC_GPIOB_CLK_ENABLE();                  /* 使能GPIOB时钟 */
        break;
    case GPIOC_BASE:
        __HAL_RCC_GPIOC_CLK_ENABLE();                  /* 使能GPIOC时钟 */
        break;
#ifdef GPIOD
    case GPIOD_BASE:
        __HAL_RCC_GPIOD_CLK_ENABLE();                  /* 使能GPIOD时钟 */
        break;
#endif
#ifdef GPIOE
    case GPIOE_BASE:
        __HAL_RCC_GPIOE_CLK_ENABLE();                  /* 使能GPIOE时钟 */
        break;
#endif
#ifdef GPIOF
    case GPIOF_BASE:
        __HAL_RCC_GPIOF_CLK_ENABLE();                  /* 使能GPIOF时钟 */
        break;
#endif
#ifdef GPIOG
    case GPIOG_BASE:
        __HAL_RCC_GPIOG_CLK_ENABLE();                  /* 使能GPIOG时钟 */
        break;
#endif
#ifdef GPIOH
    case GPIOH_BASE:
        __HAL_RCC_GPIOH_CLK_ENABLE();                  /* 使能GPIOH时钟 */
        break;
#endif
#ifdef GPIOI
    case GPIOI_BASE:
        __HAL_RCC_GPIOI_CLK_ENABLE();                  /* 使能GPIOI时钟 */
        break;
#endif
#ifdef GPIOJ
    case GPIOJ_BASE:
        __HAL_RCC_GPIOJ_CLK_ENABLE();                  /* 使能GPIOJ时钟 */
        break;
#endif
#ifdef GPIOK
    case GPIOK_BASE:
        __HAL_RCC_GPIOK_CLK_ENABLE();                  /* 使能GPIOK时钟 */
        break;
#endif
    default:
        return 1;                                     /* 不支持的GPIO端口 */
    }

    return 0;                                         /* 时钟使能成功 */
}

/**
 * @brief      判断是否只选择了一个 GPIO 引脚（即是否是单个引脚位）
 * @param[in]  u32GPIOPin  GPIO引脚掩码（如 GPIO_PIN_0, GPIO_PIN_5 等）
 * @retval     0           是单个有效引脚
 * @retval     1           无效或多引脚组合
 */
u8 is_one_pin(u32 u32GPIOPin)
{
    u8 i;

    for (i = 0; i < 16; i++)                                            /* 遍历 GPIO 的 16 个有效位 */
    {                   
        if (u32GPIOPin & 0x00000001)                                    /* 当前最低位是1 */
        {                   
            if (u32GPIOPin == 1)                    
                return 0;                                               /* 仅1位为1，是单个pin */
                
            return 1;                                                   /* 多位为1，不是单个pin */
        }                   
        u32GPIOPin >>= 1;                                               /* 右移，检测下一位 */
    }                   
                
    return 1;                                                           /* 无1位，不是单个pin */
}               



void gpio_init_error_handler(u8 error)//错误处理函数
{
    while(error)
    {
        //To do: 错误处理
    }
}

/**
 * @brief      初始化GPIO引脚
 * @param[in]  pstGPIO  指向MyGpio_ST结构体的指针，包含GPIO端口和引脚信息
 * @param[in]  Speed    GPIO速度（如GPIO_SPEED_FREQ_LOW/MEDIUM/HIGH）
 * @param[in]  Mode     GPIO模式（如GPIO_MODE_OUTPUT_PP/GPIO_MODE_OUTPUT_OD等）
 * @note       1. 检查是否为单个有效引脚
 *             2. 使能GPIO端口时钟
 *             3. 配置参数并初始化
 */
void gpio_pin_init(MyGpio_ST *pstGPIO, u32 Speed, u32 Mode)
{
    GPIO_InitTypeDef GPIO_Initure;                                      /* GPIO初始化结构体 */

    gpio_init_error_handler(is_one_pin(pstGPIO->u32Pin));               /* 检查单个有效引脚 */
    gpio_init_error_handler(hal_gpio_rcc_clk_en(pstGPIO->pstGTD));      /* 使能GPIO时钟 */

    GPIO_Initure.Speed = Speed;                                         /* 设置GPIO速度 */
    GPIO_Initure.Pin   = pstGPIO->u32Pin;                               /* 设置初始化的引脚 */
    GPIO_Initure.Mode  = Mode;                                          /* 设置GPIO模式 */
    GPIO_Initure.Pull  = GPIO_PULLUP;                                   /* 默认上拉 */

    HAL_GPIO_Init(pstGPIO->pstGTD, &GPIO_Initure);                      /* 调用HAL库初始化GPIO */
}

/**
 * @brief      初始化I2C的WP（写保护）引脚
 * @param[out] pstGpio   指向MyGpio_ST结构体的指针
 * @param[in]  pstGTD    GPIO端口（如GPIOA, GPIOB等）
 * @param[in]  Pin       GPIO引脚号（如GPIO_PIN_0等）
 * @note       配置为低速推挽输出模式，默认用于控制存储器的写保护功能
 */
static void iic_wp_init(MyGpio_ST* pstGpio, GPIO_TypeDef *pstGTD, u32 Pin)
{
    pstGpio->pstGTD = pstGTD;                                            /* 设置GPIO端口 */
    pstGpio->u32Pin = Pin;                                               /* 设置引脚号 */
    gpio_pin_init(pstGpio, GPIO_SPEED_FREQ_LOW, GPIO_MODE_OUTPUT_PP);    /* 低速推挽输出 */
}


/**
 * @brief      初始化I2C的SCL（时钟线）引脚
 * @param[out] pstGpio   指向MyGpio_ST结构体的指针
 * @param[in]  pstGTD    GPIO端口（如GPIOA, GPIOB等）
 * @param[in]  Pin       GPIO引脚号（如GPIO_PIN_0等）
 * @note       配置为低速推挽输出模式，符合I2C标准要求
 */
static void iic_scl_init(MyGpio_ST *pstGpio, GPIO_TypeDef *pstGTD, u32 Pin)
{
    pstGpio->pstGTD = pstGTD;                                            /* 设置GPIO端口 */
    pstGpio->u32Pin = Pin;                                               /* 设置引脚号 */
    gpio_pin_init(pstGpio, GPIO_SPEED_FREQ_LOW, GPIO_MODE_OUTPUT_PP);    /* 低速推挽输出 */
}


/**
 * @brief      初始化I2C的SDA（数据线）引脚
 * @param[out] pstGpio   指向MyGpio_ST结构体的指针
 * @param[in]  pstGTD    GPIO端口（如GPIOA, GPIOB等）
 * @param[in]  Pin       GPIO引脚号（如GPIO_PIN_0等）
 * @note       配置为低速开漏输出模式，符合I2C标准（支持仲裁和同步）
 */
static void iic_sda_init(MyGpio_ST *pstGpio, GPIO_TypeDef *pstGTD, u32 Pin)
{
    pstGpio->pstGTD = pstGTD;                                            /* 设置GPIO端口 */
    pstGpio->u32Pin = Pin;                                               /* 设置引脚号 */
    gpio_pin_init(pstGpio, GPIO_SPEED_FREQ_LOW, GPIO_MODE_OUTPUT_OD);    /* 低速开漏输出 */
}

void _iic_delay_us(IICPara_ST* pstPara)                                  //微秒级延时
{
    delay_us(pstPara->u16DelayUs);
}

void _iic_delay_cnt(IICPara_ST* pstPara)                                 //低于1微秒的延时，非精确
{
    u16 i = pstPara->u16DelayUs;
    while(i--);
}

/**
 * @brief      产生I2C起始信号
 * @param[in]  pstPara  指向IIC参数结构体的指针
 * @note       时序：SDA高→SCL高→SDA低(在SCL高)→SCL低，操作间有延时
 */
void iic_start(IICPara_ST *pstPara)
{
    _iic_sda_h(pstPara);                                            /* SDA线置高 */
    pstPara->iic_delay(pstPara);                                    /* 保持延时 */
                        
    _iic_scl_h(pstPara);                                            /* SCL线置高 */
    pstPara->iic_delay(pstPara);                                    /* 保持延时 */
                        
    _iic_sda_l(pstPara);                                            /* SCL高时SDA由高变低(起始条件) */
    pstPara->iic_delay(pstPara);                                    /* 保持延时 */
                        
    _iic_scl_l(pstPara);                                            /* SCL线置低，准备数据传输 */
    pstPara->iic_delay(pstPara);                                    /* 保持延时 */
}


/**
 * @brief      产生I2C停止信号
 * @param[in]  pstPara  指向IIC参数结构体的指针
 * @note       时序：SDA低→SCL高→SDA高(在SCL高)，操作间有延时
 */
void iic_stop(IICPara_ST *pstPara)
{
    _iic_sda_l(pstPara);                                            /* SDA线置低 */
    pstPara->iic_delay(pstPara);                                    /* 保持延时 */
                        
    _iic_scl_h(pstPara);                                            /* SCL线置高 */
    pstPara->iic_delay(pstPara);                                    /* 保持延时 */
                        
    _iic_sda_h(pstPara);                                            /* SCL高时SDA由低变高(停止条件) */
    pstPara->iic_delay(pstPara);                                    /* 保持延时 */
}                       


/**
 * @brief      产生ACK/NACK应答信号
 * @param[in]  pstPara  指向IIC参数结构体的指针
 * @param[in]  Ack      应答标志：1-发送ACK，0-发送NACK
 * @note       SCL低→SDA设置→SCL高→SCL低，操作间均有延时
 */
static void iic_ack(IICPara_ST *pstPara, u8 Ack)
{
    _iic_scl_l(pstPara);                                            /* SCL线置低 */
    pstPara->iic_delay(pstPara);                                    /* 保持延时 */
                        
    if (Ack)                        
        _iic_sda_l(pstPara);                                        /* SDA低，ACK应答 */
    else                        
        _iic_sda_h(pstPara);                                        /* SDA高，NACK应答 */
    pstPara->iic_delay(pstPara);                                    /* 保持延时 */
                        
    _iic_scl_h(pstPara);                                            /* SCL线置高，主机输出应答信号 */
    pstPara->iic_delay(pstPara);                                    /* 保持延时 */
                        
    _iic_scl_l(pstPara);                                            /* SCL线置低，完成应答周期 */
    pstPara->iic_delay(pstPara);                                    /* 保持延时 */
}                       

/**
 * @brief      等待并检查I2C从设备的应答信号(ACK)
 * @param[in]  pstPara   指向IIC参数结构体的指针
 * @retval     u8        返回应答状态
 *                      - 0: 收到ACK应答(从设备正常响应)
 *                      - 1: 未收到ACK应答(超时或从设备无响应)
 * @note       检测过程包括SCL拉低、SDA释放、SCL拉高循环检测和结果返回
 */
static u8 iic_wait_ack(IICPara_ST *pstPara)
{
    u16 i = ACK_WAIT_TIMES;                                         /* 初始化超时计数器 */
                
    _iic_scl_l(pstPara);                                            /* SCL拉低，准备ACK检测 */
    pstPara->iic_delay(pstPara);                                    /* 保持延时 */
                
    _iic_sda_h(pstPara);                                            /* SDA释放，进入输入模式 */
    pstPara->iic_delay(pstPara);                                    /* 保持延时 */
                
    _iic_scl_h(pstPara);                                            /* SCL拉高，等待从机ACK */
    pstPara->iic_delay(pstPara);                                    /* 保持延时 */
                
    while (--i && _iic_sda_read(pstPara))                           /* 检测SDA低电平(ACK)或超时 */
    {               
        pstPara->iic_delay(pstPara);                                /* 检测后延时 */
    }               
                
    _iic_scl_l(pstPara);                                            /* SCL拉低，结束ACK检测 */
    pstPara->iic_delay(pstPara);                                    /* 保持延时 */
                
    _iic_sda_h(pstPara);                                            /* SDA保持高电平 */
    pstPara->iic_delay(pstPara);                                    /* 保持延时 */
                
    return !i;                                                      /* i不为0则收到ACK(0)，否则超时(1) */
}

/**
 * @brief      I2C总线发送一个字节数据
 * @param[in]  pstPara     指向IIC参数结构体的指针，包含总线配置信息
 * @param[in]  SendByte    要发送的字节数据
 * @retval     u8          返回应答状态
 *                         - 0: 收到ACK应答(从设备正常接收)
 *                         - 1: 无ACK应答(从设备未响应或接收失败)
 * @note       严格的时序控制确保I2C协议兼容性
 */
u8 iic_send_byte(IICPara_ST *pstPara, u8 SendByte)
{
    u8 i = 8;                                                       /* 8位数据计数器 */
            
    while (i--)                                                     /* 逐位发送字节数据(MSB优先) */
    {           
        _iic_scl_l(pstPara);                                        /* SCL拉低，准备改变SDA数据 */
        delay_us(pstPara->u16DelayUs / 2);                          /* 半周期延时 */
            
        if (SendByte & 0x80)                                        /* 最高位(MSB)为1 */
            _iic_sda_h(pstPara);                                    /* 发送'1' */
        else            
            _iic_sda_l(pstPara);                                    /* 发送'0' */
        delay_us(pstPara->u16DelayUs / 2);                          /* 半周期延时 */
            
        _iic_scl_h(pstPara);                                        /* SCL拉高，从机采样数据 */
        pstPara->iic_delay(pstPara);                                /* 完整周期延时 */
            
        SendByte <<= 1;                                             /* 左移准备下一位 */
    }           
            
    _iic_scl_l(pstPara);                                            /* SCL拉低，结束数据传输 */
    pstPara->iic_delay(pstPara);                                    /* 保持延时 */
            
    return iic_wait_ack(pstPara);                                   /* 检查从设备应答 */
}


/**
 * @brief      从I2C总线上读取一个字节
 * @param[in]  pstPara   IIC参数结构体指针，包含GPIO口等信息
 * @param[in]  Ack       是否发送应答：Ack=1 发送ACK（继续读取）；Ack=0 发送NACK（结束读取）
 * @retval     u8        返回读取到的字节
 */
u8 iic_read_byte(IICPara_ST *pstPara, u8 Ack)
{
    u8 i = 8;                                                       /* 8位计数器 */
    u8 ReceiveByte = 0;                                             /* 存储接收到的字节数据 */
                    
    _iic_sda_h(pstPara);                                            /* 释放SDA线，输入模式 */
    pstPara->iic_delay(pstPara);                                    /* 延时，确保总线稳定 */
                    
    while (i--)                                                     /* 循环接收8位数据（MSB先） */
    {                   
        _iic_scl_l(pstPara);                                        /* 拉低SCL，准备读取数据位 */
        pstPara->iic_delay(pstPara);                                /* 延时以满足I2C时序要求 */
                    
        ReceiveByte <<= 1;                                          /* 左移一位，为接收新bit腾出低位 */
                    
        _iic_scl_h(pstPara);                                        /* 拉高SCL，准备读取SDA上的数据 */
        pstPara->iic_delay(pstPara);                                /* 等待从设备输出稳定 */
                    
        if (_iic_sda_read(pstPara))                                 /* 读取SDA线电平 */
            ReceiveByte |= 0x01;                                    /* 设置最低位为1 */
    }                   
                    
    _iic_scl_l(pstPara);                                            /* 拉低SCL，准备进入应答阶段 */
    pstPara->iic_delay(pstPara);                                    /* 延时保持时序稳定 */
                    
    iic_ack(pstPara, Ack);                                          /* 发送应答信号 */
                    
    return ReceiveByte;                                             /* 返回读取到的字节 */
}

/**
 * @brief      获取IIC默认参数配置
 * @param[out] pstInit   IIC初始化结构体指针，用于存储默认配置
 * @retval     void
 * @note       本函数为用户提供一组默认IIC配置参数
 *             方便快速初始化，用户可在此基础上进行定制化修改。
 */
void iic_get_default(IICInit_ST *pstInit)
{
    pstInit->pstGTD_SCL    = GPIOC;                                 /* 默认SCL引脚端口：GPIOC */
    pstInit->u32Pin_SCL    = GPIO_PIN_9;                            /* 默认SCL引脚：PC9 */
    pstInit->pstGTD_SDA    = GPIOC;                                 /* 默认SDA引脚端口：GPIOC */
    pstInit->u32Pin_SDA    = GPIO_PIN_8;                            /* 默认SDA引脚：PC8 */
    pstInit->pstGTD_WP     = NULL;                                  /* 写保护引脚：不使用 */
    pstInit->u32Pin_WP     = 0;                                     /* 写保护引脚号：无 */
    pstInit->iic_delay     = _iic_delay_us;                         /* 时序延时函数：微秒延时 */
    pstInit->u16DelayUs    = 2;                                     /* 默认延时2微秒 */
    pstInit->u8AddrRD      = 129;                                   /* 默认IIC读地址 */
    pstInit->u8AddrWR      = 128;                                   /* 默认IIC写地址 */
    pstInit->pu8SysErrFlag = NULL;                                  /* 不启用系统错误标志 */
    pstInit->pstCnt        = NULL;                                  /* 不启用错误统计功能 */
    pstInit->u16ReadTime   = 100;                                   /* 默认读取周期 */
    pstInit->u8ErrMax      = 10;                                    /* 最大连续错误次数 */
}

/**
 * @brief      IIC器件初始化模板
 * @param[out] pstIIC   IIC管理结构体指针
 * @retval     void
 * @note       获取一组默认IIC参数，并初始化目标IIC管理结构体。
 */
void device_iic_init_template(IICManager_ST *pstIIC)
{
    IICInit_ST stIICInit = {0};                                      /* 初始化配置结构体 */
                            
//    iic_get_default(&stIICInit);                                   /* 获取默认配置 */
                            
//    stIICInit.pstGTD_SCL     = GPIOC;                              /* SCL端口 */
//    stIICInit.u32Pin_SCL     = GPIO_PIN_9;                         /* SCL引脚 */
                            
//    stIICInit.pstGTD_SDA     = GPIOC;                              /* SDA端口 */
//    stIICInit.u32Pin_SDA     = GPIO_PIN_8;                         /* SDA引脚 */
                            
//    stIICInit.pstGTD_WP      = NULL;                               /* 不使用WP */
//    stIICInit.u32Pin_WP      = 0;                                  /* WP引脚号 */
                            
//    stIICInit.u8AddrRD       = 129;                                /* 读地址 */
//    stIICInit.u8AddrWR       = 128;                                /* 写地址 */
//    stIICInit.u16DelayUs     = 2;                                  /* 时序延时 */
//    stIICInit.iic_delay      = _iic_delay_us;                      /* 微秒延时 */
                            
//    stIICInit.pu8SysErrFlag  = NULL;                               /* 系统错误标志 */
//    stIICInit.pstCnt         = NULL;                               /* 错误统计结构体 */
//    stIICInit.u8ErrMax       = 10;                                 /* 连续最大错误次数 */
                            
    iic_init(pstIIC, &stIICInit);                                    /* 初始化IIC管理结构体 */
}                           


/**
 * @brief      IIC参数初始化函数
 * @param[out] pstIIC      IIC管理结构体指针，用于存储初始化后的参数和状态
 * @param[in]  pstIICInit  IIC初始化配置结构体指针，包含用户设置的GPIO和延时参数等
 * @retval     void
 */
void iic_init(IICManager_ST *pstIIC, IICInit_ST *pstIICInit)
{
    IICPara_ST *pstPara = &pstIIC->stPara;                                           /* 便于赋值 */

    /* GPIO初始化部分 */
    iic_scl_init(&pstPara->stSCL, pstIICInit->pstGTD_SCL, pstIICInit->u32Pin_SCL);   /* 初始化SCL引脚 */
    iic_sda_init(&pstPara->stSDA, pstIICInit->pstGTD_SDA, pstIICInit->u32Pin_SDA);   /* 初始化SDA引脚 */

    if (NULL != pstIICInit->pstGTD_WP)
        iic_wp_init(&pstPara->stWP, pstIICInit->pstGTD_WP, pstIICInit->u32Pin_WP);   /* 如定义WP引脚则初始化 */

    /* 参数赋值部分 */
    pstPara->iic_delay      = pstIICInit->iic_delay;                                 /* 设置延时函数指针 */
    pstPara->u16DelayUs     = MY_MAX(pstIICInit->u16DelayUs, 2);                     /* 延时保证不低于2us */
    pstPara->u8AddrRD       = pstIICInit->u8AddrRD;                                  /* 设备读地址 */
    pstPara->u8AddrWR       = pstIICInit->u8AddrWR;                                  /* 设备写地址 */
    pstIIC->stData.pu8ErrFlag = pstIICInit->pu8SysErrFlag;                           /* 错误标志位指针 */
    pstPara->u16ReadTime    = pstIICInit->u16ReadTime;                               /* 读取周期 */
    pstPara->u8ErrMax       = pstIICInit->u8ErrMax;                                  /* 最大错误次数 */

    pstIIC->pstCnt = pstIICInit->pstCnt;                                             /* 错误统计结构体指针 */

    if (NULL != pstIIC->pstCnt)
        pstIIC->pstCnt->u8CircleClearFlag = 1;                                       /* 首次初始化清除统计 */

    iic_stop(pstPara);                                                               /* 发送STOP信号，释放总线 */
}


/**
 * @brief IIC读取周期计时器任务（周期调用，用于记录读取间隔时间）
 * @param pstData 指向 IIC 数据结构体（IICData_ST）的指针
 * @retval 无
 *
 * @note 本函数通常在定时器中断或周期任务中调用，用于记录两次IIC读取之间的时间间隔。
 *       它对 u16ReadTime 执行自增操作，最多不超过 0xFFFF。
 */
void iic_tim_task(IICData_ST *pstData)
{
    // 如果当前读取时间未达最大值0xFFFF，则自增
    // 否则保持在最大值（防止溢出）
    pstData->u16ReadTime = pstData->u16ReadTime < 0xFFFF ? pstData->u16ReadTime + 1 : 0xFFFF;
}

/**
 * @brief      IIC错误统计函数（支持滑动窗口误码率统计）
 * @param[in,out] pstCnt 指向错误计数结构体的指针（ErrCnt_ST）
 * @param[in]     Err    本次通信是否出错，非0表示出错，0表示正常
 * @retval      void
 */
void err_count(ErrCnt_ST *pstCnt, u8 Err)
{
    Err = !(!Err);                                                              /* 标准化为0或1 */
                
    u8 u8Index = (pstCnt->u8Idx) >> 3;                                          /* 计算字节索引 */
    u8 u8Bit   = (pstCnt->u8Idx) & 0x07;                                        /* 计算bit位索引 */
                
    pstCnt->u32Cnt++;                                                           /* 总通信次数+1 */
    pstCnt->u32ErrCnt += Err;                                                   /* 错误次数累加 */
                
    pstCnt->u8RateC += Err;                                                     /* 当前窗口错误数+1 */
    pstCnt->u8RateC -= (pstCnt->u8Buf[u8Index] >> u8Bit) & 0x01;                /* 减去原bit值 */
                
    pstCnt->u8Buf[u8Index] &= ~BIT(u8Bit);                                      /* 清除该bit */
    pstCnt->u8Buf[u8Index] |= Err << u8Bit;                                     /* 写入当前bit */
                
    pstCnt->u8Idx = (pstCnt->u8Idx + 1 < IIC_CNT_CYSLE) ?               
                    (pstCnt->u8Idx + 1) : 0;                                    /* 滑动窗口循环 */
                
    if (pstCnt->u8Top < IIC_CNT_CYSLE)              
    {               
        pstCnt->u8Top++;                                                        /* 窗口未满，仅累加 */
        return;             
    }               
                
    pstCnt->u8Top = IIC_CNT_CYSLE;                                              /* 窗口上限 */
                
    pstCnt->u8RateT   = IIC_CNT_CYSLE * pstCnt->u32ErrCnt / pstCnt->u32Cnt;     /* 总体误码率 */
    pstCnt->u8RateMax = MY_MAX(pstCnt->u8RateC, pstCnt->u8RateMax);             /* 最大窗口误码率 */
    pstCnt->u8RateMin = MY_MIN(pstCnt->u8RateC, pstCnt->u8RateMin);             /* 最小窗口误码率 */
}


/**
 * @brief      IIC通信错误检测与统计
 * @param[in,out] pstIIC      IIC管理结构体指针
 * @param[in]     u8ErrorFlag 错误标志位，每一位表示不同的错误类型
 *                            - BIT(E_I2CErr_Crc)：CRC校验错误
 *                            - BIT(E_I2CErr_Ack)：ACK丢失
 * @retval      void
 */
void iic_error_check(IICManager_ST *pstIIC, u8 u8ErrorFlag)
{
    IICPara_ST *pstPara = &pstIIC->stPara;                                      /* IIC配置信息 */
    IICData_ST *pstData = &pstIIC->stData;                                      /* IIC运行状态 */
    IICCnt_ST  *pstCnt  = pstIIC->pstCnt;                                       /* 错误统计结构体 */
    u8 i;           
            
    if (u8ErrorFlag)                                                            /* 有错误发生 */
    {           
        if (pstData->u8ErrCnt < pstPara->u8ErrMax)          
            pstData->u8ErrCnt++;                                                /* 错误次数递增 */
        else            
            pstData->u8ErrFlag = 1;                                             /* 超阈值，标记异常 */
    }           
    else            
    {           
        pstData->u8ErrFlag = 0;                                                 /* 清除异常标志 */
        if (pstData->u8ErrCnt)          
            pstData->u8ErrCnt = 0;                                              /* 累积错误清零 */
    }           
            
    if (NULL != pstData->pu8ErrFlag)            
        *pstData->pu8ErrFlag = pstData->u8ErrFlag;                              /* 更新系统错误标志 */
            
    if (NULL == pstCnt)                                                         /* 未启用统计结构体则退出 */
        return;         
            
    if (pstCnt->u8CircleClearFlag)          
    {           
        pstCnt->u8CircleClearFlag = 0;                                          /* 清除触发标志 */
        memset(pstCnt->stErrCnt, 0, sizeof(pstCnt->stErrCnt));                  /* 清空错误统计 */
        for (i = 0; i < E_I2CErr_Max; i++)          
            pstCnt->stErrCnt[i].u8RateMin = 99;                                 /* 初始化最小值 */
    }

    err_count(&pstCnt->stErrCnt[E_I2CErr_Crc], u8ErrorFlag & BIT(E_I2CErr_Crc));                 /* 统计CRC错误 */
    err_count(&pstCnt->stErrCnt[E_I2CErr_Ack], u8ErrorFlag & BIT(E_I2CErr_Ack));                 /* 统计ACK错误 */
    err_count(&pstCnt->stErrCnt[E_I2CErr_Total], u8ErrorFlag & (BIT(E_I2CErr_Crc) | BIT(E_I2CErr_Ack))); /* 统计总错误 */
}

 /***************************************************************************************************/

