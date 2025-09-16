#ifndef __FAN_CONTROL_H
#define __FAN_CONTROL_H

#include "tim.h"
#include "delay.h"
#include "fan_filter.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Exported types ------------------------------------------------------------*/

/**
  * @brief  ����״̬�ṹ��
  */
typedef struct {
    uint16_t rpm;           // ��ǰת��
    uint8_t  duty_cycle;    // ռ�ձ�
    uint8_t  fault;         // ���ϱ�־
    uint8_t  enable;        // ʹ��״̬
} FanStatus_t;

/* Exported constants --------------------------------------------------------*/
#define FAN_MIN_RPM         500     // ��Сת��
#define FAN_MAX_RPM         4000    // ���ת��
#define FAN_DEFAULT_RPM     2000    // Ĭ��ת��

/* Exported functions prototypes ---------------------------------------------*/
// ��ʼ������
void Fan_Init(void);

// �������ƺ���
void Fan_Start(void);
void Fan_Stop(void);
void Fan_SetDutyCycle(uint8_t duty);
//void Fan_SetTargetRPM(uint16_t rpm);

// ״̬��ȡ����
uint16_t Fan_GetRPM(void);
uint8_t  Fan_GetDutyCycle(void);
uint8_t  Fan_GetFaultStatus(void);


// ���º��� (��ѭ������)
void Fan_Update(void);

// ���Ժ���
void Fan_PrintStatus(void);

#ifdef __cplusplus
}
#endif

#endif /* __FAN_CONTROL_H */

