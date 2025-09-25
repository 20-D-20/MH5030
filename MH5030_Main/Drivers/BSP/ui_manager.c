#include "ui_manager.h"
#include "key_manager.h"

/* ȫ�ֱ��� */
uint8_t g_current_gun_id = POLL_H2SO4_MIST;
SystemMode_e g_current_mode = MODE_BROWSE;
uint8_t g_current_page_id = PAGE_STARTUP;
PageData_t g_pages[PAGE_MAX];
//int16_t g_gun_temp_measured = 0;
//int16_t g_cavity_temp_measured = 0;
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

    /* ǹ���¶����� */
    g_pages[PAGE_GUN_SETTING].page_id = PAGE_GUN_SETTING;
    g_pages[PAGE_GUN_SETTING].is_editable = true;
    g_pages[PAGE_GUN_SETTING].current_value =g_system_status.front_temp_sv;             /* ��ʼ�趨ֵ */
    g_pages[PAGE_GUN_SETTING].min_value = 0;
    g_pages[PAGE_GUN_SETTING].max_value = 240;
    g_pages[PAGE_GUN_SETTING].step = 5;                                                 /* �¶����ò��� */
    
    /* �¶���ʾҳ�� */
    g_pages[PAGE_TEMP_DISPLAY].page_id = PAGE_TEMP_DISPLAY;
    g_pages[PAGE_TEMP_DISPLAY].is_editable = false;                                     /* �ɽ���༭ģʽ */
    g_pages[PAGE_TEMP_DISPLAY].current_value = 0;                                       /* ���ڴ洢�������� */

    /* ǹ��ѡ��ҳ�� */
    g_pages[PAGE_GUN_SELECT].page_id = PAGE_GUN_SELECT;
    g_pages[PAGE_GUN_SELECT].is_editable = true;                                        /* �ɽ���༭ģʽ */
    g_pages[PAGE_GUN_SELECT].current_value = 0;                                         /* ���ڴ洢�������� */

    /* ���fӢǹ��ר�������¶���ʾҳ�� */
    g_pages[PAGE_Airflow_DISPLAY].page_id = PAGE_Airflow_DISPLAY;                       /* ֻ������ */
    g_pages[PAGE_Airflow_DISPLAY].is_editable = false;
    g_pages[PAGE_Airflow_DISPLAY].current_value = 0;
    
    /* ǻ���¶����� */
    g_pages[PAGE_CAVITY_SETTING].page_id = PAGE_CAVITY_SETTING;
    g_pages[PAGE_CAVITY_SETTING].is_editable = true;
    g_pages[PAGE_CAVITY_SETTING].current_value = g_system_status.rear_temp_sv;          /* ��ʼ�趨ֵ */
    g_pages[PAGE_CAVITY_SETTING].min_value = 0;
    g_pages[PAGE_CAVITY_SETTING].max_value = 240;
    g_pages[PAGE_CAVITY_SETTING].step = 5;                                              /* �¶����ò��� */ 
    
    /* �����¿ص��� */
    g_pages[PAGE_SMART_CONTROL].page_id = PAGE_SMART_CONTROL;
    g_pages[PAGE_SMART_CONTROL].is_editable = true;                                     /* �ɽ���༭ģʽ */
    g_pages[PAGE_SMART_CONTROL].current_value = 0;                                      /* ���ڴ洢�������� */
    
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
            
        case PAGE_Airflow_DISPLAY:
            Display_Airflow_Page();
            break;
        
        case PAGE_GUN_SELECT:
            Display_Gun_Select_Page();
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
    DispString(8, 0, "ǹ�ܡ�", false);
    DispString(72, 0, "ǻ���", false);
    
     /* �ָ��� */
    draw_vspan(64,1,64);
    draw_hline(1,123,18);

    /* ��ʾ����ֵ */
    Show_Word_U_16x32(8,24,(u16)g_system_status.front_temp_pv,3,0,false);
    Show_Word_U_16x32(70,24,(u16)g_system_status.rear_temp_pv,3,0,false);
}

/**
 * @brief  ǹ��ѡ��ҳ��
 */
void Display_Gun_Select_Page(void)
{
     /* ǹ��ѡ����� */
    DispString(32, 0, "ǹ������", false);
    
    /* �ָ��� */
    draw_hline(1,127,20);

    DispString(14, 24, "������", g_current_gun_id == POLL_H2SO4_MIST);
    DispString(68, 24, "���fӢ", g_current_gun_id == POLL_DIOXIN);
    dispHzChar(84,24,31,g_current_gun_id == POLL_DIOXIN);        /* �f */
    
    /* �ָ��� */
    draw_hline(13,115,44);
    draw_vspan(64,21,43);
    draw_vspan(45,45,64);
    draw_vspan(84,45,64);
    
    DispString(22,48,"��", g_current_gun_id == POLL_AMMONIA);
    DispString(50,48,"3041", g_current_gun_id == POLL_CODE_3091);
    DispString(90,48,"��", g_current_gun_id == POLL_MERCURY);

}


/**
 * @brief  ��ʾǹ���¶�����ҳ��
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
    extern KeyPressCounter_t g_stOkCntrAutotune;
    
    DispString(16, 0, "�����¿ص���", false);
    draw_hline(1, 127, 20);
    
    DispString(32, 32, "�Ѱ�:", false);
    Show_Word_U(72, 32, g_stOkCntrAutotune.ok_press_count, 1, 0, 
                g_stOkCntrAutotune.ok_press_count > 0);
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

}

/**
 * @brief  ��ʾ���fӢҳ��
 */
void Display_Airflow_Page(void)
{

    DispString(32, 0, "�����¶�", false);
    draw_hline(1, 127, 20);
    Show_Word_U(48, 32, g_dioxin_temp1, 3, 0, false);
    DispString(74, 32, "��", false);
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


