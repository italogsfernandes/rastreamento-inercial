#include "reg24le1.h" //Defini��es de muitos endere�os de registradores.
#include "stdint.h" //inteiros uint8_t, int8_t, uint16_t....
#include "stdbool.h" //Booleanos
#include "API.h"
#include "nRF-SPIComands.h"


//Subendere�os usados no sistema
#define MY_SUB_ADDR 0x01
#define OTHER_SUB_ADDR 0x02
// pacote = [sub_endere�o_destinatario] QWH	QWL	QXH	QXL	QYH	QYL	QZH	QZL	AXH	AXL	AYH	AYL	AZH	AZL GXH	GXL	GYH	GYL	GZH	GZL	MXH	MXL	MYH	MYL	MZH	MZL
uint8_t pacote[27] = {OTHER_SUB_ADDR, 17,143,253,24,255,38,194,135,0,0,0,0,255,254,3,222,255,131,31,36,0,103,0,131,0,174};
// pacote a receber= [sub_endere�o_destinatario] [parar ou iniciar leituras]
#define Sinal_iniciar 0x0A
#define Sinal_parar 0x0B


#define INTERRUPT_TMR0	1

//Defini��es dos bot�es e leds
#define	PIN32
#ifdef 	PIN32
//Pushbuttons
sbit S1  = P0^2;    // 1/0=no/press
sbit S2  = P1^4;    // 1/0=no/press
//LEDS
sbit LED1 = P0^3; // 1/0=light/dark
sbit LED2 = P0^6; // 1/0=light/dark
#endif

void delay_ms(unsigned int x);
void start_T0(void);
void stop_T0(void);
void enviar_pacote_leituras();
int i = 0;

/**************************************************/
// Vari�veis do TMR0
unsigned char NBT0H  = 0x52;			// Este tempo
unsigned char NBT0L  = 0x63;			// equivale a
unsigned char NOVT0  = 0x00;			// Freq. de Amostragem de 30Hz
int timer_flag = 3;
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

void luzes_iniciais(void){
        LED1 = 1; LED2 = 0;
        delay_ms(1000);
        LED1 = 0; LED2 = 1;
        delay_ms(1000);
        LED1 = 1; LED2 = 1;
        delay_ms(1000);
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
	// Radio + SPI setup
    RFCE = 0;       // Radio chip enable low
    RFCKEN = 1;     // Radio clk enable
    RF = 1;

    rf_init();
    EA=1; //ativa as interrup��es
	luzes_iniciais();
    RX_Mode();
}
void main(void){
	setup();
	while(1){
		if(newPayload){
			//verifica se o sinal � para mim
			if(rx_buf[0]== MY_SUB_ADDR){
				 switch(rx_buf[1]){
					case Sinal_iniciar:
                        LED1 = 1;
						tx_buf[0] = OTHER_SUB_ADDR;
						tx_buf[1] = 'S';
						TX_Mode_NOACK(2);
						RX_Mode();
                        start_T0();
						break;
					case Sinal_parar:
						LED1 = 0;
						tx_buf[0] = OTHER_SUB_ADDR;
						tx_buf[1] = 'E';
						TX_Mode_NOACK(2);
						RX_Mode();
                        stop_T0();
						break;
				}

			}
			sta = 0;
     		newPayload = 0;
		}
		if(timer_flag <= 0){
			timer_flag = 3;
            enviar_pacote_leituras();
		}
	}
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
/********************PACOTE******************/
void enviar_pacote_leituras(){
	LED2 = 1;
	tx_buf[0] = OTHER_SUB_ADDR;
	for(i = 1; i<27; i++){
		tx_buf[i] = pacote[i];
	}
	TX_Mode_NOACK(27);
	RX_Mode();
	LED2 = 0;
}
