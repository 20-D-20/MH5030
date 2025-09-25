#include "key_manager.h"

/* 全局变量定义 */
KeyPressCounter_t g_stOkCntrAutotune = {0};          /* 记录进入自增定模式的OK按键次数 */
KeyPressCounter_t g_stEscCntrGun = {0};              /* 记录进入枪管选择模式ESC键次数 */

/**
 * @brief  初始化按键管理器
 */
void Key_Manager_Init(void)
{
    g_stOkCntrAutotune.autotune_triggered = 0 ;
    g_stOkCntrAutotune.esc_press_count = 0;
    g_stOkCntrAutotune.last_press_time = 0;
    g_stOkCntrAutotune.ok_press_count = 0;
    
    g_stEscCntrGun.esc_press_count = 0 ;
    g_stEscCntrGun.last_press_time = 0;
    g_stEscCntrGun.ok_press_count = 0 ;
}

/**
 * @brief  处理枪管设置模式按键
 */
void Process_Gun_Select_Mode_Key(uint8_t key)
{
    switch(key)
    {
        case KEY_UP:  /* 前翻页 */
            Handle_Gun_Select(KEY_UP);
            break;
            
        case KEY_DOWN:/* 后翻页 */
            Handle_Gun_Select(KEY_DOWN);
            break;
            
        case KEY_CONFIRM:/* 确认键 */
            /* 未实现功能 */
            break;
        
        case KEY_RETURN:  /* ESC键 */
            /* 在枪管选择页面按下ESC回退到温度显示页面                */
            if(g_pages[g_current_page_id].is_editable && g_current_page_id == PAGE_GUN_SELECT)
            {
                /* 枪管选择页面切换温度显示页面 */
                g_current_mode = MODE_BROWSE;
                g_current_page_id = PAGE_TEMP_DISPLAY;
                Send_UI_Message(MSG_PAGE_CHANGE, g_current_page_id, 0, 0);    
            }
            break;
    }
}
/**
 * @brief  处理浏览模式按键
 */
void Process_Browse_Mode_Key(uint8_t key)
{
//    UIMessage_t msg;
    switch(key)
    {
        case KEY_UP:  /* 前翻页 */
            Handle_Page_Navigation(KEY_UP);
            break;
            
        case KEY_DOWN:  /* 后翻页 */
            Handle_Page_Navigation(KEY_DOWN);
            break;
            
        case KEY_CONFIRM:  /* 确认键 */
            if(g_pages[g_current_page_id].is_editable)
            {
                /* 智能温控页面特殊处理 */
                if(g_current_page_id == PAGE_SMART_CONTROL)
                {
                    /* 检查是否触发自整定 */
                    Check_Autotune_Trigger();
                }
                else
                {
                    /* 普通页面进入编辑模式 */
                    g_current_mode = MODE_EDIT;
                    Send_UI_Message(MSG_MODE_CHANGE, g_current_page_id, 0, 0);
                }
            }
            break;
            
        case KEY_RETURN:  /* ESC键 */
            /* 温度显示页面，按ESC键进入枪管选择页面             */
            if(g_current_page_id == PAGE_TEMP_DISPLAY)
            {
               /* 检测进入枪管选择界面 */
               Check_GunSelect_Trigger(); 
            }

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
        case KEY_UP:  /* 增加数值 */
            Handle_Value_Adjustment(KEY_UP);
            break;
            
        case KEY_DOWN:  /* 减少数值 */
            Handle_Value_Adjustment(KEY_DOWN);
            break;
            
        case KEY_CONFIRM:  /* 确认键 */
            /* 智能温控界面的特殊处理 */
            if(g_current_page_id == PAGE_SMART_CONTROL)
            {
                Check_Autotune_Trigger();
            }
            break;
            
        case KEY_RETURN:  /* 退出编辑 */
            g_current_mode = MODE_BROWSE;
            Reset_Key_Counter(&g_stOkCntrAutotune);  /* 退出时重置计数 */
            Send_UI_Message(MSG_MODE_CHANGE, g_current_page_id, 0, 0);
            break;
    }
}

/**
 * @brief  处理自整定模式按键
 */
void Process_Autotune_Key(uint8_t key)
{
    switch(key)
        {
            case KEY_UP:
            case KEY_DOWN:
            case KEY_CONFIRM:
                /* 自整定进行中，忽略这些按键 */
                /* 可选：蜂鸣器提示操作无效 */
                // HAL_GPIO_WritePin(BELL_GPIO_Port, BELL_Pin, GPIO_PIN_SET);
                // osDelay(100);
                // HAL_GPIO_WritePin(BELL_GPIO_Port, BELL_Pin, GPIO_PIN_RESET);
                break;
                
            case KEY_RETURN:  /* 只有ESC键有效 */
                /* 确认是否要停止自整定 */
                Stop_Autotune();
                g_current_mode = MODE_BROWSE;
                g_current_page_id = PAGE_SMART_CONTROL;
                g_stOkCntrAutotune.autotune_triggered = 0;
                Reset_Key_Counter(&g_stOkCntrAutotune);
                
                /* 清除自整定状态 */
                g_system_status.autotune_complete = 0;
                g_system_status.mode = PID_MODE_RUN;
                /* 返回智能温控页面 */
                Send_UI_Message(MSG_PAGE_CHANGE, PAGE_SMART_CONTROL, 0, 1);
                break;
        }

}

/**
 * @brief  处理枪管选择界面按键操作
 */
void Handle_Gun_Select(uint8_t key)
{
    if(key == KEY_UP)
    {
        if(g_current_gun_id > POLL_H2SO4_MIST)
          {
              g_current_gun_id--;
          }
          else if(g_current_gun_id == POLL_H2SO4_MIST)
          {
              g_current_gun_id = POLL_MERCURY;  /* 循环到最后一个选项 */
          }
        
    }
    else if(key == KEY_DOWN)
    {
        if(g_current_gun_id < POLL_MERCURY)
        {
            g_current_gun_id++;
        }
        else if(g_current_gun_id == POLL_MERCURY)
        {
            g_current_gun_id = POLL_H2SO4_MIST;  /* 循环到第一个选项 */
        }
    }

   /* 更新页面显示 */
   Send_UI_Message(MSG_PAGE_CHANGE, g_current_page_id, 0, 1);

}

/**
 * @brief  处理页面导航
 */
void Handle_Page_Navigation(uint8_t key)
{
    uint8_t old_page = g_current_page_id;
    
    if(key == KEY_UP)  /* 前翻页 */
    {
        if(g_current_page_id > PAGE_TEMP_DISPLAY)
        {
            g_current_page_id--;
        }
        else if(g_current_page_id == PAGE_TEMP_DISPLAY)
        {
            g_current_page_id = PAGE_Airflow_DISPLAY;  /* 循环到最后页 */
        }
    }
    else if(key == KEY_DOWN)  /* 后翻页 */
    {
        if(g_current_page_id < PAGE_Airflow_DISPLAY && 
           g_current_page_id != PAGE_STARTUP)
        {
            g_current_page_id++;
        }
        else if(g_current_page_id == PAGE_Airflow_DISPLAY)
        {
            g_current_page_id = PAGE_TEMP_DISPLAY;  /* 循环到第一页 */
        }
    }
    
    /* 切换页面时重置OK键计数 */
    if(old_page != g_current_page_id)
    {
        Reset_Key_Counter(&g_stOkCntrAutotune);
        Reset_Key_Counter(&g_stEscCntrGun);
        /* 页面切换 */
        Send_UI_Message(MSG_PAGE_CHANGE, g_current_page_id, 0, 1);
    }
}

/**
 * @brief  处理数值调整
 */
void Handle_Value_Adjustment(uint8_t key)
{
    int16_t old_value = g_pages[g_current_page_id].current_value;
    int16_t new_value = old_value;
    
    if(key == KEY_UP)
    {
        new_value = old_value + g_pages[g_current_page_id].step;
        if(new_value > g_pages[g_current_page_id].max_value)
            new_value = g_pages[g_current_page_id].max_value;
    }
    else if(key == KEY_DOWN)
    {
        new_value = old_value - g_pages[g_current_page_id].step;
        if(new_value < g_pages[g_current_page_id].min_value)
            new_value = g_pages[g_current_page_id].min_value;
    }
    
    if(new_value != old_value)
    {
        g_pages[g_current_page_id].current_value = new_value;
        
        /* 保存参数到EEPROM */
        if(g_current_page_id == PAGE_GUN_SETTING)
        {
            g_system_status.front_temp_sv = (float)new_value;
            Check_And_Switch_Group();                            /* 检查是否需要切换参数组 */
        }
        else if(g_current_page_id == PAGE_CAVITY_SETTING)
        {
            g_system_status.rear_temp_sv = (float)new_value;
			Check_And_Switch_Group();                            /* 检查是否需要切换参数组 */
        }
        
        Send_UI_Message(MSG_VALUE_UPDATE, g_current_page_id, new_value, 0);
    }
}

/**
 * @brief  检查进入枪管选择页面是否触发
 */
void Check_GunSelect_Trigger(void)
{
    uint32_t current_time = osKernelSysTick();
    
    /* 超时重置（1秒内要按完2次） */
    if((current_time - g_stEscCntrGun.last_press_time) > ESC_KEY_TIMEOUT)
    {
        g_stEscCntrGun.esc_press_count = 0;
    }
    
    g_stEscCntrGun.esc_press_count++;
    g_stEscCntrGun.last_press_time = current_time;
    
    /* 检查是否达到触发条件 */
    if(g_stEscCntrGun.esc_press_count >= ESC_KEY_COUNT_MAX)
    {
        /* 温度显示页面进入枪管选择页面 */
        g_current_mode = MODE_SELECT;
        g_current_page_id = PAGE_GUN_SELECT;
        
        /* 清空ESC按键值 */
        Reset_Key_Counter(&g_stEscCntrGun);
 
        /* 进行页面跳转 */
        Send_UI_Message(MSG_MODE_CHANGE, g_current_page_id, 0, 0);  
    }
}


/**
 * @brief  检查是否触发自整定
 */
void Check_Autotune_Trigger(void)
{
    uint32_t current_time = osKernelSysTick();
    
    /* 超时重置（10秒内要按完7次） */
    if((current_time - g_stOkCntrAutotune.last_press_time) > OK_KEY_TIMEOUT)
    {
        g_stOkCntrAutotune.ok_press_count = 0;
    }
    
    g_stOkCntrAutotune.ok_press_count++;
    g_stOkCntrAutotune.last_press_time = current_time;
    
    /* 更新界面显示按键次数 */
    Send_UI_Message(MSG_VALUE_UPDATE, PAGE_SMART_CONTROL, 
                   g_stOkCntrAutotune.ok_press_count, 0);
    
    /* 检查是否达到触发条件 */
    if(g_stOkCntrAutotune.ok_press_count >= OK_KEY_COUNT_MAX && !g_stOkCntrAutotune.autotune_triggered)
    {
        /* 启动自整定 */
        Start_Autotune();
        /* 修改模式为自整定模式 */
        g_current_mode = MODE_AUTOTUNE;
        g_stOkCntrAutotune.autotune_triggered = 1;
        
        /* 切换到进度显示界面 */
        Send_UI_Message(MSG_MODE_CHANGE, PAGE_SMART_CONTROL, 0, 1);
    }
}

/**
 * @brief  重置按键计数器
 */
void Reset_Key_Counter(KeyPressCounter_t *key_counter)
{
    key_counter->ok_press_count = 0;
    key_counter->last_press_time = 0;
    key_counter->esc_press_count = 0;
}

/**
 * @brief  发送UI消息
 */
void Send_UI_Message(MsgType_e type, uint8_t page_id, int16_t value, uint8_t refresh)
{
    UIMessage_t msg;
    msg.msg_type = type;
    msg.page_id = page_id;
    msg.new_value = value;
    msg.refresh_type = refresh;
    xQueueSend(UI_Queue, &msg, 0);
}


