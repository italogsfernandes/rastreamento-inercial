#include "reg24le1.h" //Defini��es de muitos endere�os de registradores.
#include "stdint.h" //inteiros uint8_t, int8_t, uint16_t....
#include "stdbool.h" //Booleanos
#include "API.h"
#include "nRF-SPIComands.h"


//Subendere�os usados no sistema
#define MY_SUB_ADDR 0x01
#define OTHER_SUB_ADDR 0x02
// pacote = [sub_endere�o_destinatario] AXH	AXL	AYH	AYL	AZH	AZL
uint8_t pacote[7] = {MY_SUB_ADDR, 0,0,0,0,255,254};
// pacote a receber= [sub_endere�o_destinatario] [parar ou iniciar leituras]
#define Sinal_request_data 0x0A
#define Sinal_LEDS 0x0B


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
void enviar_pacote_leituras();
int i = 0;


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
			if(rx_buf[0] == MY_SUB_ADDR){
				 switch(rx_buf[1]){
					case Sinal_request_data:
						enviar_pacote_leituras();
						break;
					case Sinal_LEDS:
						LED1 = !LED1;
						LED2 = !LED2;
						delay_ms(500);
						LED1 = !LED1;
						LED2 = !LED2;
						delay_ms(500);
						break;
				}

			}
			sta = 0;
     		newPayload = 0;
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
/********************PACOTE******************/
void enviar_pacote_leituras(){
	LED2 = 1;
	tx_buf[0] = MY_SUB_ADDR;
	for(i = 1; i<7; i++){
		tx_buf[i] = pacote[i];
	}
	TX_Mode_NOACK(7);
	RX_Mode();
	LED2 = 0;
}
