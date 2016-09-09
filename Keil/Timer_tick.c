

#include "reg24le1.h" // I/O header file for NRF24LE1
#include <stdint.h>
#include <stdbool.h>

#define INTERRUPT_TMR0	1

#define	PIN32
#ifdef 	PIN32
sbit S2 = P1^4;                               	// 1/0=no/press
sbit S1 = P0^2;                               	// 1/0=no/press

sbit LED_VD = P0^6;                             // 1/0=light/dark
sbit LED_VM = P0^3;                             // 1/0=light/dark
#endif

/**************************************************/
// Vari�veis do TMR0
unsigned char NBT0H  = 0xE5;			// Este tempo
unsigned char NBT0L  = 0xF6;			// equivale a
unsigned char NOVT0  = 0x00;			// Freq. de Amostragem
unsigned char NPRT0H = 0x00;			// de 200Hz
unsigned char NPRT0L = 0x00;			//
unsigned char count;

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

/**************************************************/
void TMR0_IRQ(void) interrupt INTERRUPT_TMR0
{
	TH0 = NBT0H;
	TL0 = NBT0L;
	LED_VD = !LED_VD;
	LED_VM = !LED_VM;
	//pisca os leds sempre que a interrupção pedir
}

// main function
void main()
{
	P0DIR = 0xB7; //P03 e P06 Outputs
	P1DIR = 0xFF; //Inputs
	P2DIR = 0XFF; //Inputs
	P0CON = 0x00;
	P1CON = 0x00;
	P2CON = 0x00;

	LED_VD = 1;
	LED_VM = 0;
	start_T0();
}
