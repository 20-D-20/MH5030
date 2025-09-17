#ifndef __MAX6675_H
#define __MAX6675_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"
#include <stdbool.h>
#include <string.h>
#include "delay.h"
#include "main.h"

/* ���ò��� */
#define MAX6675_CHANNELS        1           /* ͨ������ */
#define MAX6675_FILTER_SIZE     4           /* �˲�����С */
#define MAX6675_CONVERSION_TIME 250         /* ת��ʱ��(ms) */

/* ͨ������ */
typedef enum {
    MAX6675_CH1 = 0,
    MAX6675_CH2 = 1,
    MAX6675_CH3 = 2,
    MAX6675_CH4 = 3
} MAX6675_Channel_e;

/* �����붨�� */
typedef enum {
    MAX6675_OK               = 0,
    MAX6675_ERROR_OPEN       = -1,
    MAX6675_ERROR_COMM       = -2,
    MAX6675_ERROR_RANGE      = -3,
    MAX6675_ERROR_INVALID_CH = -4,
    MAX6675_ERROR_TIMEOUT    = -5
} MAX6675_Error_e;

/* �����¶�ֵ */
#define TEMP_INVALID            999.0f
#define TEMP_MIN                0.0f
#define TEMP_MAX                1024.0f

/* GPIO���ýṹ�� */
typedef struct {
    GPIO_TypeDef* port;
    uint16_t pin;
} MAX6675_GPIO_t;

/* ͨ��״̬�ṹ�� */
typedef struct {
    float temperature;                      /* ��ǰ�¶� */
    float temp_buffer[MAX6675_FILTER_SIZE]; /* �˲������� */
    float temp_filter;                      /* �˲�����¶� */
    uint8_t buffer_index;                   /* ���������� */
    bool is_initialized;                    /* ��ʼ����־ */
    bool is_open;                           /* ��·��־ */
    bool comm_error;                        /* ͨ�Ŵ����־ */
    uint32_t last_read_tick;                /* �ϴζ�ȡʱ�� */
    uint32_t error_count;                   /* ������� */
} MAX6675_ChannelStatus_t;

/* MAX6675���ṹ�� */
typedef struct {
    /* GPIO���� */
    MAX6675_GPIO_t sck;                    /* ʱ������ */
    MAX6675_GPIO_t so;                     /* ����������� */
    MAX6675_GPIO_t cs[MAX6675_CHANNELS];   /* Ƭѡ�������� */
    
    /* ͨ��״̬ */
    MAX6675_ChannelStatus_t channels[MAX6675_CHANNELS];
    
    /* ���ò��� */
    bool use_filter;                       /* �Ƿ�ʹ���˲� */
    uint32_t read_timeout;                 /* ��ȡ��ʱʱ�� */
    
    /* ͳ����Ϣ */
    uint32_t total_reads;                  /* �ܶ�ȡ���� */
    uint32_t total_errors;                 /* �ܴ������ */
} MAX6675_Handle_t;

/* ��ʼ�����ýṹ�� */
typedef struct {
    /* SCK���� */
    GPIO_TypeDef* sck_port;
    uint16_t sck_pin;
    
    /* SO���� */
    GPIO_TypeDef* so_port;
    uint16_t so_pin;
    
    /* CS�������� */
    GPIO_TypeDef* cs_ports[MAX6675_CHANNELS];
    uint16_t cs_pins[MAX6675_CHANNELS];
    
    /* ����ѡ�� */
    bool enable_filter;
    uint32_t timeout_ms;
} MAX6675_Config_t;

/* ȫ�ֱ������� */

extern MAX6675_Handle_t g_stMax6675;


/* ������������ */
void    MAX6675_Setup(void);
void    Temperature_Monitor_Task(void);
bool    MAX6675_IsChannelReady(MAX6675_Handle_t* handle, uint8_t channel);
HAL_StatusTypeDef    MAX6675_Init(MAX6675_Handle_t* handle, const MAX6675_Config_t* config);
HAL_StatusTypeDef    MAX6675_DeInit(MAX6675_Handle_t* handle);
MAX6675_Error_e      MAX6675_ReadTemperature(MAX6675_Handle_t* handle, uint8_t channel, float* temperature);
MAX6675_Error_e      MAX6675_ReadTemperatureFiltered(MAX6675_Handle_t* handle, uint8_t channel, float* temperature);
HAL_StatusTypeDef    MAX6675_ReadAllChannels(MAX6675_Handle_t* handle, float* temperatures);
HAL_StatusTypeDef    MAX6675_GetChannelStatus(MAX6675_Handle_t* handle, uint8_t channel, MAX6675_ChannelStatus_t* status);
HAL_StatusTypeDef    MAX6675_ClearErrors(MAX6675_Handle_t* handle, uint8_t channel);

/* ���ߺ��� */
const char*     MAX6675_GetErrorString(MAX6675_Error_e error);
void    MAX6675_PrintStatus(MAX6675_Handle_t* handle);

#ifdef __cplusplus
}
#endif

#endif /* __MAX6675_H */

