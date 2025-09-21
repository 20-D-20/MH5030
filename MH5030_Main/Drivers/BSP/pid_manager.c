#include "pid_manager.h"

/* ---------------------- 全局变量定义 ---------------------- */
System_Status_t g_system_status = {0};     /* 全局系统状态 */
PID_Storage_t g_pid_storage = {0};         /* PID存储结构 */

/**
 * @brief      初始化PID参数管理器
 * @param      无
 * @retval     无
 */
void Init_PID_Manager(void)
{
    /* 初始化系统状态 */
    memset(&g_system_status, 0, sizeof(System_Status_t));
    
    /* 设置默认温度 */
    g_system_status.front_temp_sv = 120.0f;
    g_system_status.rear_temp_sv = 120.0f;
    
    /* 尝试从EEPROM加载参数 */
    if (!Load_All_Parameters())
    {
        /* 加载失败，初始化为默认值 */
        g_pid_storage.magic = PID_MAGIC_WORD;
        g_pid_storage.version = PID_VERSION;
        
        /* 设置所有组为无效 */
        for (uint8_t i = 0; i < PARAM_GROUP_MAX; i++)
        {
            g_pid_storage.group[i].valid = 0;
        }
    }
    
    /* 确定当前参数组 */
    g_system_status.param_group = Get_Param_Group(g_system_status.front_temp_sv);
    
    /* 尝试加载当前组参数 */
    if (!Load_Parameters_From_Group(g_system_status.param_group))
    {
        /* 当前组无有效参数，设置默认值 */
        Set_Default_Parameters(g_system_status.param_group);
        g_system_status.param_loaded = 0;
    }
    else
    {
        g_system_status.param_loaded = 1;
    }
}

/**
 * @brief      根据设定温度获取参数组索引
 * @param      temp_sv    设定温度
 * @retval     参数组索引(0/1/2)
 */
uint8_t Get_Param_Group(float temp_sv)
{
    if (temp_sv <= TEMP_GROUP0_MAX)
    {
        return PARAM_GROUP_120;    /* ≤140℃ */
    }
    else if (temp_sv <= TEMP_GROUP1_MAX)
    {
        return PARAM_GROUP_160;    /* 141-180℃ */
    }
    else
    {
        return PARAM_GROUP_220;    /* >180℃ */
    }
}

/**
 * @brief      检查并切换参数组
 * @param      无
 * @retval     无
 */
void Check_And_Switch_Group(void)
{
    uint8_t new_group;
    
    /* 获取新的参数组 */
    new_group = Get_Param_Group(g_system_status.front_temp_sv);
    
    /* 检查是否需要切换 */
    if (new_group != g_system_status.param_group)
    {
        g_system_status.param_group = new_group;
        
        /* 加载新组参数 */
        if (!Load_Parameters_From_Group(new_group))
        {
            /* 新组无有效参数，使用默认值 */
            Set_Default_Parameters(new_group);
            g_system_status.param_loaded = 0;
        }
        else
        {
            g_system_status.param_loaded = 1;
        }
        
        /* 应用参数到PID控制器 */
        Apply_Parameters_To_PID(new_group);
    }
}

/**
 * @brief      从EEPROM加载所有参数
 * @param      无
 * @retval     true:成功  false:失败
 */
bool Load_All_Parameters(void)
{
    uint8_t result;
    
    /* 从EEPROM读取 */
    result = FM_ReadByteseq(EEPROM_BASE_ADDR, &g_pid_storage, sizeof(PID_Storage_t));
    
    if (result != 0)
    {
        return false;    /* 读取失败 */
    }
    
    /* 验证参数有效性 */
    if (!Verify_Parameters(&g_pid_storage))
    {
        return false;    /* 参数无效 */
    }
    
    return true;
}

/**
 * @brief      保存所有参数到EEPROM
 * @param      无
 * @retval     无
 */
void Save_All_Parameters(void)
{
    /* 更新校验和 */
    g_pid_storage.checksum = Calculate_Checksum(&g_pid_storage);
    
    /* 写入EEPROM */
    FM_WriteByteseq(EEPROM_BASE_ADDR, &g_pid_storage, sizeof(PID_Storage_t));
}

/**
 * @brief      验证参数有效性
 * @param      storage    存储结构指针
 * @retval     true:有效  false:无效
 */
bool Verify_Parameters(PID_Storage_t *storage)
{
    uint16_t calc_checksum;
    
    /* 检查魔术字 */
    if (storage->magic != PID_MAGIC_WORD)
    {
        return false;
    }
    
    /* 检查版本号 */
    if (storage->version != PID_VERSION)
    {
        return false;
    }
    
    /* 计算并验证校验和 */
    calc_checksum = Calculate_Checksum(storage);
    if (calc_checksum != storage->checksum)
    {
        return false;
    }
    
    return true;
}

/**
 * @brief      计算校验和
 * @param      storage    存储结构指针
 * @retval     校验和
 */
uint16_t Calculate_Checksum(PID_Storage_t *storage)
{
    uint16_t checksum = 0;
    uint8_t *ptr = (uint8_t*)storage;
    uint16_t len = sizeof(PID_Storage_t) - sizeof(uint16_t);  /* 不包括checksum本身 */
    
    for (uint16_t i = 0; i < len; i++)
    {
        checksum += ptr[i];
    }
    
    return checksum;
}

/**
 * @brief      从指定组加载参数
 * @param      group    参数组索引
 * @retval     true:成功  false:失败
 */
bool Load_Parameters_From_Group(uint8_t group)
{
    PID_Param_Group_t *param_group;
    
    if (group >= PARAM_GROUP_MAX)
    {
        return false;
    }
    
    param_group = &g_pid_storage.group[group];
    
    /* 检查有效标志 */
    if (!param_group->valid)
    {
        return false;
    }
    
    /* 加载前枪管参数 */
    g_stPidFront.Kp = param_group->front_Kp;
    g_stPidFront.Ti = param_group->front_Ti;
    g_stPidFront.Td = param_group->front_Td;
    
    /* 加载腔体参数 */
    g_stPidRear.Kp = param_group->rear_Kp;
    g_stPidRear.Ti = param_group->rear_Ti;
    g_stPidRear.Td = param_group->rear_Td;
    
    return true;
}

/**
 * @brief      保存参数到指定组
 * @param      group    参数组索引
 * @retval     无
 */
void Save_Parameters_To_Group(uint8_t group)
{
    PID_Param_Group_t *param_group;
    
    if (group >= PARAM_GROUP_MAX)
    {
        return;
    }
    
    param_group = &g_pid_storage.group[group];
    
    /* 保存前枪管参数 */
    param_group->front_Kp = g_stPidFront.Kp;
    param_group->front_Ti = g_stPidFront.Ti;
    param_group->front_Td = g_stPidFront.Td;
    
    /* 保存腔体参数 */
    param_group->rear_Kp = g_stPidRear.Kp;
    param_group->rear_Ti = g_stPidRear.Ti;
    param_group->rear_Td = g_stPidRear.Td;
    
    /* 设置有效标志 */
    param_group->valid = 1;
    
    /* 保存到EEPROM */
    Save_All_Parameters();
}

/**
 * @brief      设置默认参数
 * @param      group    参数组索引
 * @retval     无
 */
void Set_Default_Parameters(uint8_t group)
{
    /* 根据不同温度组设置默认参数 */
    switch (group)
    {
        case PARAM_GROUP_120:
            /* 120℃默认参数 */
            g_stPidFront.Kp = 10.0f;
            g_stPidFront.Ti = 150.0f;
            g_stPidFront.Td = 30.0f;
            
            g_stPidRear.Kp = 8.0f;
            g_stPidRear.Ti = 180.0f;
            g_stPidRear.Td = 35.0f;
            break;
            
        case PARAM_GROUP_160:
            /* 160℃默认参数 */
            g_stPidFront.Kp = 12.0f;
            g_stPidFront.Ti = 200.0f;
            g_stPidFront.Td = 40.0f;
            
            g_stPidRear.Kp = 10.0f;
            g_stPidRear.Ti = 220.0f;
            g_stPidRear.Td = 45.0f;
            break;
            
        case PARAM_GROUP_220:
            /* 220℃默认参数 */
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
 * @brief      应用参数到PID控制器
 * @param      group    参数组索引
 * @retval     无
 */
void Apply_Parameters_To_PID(uint8_t group)
{
    if (group >= PARAM_GROUP_MAX)
    {
        return;
    }
    
    /* 如果有有效参数，从存储加载 */
    if (g_pid_storage.group[group].valid)
    {
        Load_Parameters_From_Group(group);
    }
    else
    {
        /* 否则使用默认参数 */
        Set_Default_Parameters(group);
    }
    
    /* 清除积分项，避免切换时的冲击 */
    g_stPidFront.SEk = 0;
    g_stPidRear.SEk = 0;
}

/**
 * @brief      启动自整定
 * @param      无
 * @retval     无
 */
void Start_Autotune(void)
{
    /* 设置自整定标志 */
    g_system_status.autotune_request = 1;
    g_system_status.mode = PID_MODE_AUTOTUNE;
    
    /* 重置穿越计数 */
    g_system_status.front_cross_cnt = 0;
    g_system_status.rear_cross_cnt = 0;
    g_system_status.autotune_complete = 0;
    
    /* 重新初始化自整定参数 */
    TunePretreatment(&g_stPidFront, &g_stPidFrontAuto);
    TunePretreatment(&g_stPidRear, &g_stPidRearAuto);
}

/**
 * @brief      停止自整定
 * @param      无
 * @retval     无
 */
void Stop_Autotune(void)
{
    /* 清除自整定标志 */
    g_system_status.autotune_request = 0;
    g_system_status.mode = PID_MODE_RUN;
    
    /* 关闭自整定 */
    g_stPidFrontAuto.tuneEnable = 0;
    g_stPidRearAuto.tuneEnable = 0;
}

/**
 * @brief      检查自整定是否完成
 * @param      无
 * @retval     无
 */
void Check_Autotune_Complete(void)
{
    /* 更新穿越计数 */
    g_system_status.front_cross_cnt = g_stPidFrontAuto.zeroAcrossCounter;
    g_system_status.rear_cross_cnt = g_stPidRearAuto.zeroAcrossCounter;
    
    /* 检查两路是否都完成 */
    if (g_stPidFrontAuto.tuneEnable == 0 && g_stPidRearAuto.tuneEnable == 0)
    {
        g_system_status.autotune_complete = 1;
        Save_Autotune_Results();
        Stop_Autotune();
    }
}

/**
 * @brief      保存自整定结果
 * @param      无
 * @retval     无
 */
void Save_Autotune_Results(void)
{
    /* 保存到当前参数组 */
    Save_Parameters_To_Group(g_system_status.param_group);
    
    /* 设置参数已加载标志 */
    g_system_status.param_loaded = 1;
}

/**
 * @brief      更新系统状态
 * @param      无
 * @retval     无
 */
void Update_System_Status(void)
{
    /* 更新自整定状态 */
    if (g_system_status.mode == PID_MODE_AUTOTUNE)
    {
        Check_Autotune_Complete();
    }
}

/**
 * @brief      清除错误状态
 * @param      无
 * @retval     无
 */
void Clear_Error_Status(void)
{
    g_system_status.error_code = 0;
    g_system_status.temp_error = 0;
}

