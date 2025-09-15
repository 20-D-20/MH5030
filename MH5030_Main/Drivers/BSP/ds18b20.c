#include "ds18b20.h"

/*Ds18b20ȫ�ֱ���*/

Ds18b20Sensor_t ds18b20_1;
//Ds18b20Sensor_t ds18b20_2;
OneWire_t OneWire_1;
//OneWire_t OneWire_2;

/**
 * @brief      �ֶ���������ȡ DS18B20 �¶ȣ����豸������
 * @param      ds18b20   Ŀ�괫�����ľ��ָ�루д�� Temperature �� DataIsValid��
 * @param      OneWire   OneWire ���߾��ָ��
 * @retval     true      ��ȡ�ɹ���ds18b20->Temperature Ϊ���϶ȣ�DataIsValid = true
 * @retval     false     ��ʱ���ȡʧ�ܣ�DataIsValid = false��Temperature �����»򱣳��ϴ�ֵ��
 *
 * ���̣�
 * 1) �����¶�ת�������� VDD ���磬����ǿ��������
 * 2) ��ѯ�ȴ�ת����ɻ�ʱ��΢�뼶��ʱ��������ϵͳ tick����
 * 3) ��ɺ��ȡ�¶ȣ���ʧ�ܻ�ʱ�����Ϊ��Ч��
 *
 * ע�⣺
 * - ���������������Ͻ���һ�� DS18B20����������ƥ�䣩����Ϊ���豸������ Match ROM��
 * - ��ѯ�������á�����ȡ�����ķ�ʽ���㣬�������ϴ�ʱ����ѯһ�Ρ�
 */
bool Ds18b20_ManualConvert(Ds18b20Sensor_t *ds18b20, OneWire_t *OneWire)
{
    /* �������豸�¶�ת�����ڲ�Ӧ������λ�� SKIP/MATCH ROM �߼��� */
    DS18B20_Start(OneWire);                                              /* ��ʼת�� */

    /* ��ѯ�ȴ���ɻ�ʱ������ȡ��������©��һ�� */
    uint32_t loops = (DS18B20_CONVERT_TIMEOUT_US + DS18B20_POLL_INTERVAL_US - 1)
                     / DS18B20_POLL_INTERVAL_US;                         /* ������ѯ���� */
    bool done = false;                                                   /* ת����ɱ�־ */

    while (loops--)
    {
        if (DS18B20_AllDone(OneWire))                                    /* ת����ɣ� */
        {
            done = true;
            break;
        }
        delay_us(DS18B20_POLL_INTERVAL_US);                              /* ΢�뼶��ѯ��� */
    }

    if (!done)
    {
        ds18b20->DataIsValid = false;                                    /* ��ʱ��������Ч */
        return false;
    }

    /* ת����ɺ��ȡ�¶ȵ����ر������ɹ���д�ؾ�� */
    float t = 0.0f;
    bool ok = DS18B20_Read(OneWire, &t);                                 /* ��ȡ�¶ȣ��棩 */
    ds18b20->DataIsValid = ok;                                           /* �����Ч�� */
    if (ok)
    {
        ds18b20->Temperature = t;                                        /* ���ɹ�ʱ���� */
    }
    return ok;
}

/**
 * @brief      ���� DS18B20 �¶�ת�������豸/��ѡ��������
 * @param      OneWire   OneWire ���߾��ָ��
 * @retval     1         �豸��λ�����·�ת������
 * @retval     0         δ��⵽�豸��λ����λ����Ӧ��
 *
 * ���̣�
 * 1) ��λ������豸��λ��presence pulse����
 * 2) ���� SKIP ROM�����豸������ǰƥ��ĳ�������
 * 3) ���� CONVERT T �����������¶�ת����
 *
 * ˵����
 * - ��ʵ�ּ������� VDD ���磬����ǿ��������Ϊ������Դģʽ������ת���ڼ��ṩǿ������
 */
uint8_t DS18B20_Start(OneWire_t *OneWire)
{
    uint8_t present = OneWire_Reset(OneWire);                   /* ��λ�������λ */
    OneWire_WriteByte(OneWire, ONEWIRE_CMD_SKIPROM);            /* ���� ROM�����豸/��ѡ���� */
    OneWire_WriteByte(OneWire, DS18B20_CMD_CONVERTTEMP);        /* �����¶�ת�� */
    return present;                                              /* ������λ��־ */
}


/**
 * @brief      ��ȡ DS18B20 �¶ȣ���ȡ Scratchpad ������Ϊ���϶ȣ�
 * @param      OneWire       OneWire ���߾��ָ��
 * @param      destination   ����¶�ָ�루��λ���棩
 * @retval     true          �¶ȶ�ȡ��У��ɹ���*destination д����Чֵ
 * @retval     false         ת��δ��ɡ�CRC У��ʧ�ܻ�������ȡʧ��
 *
 * ����˵����
 * 1) ͨ����һ�� time slot �ж�ת���Ƿ���ɣ������ͷ�Ϊ 1 ��ʾ��ɣ���
 * 2) ��λ �� SKIP ROM �� �� SCRATCHPAD��9 �ֽڣ���
 * 3) ����ǰ 8 �ֽڵ� CRC ����� 9 �ֽڱȶԣ�ʧ���򷵻� false��
 * 4) ����ԭʼ�¶ȣ������ֽڣ��������üĴ��������ֱ��ʣ�9~12bit�����������϶ȡ�
 * 5) �����£�ԭʼֵΪ���룩���ɹ���д�� *destination ������ true��
 *
 * ��ע��
 * - ��ʹ�ü�����Դ������ת��������ת���ڼ��ṩǿ�������˺��������ڵ���ǰ�����ת��������
 */
bool DS18B20_Read(OneWire_t *OneWire, float *destination)
{
    uint16_t temperature;                                                   /* ԭʼ�¶ȼĴ���ֵ */
    uint8_t  resolution;                                                    /* �������ֱ��� 9~12bit */
    int8_t   digit, minus = 0;                                              /* ������������ű�־ */
    float    decimal = 0.0f;                                                /* С������ */
    uint8_t  i = 0;                                                         /* ѭ������ */
    uint8_t  data[9];                                                       /* Scratchpad ������ */
    uint8_t  crc;                                                           /* ����õ��� CRC */

    if (!OneWire_ReadBit(OneWire))                                          /* ת����ɣ�1=��� */
    {
        return false;                                                       /* δ�����ֱ�ӷ��� */
    }

    OneWire_Reset(OneWire);                                                 /* ��λ���� */
    OneWire_WriteByte(OneWire, ONEWIRE_CMD_SKIPROM);                        /* ���豸������ ROM */
    OneWire_WriteByte(OneWire, ONEWIRE_CMD_RSCRATCHPAD);                    /* ��ȡ Scratchpad */

    for (i = 0; i < 9; i++)                                                 /* ��ȡ 9 �ֽ� */
    {
        data[i] = OneWire_ReadByte(OneWire);                                /* ���ֽڶ�ȡ */
    }

    crc = OneWire_CRC8(data, 8);                                            /* ����ǰ 8 �ֽ� CRC */
    if (crc != data[8])                                                     /* У��ʧ�ܣ� */
    {
        return false;                                                       /* CRC ��Ч */
    }

    temperature = (uint16_t)(data[0] | (data[1] << 8));                     /* �¶ȵ�/���ֽ� */
    OneWire_Reset(OneWire);                                                 /* ���긴λ���ͷ����� */

    if (temperature & 0x8000)                                               /* ���λ=1 Ϊ���� */
    {
        temperature = (uint16_t)(~temperature + 1U);                        /* ����ת��ֵ */
        minus = 1;                                                          /* ��¼���� */
    }

    resolution = (uint8_t)(((data[4] & 0x60) >> 5) + 9U);                   /* �����ֱ��� */

    digit  = (int8_t)(temperature >> 4);                                    /* �������֣���λ */
    digit |= (int8_t)(((temperature >> 8) & 0x07) << 4);                    /* �������֣����� */

    switch (resolution)                                                     /* С�����ֱ��ʼ��� */
    {
        case 9:
            decimal = (float)((temperature >> 3) & 0x01) * (float)DS18B20_DECIMAL_STEPS_9BIT;
            break;
        case 10:
            decimal = (float)((temperature >> 2) & 0x03) * (float)DS18B20_DECIMAL_STEPS_10BIT;
            break;
        case 11:
            decimal = (float)((temperature >> 1) & 0x07) * (float)DS18B20_DECIMAL_STEPS_11BIT;
            break;
        case 12:
            decimal = (float)(temperature & 0x0F) * (float)DS18B20_DECIMAL_STEPS_12BIT;
            break;
        default:
            return false;                                                   /* �쳣�ֱ��� */
    }

    decimal = (float)digit + decimal;                                       /* �ϳ�С��+���� */
    if (minus)                                                              /* ���´��� */
    {
        decimal = -decimal;
    }

    *destination = decimal;                                                 /* д���¶�ֵ */
    return true;                                                            /* �ɹ� */
}

/**
 * @brief      ��ȡ DS18B20 �ķֱ������ã�9/10/11/12 λ��
 * @param      OneWire   OneWire ���߾��ָ��
 * @retval     9/10/11/12    ��ǰ�ֱ���λ��
 * @retval     0             ��ȡʧ�ܣ����豸����Ӧ��
 *
 * ˵����
 * - �ֱ���λλ�����üĴ�����Scratchpad �� 5 �ֽڣ����� data[4]���� R1��R0 λ��bit6��bit5����
 * - ������ͨ�� SKIP ROM ֱ�ӷ��������ϵĵ���������ѡ���ģ�������
 */
uint8_t DS18B20_GetResolution(OneWire_t *OneWire)
{
    uint8_t conf;                                                           /* ���üĴ��������ֱ���λ�� */
                                                                            
    if (!OneWire_Reset(OneWire))                                            /* ��λ�������λ��0 ��ʾ����Ӧ */
    {                                                                       
        return 0;                                                           /* �豸����λ������ʧ�� */
    }                                                                       
                                                                            
    OneWire_WriteByte(OneWire, ONEWIRE_CMD_SKIPROM);                        /* ���� ROM�����豸/��ѡ���� */
    OneWire_WriteByte(OneWire, ONEWIRE_CMD_RSCRATCHPAD);                    /* ��ȡ Scratchpad ���� */
                                                                            
    (void)OneWire_ReadByte(OneWire);                                        /* �����¶� LSB */
    (void)OneWire_ReadByte(OneWire);                                        /* �����¶� MSB */
    (void)OneWire_ReadByte(OneWire);                                        /* �������±�����ֵ TH */
    (void)OneWire_ReadByte(OneWire);                                        /* �������±�����ֵ TL */
                                                                            
    conf = OneWire_ReadByte(OneWire);                                       /* ��ȡ���üĴ��������� R1/R0�� */
                                                                            
    /* ��ѡ�������λ���ͷ����ߣ����ϲ���������Ҫ�󣬿��Ƴ� */               
    OneWire_Reset(OneWire);                                                 /* �ͷ����� */
                                                                            
    return (uint8_t)(((conf & 0x60u) >> 5) + 9u);                           /* (R1:R0)��[00..11] ӳ��Ϊ 9..12 λ */
}                                                                           


/**
 * @brief      ���� DS18B20 ���¶ȷֱ��ʣ�9/10/11/12 λ��
 * @param      OneWire        OneWire ���߾��ָ��
 * @param      resolution     Ŀ��ֱ��ʣ�DS18B20_Resolution_t��
 * @retval     1              ���óɹ��������Ƶ��ڲ� EEPROM ��ɣ�
 * @retval     0              �豸����Ӧ�����ʧ��
 *
 * ���̣�
 * 1) �� Scratchpad �Ի�ȡ��ǰ TH��TL �����üĴ�����ֻ�޸� R1/R0 ��λ����������λ���䣩��
 * 2) д�� TH��TL�����üĴ����� Scratchpad��
 * 3) ִ�� Copy Scratchpad ������д���ڲ� EEPROM������ ��10ms������Ϊ������Դ��ǿ������
 *
 * ��ʾ��
 * - ���������ж���豸������������ Match ROM���˴�ʹ�� SKIP ROM �������ڵ��豸����ѡ���豸��
 * - Copy Scratchpad �ڼ䣬��������������ֱ����ɣ�����ѯ�������ͷ����ж���ɡ�
 */
uint8_t DS18B20_SetResolution(OneWire_t *OneWire, DS18B20_Resolution_t resolution)
{
    uint8_t th, tl, conf;

    if (!OneWire_Reset(OneWire))                                                /* ��λ�����λ */
    {
        return 0;
    }
    OneWire_WriteByte(OneWire, ONEWIRE_CMD_SKIPROM);                            /* ���� ROM */
    OneWire_WriteByte(OneWire, ONEWIRE_CMD_RSCRATCHPAD);                        /* �� Scratchpad */

    (void)OneWire_ReadByte(OneWire);                                            /* ���� Temp LSB */
    (void)OneWire_ReadByte(OneWire);                                            /* ���� Temp MSB */
    th   = OneWire_ReadByte(OneWire);                                           /* �� TH */
    tl   = OneWire_ReadByte(OneWire);                                           /* �� TL */
    conf = OneWire_ReadByte(OneWire);                                           /* �����üĴ��� */

    /* ���޸ķֱ���λ��R1/R0 �ֱ�Ϊ bit6/bit5��������λ���ֲ��� */
    switch (resolution)
    {
        case DS18B20_Resolution_9bits:
            conf &= (uint8_t)~(1u << DS18B20_RESOLUTION_R1);
            conf &= (uint8_t)~(1u << DS18B20_RESOLUTION_R0);
            break;
        case DS18B20_Resolution_10bits:
            conf &= (uint8_t)~(1u << DS18B20_RESOLUTION_R1);
            conf |=  (uint8_t)(1u << DS18B20_RESOLUTION_R0);
            break;
        case DS18B20_Resolution_11bits:
            conf |=  (uint8_t)(1u << DS18B20_RESOLUTION_R1);
            conf &= (uint8_t)~(1u << DS18B20_RESOLUTION_R0);
            break;
        case DS18B20_Resolution_12bits:
            conf |=  (uint8_t)(1u << DS18B20_RESOLUTION_R1);
            conf |=  (uint8_t)(1u << DS18B20_RESOLUTION_R0);
            break;
        default:
            return 0;                                                           /* �Ƿ���� */
    }

    if (!OneWire_Reset(OneWire))                                                /* �ٴθ�λ */
    {
        return 0;
    }
    OneWire_WriteByte(OneWire, ONEWIRE_CMD_SKIPROM);                            /* ���� ROM */
    OneWire_WriteByte(OneWire, ONEWIRE_CMD_WSCRATCHPAD);                        /* д Scratchpad */

    OneWire_WriteByte(OneWire, th);                                             /* д TH */
    OneWire_WriteByte(OneWire, tl);                                             /* д TL */
    OneWire_WriteByte(OneWire, conf);                                           /* д���üĴ��� */

    if (!OneWire_Reset(OneWire))                                                /* ��λ���뿽�� */
    {
        return 0;
    }
    OneWire_WriteByte(OneWire, ONEWIRE_CMD_SKIPROM);                            /* ���� ROM */
    OneWire_WriteByte(OneWire, ONEWIRE_CMD_CPYSCRATCHPAD);                      /* ������ EEPROM */

    /* ���� ��10ms��ͨ����ȡһ�� time slot ��ѯ��ɣ�1=��ɣ�0=æ�� */
    {
        /* ����ѯ�����ȴ� ~15ms */
        const uint32_t max_us = 15000u;                                         /* ��ȴ� 15ms */
        const uint32_t step_us = 500u;                                          /* ��ѯ���� 0.5ms */
        uint32_t waited = 0;

        while (!OneWire_ReadBit(OneWire))                                       /* �͵�ƽ=æ */
        {
            delay_us(step_us);
            waited += step_us;
            if (waited >= max_us)
            {
                return 0;                                                       /* ��ʱʧ�� */
            }
        }
    }

    return 1;
}


uint8_t DS18B20_AllDone(OneWire_t* OneWire)
{
    /* If read bit is low, then device is not finished yet with calculation temperature */
    return OneWire_ReadBit(OneWire);
}


