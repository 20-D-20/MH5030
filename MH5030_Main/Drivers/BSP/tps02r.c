#include "tps02r.h"
// ����ȫ��IIC����ṹ��ͳ�ʼ�����ýṹ��
IICManager_ST               tps02r_iicmanger;

void tps02r_iic_init(IICManager_ST *pstIIC)
{
    IICInit_ST stIICInit = {0};                                                /* ��ʼ�����ýṹ�� */
                             
    stIICInit.pstGTD_SCL     = TPS02RAH_SCL_GPIO_Port;                         /* SCL�˿� */
    stIICInit.u32Pin_SCL     = TPS02RAH_SCL_Pin;                               /* SCL���� */
                            
    stIICInit.pstGTD_SDA     = TPS02RAH_SDA_GPIO_Port;                         /* SDA�˿� */
    stIICInit.u32Pin_SDA     = TPS02RAH_SDA_Pin;                               /* SDA���� */
                            
    stIICInit.pstGTD_WP      = NULL;                                           /* ��ʹ��WP */
    stIICInit.u32Pin_WP      = 0;                                              /* WP���ź� */
                            
    stIICInit.u8AddrRD       = TPS02R_I2C_ADR_R;                               /* ����ַ */
    stIICInit.u8AddrWR       = TPS02R_I2C_ADR_W;                               /* д��ַ */
    stIICInit.u16DelayUs     = 2;                                              /* ʱ����ʱ */
    stIICInit.iic_delay      = _iic_delay_us;                                  /* ΢����ʱ */
                            
    iic_init(pstIIC, &stIICInit);                                              /* IIC��ʼ�� */      
}

/**
 * @brief      ����ȡ���ļĴ���ֵתΪ�¶�ֵ
 * @param      p_data  ��ȡ���ļĴ������ݣ�3�ֽڣ�
 * @retval     �¶�ֵ����λ���棩
 */
static float __tps02r_reg_to_temp(uint8_t *p_data)
{
    uint32_t temp = 0;

    /* �ϲ��Ĵ�������Ϊ 24 λֵ */
    temp |= p_data[0] << 16;
    temp |= p_data[1] << 8;
    temp |= p_data[2];

    /* ��ǰ�����¶�Ϊ���¶�ֵ */
    if (temp >= 8388608)
    {
        return (-(float)((16777216 - temp) / (float)(0x1 << 13)));              /* ���¶ȴ��� */
    }
    else
    {
        return ((float)(temp / (float)(0x1 << 13)));                            /* ���¶ȴ��� */
    }
}

/**
 * @brief      ��ȡָ���Ĵ���ͨ�����¶�ֵ
 * 
 * @param      chan    �¶�ͨ����0 �� 1���ֱ��ʾͨ�� 0 ��ͨ�� 1��
 * @param      p_data   �洢�¶����ݵ�ָ�룬��ȡ���¶�ֵ�������ڴ�ָ��ָ����ڴ���
 * 
 * @retval     TPS02R_FUN_OK     �ɹ���ȡ�¶�ֵ
 * @retval     TPS02R_VALUE_ERROR �����������豸ָ�������ָ��Ϊ NULL��ͨ����Ч
 * @retval     TPS02R_FUN_ERROR  ��ȡ����ʧ��
 */
int tps02r_get_temp(int8_t chan, float *p_data)
{
    u8 error = 0;                                                               /* �洢����״̬ */
    float temp_buff[2] = {0};                                                   /* �洢ͨ���¶�ֵ�Ļ����� */
    uint8_t rx_buff[6] = {0};                                                   /* �洢��ȡ�����ݻ����� */

    if (p_data == NULL) 
    {
        return TPS02R_VALUE_ERROR;                                              /* ������飬�豸ָ�������ָ��Ϊ�� */
    }

    if (chan < 0 || chan > 1) 
    {
        return TPS02R_VALUE_ERROR;                                              /* ������飬ͨ������Ч */
    }
    
    /* ���ö�ȡ���ݺ���������ֵ�ж϶�ȡ�Ƿ�ɹ� */
    iic_start(&tps02r_iicmanger.stPara);                                        /* ��ʼ */    
    error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_I2C_ADR_W);          /* �豸д��ַ */
    error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_TEMP_ADDR);          /* �������� */
    iic_start(&tps02r_iicmanger.stPara);                                        /* ��ʼ */
    error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_I2C_ADR_R);          /* �豸д��ַ */

    if (error != 0)                                                             /* ����ֵ�ж϶�ȡ�Ƿ�ɹ� */
    {
        return TPS02R_FUN_ERROR;                                                /* ���ݶ�ȡʧ�ܣ����ش����� */
    }

    for(int i=0; i < TPS02R_TEMP_LEN;i++)                                       /* ��ȡ�¶ȼĴ������ݣ���6���ֽ� */
    {
        if(i < TPS02R_TEMP_LEN - 1)
        {
            rx_buff[i] = iic_read_byte(&tps02r_iicmanger.stPara, 1);            /* �����ֽڣ�������ACK */
        }
        else
        {
            rx_buff[i] = iic_read_byte(&tps02r_iicmanger.stPara, 0);            /* ��ȡ����������NACK */
            iic_stop(&tps02r_iicmanger.stPara);                                 /* ֹͣ�ź� */
        }
    }
    
    if (chan == 0) 
    {
        temp_buff[0] = __tps02r_reg_to_temp(&rx_buff[0]);
        *p_data = temp_buff[0];                                                 /* ��ͨ�� 0 ���¶�ֵ�洢������ָ�� */
    }
    else if (chan == 1) 
    {
        temp_buff[1] = __tps02r_reg_to_temp(&rx_buff[3]);
        *p_data = temp_buff[1];                                                 /* ��ͨ�� 1 ���¶�ֵ�洢������ָ�� */
    }

    return TPS02R_FUN_OK;                                                       /* �ɹ���ȡ�¶�ֵ�����سɹ�״̬ */
}



/**
 * @brief      ����ͨ���������¶�ֵ
 * 
 * @param      chan    �¶�ͨ����0 �� 1���ֱ��ʾͨ�� 0 ��ͨ�� 1��
 * @param      temp    Ҫ���õ������¶�ֵ����λ�����϶ȣ�
 * 
 * @retval     TPS02R_FUN_OK     �ɹ����������¶�ֵ
 * @retval     TPS02R_VALUE_ERROR �����������豸ָ��Ϊ�ա�ͨ����Ч��
 * @retval     TPS02R_FUN_ERROR  д������ʧ��
 */
int tps02r_set_high(int8_t chan, float temp)
{
    int error = 0;                                                              /* �洢����״̬ */
    uint32_t temp_code = 0;                                                     /* �洢�¶ȱ��� */
    uint8_t temp_buff[6] = {0};                                                 /* �洢�¶����ݻ����� */
    int32_t temp1 = 0;                                                          /* �洢ת������¶�ֵ */
    
    temp1 = (int64_t)((temp) * (0x1 << 13));                                    /* ���¶�ת��Ϊ�����ʽ */

    /* ��ȡ��ǰ���¶����� */
    iic_start(&tps02r_iicmanger.stPara);                                        /* ��ʼ */    
    error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_I2C_ADR_W);          /* �豸д��ַ */
    error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_THIG_ADDR);          /* �������� */
    iic_start(&tps02r_iicmanger.stPara);                                        /* ��ʼ */
    error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_I2C_ADR_R);          /* �豸д��ַ */

    if (error != 0)                                                             /* ����ֵ�ж϶�ȡ�Ƿ�ɹ� */
    {
        return TPS02R_FUN_ERROR;                                                /* ���ݶ�ȡʧ�ܣ����ش����� */
    }

    for(int i=0; i<TPS02R_THIG_LEN;i++)                                         /* �����¶ȼĴ������ݣ���6���ֽ� */
    {
        if(i < TPS02R_THIG_LEN - 1)
        {
            temp_buff[i] = iic_read_byte(&tps02r_iicmanger.stPara, 1);          /* �����ֽڣ�������ACK */
        }
        else
        {
            temp_buff[i] = iic_read_byte(&tps02r_iicmanger.stPara, 0);          /* ��ȡ����������NACK */
            iic_stop(&tps02r_iicmanger.stPara);                                 /* ֹͣ�ź� */
        }
    }

    /* �����¶�ֵΪ��������� */
    if (temp1 < 0) 
    {
        temp1 = -temp1;
        temp_code = 16777216 - temp1;                                           /* ���㸺�¶ȵı���ֵ */
    } 
    else 
    {
        temp_code = temp1;                                                      /* ���¶�ֱ��ʹ��ת�����ֵ */
    }

    /* ����ͨ��ѡ���Ӧ�Ļ���������������ֵ */
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
        return TPS02R_VALUE_ERROR;                                               /* ������飬ͨ����Ч */
    }

    /* �����õ������¶�ֵд���豸 */
    iic_start(&tps02r_iicmanger.stPara);                                         /* ��ʼ */    
    error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_I2C_ADR_W);           /* �豸д��ַ */
    error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_THIG_ADDR);           /* �������� */

    if (error != 0)                                                              /* ����ֵ�ж϶�ȡ�Ƿ�ɹ� */
    {
        return TPS02R_FUN_ERROR;                                                 /* ���ݶ�ȡʧ�ܣ����ش����� */
    }

    for(int i=0; i < TPS02R_THIG_LEN;i++)                                        /* ��ȡ�¶ȼĴ������ݣ���6���ֽ� */
    {
        if(i < TPS02R_THIG_LEN - 1)
        {
            iic_send_byte(&tps02r_iicmanger.stPara, temp_buff[i]);               /* �����õ������¶�ֵд��Ĵ���  */
        }
        else
        {
            iic_send_byte(&tps02r_iicmanger.stPara, temp_buff[i]);               
            iic_stop(&tps02r_iicmanger.stPara);                                  /* ֹͣ�ź� */
        }
    }

    return TPS02R_FUN_OK;                                                        /* �ɹ����������¶�ֵ�����سɹ�״̬ */
}


/**
 * @brief      ����ͨ���������¶�ֵ
 * 
 * @param      chan    �¶�ͨ����0 �� 1���ֱ��ʾͨ�� 0 ��ͨ�� 1��
 * @param      temp    Ҫ���õ������¶�ֵ����λ�����϶ȣ�
 * 
 * @retval     TPS02R_FUN_OK     �ɹ����������¶�ֵ
 * @retval     TPS02R_VALUE_ERROR �����������豸ָ��Ϊ�ա�ͨ����Ч��
 * @retval     TPS02R_FUN_ERROR  д������ʧ��
 */

int tps02r_set_low(int8_t chan, float temp)
{
    int error = 0;                                                              /* �洢����״̬ */
    uint32_t temp_code = 0;                                                     /* �洢�¶ȱ��� */
    uint8_t temp_buff[6] = {0};                                                 /* �洢�¶����ݻ����� */
    int32_t temp1 = 0;                                                          /* �洢ת������¶�ֵ */
        
    temp1 = (int64_t)((temp) * (0x1 << 13));                                    /* ���¶�ת��Ϊ�����ʽ */
        
    /* ��ȡ��ǰ���¶����� */ 
    iic_start(&tps02r_iicmanger.stPara);                                        /* ��ʼ */    
    error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_I2C_ADR_W);          /* �豸д��ַ */
    error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_TLOW_ADDR);          /* �������� */
    iic_start(&tps02r_iicmanger.stPara);                                        /* ��ʼ */
    error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_I2C_ADR_R);          /* �豸д��ַ */
    
    if (error != 0)                                                             /* ����ֵ�ж϶�ȡ�Ƿ�ɹ� */
    {   
        return TPS02R_FUN_ERROR;                                                /* ���ݶ�ȡʧ�ܣ����ش����� */
    }   
    
    for(int i=0; i < TPS02R_TLOW_LEN;i++)                                       /* ��ȡ�¶ȼĴ������ݣ���6���ֽ� */
    {   
        if(i < TPS02R_TLOW_LEN - 1) 
        {   
            temp_buff[i] = iic_read_byte(&tps02r_iicmanger.stPara, 1);          /* �����ֽڣ�������ACK */
        }   
        else    
        {   
            temp_buff[i] = iic_read_byte(&tps02r_iicmanger.stPara, 0);          /* ��ȡ����������NACK */
            iic_stop(&tps02r_iicmanger.stPara);                                 /* ֹͣ�ź� */
        }   
    }   
    
    /* �����¶�ֵΪ��������� */   
    if (temp1 < 0)  
    {   
        temp1 = -temp1; 
        temp_code = 16777216 - temp1;                                           /* ���㸺�¶ȵı���ֵ */
    } 
    else 
    {
        temp_code = temp1;                                                      /* ���¶�ֱ��ʹ��ת�����ֵ */
    }

    /* ����ͨ��ѡ���Ӧ�Ļ���������������ֵ */
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
        return TPS02R_VALUE_ERROR;                                               /* ������飬ͨ����Ч */
    }

    /* �����õ������¶�ֵд���豸 */
    iic_start(&tps02r_iicmanger.stPara);                                         /* ��ʼ */    
    error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_I2C_ADR_W);           /* �豸д��ַ */
    error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_TLOW_ADDR);           /* �������� */

    if (error != 0)                                                              /* ����ֵ�ж϶�ȡ�Ƿ�ɹ� */
    {
        return TPS02R_FUN_ERROR;                                                 /* ���ݶ�ȡʧ�ܣ����ش����� */
    }

    for(int i=0; i < TPS02R_TLOW_LEN;i++)                                        /* �����¶ȼĴ������ݣ���6���ֽ� */
    {
        if(i < TPS02R_TLOW_LEN - 1)
        {
            iic_send_byte(&tps02r_iicmanger.stPara, temp_buff[i]);               /* �����õ������¶�ֵд��Ĵ���  */
        }
        else
        {
            iic_send_byte(&tps02r_iicmanger.stPara, temp_buff[i]);               
            iic_stop(&tps02r_iicmanger.stPara);                                  /* ֹͣ�ź� */
        }
    }

    return TPS02R_FUN_OK;
}

/**
 * @brief      ������ͨ���Ĳ������ʽ�������
 * @param      chan_en  ����ѡ��0x00/0x01/0x02/0x03
 * @param      rate     ��������
 
 * @retval     TPS02R_FUN_OK     ���óɹ�
 * @retval     TPS02R_FUN_ERROR  ��дʧ��
 */
int tps02r_set_chan_sampling_rate(int8_t chan_en, uint32_t rate)
{
    uint8_t cfg_buff[2] = {0};                                                                  /* ��ʱ���û��� */
    int error = 0;
    /* ��ȡ��ǰ�����üĴ��������� */
    iic_start(&tps02r_iicmanger.stPara);                                                        /* ��ʼ */    
    error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_I2C_ADR_W);                          /* �豸д��ַ */
    error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_CFG_ADDR);                           /* �������� */
    iic_start(&tps02r_iicmanger.stPara);                                                        /* ��ʼ */
    error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_I2C_ADR_R);                          /* �豸д��ַ */

    if (error != 0)                                                                             /* ����ֵ�ж϶�ȡ�Ƿ�ɹ� */
    {
        return TPS02R_FUN_ERROR;                                                                /* ���ݶ�ȡʧ�ܣ����ش����� */
    }

    for(int i=0; i < TPS02R_CFG_LEN;i++)                                                        /* ��ȡ���üĴ������ݣ���2���ֽ� */
    {
        if(i < TPS02R_CFG_LEN - 1)
        {
            cfg_buff[i] = iic_read_byte(&tps02r_iicmanger.stPara, 1);                           /* �����ֽڣ�������ACK */
        }
        else
        {
            cfg_buff[i] = iic_read_byte(&tps02r_iicmanger.stPara, 0);                           /* ��ȡ����������NACK */
            iic_stop(&tps02r_iicmanger.stPara);                                                 /* ֹͣ�ź� */
        }
    }

    if (chan_en == 0x00 || chan_en == 0x02 || chan_en == 0x03)
    {
        cfg_buff[0] &= (~TPS02R_CFG_RATE_MASK);                                                 /* ���ԭ�������� */
        cfg_buff[0] |= (rate << TPS02R_CFG_RATE_SHIFT) & TPS02R_CFG_RATE_MASK;                  /* �����²������� */
    }
    else if (chan_en == 0x01)
    {
        cfg_buff[1] &= (~TPS02R_CFG_RATE_MASK);                                                 /* ���ԭ�������� */
        cfg_buff[1] |= (rate << TPS02R_CFG_RATE_SHIFT) & TPS02R_CFG_RATE_MASK;                  /* �����²������� */
    }

     /* �����õĲ�����д���豸 */
    iic_start(&tps02r_iicmanger.stPara);                                                        /* ��ʼ */    
    error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_I2C_ADR_W);                          /* �豸д��ַ */
    error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_CFG_ADDR);                           /* �������� */

    if (error != 0)                                                                             /* ����ֵ�ж϶�ȡ�Ƿ�ɹ� */
    {
        return TPS02R_FUN_ERROR;                                                                /* ���ݶ�ȡʧ�ܣ����ش����� */
    }

    for(int i=0; i < TPS02R_CFG_LEN;i++)                                                        /* �����¶ȼĴ������ݣ���6���ֽ� */
    {
        if(i < TPS02R_CFG_LEN - 1)
        {
            iic_send_byte(&tps02r_iicmanger.stPara, cfg_buff[i]);                               /* �����õĲ�����д��Ĵ��� */
        }
        else
        {
            iic_send_byte(&tps02r_iicmanger.stPara, cfg_buff[i]);               
            iic_stop(&tps02r_iicmanger.stPara);                                                 /* ֹͣ�ź� */
        }
    }
    
    return TPS02R_FUN_OK;                                                                       /* ���óɹ� */
}

/**
 * @brief      �� tps02r ģ������ó�ʼ��ΪĬ��ֵ
 * @retval     TPS02R_VALUE_ERROR  ���Ϊ��
 * @retval     TPS02R_FUN_ERROR    ��д�Ĵ���ʧ��
 * @retval     TPS02R_FUN_OK       ��ʼ���ɹ�
 */
int tps02r_cfg_init(void)
{
    int     error = 0;                                                                         /* ������ */
    uint8_t cfg_buff[2]   = {0};                                                               /* ���üĴ������� */
    uint8_t temph_buff[6] = {0};                                                               /* �����¶ȼĴ������� */
    uint8_t templ_buff[6] = {0};                                                               /* �����¶ȼĴ������� */

    /* Ĭ���¶�����ֵΪ 1023.999878�棨����ͨ���� */
    temph_buff[0] = 0x7f;
    temph_buff[1] = 0xff;
    temph_buff[2] = 0xff;

    temph_buff[3] = 0x7f;
    temph_buff[4] = 0xff;
    temph_buff[5] = 0xff;


    /* �����õ������¶�ֵд���豸 */
    iic_start(&tps02r_iicmanger.stPara);                                                       /* ��ʼ */    
    error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_I2C_ADR_W);                         /* �豸д��ַ */
    error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_THIG_ADDR);                         /* �������� */

    if (error != 0)                                                                            /* ����ֵ�ж϶�ȡ�Ƿ�ɹ� */
    {
        return TPS02R_FUN_ERROR;                                                               /* ���ݶ�ȡʧ�ܣ����ش����� */
    }

    for(int i=0; i < TPS02R_THIG_LEN;i++)                                                      /* ���ͽo�¶ȼĴ������ݣ���6���ֽ� */
    {
        if(i < TPS02R_THIG_LEN - 1)
        {
            iic_send_byte(&tps02r_iicmanger.stPara, temph_buff[i]);                            /* �����õ��������¶�ֵд��Ĵ���  */
        }
        else
        {
            iic_send_byte(&tps02r_iicmanger.stPara, temph_buff[i]);               
            iic_stop(&tps02r_iicmanger.stPara);                                                /* ֹͣ�ź� */
        }
    }

    /* Ĭ���¶�����ֵΪ -0.000122�棨����ͨ���� */
    templ_buff[0] = 0xff;
    templ_buff[1] = 0xff;
    templ_buff[2] = 0xff;

    templ_buff[3] = 0xff;
    templ_buff[4] = 0xff;
    templ_buff[5] = 0xff;

    /* �����õ������¶�ֵд���豸 */
    iic_start(&tps02r_iicmanger.stPara);                                                       /* ��ʼ */    
    error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_I2C_ADR_W);                         /* �豸д��ַ */
    error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_TLOW_ADDR);                         /* �������� */

    if (error != 0)                                                                            /* ����ֵ�ж϶�ȡ�Ƿ�ɹ� */
    {
        return TPS02R_FUN_ERROR;                                                               /* ���ݶ�ȡʧ�ܣ����ش����� */
    }

    for(int i=0; i < TPS02R_TLOW_LEN;i++)                                                      /* �����¶ȼĴ������ݣ���6���ֽ� */
    {
        if(i < TPS02R_TLOW_LEN - 1)
        {
            iic_send_byte(&tps02r_iicmanger.stPara, templ_buff[i]);                             /* �����õ������¶�ֵд��Ĵ���  */
        }
        else
        {
            iic_send_byte(&tps02r_iicmanger.stPara, templ_buff[i]);               
            iic_stop(&tps02r_iicmanger.stPara);                                                 /* ֹͣ�ź� */
        }
    }

    /* ��ȡ��ǰ�����üĴ��������� */
    iic_start(&tps02r_iicmanger.stPara);                                                        /* ��ʼ */    
    error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_I2C_ADR_W);                          /* �豸д��ַ */
    error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_CFG_ADDR);                           /* �������� */
    iic_start(&tps02r_iicmanger.stPara);                                                        /* ��ʼ */
    error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_I2C_ADR_R);                          /* �豸д��ַ */

    if (error != 0)                                                                             /* ����ֵ�ж϶�ȡ�Ƿ�ɹ� */
    {
        return TPS02R_FUN_ERROR;                                                                /* ���ݶ�ȡʧ�ܣ����ش����� */
    }

    for(int i=0; i < TPS02R_CFG_LEN;i++)                                                        /* ��ȡ���üĴ������ݣ���2���ֽ� */
    {
        if(i < TPS02R_CFG_LEN - 1)
        {
            cfg_buff[i] = iic_read_byte(&tps02r_iicmanger.stPara, 1);                           /* �����ֽڣ�������ACK */
        }
        else
        {
            cfg_buff[i] = iic_read_byte(&tps02r_iicmanger.stPara, 0);                           /* ��ȡ����������NACK */
            iic_stop(&tps02r_iicmanger.stPara);                                                 /* ֹͣ�ź� */
        }
    }

    /* Ĭ�����üĴ��� BYTE1��EN=0, R0=0, F1=F0=1, POL=0, TM=0, SD=0���̶�λ���֣� */
    cfg_buff[0] &= (~0x3E);
    cfg_buff[0] |= TPS02R_ALARM_MODE_2 & TPS02R_CFG_ALARM_MODE_MASK;
    cfg_buff[0] |= (0x03 << TPS02R_CFG_TRIGGER_WARNING_SHIFT) & TPS02R_CFG_TRIGGER_WARNING_MASK;
    cfg_buff[0] |= (TPS02R_SAMPLE_RATE_10 << TPS02R_CFG_RATE_SHIFT) & TPS02R_CFG_RATE_MASK;

    /* Ĭ�����üĴ��� BYTE2��EN=1, R0=0, F1=F0=1, POL=0, TM=0, SD=0���̶�λ���֣� */
    cfg_buff[1] &= (~0x3E);
    cfg_buff[1] |= TPS02R_ALARM_MODE_2 & TPS02R_CFG_ALARM_MODE_MASK;
    cfg_buff[1] |= (0x03 << TPS02R_CFG_TRIGGER_WARNING_SHIFT) & TPS02R_CFG_TRIGGER_WARNING_MASK;
    cfg_buff[1] |= (TPS02R_SAMPLE_RATE_10 << TPS02R_CFG_RATE_SHIFT) & TPS02R_CFG_RATE_MASK;
    cfg_buff[1] |= (0x01 << TPS02R_CFG_EN_SHIFT);

    /* ������д���豸 */
    iic_start(&tps02r_iicmanger.stPara);                                                        /* ��ʼ */    
    error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_I2C_ADR_W);                          /* �豸д��ַ */
    error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_CFG_ADDR);                           /* �������� */

    if (error != 0)                                                                             /* ����ֵ�ж϶�ȡ�Ƿ�ɹ� */
    {
        return TPS02R_FUN_ERROR;                                                                /* ���ݶ�ȡʧ�ܣ����ش����� */
    }

    for(int i=0; i < TPS02R_CFG_LEN;i++)                                                        /* �������ã���2���ֽ� */
    {
        if(i < TPS02R_CFG_LEN - 1)
        {
            iic_send_byte(&tps02r_iicmanger.stPara, cfg_buff[i]);                               /* �����ò���д��Ĵ��� */
        }
        else
        {
            iic_send_byte(&tps02r_iicmanger.stPara, cfg_buff[i]);               
            iic_stop(&tps02r_iicmanger.stPara);                                                 /* ֹͣ�ź� */
        }
    }

    return TPS02R_FUN_OK;                                                                       /* ��ʼ���ɹ� */
}


/**
 * @brief      ��ȡ���üĴ���ͨ������
 * @param      chan    ͨ����0 �� 1
 * @param      p_data  �������Ӧͨ���������ֽ�
 
 * @retval     TPS02R_VALUE_ERROR  ��ηǷ�
 * @retval     TPS02R_FUN_ERROR    ��ȡ�Ĵ���ʧ��
 * @retval     TPS02R_FUN_OK       �ɹ�
 */
int tps02r_get_cfg_value(int8_t chan, uint8_t *p_data)
{
    /* ����п� */
    if (p_data == NULL)
    {
        return TPS02R_VALUE_ERROR;
    }

    int error = 0;
    uint8_t cfg_buff[2] = {0};

    /* ��ȡ���üĴ��� */
   iic_start(&tps02r_iicmanger.stPara);                                                        /* ��ʼ */    
   error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_I2C_ADR_W);                          /* �豸д��ַ */
   error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_CFG_ADDR);                           /* �������� */
   iic_start(&tps02r_iicmanger.stPara);                                                        /* ��ʼ */
   error |= iic_send_byte(&tps02r_iicmanger.stPara,TPS02R_I2C_ADR_R);                          /* �豸д��ַ */

   if (error != 0)                                                                             /* ����ֵ�ж϶�ȡ�Ƿ�ɹ� */
   {
       return TPS02R_FUN_ERROR;                                                                /* ���ݶ�ȡʧ�ܣ����ش����� */
   }

   for(int i=0; i < TPS02R_CFG_LEN;i++)                                                        /* ��ȡ���üĴ������ݣ���2���ֽ� */
   {
       if(i < TPS02R_CFG_LEN - 1)
       {
           cfg_buff[i] = iic_read_byte(&tps02r_iicmanger.stPara, 1);                           /* �����ֽڣ�������ACK */
       }
       else
       {
           cfg_buff[i] = iic_read_byte(&tps02r_iicmanger.stPara, 0);                           /* ��ȡ����������NACK */
           iic_stop(&tps02r_iicmanger.stPara);                                                 /* ֹͣ�ź� */
       }
   }

    /* ����ͨ���������� */
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

