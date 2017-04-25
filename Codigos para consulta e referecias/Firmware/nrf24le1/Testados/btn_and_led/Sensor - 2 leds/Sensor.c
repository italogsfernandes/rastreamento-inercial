/*Codigo para enviar um sinal simples por radio para acender e apagar um led
* que se encontra conectado a outro radio.
* Infomações:
* Sinal:
*       - Btn1 em P0-2 - envia numero 1 via radio
*       - Btn2 em P1-4 - envia numero 2 via radio
*       - LED1 em P0-3 - altera o estado ao receber numero 1
*       - LED2 em P0-6 - altera o estado ao receber numero 2
*/

#include "reg24le1.h" //Definições de muitos endereços de registradores.
#include "stdint.h" //inteiros uint8_t, int8_t, uint16_t....
#include "stdbool.h" //Bolleanos
#include "API.h" //Define alguns registers e cabeçalhos de funções SPI
#include "app.h" //Some UART and io functions
#include "nRF-SPIComands.h" //rf_init, RF_IRQ, TX, RX, SPI_Write, SPI_Read ..

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

void CheckButtons(void)
{
	if (!S1) {
		//LED1 = !LED1; Feedback
		tx_buf[0]=1;
		TX_Mode_NOACK(1);
		RX_Mode();
		delay(300);
	}
	if(!S2){
		//LED2 = !LED2; Feedback
		tx_buf[0]=2;
		TX_Mode_NOACK(1);
		RX_Mode();
		delay(300);
  }
}


void luzes_iniciais(void){
        LED1 = 1; LED2 = 0;
        delay(1000);
        LED1 = 0; LED2 = 1;
        delay(1000);
        LED1 = 1; LED2 = 1;
        delay(1000);
        LED1 = 0; LED2 = 0;
}
void main(void){
    // Set up GPIO
    P0DIR = 0xB7;   // 1011 0111 - 1/0 = In/Out - Output: P0.3 e P0.6
    P1DIR = 0xFF;   // Tudo input
    P2DIR = 0xFF;
    P0CON = 0x00;  	// All general I/O
    P1CON = 0x00;  	// All general I/O
    P2CON = 0x00;  	// All general I/O

    luzes_iniciais();

    // Radio + SPI setup
    RFCE = 0;       // Radio chip enable low
    RFCKEN = 1;     // Radio clk enable
    RF = 1;

    rf_init();
    EA=1; //ativa as interrupções

    RX_Mode();
		while(1){
			CheckButtons();
			if(newPayload){
				switch(rx_buf[0]){
					case 1:
						LED1 = !LED1;
						break;
					case 2:
						LED2 = !LED2;
						break;
				}
			}
			sta = 0;
      newPayload = 0;
			delay(100);
		}

//    while(1){
//        CheckButtons();
//        if(newPayload)								// finish received
//        {
//            if(rx_buf[0] == 1){
//                LED1 = !LED1;
//            } else if(rx_buf[0] == 2){
//                LED2 = !LED2;
//            }
//            sta = 0;
//            newPayload = 0;
//        }
//    }

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