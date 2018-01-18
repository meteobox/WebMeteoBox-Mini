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
#include <string.h>
#include <ctype.h>   
#include <math.h> 
#include <stdlib.h>
#include "ip_arp_udp_tcp.h"
#include "enc28j60.h"
#include "dhcp_client.h"
#include "protocol.h"
#include "term.h"
#include "DHT.h"
#include "net.h"
#include "dnslkup.h"
#include "i2csoft.h"
#include "task_config.h"
#include "VirtualWire.h"
#include "led_display.h" 


#define eeprom_config_s		0x81	
//___________________________________________________________________________

#define VREF         3.30

#define CONV_FACTOR 0.005 /* Коэфициент для дозиметра */ //0.0057//0.00812 

#define COUNT_AUTOUPDATE 5

typedef struct mode_block {
	char             *name;
	unsigned int     id;
}modes;

const static modes update_time[COUNT_AUTOUPDATE] =
{
	/*{"Max",  1},*/	   // Умножать на 100.
	{"5s",    5},
	{"30s",  30},
	/*{"30s",  30},*/
	{"1m",   60},
	/*{"3m",   180},*/
	{"5m",   305}, /* 300 */
	{"10m",  605} /* 600 */
	/*{"15m",  900},
	{"20m",  1200}*/
};


extern StrConfigParam NetworkParam;

/*
#ifdef ADD_LOGGER_2 // 14.08.2014
	extern Server_config server_data[2];
#else
	extern Server_config server_data[1]; // 1 логгер
#endif*/

#ifdef REBOOT_AFTER
	char reboot_counter=0;
#endif

#ifdef TIMER_RELAY 
	volatile unsigned int relay_timier;
	char humidity_secs=100; /* счетчик количества измерений*/
	unsigned long int last_relay_time; /* время с момента последнего включения реле 12.08.2016 */
#endif

unsigned long int sessionid; // Сессия соединения. устанавливается при правильном вводе логина и пароля.
unsigned long int usersession;
unsigned long int Time_ms=0L; // global timer
unsigned long int DS18B20refreshTime = 500; // *10ms
unsigned long int WebrefreshTime0 = 600; // 1min
#ifdef ADD_LOGGER_2	
unsigned long int WebrefreshTime1 = 600; // 1min
#endif
volatile unsigned long int TimeTs0=0L;
volatile unsigned long int TimeWt0=0L;
volatile unsigned long int TimeWt1=0L;
volatile unsigned char Tr0  = 0;
volatile unsigned char Wt0  = 0; // Logger 1 refresh timer
volatile unsigned char Wt1  = 0; // Logger 2 refresh timer
//volatile uint8_t second=0;
volatile uint8_t hour=0;
// global packet buffer
uint8_t buf[BUFFER_SIZE+1];
static char xbuf[260]; // Для разбора строки http запроса! 350 было 300 и работало нормально
uint8_t * bufrecv;
uint16_t buf_pos=0;
char tmpstr[25];
char tmpstr2[25];
//char MLogin[15];  // Login
char MPasswd[15]; // Password
// --- there should not be any need to changes things below this line ---
#define TRANS_NUM_GWMAC 1
 static uint8_t start_web_client=0; 
 static uint8_t gwmac[6];
 static uint8_t otherside_www_ip[4]={0,}; // will be filled by dnslkup
 static int8_t dns_state=0;
 static int8_t gw_arp_state=0;
 static uint8_t web_client_sendok=0;
 volatile uint8_t sec=0;
 volatile uint8_t sec_dhcp=0;
static char sec_cnt=0;

#ifdef VIRTUAL_SENSOR
	float kwt_curr;
	float kwt_now;
	volatile char m_flag;
#endif

#ifdef PAY_SENSOR	
	float pay_array[12];
	unsigned char pay_index;
	volatile unsigned payflag=0;
#endif

#ifdef ENABLE_DOZIMETR 
	volatile unsigned int tick_counter=0L; // для дозиметра
	unsigned int geiger_counter=0L;
	static volatile uint8_t sec_dozimetr=0;
	unsigned int doz_buffer[DOZBUFSIZE+1];
	unsigned char doz_index=0;
#endif

/*------------------- barometr bmp085 --------------------*/

 int ac1;
 int ac2;
 int ac3;
 unsigned int ac4;
 unsigned int ac5;
 unsigned int ac6;
 int b1;
 int b2;
 //int mb;
 int mc;
 int md;
 
 int temperature;
 long pressure;

uint16_t get_sensor_value_type_str(unsigned char num,float offset, char *buf1,char *buf2); 

 // parse a string that is an IP address and extract the IP to ip_byte_str
 uint8_t parse_ip(uint8_t *ip_byte_str,const char *str)
 {
	 char strbuf[4];
	 uint8_t bufpos=0;
	 uint8_t i=0;
	 while(i<4){
		 ip_byte_str[i]=0;
		 i++;
	 }
	 i=0;
	 while(*str && i<4){
		 // if a number then start
		 if (bufpos < 3 && isdigit(*str)){
			 strbuf[bufpos]=*str; // copy
			 bufpos++;
		 }
		 if (bufpos && *str == '.'){
			 strbuf[bufpos]='\0';
			 ip_byte_str[i]=(atoi(strbuf)&0xff);
			 i++;
			 bufpos=0;
		 }
		 str++;
	 }
	 if (i==3){ // must have read the first componets of the IP
		 strbuf[bufpos]='\0';
		 ip_byte_str[i]=(atoi(strbuf)&0xff);
		 return(0);
	 }
	 return(1);
 }

char LED_READ()
{
	return ((PIND&(1<<5))>0);
} 
 
 void LED_ON_OFF( char ckl )
 {	 
	 if (ckl == 1) PORTD |= (1<<5);
	 else PORTD &= ~(1<<5);
 }
 
 
 char PC_READ(char pin)
 {
	 return ((PINC&(1<<pin))>0);
 }
 

 char PB_READ(char pin)
 {
	return ((PINB&(1<<pin))>0);
 }
void PB_WRITE( char pin, char val )
 {
  if (val == 1) PORTB |= (1<<pin);
  else PORTB &= ~(1<<pin);
 }
 
 
 void LED_ON_OFF_RELAY_MINI( char ckl )
 {
	 if (ckl == 1) PORTB |= (1<<7);
	 else PORTB &= ~(1<<7);
 } 

 
 void PC_WRITE( char pin, char val )
 {
	 if (val == 1) PORTC |= (1<<pin);
	 else PORTC &= ~(1<<pin);
 }
/*
 void PF_WRITE( char pin, char val )
 {
	// if (val == 1) PORTF |= (1<<pin);
	// else PORTF &= ~(1<<pin);
 }
 */
 
 void leddebugsignal (char a) //Мигалка для отладки
 {
	 char b;

	 for (b=0; b<a; b++)
	 {
		 LED_ON_OFF(1);
#ifdef WEB_RELAY_MINI
		LED_ON_OFF_RELAY_MINI(1);
#endif		 
		 my_wait_1ms(50);
		 LED_ON_OFF(0);
#ifdef WEB_RELAY_MINI
		LED_ON_OFF_RELAY_MINI(0);
#endif		 
		 my_wait_1ms(50);
		 asm("WDR");
	 }
 }
 
/*
Ищем и настраиваем сенсоры на самую высокую точность
*/ 
char search_and_configure_sensors(unsigned char ds_pin, unsigned char dht_pin, unsigned char * count )
{
unsigned char a;	
unsigned char fcnt;

	OWI_SearchDevices(&all_sensors[*count], MAX_SENSORS-*count, ds_pin, &fcnt);
	
	for (a=*count; a<(fcnt+*count); a++) // На 1 (рядом с mini USB)
	{
		Read_scratchpad(ds_pin,a);
		if ((char)all_sensors[a].scratchpad[4] != 0x7F) Write_scratchpad(ds_pin,a);
		all_sensors[a].pin = ds_pin; 
		all_sensors[a].sensor_type=SENSOR_DS18B20;
	}
	
	if (fcnt==0) // Если нету ds18b20 поищем там DHT
	{
		ReadDHT(dht_pin);
		my_wait_1ms(250);asm("WDR");
		my_wait_1ms(250);asm("WDR");
		ReadDHT(dht_pin); // Check AM2301 in DS18B20 port 10.10.2013*/
		if (bGlobalErr==0) 
		{
			all_sensors[*count].pin   = dht_pin;
			all_sensors[*count+1].pin = dht_pin;
			all_sensors[*count].sensor_type = SENSOR_DHT;
			all_sensors[*count+1].sensor_type=SENSOR_DHT;
			*count+=2;			
		}
			
		return(0);
	}
	
	Start_18b20_Convert(ds_pin);
	*count+=fcnt;	
return(fcnt); //  Нашли fcnt датчиков ds18b20 	
} 
 

int bmp085_read_temperature_and_pressure(int* temperature, long int* pressure) {
	long ut;
	long up;
	long x1, x2, x3, b3, b5, b6, p;
	unsigned long b4, b7;
	char oversampling_setting = 3;
	
	if (ac1 == -1 && ac2 == -1 && ac3 == -1) return (-1);

	//cli();
	ut = bmp085_read_ut();
	
	bmp085_read_up(&up);
	//sei();
		
	//up=30000;
	
	//ut=27898; 
	//up=23843;

	//calculate the temperature
	x1 = ((long int)ut - ac6) * ac5 >> 15;
	x2 = ((long int) mc << 11) / (x1 + md);
	b5 = x1 + x2;
	*temperature = (b5 + 8) >> 4;

	//calculate the pressure
	b6 = b5 - 4000;
	x1 = (b2 * (b6 * b6 >> 12)) >> 11;
	x2 = ac2 * b6 >> 11;
	x3 = x1 + x2;

	//b3 = (((int32_t) ac1 * 4 + x3)<> 2;

	//if (oversampling_setting == 3)
	b3 = ((unsigned long int) ac1 * 4 + x3 + 2) << 1;
	//if (oversampling_setting == 2) b3 = ((int32_t) ac1 * 4 + x3 + 2);
	//if (oversampling_setting == 1) b3 = ((int32_t) ac1 * 4 + x3 + 2) >> 1;
	//if (oversampling_setting == 0) b3 = ((int32_t) ac1 * 4 + x3 + 2) >> 2;

	
	x1 = ac3 * b6 >> 13;
	x2 = (b1 * (b6 * b6 >> 12)) >> 16;
	x3 = ((x1 + x2) + 2) >> 2;
	b4 = (ac4 * (unsigned long int) (x3 + 32768)) >> 15;
	b7 = ((unsigned long int) up - b3) * (50000 >> oversampling_setting);
	p = b7 < 0x80000000 ? (b7 * 2) / b4 : (b7 / b4) * 2;
	
	/*
	  b7 = ((unsigned long)(up - b3) * (50000>>oversampling_setting));
	  if (b7 < 0x80000000)
	  p = (b7<<1)/b4;
	  else
	  p = (b7/b4)<<1;
	  */

	x1 = (p >> 8) * (p >> 8);
	x1 = (x1 * 3038) >> 16;
	x2 = (-7357 * p) >> 16;
	*pressure = p + ((x1 + x2 + 3791) >> 4);
	
	return (0);
}

 /*------------------- barometr bmp085 --------------------*/
 
 unsigned int read_adc_int(unsigned char adc_input)
 {
	// float adc,bar;
	 ADMUX=adc_input | (ADC_REF_AVCC & 0xff);
	 // Start the AD conversion
	 ADCSRA|=0x40;
	 // Wait for the AD conversion to complete
	 while ((ADCSRA & 0x10)==0);
	 ADCSRA|=0x10;

	 return ADCW;
 }
 
 float read_current()
 {
	 unsigned int i,a;
	 float k=0,l=0;
	 
	 a=read_adc_int(6);
	 
	 // for(i=0; read_ADC(1)<10 && i<100; i++)
	 // continue;

	 for (i=0; i<400; i++)
	 {
		 k+= abs(read_adc_int(0)-a);
		 _delay_us(25);
	 }
	 //return k/256;
	 k/= 400;
	 //l=k/1024.0*VREF*30;
	 l=k/1023.0*VREF*30; // 02.02.2014
	 //l*= .10;	// LSB for 30A range
	 return 220*l;   // return WATT 220Vrms x Irms
	 //return l;
 }

 float read_current_sqrt(char adc_num, int coef) /* вычисление среднеквадратичного */
 {
	 unsigned int i,a;
	 float k=0,l=0,q=0;
	 
	 a=read_adc_int(7);
	 
	 // for(i=0; read_ADC(1)<10 && i<100; i++)
	 // continue;

	 for (i=0; i<400; i++)
	 {
		 q=abs(read_adc_int(adc_num)-a);
		 
		 
		 k += (q*q);
		 
		 _delay_us(25);
	 }
	 //return k/256;
	 k/=400;
	 q=sqrt(k);
	 
	 //l=q/1024.0*VREF*1000;
	 //return l;
	 
	 //l=q/1023.0*VREF*30;
	 l=q/1023.0*VREF*SCT_COEFF;
	 //l*= .10;	// LSB for 30A range
	 //return 220*l;   // return WATT 220Vrms x Irms
	 return coef*l;   // return WATT 220Vrms x Irms
	 //return l;
 }


 float read_current_volt(char adc_num)
 {
	 unsigned int i,a;
	 float k=0,l=0,q=0;
	 
	 a=read_adc_int(6);
	 
	 // for(i=0; read_ADC(1)<10 && i<100; i++)
	 // continue;

	 for (i=0; i<400; i++)
	 {
		 q=abs(read_adc_int(adc_num)-a);
		 
		 
		 k += (q*q);
		 
		 _delay_us(25); // 25
	 }
	 //return k/256;
	 k/=400;
	 q=sqrt(k);
	 
	 //l=q/1024.0*VREF*1000;
	 //return l;
	 
	 l=q/1023.0*VREF;
	 //l*= .10;	// LSB for 30A range
	 //return 411.4*l;   // return WATT 220Vrms x Irms
	 return l;
 }


// the __attribute__((unused)) is a gcc compiler directive to avoid warnings about unsed variables.
void browserresult_callback(uint16_t webstatuscode,uint16_t datapos, uint16_t len __attribute__((unused))){
	if (webstatuscode==200){
		web_client_sendok++;
		//printf("Apache OK %d\r\n",web_client_sendok);
	}
	// clear pending state at sucessful contact with the
	// web server even if account is expired:
	//printf("callback start_web_client=%d\r\n",start_web_client);
	if (start_web_client==2) start_web_client=3;
}

// the __attribute__((unused)) is a gcc compiler directive to avoid warnings about unsed variables.
void arpresolver_result_callback(uint8_t *ip __attribute__((unused)),uint8_t transaction_number,uint8_t *mac){
	uint8_t i=0;
	if (transaction_number==TRANS_NUM_GWMAC){
		// copy mac address over:
		while(i<6){gwmac[i]=mac[i];i++;}
	}
}

#ifdef ENABLE_LCD_74HC595
ISR(TIMER2_COMPA_vect){
	 Send_Buff_to_LED();	
}
#endif	


#ifdef ENABLE_DOZIMETR 
ISR(PCINT2_vect){
	if( bit_is_clear(PIND,7)) tick_counter++;
}
#endif




ISR(TIMER0_COMPA_vect){
Time_ms++; 

#ifdef TIMER_RELAY
	last_relay_time++; /* считаем время от запуска реле. при включении реле сбросим эту переменную */
#endif	


	sec_cnt++;
	if (sec_cnt>=100)
	{
		sec_cnt=0;
		sec++;		
		
#ifdef TIMER_RELAY
		last_relay_time++; /* считаем время от запуска реле. при включении реле сбросим эту переменную */
		if (PC_READ(all_relay[0].pio))
		{
			if (all_relay[0].max > 0)
			{			
				if (relay_timier>=(unsigned int)(all_relay[0].max))
				{
					PC_WRITE(all_relay[0].pio,RELAY_OFF);
					//relay_timier=0;
				}
				else relay_timier++;	
			}
		}
#endif	
		
		
#ifdef ENABLE_DOZIMETR	// Дозиметр 08.08.2014
	sec_dozimetr++;
	if (sec_dozimetr>60)
	{
		sec_dozimetr=0;
		geiger_counter=tick_counter;
	
		doz_buffer[doz_index]=tick_counter;
		doz_index++;
		if (doz_index>DOZBUFSIZE) doz_index=0;
	
		tick_counter=0;
	}
#endif		
		

#ifdef VIRTUAL_SENSOR		
		m_flag=1;
#endif		
/*
		second++;
		if (second==60) 
		{ 
			second=0;
		}*/
				
		sec_dhcp++;
	    if (sec_dhcp>5){
		    sec_dhcp=0;
		    dhcp_6sec_tick();
	    }
	}
	
 if (Wt0 == 1){
	 if( TimeWt0 > 0) --TimeWt0;
	 else { Wt0 = 0; }
 }
 
 if (Wt1 == 1){
	 if( TimeWt1 > 0) --TimeWt1;
	 else { Wt1 = 0; }
 }
 
 if (Tr0 == 1){
	 if( TimeTs0 > 0) --TimeTs0;
	 else { Tr0 = 0; 
	 
#ifdef PAY_SENSOR		 
	 payflag=0;
#endif	 
	 }
 }

}

unsigned char D2C(
char c	/**< is a Hex(0x00~0x0F) to convert to a character */
)
{
	uint16_t t = (uint16_t) c;
	if (t >= 0 && t <= 9)
	return '0' + c;
	if (t >= 10 && t <= 15)
	return 'A' + c - 10;

	return c;
}

char C2D(
char c	/**< is a character('0'-'F') to convert to HEX */
)
{
	if (c >= '0' && c <= '9')
	return c - '0';
	if (c >= 'a' && c <= 'f')
	return 10 + c -'a';
	if (c >= 'A' && c <= 'F')
	return 10 + c -'A';

	return (char)c;
}

unsigned char h2int(char c)
{
	if (c >= '0' && c <='9'){
		return((unsigned char)c - '0');
	}
	if (c >= 'a' && c <='f'){
		return((unsigned char)c - 'a' + 10);
	}
	if (c >= 'A' && c <='F'){
		return((unsigned char)c - 'A' + 10);
	}
	return(0);
}

char* my_itoa_long(
unsigned long int value,	/**< is a integer value to be converted */
char* str	/**< is a pointer to string to be returned */)
{
	ultoa(value,str,10); // add 15.08.2014
	return(str);
}

unsigned int ATOI(char* str, int base)
{
	int num = 0;

	while (*str != 0) num = num * base + C2D(*str++);

	return num;
}


void data_get(char *x, char delimiter, char base)
{
	int a,b,n=0,j=0;
	char buf[10];

	b = strlen(x);
	for (a=0; a<b; a++)
	{
		if (*(x+a) == delimiter) // if (*(x+a) == '.')
		{
			buf[j++] = '\0';
			xbuf[n++] = (char)ATOI(buf,base);
			j=0;
		}
		else buf[j++] = *(x+a);
	}
	
	buf[j++] = '\0';
	xbuf[n++] = ATOI(buf,base);
} // data_get


char * findstr(char * s, char * find, char * stop)
{
	int len,len2;
	char *x2;	
	
	len = strlen(find);
	x2=strstr(s,find);
	if (x2==NULL) { xbuf[0]=0x00; return(xbuf); }
	
	strcpy(xbuf,&x2[len]);
	len = strlen(xbuf);
	
	x2=strstr(xbuf,stop);
	len2 = strlen(x2);
	len = len-len2;
	

	xbuf[len] = '\0';
	s = xbuf;
	
	return s; 
}

char * findstr_P(char * s, const char * find, const char * stop) // Храним константы в памяти программ 18.09.2014
{
	int len,len2;
	char *x2;
	
	len = strlen_P(find);
	x2=strstr_P(s,find);
	if (x2==NULL) { xbuf[0]=0x00; return(xbuf); }
	
	strcpy(xbuf,&x2[len]);
	len = strlen(xbuf);
	
	x2=strstr_P(xbuf,stop);
	len2 = strlen(x2);
	len = len-len2;
	
	xbuf[len] = '\0';
	s = xbuf;
	
	return s;
}

void urldecode(char *urlbuf)
{
	char c;
	char *dst;
	dst=urlbuf;
	while ((c = *urlbuf)) {
		if (c == '+') c = ' ';
		if (c == '%') {
			urlbuf++;
			c = *urlbuf;
			urlbuf++;
			c = (h2int(c) << 4) | h2int(*urlbuf);
		}
		*dst = c;
		dst++;
		urlbuf++;
	}
	*dst = '\0';
}

#ifdef ADD_RELAY
void exe_relay() // Работа с Реле
{
	unsigned char a,b;
	for (a=0; a<MAX_RELAY; a++)
	if (all_relay[a].flag != 0)
	{
		for (b=0; b<MAX_SENSORS; b++)
		{
			if (all_sensors[b].enable==0xFF && all_sensors[b].sensor_type !=0)
			{
				if (memcmp(all_sensors[b].id,all_relay[a].id,8)==0)
				{
					if ((all_sensors[b].value+all_sensors[b].offset)<all_relay[a].min) 
					{
						if (a<4) PC_WRITE(all_relay[a].pio,RELAY_ON); else PB_WRITE(all_relay[a].pio,RELAY_ON); 
					}
						
					if ((all_sensors[b].value+all_sensors[b].offset)>all_relay[a].max) 
					{
						if (a<4) PC_WRITE(all_relay[a].pio,RELAY_OFF); else PB_WRITE(all_relay[a].pio,RELAY_OFF);
					}
				}
			}
		}
	}
}
#endif

#ifdef ENABLE_RF_MODULE 
void exe_wireless() 
{
unsigned long diff,day,hr,min,sec;
static long int ccc=0;
		
	diff=floor(Time_ms/100); 
	day=floor(diff/60/60/24);
	diff-=day*60*60*24;
	hr =floor(diff/60/60);
	diff-=hr*60*60;
	min=floor(diff/60);
	diff -= min*60;
	sec=diff;
	
	ccc++;		
		
	my_itoa_long(hr,tmpstr2);
	strcat(tmpstr2,":");
	my_itoa_long(min,tmpstr);
	strcat(tmpstr2,tmpstr);	
	strcat(tmpstr2,":");
	my_itoa_long(sec,tmpstr);
	strcat(tmpstr2,tmpstr);
	strcat(tmpstr2,"  ");	
	//strcat(tmpstr2," LEXA");
	my_itoa_long(ccc,tmpstr);
	strcat(tmpstr2,tmpstr);	
	
    PORTC |= (1<<6);
	
	leddebugsignal(10);
	
	vw_send((uint8_t*)tmpstr2, strlen(tmpstr2));
	vw_wait_tx();

	PORTC &= ~(1<<6);
	
/*
		get_sensor_value_type_str(0,all_sensors[0].offset,tmpstr,tmpstr2);
		strcpy(tmpstr2,(char *)all_sensors[0].name);
		strcat(tmpstr," ");
		strcat(tmpstr,tmpstr2);

		vw_send((uint8_t*)tmpstr, strlen(tmpstr));
		vw_wait_tx();
		
		get_sensor_value_type_str(1,all_sensors[1].offset,tmpstr,tmpstr2);
		strcpy(tmpstr2,(char *)all_sensors[1].name);
		strcat(tmpstr," ");
		strcat(tmpstr,tmpstr2);
		vw_send((uint8_t*)tmpstr, strlen(tmpstr));
		vw_wait_tx();*/

}
#endif	

#ifdef ENABLE_DOZIMETR	// Дозиметр
float calculate_doz(void)
{
	unsigned char a;
	float tmp=0;

	
	for (a=0; a<DOZBUFSIZE; a++) tmp+=doz_buffer[a];
	
	tmp=(tmp/DOZBUFSIZE)*CONV_FACTOR;
	
	//if (tmp<0.05) tmp = 0.05;
	//if (tmp>0.29) tmp = 0.29;
	
	return (tmp); // Среднее CPM от дозиметра
}
#endif


void exe_sensors() // Опрашиваем все сенсоры и обновляем массив all_sensors 28.12.2012
{
unsigned char a;
#ifdef PAY_SENSOR
	unsigned char b;
#endif

	if (!Tr0) { 
		LED_ON_OFF(!LED_READ());

#ifdef WEB_RELAY_MINI		
		LED_ON_OFF_RELAY_MINI(!((PINB&(1<<7))>0));
#endif	

#ifdef TIMER_RELAY
	humidity_secs++;	
#endif	


#ifdef REBOOT_AFTER
	if (reboot_counter>=REBOOT_AFTER)
	{
		ENC28J60_INIT_CHIP();
		reboot_counter=0;
		TimeWt0=1000; Wt0=1; // перезаряжаем на 10 секунд
#ifdef ADD_LOGGER_2				
		TimeWt1=1000; Wt1=1; // перезаряжаем на 10 секунд
#endif		
	}
#endif




		DS18B20refreshTime=500; //опрашиваем датчики 1 раз в 5 секунд!
		TimeTs0=DS18B20refreshTime;	
		
#ifdef ADD_RELAY  // 29.05.2015
#ifndef TIMER_RELAY  // 06.05.2016 /* шеддулер не нужен если режим автовыключения */
		exe_relay();
#endif					
#endif			
		
		
		for (a=0; a<MAX_SENSORS; a++) {
			switch (all_sensors[a].sensor_type) {

#ifdef ENABLE_DOZIMETR
				case SENSOR_DOZIMETR_BOX3:
						all_sensors[a].type=6; //  ДОЗА в uSv/hr  28.02.2014
						all_sensors[a].value=calculate_doz(); // read_current_sqrt(6,1); // read_current_sqrt
						all_sensors[a].id[0]=0xAA; // Создаём уникальный ID на базе mac адреса
						all_sensors[a].id[1]=0x77;
						memcpy(&all_sensors[a].id[2],NetworkParam.mac,6);
				break;	
#endif				
				
				
#ifdef ENABLE_BMP180
				case SENSOR_BMP180:
					if (bmp085_read_temperature_and_pressure(&temperature,&pressure) == 0)
					{
						all_sensors[a].type=3; // 3 - Давление
						all_sensors[a].value=(pressure/133.32); // Преобразуем Паскали в Мм
						//all_sensors[cnt].value=temperature; // Преобразуем Паскали в Мм
						all_sensors[a].id[0]=0xAA; // Создаём уникальный ID на базе mac адреса
						all_sensors[a].id[1]=0x0D;
						memcpy(&all_sensors[a].id[2],NetworkParam.mac,6);
					}
				break;	
#endif		

#ifdef SCT_013_030
				case SENSOR_SCT_013_030:
					all_sensors[a].type=4; // 4 - Мощность Ватт
					all_sensors[a].value=kwt_now; // read_current_sqrt(6,1); // read_current_sqrt
					all_sensors[a].id[0]=0xAC; // Создаём уникальный ID на базе mac адреса
					all_sensors[a].id[1]=0x0E;
					memcpy(&all_sensors[a].id[2],NetworkParam.mac,6);
				break;
#endif		

#ifdef VIRTUAL_SENSOR
					case SENSOR_KWT_VIRTUAL:
						all_sensors[a].type=8; // 8 - Мощность КилоВатт в Час
						all_sensors[a].value=kwt_curr; // read_current_sqrt(6,1); // read_current_sqrt
						all_sensors[a].id[0]=0xAC; // Создаём уникальный ID на базе mac адреса
						all_sensors[a].id[1]=0x1E;
						memcpy(&all_sensors[a].id[2],NetworkParam.mac,6);
					break;
#endif

#ifdef PAY_SENSOR
				case SENSOR_PAY_VIRTUAL:
					all_sensors[a].value=0;
					for (b=0; b<12; b++) all_sensors[a].value+=pay_array[b];
					all_sensors[a].type=9; // 9 - Мощность КилоВатт в Час в Рублях
					//pay_array=kwt_curr*all_sensors[2].offset; // read_current_sqrt(6,1); // read_current_sqrt
					all_sensors[a].value=all_sensors[a].value/12.0f; // Среднее за минуту. 1 раз в 5 секунд
					all_sensors[a].id[0]=0xAC; // Создаём уникальный ID на базе mac адреса
					all_sensors[a].id[1]=0x2E;
					memcpy(&all_sensors[a].id[2],NetworkParam.mac,6);
				break;
#endif

#ifdef HUMIDITY_SENSOR
				case SENSOR_ANALOG_HUMIDITY:
					if (humidity_secs>12) /* измеряем раз в минуту */
					{
						humidity_secs=0;
						PORTC |= (1<<1);
						my_wait_1ms(150);
						all_sensors[a].type=11; /* влажность %d */
						all_sensors[a].value=read_adc_int(2) / 1023.0*VREF; ///1023.0*VREF; 
						all_sensors[a].id[0]=0xDA; // Создаём уникальный ID на базе mac адреса
						all_sensors[a].id[1]=0xEE;
						memcpy(&all_sensors[a].id[2],NetworkParam.mac,6);
						PORTC &= ~(1<<1);
					}
				break;
#endif
				
				case SENSOR_DS18B20:
					if (DS18B20_ReadTemperature_Fast_Float(all_sensors[a].pin, all_sensors[a].id,&all_sensors[a].value) == READ_SUCCESSFUL)
					{
						all_sensors[a].type=1; // 1 - температура
						all_sensors[a].error=0;
					}
					else all_sensors[a].error=1;
					Start_18b20_Convert(all_sensors[a].pin);
				break;
				
				case SENSOR_DHT:
					ReadDHT(all_sensors[a].pin);
					if (bGlobalErr==0) {// Есть данные с AM3202 - датчик  влажности и температуры
						// 2 датчик влажности
						all_sensors[a].error=0;
						all_sensors[a].type=2; // 2 - влажность
						all_sensors[a].value=(dht_hum/10.0);
						all_sensors[a].id[0]=0xAA; // Создаём уникальный ID на базе mac адреса
						all_sensors[a].id[1]=0x1A+a;
						memcpy(&all_sensors[a].id[2],NetworkParam.mac,6);
						a++;
						// 2 датчик темературы
						all_sensors[a].error=0;
						all_sensors[a].type=1; // 1 - температура
						all_sensors[a].value=(dht_term/10.0);
						all_sensors[a].id[0]=0xAA; // Создаём уникальный ID на базе mac адреса
						all_sensors[a].id[1]=0x2A+a;
						memcpy(&all_sensors[a].id[2],NetworkParam.mac,6);
					}
					else // Ошибка ?!
					{
						all_sensors[a].error=1;
						a++;
						all_sensors[a].error=1;
					}
				break;				
			}
		
		} // for (a=0; a<MAX_SENSORS; a++) { 	
		
//------------------------------------------------------------------------------------------------------
#ifdef ENABLE_LCD_74HC595
for (a=0; a<MAX_SENSORS; a++) {
	if (all_sensors[a].name[0]=='*') { Val_to_LED_BUF(all_sensors[a]); break;}
}
#endif
//------------------------------------------------------------------------------------------------------
			

	Tr0=1;
	}

}


uint16_t http200ok(void)
{
     return(fill_tcp_data_p(buf,0,PSTR("HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nPragma: no-cache\r\n\r\n")));
}

uint16_t http404(void)
{
	return(fill_tcp_data_p(buf,0,PSTR("HTTP/1.0 404 Not Found\r\nContent-Type: text/html\r\n\r\nErr")));
}

uint16_t page_header(uint8_t *data,uint16_t pl)
{
	//return(fill_tcp_data_p(data,pl,PSTR("<html><head><title>WebMeteoBox3</title><meta http-equiv=\"Content-Type\" content=\"text/html; charset=windows-1251\" /></head><body>")));
	return(fill_tcp_data_p(data,pl,PSTR("<html><head><meta charset=\"windows-1251\"/></head><body>")));	
	//return(fill_tcp_data_p(data,pl,PSTR("<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=windows-1251\"/></head><body>")));	
	//return((uint8_t *)data);
}

/*
uint16_t input_type_text_name_(uint8_t *data,uint16_t pl)
{
	return(fill_tcp_data_p(data,pl,PSTR("<input type=text name=")));	
}
*/

uint16_t input_type_text_name_2(uint8_t *data,uint16_t pos, const char * start, const char * name, const char * val, const char * end)
{
uint16_t pl;

    pl=fill_tcp_data_p(data,pos,start);
	pl=fill_tcp_data_p(data,pl,PSTR("<input type=text name="));	
	pl=fill_tcp_data_p(data,pl,name);
	pl=fill_tcp_data_p(data,pl,PSTR(" value=\""));	
	pl=fill_tcp_data  (data,pl,val);
	pl=fill_tcp_data_p(data,pl,PSTR("\">"));
	pl=fill_tcp_data_p(data,pl,end);
	return(pl);
}


uint16_t get_sensor_value_type_str(unsigned char num,float offset, char *buf1,char *buf2) // buf1=value. buf2=char comment
{
// код стал меньше! 13.01.2015	
// Program Memory Usage 	:	30638 bytes   93,5 % Full
// Data Memory Usage 		:	1797 bytes    87,7 % Full
char param[6];
float myval=0;

	switch (all_sensors[num].type) {
		case 1:   myval=all_sensors[num].value+offset; strcpy_P(param,PSTR("%2.2f")); strcpy_P(buf2,PSTR("&deg;C"));
			break;
		case 2:   myval=all_sensors[num].value+offset; strcpy_P(param,PSTR("%2.1f")); strcpy_P(buf2,PSTR("%"));		
			break;
		case 3:   myval=all_sensors[num].value+offset; strcpy_P(param,PSTR("%3.1f")); strcpy_P(buf2,PSTR("mm"));
			break;
			
#ifdef SCT_013_030			
		case 4:   if (offset==0.00) { // Для токовых клещей
					myval=all_sensors[num].value; strcpy_P(param,PSTR("%3.1f"));
					strcpy_P(buf2,PSTR("A"));
				  }
					else { myval=all_sensors[num].value*offset; strcpy_P(param,PSTR("%3.1f")); strcpy_P(buf2,PSTR("W")); }
			break;
#endif			
/*			
		case 5:   sprintf(buf1,"%3.3f",all_sensors[num].value*offset); strcpy_P(buf2,PSTR("V"));
			break;
*/
#ifdef ENABLE_DOZIMETR
		case 6:   myval=all_sensors[num].value+offset; strcpy_P(param,PSTR("%1.2f")); strcpy_P(buf2,PSTR("uSv/hr"));
			break;
#endif			
			
#ifdef VIRTUAL_SENSOR			
/*
		case 7:   sprintf(buf1,"%3.1f",all_sensors[num].value*offset); strcpy_P(buf2,PSTR("kWm"));
			break;	
*/					
		case 8:   myval=all_sensors[num].value; strcpy_P(param,PSTR("%3.2f")); strcpy_P(buf2,PSTR("kW/h"));
			break;	
#endif		

#ifdef PAY_SENSOR			
		case 9:   myval=all_sensors[num].value; strcpy_P(param,PSTR("%1.3f")); strcpy_P(buf2,PSTR("Руб/мин"));
		//case 9:   myval=all_sensors[num].value; strcpy_P(param,PSTR("%1.3f")); strcpy_P(buf2,PSTR("грн/мин"));
			break;	
#endif
			
		case 10:  myval=all_sensors[num].value; strcpy_P(param,PSTR("%f")); // для отладки %.0f"
			//break;
			
#ifdef HUMIDITY_SENSOR
		case 11:   myval=all_sensors[num].value; strcpy_P(param,PSTR("%f")); strcpy_P(buf2,PSTR(" Вольт"));
			break;
#endif
			
	}
	
	sprintf(buf1,param,myval); 

return(0);		
}


uint16_t print_webpage_mainframe(uint8_t *buf)
{
	uint16_t pl;
	pl=http200ok();
	my_itoa_long(sessionid,tmpstr);
	pl=fill_tcp_data_p(buf,pl,PSTR("<html><frameset cols=\"170,*\"><frameset rows=\"260, *\" scrolling=\"no\">"));
	pl=fill_tcp_data_p(buf,pl,PSTR("<frame src=/menu?sid="));
	pl=fill_tcp_data(buf,pl,tmpstr);
	pl=fill_tcp_data_p(buf,pl,PSTR(" name=mf>")); // menu frame
	pl=fill_tcp_data_p(buf,pl,PSTR("<frame src=/dev?sid="));
	pl=fill_tcp_data(buf,pl,tmpstr);
	pl=fill_tcp_data_p(buf,pl,PSTR(" name=lf>"));	// sensor frame
#ifdef TIMER_RELAY
	pl=fill_tcp_data_p(buf,pl,PSTR("</frameset><frame src=/rel?num=0&sid="));
#else	
	pl=fill_tcp_data_p(buf,pl,PSTR("</frameset><frame src=/info?sid="));
#endif	
	
	pl=fill_tcp_data(buf,pl,tmpstr);	
	pl=fill_tcp_data_p(buf,pl,PSTR(" name=rf>")); 
	pl=fill_tcp_data_p(buf,pl,PSTR("</frameset></html>"));
	return(pl);
}

uint16_t print_webpage_tcpip_settings(uint8_t *buf)
{
	uint16_t pl;
	pl=http200ok();
	pl=page_header(buf,pl);

#ifdef ENABLE_TCP_SETINGS
    strcpy_P(tmpstr2,PSTR("%d.%d.%d.%d"));
	pl=fill_tcp_data_p(buf,pl,PSTR("<pre><form method=get>"));
	
	sprintf(tmpstr,tmpstr2,NetworkParam.ip[0],NetworkParam.ip[1],NetworkParam.ip[2],NetworkParam.ip[3]); // делаем строку с IP	
	pl=input_type_text_name_2(buf,pl,PSTR("IP   "),PSTR("i"),tmpstr,PSTR("<br>MAC  "));
	sprintf_P(tmpstr,PSTR("%02x:%02x:%02x:%02x:%02x:%02x"),NetworkParam.mac[0],NetworkParam.mac[1],NetworkParam.mac[2],NetworkParam.mac[3],NetworkParam.mac[4],NetworkParam.mac[5]); // делаем строку с MAC
	pl=input_type_text_name_2(buf,pl,PSTR(""),PSTR("m"),tmpstr,PSTR("<br>MASK "));
	sprintf(tmpstr,tmpstr2,NetworkParam.subnet[0],NetworkParam.subnet[1],NetworkParam.subnet[2],NetworkParam.subnet[3]); // делаем строку с MASK
	pl=input_type_text_name_2(buf,pl,PSTR(""),PSTR("s"),tmpstr,PSTR("<br>GW   "));
	sprintf(tmpstr,tmpstr2,NetworkParam.gw[0],NetworkParam.gw[1],NetworkParam.gw[2],NetworkParam.gw[3]); // делаем строку с Gateway	
	pl=input_type_text_name_2(buf,pl,PSTR(""),PSTR("g"),tmpstr,PSTR("<br>DNS  "));
	sprintf(tmpstr,tmpstr2,NetworkParam.dns[0],NetworkParam.dns[1],NetworkParam.dns[2],NetworkParam.dns[3]); // делаем строку с DNS
	pl=input_type_text_name_2(buf,pl,PSTR(""),PSTR("d"),tmpstr,PSTR(""));
	
	pl=fill_tcp_data_p(buf,pl,PSTR("<br><input type=checkbox name=dh"));
	if (NetworkParam.dhcp==0xFF) pl=fill_tcp_data_p(buf,pl,PSTR(" checked"));
	//pl=fill_tcp_data_p(buf,pl,PSTR(">DHCP<br>"));
	pl=fill_tcp_data_p(buf,pl,PSTR(">DHCP<br><input type=hidden name=sid value=\""));	
	my_itoa_long(sessionid,tmpstr); 
	pl=fill_tcp_data(buf,pl,tmpstr); // Вставляем сессию
	pl=fill_tcp_data_p(buf,pl,PSTR("\"><input type=submit name=act value=\"Save & Reboot\"></form>"));		

#else
	pl=fill_tcp_data_p(buf,pl,PSTR("<center><br><br>TCP/IP configure ConfigTool.exe only!"));
#endif
	return(pl);
}

#ifdef ADD_RELAY
uint16_t print_relay_settings(uint8_t *buf, unsigned char num)
{
	uint16_t pl,a;
	pl=http200ok();
	//pl=page_header(buf,pl);

	//pl=fill_tcp_data_p(buf,pl,PSTR("Relay "));
	//sprintf(tmpstr,"%d",num);
	//pl=fill_tcp_data(buf,pl,tmpstr);

	//pl=fill_tcp_data_p(buf,pl,PSTR(" Settings<br><form method=get action=\"\">"));
	
	pl=fill_tcp_data_p(buf,pl,PSTR("<pre><form method=get>"));
	
	pl=fill_tcp_data_p(buf,pl,PSTR("<input type=submit name=snd value=On style=\"width:65px;height:65px;\">"));
	pl=fill_tcp_data_p(buf,pl,PSTR("   <input type=submit name=snd value=Off style=\"width:65px;height:65px;\"> "));	
	
	//pl=fill_tcp_data_p(buf,pl,PSTR("<input type=submit name=snd value=On>"));
	//pl=fill_tcp_data_p(buf,pl,PSTR("   <input type=submit name=snd value=Off> "));	
	
	
	if (num<4) a=PC_READ(all_relay[num].pio);
	else a=PB_READ(all_relay[num].pio);
	
    if (a==RELAY_ON) { strcpy_P(tmpstr,PSTR("ON")); } else {  strcpy_P(tmpstr,PSTR("OFF")); }	
	//pl=fill_tcp_data_p(buf,pl,PSTR(">"));	
    pl=fill_tcp_data(buf,pl,tmpstr); 

	pl=fill_tcp_data_p(buf,pl,PSTR("<p><input type=hidden name=num value="));
	my_itoa_long(num,tmpstr);
	pl=fill_tcp_data(buf,pl,tmpstr); 	
	pl=fill_tcp_data_p(buf,pl,PSTR("><input type=hidden name=sid value="));
	my_itoa_long(sessionid,tmpstr);
	pl=fill_tcp_data(buf,pl,tmpstr); // Вставляем сессию
	//pl=fill_tcp_data_p(buf,pl,PSTR(">"));

#ifdef TIMER_RELAY
	sprintf_P(tmpstr,PSTR("%.0f"),all_relay[num].max);
	pl=input_type_text_name_2(buf,pl,PSTR(">\rAuto off second:"),PSTR("max"),tmpstr,PSTR(""));
	pl=fill_tcp_data_p(buf,pl,PSTR("<p>seconds passed: "));
	sprintf_P(tmpstr,PSTR("%d"),relay_timier);
	pl=fill_tcp_data(buf,pl,tmpstr);

#ifdef HUMIDITY_SENSOR	
	pl=fill_tcp_data_p(buf,pl,PSTR("<p>Volt: "));	
	sprintf_P(tmpstr,PSTR("%f"),all_sensors[0].value);
	pl=fill_tcp_data(buf,pl,tmpstr);	
#endif	
	pl=fill_tcp_data_p(buf,pl,PSTR("<p>Last relay on sec: "));
	my_itoa_long(last_relay_time/100,tmpstr);
	pl=fill_tcp_data(buf,pl,tmpstr);
#else	
	pl=fill_tcp_data_p(buf,pl,PSTR(">")); // 22.11.2016	
#endif	
	
	
	
  //  pl=fill_tcp_data_p(buf,pl,PSTR("<p><input type=submit name=snd value=Save></form>"));	

#ifndef TIMER_RELAY

	sprintf_P(tmpstr,PSTR("%2.2f"),all_relay[num].min);
	pl=input_type_text_name_2(buf,pl,PSTR("\rMIN:"),PSTR("min"),tmpstr,PSTR(""));
	sprintf_P(tmpstr,PSTR("%2.2f"),all_relay[num].max);
	pl=input_type_text_name_2(buf,pl,PSTR("\rMAX:"),PSTR("max"),tmpstr,PSTR(""));	
	pl=fill_tcp_data_p(buf,pl,PSTR("<p><select name=sen><option value=100>Not used</option>"));	
	
	for (a=0; a<MAX_SENSORS; a++)
	{
		if (all_sensors[a].enable==0xFF && all_sensors[a].sensor_type !=0)
		{
			pl=fill_tcp_data_p(buf,pl,PSTR("<option value="));
			//sprintf(tmpstr,"%d",a);
			itoa(a,tmpstr,10);
			pl=fill_tcp_data(buf,pl,tmpstr);
			
			if (memcmp(all_sensors[a].id,all_relay[num].id,8)==0) pl=fill_tcp_data_p(buf,pl,PSTR(" selected"));
			
			pl=fill_tcp_data_p(buf,pl,PSTR(">"));
			pl=fill_tcp_data(buf,pl,(char *)all_sensors[a].name);
			pl=fill_tcp_data_p(buf,pl,PSTR("</option>"));
		}
	}
	pl=fill_tcp_data_p(buf,pl,PSTR("</select><p><input type=submit name=snd value=Save></form>"));
#endif		
		
	return(pl);
}

#endif


uint16_t print_webpage_logger_settings(uint8_t *buf, unsigned char num)
{
	uint16_t pl,a;
	pl=http200ok();
	//return(pl);

	pl=fill_tcp_data_p(buf,pl,PSTR("<form method=get>"));
	//pl=fill_tcp_data_p(buf,pl,PSTR("<center><table border=0><tr><td>"));
	pl=fill_tcp_data_p(buf,pl,PSTR("<input type=checkbox name=en"));
	if (server_data[num].enable==0xFF) pl=fill_tcp_data_p(buf,pl,PSTR(" checked"));
	pl=fill_tcp_data_p(buf,pl,PSTR(">Enable/Disable<br>Host name<br>"));


	pl=input_type_text_name_2(buf,pl,PSTR(""),PSTR("host"),(char *)server_data[num].server_name,PSTR(" (max len 25)"));
	my_itoa_long(server_data[num].port,tmpstr);
	pl=input_type_text_name_2(buf,pl,PSTR("<br>Port<br>"),PSTR("port"),tmpstr,PSTR(""));
	pl=input_type_text_name_2(buf,pl,PSTR("<br>Path<br>"),PSTR("path"),(char *)server_data[num].script_path,PSTR("<br>interval<br><SELECT name=sp>"));
		
	for (a=0; a<COUNT_AUTOUPDATE; a++) // Массив speed
	{
		pl=fill_tcp_data_p(buf,pl,PSTR("<option value="));
		//sprintf(tmpstr,"%d",a);
		itoa(a,tmpstr,10); 
		pl=fill_tcp_data(buf,pl,tmpstr);
		
		if (server_data[num].refresh_time==update_time[a].id) pl=fill_tcp_data_p(buf,pl,PSTR(" selected"));
		
		pl=fill_tcp_data_p(buf,pl,PSTR(">"));
		pl=fill_tcp_data(buf,pl,update_time[a].name);
		pl=fill_tcp_data_p(buf,pl,PSTR("</option>"));
	}

	pl=fill_tcp_data_p(buf,pl,PSTR("</SELECT><br><br><input type=submit name=\"ls"));
	itoa(num,tmpstr,10);  
	pl=fill_tcp_data(buf,pl,tmpstr);
	pl=fill_tcp_data_p(buf,pl,PSTR("\" value=\"Save&Test\"><input type=hidden name=sid value="));
	my_itoa_long(sessionid,tmpstr);
	pl=fill_tcp_data(buf,pl,tmpstr); // Вставляем сессию	
	pl=fill_tcp_data_p(buf,pl,PSTR("></form>"));
	
	return(pl);
}


uint16_t print_webpage_login(uint8_t *buf)
{
	uint16_t pl;
	pl=http200ok();
	pl=page_header(buf,pl);
	//my_itoa_long(Time_ms,tmpstr);
	//pl=fill_tcp_data(buf,pl,tmpstr);
	pl=fill_tcp_data_p(buf,pl,PSTR("<center><br>Password<form><input type=password name=pa><br><input type=submit value=Login name=in></form>"));
	//pl=fill_tcp_data_p(buf,pl,PSTR("<input type=submit value=Login name=in>"));
	//pl=fill_tcp_data_p(buf,pl,PSTR("</html>"));
	return(pl);
}


uint16_t print_webpage_parol_change(uint8_t *buf, char flag)
{
	uint16_t pl;
	pl=http200ok();
	pl=page_header(buf,pl);
	
	//pl=fill_tcp_data_p(buf,pl,PSTR("Change Password<br><br><b>"));
	if (flag==0)
	{		
		//pl=fill_tcp_data_p(buf,pl,PSTR("Enter new password:</b><form><input type=text name=new maxlength=14>"));
		//pl=fill_tcp_data_p(buf,pl,PSTR("<br>(minimum password length 5 characters)"));		
		pl=fill_tcp_data_p(buf,pl,PSTR("new password:<form><input type=text name=new maxlength=14><br><input type=submit value=Change name=cha><input type=hidden name=sid value=\""));
		//pl=fill_tcp_data_p(buf,pl,PSTR("<input type=hidden name=sid value=\""));
		my_itoa_long(sessionid,tmpstr);
		pl=fill_tcp_data(buf,pl,tmpstr); // Вставляем сессию
		pl=fill_tcp_data_p(buf,pl,PSTR("\"></form>"));
	}
	else
	{
		pl=fill_tcp_data_p(buf,pl,PSTR("Changed!"));
	}
	//pl=fill_tcp_data_p(buf,pl,PSTR("</html>"));
	return(pl);
}


uint16_t print_webpage_info(uint8_t *buf)
{
	uint16_t pl;
	pl=http200ok();
	pl=page_header(buf,pl);

	//pl=fill_tcp_data_p(buf,pl,PSTR("<center><br><br><br><br>"));
	//pl=fill_tcp_data_p(buf,pl,PSTR("<center><h1>&#9728; <u>WebMeteoBox3 mini</u> &#9730;</h1>"));
	//pl=fill_tcp_data_p(buf,pl,PSTR("<center><h1>&#9728; <u>WebMeteoBox3 mini</u> &#9730;</h1>"));
#ifdef METEOBOX_DEMO_MODE	
	pl=fill_tcp_data_p(buf,pl,PSTR("<h1><font color=#f36223>DEMO MODE</font></h1>"));
#endif
	pl=fill_tcp_data_p(buf,pl,PSTR("FW: " FIRMWARE_VERSION "</html>"));
	//pl=fill_tcp_data_p(buf,pl,PSTR("</html>"));
	return(pl);
}

uint16_t print_webpage_reboot(uint8_t *buf)
{
	uint16_t pl;
	pl=http200ok();
	//pl=page_header(buf,pl);

	//pl=fill_tcp_data_p(buf,pl,PSTR("<SCRIPT LANGUAGE=\"javascript\">function rx(){var q=confirm(\"Are you sure to reboot?\");if(q==false)return false;else return true;}</SCRIPT><center><br><br><a href=\"/reb?yes&sid="));
	pl=fill_tcp_data_p(buf,pl,PSTR("<center><br><br><a href=/reb?yes&sid="));
	//pl=fill_tcp_data_p(buf,pl,PSTR("<center><br><br><a href=\"/reb?yes&sid="));
	//pl=fill_tcp_data_p(buf,pl,PSTR("<a href=\"/reb?yes&sid="));
	my_itoa_long(sessionid,tmpstr);
	pl=fill_tcp_data(buf,pl,tmpstr); // Вставляем сессию
	pl=fill_tcp_data_p(buf,pl,PSTR(">Reboot</a>"));
	//pl=fill_tcp_data_p(buf,pl,PSTR("</b></html>"));
	return(pl);
}



uint16_t print_webpage_sensor_config(uint8_t *buf, unsigned char a, unsigned char flag)
{
	uint16_t pl;
	pl=http200ok();
	pl=page_header(buf,pl);
	pl=fill_tcp_data_p(buf,pl,PSTR("<body>"));	
	if (flag == 1)	pl=fill_tcp_data_p(buf,pl,PSTR("<SCRIPT language=\"JavaScript\">parent.frames['lf'].location.reload();</SCRIPT>"));	
	//pl=fill_tcp_data_p(buf,pl,PSTR("<center><br><br>Sensor configuration<br>"));
	pl=fill_tcp_data_p(buf,pl,PSTR("<form method=get>"));
    //pl=fill_tcp_data_p(buf,pl,PSTR("<center>"));
    pl=fill_tcp_data_p(buf,pl,PSTR("ID: <b>"));
	sprintf_P(tmpstr,PSTR("%02X%02X%02X%02X%02X%02X%02X%02X"),all_sensors[a].id[0],all_sensors[a].id[1],all_sensors[a].id[2],all_sensors[a].id[3],all_sensors[a].id[4],all_sensors[a].id[5],all_sensors[a].id[6],all_sensors[a].id[7]); // делаем строку с Sensor id
	pl=fill_tcp_data(buf,pl,tmpstr);
	pl=fill_tcp_data_p(buf,pl,PSTR("</b><br><input type=checkbox name=en"));
	if (all_sensors[a].enable==0xFF) pl=fill_tcp_data_p(buf,pl,PSTR(" checked"));
	//pl=fill_tcp_data_p(buf,pl,PSTR(">Enable/Disabe<br>"));		
	//pl=fill_tcp_data_p(buf,pl,PSTR(">Enable/Disabe<br>Sensor name:<br><input type=text name=na maxlength=14 value=\""));	
	//pl=fill_tcp_data(buf,pl,(char *)all_sensors[a].name);
	
	
	pl=input_type_text_name_2(buf,pl,PSTR(">Enable/Disabe<br>Sensor name:<br>"),PSTR("na"),(char *)all_sensors[a].name,PSTR(" (max len 14)"));
	sprintf_P(tmpstr,PSTR("%2.2f"),all_sensors[a].offset);
	pl=input_type_text_name_2(buf,pl,PSTR("<br>Offset<br>"),PSTR("of"),tmpstr,PSTR("<br>Current value<br>"));
	

	//pl=fill_tcp_data_p(buf,pl,PSTR("<br>Current value<br>"));
	get_sensor_value_type_str(a,0,tmpstr,tmpstr2);
	pl=fill_tcp_data(buf,pl,tmpstr);
	pl=fill_tcp_data(buf,pl,tmpstr2);
	pl=fill_tcp_data_p(buf,pl,PSTR("<br>Calculated<br>"));
	get_sensor_value_type_str(a,all_sensors[a].offset,tmpstr,tmpstr2);
	pl=fill_tcp_data(buf,pl,tmpstr);
	pl=fill_tcp_data(buf,pl,tmpstr2);	
	pl=fill_tcp_data_p(buf,pl,PSTR("<input type=hidden name=num value="));
	//sprintf_P(tmpstr,PSTR("%d"),a);
	itoa(a,tmpstr,10); 
	pl=fill_tcp_data(buf,pl,tmpstr);	
	pl=fill_tcp_data_p(buf,pl,PSTR("><input type=hidden name=sid value="));
	my_itoa_long(sessionid,tmpstr);
	pl=fill_tcp_data(buf,pl,tmpstr); // Вставляем сессию
	pl=fill_tcp_data_p(buf,pl,PSTR("><br><br><input type=submit value=Save name=sa>"));
	//pl=fill_tcp_data_p(buf,pl,PSTR("</html>"));
	return(pl);
}


uint16_t add_target_and_session(uint8_t *buf, uint16_t pos, const char * param, const char * name)
{
	uint16_t pl;
	my_itoa_long(sessionid,tmpstr);
	
	pl=fill_tcp_data_p(buf,pos,PSTR("<a href=/"));
	pl=fill_tcp_data_p(buf,pl,param);
	pl=fill_tcp_data_p(buf,pl,PSTR("sid="));
	pl=fill_tcp_data(buf,pl,tmpstr);
	pl=fill_tcp_data_p(buf,pl,PSTR(" target=rf>"));
	pl=fill_tcp_data_p(buf,pl,name);
	pl=fill_tcp_data_p(buf,pl,PSTR("</a><br>"));
	
	return (pl);
}



uint16_t print_webpage_leftmenu(uint8_t *buf)
{
	uint16_t pl;
	//unsigned char a,b;
	pl=http200ok();
	pl=page_header(buf,pl);
	
	my_itoa_long(sessionid,tmpstr);
	//pl=fill_tcp_data_p(buf,pl,PSTR("<body><center>"));

	pl=add_target_and_session(buf,pl,PSTR("ip?"),PSTR("TCP/IP"));
	pl=add_target_and_session(buf,pl,PSTR("pa?"),PSTR("Password"));
	pl=add_target_and_session(buf,pl,PSTR("log1?"),PSTR("Logger 1"));
	
#ifdef ADD_LOGGER_2	
	pl=add_target_and_session(buf,pl,PSTR("log2?"),PSTR("Logger 2"));
#endif	
	
#ifdef ADD_RELAY
	#ifdef WEB_RELAY_MINI_1
		pl=add_target_and_session(buf,pl,PSTR("rel?num=0&"),PSTR("Relay"));
	#else
		pl=add_target_and_session(buf,pl,PSTR("rel?num=0&"),PSTR("Relay 0"));
		pl=add_target_and_session(buf,pl,PSTR("rel?num=1&"),PSTR("Relay 1"));
		//pl=add_target_and_session(buf,pl,PSTR("rel?num=2&"),PSTR("Relay 2"));
		//pl=add_target_and_session(buf,pl,PSTR("rel?num=3&"),PSTR("Relay 3"));
		//pl=add_target_and_session(buf,pl,PSTR("rel?num=4&"),PSTR("Relay 4"));
		//pl=add_target_and_session(buf,pl,PSTR("rel?num=5&"),PSTR("Relay 6"));
	#endif		
#endif	
	
	pl=add_target_and_session(buf,pl,PSTR("reb?"),PSTR("Reboot"));	
	pl=add_target_and_session(buf,pl,PSTR("ex?"),PSTR("Exit"));
	
	return(pl);
}


uint16_t print_webpage_sensors_left_menu(uint8_t *buf)
{
	uint16_t pl,a;	
	pl=http200ok();
	pl=page_header(buf,pl);

	pl=fill_tcp_data_p(buf,pl,PSTR("<html><body><center>Sensors</center><br>"));
	for (a=0; a<MAX_SENSORS; a++)
	{
		if (all_sensors[a].sensor_type !=0)
		{
			pl=fill_tcp_data_p(buf,pl,PSTR("<a href=/sen?num="));
			//sprintf(tmpstr,"%d",a);
			itoa(a,tmpstr,10); 
			pl=fill_tcp_data(buf,pl,tmpstr);
			pl=fill_tcp_data_p(buf,pl,PSTR("&sid="));
			my_itoa_long(sessionid,tmpstr);
			pl=fill_tcp_data(buf,pl,tmpstr);
			pl=fill_tcp_data_p(buf,pl,PSTR(" target=rf>"));
			pl=fill_tcp_data(buf,pl,(char *)all_sensors[a].name);
			pl=fill_tcp_data_p(buf,pl,PSTR("</a><br>"));
		}
	}
	return(pl);
}



uint16_t print_webpage_index(uint8_t *buf)
{
	uint16_t pl,a,b,c;
	unsigned long diff,day,hr,min,sec;
	pl=http200ok();
	pl=page_header(buf,pl);

#ifdef WEB_RELAY_MINI
	#ifdef METEOBOX_DEMO_MODE
		pl=fill_tcp_data_p(buf,pl,PSTR("<center><h2><u>WebMeteoBox-Relay</u> &#9730;<font color=red>!!!DEMO MODE!!!</font></h2><table border=0><tr><td valign=top><pre><h2>"));
	#else
		pl=fill_tcp_data_p(buf,pl,PSTR("<title>WebMeteoBox-Relay</title><center><h2><u>WebMeteoBox-Relay</u> &#9730;</h2><table border=0><tr><td valign=top><pre><h2>"));
	#endif
#else
	#ifdef METEOBOX_DEMO_MODE
		//pl=fill_tcp_data_p(buf,pl,PSTR("<center><h2><u>WebMeteoBox-mini</u> &#9730;<font color=red>!!!DEMO MODE!!!</font></h2><table border=0><tr><td valign=top><pre><h2>"));
        pl=fill_tcp_data_p(buf,pl,PSTR("<center><h2><u>WebMeteoBox-mini</u> &#9730;</h2><table border=0><tr><td valign=top><pre><h2>"));		
	#else
		pl=fill_tcp_data_p(buf,pl,PSTR("<title>WebMeteoBox-mini</title><center><h2><u>WebMeteoBox-mini</u> &#9730;</h2><table border=0><tr><td valign=top><pre><h2>"));
	#endif
#endif

	for (a=0; a<MAX_SENSORS; a++)
	{
		if (all_sensors[a].enable==0xFF && all_sensors[a].sensor_type !=0)
		{
			if (all_sensors[a].error != 0) pl=fill_tcp_data_p(buf,pl,PSTR("<s>"));	
			pl=fill_tcp_data(buf,pl,(char *)all_sensors[a].name);
			c=strlen((char *)all_sensors[a].name);
			for (b=c; b<15; b++) pl=fill_tcp_data_p(buf,pl,PSTR(" "));
			get_sensor_value_type_str(a,all_sensors[a].offset,tmpstr,tmpstr2);
			pl=fill_tcp_data(buf,pl,tmpstr);
			pl=fill_tcp_data(buf,pl,tmpstr2);
			if (all_sensors[a].error != 0) pl=fill_tcp_data_p(buf,pl,PSTR("</s>"));		
			pl=fill_tcp_data_p(buf,pl,PSTR("\r\n"));
		}
	}
#ifdef METEOBOX_DEMO_MODE
	pl=fill_tcp_data_p(buf,pl,PSTR("</h2></pre></td></tr></table><br><a href=http://sensorcloud.tk/aWozwKTpruFKVJb.trend>Смотреть график нагрузки</a><br><br><a href=/lg>Login</a><br>(pass:<b>admin</b>)<br>"));
#else
	pl=fill_tcp_data_p(buf,pl,PSTR("</h2></pre></td></tr></table><br><br><a href=/lg>Login</a><br>"));
#endif

#ifdef DEBUG_METEOBOX
	pl=fill_tcp_data_p(buf,pl,PSTR("DBG_MS: "));
	my_itoa_long(Time_ms*10,tmpstr);
	pl=fill_tcp_data(buf,pl,tmpstr);
#endif
	
	//my_itoa_long(tick_counter,tmpstr);
	//pl=fill_tcp_data(buf,pl,tmpstr);
	
	//pl=fill_tcp_data_p(buf,pl,PSTR("<p>Designed for <a href=\"http://narodmon.ru\" target=\"_blank\">Public monitoring</a> project</p><br>Uptime: "));
	

#ifdef TIMER_RELAY
	pl=fill_tcp_data_p(buf,pl,PSTR("<p>Last relay on: "));

	diff=floor(last_relay_time/100);
	day=floor(diff/60/60/24);
	diff-=day*60*60*24;
	hr =floor(diff/60/60);
	diff-=hr*60*60;
	min=floor(diff/60);
	diff -= min*60;
	sec=diff;

	my_itoa_long(day,tmpstr);
	pl=fill_tcp_data(buf,pl,tmpstr);
	pl=fill_tcp_data_p(buf,pl,PSTR(" days "));
	my_itoa_long(hr,tmpstr);
	pl=fill_tcp_data(buf,pl,tmpstr);
	pl=fill_tcp_data_p(buf,pl,PSTR(":"));
	my_itoa_long(min,tmpstr);
	pl=fill_tcp_data(buf,pl,tmpstr);
	pl=fill_tcp_data_p(buf,pl,PSTR(":"));
	my_itoa_long(sec,tmpstr);
	pl=fill_tcp_data(buf,pl,tmpstr);
	pl=fill_tcp_data_p(buf,pl,PSTR("<p>"));	
#endif	
	
	
	
	pl=fill_tcp_data_p(buf,pl,PSTR("Uptime: "));
	
	diff=floor(Time_ms/100);
	day=floor(diff/60/60/24);
	diff-=day*60*60*24;
	hr =floor(diff/60/60);
	diff-=hr*60*60;
	min=floor(diff/60);
	diff -= min*60;
	sec=diff;
	
	my_itoa_long(day,tmpstr);
	pl=fill_tcp_data(buf,pl,tmpstr);
	pl=fill_tcp_data_p(buf,pl,PSTR(" days "));
	my_itoa_long(hr,tmpstr);
	pl=fill_tcp_data(buf,pl,tmpstr);
	pl=fill_tcp_data_p(buf,pl,PSTR(":"));
	my_itoa_long(min,tmpstr);
	pl=fill_tcp_data(buf,pl,tmpstr);
	pl=fill_tcp_data_p(buf,pl,PSTR(":"));
	my_itoa_long(sec,tmpstr);
	pl=fill_tcp_data(buf,pl,tmpstr);
	

	//pl=fill_tcp_data_p(buf,pl,PSTR("<p>Designed for <a href=\"http://narodmon.ru\" target=\"_blank\">Public monitoring</a> project<br>Powered By <a href=\"http://meteobox.tk\">MeteoBox.tk</a> ver " FIRMWARE_VERSION "</font></p>"));
	
	pl=fill_tcp_data_p(buf,pl,PSTR("</body></html>"));
	return(pl);
}




void process_http_server()
{
	uint16_t len;
	char sendflag=0;
	unsigned char flag,a,use_dns=0;
	unsigned char current_logger=0;
	unsigned char post_flag=0;

	//len=enc28j60PacketReceive(BUFFER_SIZE, buf);
	len=enc28j60PacketReceive(256, buf);
	buf[BUFFER_SIZE]='\0'; // 04.06.2014
//----------------DHCP initialization -------------- 04.06.2014 -----------------	
	if (NetworkParam.dhcp==0xFF)
	{		
		// DHCP renew IP:
		len=packetloop_dhcp_renewhandler(buf,len);
	}
//----------------DHCP initialization -------------- 04.06.2014 -----------------	
	enc28j60EnableBroadcast(); // 11.06.2014 for udp configurator
	buf_pos=packetloop_arp_icmp_tcp(buf,len);
    // dat_p will be unequal to zero if there is a valid  http get
/*---------------------------------------------------------------------------------------------------------*/    
if (buf_pos==0)
{
				// we are idle here trigger arp and dns stuff here
				if (gw_arp_state==0){
					// find the mac address of the gateway (e.g your dsl router).

					get_mac_with_arp(NetworkParam.gw,TRANS_NUM_GWMAC,&arpresolver_result_callback);
					gw_arp_state=1;
				}
				if (get_mac_with_arp_wait()==0 && gw_arp_state==1){
					// done we have the mac address of the GW
					gw_arp_state=2;
				}
}
	
	if ((Wt0==0) || (Wt1==0)) {
		
		if (Wt0==0) { current_logger=0; if (server_data[current_logger].enable != 0xFF) { TimeWt0=WebrefreshTime0; Wt0=1; } else post_flag=1; }
#ifdef ADD_LOGGER_2			
		if (Wt1==0) { current_logger=1; if (server_data[current_logger].enable != 0xFF) { TimeWt1=WebrefreshTime1; Wt1=1; } else post_flag=1; }
#endif	
		if (post_flag==1) {
		
			if(len==0){
				for (a=0; a<strlen((char *)server_data[current_logger].server_name); a++) // ищем точки в имени сервера, если их 3 значит там ip адрес
					if (server_data[current_logger].server_name[a] == '.') use_dns++;
					
				if (use_dns<3) // используем DNS
				{
					if (dns_state==0 && gw_arp_state==2){
						if (!enc28j60linkup()) return; // only for dnslkup_request we have to check if the link is up.
						sec=0;
						dns_state=1;			
						dnslkup_request(buf,(char *)server_data[current_logger].server_name,gwmac);
						return;
					}
					if (dns_state==1 && dnslkup_haveanswer()){
						dns_state=2;
						dnslkup_get_ip(otherside_www_ip);
						//printf("host ip: %d.%d.%d.%d\r\n",otherside_www_ip[0],otherside_www_ip[1],otherside_www_ip[2],otherside_www_ip[3]);
					}
					if (dns_state!=2){
						// retry every minute if dns-lookup failed:
						if (sec > 60){
							dns_state=0;
#ifdef REBOOT_AFTER								
							reboot_counter++;
#endif							
							//printf("if (sec > 60)");
						}
						// don't try to use web client before we have a result of dns-lookup
						return;
					}
				} else  { // Вместо DNS используем свой сервер
					
					
					//data_get((char *)server_data[current_logger].server_name,'.',10);
					//memcpy(otherside_www_ip,xbuf,4);
					parse_ip(otherside_www_ip,(char *)server_data[current_logger].server_name);
					//leddebugsignal(10);
					/*
					otherside_www_ip[0]=server_data[current_logger].server_ip[0];
					otherside_www_ip[1]=server_data[current_logger].server_ip[1];
					otherside_www_ip[2]=server_data[current_logger].server_ip[2];
					otherside_www_ip[3]=server_data[current_logger].server_ip[3];
					*/
					
				/*	otherside_www_ip[0]=192;
					otherside_www_ip[1]=168;
					otherside_www_ip[2]=1;
					otherside_www_ip[3]=33;*/
					
					
				}
				//----------
				if (start_web_client==1){
					start_web_client=2;
					
					strcpy_P(xbuf,PSTR("ID="));
					sprintf_P(tmpstr,PSTR("%02X%02X%02X%02X%02X%02X"),NetworkParam.mac[0],NetworkParam.mac[1],NetworkParam.mac[2],NetworkParam.mac[3],NetworkParam.mac[4],NetworkParam.mac[5]); // делаем строку с MAC
					strcat(xbuf,tmpstr);
					
					for (a=0;a<MAX_SENSORS;a++) 
					{								
						if (all_sensors[a].enable != 0xFF || all_sensors[a].sensor_type == 0 || all_sensors[a].error == 1) continue; // Сенсор выключен, не отправляем данные на сайт fix 28.01.2015
						sprintf_P(tmpstr,PSTR("&%02X%02X%02X%02X%02X%02X%02X%02X"),all_sensors[a].id[0],all_sensors[a].id[1],all_sensors[a].id[2],all_sensors[a].id[3],all_sensors[a].id[4],all_sensors[a].id[5],all_sensors[a].id[6],all_sensors[a].id[7]); // делаем строку с Sensor id
						strcat(xbuf,tmpstr);
						get_sensor_value_type_str(a,all_sensors[a].offset,tmpstr,tmpstr2); 
						sprintf_P(tmpstr2,PSTR("=%s"),tmpstr);
						strcat(xbuf,tmpstr2);														
					}	

					client_http_post((char *)server_data[current_logger].script_path,"",(char *)server_data[current_logger].server_name,NULL,xbuf,&browserresult_callback,otherside_www_ip,gwmac,(uint16_t)server_data[current_logger].port);
					sec=0;	
#ifdef ADD_LOGGER_2								
					if (current_logger==0) { TimeWt0=WebrefreshTime0; Wt0=1; } else { TimeWt1=WebrefreshTime1; Wt1=1; }
#else						
					if (current_logger==0) { TimeWt0=WebrefreshTime0; Wt0=1; } 							
#endif			 	    
					dns_state=0;
							
				}
				//if (start_web_client==3 || sec>=5  ){ start_web_client=1; }
			    if (start_web_client==3 || sec>=8  ) {
					 start_web_client=1;
#ifdef REBOOT_AFTER						 
					 if (sec>=8) reboot_counter++;
#endif					 
					 /* leddebugsignal(10);*/
			    }
				return;
			} // if(len==0){
		} // if (post_flag==1) { 
	} // if ((Wt0==0) && (Wt1==0)) { 
	
/*---------------------------------------------------------------------------------------------------------*/    	
	if(buf_pos==0) { udp_client_check_for_dns_answer(buf,len); 
/*------------------------------ UDP CONFIGURATION --------------------------------------------------------*/
#ifdef ENABLE_UDP_SEARCH		   		
		if (buf[IP_PROTO_P]==IP_PROTO_UDP_V&&buf[UDP_DST_PORT_H_P]==(char)(CONFIG_CLIENT_PORT>>8)&&buf[UDP_DST_PORT_L_P]==(char)(CONFIG_CLIENT_PORT)){

			if(strncmp_P((char *)&buf[UDP_DATA_P],PSTR("SRCH"),4) == 0)
			{
				memcpy_P((unsigned char*)xbuf, PSTR("MINI"), 4);
				memcpy((unsigned char*)xbuf+4 ,NetworkParam.gw, 4);
				memcpy((unsigned char*)xbuf+8 ,NetworkParam.subnet, 4);
				memcpy((unsigned char*)xbuf+12,NetworkParam.ip, 4);
				memcpy((unsigned char*)xbuf+16,NetworkParam.mac, 6);
				memcpy((unsigned char*)xbuf+22,NetworkParam.dns, 4);
				make_udp_reply_from_request(buf,xbuf,26,CONFIG_SERVER_PORT);
			}			
			else if(strncmp_P((char *)&buf[UDP_DATA_P],PSTR("SETQ"),4) == 0) // Изменение конфигурации
			{
				if (memcmp(&buf[UDP_DATA_P+16], NetworkParam.mac, 6) == 0)					
					
					{
						memcpy_P((unsigned char*)xbuf, PSTR("SETR"), 4);
						make_udp_reply_from_request(buf,xbuf,4,CONFIG_SERVER_PORT);
						
						memcpy(NetworkParam.gw,     &buf[UDP_DATA_P+4],  4);
						memcpy(NetworkParam.subnet, &buf[UDP_DATA_P+8],  4);
						memcpy(NetworkParam.ip,     &buf[UDP_DATA_P+12], 4);						
						memcpy(NetworkParam.dns,    &buf[UDP_DATA_P+22], 4);
						eeprom_write_block(&NetworkParam, (unsigned char*)EEPOP, sizeof(NetworkParam));
						GoApp();
					}
			}
			else if(strncmp_P((char *)&buf[UDP_DATA_P], PSTR("R**T"),4) == 0) // сброс к настройкам по умолчанию
			{
				if (memcmp(&buf[UDP_DATA_P+16], NetworkParam.mac, 6) == 0)
				{
					default_network(1);
					GoApp();
				}
			}
		}
#endif		
/*------------------------------ UDP CONFIGURATION --------------------------------------------------------*/   				
		return;
	}  // no http request
	
	if (strncmp_P((char *)&buf[buf_pos],PSTR("GET /"),5) != 0) return;
	
	bufrecv=&buf[buf_pos];
	strcpy((char *)bufrecv,findstr((char *)&buf[buf_pos],"GET /"," HTTP")); // Вытаскиваем GET запрос
	
	if (strlen((char *)bufrecv) == 0 ){
		buf_pos=print_webpage_index(buf);

#ifdef ENABLE_RF_MODULE 
	exe_wireless(); // При обновлении Index отправляем данные
#endif

		sendflag=1;
		goto SENDTCP;
	}
	

	if (strncmp_P((char *)bufrecv,PSTR("lg"),2) == 0 ){
		strcpy((char *)xbuf,findstr((char *)bufrecv,"pa=","&"));
		if (strlen(xbuf) > 3)
		{
			if (strstr(xbuf,MPasswd))
			{
				sessionid=Time_ms*121121; //Создали сессию 10.06.2015
				//sessionid=INT64_MAX;
				buf_pos=print_webpage_mainframe(buf);
				//printf(">print_webpage_mainframe\r\n");
				sendflag=1;
				goto SENDTCP;
			}
		}
		buf_pos=print_webpage_login(buf);
		//printf(">print_webpage_login\r\n");
		sendflag=1;
		goto SENDTCP;
	}
	
	

#ifdef ENABLE_JSON	
	if (strncmp_P((char *)bufrecv,PSTR("json"),4) == 0  || (strncmp_P((char *)bufrecv,PSTR("xson"),4) == 0)){
		
// Новая идея. возвращаем json и сбрасываем таймер обновления датчиков! 
// но в таком режиме нужно самому контролироват ьинтервал запросов т.к. некоторые датчики измеряют около 1 сек.
		if (strncmp_P((char *)bufrecv,PSTR("xson"),4) == 0) {  // 17.04.2015 
			//TimeTs0=0; // сбрасываем таймер!
			Tr0=0; // сбрасываем флаг.
		}
//-------------------------------------------------------------------------------------------------------------		
		buf_pos=fill_tcp_data_p(buf,0,PSTR("HTTP/1.0 200 OK\r\nContent-type: application/json\r\n\r\n"));
		buf_pos=fill_tcp_data_p(buf,buf_pos,PSTR("{\"Sensors\":[\r\n"));

//
// Можно сэкономить 26 байт! 
//		
		sendflag=0;
		for (a=0; a<MAX_SENSORS; a++) if (all_sensors[a].enable==0xFF && all_sensors[a].sensor_type !=0) sendflag++;
		for (a=0; a<MAX_SENSORS; a++)
		{
			if (all_sensors[a].enable==0xFF && all_sensors[a].sensor_type !=0)
			{
				buf_pos=fill_tcp_data_p(buf,buf_pos,PSTR("{\r\n\"I\": ")); // Type
				sprintf_P(tmpstr,PSTR("\"%02X%02X%02X%02X%02X%02X%02X%02X"),all_sensors[a].id[0],all_sensors[a].id[1],all_sensors[a].id[2],all_sensors[a].id[3],all_sensors[a].id[4],all_sensors[a].id[5],all_sensors[a].id[6],all_sensors[a].id[7]); // делаем строку с Sensor id
				buf_pos=fill_tcp_data(buf,buf_pos,tmpstr);
				buf_pos=fill_tcp_data_p(buf,buf_pos,PSTR("\",\r\n\"T\":\"")); // Type
				//sprintf(tmpstr,"%d",all_sensors[a].type);
				itoa(all_sensors[a].type,tmpstr,10); 
				buf_pos=fill_tcp_data(buf,buf_pos,tmpstr);
				buf_pos=fill_tcp_data_p(buf,buf_pos,PSTR("\",\r\n\"N\":\"")); // Name
				buf_pos=fill_tcp_data(buf,buf_pos,(char *)all_sensors[a].name);
				buf_pos=fill_tcp_data_p(buf,buf_pos,PSTR("\",\r\n\"V\":\"")); // Value
				get_sensor_value_type_str(a,all_sensors[a].offset,tmpstr,tmpstr2);
				buf_pos=fill_tcp_data(buf,buf_pos,tmpstr);
				buf_pos=fill_tcp_data_p(buf,buf_pos,PSTR("\",\r\n\"E\":\"")); // Error
				//sprintf(tmpstr,"%d",all_sensors[a].error);
				itoa(all_sensors[a].error,tmpstr,10); 
				buf_pos=fill_tcp_data(buf,buf_pos,tmpstr);				
				buf_pos=fill_tcp_data_p(buf,buf_pos,PSTR("\"\r\n}"));
				if (a+1<sendflag) buf_pos=fill_tcp_data_p(buf,buf_pos,PSTR(",\r\n"));	
			}
		}
#ifdef ADD_RELAY
		buf_pos=fill_tcp_data_p(buf,buf_pos,PSTR("],\"Relays\":["));
		for (a=0; a<MAX_RELAY; a++)
		{
			if (a<4) sendflag=PC_READ(all_relay[a].pio);
				else sendflag=PB_READ(all_relay[a].pio);
			itoa(sendflag,tmpstr,10); 
			buf_pos=fill_tcp_data(buf,buf_pos,tmpstr);	
			if (a<MAX_RELAY-1)
				buf_pos=fill_tcp_data_p(buf,buf_pos,PSTR(","));
		}
#endif			
		buf_pos=fill_tcp_data_p(buf,buf_pos,PSTR("]}"));
	
		sendflag=1;
		goto SENDTCP;
	}
#endif 		
	
	if (strncmp_P((char *)bufrecv,PSTR("favicon.ico"),11) == 0 ){
		//printf(">favicon.ico\r\n");
		buf_pos=http404();
		sendflag=1;
		goto SENDTCP;
	}
	
	strcpy((char *)tmpstr,findstr((char *)bufrecv,"sid=","&"));
	my_itoa_long(sessionid,tmpstr2);
	//printf("session GET: %s   Saved: %s\r\n",tmpstr,tmpstr2);

#ifdef DEBUG_METEOBOX			
	if (1) //При отладке отключаем авторизацию
#else	
	if ((strstr(tmpstr,tmpstr2)) && (sessionid != 0)) // Сравниваем сессию пользователя и сохраненную, сессия должны быть проинициализированна т.е. не равна 0
#endif 	
	{
		if (strncmp_P((char *)bufrecv,PSTR("menu"),4) == 0 ){
			buf_pos=print_webpage_leftmenu(buf);
			//printf(">print_webpage_leftmenu\r\n");
			sendflag=1;
			goto SENDTCP;
		}
		
		if (strncmp_P((char *)bufrecv,PSTR("dev"),3) == 0 ){
			buf_pos=print_webpage_sensors_left_menu(buf);
			sendflag=1;
			goto SENDTCP;
		}		
	
		if (strncmp_P((char *)bufrecv,PSTR("info"),4) == 0 ){
#ifndef TIMER_RELAY					
			buf_pos=print_webpage_info(buf);
#endif			
			//buf_pos=print_relay_settings(buf,0);
			//printf(">print_webpage_leftmenu\r\n");
			sendflag=1;
			goto SENDTCP;
		}
		
		if (strncmp_P((char *)bufrecv,PSTR("reb"),3) == 0 ){
			strcpy((char *)tmpstr,findstr((char *)bufrecv,"reb?","&"));
			if (strncmp("yes",(char *)tmpstr,3) == 0 )
			{
				buf_pos=http200ok();
#ifdef METEOBOX_DEMO_MODE
				buf_pos=fill_tcp_data_p(buf,buf_pos,PSTR("<HTML><body><h1>Reboot disabled in demo mode!</h1></html>"));
				www_server_reply(buf,buf_pos);
				return;
#else				
				buf_pos=fill_tcp_data_p(buf,buf_pos,PSTR("<HTML><HEAD><SCRIPT language=\"JavaScript\">top.location=\"/\";</SCRIPT>"));
				www_server_reply(buf,buf_pos);
				GoApp();
#endif									
			}
			buf_pos=print_webpage_reboot(buf);
			sendflag=1;
			goto SENDTCP;
		}		
		
		if (strncmp_P((char *)bufrecv,PSTR("ex"),2) == 0 ){ // Обработчик ссылки Exit
			buf_pos=http200ok();
			buf_pos=fill_tcp_data_p(buf,buf_pos,PSTR("<HTML><HEAD><SCRIPT language=\"JavaScript\">top.location=\"/\";</SCRIPT>"));
			sessionid=0; // Clear session
			sendflag=1;
			goto SENDTCP;
		}	
		
		if (strncmp_P((char *)bufrecv, PSTR("sen"), 3) == 0 ){
			strcpy((char *)tmpstr,findstr((char *)bufrecv,"num=","&"));
			a=ATOI(tmpstr,10);
			strcpy((char *)tmpstr,findstr((char *)bufrecv,"sa=","&"));
			flag=0;
			if (strncmp_P((char *)tmpstr,PSTR("Save"),4) == 0 ){ // Save Logger configuration
				strcpy((char *)tmpstr,findstr((char *)bufrecv,"en=","&"));
				if (strncmp("on",(char *)tmpstr,2) == 0) all_sensors[a].enable=0xFF; else all_sensors[a].enable=0x00;// Sensor enable ?				
				strcpy(xbuf,findstr((char *)bufrecv,"na=","&"));
				urldecode(xbuf);
#ifdef METEOBOX_DEMO_MODE				
				if (strlen(xbuf)>1) strcpy((char *)all_sensors[a].name,xbuf);
#else				
				strcpy((char *)all_sensors[a].name,xbuf);
#endif				
				strcpy((char *)tmpstr,findstr((char *)bufrecv,"of=","&"));
				all_sensors[a].offset=atof(tmpstr);	
				//all_sensors[a].value=0.0f;
				eeprom_write_block(&all_sensors[a].value, (unsigned char*)(eeprom_sensors_config+(a*sizeof(all_sensors[0]))), sizeof(all_sensors[0]));			
				flag=1; 
			}			
			buf_pos=print_webpage_sensor_config(buf,a,flag);
			sendflag=1;
			goto SENDTCP;
		}	
		
		if (strncmp_P((char *)bufrecv, PSTR("pa"), 2) == 0 ){
			strcpy((char *)tmpstr,findstr((char *)bufrecv,"new=","&"));
			flag=0;
#ifndef METEOBOX_DEMO_MODE
			if (strlen(tmpstr)>0)
			{
				eeprom_write_block((char*)tmpstr,(unsigned char*)eeprom_password, 15);
				strcpy(MPasswd,tmpstr);
				flag=1;
			}
#endif			
			buf_pos=print_webpage_parol_change(buf,flag);
			sendflag=1;
			goto SENDTCP;			
		}
		
	
#ifdef ADD_RELAY	
		if (strncmp_P((char *)bufrecv, PSTR("rel"), 3) == 0 ){
			strcpy((char *)tmpstr,findstr((char *)bufrecv,"num=","&"));
			a=ATOI(tmpstr,10);

#ifdef TIMER_RELAY			
			humidity_secs=100; /* включаем принудительное измерение влажности */
#endif			
			
			strcpy((char *)tmpstr,findstr((char *)bufrecv,"snd=","&"));
			if (strncmp_P((char *)tmpstr, PSTR("On"), 2) == 0 ){ // Relay on button press
#ifdef TIMER_RELAY
				relay_timier=0;
				last_relay_time=0;
#endif				
				if (a<4) PC_WRITE(all_relay[a].pio,RELAY_ON);
				else  PB_WRITE(all_relay[a].pio,RELAY_ON);
			}
			
			if (strncmp_P((char *)tmpstr, PSTR("Off"), 3) == 0 ){ // Relay off button press
				if (a<4) PC_WRITE(all_relay[a].pio,RELAY_OFF);
				else  PB_WRITE(all_relay[a].pio,RELAY_OFF);
			}
			
#ifndef METEOBOX_DEMO_MODE
			if (strncmp_P((char *)tmpstr, PSTR("Save"), 4) == 0 ) {	// Relay Save button press
					strcpy((char *)tmpstr,findstr((char *)bufrecv,"min=","&"));
					all_relay[a].min=atof(tmpstr);
					strcpy((char *)tmpstr,findstr((char *)bufrecv,"max=","&"));
					all_relay[a].max=atof(tmpstr);
					strcpy((char *)tmpstr,findstr((char *)bufrecv,"sen=","&"));
					flag=ATOI(tmpstr,10);
					if (flag==100) all_relay[a].flag=0; else all_relay[a].flag=0xFF;
					memcpy(all_relay[a].id,all_sensors[flag].id,8);
					eeprom_update_block(&all_relay[a], (unsigned char*)eeprom_relay_config+(a*sizeof(all_relay[0])), sizeof(all_relay[0]));
			}
#endif
			buf_pos=print_relay_settings(buf,a);
			sendflag=1;
			goto SENDTCP;
		}
#endif		

			
		
		if ((strncmp_P((char *)bufrecv,PSTR("log1"),4) == 0) || (strncmp_P((char *)bufrecv,PSTR("log2"),4) == 0)){	
			if (strncmp_P((char *)bufrecv,PSTR("log1"),4) == 0) flag=0; else flag=1;	

			strcpy((char *)xbuf,findstr((char *)bufrecv,"Save%26","&"));	
#ifndef METEOBOX_DEMO_MODE
			if (strncmp_P((char *)xbuf,PSTR("Test"),4) == 0 ){ // Save Logger configuration				
				strcpy((char *)xbuf,findstr((char *)bufrecv,"en=","&"));
				if (strncmp_P((char *)xbuf,PSTR("on"),2) == 0) server_data[flag].enable=0xFF; else server_data[flag].enable=0x00;// logger 1 enable ?
				
				/*
				strcpy((char *)tmpstr,findstr((char *)bufrecv,"i=","&"));
				data_get(tmpstr,'.',10);
				memcpy(server_data[flag].server_ip,xbuf,4);
				*/
				
				strcpy((char *)server_data[flag].server_name,findstr((char *)bufrecv,"host=","&"));
				
				memset(server_data[flag].script_path,0x00,sizeof(server_data[0].script_path));
				strcpy((char *)xbuf,findstr((char *)bufrecv,"path=","&"));				
				urldecode(xbuf);	
				strcpy((char *)server_data[flag].script_path,(char *)xbuf);		
						
				strcpy((char *)xbuf,findstr((char *)bufrecv,"port=","&"));	
				server_data[flag].port=ATOI(xbuf,10);
				strcpy((char *)xbuf,findstr((char *)bufrecv,"sp=","&"));
				server_data[flag].refresh_time=update_time[ATOI(xbuf,10)].id;	

				if (flag==0) {
					WebrefreshTime0=(unsigned long int)server_data[flag].refresh_time*100;
					eeprom_update_block(&server_data[flag], (unsigned char*)eeprom_server_config, sizeof(server_data[0]));
					Wt0 = 0;
				}
#ifdef ADD_LOGGER_2					
				else {
					WebrefreshTime1=(unsigned long int)server_data[flag].refresh_time*100;
					eeprom_update_block(&server_data[flag], (unsigned char*)eeprom_server_config+sizeof(server_data[0]), sizeof(server_data[0]));
					Wt1 = 0;
				}
#endif	// ADD_LOGGER_2			
			}	
#endif	// METEOBOX_DEMO_MODE		
			buf_pos=print_webpage_logger_settings(buf, flag); // logger 
			sendflag=1;
			//printf(">print_webpage_logger_settings\r\n");
			goto SENDTCP;
		}		

		if ((strncmp_P((char *)bufrecv,PSTR("ip"),2) == 0) || (strncmp_P((char *)bufrecv,PSTR("pi"),2) == 0)){
			//if (strncmp_P((char *)bufrecv,PSTR("ip"),2) == 0) flag=0; else flag=1;
			
			strcpy((char *)tmpstr,findstr((char *)bufrecv,"act=","+"));

#ifdef ENABLE_TCP_SETINGS			
#ifndef METEOBOX_DEMO_MODE
			if (strncmp_P((char *)tmpstr,PSTR("Save"),4) == 0 ){ // Save EEPROM & REBOOT
				
				if (strncmp_P((char *)bufrecv,PSTR("ip"),2) != 0) { // Секретная страницы смены MAC адреса! /pi  02.12.2014
					strcpy((char *)tmpstr,findstr((char *)bufrecv,"m=","&"));				
					urldecode(tmpstr);
					data_get((char *)tmpstr,':',16);
					//NetworkParam.mac[0]=xbuf[0]; NetworkParam.mac[1]=xbuf[1]; NetworkParam.mac[2]=xbuf[2]; NetworkParam.mac[3]=xbuf[3]; NetworkParam.mac[4]=xbuf[4]; NetworkParam.mac[5]=xbuf[5];
					memcpy(&NetworkParam.mac,&xbuf,6);
				}
				strcpy((char *)tmpstr,findstr((char *)bufrecv,"s=","&"));
				//printf("s=%s\r\n",tmpstr);
				data_get((char *)tmpstr,'.',10);
				//NetworkParam.subnet[0]=xbuf[0]; NetworkParam.subnet[1]=xbuf[1]; NetworkParam.subnet[2]=xbuf[2]; NetworkParam.subnet[3]=xbuf[3];
				memcpy(&NetworkParam.subnet,&xbuf,4);
				strcpy((char *)tmpstr,findstr((char *)bufrecv,"g=","&"));
				//printf("g=%s\r\n",tmpstr);	
				data_get((char *)tmpstr,'.',10);
				//NetworkParam.gw[0]=xbuf[0]; NetworkParam.gw[1]=xbuf[1]; NetworkParam.gw[2]=xbuf[2]; NetworkParam.gw[3]=xbuf[3];							
				memcpy(&NetworkParam.gw,&xbuf,4);
				strcpy((char *)tmpstr,findstr((char *)bufrecv,"d=","&"));
				//printf("d=%s\r\n",tmpstr);	
				data_get((char *)tmpstr,'.',10);
				//NetworkParam.dns[0]=xbuf[0]; NetworkParam.dns[1]=xbuf[1]; NetworkParam.dns[2]=xbuf[2]; NetworkParam.dns[3]=xbuf[3];
				memcpy(&NetworkParam.dns,&xbuf,4);
				
				strcpy((char *)tmpstr,findstr((char *)bufrecv,"dh=","&"));
				if (strncmp("on",(char *)tmpstr,2) == 0) NetworkParam.dhcp=0xFF; else NetworkParam.dhcp=0x00;// DHCP enable ? add.03.06.2014
				
				strcpy((char *)tmpstr,findstr((char *)bufrecv,"i=","&"));
				data_get((char *)tmpstr,'.',10);
				memcpy(&NetworkParam.ip,&xbuf,4);
				//NetworkParam.ip[0]=xbuf[0]; NetworkParam.ip[1]=xbuf[1]; NetworkParam.ip[2]=xbuf[2]; NetworkParam.ip[3]=xbuf[3];
				
				
				eeprom_update_block(&NetworkParam, (unsigned char*)EEPOP, sizeof(NetworkParam));	
				
				buf_pos=http200ok();
				buf_pos=fill_tcp_data_p(buf,buf_pos,PSTR("<HEAD><SCRIPT language=\"JavaScript\">top.location=\"http://"));				
				buf_pos=fill_tcp_data(buf,buf_pos,tmpstr);
				buf_pos=fill_tcp_data_p(buf,buf_pos,PSTR("\";</SCRIPT>"));
				www_server_reply(buf,buf_pos);
				//printf("!!! len %d\r\n",buf_pos);	
				GoApp();						
			}
#endif	// #ifdef METEOBOX_DEMO_MODE		
#endif	// #ifdef ENABLE_TCP_SETINGS
			
			buf_pos=print_webpage_tcpip_settings(buf);
			sendflag=1;
			//printf(">print_webpage_tcpip_settings\r\n");
			goto SENDTCP;
		}
	}
	else
	{
		//printf(">session wrong!!!\r\n");
		buf_pos=http404();
		sendflag=1;
		goto SENDTCP;
	}
	
	
SENDTCP:
	if (sendflag==1) { 
		www_server_reply(buf,buf_pos); // send web page data
#ifdef REBOOT_AFTER		
		reboot_counter=0;
#endif		
	}
	//printf("www  len %d\r\n",buf_pos);	
}



