#ifndef ONEWIRE_H
#define ONEWIRE_H 

#include "delay.h"
#include "tim.h"

/* C++ detection */
#ifdef __cplusplus
extern "C" {
#endif
#define	OneWireDelay(x)			delay_us(x)

typedef struct {
    GPIO_TypeDef* GPIOx;                            /*!< ���� I/O ���ܵ� GPIO �˿ڣ����� GPIOA, GPIOB �� */
    uint16_t GPIO_Pin;                              /*!< ���� I/O ���ܵ� GPIO ���ű�ţ����� GPIO_PIN_0, GPIO_PIN_1 �� */
    
    uint8_t LastDiscrepancy;                        /*!< ���������е�˽�б�������¼�ϴ�����ʱ�豸��ַ��������λ������ָ�������������� */
    uint8_t LastFamilyDiscrepancy;                  /*!< ���������е�˽�б�������¼�ϴ�����ʱ�豸��ͥ���ͣ�Family Code����������λ���������ֲ�ͬ���͵��豸 */
    uint8_t LastDeviceFlag;                         /*!< ���������е�˽�б�������־�Ƿ��Ѿ��ҵ��������豸���������豸���ҵ�ʱ�ñ�־Ϊ 1 */
    
    uint8_t ROM_NO[8];                              /*!< �洢����ҵ��豸�� ROM ��ַ��8 �ֽڣ��������豸��Ψһ��ʶ�� */
} OneWire_t;

/* Pin settings */
void ONEWIRE_LOW(OneWire_t *gp);			
void ONEWIRE_HIGH(OneWire_t *gp);		
void ONEWIRE_INPUT(OneWire_t *gp);		
void ONEWIRE_OUTPUT(OneWire_t *gp);		

#define ONEWIRE_CMD_RSCRATCHPAD           0xBE      /* ��ȡ Scratchpad �ڴ� */
#define ONEWIRE_CMD_WSCRATCHPAD           0x4E      /* д�� Scratchpad �ڴ� */
#define ONEWIRE_CMD_CPYSCRATCHPAD         0x48      /* �� Scratchpad ���ݸ��Ƶ� EEPROM */
#define ONEWIRE_CMD_RECEEPROM             0xB8      /* ��ȡ EEPROM ���� */
#define ONEWIRE_CMD_RPWRSUPPLY            0xB4      /* ��ȡ����ƴ������ĵ�Դ��Ӧ״̬ */
#define ONEWIRE_CMD_SEARCHROM             0xF0      /* ���������ϵ������豸 */
#define ONEWIRE_CMD_READROM               0x33      /* ��ȡ ROM ��ַ */
#define ONEWIRE_CMD_MATCHROM              0x55      /* ��ȷƥ�� ROM ��ַ */
#define ONEWIRE_CMD_SKIPROM               0xCC      /* ���� ROM ��ַ�����ڵ��豸��㲥ģʽ��*/


//#######################################################################################################
void OneWire_Init(OneWire_t* OneWireStruct, GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
uint8_t OneWire_Reset(OneWire_t* OneWireStruct);
uint8_t OneWire_ReadByte(OneWire_t* OneWireStruct);
void OneWire_WriteByte(OneWire_t* OneWireStruct, uint8_t byte);
void OneWire_WriteBit(OneWire_t* OneWireStruct, uint8_t bit);
uint8_t OneWire_ReadBit(OneWire_t* OneWireStruct);
uint8_t OneWire_Search(OneWire_t* OneWireStruct, uint8_t command);
void OneWire_ResetSearch(OneWire_t* OneWireStruct);
uint8_t OneWire_First(OneWire_t* OneWireStruct);
uint8_t OneWire_Next(OneWire_t* OneWireStruct);
void OneWire_GetFullROM(OneWire_t* OneWireStruct, uint8_t *firstIndex);
void OneWire_Select(OneWire_t* OneWireStruct, uint8_t* addr);
void OneWire_SelectWithPointer(OneWire_t* OneWireStruct, uint8_t* ROM);
uint8_t OneWire_CRC8(uint8_t* addr, uint8_t len);
//#######################################################################################################
 
/* C++ detection */
#ifdef __cplusplus
}
#endif

#endif

