#include "key_manager.h"
#include "FM24CXX.h"

/* 全局变量 */
KeyManager_t g_key_manager = {0};

/**
 * @brief  初始化按键管理器
 */
void Key_Manager_Init(void)
{
    memset(&g_key_manager, 0, sizeof(KeyManager_t));
}

/**
 * @brief  发送UI消息
 */
void Send_UI_Message(MsgType_e type, uint8_t page_id, int16_t value, uint8_t refresh_type)
{
    UIMessage_t msg;
    msg.msg_type = type;
    msg.page_id = page_id;
    msg.new_value = value;
    msg.refresh_type = refresh_type;
    xQueueSend(UI_Queue, &msg, 0);
}

/**
 * @brief  处理浏览模式按键
 */
void Process_Browse_Mode_Key(uint8_t key)
{
    switch(key)
    {
        case KEY_UP:
            Handle_Page_Navigation(KEY_UP);
            break;
            
        case KEY_DOWN:
            Handle_Page_Navigation(KEY_DOWN);
            break;
            
        case KEY_CONFIRM:
            if(g_pages[g_current_page_id].is_editable)
            {
                /* 智能温控界面特殊处理 */
                if(g_current_page_id == PAGE_PID_CONTROL)
                {
                    Check_OK_Multiple_Press();
                }
                else
                {
                    /* 普通编辑页面 */
                    g_current_mode = MODE_EDIT;
                    Send_UI_Message(MSG_MODE_CHANGE, g_current_page_id, 0, 0);
                }
            }
            break;
            
        case KEY_RETURN:
            /* 浏览模式下ESC无操作 */
            break;
    }
}

/**
 * @brief  处理编辑模式按键
 */
void Process_Edit_Mode_Key(uint8_t key)
{
    switch(key)
    {
        case KEY_UP:
        case KEY_DOWN:
            Handle_Value_Adjustment(key);
            break;
            
        case KEY_CONFIRM:
            /* 编辑模式下OK无操作（数据已实时保存） */
            break;
            
        case KEY_RETURN:
            /* 退出编辑模式 */
            g_current_mode = MODE_BROWSE;
            Send_UI_Message(MSG_MODE_CHANGE, g_current_page_id, 0, 0);
            break;
    }
}

/**
 * @brief  处理页面导航
 */
void Handle_Page_Navigation(uint8_t key)
{
    uint8_t old_page = g_current_page_id;
    
    if(key == KEY_UP)
    {
        /* 前翻页 */
        if(g_current_page_id > PAGE_TEMP_DISPLAY)
        {
            g_current_page_id--;
        }
        else if(g_current_page_id == PAGE_TEMP_DISPLAY)
        {
            g_current_page_id = PAGE_STATUS_DISPLAY;  // 循环到最后页
        }
    }
    else if(key == KEY_DOWN)
    {
        /* 后翻页 */
        if(g_current_page_id < PAGE_STATUS_DISPLAY && 
           g_current_page_id != PAGE_STARTUP)
        {
            g_current_page_id++;
        }
        else if(g_current_page_id == PAGE_STATUS_DISPLAY)
        {
            g_current_page_id = PAGE_TEMP_DISPLAY;  // 循环到第一页
        }
    }
    
    /* 跳过自整定进度页面（只能从PID控制页面进入） */
    if(g_current_page_id == PAGE_AUTOTUNE_PROGRESS && 
       g_system_status.mode != MODE_AUTOTUNE)
    {
        if(key == KEY_UP)
            g_current_page_id--;
        else
            g_current_page_id++;
    }
    
    if(old_page != g_current_page_id)
    {
        Send_UI_Message(MSG_PAGE_CHANGE, g_current_page_id, 0, 1);
    }
}

/**
 * @brief  处理数值调整
 */
void Handle_Value_Adjustment(uint8_t key)
{
    int16_t old_value, new_value;
    
    old_value = g_pages[g_current_page_id].current_value;
    
    if(key == KEY_UP)
    {
        new_value = old_value + g_pages[g_current_page_id].step;
        if(new_value > g_pages[g_current_page_id].max_value)
            new_value = g_pages[g_current_page_id].max_value;
    }
    else  // KEY_DOWN
    {
        new_value = old_value - g_pages[g_current_page_id].step;
        if(new_value < g_pages[g_current_page_id].min_value)
            new_value = g_pages[g_current_page_id].min_value;
    }
    
    if(new_value != old_value)
    {
        g_pages[g_current_page_id].current_value = new_value;
        
        /* 保存设置值 */
        Save_Temperature_Setting(g_current_page_id, new_value);
        
        /* 发送更新消息 */
        Send_UI_Message(MSG_VALUE_UPDATE, g_current_page_id, new_value, 0);
    }
}

/**
 * @brief  检测OK键多次连击
 */
void Check_OK_Multiple_Press(void)
{
    uint32_t current_tick = osKernelSysTick();
    
    /* 首次按下或超时重置 */
    if(g_key_manager.ok_press_count == 0 || 
       (current_tick - g_key_manager.ok_first_press_tick) > OK_KEY_TIMEOUT)
    {
        g_key_manager.ok_press_count = 1;
        g_key_manager.ok_first_press_tick = current_tick;
    }
    else
    {
        g_key_manager.ok_press_count++;
        
        /* 达到连击次数，触发自整定 */
        if(g_key_manager.ok_press_count >= OK_KEY_COUNT_MAX)
        {
            g_key_manager.ok_press_count = 0;
            
            /* 启动自整定 */
            Start_Autotune();
            
            /* 切换到自整定进度页面 */
            g_current_page_id = PAGE_AUTOTUNE_PROGRESS;
            Send_UI_Message(MSG_PAGE_CHANGE, PAGE_AUTOTUNE_PROGRESS, 0, 1);
        }
    }
    
    /* 显示当前连击次数（可选） */
    Send_UI_Message(MSG_VALUE_UPDATE, PAGE_PID_CONTROL, 
                    g_key_manager.ok_press_count, 0);
}

/**
 * @brief  保存温度设置
 */
void Save_Temperature_Setting(uint8_t page_id, int16_t value)
{
    uint16_t eeprom_addr;
    
    switch(page_id)
    {
        case PAGE_GUN_SETTING:
            eeprom_addr = EEPROM_GUN_TEMP_ADDR;
            g_system_status.front_temp_sv = (float)value;
            /* 检查是否需要切换PID参数组 */
            Check_And_Switch_Group();
            break;
            
        case PAGE_CAVITY_SETTING:
            eeprom_addr = EEPROM_CAVITY_TEMP_ADDR;
            g_system_status.rear_temp_sv = (float)value;
            break;
            
        default:
            return;
    }
    
    /* 保存到EEPROM */
    FM_WriteWordseq(eeprom_addr, &value, 1);
}

