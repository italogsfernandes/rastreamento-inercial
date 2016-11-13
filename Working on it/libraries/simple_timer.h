#ifndef SIMPLE_TIMER_H
#define SIMPLE_TIMER_H

#include <stdint.h>
#include <stdbool.h>

#define INTERRUPT_TMR0	1 //timer
#define NOVT0   0x00 

uint8_t NBT0L = 0x63;// Este tempo equivale a
uint8_t NBT0H = 0x52; // Freq. de Amostragem de 30Hz			

/**************************************************/
int timer_flag = 1;
void TMR0_IRQ(void) interrupt INTERRUPT_TMR0
{
	if(!NOVT0)
	{
		timer_flag--;
		TH0= NBT0H;
		TL0= NBT0L;
	}
}
//Timer comfigurado para freq de amostragem 30Hz
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

int8_t setup_timer(uint8_t periodo, int8_t freq_multiplier){
	timer_flag = freq_multiplier;
	NBT0L = 0x63;
	NBT0H = 0x52;
	return 0;
}

#endif