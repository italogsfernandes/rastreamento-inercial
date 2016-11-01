/*
INT0 is P0.5 and for INT1 is P0.6.
First write 1 in bit7 of IEN0 to enable global interrupts and 1 in bit0.
Write 1 in bit3 of INTEXP to select INT0.
We have to write ISR for INT0.
*/
/*Portas usadas
P0.2 = btn 1 - pull up
P0.4 = btn 2 - pull up
P0.3 = LED vermelho
P0.6 = Interrupt ...
P0.5 = W2SDA
P0.4 = W2SCL
*/

#include"reg24le1.h"         // I/O header file for NRF24LE1
#include"stdint.h"         // header file containing standard I/O functions
#include"hal_delay.h"      // header file containing delay functions
#include"isrdef24le1.h"    //header file containing Interrupt Service Routine definition for NRF

#define INTERRUPT_IPF 2		//referencia keil interrupt 8051
//Definicoes dos botoes e leds
#define	PIN32 //m�dulo com 32 pinos
#ifdef 	PIN32
//Pushbuttons
sbit S1  = P0^2;    // 1/0=no/press
sbit S2  = P1^4;    // 1/0=no/press
//LEDS
sbit LEDVM = P0^3; // 1/0=light/dark
//LED desativado pois havera uma interrupção nele:
//sbit LEDVD = P0^6; // 1/0=light/dark
#endif

void luzes_iniciais(void);
void italo_delay_ms(unsigned int x);

void setup(void){
    //*************************** Init GPIO Pins
    P0DIR = 0xF7;   // 1111 0111 - 1/0 = In/Out - Output: P0.3
    P1DIR = 0xFF;   // Tudo input
    P2DIR = 0xFF;
    P0CON = 0x00;  	// All general I/O
	//PQ p1com?
    P1CON |= 0x53;  	// All general I/O
    P2CON = 0x00;  	// All general I/O
    //*************************** Init I2C
    luzes_iniciais();
}

void main() // main code
{
    setup();
	EX0=1;
	INTEXP = 0x10;
	IT0 = 1;
	EA = 1;
	LEDVM = 1;
    /*
	IEN0&=0x81;          // enable interrupt from pin
    INTEXP&=0x10;       // enable INT1
    TCON&=0x04;         //select falling for int1 mode
	*/

    while(1);                     // infinite loop, wait for interrupt

}

void ext0_irq(void) interrupt INTERRUPT_IPF 
{
    LEDVM = !LEDVM;
}
/****************/
void italo_delay_ms(unsigned int x)
{
    unsigned int i,j;
    i=0;
    for(i=0;i<x;i++)
    {
       j=508;
           ;
       while(j--);
    }
}
void luzes_iniciais(void){
        LEDVM = 1;
        italo_delay_ms(1000);
        LEDVM = 0;
        italo_delay_ms(1000);
        LEDVM = 1;
        italo_delay_ms(1000);
        LEDVM = 0;
}
/***************/
