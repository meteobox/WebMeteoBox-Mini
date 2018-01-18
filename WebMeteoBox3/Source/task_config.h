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


#ifndef TASK_CONFIG_H_
#define TASK_CONFIG_H_

//-----------------------------------------------------------------------------
// AVR ADC Defination
#define ADC_SR_REG							ADCSR

#define ADC_PRESCALE_DIV2                   1
#define ADC_PRESCALE_DIV4                   2
#define ADC_PRESCALE_DIV8                   3
#define ADC_PRESCALE_DIV16                  4
#define ADC_PRESCALE_DIV32                  5
#define ADC_PRESCALE_DIV64                  6
#define ADC_PRESCALE_DIV128                 7

#define ADC_ENABLE                          0x80
#define ADC_START_CONVERSION                0x40
#define ADC_FREE_RUNNING                    0x20
#define ADC_COMPLETE                        0x10
#define ADC_INT_ENABLE                      0x08

#define ADC_REF_VREF                        0
#define ADC_REF_AVCC                        0x40
#define ADC_REF_INTERNAL                    0xC0

#ifdef ADD_LOGGER_2
	Server_config server_data[2]; // 2 כמדדונא
#else
	Server_config server_data[1]; // 1 כמדדונ
#endif


void default_network(char level);
void mcu_init(void);
void SetConfig(void);
void ENC28J60_RST( char ckl );
//void ENC28J60_REBOOT();
void ENC28J60_INIT_CHIP();


#endif /* TASK_CONFIG_H_ */