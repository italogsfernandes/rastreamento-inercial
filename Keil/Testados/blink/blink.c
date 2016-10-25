#include "reg24le1.h" //Definições de muitos endereços de registradores.
#include "stdint.h" //inteiros uint8_t, int8_t, uint16_t....
#include "stdbool.h" //Bolleanos

//Definições dos botões e leds
#define	PIN32
#ifdef 	PIN32
//Pushbuttons
sbit S1  = P0^2;    // 1/0=no/press
sbit S2  = P1^4;    // 1/0=no/press
//LEDS
sbit LED1 = P0^3; // 1/0=light/dark
sbit LED2 = P0^6; // 1/0=light/dark
#endif

void delay(unsigned int x);

void luzes_iniciais(void){
        LED1 = 1; LED2 = 0;
        delay(1000);
        LED1 = 0; LED2 = 1;
        delay(1000);
        LED1 = 1; LED2 = 1;
        delay(1000);
        LED1 = 0; LED2 = 0;
}

void setup(void){
	 // Set up GPIO
    P0DIR = 0xB7;   // 1011 0111 - 1/0 = In/Out - Output: P0.3 e P0.6
    P1DIR = 0xFF;   // Tudo input
    P2DIR = 0xFF;
    P0CON = 0x00;  	// All general I/O
    P1CON = 0x00;  	// All general I/O
    P2CON = 0x00;  	// All general I/O

    luzes_iniciais();
}
void main(void){
	setup();
	while(1){
	 	LED1 = !LED1;
		LED2 = !LED2;
		delay(1000);
	}
}

/**************************************************/
void delay(unsigned int x)
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
/**************************************************/