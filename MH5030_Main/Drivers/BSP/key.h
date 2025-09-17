#ifndef __KEY_H
#define __KEY_H

#include "main.h"

/******************************************************************************************/

#define KEY1        HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin)     /* 读取KEY1引脚 */
#define KEY2        HAL_GPIO_ReadPin(KEY2_GPIO_Port, KEY2_Pin)     /* 读取KEY2引脚 */
#define KEY3        HAL_GPIO_ReadPin(KEY3_GPIO_Port, KEY3_Pin)     /* 读取KEY3引脚 */
#define KEY4        HAL_GPIO_ReadPin(KEY4_GPIO_Port, KEY4_Pin)     /* 读取KEY4引脚 */


#define KEY_CONFIRM  1               /* KEY1按下 */
#define KEY_RETURN   2               /* KEY2按下 */
#define KEY_DOWN     3               /* KEY3按下 */
#define KEY_UP       4               /* KEY4按下 */

uint8_t key_scan(uint8_t mode);  /* 按键扫描函数 */

#endif


