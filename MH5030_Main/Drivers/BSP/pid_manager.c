#include "pid_manager.h"

/* ---------------------- ȫ�ֱ������� ---------------------- */
System_Status_t g_system_status = {0};     /* ȫ��ϵͳ״̬ */
PID_Storage_t g_pid_storage = {0};         /* PID�洢�ṹ */

/**
 * @brief      ��ʼ��PID����������
 * @param      ��
 * @retval     ��
 */
void Init_PID_Manager(void)
{
    /* ��ʼ��ϵͳ״̬ */
    memset(&g_system_status, 0, sizeof(System_Status_t));
    
    /* ����Ĭ���¶� */
    g_system_status.front_temp_sv = 120.0f;
    g_system_status.rear_temp_sv = 120.0f;
    
    /* ���Դ�EEPROM���ز��� */
    if (!Load_All_Parameters())
    {
        /* ����ʧ�ܣ���ʼ��ΪĬ��ֵ */
        g_pid_storage.magic = PID_MAGIC_WORD;
        g_pid_storage.version = PID_VERSION;
        
        /* ����������Ϊ��Ч */
        for (uint8_t i = 0; i < PARAM_GROUP_MAX; i++)
        {
            g_pid_storage.group[i].valid = 0;
        }
    }
    
    /* ȷ����ǰ������ */
    g_system_status.param_group = Get_Param_Group(g_system_status.front_temp_sv);
    
    /* ���Լ��ص�ǰ����� */
    if (!Load_Parameters_From_Group(g_system_status.param_group))
    {
        /* ��ǰ������Ч����������Ĭ��ֵ */
        Set_Default_Parameters(g_system_status.param_group);
        g_system_status.param_loaded = 0;
    }
    else
    {
        g_system_status.param_loaded = 1;
    }
}

/**
 * @brief      �����趨�¶Ȼ�ȡ����������
 * @param      temp_sv    �趨�¶�
 * @retval     ����������(0/1/2)
 */
uint8_t Get_Param_Group(float temp_sv)
{
    if (temp_sv <= TEMP_GROUP0_MAX)
    {
        return PARAM_GROUP_120;    /* ��140�� */
    }
    else if (temp_sv <= TEMP_GROUP1_MAX)
    {
        return PARAM_GROUP_160;    /* 141-180�� */
    }
    else
    {
        return PARAM_GROUP_220;    /* >180�� */
    }
}

/**
 * @brief      ��鲢�л�������
 * @param      ��
 * @retval     ��
 */
void Check_And_Switch_Group(void)
{
    uint8_t new_group;
    
    /* ��ȡ�µĲ����� */
    new_group = Get_Param_Group(g_system_status.front_temp_sv);
    
    /* ����Ƿ���Ҫ�л� */
    if (new_group != g_system_status.param_group)
    {
        g_system_status.param_group = new_group;
        
        /* ����������� */
        if (!Load_Parameters_From_Group(new_group))
        {
            /* ��������Ч������ʹ��Ĭ��ֵ */
            Set_Default_Parameters(new_group);
            g_system_status.param_loaded = 0;
        }
        else
        {
            g_system_status.param_loaded = 1;
        }
        
        /* Ӧ�ò�����PID������ */
        Apply_Parameters_To_PID(new_group);
    }
}

/**
 * @brief      ��EEPROM�������в���
 * @param      ��
 * @retval     true:�ɹ�  false:ʧ��
 */
bool Load_All_Parameters(void)
{
    uint8_t result;
    
    /* ��EEPROM��ȡ */
    result = FM_ReadByteseq(EEPROM_BASE_ADDR, &g_pid_storage, sizeof(PID_Storage_t));
    
    if (result != 0)
    {
        return false;    /* ��ȡʧ�� */
    }
    
    /* ��֤������Ч�� */
    if (!Verify_Parameters(&g_pid_storage))
    {
        return false;    /* ������Ч */
    }
    
    return true;
}

/**
 * @brief      �������в�����EEPROM
 * @param      ��
 * @retval     ��
 */
void Save_All_Parameters(void)
{
    /* ����У��� */
    g_pid_storage.checksum = Calculate_Checksum(&g_pid_storage);
    
    /* д��EEPROM */
    FM_WriteByteseq(EEPROM_BASE_ADDR, &g_pid_storage, sizeof(PID_Storage_t));
}

/**
 * @brief      ��֤������Ч��
 * @param      storage    �洢�ṹָ��
 * @retval     true:��Ч  false:��Ч
 */
bool Verify_Parameters(PID_Storage_t *storage)
{
    uint16_t calc_checksum;
    
    /* ���ħ���� */
    if (storage->magic != PID_MAGIC_WORD)
    {
        return false;
    }
    
    /* ���汾�� */
    if (storage->version != PID_VERSION)
    {
        return false;
    }
    
    /* ���㲢��֤У��� */
    calc_checksum = Calculate_Checksum(storage);
    if (calc_checksum != storage->checksum)
    {
        return false;
    }
    
    return true;
}

/**
 * @brief      ����У���
 * @param      storage    �洢�ṹָ��
 * @retval     У���
 */
uint16_t Calculate_Checksum(PID_Storage_t *storage)
{
    uint16_t checksum = 0;
    uint8_t *ptr = (uint8_t*)storage;
    uint16_t len = sizeof(PID_Storage_t) - sizeof(uint16_t);  /* ������checksum���� */
    
    for (uint16_t i = 0; i < len; i++)
    {
        checksum += ptr[i];
    }
    
    return checksum;
}

/**
 * @brief      ��ָ������ز���
 * @param      group    ����������
 * @retval     true:�ɹ�  false:ʧ��
 */
bool Load_Parameters_From_Group(uint8_t group)
{
    PID_Param_Group_t *param_group;
    
    if (group >= PARAM_GROUP_MAX)
    {
        return false;
    }
    
    param_group = &g_pid_storage.group[group];
    
    /* �����Ч��־ */
    if (!param_group->valid)
    {
        return false;
    }
    
    /* ����ǰǹ�ܲ��� */
    g_stPidFront.Kp = param_group->front_Kp;
    g_stPidFront.Ti = param_group->front_Ti;
    g_stPidFront.Td = param_group->front_Td;
    
    /* ����ǻ����� */
    g_stPidRear.Kp = param_group->rear_Kp;
    g_stPidRear.Ti = param_group->rear_Ti;
    g_stPidRear.Td = param_group->rear_Td;
    
    return true;
}

/**
 * @brief      ���������ָ����
 * @param      group    ����������
 * @retval     ��
 */
void Save_Parameters_To_Group(uint8_t group)
{
    PID_Param_Group_t *param_group;
    
    if (group >= PARAM_GROUP_MAX)
    {
        return;
    }
    
    param_group = &g_pid_storage.group[group];
    
    /* ����ǰǹ�ܲ��� */
    param_group->front_Kp = g_stPidFront.Kp;
    param_group->front_Ti = g_stPidFront.Ti;
    param_group->front_Td = g_stPidFront.Td;
    
    /* ����ǻ����� */
    param_group->rear_Kp = g_stPidRear.Kp;
    param_group->rear_Ti = g_stPidRear.Ti;
    param_group->rear_Td = g_stPidRear.Td;
    
    /* ������Ч��־ */
    param_group->valid = 1;
    
    /* ���浽EEPROM */
    Save_All_Parameters();
}

/**
 * @brief      ����Ĭ�ϲ���
 * @param      group    ����������
 * @retval     ��
 */
void Set_Default_Parameters(uint8_t group)
{
    /* ���ݲ�ͬ�¶�������Ĭ�ϲ��� */
    switch (group)
    {
        case PARAM_GROUP_120:
            /* 120��Ĭ�ϲ��� */
            g_stPidFront.Kp = 10.0f;
            g_stPidFront.Ti = 150.0f;
            g_stPidFront.Td = 30.0f;
            
            g_stPidRear.Kp = 8.0f;
            g_stPidRear.Ti = 180.0f;
            g_stPidRear.Td = 35.0f;
            break;
            
        case PARAM_GROUP_160:
            /* 160��Ĭ�ϲ��� */
            g_stPidFront.Kp = 12.0f;
            g_stPidFront.Ti = 200.0f;
            g_stPidFront.Td = 40.0f;
            
            g_stPidRear.Kp = 10.0f;
            g_stPidRear.Ti = 220.0f;
            g_stPidRear.Td = 45.0f;
            break;
            
        case PARAM_GROUP_220:
            /* 220��Ĭ�ϲ��� */
            g_stPidFront.Kp = 15.0f;
            g_stPidFront.Ti = 250.0f;
            g_stPidFront.Td = 50.0f;
            
            g_stPidRear.Kp = 12.0f;
            g_stPidRear.Ti = 280.0f;
            g_stPidRear.Td = 55.0f;
            break;
            
        default:
            break;
    }
}

/**
 * @brief      Ӧ�ò�����PID������
 * @param      group    ����������
 * @retval     ��
 */
void Apply_Parameters_To_PID(uint8_t group)
{
    if (group >= PARAM_GROUP_MAX)
    {
        return;
    }
    
    /* �������Ч�������Ӵ洢���� */
    if (g_pid_storage.group[group].valid)
    {
        Load_Parameters_From_Group(group);
    }
    else
    {
        /* ����ʹ��Ĭ�ϲ��� */
        Set_Default_Parameters(group);
    }
    
    /* �������������л�ʱ�ĳ�� */
    g_stPidFront.SEk = 0;
    g_stPidRear.SEk = 0;
}

/**
 * @brief      ����������
 * @param      ��
 * @retval     ��
 */
void Start_Autotune(void)
{
    /* ������������־ */
    g_system_status.autotune_request = 1;
    g_system_status.mode = PID_MODE_AUTOTUNE;
    
    /* ���ô�Խ���� */
    g_system_status.front_cross_cnt = 0;
    g_system_status.rear_cross_cnt = 0;
    g_system_status.autotune_complete = 0;
    
    /* ���³�ʼ������������ */
    TunePretreatment(&g_stPidFront, &g_stPidFrontAuto);
    TunePretreatment(&g_stPidRear, &g_stPidRearAuto);
}

/**
 * @brief      ֹͣ������
 * @param      ��
 * @retval     ��
 */
void Stop_Autotune(void)
{
    /* �����������־ */
    g_system_status.autotune_request = 0;
    g_system_status.mode = PID_MODE_RUN;
    
    /* �ر������� */
    g_stPidFrontAuto.tuneEnable = 0;
    g_stPidRearAuto.tuneEnable = 0;
}

/**
 * @brief      ����������Ƿ����
 * @param      ��
 * @retval     ��
 */
void Check_Autotune_Complete(void)
{
    /* ���´�Խ���� */
    g_system_status.front_cross_cnt = g_stPidFrontAuto.zeroAcrossCounter;
    g_system_status.rear_cross_cnt = g_stPidRearAuto.zeroAcrossCounter;
    
    /* �����·�Ƿ���� */
    if (g_stPidFrontAuto.tuneEnable == 0 && g_stPidRearAuto.tuneEnable == 0)
    {
        g_system_status.autotune_complete = 1;
        Save_Autotune_Results();
        Stop_Autotune();
    }
}

/**
 * @brief      �������������
 * @param      ��
 * @retval     ��
 */
void Save_Autotune_Results(void)
{
    /* ���浽��ǰ������ */
    Save_Parameters_To_Group(g_system_status.param_group);
    
    /* ���ò����Ѽ��ر�־ */
    g_system_status.param_loaded = 1;
}

/**
 * @brief      ����ϵͳ״̬
 * @param      ��
 * @retval     ��
 */
void Update_System_Status(void)
{
    /* ����������״̬ */
    if (g_system_status.mode == PID_MODE_AUTOTUNE)
    {
        Check_Autotune_Complete();
    }
}

/**
 * @brief      �������״̬
 * @param      ��
 * @retval     ��
 */
void Clear_Error_Status(void)
{
    g_system_status.error_code = 0;
    g_system_status.temp_error = 0;
}

