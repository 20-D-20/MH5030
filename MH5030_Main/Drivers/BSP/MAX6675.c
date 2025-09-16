#include "max6675.h"
#include <stdio.h>

/* ȫ�ֱ������� */
MAX6675_Handle_t g_stMax6675;
float temperatures[MAX6675_CHANNELS];

/* ˽�к������� */
static void MAX6675_GPIO_Init(MAX6675_Handle_t* handle, const MAX6675_Config_t* config);
static void MAX6675_SetCS(MAX6675_Handle_t* handle, uint8_t channel, GPIO_PinState state);
static void MAX6675_SetSCK(MAX6675_Handle_t* handle, GPIO_PinState state);
static GPIO_PinState MAX6675_ReadSO(MAX6675_Handle_t* handle);
static uint16_t MAX6675_ReadRawData(MAX6675_Handle_t* handle, uint8_t channel);
static float MAX6675_RawToTemperature(uint16_t raw_data);
static float MAX6675_ApplyFilter(MAX6675_Handle_t* handle, uint8_t channel, float new_temp);
static void MAX6675_Delay_us(uint32_t us);

/**
* @brief  �¶ȼ������
* @note   �ú�����Ҫ���ڲ���
* @param  None
* @retval None
*/
void Temperature_Monitor_Task(void)
{
   MAX6675_Error_e result;
   float temp;
   static uint32_t cycle_count = 0;
   
   printf("--- Cycle %lu ---\r\n", ++cycle_count);
   
   /* ����1: ������ȡÿ��ͨ�� */
   for(uint8_t i = 0; i < MAX6675_CHANNELS; i++) 
   {
       /* ���ͨ���Ƿ�׼���� */
       if(!MAX6675_IsChannelReady(&g_stMax6675, i)) 
       {
           printf("CH%d: Not ready\r\n", i + 1);
           continue;
       }
       
       /* ��ȡ���˲����¶� */
       result = MAX6675_ReadTemperatureFiltered(&g_stMax6675, i, &temp);
       
       /* ���ݽ������ */
       switch(result) {
           case MAX6675_OK:
               printf("CH%d: %.2f ��C", i + 1, temp);
               
               /* �¶ȱ������ */
               if(temp > 100.0f) 
               {
                   printf(" [HIGH TEMP WARNING!]");
               } else if(temp < 10.0f) {
                   printf(" [LOW TEMP WARNING!]");
               }
               printf("\r\n");
               break;
               
           case MAX6675_ERROR_OPEN:
               printf("CH%d: Thermocouple OPEN!\r\n", i + 1);
               break;
               
           case MAX6675_ERROR_COMM:
               printf("CH%d: Communication ERROR!\r\n", i + 1);
               break;
               
           case MAX6675_ERROR_RANGE:
               printf("CH%d: Out of RANGE!\r\n", i + 1);
               break;
               
           case MAX6675_ERROR_TIMEOUT:
               printf("CH%d: Read TIMEOUT!\r\n", i + 1);
               break;
               
           default:
               printf("CH%d: Unknown error (%d)\r\n", i + 1, result);
               break;
       }
       
   }
   
   /* ÿ10�����ڴ�ӡһ��ͳ����Ϣ */
   if(cycle_count % 10 == 0) {
       printf("\r\n");
       MAX6675_PrintStatus(&g_stMax6675);
   }
}

/**
 * @brief  MAX6675��ʼ������
 * @param  None
 * @retval None
 */
 void MAX6675_Setup(void)
{
    MAX6675_Config_t config = {0};
    
    /* ����SCK���� - PA12 */
    config.sck_port = MAX6675_SCK_GPIO_Port;
    config.sck_pin = MAX6675_SCK_Pin;
    
    /* ����SO���� - PA10 */
    config.so_port = MAX6675_SO_A_GPIO_Port;
    config.so_pin = MAX6675_SO_A_Pin;
    
    /* ����CS���� -    PA11 */
    config.cs_ports[0] = MAX6675__CS_GPIO_Port;
    config.cs_pins[0] = MAX6675__CS_Pin; 
    
#if 0   /*����ʵ��Ӧ����Ҫȥ��������ͨ��*/
    config.cs_ports[1] = GPIOA;
    config.cs_pins[1] = GPIO_PIN_3;  /* CS2 - PA3 */
    
    config.cs_ports[2] = GPIOA;
    config.cs_pins[2] = GPIO_PIN_2;  /* CS3 - PA2 */
    
    config.cs_ports[3] = GPIOA;
    config.cs_pins[3] = GPIO_PIN_1;  /* CS4 - PA1 */
#endif

    /* ʹ���˲� */
    config.enable_filter = true;
    config.timeout_ms = 100;
    
    /* ��ʼ��MAX6675 */
    MAX6675_Init(&g_stMax6675, &config);

}

/**
 * @brief  ΢�뼶��ʱ����
 * @param  us: ��ʱ΢����
 * @retval None
 */
static void MAX6675_Delay_us(uint32_t us)
{
    uint32_t ticks = us * (SystemCoreClock / 1000000) / 5;
    while(ticks--) {
        __NOP();
    }
}

/**
 * @brief  ��ʼ��MAX6675
 * @param  handle: MAX6675���
 * @param  config: ���ò���
 * @retval HAL״̬
 */
HAL_StatusTypeDef MAX6675_Init(MAX6675_Handle_t* handle, const MAX6675_Config_t* config)
{
    if(handle == NULL || config == NULL) {
        return HAL_ERROR;
    }
    
    /* ��վ�� */
    memset(handle, 0, sizeof(MAX6675_Handle_t));
    
    /* �������� */
    handle->use_filter = config->enable_filter;
    handle->read_timeout = config->timeout_ms;
    
    /* ��ʼ��GPIO */
    MAX6675_GPIO_Init(handle, config);
    
    /* ��ʼ��ͨ��״̬ */
    for(uint8_t i = 0; i < MAX6675_CHANNELS; i++) 
    {
        handle->channels[i].temperature = 0.0f;
        handle->channels[i].buffer_index = 0;
        handle->channels[i].is_initialized = false;
        handle->channels[i].is_open = false;
        handle->channels[i].comm_error = false;
        handle->channels[i].last_read_tick = 0;
        handle->channels[i].error_count = 0;
        
        /* ����˲������� */
        for(uint8_t j = 0; j < MAX6675_FILTER_SIZE; j++) 
        {
            handle->channels[i].temp_buffer[j] = 0.0f;
        }
        
        /* CS���� */
        MAX6675_SetCS(handle, i, GPIO_PIN_SET);
    }
    
    /* SCK���� */
    MAX6675_SetSCK(handle, GPIO_PIN_RESET);
     
    return HAL_OK;
}

/**
 * @brief  GPIO��ʼ��
 * @param  handle: MAX6675���
 * @param  config: ���ò���
 * @retval None
 */
static void MAX6675_GPIO_Init(MAX6675_Handle_t* handle, const MAX6675_Config_t* config)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* ����GPIO���� */
    handle->sck.port = config->sck_port;
    handle->sck.pin = config->sck_pin;
    handle->so.port = config->so_port;
    handle->so.pin = config->so_pin;
    
    for(uint8_t i = 0; i < MAX6675_CHANNELS; i++) 
    {
        handle->cs[i].port = config->cs_ports[i];
        handle->cs[i].pin = config->cs_pins[i];
    }
    
    /* ʹ��GPIOʱ�� - ����ʵ��ʹ�õĶ˿��޸� */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    /* ����SCK���� - ������� */
    GPIO_InitStruct.Pin = handle->sck.pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(handle->sck.port, &GPIO_InitStruct);
    
    /* ����CS���� - ������� */
    for(uint8_t i = 0; i < MAX6675_CHANNELS; i++) 
    {
        GPIO_InitStruct.Pin = handle->cs[i].pin;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(handle->cs[i].port, &GPIO_InitStruct);
    }
    
    /* ����SO���� - �������� */
    GPIO_InitStruct.Pin = handle->so.pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(handle->so.port, &GPIO_InitStruct);
}

/**
 * @brief  ����CS����״̬
 * @param  handle: MAX6675���
 * @param  channel: ͨ����
 * @param  state: ����״̬
 * @retval None
 */
static void MAX6675_SetCS(MAX6675_Handle_t* handle, uint8_t channel, GPIO_PinState state)
{
    if(channel < MAX6675_CHANNELS) 
    {
        HAL_GPIO_WritePin(handle->cs[channel].port, handle->cs[channel].pin, state);
    }
}

/**
 * @brief  ����SCK����״̬
 * @param  handle: MAX6675���
 * @param  state: ����״̬
 * @retval None
 */
static void MAX6675_SetSCK(MAX6675_Handle_t* handle, GPIO_PinState state)
{
    HAL_GPIO_WritePin(handle->sck.port, handle->sck.pin, state);
}

/**
 * @brief  ��ȡSO����״̬
 * @param  handle: MAX6675���
 * @retval ����״̬
 */
static GPIO_PinState MAX6675_ReadSO(MAX6675_Handle_t* handle)
{
    return HAL_GPIO_ReadPin(handle->so.port, handle->so.pin);
}

/**
 * @brief  ��ȡԭʼ����
 * @param  handle: MAX6675���
 * @param  channel: ͨ����
 * @retval 16λԭʼ����
 */
static uint16_t MAX6675_ReadRawData(MAX6675_Handle_t* handle, uint8_t channel)
{
    uint16_t data = 0;
    uint32_t timeout_start;
    
    if(channel >= MAX6675_CHANNELS) 
    {
        return 0xFFFF;
    }
    
    /* ��¼��ʼʱ�� */
    timeout_start = HAL_GetTick();
    
    /* CS���ͣ���ʼSPIͨ�� */
    MAX6675_SetCS(handle, channel, GPIO_PIN_RESET);
    MAX6675_Delay_us(1);
    
    /* ��ȡ16λ���� */
    for(uint8_t i = 0; i < 16; i++) 
   {
        /* ��ʱ��� */
        if((HAL_GetTick() - timeout_start) > handle->read_timeout) 
        {
            MAX6675_SetCS(handle, channel, GPIO_PIN_SET);
            return 0xFFFF;
        }
        
        /* SCK������ */
        MAX6675_SetSCK(handle, GPIO_PIN_SET);
        MAX6675_Delay_us(1);
        
        /* ��ȡ����λ */
        data <<= 1;
        if(MAX6675_ReadSO(handle) == GPIO_PIN_SET) 
        {
            data |= 0x0001;
        }
        
        /* SCK�½��� */
        MAX6675_SetSCK(handle, GPIO_PIN_RESET);
        MAX6675_Delay_us(1);
    }
    
    /* CS���ߣ�����ͨ�� */
    MAX6675_SetCS(handle, channel, GPIO_PIN_SET);
    
    /* ����ͳ�� */
    handle->total_reads++;
    
    return data;
}

/**
 * @brief  ԭʼ����ת��Ϊ�¶�
 * @param  raw_data: ԭʼ����
 * @retval �¶�ֵ�����϶ȣ�
 */
static float MAX6675_RawToTemperature(uint16_t raw_data)
{
    /* ��ȡ12λ�¶����� (D14-D3) */
    uint16_t temp_data = (raw_data >> 3) & 0x0FFF;
    
    /* ת��Ϊ�¶�ֵ���ֱ���0.25��C */
    return (float)temp_data * 0.25f;
}

/**
 * @brief  Ӧ���˲���
 * @param  handle: MAX6675���
 * @param  channel: ͨ����
 * @param  new_temp: ���¶�ֵ
 * @retval �˲�����¶�
 */
static float MAX6675_ApplyFilter(MAX6675_Handle_t* handle, uint8_t channel, float new_temp)
{
    float sum = 0;
    uint8_t count = 0;
    MAX6675_ChannelStatus_t* ch_status;
    
    if(channel >= MAX6675_CHANNELS) 
    {
        return new_temp;
    }
    
    ch_status = &handle->channels[channel];
    
    /* �洢��ֵ */
    ch_status->temp_buffer[ch_status->buffer_index] = new_temp;
    ch_status->buffer_index = (ch_status->buffer_index + 1) % MAX6675_FILTER_SIZE;
    
    /* ����ǵ�һ�ζ�ȡ��������������� */
    if(!ch_status->is_initialized) {
        for(uint8_t i = 0; i < MAX6675_FILTER_SIZE; i++) 
        {
            ch_status->temp_buffer[i] = new_temp;
        }
        ch_status->is_initialized = true;
        return new_temp;
    }
    
    /* ����ƽ��ֵ */
    for(uint8_t i = 0; i < MAX6675_FILTER_SIZE; i++) 
    {
        if(ch_status->temp_buffer[i] > 0) 
        {
            sum += ch_status->temp_buffer[i];
            count++;
        }
    }
    
    return (count > 0) ? (sum / count) : new_temp;
}

/**
 * @brief  ��ȡ�¶�
 * @param  handle: MAX6675���
 * @param  channel: ͨ����
 * @param  temperature: �¶�ֵָ��
 * @retval ������
 */
MAX6675_Error_e MAX6675_ReadTemperature(MAX6675_Handle_t* handle, uint8_t channel, float* temperature)
{
    uint16_t raw_data;
    float temp_value;
    
    if(handle == NULL || temperature == NULL) 
    {
        return MAX6675_ERROR_INVALID_CH;
    }
    
    if(channel >= MAX6675_CHANNELS) 
    {
        return MAX6675_ERROR_INVALID_CH;
    }
    
    /* ����ȡ��� */
    uint32_t current_tick = HAL_GetTick();
    if((current_tick - handle->channels[channel].last_read_tick) < MAX6675_CONVERSION_TIME) 
    {
        /* �����ϴε�ֵ */
        *temperature = handle->channels[channel].temperature;
        return MAX6675_OK;
    }
    
    /* ��ȡԭʼ���� */
    raw_data = MAX6675_ReadRawData(handle, channel);
    
    /* ��鳬ʱ */
    if(raw_data == 0xFFFF) 
    {
        handle->channels[channel].error_count++;
        handle->total_errors++;
        return MAX6675_ERROR_TIMEOUT;
    }
    
    /* ���D0λ - Ӧ��Ϊ0 */
    if(raw_data & 0x0001) 
    {
        handle->channels[channel].comm_error = true;
        handle->channels[channel].error_count++;
        handle->total_errors++;
        return MAX6675_ERROR_COMM;
    }
    
    /* ���D1λ - �豸ID��Ӧ��Ϊ0 */
    if(raw_data & 0x0002) {
        handle->channels[channel].comm_error = true;
        handle->channels[channel].error_count++;
        handle->total_errors++;
        return MAX6675_ERROR_COMM;
    }
    
    /* ���D2λ - �ȵ�ż��·��� */
    if(raw_data & 0x0004) 
    {
        handle->channels[channel].is_open = true;
        handle->channels[channel].error_count++;
        handle->total_errors++;
        return MAX6675_ERROR_OPEN;
    }
    
    /* ��������־ */
    handle->channels[channel].is_open = false;
    handle->channels[channel].comm_error = false;
    
    /* ת���¶� */
    temp_value = MAX6675_RawToTemperature(raw_data);
    
    /* ��Χ��� */
    if(temp_value < TEMP_MIN || temp_value > TEMP_MAX) 
    {
        handle->channels[channel].error_count++;
        handle->total_errors++;
        return MAX6675_ERROR_RANGE;
    }
    
    /* ����ͨ��״̬ */
    handle->channels[channel].temperature = temp_value;
    handle->channels[channel].last_read_tick = current_tick;
    
    *temperature = temp_value;
    
    return MAX6675_OK;
}

/**
 * @brief  ��ȡ���˲����¶�
 * @param  handle: MAX6675���
 * @param  channel: ͨ����
 * @param  temperature: �¶�ֵָ��
 * @retval ������
 */
MAX6675_Error_e MAX6675_ReadTemperatureFiltered(MAX6675_Handle_t* handle, uint8_t channel, float* temperature)
{
    MAX6675_Error_e result;
    float temp_raw;
    
    if(handle == NULL || temperature == NULL) 
    {
        return MAX6675_ERROR_INVALID_CH;
    }
    
    /* ��ȡԭʼ�¶� */
    result = MAX6675_ReadTemperature(handle, channel, &temp_raw);
    
    if(result != MAX6675_OK) 
    {
        return result;
    }
    
    /* Ӧ���˲� */
    if(handle->use_filter) 
    {
        *temperature = MAX6675_ApplyFilter(handle, channel, temp_raw);
    } 
    else
    {
        *temperature = temp_raw;
    }
    
    return MAX6675_OK;
}

/**
 * @brief  ��ȡ����ͨ��
 * @param  handle: MAX6675���
 * @param  temperatures: �¶�����
 * @retval HAL״̬
 */
HAL_StatusTypeDef MAX6675_ReadAllChannels(MAX6675_Handle_t* handle, float* temperatures)
{
    MAX6675_Error_e result;
    
    if(handle == NULL || temperatures == NULL) 
    {
        return HAL_ERROR;
    }
    
    for(uint8_t i = 0; i < MAX6675_CHANNELS; i++) 
    {
        if(handle->use_filter) 
        {
            result = MAX6675_ReadTemperatureFiltered(handle, i, &temperatures[i]);
        } 
        else 
        {
            result = MAX6675_ReadTemperature(handle, i, &temperatures[i]);
        }
        
        if(result != MAX6675_OK) 
        {
            temperatures[i] = TEMP_INVALID;
        }
        
        /* ͨ������ʱ */
        HAL_Delay(50);
    }
    
    return HAL_OK;
}

/**
 * @brief  ��ȡͨ��״̬
 * @param  handle: MAX6675���
 * @param  channel: ͨ����
 * @param  status: ״̬�ṹ��ָ��
 * @retval HAL״̬
 */
HAL_StatusTypeDef MAX6675_GetChannelStatus(MAX6675_Handle_t* handle, uint8_t channel, MAX6675_ChannelStatus_t* status)
{
    if(handle == NULL || status == NULL || channel >= MAX6675_CHANNELS) 
    {
        return HAL_ERROR;
    }
    
    memcpy(status, &handle->channels[channel], sizeof(MAX6675_ChannelStatus_t));
    
    return HAL_OK;
}

/**
 * @brief  �������
 * @param  handle: MAX6675���
 * @param  channel: ͨ����
 * @retval HAL״̬
 */
HAL_StatusTypeDef MAX6675_ClearErrors(MAX6675_Handle_t* handle, uint8_t channel)
{
    if(handle == NULL || channel >= MAX6675_CHANNELS) 
    {
        return HAL_ERROR;
    }
    
    handle->channels[channel].is_open = false;
    handle->channels[channel].comm_error = false;
    handle->channels[channel].error_count = 0;
    
    return HAL_OK;
}

/**
 * @brief  ���ͨ���Ƿ�׼����
 * @param  handle: MAX6675���
 * @param  channel: ͨ����
 * @retval true-׼���ã�false-δ׼����
 */
bool MAX6675_IsChannelReady(MAX6675_Handle_t* handle, uint8_t channel)
{
    uint32_t current_tick;
    
    if(handle == NULL || channel >= MAX6675_CHANNELS) 
    {
        return false;
    }
    
    current_tick = HAL_GetTick();
    
    return (current_tick - handle->channels[channel].last_read_tick) >= MAX6675_CONVERSION_TIME;
}

/**
 * @brief  ��ȡ�����ַ���
 * @param  error: ������
 * @retval ���������ַ���
 */
const char* MAX6675_GetErrorString(MAX6675_Error_e error)
{
    switch(error) 
    {
        case MAX6675_OK:
            return "No error";
        case MAX6675_ERROR_OPEN:
            return "Thermocouple open";
        case MAX6675_ERROR_COMM:
            return "Communication error";
        case MAX6675_ERROR_RANGE:
            return "Temperature out of range";
        case MAX6675_ERROR_INVALID_CH:
            return "Invalid channel";
        case MAX6675_ERROR_TIMEOUT:
            return "Read timeout";
        default:
            return "Unknown error";
    }
}

/**
 * @brief  ��ӡ״̬��Ϣ
 * @param  handle: MAX6675���
 * @retval None
 */
void MAX6675_PrintStatus(MAX6675_Handle_t* handle)
{
    if(handle == NULL) 
    {
        return;
    }
    
    printf("=== MAX6675 Status ===\n");
    printf("Total reads: %lu\n", handle->total_reads);
    printf("Total errors: %lu\n", handle->total_errors);
    printf("Filter: %s\n", handle->use_filter ? "Enabled" : "Disabled");
    
    for(uint8_t i = 0; i < MAX6675_CHANNELS; i++) 
    {
        printf("\nChannel %d:\n", i + 1);
        printf("  Temperature: %.2f��C\n", handle->channels[i].temperature);
        printf("  Open circuit: %s\n", handle->channels[i].is_open ? "Yes" : "No");
        printf("  Comm error: %s\n", handle->channels[i].comm_error ? "Yes" : "No");
        printf("  Error count: %lu\n", handle->channels[i].error_count);
    }
}

/**
 * @brief  ����ʼ��
 * @param  handle: MAX6675���
 * @retval HAL״̬
 */
HAL_StatusTypeDef MAX6675_DeInit(MAX6675_Handle_t* handle)
{
    if(handle == NULL) 
    {
        return HAL_ERROR;
    }
    
    /* ����CS�������� */
    for(uint8_t i = 0; i < MAX6675_CHANNELS; i++) 
    {
        MAX6675_SetCS(handle, i, GPIO_PIN_SET);
    }
    
    /* ��վ�� */
    memset(handle, 0, sizeof(MAX6675_Handle_t));
    
    return HAL_OK;
}


