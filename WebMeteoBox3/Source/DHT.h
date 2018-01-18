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


#define DHTPIN    6	//OWI_PIN_4   0x10
#define DHTPIN2   7 // Ds18B20 port! add 10.10.2013


extern unsigned char bGlobalErr;
extern unsigned char dht_dat[5];

extern signed   int dht_term;
extern unsigned int dht_hum;

void InitDHT(unsigned char pins);
void ReadDHT(unsigned char pins);