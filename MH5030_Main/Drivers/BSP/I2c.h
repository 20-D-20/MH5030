#ifndef __I2C_H
#define __I2C_H

#include "delay.h"


#define MY_MIN(a, b)      ((a) < (b) ? (a) : (b))         /* 最小值宏 */
#define MY_MAX(a, b)      ((a) < (b) ? (b) : (a))         /* 最大值宏 */
#define IIC_ADD_WR        0x16                            /* 定义 IIC 器件写地址 */
#define IIC_ADD_RD        0x17                            /* 定义 IIC 器件读地址 */
#define BIT(n)            (1U << (n))                     /* 单比特掩码宏 */
#define ACK_WAIT_TIMES    100                             /* 定义 ACK 等待超时次数 */
#define IIC_CNT_CYSLE     100                             /* 统计窗口大小为 100 次（移动窗口） */

/**
 * @brief      IIC 错误类型枚举
 */
typedef enum MY_IIC_ERROR_ENUM                            /* IIC 错误类型 */
{
    E_I2CErr_Total = 0,                                   /* 0 误码率 */
    E_I2CErr_Crc,                                         /* 1 CRC 错误率 */
    E_I2CErr_Ack,                                         /* 2 ACK 丢失率 */
    E_I2CErr_Max,                                         /* 3 */
} IICErrFlag_E;

/**
 * @brief  通用 GPIO 配置结构体
 */
typedef struct MY_GPIO_STRUCT
{
    GPIO_TypeDef *pstGTD;                                /* GPIOX */
    u32           u32Pin;                                /* GPIO_Pin_X */
} MyGpio_ST;

/**
 * @brief  IIC 参数结构体，用于保存 IIC 总线配置
 */
typedef struct MY_IIC_PARA_STRUCT
{
    MyGpio_ST   stSCL;                                  /* 时钟引脚 */
    MyGpio_ST   stSDA;                                  /* 数据引脚 */
    MyGpio_ST   stWP;                                   /* 写保护引脚（可选） */

    void      (*iic_delay)(struct MY_IIC_PARA_STRUCT *pstIIC); /* 延时函数指针 */

    u16         u16DelayUs;                             /* 单位延时时间（us） */
    u8          u8AddrWR;                               /* IIC 器件写地址 */
    u8          u8AddrRD;                               /* IIC 器件读地址 */
                        
    u16         u16ReadTime;                            /* 读取周期计数 */
    u16         u16AveCntMax;                           /* 平均采样次数（预留） */
                        
    u8          u8ErrMax;                               /* 连续错误次数上限 */
    u8          u8Enable;                               /* 启用标志 */
    u8          u8Res[2];                               /* 保留位（对齐用） */
} IICPara_ST;

/**
 * @brief  IIC 数据缓存结构体，用于记录通信状态与缓存数据
 */
typedef struct MY_IIC_DATA_STRUCT
{
    u8  *pu8ErrFlag;                                    /* 系统错误标志指针 */
    u16  u16ReadTime;                                   /* 周期读取计时器 */
    u8   u8AckTimeOut;                                  /* ACK 错误标志 */
    u8   u8ErrFlag;                                     /* 通信异常标志 */
                                
    u8   u8ErrCnt;                                      /* 连续错误计数器 */
    u8   arrayforpec[6];                                /* 接收缓存 */
    u8   u8Res[1];                                      /* 保留位 */
} IICData_ST;

/**
 * @brief  IIC 错误统计结构体，用于记录通信错误与统计误码率
 */
typedef struct ERROR_COUNT_STRUCT
{
    u32  u32Cnt;                                        /* 总通信次数 */
    u32  u32ErrCnt;                                     /* 总错误次数 */
    u8   u8Buf[14];                                     /* 错误位缓存 */
    u8   u8Idx;                                         /* 当前写入索引 */
    u8   u8Top;                                         /* 当前累计计数 */
    u8   u8RateC;                                       /* 当前窗口内错误次数 */
    u8   u8RateT;                                       /* 总误码率 */
    u8   u8RateMin;                                     /* 最小误码率 */
    u8   u8RateMax;                                     /* 最大误码率 */
} ErrCnt_ST;

typedef struct MY_IIC_ERROR_COUNT_STRUCT
{
    ErrCnt_ST stErrCnt[E_I2CErr_Max];                   // 对每种错误分别统计
    u8 u8CircleClearFlag;                               // 清除统计标志
    u8 u8Res[3];                                        // 对齐
} IICCnt_ST;


/**
 * @brief  IIC 管理结构体，统一管理 IIC 参数与运行数据
 */
typedef struct MY_IIC_MANAGEMENT_STRUCT
{
    IICPara_ST  stPara;                                 /* IIC 参数（引脚、延时等） */
    IICData_ST  stData;                                 /* 当前通信状态数据 */
    IICCnt_ST  *pstCnt;                                 /* 指向统计结构体的指针（可为 NULL） */
} IICManager_ST;

/**
 * @brief  IIC 初始化参数结构体，配置 IIC 所需的初始信息
 */
typedef struct MY_IIC_INIT_STRUCT
{
    GPIO_TypeDef *pstGTD_SCL;                           /* SCL 端口 */
    u32           u32Pin_SCL;                           /* SCL 引脚 */
    GPIO_TypeDef *pstGTD_SDA;                           /* SDA 端口 */
    u32           u32Pin_SDA;                           /* SDA 引脚 */
    GPIO_TypeDef *pstGTD_WP;                            /* 写保护端口 */
    u32           u32Pin_WP;                            /* 写保护引脚 */

    void        (*iic_delay)(IICPara_ST *pstIIC);       /* 延时函数指针 */
    u16           u16DelayUs;                           /* 延时时间 */
    u8            u8AddrWR;                             /* 写地址 */
    u8            u8AddrRD;                             /* 读地址 */
                        
    u8           *pu8SysErrFlag;                        /* 外部错误标志指针 */
    IICCnt_ST    *pstCnt;                               /* 错误统计指针 */
                        
    u16           u16ReadTime;                          /* 读取周期 */
    u8            u8ErrMax;                             /* 最大允许错误次数 */
    u8            u8Res[1];                             /* 对齐 */
} IICInit_ST;

#define _pin_h(pstMyGPIO)    (pstMyGPIO).pstGTD->BSRR = (pstMyGPIO).u32Pin
#define _pin_l(pstMyGPIO)    (pstMyGPIO).pstGTD->BSRR = (pstMyGPIO).u32Pin<<16
#define _pin_read(pstMyGPIO) (((pstMyGPIO).pstGTD->IDR & (pstMyGPIO).u32Pin) ? 1 : 0)

#define _iic_scl_h(pstIICPara)      _pin_h((pstIICPara)->stSCL)
#define _iic_scl_l(pstIICPara)      _pin_l((pstIICPara)->stSCL)
#define _iic_sda_h(pstIICPara)      _pin_h((pstIICPara)->stSDA)
#define _iic_sda_l(pstIICPara)      _pin_l((pstIICPara)->stSDA)
#define _iic_sda_read(pstIICPara)   _pin_read((pstIICPara)->stSDA)
#define _iic_wp_h(pstIICPara)      _pin_h((pstIICPara)->stWP)
#define _iic_wp_l(pstIICPara)      _pin_l((pstIICPara)->stWP)

void iic_error_check(IICManager_ST *pstIIC, u8 u8ErrorFlag);

void iic_start(IICPara_ST *pstPara);
void iic_stop(IICPara_ST *pstPara);
u8 iic_send_byte(IICPara_ST *pstPara, u8 SendByte);
u8 iic_read_byte(IICPara_ST *pstPara, u8 Ack);
void _iic_delay_us(IICPara_ST* pstPara); //微秒级延时
void _iic_delay_cnt(IICPara_ST* pstPara);    //低于1微秒的延时，非精确

void iic_tim_task(IICData_ST *pstData);
void iic_get_default(IICInit_ST *pstPara);
void iic_init(IICManager_ST *pstIIC, IICInit_ST *pstIICInit);
void device_iic_init_template(IICManager_ST *pstIIC);

#endif


