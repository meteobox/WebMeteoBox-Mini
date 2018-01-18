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

#include <stdio.h>
#include <avr/eeprom.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include "term.h"
#include "protocol.h"
#include "task_config.h"
#include "led_display.h" 
#include "enc28j60.h"

//First mac address = BOX - 424F58
// 6d - (m) т.е. Mini
#define DefPassword		    "admin\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //пароль по умолчанию admin
//\xc0\xa8\x01\xde = = 192.168.1.222
//\xc0\xa8\x00\x64 = = 192.168.0.100

#ifdef METEOBOX_DEMO_MODE	
	/* для подсети 192.168.1.222 */
	#define DefConfigParam		"\x82\x82\x82\x82\x00\x00\x00\x4F\x11\x11\x22\x05\xc0\xa8\x01\xde\xFF\xFF\xFF\x00\xc0\xa8\x01\x01\xc0\xa8\x01\x01\x00"
#else													 /*  это мак адрес     */	
	/* для подсети 192.168.1.222 */
	#define DefConfigParam		"\x82\x82\x82\x82\x00\x00\x00\x4F\x58\x6d\x02\x05\xc0\xa8\x01\xde\xFF\xFF\xFF\x00\xc0\xa8\x01\x01\xc0\xa8\x01\x01\x00"
#endif															
/* для домашней подсети 192.168.0.100 */
//#define DefConfigParam		"\x82\x82\x82\x82\x00\x00\x42\x4F\x58\x41\x02\x05\xc0\xa8\x00\x64\xFF\xFF\xFF\x00\xc0\xa8\x00\x01\xc0\xa8\x00\x01\x00"


//#define DefConfigParam		"\x82\x82\x82\x82\x00\x00\x00\x08\xdc\x00\x02\x05\xc0\xa8\x01\xde\xFF\xFF\xFF\x00\xc0\xa8\x01\x01\xc0\xa8\x01\x01\x00"

StrConfigParam NetworkParam;
/*
#ifdef ADD_LOGGER_2	
	 Server_config server_data[2]; // 2 логгера
#else
	 Server_config server_data[1]; // 1 логгер
#endif	
*/

void ENC28J60_RST( char ckl )
{
	if (ckl == 1) PORTB |= (1<<1);
	else PORTB &= ~(1<<1);
}


/*
void ENC28J60_REBOOT() 
{
	ENC28J60_RST(0);
	my_wait_1ms(10);
	ENC28J60_RST(1);
	my_wait_1ms(15);
}*/


void ENC28J60_INIT_CHIP()
{
	//ENC28J60_REBOOT();
	ENC28J60_RST(0);
	my_wait_1ms(10);
	ENC28J60_RST(1);
	my_wait_1ms(15);	
	wdt_reset();
	//initialize the hardware driver for the enc28j60
	enc28j60Init(NetworkParam.mac);
	//enc28j60clkout(2); // change clkout from 6.25MHz to 12.5MHz
	_delay_loop_1(0); // 60us
	enc28j60PhyWrite(PHLCON,0x476);
	my_wait_1ms(20);
}



void mcu_init(void)
{
	EICRA = 0x00;
	EIMSK = 0x00;
	EIFR = 0xFF;			// External Interrupt Flag Register all clear
	
	//PORTA=0xFF;     // 4 relay 0ff
    //PORTA=0x00;	// 6 relay off
	//DDRA= 0x0F;

	PORTB=0x3F; // для 5 реле WEB_RELAY_MINI
	DDRB= 0xF6;
	
	


#ifdef WEB_RELAY_MINI_1
	PORTC=0x00;
	DDRC= 0x4F;
#else
	#ifdef WEB_RELAY_MINI
		PORTC=0x00; 
		DDRC= 0x4F; 
	#else
		PORTC=0xFF; // было FF
		DDRC= 0x4F; // было FF
	#endif	
#endif	

#ifdef HUMIDITY_SENSOR
	PORTC=0x00; // было FF
	DDRC= 0x43; // было FF
#endif	

	
/*
0 в выходах реле на старте	
	PORTB=0x3F;
	DDRB= 0xF6;

	PORTC=0xF0; // было FF
	DDRC= 0x4F; // было FF
*/		
	
	PORTD=0xFF;
	DDRD= 0xE2;

	ADMUX=ADC_REF_VREF & 0xff;
	ADCSRA=0x87;

#ifdef ENABLE_LCD_74HC595
// Timer/Counter 2 initialization
// Clock source: System Clock
// Clock value: 31,250 kHz
// Mode: CTC top=OCR2A
// OC2A output: Disconnected
// OC2B output: Disconnected
// Timer Period: 4,992 ms
ASSR=(0<<EXCLK) | (0<<AS2);
TCCR2A=(0<<COM2A1) | (0<<COM2A0) | (0<<COM2B1) | (0<<COM2B0) | (1<<WGM21) | (0<<WGM20);
TCCR2B=(0<<WGM22) | (1<<CS22) | (1<<CS21) | (0<<CS20);
TCNT2=0xFF;
OCR2A=0x9B;
OCR2B=0x00;


TIMSK2=(0<<OCIE2B) | (1<<OCIE2A) | (0<<TOIE2);
#endif	

#ifdef ENABLE_DOZIMETR 
PORTD=0x7F;
DDRD= 0x62;
// External Interrupt(s) initialization
// INT0: Off
// INT1: Off
// Interrupt on any change on pins PCINT0-7: Off
// Interrupt on any change on pins PCINT8-14: Off
// Interrupt on any change on pins PCINT16-23: On
EICRA=(0<<ISC11) | (0<<ISC10) | (0<<ISC01) | (0<<ISC00);
EIMSK=(0<<INT1) | (0<<INT0);
PCICR=(1<<PCIE2) | (0<<PCIE1) | (0<<PCIE0);
PCMSK2=(1<<PCINT23) | (0<<PCINT22) | (0<<PCINT21) | (0<<PCINT20) | (0<<PCINT19) | (0<<PCINT18) | (0<<PCINT17) | (0<<PCINT16);
PCIFR=(1<<PCIF2) | (0<<PCIF1) | (0<<PCIF0);
#endif	

	
	//_______________таймер-0 кварц 8 мгц ________________________
	//ASSR  = 0x00;
	//TCCR0 = 0x0F;// TIMER1 clock is xtal/1024
	//OCR0  = 0x4E;// при xtal/1024 -- 10mc
	OCR0A=78;
    TIMSK0 |= (1 << OCIE0A);    // enable timer interrupt
	TCCR0A |= (1 << WGM01);
    TCCR0B |= (1 << CS02) /*| (1 << CS01)*/ | (1 << CS00);
    // Turn on the counter, Clock on Risf	
}



void default_network(char level) // 29.11.2013
{
    unsigned char a;	
	unsigned char old_mac[6];
	
	cli();
	
	memset(&server_data,0x00,sizeof(server_data));
	
	eeprom_write_block((unsigned char*)DefPassword,(unsigned char*)eeprom_password, 15);
	
	
#ifdef ADD_RELAY	
	memset(&all_relay,0x00,sizeof(all_relay));
	
#ifdef WEB_RELAY_MINI_1	
	all_relay[0].pio=0; // Присваиваем к номерам реле pio atmega
#else	
	all_relay[0].pio=0; // Присваиваем к номерам реле pio atmega
	all_relay[1].pio=1; //PORTC PIN  1
	all_relay[2].pio=2; //PORTC PIN  2
	all_relay[3].pio=3; //PORTC PIN  3
	all_relay[4].pio=6; //PORTB PIN  6 29.15.2015
#endif	
/*
	all_relay[5].pio=4; // 25.11.2013
*/	
	
	eeprom_write_block(&all_relay, (unsigned char*)eeprom_relay_config, sizeof(all_relay));
#endif	
	
	
	//memset(server_data,0x00,sizeof(server_data));
	
#ifdef METEOBOX_DEMO_MODE	
	server_data[0].enable=0xFF; // enable=0xff
#endif	
	strcpy_P((char *)server_data[0].server_name,PSTR("narodmon.ru"));
	strcpy_P((char *)server_data[0].script_path,PSTR("/post.php"));
	
	server_data[0].refresh_time=305; // Время обовления по умолчанию 1 раз в 5 минут = 300
	server_data[0].port=80;
	
	
#ifdef ADD_LOGGER_2	
    memcpy(&server_data[1],&server_data[0],sizeof(server_data[0]));
	
#ifdef METEOBOX_DEMO_MODE		
	server_data[1].enable=0xFF;  // Для дома	
	strcpy_P((char *)server_data[1].server_name,PSTR("sensorcloud.tk"));
	strcpy_P((char *)server_data[1].script_path,PSTR("/free/xxwqqq630ioguqhuu63a8znk.php"));	/* 25.08.2017 */
#else
	strcpy_P((char *)server_data[1].server_name,PSTR("webmeteobox.ru"));
	strcpy_P((char *)server_data[1].script_path,PSTR("/script/add.php"));	
#endif			
	server_data[1].refresh_time=60; // 1 раз в минуту = 60
#endif	
	
	
	
	eeprom_write_block(&server_data, (unsigned char*)eeprom_server_config, sizeof(server_data));
	
if (level==0)	
	{
		memcpy_P(&NetworkParam, PSTR(DefConfigParam), sizeof(NetworkParam));
	}
else
	{
		memcpy(&old_mac,&NetworkParam.mac,6); //Сохраняем MAC 
		memcpy_P(&NetworkParam, PSTR(DefConfigParam), sizeof(NetworkParam));
		memcpy(&NetworkParam.mac,&old_mac,6); //Восстанавливаем MAC 
	}	
	
	eeprom_write_block(&NetworkParam, (unsigned char*)EEPOP, sizeof(NetworkParam));
	


	for (a=0; a<MAX_SENSORS; a++) {
		strcpy_P((char *)all_sensors[0].name,PSTR("No name"));
		strcpy_P((char *)all_sensors[0].id,PSTR("00"));
		all_sensors[0].value=0;
		all_sensors[0].type=0;
		all_sensors[0].flag=0;
		all_sensors[0].offset=0.0000f;
		all_sensors[0].enable=0xFF;				
		all_sensors[0].sensor_type=0x00;	
		
	    memcpy(&all_sensors[a],&all_sensors[0],sizeof(all_sensors[0]));		
		
#ifdef SCT_013_030
		all_sensors[0].offset=220.0f;
#endif

#ifdef VIRTUAL_SENSOR
		all_sensors[1].offset=220.0f;
#endif

#ifdef PAY_SENSOR
		strcpy_P((char *)all_sensors[2].name,PSTR("Payment"));
		all_sensors[2].offset=3.18f;
#endif
		eeprom_write_block(&all_sensors[a], (unsigned char*)(eeprom_sensors_config)+(a*sizeof(all_sensors[0])), sizeof(all_sensors[0]));
	}

	//sei();
	
}


void SetConfig(void)
{
	if(eeprom_read_byte((unsigned char*)EEPOP) != SPECIALOP)
	{
		// This board is initial state
		default_network(0); // Full initialize
	}
	else
	{
		cli();
		
#ifdef ADD_RELAY
		eeprom_read_block(&all_relay, (unsigned char*)eeprom_relay_config, sizeof(all_relay));
		//asm("WDR");
#endif
		
		eeprom_read_block(&all_sensors,(unsigned char*)(eeprom_sensors_config), sizeof(all_sensors));			
		eeprom_read_block(&server_data, (unsigned char*)eeprom_server_config, sizeof(server_data));
		eeprom_read_block(&NetworkParam,(unsigned char*)EEPOP, sizeof(NetworkParam));
		
		//sei();

	}

	
}


