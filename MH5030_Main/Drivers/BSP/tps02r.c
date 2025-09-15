#include "tps02r.h"
// 定义全局IIC管理结构体和初始化配置结构体
IICManager_ST               tps02r_iicmanger;

void tps02r_iic_init(IICManager_ST *pstIIC)
{
    IICInit_ST stIICInit = {0};                                                /* 初始化配置结构体 */
                             
    stIICInit.pstGTD_SCL     = TPS02RAH_SCL_GPIO_Port;                         /* SCL端口 */
    stIICInit.u32Pin_SCL     = TPS02RAH_SCL_Pin;                               /* SCL引脚 */
                            
    stIICInit.pstGTD_SDA     = TPS02RAH_SDA_GPIO_Port;                         /* SDA端口 */
    stIICInit.u32Pin_SDA     = TPS02RAH_SDA_Pin;                               /* SDA引脚 */
                            
    stIICInit.pstGTD_WP      = NULL;                                           /* 不使用WP */
    stIICInit.u32Pin_WP      = 0;                                              /* WP引脚号 */
                            
    stIICInit.u8AddrRD       = TPS02R_I2C_ADR_R;                               /* 读地址 */
    stIICInit.u8AddrWR       = TPS02R_I2C_ADR_W;                               /* 写地址 */
    stIICInit.u16DelayUs     = 2;                                              /* 时序延时 */
    stIICInit.iic_delay      = _iic_delay_us;                                  /* 微秒延时 */
                            
    iic_init(pstIIC, &stIICInit);                                              /* IIC初始化 */      
}

/**
 * @brief      将读取到的寄存器值转为温度值
 * @param      p_data  读取到的寄存器数据（3字节）
 * @retval     温度值（单位：℃）
 */
static float __tps02r_reg_to_temp(uint8_t *p_data)
{
    uint32_t temp = 0;

    /* 合并寄存器数据为 24 位值 */
    temp |= p_data[0] << 16;
    temp |= p_data[1] << 8;
    temp |= p_data[2];

    /* 当前测量温度为负温度值 */
    if (temp >= 8388608)
    {
        return (-(float)((16777216 - temp) / (float)(0x1 << 13)));              /* 负温度处理 */
    }
    else
    {
        return ((float)(temp / (float)(0x1 << 13)));                            /* 正温度处理 */
    }
}

/**
 * @brief      获取指定寄存器通道的温度值
 * 
 * @param      chan    温度通道，0 或 1（分别表示通道 0 和通道 1）
 * @param      p_data   存储温度数据的指针，读取的温度值将保存在此指针指向的内存中
 * 
 * @retval     TPS02R_FUN_OK     成功获取温度值
 * @retval     TPS02R_VALUE_ERROR 参数错误，如设备指针或数据指针为 NULL，通道无效
 * @retval     TPS02R_FUN_ERROR  读取数据失败
 */
int tps02r_get_temp(int8_t chan, float *p_data)
{
    u8 error = 0;                                                               /* 存储返回状态 */
    float temp_buff[2] = {0};                                                   /* 存储通道温度值的缓冲区 */
    uint8_t rx_buff[6] = {0};                                                   /* 存储读取的数据缓冲区 */

    if (p_data == NULL) 
    {
        return TPS02R_VALUE_ERROR;                                              /* 参数检查，设备指针或数据指针为空 */
    }

    if (chan < 0 || chan > 1) 
    {
        return TPS02R_VALUE_ERROR;                                              /* 参数检查，通道号无效 */
    }
    
    /* 调用读取数据函数，返回值判断读取是否成功 */
    iic_start(&tps02r_iicmanger.stPara);                                        /* 起始 */    
    error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_I2C_ADR_W);          /* 设备写地址 */
    error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_TEMP_ADDR);          /* 发送命令 */
    iic_start(&tps02r_iicmanger.stPara);                                        /* 起始 */
    error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_I2C_ADR_R);          /* 设备写地址 */

    if (error != 0)                                                             /* 返回值判断读取是否成功 */
    {
        return TPS02R_FUN_ERROR;                                                /* 数据读取失败，返回错误码 */
    }

    for(int i=0; i < TPS02R_TEMP_LEN;i++)                                       /* 读取温度寄存器数据，共6个字节 */
    {
        if(i < TPS02R_TEMP_LEN - 1)
        {
            rx_buff[i] = iic_read_byte(&tps02r_iicmanger.stPara, 1);            /* 读高字节，主机发ACK */
        }
        else
        {
            rx_buff[i] = iic_read_byte(&tps02r_iicmanger.stPara, 0);            /* 读取结束主机发NACK */
            iic_stop(&tps02r_iicmanger.stPara);                                 /* 停止信号 */
        }
    }
    
    if (chan == 0) 
    {
        temp_buff[0] = __tps02r_reg_to_temp(&rx_buff[0]);
        *p_data = temp_buff[0];                                                 /* 将通道 0 的温度值存储到数据指针 */
    }
    else if (chan == 1) 
    {
        temp_buff[1] = __tps02r_reg_to_temp(&rx_buff[3]);
        *p_data = temp_buff[1];                                                 /* 将通道 1 的温度值存储到数据指针 */
    }

    return TPS02R_FUN_OK;                                                       /* 成功获取温度值，返回成功状态 */
}



/**
 * @brief      设置通道的上限温度值
 * 
 * @param      chan    温度通道，0 或 1（分别表示通道 0 和通道 1）
 * @param      temp    要设置的上限温度值（单位：摄氏度）
 * 
 * @retval     TPS02R_FUN_OK     成功设置上限温度值
 * @retval     TPS02R_VALUE_ERROR 参数错误，如设备指针为空、通道无效等
 * @retval     TPS02R_FUN_ERROR  写入数据失败
 */
int tps02r_set_high(int8_t chan, float temp)
{
    int error = 0;                                                              /* 存储返回状态 */
    uint32_t temp_code = 0;                                                     /* 存储温度编码 */
    uint8_t temp_buff[6] = {0};                                                 /* 存储温度数据缓冲区 */
    int32_t temp1 = 0;                                                          /* 存储转换后的温度值 */
    
    temp1 = (int64_t)((temp) * (0x1 << 13));                                    /* 将温度转换为编码格式 */

    /* 读取当前的温度设置 */
    iic_start(&tps02r_iicmanger.stPara);                                        /* 起始 */    
    error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_I2C_ADR_W);          /* 设备写地址 */
    error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_THIG_ADDR);          /* 发送命令 */
    iic_start(&tps02r_iicmanger.stPara);                                        /* 起始 */
    error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_I2C_ADR_R);          /* 设备写地址 */

    if (error != 0)                                                             /* 返回值判断读取是否成功 */
    {
        return TPS02R_FUN_ERROR;                                                /* 数据读取失败，返回错误码 */
    }

    for(int i=0; i<TPS02R_THIG_LEN;i++)                                         /* 发送温度寄存器数据，共6个字节 */
    {
        if(i < TPS02R_THIG_LEN - 1)
        {
            temp_buff[i] = iic_read_byte(&tps02r_iicmanger.stPara, 1);          /* 读高字节，主机发ACK */
        }
        else
        {
            temp_buff[i] = iic_read_byte(&tps02r_iicmanger.stPara, 0);          /* 读取结束主机发NACK */
            iic_stop(&tps02r_iicmanger.stPara);                                 /* 停止信号 */
        }
    }

    /* 处理温度值为负数的情况 */
    if (temp1 < 0) 
    {
        temp1 = -temp1;
        temp_code = 16777216 - temp1;                                           /* 计算负温度的编码值 */
    } 
    else 
    {
        temp_code = temp1;                                                      /* 正温度直接使用转换后的值 */
    }

    /* 根据通道选择对应的缓冲区并设置上限值 */
    if (chan == 0) 
    {
        temp_buff[0] = (temp_code >> 16) & 0xff;
        temp_buff[1] = (temp_code >> 8) & 0xff;
        temp_buff[2] = (temp_code >> 0) & 0xff;
    } 
    else if (chan == 1) 
    {
        temp_buff[3] = (temp_code >> 16) & 0xff;
        temp_buff[4] = (temp_code >> 8) & 0xff;
        temp_buff[5] = (temp_code >> 0) & 0xff;
    } 
    else 
    {
        return TPS02R_VALUE_ERROR;                                               /* 参数检查，通道无效 */
    }

    /* 将设置的上限温度值写入设备 */
    iic_start(&tps02r_iicmanger.stPara);                                         /* 起始 */    
    error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_I2C_ADR_W);           /* 设备写地址 */
    error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_THIG_ADDR);           /* 发送命令 */

    if (error != 0)                                                              /* 返回值判断读取是否成功 */
    {
        return TPS02R_FUN_ERROR;                                                 /* 数据读取失败，返回错误码 */
    }

    for(int i=0; i < TPS02R_THIG_LEN;i++)                                        /* 读取温度寄存器数据，共6个字节 */
    {
        if(i < TPS02R_THIG_LEN - 1)
        {
            iic_send_byte(&tps02r_iicmanger.stPara, temp_buff[i]);               /* 将设置的上限温度值写入寄存器  */
        }
        else
        {
            iic_send_byte(&tps02r_iicmanger.stPara, temp_buff[i]);               
            iic_stop(&tps02r_iicmanger.stPara);                                  /* 停止信号 */
        }
    }

    return TPS02R_FUN_OK;                                                        /* 成功设置上限温度值，返回成功状态 */
}


/**
 * @brief      设置通道的下限温度值
 * 
 * @param      chan    温度通道，0 或 1（分别表示通道 0 和通道 1）
 * @param      temp    要设置的上限温度值（单位：摄氏度）
 * 
 * @retval     TPS02R_FUN_OK     成功设置上限温度值
 * @retval     TPS02R_VALUE_ERROR 参数错误，如设备指针为空、通道无效等
 * @retval     TPS02R_FUN_ERROR  写入数据失败
 */

int tps02r_set_low(int8_t chan, float temp)
{
    int error = 0;                                                              /* 存储返回状态 */
    uint32_t temp_code = 0;                                                     /* 存储温度编码 */
    uint8_t temp_buff[6] = {0};                                                 /* 存储温度数据缓冲区 */
    int32_t temp1 = 0;                                                          /* 存储转换后的温度值 */
        
    temp1 = (int64_t)((temp) * (0x1 << 13));                                    /* 将温度转换为编码格式 */
        
    /* 读取当前的温度设置 */ 
    iic_start(&tps02r_iicmanger.stPara);                                        /* 起始 */    
    error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_I2C_ADR_W);          /* 设备写地址 */
    error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_TLOW_ADDR);          /* 发送命令 */
    iic_start(&tps02r_iicmanger.stPara);                                        /* 起始 */
    error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_I2C_ADR_R);          /* 设备写地址 */
    
    if (error != 0)                                                             /* 返回值判断读取是否成功 */
    {   
        return TPS02R_FUN_ERROR;                                                /* 数据读取失败，返回错误码 */
    }   
    
    for(int i=0; i < TPS02R_TLOW_LEN;i++)                                       /* 读取温度寄存器数据，共6个字节 */
    {   
        if(i < TPS02R_TLOW_LEN - 1) 
        {   
            temp_buff[i] = iic_read_byte(&tps02r_iicmanger.stPara, 1);          /* 读高字节，主机发ACK */
        }   
        else    
        {   
            temp_buff[i] = iic_read_byte(&tps02r_iicmanger.stPara, 0);          /* 读取结束主机发NACK */
            iic_stop(&tps02r_iicmanger.stPara);                                 /* 停止信号 */
        }   
    }   
    
    /* 处理温度值为负数的情况 */   
    if (temp1 < 0)  
    {   
        temp1 = -temp1; 
        temp_code = 16777216 - temp1;                                           /* 计算负温度的编码值 */
    } 
    else 
    {
        temp_code = temp1;                                                      /* 正温度直接使用转换后的值 */
    }

    /* 根据通道选择对应的缓冲区并设置上限值 */
    if (chan == 0) 
    {
        temp_buff[0] = (temp_code >> 16) & 0xff;
        temp_buff[1] = (temp_code >> 8) & 0xff;
        temp_buff[2] = (temp_code >> 0) & 0xff;
    } 
    else if (chan == 1) 
    {
        temp_buff[3] = (temp_code >> 16) & 0xff;
        temp_buff[4] = (temp_code >> 8) & 0xff;
        temp_buff[5] = (temp_code >> 0) & 0xff;
    } 
    else 
    {
        return TPS02R_VALUE_ERROR;                                               /* 参数检查，通道无效 */
    }

    /* 将设置的下限温度值写入设备 */
    iic_start(&tps02r_iicmanger.stPara);                                         /* 起始 */    
    error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_I2C_ADR_W);           /* 设备写地址 */
    error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_TLOW_ADDR);           /* 发送命令 */

    if (error != 0)                                                              /* 返回值判断读取是否成功 */
    {
        return TPS02R_FUN_ERROR;                                                 /* 数据读取失败，返回错误码 */
    }

    for(int i=0; i < TPS02R_TLOW_LEN;i++)                                        /* 发送温度寄存器数据，共6个字节 */
    {
        if(i < TPS02R_TLOW_LEN - 1)
        {
            iic_send_byte(&tps02r_iicmanger.stPara, temp_buff[i]);               /* 将设置的下限温度值写入寄存器  */
        }
        else
        {
            iic_send_byte(&tps02r_iicmanger.stPara, temp_buff[i]);               
            iic_stop(&tps02r_iicmanger.stPara);                                  /* 停止信号 */
        }
    }

    return TPS02R_FUN_OK;
}

/**
 * @brief      对配置通道的采样速率进行配置
 * @param      chan_en  配置选择：0x00/0x01/0x02/0x03
 * @param      rate     采样速率
 
 * @retval     TPS02R_FUN_OK     配置成功
 * @retval     TPS02R_FUN_ERROR  读写失败
 */
int tps02r_set_chan_sampling_rate(int8_t chan_en, uint32_t rate)
{
    uint8_t cfg_buff[2] = {0};                                                                  /* 临时配置缓存 */
    int error = 0;
    /* 读取当前的配置寄存器的设置 */
    iic_start(&tps02r_iicmanger.stPara);                                                        /* 起始 */    
    error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_I2C_ADR_W);                          /* 设备写地址 */
    error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_CFG_ADDR);                           /* 发送命令 */
    iic_start(&tps02r_iicmanger.stPara);                                                        /* 起始 */
    error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_I2C_ADR_R);                          /* 设备写地址 */

    if (error != 0)                                                                             /* 返回值判断读取是否成功 */
    {
        return TPS02R_FUN_ERROR;                                                                /* 数据读取失败，返回错误码 */
    }

    for(int i=0; i < TPS02R_CFG_LEN;i++)                                                        /* 读取配置寄存器数据，共2个字节 */
    {
        if(i < TPS02R_CFG_LEN - 1)
        {
            cfg_buff[i] = iic_read_byte(&tps02r_iicmanger.stPara, 1);                           /* 读高字节，主机发ACK */
        }
        else
        {
            cfg_buff[i] = iic_read_byte(&tps02r_iicmanger.stPara, 0);                           /* 读取结束主机发NACK */
            iic_stop(&tps02r_iicmanger.stPara);                                                 /* 停止信号 */
        }
    }

    if (chan_en == 0x00 || chan_en == 0x02 || chan_en == 0x03)
    {
        cfg_buff[0] &= (~TPS02R_CFG_RATE_MASK);                                                 /* 清除原采样速率 */
        cfg_buff[0] |= (rate << TPS02R_CFG_RATE_SHIFT) & TPS02R_CFG_RATE_MASK;                  /* 设置新采样速率 */
    }
    else if (chan_en == 0x01)
    {
        cfg_buff[1] &= (~TPS02R_CFG_RATE_MASK);                                                 /* 清除原采样速率 */
        cfg_buff[1] |= (rate << TPS02R_CFG_RATE_SHIFT) & TPS02R_CFG_RATE_MASK;                  /* 设置新采样速率 */
    }

     /* 将设置的采样率写入设备 */
    iic_start(&tps02r_iicmanger.stPara);                                                        /* 起始 */    
    error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_I2C_ADR_W);                          /* 设备写地址 */
    error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_CFG_ADDR);                           /* 发送命令 */

    if (error != 0)                                                                             /* 返回值判断读取是否成功 */
    {
        return TPS02R_FUN_ERROR;                                                                /* 数据读取失败，返回错误码 */
    }

    for(int i=0; i < TPS02R_CFG_LEN;i++)                                                        /* 发送温度寄存器数据，共6个字节 */
    {
        if(i < TPS02R_CFG_LEN - 1)
        {
            iic_send_byte(&tps02r_iicmanger.stPara, cfg_buff[i]);                               /* 将设置的采样率写入寄存器 */
        }
        else
        {
            iic_send_byte(&tps02r_iicmanger.stPara, cfg_buff[i]);               
            iic_stop(&tps02r_iicmanger.stPara);                                                 /* 停止信号 */
        }
    }
    
    return TPS02R_FUN_OK;                                                                       /* 配置成功 */
}

/**
 * @brief      将 tps02r 模块的配置初始化为默认值
 * @retval     TPS02R_VALUE_ERROR  入参为空
 * @retval     TPS02R_FUN_ERROR    读写寄存器失败
 * @retval     TPS02R_FUN_OK       初始化成功
 */
int tps02r_cfg_init(void)
{
    int     error = 0;                                                                         /* 返回码 */
    uint8_t cfg_buff[2]   = {0};                                                               /* 配置寄存器缓存 */
    uint8_t temph_buff[6] = {0};                                                               /* 上限温度寄存器缓存 */
    uint8_t templ_buff[6] = {0};                                                               /* 下限温度寄存器缓存 */

    /* 默认温度上限值为 1023.999878℃（两个通道） */
    temph_buff[0] = 0x7f;
    temph_buff[1] = 0xff;
    temph_buff[2] = 0xff;

    temph_buff[3] = 0x7f;
    temph_buff[4] = 0xff;
    temph_buff[5] = 0xff;


    /* 将设置的上限温度值写入设备 */
    iic_start(&tps02r_iicmanger.stPara);                                                       /* 起始 */    
    error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_I2C_ADR_W);                         /* 设备写地址 */
    error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_THIG_ADDR);                         /* 发送命令 */

    if (error != 0)                                                                            /* 返回值判断读取是否成功 */
    {
        return TPS02R_FUN_ERROR;                                                               /* 数据读取失败，返回错误码 */
    }

    for(int i=0; i < TPS02R_THIG_LEN;i++)                                                      /* 发送給温度寄存器数据，共6个字节 */
    {
        if(i < TPS02R_THIG_LEN - 1)
        {
            iic_send_byte(&tps02r_iicmanger.stPara, temph_buff[i]);                            /* 将设置的上限限温度值写入寄存器  */
        }
        else
        {
            iic_send_byte(&tps02r_iicmanger.stPara, temph_buff[i]);               
            iic_stop(&tps02r_iicmanger.stPara);                                                /* 停止信号 */
        }
    }

    /* 默认温度下限值为 -0.000122℃（两个通道） */
    templ_buff[0] = 0xff;
    templ_buff[1] = 0xff;
    templ_buff[2] = 0xff;

    templ_buff[3] = 0xff;
    templ_buff[4] = 0xff;
    templ_buff[5] = 0xff;

    /* 将设置的下限温度值写入设备 */
    iic_start(&tps02r_iicmanger.stPara);                                                       /* 起始 */    
    error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_I2C_ADR_W);                         /* 设备写地址 */
    error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_TLOW_ADDR);                         /* 发送命令 */

    if (error != 0)                                                                            /* 返回值判断读取是否成功 */
    {
        return TPS02R_FUN_ERROR;                                                               /* 数据读取失败，返回错误码 */
    }

    for(int i=0; i < TPS02R_TLOW_LEN;i++)                                                      /* 发送温度寄存器数据，共6个字节 */
    {
        if(i < TPS02R_TLOW_LEN - 1)
        {
            iic_send_byte(&tps02r_iicmanger.stPara, templ_buff[i]);                             /* 将设置的下限温度值写入寄存器  */
        }
        else
        {
            iic_send_byte(&tps02r_iicmanger.stPara, templ_buff[i]);               
            iic_stop(&tps02r_iicmanger.stPara);                                                 /* 停止信号 */
        }
    }

    /* 读取当前的配置寄存器的设置 */
    iic_start(&tps02r_iicmanger.stPara);                                                        /* 起始 */    
    error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_I2C_ADR_W);                          /* 设备写地址 */
    error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_CFG_ADDR);                           /* 发送命令 */
    iic_start(&tps02r_iicmanger.stPara);                                                        /* 起始 */
    error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_I2C_ADR_R);                          /* 设备写地址 */

    if (error != 0)                                                                             /* 返回值判断读取是否成功 */
    {
        return TPS02R_FUN_ERROR;                                                                /* 数据读取失败，返回错误码 */
    }

    for(int i=0; i < TPS02R_CFG_LEN;i++)                                                        /* 读取配置寄存器数据，共2个字节 */
    {
        if(i < TPS02R_CFG_LEN - 1)
        {
            cfg_buff[i] = iic_read_byte(&tps02r_iicmanger.stPara, 1);                           /* 读高字节，主机发ACK */
        }
        else
        {
            cfg_buff[i] = iic_read_byte(&tps02r_iicmanger.stPara, 0);                           /* 读取结束主机发NACK */
            iic_stop(&tps02r_iicmanger.stPara);                                                 /* 停止信号 */
        }
    }

    /* 默认配置寄存器 BYTE1：EN=0, R0=0, F1=F0=1, POL=0, TM=0, SD=0（固定位保持） */
    cfg_buff[0] &= (~0x3E);
    cfg_buff[0] |= TPS02R_ALARM_MODE_2 & TPS02R_CFG_ALARM_MODE_MASK;
    cfg_buff[0] |= (0x03 << TPS02R_CFG_TRIGGER_WARNING_SHIFT) & TPS02R_CFG_TRIGGER_WARNING_MASK;
    cfg_buff[0] |= (TPS02R_SAMPLE_RATE_10 << TPS02R_CFG_RATE_SHIFT) & TPS02R_CFG_RATE_MASK;

    /* 默认配置寄存器 BYTE2：EN=1, R0=0, F1=F0=1, POL=0, TM=0, SD=0（固定位保持） */
    cfg_buff[1] &= (~0x3E);
    cfg_buff[1] |= TPS02R_ALARM_MODE_2 & TPS02R_CFG_ALARM_MODE_MASK;
    cfg_buff[1] |= (0x03 << TPS02R_CFG_TRIGGER_WARNING_SHIFT) & TPS02R_CFG_TRIGGER_WARNING_MASK;
    cfg_buff[1] |= (TPS02R_SAMPLE_RATE_10 << TPS02R_CFG_RATE_SHIFT) & TPS02R_CFG_RATE_MASK;
    cfg_buff[1] |= (0x01 << TPS02R_CFG_EN_SHIFT);

    /* 将配置写入设备 */
    iic_start(&tps02r_iicmanger.stPara);                                                        /* 起始 */    
    error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_I2C_ADR_W);                          /* 设备写地址 */
    error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_CFG_ADDR);                           /* 发送命令 */

    if (error != 0)                                                                             /* 返回值判断读取是否成功 */
    {
        return TPS02R_FUN_ERROR;                                                                /* 数据读取失败，返回错误码 */
    }

    for(int i=0; i < TPS02R_CFG_LEN;i++)                                                        /* 发送配置，共2个字节 */
    {
        if(i < TPS02R_CFG_LEN - 1)
        {
            iic_send_byte(&tps02r_iicmanger.stPara, cfg_buff[i]);                               /* 将配置参数写入寄存器 */
        }
        else
        {
            iic_send_byte(&tps02r_iicmanger.stPara, cfg_buff[i]);               
            iic_stop(&tps02r_iicmanger.stPara);                                                 /* 停止信号 */
        }
    }

    return TPS02R_FUN_OK;                                                                       /* 初始化成功 */
}


/**
 * @brief      获取配置寄存器通道数据
 * @param      chan    通道：0 或 1
 * @param      p_data  输出：对应通道的配置字节
 
 * @retval     TPS02R_VALUE_ERROR  入参非法
 * @retval     TPS02R_FUN_ERROR    读取寄存器失败
 * @retval     TPS02R_FUN_OK       成功
 */
int tps02r_get_cfg_value(int8_t chan, uint8_t *p_data)
{
    /* 入参判空 */
    if (p_data == NULL)
    {
        return TPS02R_VALUE_ERROR;
    }

    int error = 0;
    uint8_t cfg_buff[2] = {0};

    /* 读取配置寄存器 */
   iic_start(&tps02r_iicmanger.stPara);                                                        /* 起始 */    
   error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_I2C_ADR_W);                          /* 设备写地址 */
   error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_CFG_ADDR);                           /* 发送命令 */
   iic_start(&tps02r_iicmanger.stPara);                                                        /* 起始 */
   error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_I2C_ADR_R);                          /* 设备写地址 */

   if (error != 0)                                                                             /* 返回值判断读取是否成功 */
   {
       return TPS02R_FUN_ERROR;                                                                /* 数据读取失败，返回错误码 */
   }

   for(int i=0; i < TPS02R_CFG_LEN;i++)                                                        /* 读取配置寄存器数据，共2个字节 */
   {
       if(i < TPS02R_CFG_LEN - 1)
       {
           cfg_buff[i] = iic_read_byte(&tps02r_iicmanger.stPara, 1);                           /* 读高字节，主机发ACK */
       }
       else
       {
           cfg_buff[i] = iic_read_byte(&tps02r_iicmanger.stPara, 0);                           /* 读取结束主机发NACK */
           iic_stop(&tps02r_iicmanger.stPara);                                                 /* 停止信号 */
       }
   }

    /* 根据通道返回数据 */
    if (chan == 0)
    {
        *p_data = cfg_buff[0];
    }
    else if (chan == 1)
    {
        *p_data = cfg_buff[1];
    }
    else
    {
        return TPS02R_VALUE_ERROR;
    }

    return TPS02R_FUN_OK;
}

/** end of file **/

