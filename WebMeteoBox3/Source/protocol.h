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


#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#define FIRMWARE_VERSION "1.1.9.3 ("__DATE__")" 

//#define METEOBOX_DEMO_MODE   /* Включаем если хотим сделать DEMO прошивку. с защитой от зименения ip и прочих параметров */

#define BUFFER_SIZE 700 //делаем 750 для 1-5реле!!! обычно работает с 700 байтами //1430 // Макс что смог отправить! 

#define REBOOT_AFTER      1 /* перезагрузка после xxx неудачных попыток отправки 18.01.2016 */
//-----------------------------------------------------------------------------------
#define ENABLE_ETHERNET      /* Включаем если есть ETHERNET модуль ENC28J60-mini */ 
#define ENABLE_TCP_SETINGS   /* включает настройку TCP из веб интерфейса  16.09.2014 */
#define ENABLE_UDP_SEARCH    /* Включаем если нужен UDP поисковик 03.02.2017 */ 
#define ADD_LOGGER_2         /* включает использование второго логгера  12.08.2014 */
//#define ADD_RELAY          /* включает использование реле  15.08.2014 */


/*------------------------ для mini-relay-1  06.05.2016 -----------------------------*/
//#define WEB_RELAY_MINI_1  /* прошивка с поддержкой 1 реле */
#define MAX_RELAY   	  2 // 5 /* количество реле на плате */
//#define TIMER_RELAY     /* Реле с авто отключением 28.04.2016 */
//#define HUMIDITY_SENSOR
/*-----------------------------------------------------------------------------------*/

//------------------------------------------------------------------------------------------------------------------------
//#define SCT_013_030        /* Включаем если есть токовые клещи SCT-013-030 */ 
#define SCT_COEFF 30       /* Коэффециент токовых клкщей.  */
//#define PAY_SENSOR
#ifdef SCT_013_030 
	#define VIRTUAL_SENSOR     /* Включаем если есть токовые клещи SCT-013-030 и нужно измерять квт. в час. 25.11.2014*/ 
#endif
//------------------------------------------------------------------------------------------------------------------------

//#define ENABLE_BMP180      /* Включаем если есть датчик давления BMP180 */ 
//#define ENABLE_JSON        /* Включаем если нужен протокол JSON */ 
//#define ENABLE_RF_MODULE     /* Включаем если есть протокол VirtualWire для RF модуля 433 Мгц */ 
#define ENABLE_LCD_74HC595
//#define ENABLE_DOZIMETR             /* Дозиметр */
#define RELAY_ON  0
#define RELAY_OFF 1





//#define WEB_RELAY_MINI // Включаем если нужно собрать WebRelayMini
#ifdef WEB_RELAY_MINI
	#undef ENABLE_BMP180
	#undef SCT_013_030
	#undef VIRTUAL_SENSOR
	#undef PAY_SENSOR
	#undef ENABLE_RF_MODULE
	#undef ENABLE_LCD_74HC595
	#undef ENABLE_DOZIMETR
	#undef FIRMWARE_VERSION
	#undef BUFFER_SIZE
	#undef RELAY_ON
	#undef RELAY_OFF
	#undef MAX_RELAY 

	#undef ADD_LOGGER_2         /* включает использование второго логгера  12.08.2014 */
	#define ADD_RELAY          /* включает использование реле  15.08.2014 */
	#define ENABLE_JSON 
	#define FIRMWARE_VERSION "1.1.5 WEB RELAY MINI 30.06.2016" 
	#define BUFFER_SIZE 750 
	#define MAX_RELAY 5
	#define RELAY_ON  1
	#define RELAY_OFF 0	
#endif


//-----------------------------------------------------------------------------------
#define DOZBUFSIZE                     5  // Буфер дозиметра

#define EEPOP				  0
#define SPECIALOP			  0x82

#define eeprom_login          0x40      /* добавлено 18.10.2010 */
#define eeprom_password       0x50		/* добавлено 18.10.2010 */
#define eeprom_server_config  0x0A0 // 0a1
#define eeprom_sensors_config 0x1A0 // Адрес для сохранения параметров датчиков
#define eeprom_relay_config   0x1A0+MAX_SENSORS*SIZE_SENSOR_STRUCT // Адрес для сохранения параметров датчиков
//#define DOZBUFSIZE 10  // Буфер дозиметра

#define CONFIG_CLIENT_PORT			1444 
#define CONFIG_SERVER_PORT			8444 



#define GoBoot()	do{								\
	MCUCR |= _BV(IVCE);			\
	MCUCR |= _BV(IVSEL);		\
	((void (*)())0x1e000)();	\
}while(0)

#define GoApp()		do{								\
	MCUCR |= _BV(IVCE);		\
	MCUCR &= ~_BV(IVSEL);		\
	((void (*)())0x0000)();		\
}while(0)


typedef struct
{
	unsigned char op[4];
	unsigned char ver[2];
	unsigned char mac[6];
	unsigned char ip[4];
	unsigned char subnet[4];
	unsigned char gw[4];
	unsigned char dns[4]; // 17.12.2012
	unsigned char dhcp;
} StrConfigParam;

typedef struct
{
	unsigned char enable;
	unsigned int  refresh_time;
	unsigned char server_name[26]; //30
	//unsigned char server_ip[4]; // erase 05.09.2014
	unsigned char script_path[46];
	unsigned int  port;
} Server_config;

typedef struct  // Будем хранить это в EEPROM! 18.02.2013
{
	unsigned char id[8];           //!< The 64 bit identifier.
	unsigned char flag;			   // Разрешить использование?	0=off,
	float         min;             // min
	float         max;             // max
	unsigned char pio;            // номер порта atmega
	//unsigned char opearnd;	       // ???
} io_structure; // структура реле


void	 leddebugsignal (char a);
void     LED_ON_OFF( char ckl );
char     LED_READ();
void     exe_relay();
void     process_http_server();
void     exe_sensors();
void	 exe_wireless();
uint16_t http2002ok(void);
uint16_t print_webpage_login(uint8_t *buf);
int      bmp085_read_temperature_and_pressure(int* temperature, long int* pressure);
float    read_current_sqrt(char adc_num, int coef);
char     search_and_configure_sensors(unsigned char ds_pin, unsigned char dht_pin, unsigned char * count );


#endif /* PROTOCOL_H_ */