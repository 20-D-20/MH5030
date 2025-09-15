#ifndef __TPS02R_H
#define __TPS02R_H

#include "main.h"
#include "I2c.h"
//  
// 定义全局IIC管理结构体和初始化配置结构体
extern IICManager_ST        tps02r_iicmanger;

/**             
 * \broef 设备地址
 * TPS02R_I2C_ADR_W:(0x48 <<1) | 0
 * TPS02R_I2C_ADR_W:(0x48 <<1) | 1
 */  
#define TPS02R_I2C_ADR_W     0x90                           //设备写地址
#define TPS02R_I2C_ADR_R     0x91                           //设备读地址

#define TPS02R_FUN_OK        0                              //TPS02R函数相关接口操作成功
#define TPS02R_FUN_ERROR    -1                              //TPS02R函数相关接口操作失败
#define TPS02R_VALUE_ERROR  -2                              //TPS02R函数的传入参数错误
                                    
/**                
 * \broef TPS02R设备通道
 */                
#define TPS02R_CHAN1         0                              //TPS02R设备通道1
#define TPS02R_CHAN2         1                              //TPS02R设备通道2

/**
 * \broef TPS02R选择使能通道
 */
#define TPS02R_CFG_CHAN_EN1_EN2        0x00                 //TPS02R选择使能通道1,使能通道2
#define TPS02R_CFG_CHAN_EN1_DISEN2     0x02                 //TPS02R选择使能通道1,禁能通道2
#define TPS02R_CFG_CHAN_DISEN1_DISEN2  0x03                 //TPS02R选择禁能通道1,禁能通道2
#define TPS02R_CFG_CHAN_DISEN1_EN2     0x01                 //TPS02R选择禁能通道1,使能通道2
                
/**             
 * \broef 子寄存器地址                
 */             
#define TPS02R_TEMP_ADDR (0X00)    				            //温度子寄存器地址
#define TPS02R_TEMP_LEN  (0X06)    				            //温度子寄存器长度
        
#define TPS02R_CFG_ADDR (0X01)     				            //配置子寄存器地址
#define TPS02R_CFG_LEN  (0X02)     				            //配置子寄存器长度
        
#define TPS02R_TLOW_ADDR (0X02)    				            //温度下限子寄存器地址
#define TPS02R_TLOW_LEN  (0X06)    				            //温度下限子寄存器长度
        
#define TPS02R_THIG_ADDR (0X03)    				            //温度上限子寄存器地址
#define TPS02R_THIG_LEN  (0X06)    				            //温度上限子寄存器长度

        
/**             
 * \broef 配置相关宏定义               
 */             
#define TPS02R_SAMPLE_RATE_10  (0X00)   		            //配置采样速率10 bps
#define TPS02R_SAMPLE_RATE_40  (0X01)   		            //配置采样速率40 bps
        
#define TPS02R_CFG_RATE_MASK  (0X20)    		            //配置采样率掩码
#define TPS02R_CFG_RATE_SHIFT (0X05)    		            //配置采样率移位
                
#define TPS02R_CFG_TRIGGER_WARNING_MASK  (0X18)             //配置采样率掩码
#define TPS02R_CFG_TRIGGER_WARNING_SHIFT (0X03)             //配置采样率移位

#define TPS02R_CFG_EN_MASK  (0X80)                          //配置使能掩码
#define TPS02R_CFG_EN_SHIFT (0X07)                          //配置使能移位
                
#define TPS02R_CFG_ALARM_MODE_MASK  (0X06)                  //配置警报模式掩码
#define TPS02R_ALARM_MODE_0  		(0x00)                  //模式0 比较模式 ALERT空闲时输出高电平
#define TPS02R_ALARM_MODE_1  		(0x02)                  //模式1 中断模式 ALERT空闲时输出高电平
#define TPS02R_ALARM_MODE_2  		(0x04)                  //模式2 比较模式 ALERT空闲时输出低电平
#define TPS02R_ALARM_MODE_3  		(0x06)                  //模式3 中断模式 ALERT空闲时输出低电平


void tps02r_iic_init(IICManager_ST *pstIIC);

/**
 * \brief 将tps02r模块的配置初始化为默认值
 * 
 * \note 默认选择配置寄存器通道1的参数，采样率为10 bps，触发ALERT输出信号的温度值测试个数为6个
 * \note 报警信号输出为比较模式，ALERT的初始输出状态为低电平
 * \note 默认温度上限值为1023.999878℃，默认温度下限值为-0.000122℃
 * 
 * \param[in] p_dev tps02r模块设备
 * 
 * \retval TPS02R_FUN_OK       配置成功
 * \retval TPS02R_FUN_ERROR    配置失败
 * \retval TPS02R_VALUE_ERROR  参数错误
 */
int tps02r_cfg_init(void);

/**
 * \brief 获取寄存器通道的温度值
 * 
 * \note 可以获取当前模块采集到的温度值（TPS02R_TEMP_ADDR）
 * \note 也可以获取到已经设置的温度上限（TPS02R_THIG_ADDR）和下限（TPS02R_TLOW_ADDR）
 * 
 * \param[in] chan 通道 可选TPS02R_CHAN1 TPS02R_CHAN2
 * \param[out] p_data 存放温度数据的缓存区
 * 
 * \retval TPS02R_FUN_OK       配置成功
 * \retval TPS02R_FUN_ERROR    配置失败
 * \retval TPS02R_VALUE_ERROR  参数错误
 */
int tps02r_get_temp(int8_t chan, float *p_data);

/**
 * \brief 设置通道下限值
 * 
 * \param[in] chan 通道 可选TPS02R_CHAN1 TPS02R_CHAN2
 * \param[in] temp 需要设置的下限温度值
 * 
 * \retval TPS02R_FUN_OK       配置成功
 * \retval TPS02R_FUN_ERROR    配置失败
 * \retval TPS02R_VALUE_ERROR  参数错误
 */
int tps02r_set_low(int8_t chan, float temp);


/**
 * \brief 设置通道上限值
 * 
 * \param[in] chan 通道 可选TPS02R_CHAN1 TPS02R_CHAN2
 * \param[in] temp 需要设置的上限温度值
 * 
 * \retval TPS02R_FUN_OK       配置成功
 * \retval TPS02R_FUN_ERROR    配置失败
 * \retval TPS02R_VALUE_ERROR  参数错误
 */
int tps02r_set_high( int8_t chan, float temp);

/**
 * \brief 设置通道采样速率
 * 
 * \param[in] chan_en 需要选择的使能通道，传入不同的宏会修改寄存器位的参数，可选宏如下：
 *                    修改配置寄存器字节1(实际修改)的速率参数：TPS02R_CFG_CHAN_EN1_EN2、TPS02R_CFG_CHAN_EN1_DISEN2、TPS02R_CFG_CHAN_DISEN1_DISEN2
 *                    修改配置寄存器字节2(实际修改)的速率参数：TPS02R_CFG_CHAN_DISEN1_EN2
 * 
 * \param[in] rate    速度值 可选TPS02R_SAMPLE_RATE_10、TPS02R_SAMPLE_RATE_40
 * 
 * \retval TPS02R_FUN_OK    配置成功
 * \retval TPS02R_FUN_ERROR 配置失败
 * \retval TPS02R_VALUE_ERROR  参数错误
 */
int tps02r_set_sampling_rate(int8_t chan_en, uint32_t rate);

/**
 * \brief 获取配置寄存器通道数据
 * 
 * \param[in] p_dev tps02r设备
 * \param[in] chan 通道（通道1对应配置寄存器字节1的数据，通道2对应配置寄存器字节2的数据） 可选TPS02R_CHAN1 TPS02R_CHAN2
 * \param[out] p_data 通道配置数据的缓存区
 *  
 * \retval TPS02R_FUN_OK    配置成功
 * \retval TPS02R_FUN_ERROR 配置失败
 * \retval TPS02R_VALUE_ERROR  参数错误
 */
int tps02r_get_cfg_value(int8_t chan, uint8_t *p_data);

#endif 
/** end of file **/
