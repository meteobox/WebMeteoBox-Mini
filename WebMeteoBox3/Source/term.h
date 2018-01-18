/*
 * WebMeteoBox source code. ver 1.0.1.4
 * Created by Zager V.B. and Krylov A.I. 2012-2018 Dubna (c)
 * 
 * Project home page: http://webmeteobox.ru/
 * GutHub: https://github.com/meteobox/WebMeteoBox-Mini
 * Email: valery@jinr.ru
 *
 *
 * 18.01.2018 
 *
 */

#include "OWIHighLevelFunctions.h"
#include "OWIPolled.h"
#include "protocol.h"

#ifdef SCT_013_030 
	#define MAX_SENSORS       5//6 // Если клещи, то делаем на 1 сенсор больше
#else
	#define MAX_SENSORS       5//5 // Все сенсоры 
#endif	

#ifdef ADD_RELAY
	//#define MAX_RELAY   	  5 // 5
#endif



#define SIZE_SENSOR_STRUCT  sizeof(sensor_structure)


//#define BUS   		      OWI_PIN_5 // нужно OWI_PIN_4  21.12.12
// Порты датчиков
// PORT D pin 1 
// PORT D pin 2

#define DS18B20_FAMILY_ID                0x28 
#define DS18B20_CONVERT_T                0x44
#define DS18B20_READ_SCRATCHPAD          0xbe
#define DS18B20_WRITE_SCRATCHPAD         0x4e
#define DS18B20_COPY_SCRATCHPAD          0x48
#define DS18B20_RECALL_E                 0xb8
#define DS18B20_READ_POWER_SUPPLY        0xb4

#define READ_SUCCESSFUL   0x00
#define READ_CRC_ERROR    0x01

unsigned char DS18B20_ReadTemperature(unsigned char bus, unsigned char * id, unsigned int* temperature);
unsigned char DS18B20_ReadTemperature_Fast_Float(unsigned char bus, unsigned char * id, float* temperature);
void DS18B20_to_float(unsigned int temperature, float * out);
void DS18B20_PrintTemperature(unsigned int temperature, char * out);
unsigned char Read_scratchpad(unsigned char bus, unsigned char num);
unsigned char Write_scratchpad(unsigned char bus, unsigned char num);
void readsingle();
void my_wait_1ms(int cnt);
void Start_18b20_Convert(unsigned char bus);

//OWI_device allDevices[MAX_DEVICES]; // Для хранения ds18b20
unsigned char count_sensors; // Сколько всего датчиков подключено в систему.
sensor_structure all_sensors[MAX_SENSORS]; // Все сенсоры

#ifdef ADD_RELAY
	io_structure all_relay[MAX_RELAY]; // Сколько всего реле
#endif	
