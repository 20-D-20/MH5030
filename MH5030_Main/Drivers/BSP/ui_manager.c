#include "ui_manager.h"

// ȫ�ֱ���
SystemMode_e g_current_mode = MODE_BROWSE;
uint8_t g_current_page_id = PAGE_STARTUP;
PageData_t g_pages[PAGE_MAX];
int16_t g_gun_temp_measured = 0;     // ǹ�ܲ����¶�
int16_t g_cavity_temp_measured = 0;  // ǻ������¶�

/* USER CODE BEGIN Application */
/**
 * @brief  ��ʼ��ҳ������
 */
void init_page_data(void)
{
    // ����ҳ�� - ���ɱ༭
    g_pages[PAGE_STARTUP].page_id = PAGE_STARTUP;
    g_pages[PAGE_STARTUP].is_editable = false;
    g_pages[PAGE_STARTUP].current_value = 0;
    
    // �¶���ʾҳ�� - ���ɱ༭
    g_pages[PAGE_TEMP_DISPLAY].page_id = PAGE_TEMP_DISPLAY;
    g_pages[PAGE_TEMP_DISPLAY].is_editable = false;
    g_pages[PAGE_TEMP_DISPLAY].current_value = 0;
    
    // ǹ���¶�����ҳ�� - �ɱ༭
    g_pages[PAGE_GUN_SETTING].page_id = PAGE_GUN_SETTING;
    g_pages[PAGE_GUN_SETTING].is_editable = true;
    g_pages[PAGE_GUN_SETTING].current_value = 101;  // Ĭ��ֵ
    g_pages[PAGE_GUN_SETTING].min_value = 50;
    g_pages[PAGE_GUN_SETTING].max_value = 200;
    g_pages[PAGE_GUN_SETTING].step = 5;
    
    // ǻ���¶�����ҳ�� - �ɱ༭
    g_pages[PAGE_CAVITY_SETTING].page_id = PAGE_CAVITY_SETTING;
    g_pages[PAGE_CAVITY_SETTING].is_editable = true;
    g_pages[PAGE_CAVITY_SETTING].current_value = 120;  // Ĭ��ֵ
    g_pages[PAGE_CAVITY_SETTING].min_value = 50;
    g_pages[PAGE_CAVITY_SETTING].max_value = 200;
    g_pages[PAGE_CAVITY_SETTING].step = 5;
}

/**
 * @brief  �������¶Ȼ�ȡ����
 */
void get_test_temperatures(int16_t *gun_temp, int16_t *cavity_temp)
{
    static int16_t test_gun = 95;
    static int16_t test_cavity = 115;
    
    // ģ���¶Ȳ���
    test_gun += (rand() % 5 - 2);
    test_cavity += (rand() % 5 - 2);
    
    // ���Ʒ�Χ
    if(test_gun < 90) test_gun = 90;
    if(test_gun > 110) test_gun = 110;
    if(test_cavity < 110) test_cavity = 110;
    if(test_cavity > 130) test_cavity = 130;
    
    *gun_temp = test_gun;
    *cavity_temp = test_cavity;
}