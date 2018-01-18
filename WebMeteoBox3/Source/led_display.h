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


#ifndef LED_DISPLAY_H_
#define LED_DISPLAY_H_

uint8_t shiftIn(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder);
void shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val);
void WriteNumber(int Number);
void WriteNumberToSegment(char Segment, char Value);
void WriteNumber2(int Number);
void WriteStringNumberToLCD(unsigned char num);
void Val_to_LED_BUF(sensor_structure val);
void Send_Buff_to_LED();


#endif /* LED_DISPLAY_H_ */