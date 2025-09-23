#include "key_manager.h"

/* ȫ�ֱ������� */
KeyPressCounter_t g_key_counter = {0};

/**
 * @brief  ��ʼ������������
 */
void Key_Manager_Init(void)
{
    g_key_counter.ok_press_count = 0;
    g_key_counter.last_press_time = 0;
    g_key_counter.autotune_triggered = 0;
}

/**
 * @brief  ����ǹ������ģʽ����
 */
void Process_Gun_Select_Mode_Key(uint8_t key)
{
    switch(key)
    {
        case KEY_UP:  /* ǰ��ҳ */
            Handle_Gun_Select(KEY_UP);
            break;
            
        case KEY_DOWN:/* ��ҳ */
            Handle_Gun_Select(KEY_DOWN);
            break;
            
        case KEY_CONFIRM:/* ȷ�ϼ� */
//            if(g_pages[g_current_page_id].is_editable)
//            {
//                /* �����¿�ҳ�����⴦�� */
//                if(g_current_page_id == PAGE_GUN_SELECT)
//                {
//                    Send_UI_Message(MSG_PAGE_CHANGE, g_current_page_id, 0, 1);
//                }
//            }
//            break;
            
        case KEY_RETURN:  /* ESC�� */
            /* �¶���ʾҳ�棬��ESC������ǹ��ѡ��ҳ��             */
            if(g_pages[g_current_page_id].is_editable && g_current_page_id == PAGE_GUN_SELECT)
            {
                /* �¶���ʾҳ�����ǹ��ѡ��ҳ�� */
                g_current_mode = MODE_BROWSE;
                g_current_page_id = PAGE_TEMP_DISPLAY;
                Send_UI_Message(MSG_PAGE_CHANGE, g_current_page_id, 0, 0);    
            }
            break;
    }
}
/**
 * @brief  �������ģʽ����
 */
void Process_Browse_Mode_Key(uint8_t key)
{
//    UIMessage_t msg;
    switch(key)
    {
        case KEY_UP:  /* ǰ��ҳ */
            Handle_Page_Navigation(KEY_UP);
            break;
            
        case KEY_DOWN:  /* ��ҳ */
            Handle_Page_Navigation(KEY_DOWN);
            break;
            
        case KEY_CONFIRM:  /* ȷ�ϼ� */
            if(g_pages[g_current_page_id].is_editable)
            {
                /* �����¿�ҳ�����⴦�� */
                if(g_current_page_id == PAGE_SMART_CONTROL)
                {
                    /* ����Ƿ񴥷������� */
                    Check_Autotune_Trigger();
                }
                else
                {
                    /* ��ͨҳ�����༭ģʽ */
                    g_current_mode = MODE_EDIT;
                    Send_UI_Message(MSG_MODE_CHANGE, g_current_page_id, 0, 0);
                }
            }
            break;
            
        case KEY_RETURN:  /* ESC�� */
            /* �¶���ʾҳ�棬��ESC������ǹ��ѡ��ҳ��             */
            if(g_current_page_id == PAGE_TEMP_DISPLAY)
            {
                /* �¶���ʾҳ�����ǹ��ѡ��ҳ�� */
                g_current_mode = MODE_SELECT;
                g_current_page_id = PAGE_GUN_SELECT;
                Send_UI_Message(MSG_MODE_CHANGE, g_current_page_id, 0, 0);    
            }

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
        case KEY_UP:  /* ������ֵ */
            Handle_Value_Adjustment(KEY_UP);
            break;
            
        case KEY_DOWN:  /* ������ֵ */
            Handle_Value_Adjustment(KEY_DOWN);
            break;
            
        case KEY_CONFIRM:  /* ȷ�ϼ� */
            /* �����¿ؽ�������⴦�� */
            if(g_current_page_id == PAGE_SMART_CONTROL)
            {
                Check_Autotune_Trigger();
            }
            break;
            
        case KEY_RETURN:  /* �˳��༭ */
            g_current_mode = MODE_BROWSE;
            Reset_Key_Counter();  /* �˳�ʱ���ü��� */
            Send_UI_Message(MSG_MODE_CHANGE, g_current_page_id, 0, 0);
            break;
    }
}

/**
 * @brief  ����������ģʽ����
 */
void Process_Autotune_Key(uint8_t key)
{
	switch(key)
		{
			case KEY_UP:
			case KEY_DOWN:
			case KEY_CONFIRM:
				/* �����������У�������Щ���� */
				/* ��ѡ����������ʾ������Ч */
				// HAL_GPIO_WritePin(BELL_GPIO_Port, BELL_Pin, GPIO_PIN_SET);
				// osDelay(100);
				// HAL_GPIO_WritePin(BELL_GPIO_Port, BELL_Pin, GPIO_PIN_RESET);
				break;
				
			case KEY_RETURN:  /* ֻ��ESC����Ч */
				/* ȷ���Ƿ�Ҫֹͣ������ */
				Stop_Autotune();
				g_current_mode = MODE_BROWSE;
				g_current_page_id = PAGE_SMART_CONTROL;
				g_key_counter.autotune_triggered = 0;
				Reset_Key_Counter();
				
				/* ���������״̬ */
				g_system_status.autotune_complete = 0;
				g_system_status.mode = PID_MODE_RUN;
				
				/* ���������¿�ҳ�� */
				Send_UI_Message(MSG_PAGE_CHANGE, PAGE_SMART_CONTROL, 0, 1);
				break;
		}

}

/**
 * @brief  ����ǹ��ѡ����水������
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
              g_current_gun_id = POLL_MERCURY;  /* ѭ�������ҳ */
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
            g_current_gun_id = POLL_H2SO4_MIST;  /* ѭ������һҳ */
        }
    }
   Send_UI_Message(MSG_PAGE_CHANGE, g_current_page_id, 0, 1);
//   Reset_Key_Counter();
}

/**
 * @brief  ����ҳ�浼��
 */
void Handle_Page_Navigation(uint8_t key)
{
    uint8_t old_page = g_current_page_id;
    
    if(key == KEY_UP)  /* ǰ��ҳ */
    {
        if(g_current_page_id > PAGE_TEMP_DISPLAY)
        {
            g_current_page_id--;
        }
        else if(g_current_page_id == PAGE_TEMP_DISPLAY)
        {
            g_current_page_id = PAGE_DIOXIN_DISPLAY;  /* ѭ�������ҳ */
        }
    }
    else if(key == KEY_DOWN)  /* ��ҳ */
    {
        if(g_current_page_id < PAGE_DIOXIN_DISPLAY && 
           g_current_page_id != PAGE_STARTUP)
        {
            g_current_page_id++;
        }
        else if(g_current_page_id == PAGE_DIOXIN_DISPLAY)
        {
            g_current_page_id = PAGE_TEMP_DISPLAY;  /* ѭ������һҳ */
        }
    }
    
    /* �л�ҳ��ʱ����OK������ */
    if(old_page != g_current_page_id)
    {
        Reset_Key_Counter();
        Send_UI_Message(MSG_PAGE_CHANGE, g_current_page_id, 0, 1);
    }
}

/**
 * @brief  ������ֵ����
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
        
        /* ���������EEPROM */
        if(g_current_page_id == PAGE_GUN_SETTING)
        {
            g_system_status.front_temp_sv = (float)new_value;
            Check_And_Switch_Group();  /* ����Ƿ���Ҫ�л������� */
        }
        else if(g_current_page_id == PAGE_CAVITY_SETTING)
        {
            g_system_status.rear_temp_sv = (float)new_value;
			Check_And_Switch_Group();  /* ����Ƿ���Ҫ�л������� */
        }
        
        Send_UI_Message(MSG_VALUE_UPDATE, g_current_page_id, new_value, 0);
    }
}

/**
 * @brief  ����Ƿ񴥷�������
 */
void Check_Autotune_Trigger(void)
{
    uint32_t current_time = osKernelSysTick();
    
    /* ��ʱ���ã�10����Ҫ����7�Σ� */
    if((current_time - g_key_counter.last_press_time) > OK_KEY_TIMEOUT)
    {
        g_key_counter.ok_press_count = 0;
    }
    
    g_key_counter.ok_press_count++;
    g_key_counter.last_press_time = current_time;
    
    /* ���½�����ʾ�������� */
    Send_UI_Message(MSG_VALUE_UPDATE, PAGE_SMART_CONTROL, 
                   g_key_counter.ok_press_count, 0);
    
    /* ����Ƿ�ﵽ�������� */
    if(g_key_counter.ok_press_count >= OK_KEY_COUNT_MAX && !g_key_counter.autotune_triggered)
    {
        /* ���������� */
        Start_Autotune();
        g_current_mode = MODE_AUTOTUNE;
        g_key_counter.autotune_triggered = 1;
        
        /* �л���������ʾ���� */
        Send_UI_Message(MSG_MODE_CHANGE, PAGE_SMART_CONTROL, 0, 1);
    }
}

/**
 * @brief  ���ð���������
 */
void Reset_Key_Counter(void)
{
    g_key_counter.ok_press_count = 0;
    g_key_counter.last_press_time = 0;
}

/**
 * @brief  ����UI��Ϣ
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


