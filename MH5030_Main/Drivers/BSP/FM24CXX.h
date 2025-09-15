#ifndef	_FM24CXX_H
#define	_FM24CXX_H

#include "main.h" 
#include "I2c.h"
#include "delay.h"

extern IICManager_ST   fm24cxx_iicmanager;

#define AT24C01		127
#define AT24C02		255
#define AT24C04		511
#define AT24C08		1023
#define AT24C16		2047                    
#define AT24C32		4095
#define AT24C64	    8191
#define AT24C128	16383
#define AT24C256	32767

/**
 * @brief  EEPROM���Ͷ���
 * @note   ��ǰʹ��AT24C16оƬ
 */
#define EE_TYPE     AT24C16

/**
 * @brief  ������豸��ַƫ����
 * @param  addr �洢����ַ
 * @retval ���ؼ����ĵ�ַƫ����
 */
#define SLAVEADDR_OFFSET(addr)      ((addr >> 8) << 1)

#define FRAM_READ   0xA1             /* I2C��ȡ���� */
#define FRAM_WRITE  0xA0             /* I2Cд������ */

/**
 * @brief  EEPROM��ʼ������ֽ�
 * @note   ���ڼ��EEPROM�Ƿ���������
 */
#define TEST_BYTE   0x55             /* FM24CXX��ʼ������־λ */

/**
 * @brief  д�������ƺ�
 * @param  pinstate ����״̬ (0:����д����, 1:����д����)
 */
#define EEPROM_WP(pinstate)         HAL_GPIO_WritePin(FM24CL16_WP_GPIO_Port, FM24CL16_WP_Pin, (GPIO_PinState)(pinstate))


/**
 * @brief  I2C��������ʼ��
 * @param  pstIIC IIC�������ṹ��ָ��
 */
void FM24CXX_iic_init(IICManager_ST *pstIIC);

/**
 * @brief  ����д�����ֽ�����
 * @param  addr ��ʼ��ַ (���ֵ2047����Ӧ16K bits����)
 * @param  ptr  ���ݻ�����ָ��
 * @param  size Ҫд������ݴ�С
 * @retval ���ز���״̬ (0:�ɹ�, ����:������)
 * @note   ʹ�ö��ֽ�д��ģʽ (Multiple Byte Write Model)
 */
u8 FM_WriteByteseq(u16 addr, void *ptr, u16 size);

/**
 * @brief  ��ָ����ַ������ȡ����ֽ�����
 * @param  addr ��ʼ��ַ (���ֵ2047����Ӧ16K bits����)
 * @param  ptr  ���ݻ�����ָ��
 * @param  size Ҫ��ȡ�����ݴ�С
 * @retval ���ز���״̬ (0:�ɹ�, ����:������)
 * @note   ʹ��ѡ��(���)��ȡģʽ (Selective (Random) Read)
 */
u8 FM_ReadByteseq(u16 addr, void *ptr, u16 size);

/**
 * @brief  ���FM24CXXоƬ�Ƿ���������
 * @retval ���ؼ���� (0:����, ����:�쳣)
 */
/**
 * @brief      ��� EEPROM �Ƿ��ѳ�ʼ��
 * @retval     0   ��ʾ EEPROM �ѳ�ʼ��
 *             1   ��ʾ EEPROM δ��ʼ��
 * 
 * �ú���ͨ����ȡ EEPROM �е��ض�λ�ã�`EE_TYPE`�����ж� EEPROM �Ƿ��Ѿ���ʼ����
 * �����ȡ�����ֽ�ΪԤ����� `TEST_BYTE`�����ʾ EEPROM �Ѿ���ʼ�������� 0��
 * �����ȡ�����ֽڲ��� `TEST_BYTE`����˵�� EEPROM δ��ʼ����������д�� `TEST_BYTE`�������¶�ȡ��λ��ȷ�ϳ�ʼ���ɹ���
 * �����ʼ���ɹ������� 0�����򷵻� 1��
 */
u8 FM_Check(void);

/**
 * @brief      ��ȡָ����ַ��һ���ֽ�����
 * @param      ReadAddr   ��ȡ�ĵ�ַ
 * @retval     ���ض�ȡ���ֽ�����
 * 
 * �ú���ͨ�� I2C ���ߴ� EEPROM ��ȡָ����ַ��һ���ֽ����ݡ���ȡ���̰�������ͨѶ�����͵�ַ���������ݡ�
 * ��ȡ��ɺ󣬷��ض�ȡ���ֽ����ݡ�
 */
u8 FM_ReadOneByte(u16 ReadAddr);

/**
 * @brief      ��ָ����ַд��һ���ֽ�����
 * @param      WriteAddr  д��ĵ�ַ
 * @param      DataToWrite   Ҫд��������ֽ�
 * @retval     ��
 * 
 * �ú���ͨ�� I2C ���߽�һ���ֽڵ�����д��ָ���� EEPROM ��ַ��д����̰�������ͨѶ�����͵�ַ��Ϣ��д�����ݲ�ֹͣͨѶ��
 * д����ɺ󣬺������أ��� EEPROM ���ݱ��ɹ�д�롣
 */
void FM_WriteOneByte(u16 WriteAddr, u8 DataToWrite);

//#define PARAMETER_WRITE(addr, pbuffer)		FM_WriteByteseq((u16)(addr), &pbuffer, sizeof(pbuffer))
//#define PARAMETER_READ(addr, pbuffer)		FM_ReadByteseq((u16)(addr), &pbuffer, sizeof(pbuffer))
//#define fw24_storage_compare(u32Addr,Para)  storage_compare(u32Addr,&Para,sizeof(Para),FM_WriteByteseq,FM_ReadByteseq)

#endif

