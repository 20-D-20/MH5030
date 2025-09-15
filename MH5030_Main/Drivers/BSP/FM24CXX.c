#include "FM24CXX.h"

IICManager_ST   fm24cxx_iicmanager;

/**
 * @brief      FM24CXX器件初始化
 * @param[out] pstIIC   IIC管理结构体指针
 * @retval     void
 * @note       获取一组默认IIC参数，并初始化目标IIC管理结构体。
 */
void FM24CXX_iic_init(IICManager_ST *pstIIC)
{
    IICInit_ST stIICInit = {0};                                             /* 初始化配置结构体 */
                               
    stIICInit.pstGTD_SCL     = FM24CL16_SCL_GPIO_Port;                      /* SCL端口 */
    stIICInit.u32Pin_SCL     = FM24CL16_SCL_Pin;                            /* SCL引脚 */
                               
    stIICInit.pstGTD_SDA     = FM24CL16_SDA_GPIO_Port;                      /* SDA端口 */
    stIICInit.u32Pin_SDA     = FM24CL16_SDA_Pin;                            /* SDA引脚 */
                               
    stIICInit.pstGTD_WP      = FM24CL16_WP_GPIO_Port;                       /* 使用WP */
    stIICInit.u32Pin_WP      = FM24CL16_SCL_Pin;                            /* WP引脚号 */
                               
    stIICInit.u8AddrRD       = FRAM_READ;                                   /* 读地址 */
    stIICInit.u8AddrWR       = FRAM_WRITE;                                  /* 写地址 */
    stIICInit.u16DelayUs     = 2;                                           /* 时序延时 */
    stIICInit.iic_delay      = _iic_delay_us;                               /* 微秒延时 */
                               
    iic_init(pstIIC, &stIICInit);                                           /* 初始化IIC管理结构体 */
    EEPROM_WP(1);                                                           /* 开启写保护 */
}                           
/**
 * @brief  读取一个字节的数据
 * @param  ReadAddr 要读取的存储器地址
 * @retval 返回从指定地址读取的字节数据
 */
u8 FM_ReadOneByte(u16 ReadAddr)
{
    u8 temp = 0;                /* 存储读取的数据 */
    u8 addrOffset = 0;          /* 地址偏移量 */

    iic_start(&fm24cxx_iicmanager.stPara);                                   /* 启动I2C通讯 */

#if EE_TYPE > AT24C16
    iic_send_byte(&fm24cxx_iicmanager.stPara, FRAM_WRITE);      /* 发送写命令 */
    iic_send_byte(&fm24cxx_iicmanager.stPara, ReadAddr >> 8);   /* 发送地址高字节 */
#else
    addrOffset = SLAVEADDR_OFFSET(ReadAddr);
    iic_send_byte(&fm24cxx_iicmanager.stPara, FRAM_WRITE + addrOffset);      /* 发送器件地址0XA0,写数据 */
#endif
    iic_send_byte(&fm24cxx_iicmanager.stPara, ReadAddr & 0xFF);              /* 发送地址低字节 */

    iic_start(&fm24cxx_iicmanager.stPara);                                   /* 再次启动I2C通讯 */
    iic_send_byte(&fm24cxx_iicmanager.stPara, FRAM_READ + addrOffset);       /* 进入接收模式 */

    temp = iic_read_byte(&fm24cxx_iicmanager.stPara, 0);                     /* 读取一个字节数据 */
    iic_stop(&fm24cxx_iicmanager.stPara);                                    /* 产生一个停止条件 */
    
    return temp;                                                             /* 返回读取的数据 */
}

/**
 * @brief      向指定地址写入一个字节的数据
 * @param      WriteAddr     要写入的地址
 * @param      DataToWrite   要写入的数据字节
 * @retval     无
 */
void FM_WriteOneByte(u16 WriteAddr, u8 DataToWrite)
{
    EEPROM_WP(0);                                                           /* 禁用写保护 */
    delay_us(10);                                                           /* 延时，确保写保护解除稳定 */
    iic_start(&fm24cxx_iicmanager.stPara);  /* 启动 I2C 通讯 */

#if EE_TYPE > AT24C16
    iic_send_byte(&fm24cxx_iicmanager.stPara, FRAM_WRITE);                  /* 发送写命令 */
    iic_send_byte(&fm24cxx_iicmanager.stPara, WriteAddr >> 8);              /* 发送地址高字节 */
#else
    iic_send_byte(&fm24cxx_iicmanager.stPara, FRAM_WRITE + SLAVEADDR_OFFSET(WriteAddr)); /* 发送器件地址0XA0,写数据 */
#endif
    iic_send_byte(&fm24cxx_iicmanager.stPara, WriteAddr & 0xFF);            /* 发送地址低字节 */
    iic_send_byte(&fm24cxx_iicmanager.stPara, DataToWrite);                 /* 发送数据字节 */
    iic_stop(&fm24cxx_iicmanager.stPara);                                   /* 产生一个停止条件 */

    EEPROM_WP(1);                                                           /* 恢复写保护 */
    delay_us(10);                                                           /* 延时，确保写保护恢复 */
}

/**
 * @brief      检查 EEPROM 是否已初始化
 * @param      无
 * @retval     0   表示 EEPROM 已初始化
 *             1   表示 EEPROM 未初始化
 *
 * 该函数通过读取 EEPROM 中的特定位置（`EE_TYPE`）来判断 EEPROM 是否已经初始化。
 * 如果读取到的字节为预定义的 `TEST_BYTE`，则表示 EEPROM 已经初始化并返回 0。
 * 如果读取到的字节不是 `TEST_BYTE`，则说明 EEPROM 未初始化，函数会写入 `TEST_BYTE`，并重新读取该位置确认初始化成功。
 * 如果初始化成功，返回 0，否则返回 1。
 */
u8 FM_Check(void)
{
    u8 temp;

    EEPROM_WP(0);                                                           /* 禁用写保护 */
    temp = FM_ReadOneByte(EE_TYPE);                                         /* 读取指定位置的数据，避免每次开机都写 FM24CXX */

    if(temp == TEST_BYTE)
        return 0;                                                           /* 已初始化，返回 0 */

    else                                                                    /* 排除第一次初始化的情况 */
    {
        FM_WriteOneByte(EE_TYPE, TEST_BYTE);                                /* 写入初始化标记 */
        temp = FM_ReadOneByte(EE_TYPE);                                     /* 重新读取 */

        if(temp == TEST_BYTE)
            return 0;                                                       /* 初始化成功，返回 0 */
    }

    return 1;                                                               /* 初始化失败，返回 1 */
}

/**
 * @brief      向 EEPROM 连续写入字节数据
 * @param      WriteAddr  写入的起始地址
 * @param      ptr        要写入的数据指针
 * @param      size       写入的数据大小（字节数）
 * @retval     返回值：写操作是否成功
 *             0   表示写入成功
 *             非 0 表示写入过程中出现错误
 */
u8 FM_WriteByteseq(u16 WriteAddr, void *ptr, u16 size)
{
    char *src = ptr;
    u8 u8ACKErr = 0;

    EEPROM_WP(0);                                                                                       /* 禁用写保护 */
    delay_us(10);                                                                                       /* 延时，确保写保护解除稳定 */
    iic_start(&fm24cxx_iicmanager.stPara);                                                              /* 启动 I2C 通讯 */

#if EE_TYPE > AT24C16
    u8ACKErr |= iic_send_byte(&fm24cxx_iicmanager.stPara, FRAM_WRITE);                                  /* 发送写命令 */
    u8ACKErr |= iic_send_byte(&fm24cxx_iicmanager.stPara, WriteAddr >> 8);                              /* 发送地址高字节 */
#else
    u8ACKErr |= iic_send_byte(&fm24cxx_iicmanager.stPara, FRAM_WRITE + SLAVEADDR_OFFSET(WriteAddr));    /* 发送器件地址0XA0，写数据 */
#endif
    u8ACKErr |= iic_send_byte(&fm24cxx_iicmanager.stPara, WriteAddr & 0xFF);                            /* 发送地址低字节 */

    while(size--)                                                                                       /* 循环写入数据 */
    {
        u8ACKErr |= iic_send_byte(&fm24cxx_iicmanager.stPara, *src);                                    /* 发送字节 */
        src++;                                                                                          /* 移动到下一个字节 */
    }

    iic_stop(&fm24cxx_iicmanager.stPara);                                                               /* 产生停止条件 */
    EEPROM_WP(1);                                                                                       /* 恢复写保护 */
    delay_us(10);                                                                                       /* 延时，确保写保护恢复 */

    return u8ACKErr;                                                                                    /* 返回写入错误标志 */
}


/**
 * @brief      从 EEPROM 读取连续字节数据
 * @param      ReadAddr  读取的起始地址
 * @param      ptr       存储读取数据的缓冲区指针
 * @param      size      读取的数据大小（字节数）
 * @retval     0         表示读取成功
 *             非 0     表示读取过程中出现应答错误
 */
u8 FM_ReadByteseq(u16 ReadAddr, void *ptr, u16 size)
{
    char *dst = ptr;
    u8 addrOffset = 0;
    u8 u8AckErr = 0;                                                                                   /* 应答错误标志，1 表示有错误 */
                       
    iic_start(&fm24cxx_iicmanager.stPara);                                                             /* 启动 I2C 通讯 */
                       
#if EE_TYPE > AT24C16                      
    u8AckErr |= iic_send_byte(&fm24cxx_iicmanager.stPara, FRAM_WRITE);                                 /* 发送写命令 */
    u8AckErr |= iic_send_byte(&fm24cxx_iicmanager.stPara, ReadAddr >> 8);                              /* 发送地址高字节 */
#else                      
    addrOffset = SLAVEADDR_OFFSET(ReadAddr);                       
    iic_send_byte(&fm24cxx_iicmanager.stPara, FRAM_WRITE + addrOffset);                                /* 发送器件地址0XA0，写数据 */
#endif                     
    u8AckErr |= iic_send_byte(&fm24cxx_iicmanager.stPara, ReadAddr & 0xFF);                            /* 发送地址低字节 */
                       
    iic_start(&fm24cxx_iicmanager.stPara);                                                             /* 再次启动 I2C 通讯 */
    u8AckErr |= iic_send_byte(&fm24cxx_iicmanager.stPara, FRAM_READ + addrOffset);                     /* 进入接收模式 */
                       
    size--;                                                                                            /* 数据大小减 1，因最后一个字节特殊处理 */
    while (size--)                                                                                     /* 循环读取数据 */
    {                      
        *dst++ = iic_read_byte(&fm24cxx_iicmanager.stPara, 1);                                         /* 读取字节并保存 */
    }                      
                       
    *dst = iic_read_byte(&fm24cxx_iicmanager.stPara, 0);                                               /* 读取最后一个字节，不发送ACK */
    iic_stop(&fm24cxx_iicmanager.stPara);                                                              /* 产生停止条件 */
                       
    return u8AckErr;                                                                                   /* 返回应答错误标志 */
}

