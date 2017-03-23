#ifndef SIMPLE_TIMER_H
#define SIMPLE_TIMER_H

#include "nrf24le1.h"
#include "stdint.h"
#include "reg24le1.h" //Defini��es de muitos endere�os de registradores.
#include "stdbool.h" //Booleanos
#include "API.h"

/**************************************************/
/********************TIMER*************************/
#define INTERRUPT_TMR0	1 //timer
#define NOVT0   0x00 
#define NBT0L	 	0xEA // Este tempo equivale a
#define NBT0H	 	0xCB // Freq. de Amostragem de 100Hz			
int timer_flag = 1;

void start_T0(void);
void stop_T0(void);
/********************TIMER*************************/
/**************************************************/

/**************************************************/
/********************TIMER*************************/
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
/********************TIMER*************************/
/**************************************************/
#endif