#include "ui_manager.h"

/* 全局变量 */
SystemMode_e g_current_mode = MODE_BROWSE;
uint8_t g_current_page_id = PAGE_STARTUP;
PageData_t g_pages[PAGE_MAX];
int16_t g_gun_temp_measured = 0;
int16_t g_cavity_temp_measured = 0;

/**
 * @brief  初始化页面数据
 */
void init_page_data(void)
{
    /* 启动页面 */
    g_pages[PAGE_STARTUP].page_id = PAGE_STARTUP;
    g_pages[PAGE_STARTUP].is_editable = false;
    g_pages[PAGE_STARTUP].current_value = 0;
    
    /* 温度显示页面 */
    g_pages[PAGE_TEMP_DISPLAY].page_id = PAGE_TEMP_DISPLAY;
    g_pages[PAGE_TEMP_DISPLAY].is_editable = false;
    g_pages[PAGE_TEMP_DISPLAY].current_value = 0;
    
    /* 枪管温度设置 */
    g_pages[PAGE_GUN_SETTING].page_id = PAGE_GUN_SETTING;
    g_pages[PAGE_GUN_SETTING].is_editable = true;
    g_pages[PAGE_GUN_SETTING].current_value = 101;
    g_pages[PAGE_GUN_SETTING].min_value = 50;
    g_pages[PAGE_GUN_SETTING].max_value = 200;
    g_pages[PAGE_GUN_SETTING].step = 5;
    
    /* 腔体温度设置 */
    g_pages[PAGE_CAVITY_SETTING].page_id = PAGE_CAVITY_SETTING;
    g_pages[PAGE_CAVITY_SETTING].is_editable = true;
    g_pages[PAGE_CAVITY_SETTING].current_value = 120;
    g_pages[PAGE_CAVITY_SETTING].min_value = 50;
    g_pages[PAGE_CAVITY_SETTING].max_value = 200;
    g_pages[PAGE_CAVITY_SETTING].step = 5;
    
    /* 智能温控调节页面 */
    g_pages[PAGE_PID_CONTROL].page_id = PAGE_PID_CONTROL;
    g_pages[PAGE_PID_CONTROL].is_editable = true;  // 特殊处理，用于检测OK键
    g_pages[PAGE_PID_CONTROL].current_value = 0;
    
    /* 自整定进度页面 */
    g_pages[PAGE_AUTOTUNE_PROGRESS].page_id = PAGE_AUTOTUNE_PROGRESS;
    g_pages[PAGE_AUTOTUNE_PROGRESS].is_editable = false;
    g_pages[PAGE_AUTOTUNE_PROGRESS].current_value = 0;
    
    /* 状态显示页面 */
    g_pages[PAGE_STATUS_DISPLAY].page_id = PAGE_STATUS_DISPLAY;
    g_pages[PAGE_STATUS_DISPLAY].is_editable = false;
    g_pages[PAGE_STATUS_DISPLAY].current_value = 0;
}

/**
 * @brief  显示页面
 */
void Display_Page(uint8_t page_id)
{
    clearscreen();
    
    switch(page_id)
    {
        case PAGE_STARTUP:
            Display_Startup_Page();
            break;
            
        case PAGE_TEMP_DISPLAY:
            Display_Temp_Page();
            break;
            
        case PAGE_GUN_SETTING:
            Display_Gun_Setting_Page();
            break;
            
        case PAGE_CAVITY_SETTING:
            Display_Cavity_Setting_Page();
            break;
            
        case PAGE_PID_CONTROL:
            Display_PID_Control_Page();
            break;
            
        case PAGE_AUTOTUNE_PROGRESS:
            Display_Autotune_Progress_Page();
            break;
            
        case PAGE_STATUS_DISPLAY:
            Display_Status_Page();
            break;
    }
}

/**
 * @brief  显示启动页面
 */
void Display_Startup_Page(void)
{
    DispString(40, 12, "MH5030", false);
    DispString(28, 36, "Ver 1.0.0", false);
}

/**
 * @brief  显示温度页面
 */
void Display_Temp_Page(void)
{
    DispString12(30, 0, "测量值", false);
    DispString12(80, 0, "设定值", false);
    DispString12(0, 24, "枪管", false);
    DispString12(0, 48, "腔体", false);
    draw_hline(1, 127, 16);
    
    /* 显示测量值 */
    get_test_temperatures(&g_gun_temp_measured, &g_cavity_temp_measured);
    Disp_Word_UM(38, 24, 3, g_gun_temp_measured, 0, 0);
    Disp_Word_UM(38, 48, 3, g_cavity_temp_measured, 0, 0);
    
    /* 显示设定值 */
    Disp_Word_UM(88, 24, 3, g_pages[PAGE_GUN_SETTING].current_value, 0, 0);
    Disp_Word_UM(88, 48, 3, g_pages[PAGE_CAVITY_SETTING].current_value, 0, 0);
}

/**
 * @brief  显示枪管温度设置页面
 */
void Display_Gun_Setting_Page(void)
{
    DispString(16, 0, "枪管温度设置", false);
    draw_hline(1, 127, 20);
    Show_Word_U(48, 32, g_pages[PAGE_GUN_SETTING].current_value, 3, 0, false);
    DispString(74, 32, "℃", false);
}

/**
 * @brief  显示腔体温度设置页面
 */
void Display_Cavity_Setting_Page(void)
{
    DispString(16, 0, "腔体温度设置", false);
    draw_hline(1, 127, 20);
    Show_Word_U(48, 32, g_pages[PAGE_CAVITY_SETTING].current_value, 3, 0, false);
    DispString(74, 32, "℃", false);
}

/**
 * @brief  显示智能温控调节页面
 */
void Display_PID_Control_Page(void)
{
    DispString(16, 0, "智能温控调节", false);
    draw_hline(1, 127, 20);
    
    /* 显示当前PID参数组 */
    DispString12(10, 28, "当前组:", false);
    switch(g_system_status.param_group)
    {
        case PARAM_GROUP_120:
            DispString12(58, 28, "120℃", false);
            break;
        case PARAM_GROUP_160:
            DispString12(58, 28, "160℃", false);
            break;
        case PARAM_GROUP_220:
            DispString12(58, 28, "220℃", false);
            break;
    }
    
    /* 显示提示信息 */
    DispString12(10, 46, "连按OK键7次自整定", false);
}

/**
 * @brief  显示自整定进度页面
 */
void Display_Autotune_Progress_Page(void)
{
    DispString(16, 0, "自整定进行中", false);
    draw_hline(1, 127, 20);
    
    /* 显示进度 */
    DispString12(10, 28, "前枪管:", false);
    Disp_Word_US(58, 28, 1, g_system_status.front_cross_cnt, 0, 0);
    DispString12(70, 28, "/5", false);
    
    DispString12(10, 46, "腔体:", false);
    Disp_Word_US(58, 46, 1, g_system_status.rear_cross_cnt, 0, 0);
    DispString12(70, 46, "/5", false);
    
    /* 进度条 */
    uint8_t progress = (g_system_status.front_cross_cnt + g_system_status.rear_cross_cnt) * 10;
    draw_rect(20, 56, 88, 6, false);
    if(progress > 0)
    {
        draw_rect(22, 58, (progress * 84) / 100, 2, true);
    }
}

/**
 * @brief  显示状态页面
 */
void Display_Status_Page(void)
{
    DispString(24, 0, "系统状态", false);
    draw_hline(1, 127, 16);
    
    /* TODO: 显示系统状态信息 */
    // 显示风扇状态
    // DispString12(0, 20, "风扇:", false);
    // Show_Word_U(36, 20, get_fan_rpm(), 4, 0, false);
    // DispString12(72, 20, "RPM", false);
    
    // 显示加热状态
    // DispString12(0, 36, "加热:", false);
    // DispString12(36, 36, get_heater_status() ? "开启" : "关闭", false);
    
    // 显示错误代码
    // DispString12(0, 52, "错误:", false);
    // Show_Word_U(36, 52, g_system_status.error_code, 3, 0, false);
    
    /* 临时显示 */
    DispString12(20, 30, "系统运行正常", false);
}

/**
 * @brief  更新数值显示
 */
void Update_Value_Display(uint8_t page_id, int16_t value, bool highlight)
{
    switch(page_id)
    {
        case PAGE_GUN_SETTING:
            Show_Word_U(48, 32, value, 3, 0, highlight);
            break;
            
        case PAGE_CAVITY_SETTING:
            Show_Word_U(48, 32, value, 3, 0, highlight);
            break;
            
        case PAGE_PID_CONTROL:
            /* 显示OK键按下次数（可选） */
            if(value > 0 && value < 7)
            {
                Disp_Word_US(100, 46, 1, value, 0, 0);
            }
            break;
    }
}

/**
 * @brief  更新实时数据
 */
void Update_Realtime_Data(void)
{
    if(g_current_page_id == PAGE_TEMP_DISPLAY)
    {
        /* 更新温度测量值 */
        get_test_temperatures(&g_gun_temp_measured, &g_cavity_temp_measured);
        Disp_Word_UM(38, 24, 3, g_gun_temp_measured, 0, 0);
        Disp_Word_UM(38, 48, 3, g_cavity_temp_measured, 0, 0);
    }
    else if(g_current_page_id == PAGE_AUTOTUNE_PROGRESS)
    {
        /* 更新自整定进度 */
        Update_System_Status();
        Display_Autotune_Progress_Page();
        
        /* 检查是否完成 */
        if(g_system_status.autotune_complete)
        {
            /* 返回PID控制页面 */
            g_current_page_id = PAGE_PID_CONTROL;
            Display_Page(PAGE_PID_CONTROL);
        }
    }
}

/**
 * @brief  测试用温度获取函数
 */
void get_test_temperatures(int16_t *gun_temp, int16_t *cavity_temp)
{
    static int16_t test_gun = 95;
    static int16_t test_cavity = 115;
    
    test_gun += (rand() % 5 - 2);
    test_cavity += (rand() % 5 - 2);
    
    if(test_gun < 90) test_gun = 90;
    if(test_gun > 110) test_gun = 110;
    if(test_cavity < 110) test_cavity = 110;
    if(test_cavity > 130) test_cavity = 130;
    
    *gun_temp = test_gun;
    *cavity_temp = test_cavity;
}


