#include "key_manager.h"
#include "FM24CXX.h"

/* ȫ�ֱ��� */
KeyManager_t g_key_manager = {0};

/**
 * @brief  ��ʼ������������
 */
void Key_Manager_Init(void)
{
    memset(&g_key_manager, 0, sizeof(KeyManager_t));
}

/**
 * @brief  ����UI��Ϣ
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
 * @brief  �������ģʽ����
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
                /* �����¿ؽ������⴦�� */
                if(g_current_page_id == PAGE_PID_CONTROL)
                {
                    Check_OK_Multiple_Press();
                }
                else
                {
                    /* ��ͨ�༭ҳ�� */
                    g_current_mode = MODE_EDIT;
                    Send_UI_Message(MSG_MODE_CHANGE, g_current_page_id, 0, 0);
                }
            }
            break;
            
        case KEY_RETURN:
            /* ���ģʽ��ESC�޲��� */
            break;
    }
}

/**
 * @brief  ����༭ģʽ����
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
            /* �༭ģʽ��OK�޲�����������ʵʱ���棩 */
            break;
            
        case KEY_RETURN:
            /* �˳��༭ģʽ */
            g_current_mode = MODE_BROWSE;
            Send_UI_Message(MSG_MODE_CHANGE, g_current_page_id, 0, 0);
            break;
    }
}

/**
 * @brief  ����ҳ�浼��
 */
void Handle_Page_Navigation(uint8_t key)
{
    uint8_t old_page = g_current_page_id;
    
    if(key == KEY_UP)
    {
        /* ǰ��ҳ */
        if(g_current_page_id > PAGE_TEMP_DISPLAY)
        {
            g_current_page_id--;
        }
        else if(g_current_page_id == PAGE_TEMP_DISPLAY)
        {
            g_current_page_id = PAGE_STATUS_DISPLAY;  // ѭ�������ҳ
        }
    }
    else if(key == KEY_DOWN)
    {
        /* ��ҳ */
        if(g_current_page_id < PAGE_STATUS_DISPLAY && 
           g_current_page_id != PAGE_STARTUP)
        {
            g_current_page_id++;
        }
        else if(g_current_page_id == PAGE_STATUS_DISPLAY)
        {
            g_current_page_id = PAGE_TEMP_DISPLAY;  // ѭ������һҳ
        }
    }
    
    /* ��������������ҳ�棨ֻ�ܴ�PID����ҳ����룩 */
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
 * @brief  ������ֵ����
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
        
        /* ��������ֵ */
        Save_Temperature_Setting(g_current_page_id, new_value);
        
        /* ���͸�����Ϣ */
        Send_UI_Message(MSG_VALUE_UPDATE, g_current_page_id, new_value, 0);
    }
}

/**
 * @brief  ���OK���������
 */
void Check_OK_Multiple_Press(void)
{
    uint32_t current_tick = osKernelSysTick();
    
    /* �״ΰ��»�ʱ���� */
    if(g_key_manager.ok_press_count == 0 || 
       (current_tick - g_key_manager.ok_first_press_tick) > OK_KEY_TIMEOUT)
    {
        g_key_manager.ok_press_count = 1;
        g_key_manager.ok_first_press_tick = current_tick;
    }
    else
    {
        g_key_manager.ok_press_count++;
        
        /* �ﵽ�������������������� */
        if(g_key_manager.ok_press_count >= OK_KEY_COUNT_MAX)
        {
            g_key_manager.ok_press_count = 0;
            
            /* ���������� */
            Start_Autotune();
            
            /* �л�������������ҳ�� */
            g_current_page_id = PAGE_AUTOTUNE_PROGRESS;
            Send_UI_Message(MSG_PAGE_CHANGE, PAGE_AUTOTUNE_PROGRESS, 0, 1);
        }
    }
    
    /* ��ʾ��ǰ������������ѡ�� */
    Send_UI_Message(MSG_VALUE_UPDATE, PAGE_PID_CONTROL, 
                    g_key_manager.ok_press_count, 0);
}

/**
 * @brief  �����¶�����
 */
void Save_Temperature_Setting(uint8_t page_id, int16_t value)
{
    uint16_t eeprom_addr;
    
    switch(page_id)
    {
        case PAGE_GUN_SETTING:
            eeprom_addr = EEPROM_GUN_TEMP_ADDR;
            g_system_status.front_temp_sv = (float)value;
            /* ����Ƿ���Ҫ�л�PID������ */
            Check_And_Switch_Group();
            break;
            
        case PAGE_CAVITY_SETTING:
            eeprom_addr = EEPROM_CAVITY_TEMP_ADDR;
            g_system_status.rear_temp_sv = (float)value;
            break;
            
        default:
            return;
    }
    
    /* ���浽EEPROM */
    FM_WriteWordseq(eeprom_addr, &value, 1);
}

