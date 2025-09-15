///**
// * \brief 开发接口：
// * 
// * \note 开发流程：初始化模块，配置模块，选择模块的工作参数（配置寄存器字节1或者字节2的参数），开启/关闭触发用户回调函数
// * \note 模块配置完成后，再调用MX_GPIO_ALARM_Init()初始化ALARM引脚、然后再获取温度值
// *                   
// * 1、tps02r_cfg_init               tps02r模块配置初始化 
// * 2、tps02r_get_temp               获取寄存器通道的温度值
// * 3、tps02r_set_low                设置通道下限值             
// * 4、tps02r_set_high               设置通道上限值                
// * 5、tps02r_set_sampling_rate      设置通道采样速率
// * 6、tps02r_set_warning_count      设置触发ALERT信号的温度值测试个数
// * 7、tps02r_set_alarm_mode         设置通道触发ALERT信号的模式
// * 8、tps02r_alarm_trigger_cfg      选择模块使用的配置参数，设置通道触发
// * 9、tps02r_get_cfg_value          获取配置寄存器的数据
// * 10、tps02r_alarm_trigger_on      开启触发用户回调函数
// * 11、tps02r_alarm_trigger_off     关闭触发用户回调函数
// *      
// */

//#include "main.h"
//#include <string.h>
//#include <stdio.h>
//#include "tps02r.h"

///**
// * \brief tps02r模块例程入口
// * 
// * \param[in] hi2c I2C设备
// * 
// * \retval  0 成功
// * \retval -1 失败 
// */
//int tps02r_demo_entry(I2C_HandleTypeDef hi2c)
//{
//    char info[128] = {0};
//    int ret = 0;
//    float temp_date[2] = {0};  //存放温度数据的缓存区

//    /* 初始化一个tps02r设备 */
//    zlg_tps02r_device_t tps02r_dev;
//    tps02r_dev.hi2c = &hi2c;       //I2C主机设备
//    tps02r_dev.tps02r_addr = 0x48; //tps02r模块地址

//    /* 将tps02r模块初始化为默认配置 */
//    ret = tps02r_cfg_init(&tps02r_dev);
//    if(ret < 0){
//       return -1;
//    }
//    
//    while (1) {
//        /* 第一次触发引脚中断后会清除中断状态，此处再次使能ALARM引脚中断 */
//        // HAL_NVIC_EnableIRQ(EXTI0_IRQn);

//        /* 获取通道1采集到的温度数据 */
//        ret = tps02r_get_temp(&tps02r_dev, TPS02R_TEMP_ADDR, TPS02R_CHAN1, &temp_date[0]);
//        if(ret < 0){
//            return -1;
//        }
//        /* 获取通道2采集到的温度数据 */
//        ret = tps02r_get_temp(&tps02r_dev, TPS02R_TEMP_ADDR, TPS02R_CHAN2, &temp_date[1]);
//        if(ret < 0){
//            return -1;
//        }
//        
//        // 输出温度值 精度为0.001 
//        sprintf(info,"chanl1 temperature = %0.3f\nchanl2 temperature = %0.3f\n",temp_date[0], temp_date[1]);
//        ptintf_info(info);
//            
//        delay_ms(1000);
//				ptintf_info("\n\n");
//    }
//    return 0;
//}

///** end of file **/
