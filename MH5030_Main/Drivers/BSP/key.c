#include "key.h"
#include "delay.h"

/**
 * @brief       ����ɨ�躯��
 * @param       mode:0 / 1, ���庬������:
 *   @arg       0,  ��֧��������(���������²���ʱ, ֻ�е�һ�ε��û᷵�ؼ�ֵ,
 *                  �����ɿ��Ժ�, �ٴΰ��²Ż᷵��������ֵ)
 *   @arg       1,  ֧��������(���������²���ʱ, ÿ�ε��øú������᷵�ؼ�ֵ)
 * @retval      ��ֵ, ��������:
 *              KEY_CONFIRM, 1, KEY1����
 *              KEY_RETURN,  2, KEY2����
 *              KEY_DOWN,    3, KEY3����
 *              KEY_UP,      4, KEY4����
 */

uint8_t key_scan(uint8_t mode)
{
    static uint8_t key_up = 1;  /* �������ɿ���־ */
    uint8_t keyval = 0;

    if (mode) key_up = 1;       /* ֧������ */

    if (key_up && (KEY1 == 0|| KEY2 == 0 || KEY3 == 0 || KEY4 == 0))  /* �����ɿ���־Ϊ1, ��������һ������������ */
    {
        delay_ms(10);           /* ȥ���� */
        key_up = 0;
 
        if (KEY1 == 0)  keyval = KEY_CONFIRM;
        
        if (KEY2 == 0)  keyval = KEY_RETURN;
        
        if (KEY3 == 0)  keyval = KEY_DOWN;
        
        if (KEY4 == 0)  keyval = KEY_UP;

    }
    else if (KEY1 == 1 && KEY2 == 1 && KEY3 == 1 && KEY4 ==1) /* û���κΰ�������, ��ǰ����ɿ� */
    {
        key_up = 1;
    }

    return keyval;              /* ���ؼ�ֵ */
}


