#include "temp_monitor.h"

/* ȫ�ֱ��� */
static TempMonitor_t g_monitor;

/* ҳ����� */
static const char* page_titles[PAGE_MAX_NUM] = 
{
    "ǰǹ���¶�",
    "��ǻ���¶�",
    "ǰǹ������",
    "��ǻ������"
};

/**
 * @brief  ϵͳ��ʼ��
 */
void TempMonitor_Init(void)
{
    /* ��ʼ��OLED��ʾ�� */
    SSD1305_init();
    clearscreen();
    
    /* ��ʼ��ϵͳ״̬ */
    g_monitor.current_page = PAGE_FRONT_TEMP;
    g_monitor.state = STATE_DISPLAY;
    
    /* ��ʼ���¶Ȳ��� */
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

/* �޸� Display_TempPage ���� */
static void Display_TempPage(uint8_t page_num)
{
    char str_buf[32];
    float current_temp, target_temp;
    const char* title;
    static uint8_t last_page = 0xFF;  // ��¼�ϴ�ҳ��
    static float last_current = -999.0f;  // ��¼�ϴ��¶�
    static float last_target = -999.0f;
    
    /* ����ҳ��ѡ������ */
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
    
    /* ֻ��ҳ���л�ʱ���� */
    if (last_page != page_num)
    {
        clearscreen();
        last_page = page_num;
        last_current = -999.0f;  // ǿ��ˢ����������
        last_target = -999.0f;
        
        /* ��ʾ��̬���ݣ����⡢��ǩ���ָ��ߣ� */
        uint8_t title_x = (128 - strlen(title) * 8) / 2;
        DispString(title_x, 0,(unsigned char*) title, false);
        
//        /* ���ָ��ߣ�ֻ�軭һ�Σ� */
//        for (uint8_t i = 0; i < 128; i++)
//        {
//            Set_Addr(1, i);
//            Write_Data(0x01);
//            Set_Addr(6, i);
//            Write_Data(0x80);
//        }
        
        /* ��ʾ��̬��ǩ */
        DispString(8, 16,(unsigned char*) "��ǰ:", false);
        DispString(8, 32, (unsigned char*)"Ŀ��:", false);
    }
    
    /* ֻ���±仯����ֵ���� */
    if (fabs(current_temp - last_current) > 0.01f)
    {
        /* �������ֵ���򣨾ֲ������ */
        for (uint8_t i = 48; i < 104; i++)
        {
            Set_Addr(2, i);
            Write_Data(0x00);
            Set_Addr(3, i);
            Write_Data(0x00);
        }
        /* ��ʾ����ֵ */
        sprintf(str_buf, "%.1f", current_temp);
        DispString(48, 16, (unsigned char*)str_buf, false);
        Disp_Char(88, 16, 0xA1, false);
        Disp_Char(96, 16, 'C', false);
        last_current = current_temp;
    }
    
    if (fabs(target_temp - last_target) > 0.01f)
    {
        /* �������ֵ���� */
        for (uint8_t i = 48; i < 104; i++)
        {
            Set_Addr(4, i);
            Write_Data(0x00);
            Set_Addr(5, i);
            Write_Data(0x00);
        }
        /* ��ʾ����ֵ */
        sprintf(str_buf, "%.1f", target_temp);
        DispString(48, 32, (unsigned char*)str_buf, false);
        Disp_Char(88, 32, 0xA1, false);
        Disp_Char(96, 32, 'C', false);
        last_target = target_temp;
    }
    
    /* ״̬��ʾ���ֲ����£� */
    static uint8_t last_status = 0xFF;
    uint8_t current_status;
    
    if (current_temp < target_temp - 1.0f)
    {
        current_status = 1;  // ������
    }
    else if (current_temp > target_temp + 1.0f)
    {
        current_status = 2;  // ��ȴ��
    }
    else
    {
        current_status = 3;  // �ѵ���
    }
    
    if (last_status != current_status)
    {
        /* ���״̬���� */
        for (uint8_t i = 8; i < 120; i++)
        {
            Set_Addr(6, i);
            Write_Data(0x80);  // ��������
            Set_Addr(7, i);
            Write_Data(0x00);
        }
        /* ��ʾ��״̬ */
        switch (current_status)
        {
            case 1:
                DispString(8, 48, (unsigned char*)"״̬:������...", false);
                break;
            case 2:
                DispString(8, 48, (unsigned char*)"״̬:��ȴ��...", false);
                break;
            case 3:
                DispString(8, 48, (unsigned char*)"״̬:�ѵ���", false);
                break;
        }
        last_status = current_status;
    }
}


/**
 * @brief  ��ʾ����ҳ��
 * @param  page_num: ҳ����
 */
static void Display_SetPage(uint8_t page_num)
{
    char str_buf[32];
    float set_value;
    const char* title;
    static uint8_t blink_counter = 0;
    
    /* ����ҳ���״̬ѡ������ */
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
    
    /* ���� */
    clearscreen();
    
    /* ��ʾ���� - ������ʾ */
    uint8_t title_x = (128 - strlen(title) * 8) / 2;
    DispString(title_x, 0, (unsigned char*)title, false);
    
    /* ���ָ��� */
    for (uint8_t i = 0; i < 128; i++)
    {
        Set_Addr(1, i);
        Write_Data(0x01);
    }
    
    /* ��ʾ�趨�¶� */
    DispString(8, 24, (unsigned char*)"�趨�¶�:", false);
    
    /* ��ʾ�¶�ֵ - ����״̬ʱ��˸ */
    sprintf(str_buf, "%.1f", set_value);
    if (g_monitor.state == STATE_SETTING)
    {
        blink_counter++;
        if (blink_counter < 30)
        {
            /* ��ʾ����ͷ���¶�ֵ */
            DispString(24, 32, (unsigned char*)">", false);
            DispString(32, 32, (unsigned char*)str_buf, false);
            Disp_Char(72, 32, 0xA1, false);  // �ȷ���
            Disp_Char(80, 32, 'C', false);
            DispString(96, 32, (unsigned char*)"<", false);
        }
        else if (blink_counter < 60)
        {
            /* ��˸Ч�� - ����ʾ */
        }
        else
        {
            blink_counter = 0;
        }
    }
    else
    {
        /* ������ʾ */
        DispString(32, 32, (unsigned char*)str_buf, false);
        Disp_Char(72, 32, 0xA1, false);  // �ȷ���
        Disp_Char(80, 32, 'C', false);
    }
    
    /* ���ײ��ָ��� */
    for (uint8_t i = 0; i < 128; i++)
    {
        Set_Addr(6, i);
        Write_Data(0x80);
    }
    
    /* ��ʾ������ʾ */
    if (g_monitor.state == STATE_SETTING)
    {
        DispStringS(4, 48, (unsigned char*)"UP/DN:+-  OK:Save  RT:Cancel", false);
    }
    else
    {
        DispString(16, 48, (unsigned char*)"��ȷ�ϼ���������", false);
    }
    
    /* ��ʾ����ɹ���ʾ */
    if (g_monitor.save_flag && g_monitor.save_timer > 0)
    {
        /* ������ʾ�� */
        for (uint8_t i = 2; i <= 4; i++)
        {
            Set_Addr(i, 24);
            Write_Data(0xFF);
            Set_Addr(i, 104);
            Write_Data(0xFF);
        }
        DispString(32, 24, (unsigned char*)"����ɹ�!", false);
    }
}

/**
 * @brief  ��������
 */
static void Process_Keys(void)
{
    uint8_t key = key_scan(0);  // ��֧������
    
    if (key == 0)
    {
        return;
    }
    
    switch (key)
    {
        case KEY_UP:  // KEY4��Ӧ�ϼ�
        if (g_monitor.state == STATE_DISPLAY)
        {
            /* ��ʾģʽ���л�����һҳ */
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
            /* ����ģʽ�������¶� */
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
        
    case KEY_DOWN:  // KEY3��Ӧ�¼�
        if (g_monitor.state == STATE_DISPLAY)
        {
            /* ��ʾģʽ���л�����һҳ */
            g_monitor.current_page++;
            if (g_monitor.current_page >= PAGE_MAX_NUM)
            {
                g_monitor.current_page = 0;
            }
            g_monitor.refresh_flag = 1;
        }
        else
        {
            /* ����ģʽ�������¶� */
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
            
        case KEY_CONFIRM:  // KEY1��Ӧȷ�ϼ�
            if (g_monitor.current_page == PAGE_FRONT_SET || 
                g_monitor.current_page == PAGE_REAR_SET)
            {
                if (g_monitor.state == STATE_DISPLAY)
                {
                    /* ��������ģʽ */
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
                    /* �������� */
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
            
        case KEY_RETURN:  // KEY2��Ӧ���ؼ�
            if (g_monitor.state == STATE_SETTING)
            {
                /* �˳�����ģʽ�������� */
                g_monitor.state = STATE_DISPLAY;
                g_monitor.refresh_flag = 1;
            }
            break;
    }
}

/* �޸ĸ�����ʾ���� */
void TempMonitor_UpdateDisplay(void)
{
    static DisplayBuffer_t disp_buf = {1, 0xFF};
    
    /* �ж��Ƿ���Ҫȫ��ˢ�� */
    if (disp_buf.last_page_type != g_monitor.current_page)
    {
        disp_buf.need_full_refresh = 1;
        disp_buf.last_page_type = g_monitor.current_page;
    }
    
    /* �����¶����� */
    g_monitor.front_gun.current = GetFrontGunTemp();
    g_monitor.rear_chamber.current = GetRearChamberTemp();
    
    /* ����ˢ������ѡ����·�ʽ */
    if (disp_buf.need_full_refresh)
    {
        /* ȫ��ˢ�� */
        clearscreen();
        disp_buf.need_full_refresh = 0;
    }
    
    /* ������ʾ���� */
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
 * @brief  �������� - ����ѭ���е���
 */
void TempMonitor_Process(void)
{
    /* ������ */
    Process_Keys();
    
    /* ˢ����ʾ */
    if (g_monitor.refresh_flag) 
    {
        g_monitor.refresh_flag = 0;
        TempMonitor_UpdateDisplay();
    }
}

/* �޸� TempMonitor_TimerHandler ���� */
void TempMonitor_TimerHandler(void)
{
    g_monitor.update_counter++;
    
    /* ����ˢ��Ƶ�ʣ�200ms����һ���¶���ʾ */
    if (g_monitor.update_counter >= 200)
    {
        g_monitor.update_counter = 0;
        /* ֻ����ʾ�¶�ҳ��ʱ�����¶�ֵ */
        if (g_monitor.current_page == PAGE_FRONT_TEMP || 
            g_monitor.current_page == PAGE_REAR_TEMP)
        {
            g_monitor.refresh_flag = 1;
        }
    }
    
    /* ����ҳ����Ҫ��Ƶ��ˢ�£���˸Ч���� */
    if (g_monitor.state == STATE_SETTING)
    {
        if (g_monitor.update_counter % 50 == 0)  // 50msˢ��һ��
        {
            g_monitor.refresh_flag = 1;
        }
    }
    
    /* ������ʾ��ʱ������ */
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
 * @brief  ��ȡǰǹ���¶�
 * @retval �¶�ֵ(������)
 */
__weak float GetFrontGunTemp(void)
{
    /* �������ʵ�ʵ��¶Ȼ�ȡ���� */
    /* ���磺��ADC��ȡ�¶ȴ��������ݲ�ת�� */
    /* return ADC_GetTemp(FRONT_GUN_CHANNEL); */
    
    /* ʾ��������ģ������ */
    static float temp = 25.0f;
    temp += 0.1f;
    if (temp > 30.0f) temp = 20.0f;
    return temp;
}

/**
 * @brief  ��ȡ��ǻ���¶�
 * @retval �¶�ֵ(������)
 */
__weak float GetRearChamberTemp(void)
{
    /* �������ʵ�ʵ��¶Ȼ�ȡ���� */
    /* ���磺��ADC��ȡ�¶ȴ��������ݲ�ת�� */
    /* return ADC_GetTemp(REAR_CHAMBER_CHANNEL); */
    
    /* ʾ��������ģ������ */
    static float temp = 24.0f;
    temp += 0.15f;
    if (temp > 32.0f) temp = 22.0f;
    return temp;
}

/**
 * @brief  ����ǰǹ��Ŀ���¶�
 * @param  temp: Ŀ���¶�ֵ
 */
__weak void SetFrontGunTargetTemp(float temp)
{
    /* �������ʵ�ʵ��¶ȿ��ƴ��� */
    /* ���磺����PID������Ŀ��ֵ */
    /* PID_SetTarget(FRONT_GUN_PID, temp); */
    
    /* �������EEPROM���湦�� */
    /* EEPROM_SaveFloat(FRONT_TARGET_ADDR, temp); */
}

/**
 * @brief  ���ú�ǻ��Ŀ���¶�
 * @param  temp: Ŀ���¶�ֵ
 */
__weak void SetRearChamberTargetTemp(float temp)
{
    /* �������ʵ�ʵ��¶ȿ��ƴ��� */
    /* ���磺����PID������Ŀ��ֵ */
    /* PID_SetTarget(REAR_CHAMBER_PID, temp); */
    
    /* �������EEPROM���湦�� */
    /* EEPROM_SaveFloat(REAR_TARGET_ADDR, temp); */
}

