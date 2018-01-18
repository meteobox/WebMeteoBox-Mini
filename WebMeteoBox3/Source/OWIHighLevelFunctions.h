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

/*! \file ********************************************************************
*
* Atmel Corporation
*
* \li File:               OWIHighLevelFunctions.h
* \li Compiler:           IAR EWAAVR 3.20a
* \li Support mail:       avr@atmel.com
*
* \li Supported devices:  All AVRs.
*
* \li Application Note:   AVR318 - Dallas 1-Wire(R) master.
*                         
*
* \li Description:        Header file for OWIHighLevelFunctions.c
*
*                         $Revision: 1.7 $
*                         $Date: Thursday, August 19, 2004 14:27:18 UTC $
*
*    � ������� �������� ���� ����. ������� ������� ������ ������� 1Wire
*    ���������, ��������� �� ����, � ������� ������ ����������� ����������
*    ����� ������������. ������� � ���� �� ����������������� ������� AVR318
*    � ��������� ��.
*                                    Pashgan  http://ChipEnable.Ru
*
****************************************************************************/

#ifndef _OWI_ROM_FUNCTIONS_H_
#define _OWI_ROM_FUNCTIONS_H_

#include <string.h> // Used for memcpy.

//-----------------------------------------------------------------------
typedef struct  // ����� ������� ��� � EEPROM! 28.12.2012
{
    float         value;           // ������� ��������
	unsigned char id[8];           //!< The 64 bit identifier.
	unsigned char name[15];        // ��� ����������
	unsigned char type;	           // ���: ��������,�����������,���������
	unsigned char flag;			   // ���������� �������� � ��������?
	float         offset;	       // �������� ��� ����������
	unsigned char enable;
	unsigned char scratchpad[9];   // new!!!! 28.08.2014
	unsigned char pin;             // ���� > 0 ������ ��� ����� ����� ����� ��� ds18b20
	unsigned char sensor_type;     // ��� ������� ds18b20 ��� DHT? ��� ��� ���?
	unsigned char error;           // ������ � ��������� ������ � ��������
	//float         min;             // min
	//float         max;             // max
	//char          pio;	         // ����� ����� atmega
} sensor_structure;

#define SENSOR_OTHER		 0x01
#define SENSOR_DS18B20		 0x10
#define SENSOR_DHT			 0x11
#define SENSOR_BMP180		 0x12
#define SENSOR_SCT_013_030   0x13
#define SENSOR_KWT_VIRTUAL   0x14
#define SENSOR_PAY_VIRTUAL   0x15
#define SENSOR_DOZIMETR_BOX3 0x16
#define SENSOR_ANALOG_HUMIDITY 0x17
//-----------------------------------------------------------------------


#define SEARCH_SUCCESSFUL     0x00
#define SEARCH_CRC_ERROR      0x01
#define SEARCH_ERROR          0xff
#define AT_FIRST              0xff

void OWI_SendByte(unsigned char data, unsigned char pin);
unsigned char OWI_ReceiveByte(unsigned char pin);
void OWI_SkipRom(unsigned char pin);
void OWI_ReadRom(unsigned char * romValue, unsigned char pin);
void OWI_MatchRom(unsigned char * romValue, unsigned char pin);
unsigned char OWI_SearchRom(unsigned char * bitPattern, unsigned char lastDeviation, unsigned char pins);
unsigned char OWI_SearchDevices(sensor_structure * devices, unsigned char numDevices, unsigned char pin, unsigned char *num);
unsigned char FindFamily(unsigned char familyID, sensor_structure * devices, unsigned char numDevices, unsigned char lastNum);

#endif
