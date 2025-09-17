#ifndef __KEY_H
#define __KEY_H

#include "main.h"

/******************************************************************************************/

#define KEY1        HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin)     /* ��ȡKEY1���� */
#define KEY2        HAL_GPIO_ReadPin(KEY2_GPIO_Port, KEY2_Pin)     /* ��ȡKEY2���� */
#define KEY3        HAL_GPIO_ReadPin(KEY3_GPIO_Port, KEY3_Pin)     /* ��ȡKEY3���� */
#define KEY4        HAL_GPIO_ReadPin(KEY4_GPIO_Port, KEY4_Pin)     /* ��ȡKEY4���� */


#define KEY_CONFIRM  1               /* KEY1���� */
#define KEY_RETURN   2               /* KEY2���� */
#define KEY_DOWN     3               /* KEY3���� */
#define KEY_UP       4               /* KEY4���� */

uint8_t key_scan(uint8_t mode);  /* ����ɨ�躯�� */

#endif


