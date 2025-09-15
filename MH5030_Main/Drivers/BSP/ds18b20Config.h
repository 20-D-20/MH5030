#ifndef	_DS18B20CONFIG_H
#define	_DS18B20CONFIG_H


//	Init timer on cube    1us per tick				example 72 MHz cpu >>> Prescaler=(72-1)      counter period=Max 
//###################################################################################
#define	_DS18B20_USE_FREERTOS		    				1
#define _DS18B20_MAX_SENSORS		    				1
#define	_DS18B20_GPIO												DS18B20_GPIO_Port
#define	_DS18B20_PIN												DS18B20_Pin

//#define	_DS18B20_TIMER											htim13						
////###################################################################################

#endif


