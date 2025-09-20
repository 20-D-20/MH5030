#include "ui_manager.h"

// 全局变量
SystemMode_e g_current_mode = MODE_BROWSE;
uint8_t g_current_page_id = PAGE_STARTUP;
PageData_t g_pages[PAGE_MAX];
int16_t g_gun_temp_measured = 0;     // 枪管测量温度
int16_t g_cavity_temp_measured = 0;  // 腔体测量温度

/* USER CODE BEGIN Application */
/**
 * @brief  初始化页面数据
 */
void init_page_data(void)
{
    // 启动页面 - 不可编辑
    g_pages[PAGE_STARTUP].page_id = PAGE_STARTUP;
    g_pages[PAGE_STARTUP].is_editable = false;
    g_pages[PAGE_STARTUP].current_value = 0;
    
    // 温度显示页面 - 不可编辑
    g_pages[PAGE_TEMP_DISPLAY].page_id = PAGE_TEMP_DISPLAY;
    g_pages[PAGE_TEMP_DISPLAY].is_editable = false;
    g_pages[PAGE_TEMP_DISPLAY].current_value = 0;
    
    // 枪管温度设置页面 - 可编辑
    g_pages[PAGE_GUN_SETTING].page_id = PAGE_GUN_SETTING;
    g_pages[PAGE_GUN_SETTING].is_editable = true;
    g_pages[PAGE_GUN_SETTING].current_value = 101;  // 默认值
    g_pages[PAGE_GUN_SETTING].min_value = 50;
    g_pages[PAGE_GUN_SETTING].max_value = 200;
    g_pages[PAGE_GUN_SETTING].step = 5;
    
    // 腔体温度设置页面 - 可编辑
    g_pages[PAGE_CAVITY_SETTING].page_id = PAGE_CAVITY_SETTING;
    g_pages[PAGE_CAVITY_SETTING].is_editable = true;
    g_pages[PAGE_CAVITY_SETTING].current_value = 120;  // 默认值
    g_pages[PAGE_CAVITY_SETTING].min_value = 50;
    g_pages[PAGE_CAVITY_SETTING].max_value = 200;
    g_pages[PAGE_CAVITY_SETTING].step = 5;
}

/**
 * @brief  测试用温度获取函数
 */
void get_test_temperatures(int16_t *gun_temp, int16_t *cavity_temp)
{
    static int16_t test_gun = 95;
    static int16_t test_cavity = 115;
    
    // 模拟温度波动
    test_gun += (rand() % 5 - 2);
    test_cavity += (rand() % 5 - 2);
    
    // 限制范围
    if(test_gun < 90) test_gun = 90;
    if(test_gun > 110) test_gun = 110;
    if(test_cavity < 110) test_cavity = 110;
    if(test_cavity > 130) test_cavity = 130;
    
    *gun_temp = test_gun;
    *cavity_temp = test_cavity;
}