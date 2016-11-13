#ifndef SIMPLE_TIMER_H
#define SIMPLE_TIMER_H

#include "nrf24le1.h"
#include "stdint.h"
#include "reg24le1.h" //Defini��es de muitos endere�os de registradores.
#include "stdbool.h" //Booleanos
#include "API.h"


#define INTERRUPT_TMR0	1 //timer


#define NOVT0   0x00 
uint8_t NBT0L = 0x63;// Este tempo equivale a
uint8_t NBT0H = 0x52; // Freq. de Amostragem de 30Hz			
int timer_flag = 1;

void start_T0(void);
void stop_T0(void);
void setup_T0(uint16_t freq_value, int flag_freq_multplier);

/**************************************************/
void TMR0_IRQ(void) interrupt INTERRUPT_TMR0
{
	if(!NOVT0)
	{
		timer_flag--;
		TH0= NBT0H;
		TL0= NBT0L;
	}
}
/**************************************************/
void start_T0(void)
{
	TMOD=0x31;						// Select Timer 1 --> STOPPED, Timer 0 --> TIMER/16 bits
	TH0= NBT0H;
	TL0= NBT0L;
	ET0=1;								// Active interrupt on Timer 0
	EA=1;									// Active all interrupts
	TR0=1;								// Timer 0 --> RUN
}
/**************************************************/
void stop_T0(void)
{
	TMOD=0x31;						// Select Timer 1 --> STOPPED, Timer 0 --> TIMER/16 bits
	TH0= NBT0H;
	TL0= NBT0L;
	ET0=0;								// Active interrupt on Timer 0
	EA=1;									// Active all interrupts
	TR0=0;								// Timer 0 --> RUN
}
/*********************************************/

void setup_T0(uint16_t freq_value, int flag_freq_multplier){
	timer_flag = flag_freq_multplier;
	NBT0H = freq_value >> 8;
	NBT0L = (uint8_t) freq_value;
}

#endif