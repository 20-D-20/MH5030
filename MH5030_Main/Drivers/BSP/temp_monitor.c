#include "temp_monitor.h"

/* 全局变量 */
static TempMonitor_t g_monitor;

/* 页面标题 */
static const char* page_titles[PAGE_MAX_NUM] = 
{
    "前枪管温度",
    "后腔体温度",
    "前枪管设置",
    "后腔体设置"
};

/**
 * @brief  系统初始化
 */
void TempMonitor_Init(void)
{
    /* 初始化OLED显示屏 */
    SSD1305_init();
    clearscreen();
    
    /* 初始化系统状态 */
    g_monitor.current_page = PAGE_FRONT_TEMP;
    g_monitor.state = STATE_DISPLAY;
    
    /* 初始化温度参数 */
    g_monitor.front_gun.current = 0.0f;
    g_monitor.front_gun.target = 25.0f;
    g_monitor.front_gun.temp_set = 25.0f;
    
    g_monitor.rear_chamber.current = 0.0f;
    g_monitor.rear_chamber.target = 25.0f;
    g_monitor.rear_chamber.temp_set = 25.0f;
    
    g_monitor.refresh_flag = 1;
    g_monitor.save_flag = 0;
    g_monitor.save_timer = 0;
    g_monitor.update_counter = 0;
}

/* 修改 Display_TempPage 函数 */
static void Display_TempPage(uint8_t page_num)
{
    char str_buf[32];
    float current_temp, target_temp;
    const char* title;
    static uint8_t last_page = 0xFF;  // 记录上次页面
    static float last_current = -999.0f;  // 记录上次温度
    static float last_target = -999.0f;
    
    /* 根据页面选择数据 */
    if (page_num == PAGE_FRONT_TEMP)
    {
        title = page_titles[PAGE_FRONT_TEMP];
        current_temp = g_monitor.front_gun.current;
        target_temp = g_monitor.front_gun.target;
    }
    else
    {
        title = page_titles[PAGE_REAR_TEMP];
        current_temp = g_monitor.rear_chamber.current;
        target_temp = g_monitor.rear_chamber.target;
    }
    
    /* 只在页面切换时清屏 */
    if (last_page != page_num)
    {
        clearscreen();
        last_page = page_num;
        last_current = -999.0f;  // 强制刷新所有数据
        last_target = -999.0f;
        
        /* 显示静态内容（标题、标签、分割线） */
        uint8_t title_x = (128 - strlen(title) * 8) / 2;
        DispString(title_x, 0,(unsigned char*) title, false);
        
//        /* 画分割线（只需画一次） */
//        for (uint8_t i = 0; i < 128; i++)
//        {
//            Set_Addr(1, i);
//            Write_Data(0x01);
//            Set_Addr(6, i);
//            Write_Data(0x80);
//        }
        
        /* 显示静态标签 */
        DispString(8, 16,(unsigned char*) "当前:", false);
        DispString(8, 32, (unsigned char*)"目标:", false);
    }
    
    /* 只更新变化的数值部分 */
    if (fabs(current_temp - last_current) > 0.01f)
    {
        /* 清除旧数值区域（局部清除） */
        for (uint8_t i = 48; i < 104; i++)
        {
            Set_Addr(2, i);
            Write_Data(0x00);
            Set_Addr(3, i);
            Write_Data(0x00);
        }
        /* 显示新数值 */
        sprintf(str_buf, "%.1f", current_temp);
        DispString(48, 16, (unsigned char*)str_buf, false);
        Disp_Char(88, 16, 0xA1, false);
        Disp_Char(96, 16, 'C', false);
        last_current = current_temp;
    }
    
    if (fabs(target_temp - last_target) > 0.01f)
    {
        /* 清除旧数值区域 */
        for (uint8_t i = 48; i < 104; i++)
        {
            Set_Addr(4, i);
            Write_Data(0x00);
            Set_Addr(5, i);
            Write_Data(0x00);
        }
        /* 显示新数值 */
        sprintf(str_buf, "%.1f", target_temp);
        DispString(48, 32, (unsigned char*)str_buf, false);
        Disp_Char(88, 32, 0xA1, false);
        Disp_Char(96, 32, 'C', false);
        last_target = target_temp;
    }
    
    /* 状态显示（局部更新） */
    static uint8_t last_status = 0xFF;
    uint8_t current_status;
    
    if (current_temp < target_temp - 1.0f)
    {
        current_status = 1;  // 加热中
    }
    else if (current_temp > target_temp + 1.0f)
    {
        current_status = 2;  // 冷却中
    }
    else
    {
        current_status = 3;  // 已到达
    }
    
    if (last_status != current_status)
    {
        /* 清除状态区域 */
        for (uint8_t i = 8; i < 120; i++)
        {
            Set_Addr(6, i);
            Write_Data(0x80);  // 保留底线
            Set_Addr(7, i);
            Write_Data(0x00);
        }
        /* 显示新状态 */
        switch (current_status)
        {
            case 1:
                DispString(8, 48, (unsigned char*)"状态:加热中...", false);
                break;
            case 2:
                DispString(8, 48, (unsigned char*)"状态:冷却中...", false);
                break;
            case 3:
                DispString(8, 48, (unsigned char*)"状态:已到达", false);
                break;
        }
        last_status = current_status;
    }
}


/**
 * @brief  显示设置页面
 * @param  page_num: 页面编号
 */
static void Display_SetPage(uint8_t page_num)
{
    char str_buf[32];
    float set_value;
    const char* title;
    static uint8_t blink_counter = 0;
    
    /* 根据页面和状态选择数据 */
    if (page_num == PAGE_FRONT_SET)
    {
        title = page_titles[PAGE_FRONT_SET];
        if (g_monitor.state == STATE_SETTING)
        {
            set_value = g_monitor.front_gun.temp_set;
        }
        else
        {
            set_value = g_monitor.front_gun.target;
        }
    }
    else
    {
        title = page_titles[PAGE_REAR_SET];
        if (g_monitor.state == STATE_SETTING)
        {
            set_value = g_monitor.rear_chamber.temp_set;
        }
        else
        {
            set_value = g_monitor.rear_chamber.target;
        }
    }
    
    /* 清屏 */
    clearscreen();
    
    /* 显示标题 - 居中显示 */
    uint8_t title_x = (128 - strlen(title) * 8) / 2;
    DispString(title_x, 0, (unsigned char*)title, false);
    
    /* 画分割线 */
    for (uint8_t i = 0; i < 128; i++)
    {
        Set_Addr(1, i);
        Write_Data(0x01);
    }
    
    /* 显示设定温度 */
    DispString(8, 24, (unsigned char*)"设定温度:", false);
    
    /* 显示温度值 - 设置状态时闪烁 */
    sprintf(str_buf, "%.1f", set_value);
    if (g_monitor.state == STATE_SETTING)
    {
        blink_counter++;
        if (blink_counter < 30)
        {
            /* 显示带箭头的温度值 */
            DispString(24, 32, (unsigned char*)">", false);
            DispString(32, 32, (unsigned char*)str_buf, false);
            Disp_Char(72, 32, 0xA1, false);  // 度符号
            Disp_Char(80, 32, 'C', false);
            DispString(96, 32, (unsigned char*)"<", false);
        }
        else if (blink_counter < 60)
        {
            /* 闪烁效果 - 不显示 */
        }
        else
        {
            blink_counter = 0;
        }
    }
    else
    {
        /* 正常显示 */
        DispString(32, 32, (unsigned char*)str_buf, false);
        Disp_Char(72, 32, 0xA1, false);  // 度符号
        Disp_Char(80, 32, 'C', false);
    }
    
    /* 画底部分割线 */
    for (uint8_t i = 0; i < 128; i++)
    {
        Set_Addr(6, i);
        Write_Data(0x80);
    }
    
    /* 显示操作提示 */
    if (g_monitor.state == STATE_SETTING)
    {
        DispStringS(4, 48, (unsigned char*)"UP/DN:+-  OK:Save  RT:Cancel", false);
    }
    else
    {
        DispString(16, 48, (unsigned char*)"按确认键进入设置", false);
    }
    
    /* 显示保存成功提示 */
    if (g_monitor.save_flag && g_monitor.save_timer > 0)
    {
        /* 绘制提示框 */
        for (uint8_t i = 2; i <= 4; i++)
        {
            Set_Addr(i, 24);
            Write_Data(0xFF);
            Set_Addr(i, 104);
            Write_Data(0xFF);
        }
        DispString(32, 24, (unsigned char*)"保存成功!", false);
    }
}

/**
 * @brief  按键处理
 */
static void Process_Keys(void)
{
    uint8_t key = key_scan(0);  // 不支持连按
    
    if (key == 0)
    {
        return;
    }
    
    switch (key)
    {
        case KEY_UP:  // KEY4对应上键
        if (g_monitor.state == STATE_DISPLAY)
        {
            /* 显示模式：切换到上一页 */
            if (g_monitor.current_page == 0)
            {
                g_monitor.current_page = PAGE_MAX_NUM - 1;
            }
            else
            {
                g_monitor.current_page--;
            }
            g_monitor.refresh_flag = 1;
        }
        else
        {
            /* 设置模式：增加温度 */
            if (g_monitor.current_page == PAGE_FRONT_SET)
            {
                g_monitor.front_gun.temp_set += TEMP_STEP;
                if (g_monitor.front_gun.temp_set > TEMP_RANGE_MAX)
                {
                    g_monitor.front_gun.temp_set = TEMP_RANGE_MAX;
                }
            }
            else
            {
                g_monitor.rear_chamber.temp_set += TEMP_STEP;
                if (g_monitor.rear_chamber.temp_set > TEMP_RANGE_MAX)
                {
                    g_monitor.rear_chamber.temp_set = TEMP_RANGE_MAX;
                }
            }
            g_monitor.refresh_flag = 1;
        }
        break;
        
    case KEY_DOWN:  // KEY3对应下键
        if (g_monitor.state == STATE_DISPLAY)
        {
            /* 显示模式：切换到下一页 */
            g_monitor.current_page++;
            if (g_monitor.current_page >= PAGE_MAX_NUM)
            {
                g_monitor.current_page = 0;
            }
            g_monitor.refresh_flag = 1;
        }
        else
        {
            /* 设置模式：减少温度 */
            if (g_monitor.current_page == PAGE_FRONT_SET)
            {
                g_monitor.front_gun.temp_set -= TEMP_STEP;
                if (g_monitor.front_gun.temp_set < TEMP_RANGE_MIN)
                {
                    g_monitor.front_gun.temp_set = TEMP_RANGE_MIN;
                }
            }
            else
            {
                g_monitor.rear_chamber.temp_set -= TEMP_STEP;
                if (g_monitor.rear_chamber.temp_set < TEMP_RANGE_MIN)
                {
                    g_monitor.rear_chamber.temp_set = TEMP_RANGE_MIN;
                }
            }
            g_monitor.refresh_flag = 1;
        }
        break;
            
        case KEY_CONFIRM:  // KEY1对应确认键
            if (g_monitor.current_page == PAGE_FRONT_SET || 
                g_monitor.current_page == PAGE_REAR_SET)
            {
                if (g_monitor.state == STATE_DISPLAY)
                {
                    /* 进入设置模式 */
                    g_monitor.state = STATE_SETTING;
                    if (g_monitor.current_page == PAGE_FRONT_SET)
                    {
                        g_monitor.front_gun.temp_set = g_monitor.front_gun.target;
                    }
                    else
                    {
                        g_monitor.rear_chamber.temp_set = g_monitor.rear_chamber.target;
                    }
                }
                else
                {
                    /* 保存设置 */
                    if (g_monitor.current_page == PAGE_FRONT_SET)
                    {
                        g_monitor.front_gun.target = g_monitor.front_gun.temp_set;
                        SetFrontGunTargetTemp(g_monitor.front_gun.temp_set);
                    }
                    else
                    {
                        g_monitor.rear_chamber.target = g_monitor.rear_chamber.temp_set;
                        SetRearChamberTargetTemp(g_monitor.rear_chamber.temp_set);
                    }
                    g_monitor.state = STATE_DISPLAY;
                    g_monitor.save_flag = 1;
                    g_monitor.save_timer = SAVE_DISPLAY_TIME;
                }
                g_monitor.refresh_flag = 1;
            }
            break;
            
        case KEY_RETURN:  // KEY2对应返回键
            if (g_monitor.state == STATE_SETTING)
            {
                /* 退出设置模式，不保存 */
                g_monitor.state = STATE_DISPLAY;
                g_monitor.refresh_flag = 1;
            }
            break;
    }
}

/* 修改更新显示函数 */
void TempMonitor_UpdateDisplay(void)
{
    static DisplayBuffer_t disp_buf = {1, 0xFF};
    
    /* 判断是否需要全屏刷新 */
    if (disp_buf.last_page_type != g_monitor.current_page)
    {
        disp_buf.need_full_refresh = 1;
        disp_buf.last_page_type = g_monitor.current_page;
    }
    
    /* 更新温度数据 */
    g_monitor.front_gun.current = GetFrontGunTemp();
    g_monitor.rear_chamber.current = GetRearChamberTemp();
    
    /* 根据刷新需求选择更新方式 */
    if (disp_buf.need_full_refresh)
    {
        /* 全屏刷新 */
        clearscreen();
        disp_buf.need_full_refresh = 0;
    }
    
    /* 调用显示函数 */
    switch (g_monitor.current_page)
    {
        case PAGE_FRONT_TEMP:
        case PAGE_REAR_TEMP:
            Display_TempPage(g_monitor.current_page);
            break;
        case PAGE_FRONT_SET:
        case PAGE_REAR_SET:
            Display_SetPage(g_monitor.current_page);
            break;
    }
}


/**
 * @brief  主处理函数 - 在主循环中调用
 */
void TempMonitor_Process(void)
{
    /* 处理按键 */
    Process_Keys();
    
    /* 刷新显示 */
    if (g_monitor.refresh_flag) 
    {
        g_monitor.refresh_flag = 0;
        TempMonitor_UpdateDisplay();
    }
}

/* 修改 TempMonitor_TimerHandler 函数 */
void TempMonitor_TimerHandler(void)
{
    g_monitor.update_counter++;
    
    /* 降低刷新频率：200ms更新一次温度显示 */
    if (g_monitor.update_counter >= 200)
    {
        g_monitor.update_counter = 0;
        /* 只在显示温度页面时更新温度值 */
        if (g_monitor.current_page == PAGE_FRONT_TEMP || 
            g_monitor.current_page == PAGE_REAR_TEMP)
        {
            g_monitor.refresh_flag = 1;
        }
    }
    
    /* 设置页面需要更频繁刷新（闪烁效果） */
    if (g_monitor.state == STATE_SETTING)
    {
        if (g_monitor.update_counter % 50 == 0)  // 50ms刷新一次
        {
            g_monitor.refresh_flag = 1;
        }
    }
    
    /* 保存提示计时器处理 */
    if (g_monitor.save_timer > 0)
    {
        g_monitor.save_timer--;
        if (g_monitor.save_timer == 0)
        {
            g_monitor.save_flag = 0;
            g_monitor.refresh_flag = 1;
        }
    }
}


/**
 * @brief  获取前枪管温度
 * @retval 温度值(浮点数)
 */
__weak float GetFrontGunTemp(void)
{
    /* 这里添加实际的温度获取代码 */
    /* 例如：从ADC读取温度传感器数据并转换 */
    /* return ADC_GetTemp(FRONT_GUN_CHANNEL); */
    
    /* 示例：返回模拟数据 */
    static float temp = 25.0f;
    temp += 0.1f;
    if (temp > 30.0f) temp = 20.0f;
    return temp;
}

/**
 * @brief  获取后腔体温度
 * @retval 温度值(浮点数)
 */
__weak float GetRearChamberTemp(void)
{
    /* 这里添加实际的温度获取代码 */
    /* 例如：从ADC读取温度传感器数据并转换 */
    /* return ADC_GetTemp(REAR_CHAMBER_CHANNEL); */
    
    /* 示例：返回模拟数据 */
    static float temp = 24.0f;
    temp += 0.15f;
    if (temp > 32.0f) temp = 22.0f;
    return temp;
}

/**
 * @brief  设置前枪管目标温度
 * @param  temp: 目标温度值
 */
__weak void SetFrontGunTargetTemp(float temp)
{
    /* 这里添加实际的温度控制代码 */
    /* 例如：设置PID控制器目标值 */
    /* PID_SetTarget(FRONT_GUN_PID, temp); */
    
    /* 可以添加EEPROM保存功能 */
    /* EEPROM_SaveFloat(FRONT_TARGET_ADDR, temp); */
}

/**
 * @brief  设置后腔体目标温度
 * @param  temp: 目标温度值
 */
__weak void SetRearChamberTargetTemp(float temp)
{
    /* 这里添加实际的温度控制代码 */
    /* 例如：设置PID控制器目标值 */
    /* PID_SetTarget(REAR_CHAMBER_PID, temp); */
    
    /* 可以添加EEPROM保存功能 */
    /* EEPROM_SaveFloat(REAR_TARGET_ADDR, temp); */
}

