#ifndef	_FM24CXX_H
#define	_FM24CXX_H

#include "main.h" 
#include "I2c.h"
#include "delay.h"

extern IICManager_ST   fm24cxx_iicmanager;

#define AT24C01		127
#define AT24C02		255
#define AT24C04		511
#define AT24C08		1023
#define AT24C16		2047                    
#define AT24C32		4095
#define AT24C64	    8191
#define AT24C128	16383
#define AT24C256	32767

/**
 * @brief  EEPROM类型定义
 * @note   当前使用AT24C16芯片
 */
#define EE_TYPE     AT24C16

/**
 * @brief  计算从设备地址偏移量
 * @param  addr 存储器地址
 * @retval 返回计算后的地址偏移量
 */
#define SLAVEADDR_OFFSET(addr)      ((addr >> 8) << 1)

#define FRAM_READ   0xA1             /* I2C读取命令 */
#define FRAM_WRITE  0xA0             /* I2C写入命令 */

/**
 * @brief  EEPROM初始化检测字节
 * @note   用于检测EEPROM是否正常工作
 */
#define TEST_BYTE   0x55             /* FM24CXX初始化检测标志位 */

/**
 * @brief  写保护控制宏
 * @param  pinstate 引脚状态 (0:禁用写保护, 1:启用写保护)
 */
#define EEPROM_WP(pinstate)         HAL_GPIO_WritePin(FM24CL16_WP_GPIO_Port, FM24CL16_WP_Pin, (GPIO_PinState)(pinstate))


/**
 * @brief  I2C控制器初始化
 * @param  pstIIC IIC管理器结构体指针
 */
void FM24CXX_iic_init(IICManager_ST *pstIIC);

/**
 * @brief  连续写入多个字节数据
 * @param  addr 起始地址 (最大值2047，对应16K bits容量)
 * @param  ptr  数据缓冲区指针
 * @param  size 要写入的数据大小
 * @retval 返回操作状态 (0:成功, 其他:错误码)
 * @note   使用多字节写入模式 (Multiple Byte Write Model)
 */
u8 FM_WriteByteseq(u16 addr, void *ptr, u16 size);

/**
 * @brief  从指定地址连续读取多个字节数据
 * @param  addr 起始地址 (最大值2047，对应16K bits容量)
 * @param  ptr  数据缓冲区指针
 * @param  size 要读取的数据大小
 * @retval 返回操作状态 (0:成功, 其他:错误码)
 * @note   使用选择(随机)读取模式 (Selective (Random) Read)
 */
u8 FM_ReadByteseq(u16 addr, void *ptr, u16 size);

/**
 * @brief  检测FM24CXX芯片是否正常工作
 * @retval 返回检测结果 (0:正常, 其他:异常)
 */
/**
 * @brief      检查 EEPROM 是否已初始化
 * @retval     0   表示 EEPROM 已初始化
 *             1   表示 EEPROM 未初始化
 * 
 * 该函数通过读取 EEPROM 中的特定位置（`EE_TYPE`）来判断 EEPROM 是否已经初始化。
 * 如果读取到的字节为预定义的 `TEST_BYTE`，则表示 EEPROM 已经初始化并返回 0。
 * 如果读取到的字节不是 `TEST_BYTE`，则说明 EEPROM 未初始化，函数会写入 `TEST_BYTE`，并重新读取该位置确认初始化成功。
 * 如果初始化成功，返回 0，否则返回 1。
 */
u8 FM_Check(void);

/**
 * @brief      读取指定地址的一个字节数据
 * @param      ReadAddr   读取的地址
 * @retval     返回读取的字节数据
 * 
 * 该函数通过 I2C 总线从 EEPROM 读取指定地址的一个字节数据。读取过程包括启动通讯、发送地址并接收数据。
 * 读取完成后，返回读取的字节数据。
 */
u8 FM_ReadOneByte(u16 ReadAddr);

/**
 * @brief      向指定地址写入一个字节数据
 * @param      WriteAddr  写入的地址
 * @param      DataToWrite   要写入的数据字节
 * @retval     无
 * 
 * 该函数通过 I2C 总线将一个字节的数据写入指定的 EEPROM 地址。写入过程包括启动通讯、发送地址信息、写入数据并停止通讯。
 * 写入完成后，函数返回，且 EEPROM 数据被成功写入。
 */
void FM_WriteOneByte(u16 WriteAddr, u8 DataToWrite);

//#define PARAMETER_WRITE(addr, pbuffer)		FM_WriteByteseq((u16)(addr), &pbuffer, sizeof(pbuffer))
//#define PARAMETER_READ(addr, pbuffer)		FM_ReadByteseq((u16)(addr), &pbuffer, sizeof(pbuffer))
//#define fw24_storage_compare(u32Addr,Para)  storage_compare(u32Addr,&Para,sizeof(Para),FM_WriteByteseq,FM_ReadByteseq)

#endif

