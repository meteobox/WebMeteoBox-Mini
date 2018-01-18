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


/* Define shift register pins used for seven segment display */
#include <stdlib.h>
#include <stdio.h>
#include "VirtualWire.h"
#include "term.h"
#include "protocol.h"

#define LATCH_DIO 3
#define CLK_DIO 1
#define DATA_DIO 7

#define HIGH 0x1
#define LOW  0x0

#define LSBFIRST 0
#define MSBFIRST 1

#define digitalWritePortC(a, c)      (c) ? (PORTC |= 1 << (a)) : (PORTC &= ~(1 << (a)))
#define digitalWritePortB(a, c)      (c) ? (PORTB |= 1 << (a)) : (PORTB &= ~(1 << (a)))

#ifdef ENABLE_LCD_74HC595
/* Segment byte maps for numbers 0 to 9 */
const char SEGMENT_MAP[] = {0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0X80,0X90};
/* Byte maps to select digit 1 to 4 */
const char SEGMENT_SELECT[] = {0xF1,0xF2,0xF4,0xF8};

extern sensor_structure all_sensors[MAX_SENSORS];

char LCD_BUF[6];


/*
uint8_t shiftIn(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder) {
	uint8_t value = 0;
	uint8_t i;

	for (i = 0; i < 8; ++i) {
		digitalWritePortC(clockPin, HIGH);
		if (bitOrder == LSBFIRST)
		value |= digitalRead(dataPin) << i;
		else
		value |= digitalRead(dataPin) << (7 - i);
		digitalWritePortC(clockPin, LOW);
	}
	return value;
}*/

void shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val)
{
	uint8_t i;

	for (i = 0; i < 8; i++)  {
		if (bitOrder == LSBFIRST)
		digitalWritePortB(dataPin, !!(val & (1 << i)));
		else
		digitalWritePortB(dataPin, !!(val & (1 << (7 - i))));
		
		digitalWritePortC(clockPin, HIGH);
		digitalWritePortC(clockPin, LOW);
	}
	
	digitalWritePortB(dataPin, 0);
}


/* Wite a ecimal number between 0 and 9 to one of the 4 digits of the display */
void WriteNumberToSegment(unsigned char Segment, unsigned char Value)
{

char a;
a=SEGMENT_MAP[Value];
//if (Segment==2) a &= ~(1<<7);

	digitalWritePortC(LATCH_DIO,LOW);
	shiftOut(DATA_DIO, CLK_DIO, MSBFIRST, a);
	
	//shiftOut(DATA_DIO, CLK_DIO, MSBFIRST, 0x10 );
	shiftOut(DATA_DIO, CLK_DIO, MSBFIRST, SEGMENT_SELECT[Segment] );
	digitalWritePortC(LATCH_DIO,HIGH);
}

void WriteNumberToSegmentDot(unsigned char Segment, unsigned char Value, char Dot)
{

	char a;
	a=SEGMENT_MAP[Value];
	if (Dot != 0) a &= ~(1<<7);

	digitalWritePortC(LATCH_DIO,LOW);
	shiftOut(DATA_DIO, CLK_DIO, MSBFIRST, a);
	
	//shiftOut(DATA_DIO, CLK_DIO, MSBFIRST, 0x10 );
	shiftOut(DATA_DIO, CLK_DIO, MSBFIRST, SEGMENT_SELECT[Segment] );
	digitalWritePortC(LATCH_DIO,HIGH);
}


/* Write a decimal number between 0 and 9999 to the display */

void WriteLedBuffSegment(unsigned char Segment, unsigned char Value)
{
	digitalWritePortC(LATCH_DIO,LOW);
	shiftOut(DATA_DIO, CLK_DIO, MSBFIRST, Value);
	
	//shiftOut(DATA_DIO, CLK_DIO, MSBFIRST, 0x10 );
	shiftOut(DATA_DIO, CLK_DIO, MSBFIRST, SEGMENT_SELECT[Segment] );
	digitalWritePortC(LATCH_DIO,HIGH);
}

void Val_to_LED_BUF(sensor_structure val)
{
char buf[11];
unsigned char a,tmp,p=0;	
float myval=0;

if (val.error != 0) return;

	switch (val.type) {
		case 1:   myval=val.value+val.offset; break;
		case 2:   myval=val.value+val.offset; break;
		case 3:   myval=val.value+val.offset; break;
		case 4:   if (  val.offset==0.00) myval=val.value; else myval=val.value*val.offset; break;
		case 6:   myval=val.value+val.offset; break; // Doz
		case 8:   myval=val.value; break;
		case 9:   myval=val.value; break;
	}

	sprintf(buf,"%3.3f",myval);
	//sprintf(buf,"%f",val);
	//for (a=0; a<5;a++)
	for (a=0; a<5;a++)
	{
		tmp=SEGMENT_MAP[buf[a]-48];
		
		if (buf[a+1]=='.')
		{
			tmp &= ~(1<<7); // ставим точку
			LCD_BUF[p++]=tmp;
			a++;
		}
		else LCD_BUF[p++]=tmp;		
	}
#ifdef ENABLE_DOZIMETR	
	if(val.type==6) LCD_BUF[3]=0xff; // гасим последний разряд
#endif	
}

//volatile char bpos=0;	
void Send_Buff_to_LED()
{
static char bpos=0;	
	switch (bpos) {
		case 0: WriteLedBuffSegment(3 , LCD_BUF[0]); bpos++; break;
		case 1: WriteLedBuffSegment(2 , LCD_BUF[1]); bpos++;break;
		case 2: WriteLedBuffSegment(1 , LCD_BUF[2]); bpos++;break;
		case 3: WriteLedBuffSegment(0 , LCD_BUF[3]); bpos=0; break;
	}
}




/*
void WriteStringNumberToLCD(unsigned char num)
{
char buf[6];
unsigned char a=0;
/ *
	switch (all_sensors[num].type) {
		1:
	}* /

	//sprintf(buf,"%2.2f",all_sensors[num].value);
	sprintf(buf,"%2.2f",2345.0);
	

	WriteNumberToSegment(3 , buf[a]-48);
	a++;
	WriteNumberToSegment(2 , buf[a]-48);
	a++;
	WriteNumberToSegment(1 , buf[a]-48);
	a++;
	WriteNumberToSegment(0 , buf[a]-48);
}

*/


/* Write a decimal number between 0 and 9999 to the display */
/*
void WriteNumber(int Number)
{
	WriteNumberToSegment(3 , Number / 1000);
	WriteNumberToSegment(2 , (Number / 100) % 10);
	WriteNumberToSegment(1 , (Number / 10) % 10);
	WriteNumberToSegment(0 , Number % 10);
}

char sss=0;
void WriteNumber2(int Number)
{
	switch (sss) {
	case 0: WriteNumberToSegment(3 , Number / 1000); sss++; break;
	case 1: WriteNumberToSegment(2 , (Number / 100) % 10); sss++;break;
	case 2: WriteNumberToSegment(1 , (Number / 10) % 10); sss++;break;
	case 3: WriteNumberToSegment(0 , Number % 10); sss=0; break;
	}
}*/


#endif