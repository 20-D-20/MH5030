#include "ui_manager.h"

/* ȫ�ֱ��� */
SystemMode_e g_current_mode = MODE_BROWSE;
uint8_t g_current_page_id = PAGE_STARTUP;
PageData_t g_pages[PAGE_MAX];
int16_t g_gun_temp_measured = 0;
int16_t g_cavity_temp_measured = 0;

/**
 * @brief  ��ʼ��ҳ������
 */
void init_page_data(void)
{
    /* ����ҳ�� */
    g_pages[PAGE_STARTUP].page_id = PAGE_STARTUP;
    g_pages[PAGE_STARTUP].is_editable = false;
    g_pages[PAGE_STARTUP].current_value = 0;
    
    /* �¶���ʾҳ�� */
    g_pages[PAGE_TEMP_DISPLAY].page_id = PAGE_TEMP_DISPLAY;
    g_pages[PAGE_TEMP_DISPLAY].is_editable = false;
    g_pages[PAGE_TEMP_DISPLAY].current_value = 0;
    
    /* ǹ���¶����� */
    g_pages[PAGE_GUN_SETTING].page_id = PAGE_GUN_SETTING;
    g_pages[PAGE_GUN_SETTING].is_editable = true;
    g_pages[PAGE_GUN_SETTING].current_value = 101;
    g_pages[PAGE_GUN_SETTING].min_value = 50;
    g_pages[PAGE_GUN_SETTING].max_value = 200;
    g_pages[PAGE_GUN_SETTING].step = 5;
    
    /* ǻ���¶����� */
    g_pages[PAGE_CAVITY_SETTING].page_id = PAGE_CAVITY_SETTING;
    g_pages[PAGE_CAVITY_SETTING].is_editable = true;
    g_pages[PAGE_CAVITY_SETTING].current_value = 120;
    g_pages[PAGE_CAVITY_SETTING].min_value = 50;
    g_pages[PAGE_CAVITY_SETTING].max_value = 200;
    g_pages[PAGE_CAVITY_SETTING].step = 5;
    
    /* �����¿ص���ҳ�� */
    g_pages[PAGE_PID_CONTROL].page_id = PAGE_PID_CONTROL;
    g_pages[PAGE_PID_CONTROL].is_editable = true;  // ���⴦�����ڼ��OK��
    g_pages[PAGE_PID_CONTROL].current_value = 0;
    
    /* ����������ҳ�� */
    g_pages[PAGE_AUTOTUNE_PROGRESS].page_id = PAGE_AUTOTUNE_PROGRESS;
    g_pages[PAGE_AUTOTUNE_PROGRESS].is_editable = false;
    g_pages[PAGE_AUTOTUNE_PROGRESS].current_value = 0;
    
    /* ״̬��ʾҳ�� */
    g_pages[PAGE_STATUS_DISPLAY].page_id = PAGE_STATUS_DISPLAY;
    g_pages[PAGE_STATUS_DISPLAY].is_editable = false;
    g_pages[PAGE_STATUS_DISPLAY].current_value = 0;
}

/**
 * @brief  ��ʾҳ��
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
 * @brief  ��ʾ����ҳ��
 */
void Display_Startup_Page(void)
{
    DispString(40, 12, "MH5030", false);
    DispString(28, 36, "Ver 1.0.0", false);
}

/**
 * @brief  ��ʾ�¶�ҳ��
 */
void Display_Temp_Page(void)
{
    DispString12(30, 0, "����ֵ", false);
    DispString12(80, 0, "�趨ֵ", false);
    DispString12(0, 24, "ǹ��", false);
    DispString12(0, 48, "ǻ��", false);
    draw_hline(1, 127, 16);
    
    /* ��ʾ����ֵ */
    get_test_temperatures(&g_gun_temp_measured, &g_cavity_temp_measured);
    Disp_Word_UM(38, 24, 3, g_gun_temp_measured, 0, 0);
    Disp_Word_UM(38, 48, 3, g_cavity_temp_measured, 0, 0);
    
    /* ��ʾ�趨ֵ */
    Disp_Word_UM(88, 24, 3, g_pages[PAGE_GUN_SETTING].current_value, 0, 0);
    Disp_Word_UM(88, 48, 3, g_pages[PAGE_CAVITY_SETTING].current_value, 0, 0);
}

/**
 * @brief  ��ʾǹ���¶�����ҳ��
 */
void Display_Gun_Setting_Page(void)
{
    DispString(16, 0, "ǹ���¶�����", false);
    draw_hline(1, 127, 20);
    Show_Word_U(48, 32, g_pages[PAGE_GUN_SETTING].current_value, 3, 0, false);
    DispString(74, 32, "��", false);
}

/**
 * @brief  ��ʾǻ���¶�����ҳ��
 */
void Display_Cavity_Setting_Page(void)
{
    DispString(16, 0, "ǻ���¶�����", false);
    draw_hline(1, 127, 20);
    Show_Word_U(48, 32, g_pages[PAGE_CAVITY_SETTING].current_value, 3, 0, false);
    DispString(74, 32, "��", false);
}

/**
 * @brief  ��ʾ�����¿ص���ҳ��
 */
void Display_PID_Control_Page(void)
{
    DispString(16, 0, "�����¿ص���", false);
    draw_hline(1, 127, 20);
    
    /* ��ʾ��ǰPID������ */
    DispString12(10, 28, "��ǰ��:", false);
    switch(g_system_status.param_group)
    {
        case PARAM_GROUP_120:
            DispString12(58, 28, "120��", false);
            break;
        case PARAM_GROUP_160:
            DispString12(58, 28, "160��", false);
            break;
        case PARAM_GROUP_220:
            DispString12(58, 28, "220��", false);
            break;
    }
    
    /* ��ʾ��ʾ��Ϣ */
    DispString12(10, 46, "����OK��7��������", false);
}

/**
 * @brief  ��ʾ����������ҳ��
 */
void Display_Autotune_Progress_Page(void)
{
    DispString(16, 0, "������������", false);
    draw_hline(1, 127, 20);
    
    /* ��ʾ���� */
    DispString12(10, 28, "ǰǹ��:", false);
    Disp_Word_US(58, 28, 1, g_system_status.front_cross_cnt, 0, 0);
    DispString12(70, 28, "/5", false);
    
    DispString12(10, 46, "ǻ��:", false);
    Disp_Word_US(58, 46, 1, g_system_status.rear_cross_cnt, 0, 0);
    DispString12(70, 46, "/5", false);
    
    /* ������ */
    uint8_t progress = (g_system_status.front_cross_cnt + g_system_status.rear_cross_cnt) * 10;
    draw_rect(20, 56, 88, 6, false);
    if(progress > 0)
    {
        draw_rect(22, 58, (progress * 84) / 100, 2, true);
    }
}

/**
 * @brief  ��ʾ״̬ҳ��
 */
void Display_Status_Page(void)
{
    DispString(24, 0, "ϵͳ״̬", false);
    draw_hline(1, 127, 16);
    
    /* TODO: ��ʾϵͳ״̬��Ϣ */
    // ��ʾ����״̬
    // DispString12(0, 20, "����:", false);
    // Show_Word_U(36, 20, get_fan_rpm(), 4, 0, false);
    // DispString12(72, 20, "RPM", false);
    
    // ��ʾ����״̬
    // DispString12(0, 36, "����:", false);
    // DispString12(36, 36, get_heater_status() ? "����" : "�ر�", false);
    
    // ��ʾ�������
    // DispString12(0, 52, "����:", false);
    // Show_Word_U(36, 52, g_system_status.error_code, 3, 0, false);
    
    /* ��ʱ��ʾ */
    DispString12(20, 30, "ϵͳ��������", false);
}

/**
 * @brief  ������ֵ��ʾ
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
            /* ��ʾOK�����´�������ѡ�� */
            if(value > 0 && value < 7)
            {
                Disp_Word_US(100, 46, 1, value, 0, 0);
            }
            break;
    }
}

/**
 * @brief  ����ʵʱ����
 */
void Update_Realtime_Data(void)
{
    if(g_current_page_id == PAGE_TEMP_DISPLAY)
    {
        /* �����¶Ȳ���ֵ */
        get_test_temperatures(&g_gun_temp_measured, &g_cavity_temp_measured);
        Disp_Word_UM(38, 24, 3, g_gun_temp_measured, 0, 0);
        Disp_Word_UM(38, 48, 3, g_cavity_temp_measured, 0, 0);
    }
    else if(g_current_page_id == PAGE_AUTOTUNE_PROGRESS)
    {
        /* �������������� */
        Update_System_Status();
        Display_Autotune_Progress_Page();
        
        /* ����Ƿ���� */
        if(g_system_status.autotune_complete)
        {
            /* ����PID����ҳ�� */
            g_current_page_id = PAGE_PID_CONTROL;
            Display_Page(PAGE_PID_CONTROL);
        }
    }
}

/**
 * @brief  �������¶Ȼ�ȡ����
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


