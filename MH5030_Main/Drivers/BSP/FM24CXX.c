#include "FM24CXX.h"

IICManager_ST   fm24cxx_iicmanager;

/**
 * @brief      FM24CXX������ʼ��
 * @param[out] pstIIC   IIC����ṹ��ָ��
 * @retval     void
 * @note       ��ȡһ��Ĭ��IIC����������ʼ��Ŀ��IIC����ṹ�塣
 */
void FM24CXX_iic_init(IICManager_ST *pstIIC)
{
    IICInit_ST stIICInit = {0};                                             /* ��ʼ�����ýṹ�� */
                               
    stIICInit.pstGTD_SCL     = FM24CL16_SCL_GPIO_Port;                      /* SCL�˿� */
    stIICInit.u32Pin_SCL     = FM24CL16_SCL_Pin;                            /* SCL���� */
                               
    stIICInit.pstGTD_SDA     = FM24CL16_SDA_GPIO_Port;                      /* SDA�˿� */
    stIICInit.u32Pin_SDA     = FM24CL16_SDA_Pin;                            /* SDA���� */
                               
    stIICInit.pstGTD_WP      = FM24CL16_WP_GPIO_Port;                       /* ʹ��WP */
    stIICInit.u32Pin_WP      = FM24CL16_SCL_Pin;                            /* WP���ź� */
                               
    stIICInit.u8AddrRD       = FRAM_READ;                                   /* ����ַ */
    stIICInit.u8AddrWR       = FRAM_WRITE;                                  /* д��ַ */
    stIICInit.u16DelayUs     = 2;                                           /* ʱ����ʱ */
    stIICInit.iic_delay      = _iic_delay_us;                               /* ΢����ʱ */
                               
    iic_init(pstIIC, &stIICInit);                                           /* ��ʼ��IIC����ṹ�� */
    EEPROM_WP(1);                                                           /* ����д���� */
}                           
/**
 * @brief  ��ȡһ���ֽڵ�����
 * @param  ReadAddr Ҫ��ȡ�Ĵ洢����ַ
 * @retval ���ش�ָ����ַ��ȡ���ֽ�����
 */
u8 FM_ReadOneByte(u16 ReadAddr)
{
    u8 temp = 0;                /* �洢��ȡ������ */
    u8 addrOffset = 0;          /* ��ַƫ���� */

    iic_start(&fm24cxx_iicmanager.stPara);                                   /* ����I2CͨѶ */

#if EE_TYPE > AT24C16
    iic_send_byte(&fm24cxx_iicmanager.stPara, FRAM_WRITE);      /* ����д���� */
    iic_send_byte(&fm24cxx_iicmanager.stPara, ReadAddr >> 8);   /* ���͵�ַ���ֽ� */
#else
    addrOffset = SLAVEADDR_OFFSET(ReadAddr);
    iic_send_byte(&fm24cxx_iicmanager.stPara, FRAM_WRITE + addrOffset);      /* ����������ַ0XA0,д���� */
#endif
    iic_send_byte(&fm24cxx_iicmanager.stPara, ReadAddr & 0xFF);              /* ���͵�ַ���ֽ� */

    iic_start(&fm24cxx_iicmanager.stPara);                                   /* �ٴ�����I2CͨѶ */
    iic_send_byte(&fm24cxx_iicmanager.stPara, FRAM_READ + addrOffset);       /* �������ģʽ */

    temp = iic_read_byte(&fm24cxx_iicmanager.stPara, 0);                     /* ��ȡһ���ֽ����� */
    iic_stop(&fm24cxx_iicmanager.stPara);                                    /* ����һ��ֹͣ���� */
    
    return temp;                                                             /* ���ض�ȡ������ */
}

/**
 * @brief      ��ָ����ַд��һ���ֽڵ�����
 * @param      WriteAddr     Ҫд��ĵ�ַ
 * @param      DataToWrite   Ҫд��������ֽ�
 * @retval     ��
 */
void FM_WriteOneByte(u16 WriteAddr, u8 DataToWrite)
{
    EEPROM_WP(0);                                                           /* ����д���� */
    delay_us(10);                                                           /* ��ʱ��ȷ��д��������ȶ� */
    iic_start(&fm24cxx_iicmanager.stPara);  /* ���� I2C ͨѶ */

#if EE_TYPE > AT24C16
    iic_send_byte(&fm24cxx_iicmanager.stPara, FRAM_WRITE);                  /* ����д���� */
    iic_send_byte(&fm24cxx_iicmanager.stPara, WriteAddr >> 8);              /* ���͵�ַ���ֽ� */
#else
    iic_send_byte(&fm24cxx_iicmanager.stPara, FRAM_WRITE + SLAVEADDR_OFFSET(WriteAddr)); /* ����������ַ0XA0,д���� */
#endif
    iic_send_byte(&fm24cxx_iicmanager.stPara, WriteAddr & 0xFF);            /* ���͵�ַ���ֽ� */
    iic_send_byte(&fm24cxx_iicmanager.stPara, DataToWrite);                 /* ���������ֽ� */
    iic_stop(&fm24cxx_iicmanager.stPara);                                   /* ����һ��ֹͣ���� */

    EEPROM_WP(1);                                                           /* �ָ�д���� */
    delay_us(10);                                                           /* ��ʱ��ȷ��д�����ָ� */
}

/**
 * @brief      ��� EEPROM �Ƿ��ѳ�ʼ��
 * @param      ��
 * @retval     0   ��ʾ EEPROM �ѳ�ʼ��
 *             1   ��ʾ EEPROM δ��ʼ��
 *
 * �ú���ͨ����ȡ EEPROM �е��ض�λ�ã�`EE_TYPE`�����ж� EEPROM �Ƿ��Ѿ���ʼ����
 * �����ȡ�����ֽ�ΪԤ����� `TEST_BYTE`�����ʾ EEPROM �Ѿ���ʼ�������� 0��
 * �����ȡ�����ֽڲ��� `TEST_BYTE`����˵�� EEPROM δ��ʼ����������д�� `TEST_BYTE`�������¶�ȡ��λ��ȷ�ϳ�ʼ���ɹ���
 * �����ʼ���ɹ������� 0�����򷵻� 1��
 */
u8 FM_Check(void)
{
    u8 temp;

    EEPROM_WP(0);                                                           /* ����д���� */
    temp = FM_ReadOneByte(EE_TYPE);                                         /* ��ȡָ��λ�õ����ݣ�����ÿ�ο�����д FM24CXX */

    if(temp == TEST_BYTE)
        return 0;                                                           /* �ѳ�ʼ�������� 0 */

    else                                                                    /* �ų���һ�γ�ʼ������� */
    {
        FM_WriteOneByte(EE_TYPE, TEST_BYTE);                                /* д���ʼ����� */
        temp = FM_ReadOneByte(EE_TYPE);                                     /* ���¶�ȡ */

        if(temp == TEST_BYTE)
            return 0;                                                       /* ��ʼ���ɹ������� 0 */
    }

    return 1;                                                               /* ��ʼ��ʧ�ܣ����� 1 */
}

/**
 * @brief      �� EEPROM ����д���ֽ�����
 * @param      WriteAddr  д�����ʼ��ַ
 * @param      ptr        Ҫд�������ָ��
 * @param      size       д������ݴ�С���ֽ�����
 * @retval     ����ֵ��д�����Ƿ�ɹ�
 *             0   ��ʾд��ɹ�
 *             �� 0 ��ʾд������г��ִ���
 */
u8 FM_WriteByteseq(u16 WriteAddr, void *ptr, u16 size)
{
    char *src = ptr;
    u8 u8ACKErr = 0;

    EEPROM_WP(0);                                                                                       /* ����д���� */
    delay_us(10);                                                                                       /* ��ʱ��ȷ��д��������ȶ� */
    iic_start(&fm24cxx_iicmanager.stPara);                                                              /* ���� I2C ͨѶ */

#if EE_TYPE > AT24C16
    u8ACKErr |= iic_send_byte(&fm24cxx_iicmanager.stPara, FRAM_WRITE);                                  /* ����д���� */
    u8ACKErr |= iic_send_byte(&fm24cxx_iicmanager.stPara, WriteAddr >> 8);                              /* ���͵�ַ���ֽ� */
#else
    u8ACKErr |= iic_send_byte(&fm24cxx_iicmanager.stPara, FRAM_WRITE + SLAVEADDR_OFFSET(WriteAddr));    /* ����������ַ0XA0��д���� */
#endif
    u8ACKErr |= iic_send_byte(&fm24cxx_iicmanager.stPara, WriteAddr & 0xFF);                            /* ���͵�ַ���ֽ� */

    while(size--)                                                                                       /* ѭ��д������ */
    {
        u8ACKErr |= iic_send_byte(&fm24cxx_iicmanager.stPara, *src);                                    /* �����ֽ� */
        src++;                                                                                          /* �ƶ�����һ���ֽ� */
    }

    iic_stop(&fm24cxx_iicmanager.stPara);                                                               /* ����ֹͣ���� */
    EEPROM_WP(1);                                                                                       /* �ָ�д���� */
    delay_us(10);                                                                                       /* ��ʱ��ȷ��д�����ָ� */

    return u8ACKErr;                                                                                    /* ����д������־ */
}


/**
 * @brief      �� EEPROM ��ȡ�����ֽ�����
 * @param      ReadAddr  ��ȡ����ʼ��ַ
 * @param      ptr       �洢��ȡ���ݵĻ�����ָ��
 * @param      size      ��ȡ�����ݴ�С���ֽ�����
 * @retval     0         ��ʾ��ȡ�ɹ�
 *             �� 0     ��ʾ��ȡ�����г���Ӧ�����
 */
u8 FM_ReadByteseq(u16 ReadAddr, void *ptr, u16 size)
{
    char *dst = ptr;
    u8 addrOffset = 0;
    u8 u8AckErr = 0;                                                                                   /* Ӧ������־��1 ��ʾ�д��� */
                       
    iic_start(&fm24cxx_iicmanager.stPara);                                                             /* ���� I2C ͨѶ */
                       
#if EE_TYPE > AT24C16                      
    u8AckErr |= iic_send_byte(&fm24cxx_iicmanager.stPara, FRAM_WRITE);                                 /* ����д���� */
    u8AckErr |= iic_send_byte(&fm24cxx_iicmanager.stPara, ReadAddr >> 8);                              /* ���͵�ַ���ֽ� */
#else                      
    addrOffset = SLAVEADDR_OFFSET(ReadAddr);                       
    iic_send_byte(&fm24cxx_iicmanager.stPara, FRAM_WRITE + addrOffset);                                /* ����������ַ0XA0��д���� */
#endif                     
    u8AckErr |= iic_send_byte(&fm24cxx_iicmanager.stPara, ReadAddr & 0xFF);                            /* ���͵�ַ���ֽ� */
                       
    iic_start(&fm24cxx_iicmanager.stPara);                                                             /* �ٴ����� I2C ͨѶ */
    u8AckErr |= iic_send_byte(&fm24cxx_iicmanager.stPara, FRAM_READ + addrOffset);                     /* �������ģʽ */
                       
    size--;                                                                                            /* ���ݴ�С�� 1�������һ���ֽ����⴦�� */
    while (size--)                                                                                     /* ѭ����ȡ���� */
    {                      
        *dst++ = iic_read_byte(&fm24cxx_iicmanager.stPara, 1);                                         /* ��ȡ�ֽڲ����� */
    }                      
                       
    *dst = iic_read_byte(&fm24cxx_iicmanager.stPara, 0);                                               /* ��ȡ���һ���ֽڣ�������ACK */
    iic_stop(&fm24cxx_iicmanager.stPara);                                                              /* ����ֹͣ���� */
                       
    return u8AckErr;                                                                                   /* ����Ӧ������־ */
}

