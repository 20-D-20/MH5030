#ifndef ONEWIRE_H
#define ONEWIRE_H 

#include "delay.h"
#include "tim.h"

/* C++ detection */
#ifdef __cplusplus
extern "C" {
#endif
#define	OneWireDelay(x)			delay_us(x)

typedef struct {
    GPIO_TypeDef* GPIOx;                            /*!< 用于 I/O 功能的 GPIO 端口，例如 GPIOA, GPIOB 等 */
    uint16_t GPIO_Pin;                              /*!< 用于 I/O 功能的 GPIO 引脚编号，例如 GPIO_PIN_0, GPIO_PIN_1 等 */
    
    uint8_t LastDiscrepancy;                        /*!< 搜索过程中的私有变量：记录上次搜索时设备地址的最后差异位，用于指导接下来的搜索 */
    uint8_t LastFamilyDiscrepancy;                  /*!< 搜索过程中的私有变量：记录上次搜索时设备家庭类型（Family Code）的最后差异位，用于区分不同类型的设备 */
    uint8_t LastDeviceFlag;                         /*!< 搜索过程中的私有变量：标志是否已经找到了所有设备，当所有设备都找到时该标志为 1 */
    
    uint8_t ROM_NO[8];                              /*!< 存储最近找到设备的 ROM 地址（8 字节），这是设备的唯一标识符 */
} OneWire_t;

/* Pin settings */
void ONEWIRE_LOW(OneWire_t *gp);			
void ONEWIRE_HIGH(OneWire_t *gp);		
void ONEWIRE_INPUT(OneWire_t *gp);		
void ONEWIRE_OUTPUT(OneWire_t *gp);		

#define ONEWIRE_CMD_RSCRATCHPAD           0xBE      /* 读取 Scratchpad 内存 */
#define ONEWIRE_CMD_WSCRATCHPAD           0x4E      /* 写入 Scratchpad 内存 */
#define ONEWIRE_CMD_CPYSCRATCHPAD         0x48      /* 将 Scratchpad 数据复制到 EEPROM */
#define ONEWIRE_CMD_RECEEPROM             0xB8      /* 读取 EEPROM 数据 */
#define ONEWIRE_CMD_RPWRSUPPLY            0xB4      /* 读取或控制传感器的电源供应状态 */
#define ONEWIRE_CMD_SEARCHROM             0xF0      /* 搜索总线上的所有设备 */
#define ONEWIRE_CMD_READROM               0x33      /* 读取 ROM 地址 */
#define ONEWIRE_CMD_MATCHROM              0x55      /* 精确匹配 ROM 地址 */
#define ONEWIRE_CMD_SKIPROM               0xCC      /* 跳过 ROM 地址（用于单设备或广播模式）*/


//#######################################################################################################
void OneWire_Init(OneWire_t* OneWireStruct, GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
uint8_t OneWire_Reset(OneWire_t* OneWireStruct);
uint8_t OneWire_ReadByte(OneWire_t* OneWireStruct);
void OneWire_WriteByte(OneWire_t* OneWireStruct, uint8_t byte);
void OneWire_WriteBit(OneWire_t* OneWireStruct, uint8_t bit);
uint8_t OneWire_ReadBit(OneWire_t* OneWireStruct);
uint8_t OneWire_Search(OneWire_t* OneWireStruct, uint8_t command);
void OneWire_ResetSearch(OneWire_t* OneWireStruct);
uint8_t OneWire_First(OneWire_t* OneWireStruct);
uint8_t OneWire_Next(OneWire_t* OneWireStruct);
void OneWire_GetFullROM(OneWire_t* OneWireStruct, uint8_t *firstIndex);
void OneWire_Select(OneWire_t* OneWireStruct, uint8_t* addr);
void OneWire_SelectWithPointer(OneWire_t* OneWireStruct, uint8_t* ROM);
uint8_t OneWire_CRC8(uint8_t* addr, uint8_t len);
//#######################################################################################################
 
/* C++ detection */
#ifdef __cplusplus
}
#endif

#endif

