#include "ui_manager.h"
#include "key_manager.h"

/* ȫ�ֱ��� */
SystemMode_e g_current_mode = MODE_BROWSE;
uint8_t g_current_page_id = PAGE_STARTUP;
PageData_t g_pages[PAGE_MAX];
int16_t g_gun_temp_measured = 0;
int16_t g_cavity_temp_measured = 0;
int16_t g_dioxin_temp1 = 0;
int16_t g_dioxin_temp2 = 0;

/**
 * @brief  ��ʼ��UI������
 */
void UI_Manager_Init(void)
{
    init_page_data();
    SSD1305_init();
    clearscreen();
}

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
    g_pages[PAGE_GUN_SETTING].max_value = 250;
    g_pages[PAGE_GUN_SETTING].step = 5;
    
    /* ǻ���¶����� */
    g_pages[PAGE_CAVITY_SETTING].page_id = PAGE_CAVITY_SETTING;
    g_pages[PAGE_CAVITY_SETTING].is_editable = true;
    g_pages[PAGE_CAVITY_SETTING].current_value = 120;
    g_pages[PAGE_CAVITY_SETTING].min_value = 50;
    g_pages[PAGE_CAVITY_SETTING].max_value = 250;
    g_pages[PAGE_CAVITY_SETTING].step = 5;
    
    /* �����¿ص��� */
    g_pages[PAGE_SMART_CONTROL].page_id = PAGE_SMART_CONTROL;
    g_pages[PAGE_SMART_CONTROL].is_editable = true;  /* �ɽ���༭ģʽ */
    g_pages[PAGE_SMART_CONTROL].current_value = 0;   /* ���ڴ洢�������� */
    
    /* ���fӢ��ʾҳ�� */
    g_pages[PAGE_DIOXIN_DISPLAY].page_id = PAGE_DIOXIN_DISPLAY;
    g_pages[PAGE_DIOXIN_DISPLAY].is_editable = false;
    g_pages[PAGE_DIOXIN_DISPLAY].current_value = 0;
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
            
        case PAGE_SMART_CONTROL:
            if(g_current_mode == MODE_AUTOTUNE)
                Display_Autotune_Progress_Page();
            else
                Display_Smart_Control_Page();
            break;
            
        case PAGE_DIOXIN_DISPLAY:
            Display_Dioxin_Page();
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
 * @brief  ��ʾǹ������ҳ��
 */
void Display_Gun_Setting_Page(void)
{
    DispString(16, 0, "ǹ���¶�����", false);
    draw_hline(1, 127, 20);
    Show_Word_U(48, 32, g_pages[PAGE_GUN_SETTING].current_value, 3, 0, 
                g_current_mode == MODE_EDIT);
    DispString(74, 32, "��", false);
}

/**
 * @brief  ��ʾǻ������ҳ��
 */
void Display_Cavity_Setting_Page(void)
{
    DispString(16, 0, "ǻ���¶�����", false);
    draw_hline(1, 127, 20);
    Show_Word_U(48, 32, g_pages[PAGE_CAVITY_SETTING].current_value, 3, 0, 
                g_current_mode == MODE_EDIT);
    DispString(74, 32, "��", false);
}

/**
 * @brief  ��ʾ�����¿�ҳ��
 */
void Display_Smart_Control_Page(void)
{
    extern KeyPressCounter_t g_key_counter;
    
    DispString(16, 0, "�����¿ص���", false);
    draw_hline(1, 127, 20);
    
    DispString(32, 32, "�Ѱ�:", false);
    Show_Word_U(72, 32, g_key_counter.ok_press_count, 1, 0, 
                g_key_counter.ok_press_count > 0);
    DispString(80, 32, "/", false);
    Show_Word_U(88, 32, OK_KEY_COUNT_MAX , 1, 0, false);
}

/**
 * @brief  ��ʾ����������ҳ��
 */
void Display_Autotune_Progress_Page(void)
{
     uint8_t progress = 0;
    
    /* ������� */
    uint8_t front_progress = g_system_status.front_cross_cnt;
    uint8_t rear_progress = g_system_status.rear_cross_cnt;
    progress = (front_progress < rear_progress) ? front_progress : rear_progress;
    
    if(progress > 5) progress = 5;
    
    /* ��ʾ���� */
    DispString(16, 0, "�����¿ص���", false);
    draw_hline(1, 127, 20);
    
    /* ��ʾ���� */
    DispString(32, 32, "����:", false);
    Show_Word_U(72, 32, progress, 1, 0, true);
    DispString(80, 32, "/", false);
    Show_Word_U(88, 32, 5, 1, 0, false);
    
    /* ��ʾ��ʾ��Ϣ */
    DispString(16, 48, "ESC��ֹͣ", false);
    
//    /* ��ѡ����ʾ������ */
//    uint8_t bar_width = (progress * 100) / 5;  // ת��Ϊ�ٷֱȿ��
//    draw_rect(14, 56, 100, 6, false);          // ���
//    if(bar_width > 0)
//    {
//        draw_rect(14, 56, bar_width, 6, true); // ������
//    }

}

/**
 * @brief  ��ʾ���fӢҳ��
 */
void Display_Dioxin_Page(void)
{
    /* ��ʾ���� */
    dispHzChar(24, 0, 35, false);  /* �f */
    DispString(8, 0, "��", false);
    DispString(40, 0, "Ӣ", false);
    DispString(80, 0, "3091", false);
    
    /* �ָ��� */
    draw_hline(1, 127, 18);
    draw_vspan(64, 1, 64);
    
    /* ��ȡ����ʾ�¶� */
    get_dioxin_temperatures(&g_dioxin_temp1, &g_dioxin_temp2);
    Show_Word_U(14, 32, g_dioxin_temp1, 3, 0, false);
    DispString(38, 32, "��", false);
    Show_Word_U(78, 32, g_dioxin_temp2, 3, 0, false);
    DispString(102, 32, "��", false);
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
            
        case PAGE_SMART_CONTROL:
            if(g_current_mode != MODE_AUTOTUNE)
            {
                Show_Word_U(72, 32, value, 1, 0, value > 0);
            }
            break;
    }
}

/**
 * @brief  ��ȡ���fӢ�¶ȣ����Ժ�����
 */
void get_dioxin_temperatures(int16_t *temp1, int16_t *temp2)
{
    /* TODO: �滻Ϊʵ�ʵ��¶Ȼ�ȡ���� */
    static int16_t test_temp1 = 98;
    static int16_t test_temp2 = 102;
    
    /* ģ���¶Ȳ��� */
    test_temp1 += (rand() % 3 - 1);
    test_temp2 += (rand() % 3 - 1);
    
    if(test_temp1 < 95) test_temp1 = 95;
    if(test_temp1 > 105) test_temp1 = 105;
    if(test_temp2 < 98) test_temp2 = 98;
    if(test_temp2 > 108) test_temp2 = 108;
    
    *temp1 = test_temp1;
    *temp2 = test_temp2;
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


