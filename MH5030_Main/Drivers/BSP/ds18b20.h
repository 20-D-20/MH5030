#ifndef _DS18B20_H
#define _DS18B20_H

#include "onewire.h"
#include <stdbool.h>
#include "delay.h"

/**
 * @brief      DS18B20 �豸���
 *
 * ���浥�� DS18B20 �豸�� 64-bit ROM ��ַ�����һ�ζ�ȡ���¶��Լ�������Ч��־��
 */
typedef struct
{
    uint8_t Address[8];   /* 64-bit ROM ��ַ��LSB ��ǰ */
    float   Temperature;  /* ���һ�ζ�ȡ���¶�ֵ���棩 */
    bool    DataIsValid;  /* ���һ�ζ�ȡ�������Ƿ���Ч */
} Ds18b20Sensor_t;

/* -------------------------------------------------------------------------------------- */
/* �ⲿ�������                                                                            */
/* -------------------------------------------------------------------------------------- */

/**
 * @brief      �ⲿ DS18B20 �豸�����ʾ������·��
 */
extern Ds18b20Sensor_t ds18b20_1;
//extern Ds18b20Sensor_t ds18b20_2;

/**
 * @brief      �ⲿ OneWire ���߾����ʾ������·��
 */
extern OneWire_t OneWire_1;
//extern OneWire_t OneWire_2;

/* -------------------------------------------------------------------------------------- */
/* �����������                                                                          */
/* -------------------------------------------------------------------------------------- */

/**
 * @brief      DS18B20 �����루ROM �� 1 �ֽڣ�
 */
#define DS18B20_FAMILY_CODE                     0x28

/**
 * @brief      ������������
 */
#define DS18B20_CMD_ALARMSEARCH                 0xEC

/**
 * @brief      �����¶�ת�����Convert T��
 */
#define DS18B20_CMD_CONVERTTEMP                 0x44  /* Convert temperature */

/* �ֱ����µ�С���������棩 */
#define DS18B20_DECIMAL_STEPS_12BIT             0.0625f
#define DS18B20_DECIMAL_STEPS_11BIT             0.125f
#define DS18B20_DECIMAL_STEPS_10BIT             0.25f
#define DS18B20_DECIMAL_STEPS_9BIT              0.5f

/**
 * @brief      ת����ɵȴ��ĳ�ʱ����ѯ�����΢�룩
 *
 * 12 λ�ֱ��ʵ���ת��ʱ�� 750ms���˴�ȡ 1s ��Ϊԣ������ѯ��� 1ms��
 */
#define DS18B20_CONVERT_TIMEOUT_US              (1000000u)   /* ���ȴ� 1 s */
#define DS18B20_POLL_INTERVAL_US                (1000u)      /* ÿ 1 ms ��ѯһ�� */

/* ���üĴ����зֱ���λλ�ã�R1��R0�� */
#define DS18B20_RESOLUTION_R1                   6
#define DS18B20_RESOLUTION_R0                   5

/* �����ȣ����� CRC ʱΪ 9 �ֽڣ����� 2 �ֽڣ����¶ȼĴ����� */
#ifdef DS18B20_USE_CRC
#define DS18B20_DATA_LEN                        9
#else
#define DS18B20_DATA_LEN                        2
#endif

/* -------------------------------------------------------------------------------------- */
/* �ֱ���ö��                                                                              */
/* -------------------------------------------------------------------------------------- */

/**
 * @brief      DS18B20 �ֱ�������
 */
typedef enum
{
    DS18B20_Resolution_9bits  = 9,   /*!< 9  λ�ֱ��ʣ����� 0.5 �� */
    DS18B20_Resolution_10bits = 10,  /*!< 10 λ�ֱ��ʣ����� 0.25 �� */
    DS18B20_Resolution_11bits = 11,  /*!< 11 λ�ֱ��ʣ����� 0.125 �� */
    DS18B20_Resolution_12bits = 12   /*!< 12 λ�ֱ��ʣ����� 0.0625 �� */
} DS18B20_Resolution_t;

/* -------------------------------------------------------------------------------------- */
/* API ������Doxygen ���ͷ��ע�ͣ�                                                        */
/* -------------------------------------------------------------------------------------- */

/**
 * @brief      �ֶ���������ȡһ���¶ȣ����豸/��ѡ����
 * @param      ds18b20    Ŀ�괫�������ָ�루�ڲ���д Temperature��DataIsValid��
 * @param      OneWire    OneWire ���߾��ָ��
 * @retval     true       ת������Ҷ�ȡ�ɹ���Temperature Ϊ��Ч���϶�
 * @retval     false      ��ʱ���ȡʧ�ܣ�DataIsValid ��Ϊ false
 */
bool Ds18b20_ManualConvert(Ds18b20Sensor_t *ds18b20, OneWire_t *OneWire);

/**
 * @brief      �����¶�ת����Convert T��
 * @param      OneWireStruct   OneWire ���߾��ָ��
 * @retval     1               �豸��λ����λ����Ӧ�������·�ת������
 * @retval     0               �豸����Ӧ��δ��λ��
 */
uint8_t DS18B20_Start(OneWire_t *OneWireStruct);

/**
 * @brief      ��ȡ�¶ȣ���ȡ Scratchpad ������Ϊ���϶ȣ�
 * @param      OneWireStruct   OneWire ���߾��ָ��
 * @param      destination     ����¶�ָ�루��λ���棩
 * @retval     true            ��ȡ�ɹ��� CRC У��ͨ���������� CRC��
 * @retval     false           ת��δ��ɻ� CRC У��ʧ��/��ȡ����
 */
bool DS18B20_Read(OneWire_t *OneWireStruct, float *destination);

/**
 * @brief      ��ȡ�豸�ֱ���
 * @param      OneWireStruct   OneWire ���߾��ָ��
 * @retval     9/10/11/12      ��ǰ�ֱ���λ��Ӧ��λ��
 * @retval     0               ��ȡʧ��
 */
uint8_t DS18B20_GetResolution(OneWire_t *OneWireStruct);

/**
 * @brief      �����豸�ֱ���
 * @param      OneWireStruct   OneWire ���߾��ָ��
 * @param      resolution      Ŀ��ֱ��ʣ�9/10/11/12 λ��
 * @retval     1               ���óɹ���д�� EEPROM����ʵ�֣�
 * @retval     0               ����ʧ��
 */
uint8_t DS18B20_SetResolution(OneWire_t *OneWireStruct, DS18B20_Resolution_t resolution);

/**
 * @brief      �ж�ת���Ƿ���ɣ������ͷż���ɣ�
 * @param      OneWireStruct   OneWire ���߾��ָ��
 * @retval     1               ת����ɣ��ɶ�ȡ��
 * @retval     0               ת��δ���
 */
uint8_t DS18B20_AllDone(OneWire_t *OneWireStruct);

#endif /* _DS18B20_H */
