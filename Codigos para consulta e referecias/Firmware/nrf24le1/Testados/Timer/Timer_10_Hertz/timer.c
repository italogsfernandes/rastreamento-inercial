/*C�digo para comunica��o UART entre o nrf24le1 e um arduino
* pin P0.3 - TX
* pin P0.4 - RX
* please do not use P0.3 as LED or P0.4 as W2SCL
* i dont know if P0.4 could work with 2 functions (RX and SCL)
*/

#include "reg24le1.h" // I/O header file for NRF24LE1
#include "stdint.h"
#include "API.h"

#define INTERRUPT_TMR0	1

//Defini��es dos bot�es e leds
#define	PIN32
#ifdef 	PIN32
//Pushbuttons
sbit S1  = P0^2;    // 1/0=no/press
sbit S2  = P1^4;    // 1/0=no/press
//LEDS
sbit LEDVD = P0^6; // 1/0=light/dark
#endif

void uart_init(void);
void uart_putchar(uint8_t x);
void putstring(char *s);
void delay_ms(unsigned int x);
void start_T0(void);
void stop_T0(void);

/**************************************************/
// Vari�veis do TMR0
unsigned char NBT0H  = 0x52;			// Este tempo
unsigned char NBT0L  = 0x63;			// equivale a
unsigned char NOVT0  = 0x00;			// Freq. de Amostragem de 30Hz

/**************************************************/
int timer_flag = 3;

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

void luzes_iniciais(void){
        LEDVD = 1;
        delay_ms(1000);
        LEDVD = 0;
        delay_ms(1000);
        LEDVD = 1;
        delay_ms(1000);
        LEDVD = 0;
}

void setup(void){
	 // Set up GPIO
    P0DIR = 0xB7;   // 1011 0111 - 1/0 = In/Out - Output: P0.3 e P0.6 - Input: P0.4 e outros
    P1DIR = 0xFF;   // Tudo input
    P2DIR = 0xFF;
    P0CON = 0x00;  	// All general I/O
    P1CON = 0x00;  	// All general I/O
    P2CON = 0x00;  	// All general I/O

	// Initializes the UART
	uart_init();

	// Enable global interrupts
	EA = 1;


    luzes_iniciais();
}
void main(void){
	setup();
	while(1){
		if(!S1){
			putstring("acendeu\n");
			LEDVD = 1;
			start_T0();
			delay_ms(100); //evita ruidos
			while(!S1); //espera soltar o botao
			delay_ms(100);
		}
		if(!S2){
			putstring("apagou\n");
			LEDVD = 0;
			stop_T0();
			delay_ms(100);
			while(!S2);//espera soltar o botao
			delay_ms(100);
		}
		if(timer_flag <= 0){
			putstring("Lorem Ipsum10_HZ\n");
			timer_flag = 3;
		}
	}
}
/**************************************************/
/****************UART******************************/
/**************************************************/
void uart_init(void)
{
    ES0 = 0;                      // Disable UART0 interrupt while initializing(1:??????????? INE0^4)
    REN0 = 1;                     // Enable receiver(1:??????????? S0CON^4)
    SM0 = 0;                      // Mode 1..  ??8???g? SM0 SM1??01??
    SM1 = 1;                      // ..8 bit variable baud rate
    PCON |= 0x80;                 // SMOD = 1(????0?????????)
    ADCON |= 0x80;                // Select internal baud rate generator
								  // (ADCON??????0??????????J?????'???????????????????? )
    S0RELL = 0xf3;                // baudrate 38400
    S0RELH = 0x03;
    TI0 = 0;					  // S0CON^1:?????????????????????????
	S0BUF=0x00;					  //????0????????J???
}
/**************************************************/
void uart_putchar(uint8_t x)
{
	while (!TI0);
	TI0=0;
	S0BUF=x;
}

/*****************************/
// Repeated putchar to print a string
void putstring(char *s)
{
	while(*s != 0)
	uart_putchar(*s++);
}
/**************************************************/
void delay_ms(unsigned int x)
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
/****************TIMER*****************************/
/**************************************************/
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
