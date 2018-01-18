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

#include <util/delay.h>
#include "OWIPolled.h"
#include "compilers.h"

#include "OWIBitFunctions.h"

#include "term.h"
#include "DHT.h"
#include "protocol.h"

unsigned char bGlobalErr=1; // 0 = ok
//unsigned char dht_dat[5];
signed   int dht_term=0;
unsigned int dht_hum=0;

/*
void InitDHT(unsigned char pins){
	OWI_Init(pins);
}
*/


char READBITDHT(unsigned char pins)
{
  return ((PIND&(1<<pins))>0);
}

unsigned char DHT_DetectPresence(unsigned char pins)
{
    unsigned char intState;
    unsigned char presenceDetected=0;
    
    // Disable interrupts.
    intState = __save_interrupt();
    __disable_interrupt();
    
    
/*
  if (ckl == 1) PORTD |= (1<<4);
  else PORTD &= ~(1<<4);
 */
	
	// Drive bus low and delay.
    //OWI_PULL_BUS_LOW(pins);
	
	DDRD |= (1<<pins);
	//DDRF &= ~ (1<<pins);
	PORTD &= ~(1<<pins);
	
    //DDRF=0xFF;
	//PORTF &= ~0x10;
	
	
	my_wait_1ms(1); // bylo 18	
	
	PORTD |= (1<<pins);
    _delay_us(40);
	
	//DDRF |= (1<<pins);
	DDRD &= ~(1<<pins);
	

	_delay_us(40);
    
    // Sample bus to detect presence signal and delay.
    presenceDetected = READBITDHT(pins); 

	//_delay_us(80);

    // Restore interrupts.
    __restore_interrupt(intState);
    
    return presenceDetected;
}




unsigned char DHT_Read_byte(unsigned char pins)
{
  unsigned char intState;
  unsigned char i = 0;
  unsigned char result=0;
  unsigned int  exit_counter;
  
    intState = __save_interrupt();
    __disable_interrupt();
  
  for(i=0; i< 8; i++){
      //We enter this during the first start bit (low for 50uS) of the byte
      //Next: wait until pin goes high
      exit_counter=0;
	  while(READBITDHT(pins)==0)
	  {
		asm("WDR");
		if (exit_counter++>10000) goto EXIT_DHT_Read_byte;	 // add 16.05.2014	
	  }
	  //Was: while(!(PINC & _BV(dht_PIN)));
            //signalling end of start of bit's transmission.

      //Dataline will now stay high for 27 or 70 uS, depending on
            //whether a 0 or a 1 is being sent, respectively.
      _delay_us(30);//AFTER pin is high, wait further period, to be
        //into the part of the timing diagram where a 0 or a 1 denotes
        //the datum being send. The "further period" was 30uS in the software
        //that this has been created from. I believe that a higher number
        //(45?) would be more appropriate.

      //Next: Wait while pin still high
      if (READBITDHT(pins)==1)//Was: if(PINC & _BV(dht_PIN))
 	   result |=(1<<(7-i));// "add" (not just addition) the 1
                      //to the growing byte
    //Next wait until pin goes low again, which signals the START
    //of the NEXT bit's transmission.
      exit_counter=0;
	  while (READBITDHT(pins)==1)//Was: while((PINC & _BV(dht_PIN)));	  
	  {
	    asm("WDR");
		if (exit_counter++>10000) goto EXIT_DHT_Read_byte;	// add 16.05.2014	
	  }
    }//end of "for.."
	
EXIT_DHT_Read_byte:	
	__restore_interrupt(intState);
	
  return result;
}//end of "read_dht_dat()"



void ReadDHT(unsigned char pins){
unsigned char it;
unsigned char dht_check_sum=0;
unsigned int  exit_counter;
unsigned char dht_dat[5];


bGlobalErr=0;

cli();

bGlobalErr=DHT_DetectPresence(pins);
if (bGlobalErr==1) goto EXIT_ON_GLOBAL_ERROR;

_delay_us(50); // 80

exit_counter=0;
while (READBITDHT(pins)==0)  { // add 09.06.2014
	    if (exit_counter++>70) { break;	}
	    _delay_us(1);
}

//printf("%d\r\n",exit_counter);
//if (it==0) { bGlobalErr=2; return; }
	
if (READBITDHT(pins)==0) { bGlobalErr=2; goto EXIT_ON_GLOBAL_ERROR; }

_delay_us(80);


for (it=0; it<5; it++)  dht_dat[it] = DHT_Read_byte(pins);
//sei();

//Next see if data received consistent with checksum received
dht_check_sum = dht_dat[0]+dht_dat[1]+dht_dat[2]+dht_dat[3];
/*Condition in following "if" says "if fifth byte from sensor
       not the same as the sum of the first four..."*/
if(dht_dat[4]!= dht_check_sum)
   {bGlobalErr=55; goto EXIT_ON_GLOBAL_ERROR; }//Was: Serial.println("DHT11 checksum error");
   
 dht_hum=dht_dat[0]*256+dht_dat[1];
 
 dht_term=(dht_dat[2]&0x7F)*256+dht_dat[3];   
 if ((dht_dat[2]&0x80)>0) dht_term *= -1;
 

EXIT_ON_GLOBAL_ERROR:	
sei();   
   
};//end ReadDHT()