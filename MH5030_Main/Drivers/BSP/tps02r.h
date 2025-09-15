#ifndef __TPS02R_H
#define __TPS02R_H

#include "main.h"
#include "I2c.h"
//  
// ����ȫ��IIC����ṹ��ͳ�ʼ�����ýṹ��
extern IICManager_ST        tps02r_iicmanger;

/**             
 * \broef �豸��ַ
 * TPS02R_I2C_ADR_W:(0x48 <<1) | 0
 * TPS02R_I2C_ADR_W:(0x48 <<1) | 1
 */  
#define TPS02R_I2C_ADR_W     0x90                           //�豸д��ַ
#define TPS02R_I2C_ADR_R     0x91                           //�豸����ַ

#define TPS02R_FUN_OK        0                              //TPS02R������ؽӿڲ����ɹ�
#define TPS02R_FUN_ERROR    -1                              //TPS02R������ؽӿڲ���ʧ��
#define TPS02R_VALUE_ERROR  -2                              //TPS02R�����Ĵ����������
                                    
/**                
 * \broef TPS02R�豸ͨ��
 */                
#define TPS02R_CHAN1         0                              //TPS02R�豸ͨ��1
#define TPS02R_CHAN2         1                              //TPS02R�豸ͨ��2

/**
 * \broef TPS02Rѡ��ʹ��ͨ��
 */
#define TPS02R_CFG_CHAN_EN1_EN2        0x00                 //TPS02Rѡ��ʹ��ͨ��1,ʹ��ͨ��2
#define TPS02R_CFG_CHAN_EN1_DISEN2     0x02                 //TPS02Rѡ��ʹ��ͨ��1,����ͨ��2
#define TPS02R_CFG_CHAN_DISEN1_DISEN2  0x03                 //TPS02Rѡ�����ͨ��1,����ͨ��2
#define TPS02R_CFG_CHAN_DISEN1_EN2     0x01                 //TPS02Rѡ�����ͨ��1,ʹ��ͨ��2
                
/**             
 * \broef �ӼĴ�����ַ                
 */             
#define TPS02R_TEMP_ADDR (0X00)    				            //�¶��ӼĴ�����ַ
#define TPS02R_TEMP_LEN  (0X06)    				            //�¶��ӼĴ�������
        
#define TPS02R_CFG_ADDR (0X01)     				            //�����ӼĴ�����ַ
#define TPS02R_CFG_LEN  (0X02)     				            //�����ӼĴ�������
        
#define TPS02R_TLOW_ADDR (0X02)    				            //�¶������ӼĴ�����ַ
#define TPS02R_TLOW_LEN  (0X06)    				            //�¶������ӼĴ�������
        
#define TPS02R_THIG_ADDR (0X03)    				            //�¶������ӼĴ�����ַ
#define TPS02R_THIG_LEN  (0X06)    				            //�¶������ӼĴ�������

        
/**             
 * \broef ������غ궨��               
 */             
#define TPS02R_SAMPLE_RATE_10  (0X00)   		            //���ò�������10 bps
#define TPS02R_SAMPLE_RATE_40  (0X01)   		            //���ò�������40 bps
        
#define TPS02R_CFG_RATE_MASK  (0X20)    		            //���ò���������
#define TPS02R_CFG_RATE_SHIFT (0X05)    		            //���ò�������λ
                
#define TPS02R_CFG_TRIGGER_WARNING_MASK  (0X18)             //���ò���������
#define TPS02R_CFG_TRIGGER_WARNING_SHIFT (0X03)             //���ò�������λ

#define TPS02R_CFG_EN_MASK  (0X80)                          //����ʹ������
#define TPS02R_CFG_EN_SHIFT (0X07)                          //����ʹ����λ
                
#define TPS02R_CFG_ALARM_MODE_MASK  (0X06)                  //���þ���ģʽ����
#define TPS02R_ALARM_MODE_0  		(0x00)                  //ģʽ0 �Ƚ�ģʽ ALERT����ʱ����ߵ�ƽ
#define TPS02R_ALARM_MODE_1  		(0x02)                  //ģʽ1 �ж�ģʽ ALERT����ʱ����ߵ�ƽ
#define TPS02R_ALARM_MODE_2  		(0x04)                  //ģʽ2 �Ƚ�ģʽ ALERT����ʱ����͵�ƽ
#define TPS02R_ALARM_MODE_3  		(0x06)                  //ģʽ3 �ж�ģʽ ALERT����ʱ����͵�ƽ


void tps02r_iic_init(IICManager_ST *pstIIC);

/**
 * \brief ��tps02rģ������ó�ʼ��ΪĬ��ֵ
 * 
 * \note Ĭ��ѡ�����üĴ���ͨ��1�Ĳ�����������Ϊ10 bps������ALERT����źŵ��¶�ֵ���Ը���Ϊ6��
 * \note �����ź����Ϊ�Ƚ�ģʽ��ALERT�ĳ�ʼ���״̬Ϊ�͵�ƽ
 * \note Ĭ���¶�����ֵΪ1023.999878�棬Ĭ���¶�����ֵΪ-0.000122��
 * 
 * \param[in] p_dev tps02rģ���豸
 * 
 * \retval TPS02R_FUN_OK       ���óɹ�
 * \retval TPS02R_FUN_ERROR    ����ʧ��
 * \retval TPS02R_VALUE_ERROR  ��������
 */
int tps02r_cfg_init(void);

/**
 * \brief ��ȡ�Ĵ���ͨ�����¶�ֵ
 * 
 * \note ���Ի�ȡ��ǰģ��ɼ������¶�ֵ��TPS02R_TEMP_ADDR��
 * \note Ҳ���Ի�ȡ���Ѿ����õ��¶����ޣ�TPS02R_THIG_ADDR�������ޣ�TPS02R_TLOW_ADDR��
 * 
 * \param[in] chan ͨ�� ��ѡTPS02R_CHAN1 TPS02R_CHAN2
 * \param[out] p_data ����¶����ݵĻ�����
 * 
 * \retval TPS02R_FUN_OK       ���óɹ�
 * \retval TPS02R_FUN_ERROR    ����ʧ��
 * \retval TPS02R_VALUE_ERROR  ��������
 */
int tps02r_get_temp(int8_t chan, float *p_data);

/**
 * \brief ����ͨ������ֵ
 * 
 * \param[in] chan ͨ�� ��ѡTPS02R_CHAN1 TPS02R_CHAN2
 * \param[in] temp ��Ҫ���õ������¶�ֵ
 * 
 * \retval TPS02R_FUN_OK       ���óɹ�
 * \retval TPS02R_FUN_ERROR    ����ʧ��
 * \retval TPS02R_VALUE_ERROR  ��������
 */
int tps02r_set_low(int8_t chan, float temp);


/**
 * \brief ����ͨ������ֵ
 * 
 * \param[in] chan ͨ�� ��ѡTPS02R_CHAN1 TPS02R_CHAN2
 * \param[in] temp ��Ҫ���õ������¶�ֵ
 * 
 * \retval TPS02R_FUN_OK       ���óɹ�
 * \retval TPS02R_FUN_ERROR    ����ʧ��
 * \retval TPS02R_VALUE_ERROR  ��������
 */
int tps02r_set_high( int8_t chan, float temp);

/**
 * \brief ����ͨ����������
 * 
 * \param[in] chan_en ��Ҫѡ���ʹ��ͨ�������벻ͬ�ĺ���޸ļĴ���λ�Ĳ�������ѡ�����£�
 *                    �޸����üĴ����ֽ�1(ʵ���޸�)�����ʲ�����TPS02R_CFG_CHAN_EN1_EN2��TPS02R_CFG_CHAN_EN1_DISEN2��TPS02R_CFG_CHAN_DISEN1_DISEN2
 *                    �޸����üĴ����ֽ�2(ʵ���޸�)�����ʲ�����TPS02R_CFG_CHAN_DISEN1_EN2
 * 
 * \param[in] rate    �ٶ�ֵ ��ѡTPS02R_SAMPLE_RATE_10��TPS02R_SAMPLE_RATE_40
 * 
 * \retval TPS02R_FUN_OK    ���óɹ�
 * \retval TPS02R_FUN_ERROR ����ʧ��
 * \retval TPS02R_VALUE_ERROR  ��������
 */
int tps02r_set_sampling_rate(int8_t chan_en, uint32_t rate);

/**
 * \brief ��ȡ���üĴ���ͨ������
 * 
 * \param[in] p_dev tps02r�豸
 * \param[in] chan ͨ����ͨ��1��Ӧ���üĴ����ֽ�1�����ݣ�ͨ��2��Ӧ���üĴ����ֽ�2�����ݣ� ��ѡTPS02R_CHAN1 TPS02R_CHAN2
 * \param[out] p_data ͨ���������ݵĻ�����
 *  
 * \retval TPS02R_FUN_OK    ���óɹ�
 * \retval TPS02R_FUN_ERROR ����ʧ��
 * \retval TPS02R_VALUE_ERROR  ��������
 */
int tps02r_get_cfg_value(int8_t chan, uint8_t *p_data);

#endif 
/** end of file **/
