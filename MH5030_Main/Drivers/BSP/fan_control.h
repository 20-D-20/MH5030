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
  * @brief  风扇状态结构体
  */
typedef struct {
    uint16_t rpm;           // 当前转速
    uint8_t  duty_cycle;    // 占空比
    uint8_t  fault;         // 故障标志
    uint8_t  enable;        // 使能状态
} FanStatus_t;

/* Exported constants --------------------------------------------------------*/
#define FAN_MIN_RPM         500     // 最小转速
#define FAN_MAX_RPM         4000    // 最大转速
#define FAN_DEFAULT_RPM     2000    // 默认转速

/* Exported functions prototypes ---------------------------------------------*/
// 初始化函数
void Fan_Init(void);

// 基本控制函数
void Fan_Start(void);
void Fan_Stop(void);
void Fan_SetDutyCycle(uint8_t duty);
//void Fan_SetTargetRPM(uint16_t rpm);

// 状态获取函数
uint16_t Fan_GetRPM(void);
uint8_t  Fan_GetDutyCycle(void);
uint8_t  Fan_GetFaultStatus(void);


// 更新函数 (主循环调用)
void Fan_Update(void);

// 调试函数
void Fan_PrintStatus(void);

#ifdef __cplusplus
}
#endif

#endif /* __FAN_CONTROL_H */

