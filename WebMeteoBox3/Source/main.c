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

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ip_arp_udp_tcp.h"
#include "enc28j60.h"
#include "dhcp_client.h"
#include "timeout.h"
#include "protocol.h"
#include "OWIPolled.h"
#include "OWIHighLevelFunctions.h"
#include "OWIBitFunctions.h"
#include "term.h"
#include "DHT.h"
#include "task_config.h"
#include "i2csoft.h"
#include "VirtualWire.h"
#include "led_display.h" 


// server listen port for www
#define MYWWWPORT 80
extern uint8_t buf[BUFFER_SIZE+1];

#ifdef ADD_LOGGER_2 // 14.08.2014
	//extern Server_config server_data[2]; // 2 логгера
	extern unsigned long int WebrefreshTime0;
	extern unsigned long int WebrefreshTime1;
#else
	extern unsigned long int WebrefreshTime0;
	//extern Server_config server_data[1]; // 1 логгер
#endif


extern StrConfigParam NetworkParam;
extern uint16_t buf_pos;
//extern uint8_t second;
extern char MPasswd[15]; // Password
//just taken from the BMP085 datasheet

extern int ac1;
extern int ac2;
extern int ac3;
extern unsigned int ac4;
extern unsigned int ac5;
extern unsigned int ac6;
extern int b1;
extern int b2;
//extern int mb;
extern int mc;
extern int md;
extern int temperature;
extern long pressure;

#ifdef VIRTUAL_SENSOR	
	extern float kwt_curr;
	extern float kwt_now;
	extern char m_flag;
#endif	

#ifdef PAY_SENSOR
	extern float pay_array[12];
	extern unsigned char pay_index;
	extern unsigned payflag;
#endif

#ifdef ENABLE_DOZIMETR	// Дозиметр 08.08.2014
extern unsigned int doz_buffer[DOZBUFSIZE+1];
#endif

/*
static int my_putchar(char c, FILE *stream){
	while((UCSR0A&(1<<UDRE0)) == 0);
	UDR0 = c;
	return 0;
}


static FILE mystdout = FDEV_SETUP_STREAM (my_putchar, NULL, _FDEV_SETUP_WRITE);
*/

int main(void){
int a;	
buf_pos=0; // сбда будем класть кол-во датчиков
//stdout = &mystdout;
cli();
		wdt_enable(WDTO_2S);
		
        mcu_init();   // Инициализируем регистры процессора
		
		//leddebugsignal(10);
		
		if (((PINB&(1<<0))>0)==0) {
			default_network(0);  // Full initialize
			//while ((((PINB&(1<<0))>0) == 0)) leddebugsignal(1);
		}
		

my_wait_1ms(250); // делаем задержку при включении	
asm("WDR");
		
		
#ifdef ENABLE_ETHERNET		
	SetConfig();  // Подгружаем параметры из EEPROM
#else
	LED_ON_OFF(0);	// гасим лампочку
#endif		

/*
while(1)
{
   //WriteNumber(5678);
   //WriteStringNumberToLCD(1);
   Val_to_LED_BUF(78.12);
   Send_Buff_to_LED();
   asm("WDR");
}*/


	for (a=0; a<MAX_SENSORS; a++)
		{
			//all_sensors[a].enable=0;
			all_sensors[a].sensor_type=0;
		}
		
#ifdef ENABLE_RF_MODULE
	PORTB |= (1<<6);
	vw_set_tx_pin(2);
	vw_set_rx_pin(3);
	vw_set_ptt_pin(0);
	vw_setup(2000);
#endif		

		eeprom_read_block(&MPasswd,          (void *)eeprom_password,  15);	
		
		WebrefreshTime0  = (unsigned long int)server_data[0].refresh_time*100;
#ifdef ADD_LOGGER_2			
		WebrefreshTime1  = (unsigned long int)server_data[1].refresh_time*100;	
#endif		
		
		
//-------------- Sensos configure ---------------------------------
#ifdef ENABLE_DOZIMETR
	for (a=0; a<DOZBUFSIZE; a++) doz_buffer[a]=20;
	all_sensors[buf[0]].sensor_type = SENSOR_DOZIMETR_BOX3;
	buf[0]++; // Нашли датчик	
#endif


#ifdef SCT_013_030 
		all_sensors[buf[0]].sensor_type = SENSOR_SCT_013_030;
		buf[0]++; // Нашли датчик
#endif

#ifdef VIRTUAL_SENSOR
		all_sensors[buf[0]].sensor_type = SENSOR_KWT_VIRTUAL;
		buf[0]++; // Нашли датчик
#endif

#ifdef PAY_SENSOR
		all_sensors[buf[0]].sensor_type = SENSOR_PAY_VIRTUAL;
		buf[0]++; // Нашли датчик 
#endif

//--- bmp180 ----------------------------------------------
#ifdef ENABLE_BMP180
		SoftI2CInit();
		SoftI2CStart();

		ac1=read_bmp085_int_register(0xAA);
		ac2=read_bmp085_int_register(0xAC);
		ac3=read_bmp085_int_register(0xAE);
		ac4=read_bmp085_int_register(0xB0);
		ac5=read_bmp085_int_register(0xB2);
		ac6=read_bmp085_int_register(0xB4);
		b1 = read_bmp085_int_register(0xB6);
		b2 = read_bmp085_int_register(0xB8);
		//mb = read_bmp085_int_register(0xBA);
		mc = read_bmp085_int_register(0xBC);
		md = read_bmp085_int_register(0xBE);
//--- bmp180 ----------------------------------------------

		if (bmp085_read_temperature_and_pressure(&temperature,&pressure) == 0)	
		{
			all_sensors[buf[0]].sensor_type = SENSOR_BMP180;
			buf[0]++; // Нашли датчик давления
		}
#endif			


#ifdef HUMIDITY_SENSOR
		all_sensors[buf[0]].sensor_type = SENSOR_ANALOG_HUMIDITY;
		buf[0]++; // Нашли датчик
#endif	

		OWI_Init(OWI_PIN_6); // DHT
		OWI_Init(OWI_PIN_7); // 1-Wire 

		search_and_configure_sensors(OWI_PIN_6,DHTPIN,&buf[0]);  // На 1 (рядом с mini USB)
		
#ifndef SCT_013_030 // Если включены клещи , значит нету второго датчика!
#ifndef ENABLE_DOZIMETR // Если включены клещи , значит нету второго датчика!
		search_and_configure_sensors(OWI_PIN_7,DHTPIN2,&buf[0]);
#endif				
#endif	
		
		 my_wait_1ms(751); // Если есть ds18b20 делаем задержку 

//-----------------------------------  enc28j60 initialization ------------------	
#ifdef ENABLE_ETHERNET
		ENC28J60_INIT_CHIP();
//Val_to_LED_BUF(1234.0);		
		sei();
//----------------DHCP initialization -------------- 03.06.2014 -----------------		
		if (NetworkParam.dhcp==0xFF)
		{
			init_mac(NetworkParam.mac);
			a=0;
			while(a==0){
				a=enc28j60PacketReceive(BUFFER_SIZE, buf);
				buf[BUFFER_SIZE]='\0';
				a=packetloop_dhcp_initial_ip_assignment(buf,a,NetworkParam.mac[5]);
				leddebugsignal(1);
			}
			dhcp_get_my_ip(NetworkParam.ip,NetworkParam.subnet,NetworkParam.gw);
			memcpy(NetworkParam.dns,NetworkParam.gw,4); // Делаем DNS как Gateway // 11.06.2014
			client_ifconfig(NetworkParam.ip,NetworkParam.subnet);		
		}
//----------------DHCP initialization -------------- 03.06.2014 -----------------				
        //init the ethernet/ip layer:
        init_udp_or_www_server(NetworkParam.mac,NetworkParam.ip);
        www_server_port(MYWWWPORT);
//-----------------------------------  enc28j60 initialization ------------------			
#else
	//sei(); // #ifdef ENABLE_ETHERNET
#endif	

//kwt_curr=274.33;	
        while(1)
		{
		//Val_to_LED_BUF(all_sensors[3].value);			
   
  		//Send_Buff_to_LED();
		  
		  if (!enc28j60linkup()) ENC28J60_INIT_CHIP();
		  
			
			asm("WDR");
#ifdef ENABLE_ETHERNET			
			process_http_server();
#endif			
			exe_sensors();
#ifdef ADD_RELAY			
			//exe_relay();
#endif

#ifdef VIRTUAL_SENSOR
			if (m_flag==1) {				
				kwt_now=read_current_sqrt(6,1);
				kwt_curr+=((kwt_now*all_sensors[1].offset)/3600.0)/1000.0; // Добавляем данные 1 раз в секунду.
				m_flag=0;
			}
#endif // VIRTUAL_SENSOR

#ifdef PAY_SENSOR	
			if (!payflag) { 				
				pay_array[pay_index]=(((kwt_now*all_sensors[1].offset)/60.0)/1000.0)*all_sensors[2].offset;
				pay_index++;
				if (pay_index>12) pay_index=0;
				payflag=1;
			}
#endif // PAY_SENSOR		
		}

return (0);
}
