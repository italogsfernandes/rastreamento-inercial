/* Explicação:
	* pin P0.3 - TX
	* pin P0.4 - RX
	Receptor:
	Redireciona os dados lidos para uma porta serial.
	Pode enviar sinais para acender leds(verificando assim a comunica��o) ou requisitando leitura.

	Leitor Serial:
	Recebe o pacote
	[Start] [Size] [ADDR]
    [XAC_H] [XAC_L] [YAC_H] [YAC_L] [ZAC_H] [ZAC_L]
    [XGY_H] [XGY_L] [YGY_H] [YGY_L] [ZGY_H] [ZGY_L]
    [End]
	Interpreta atraves de Start, size e End, ent�o mostra as leituras
*/
/***********************************************/

#include "reg24le1.h" //Defini??es de muitos endere?os de registradores.
#include "stdint.h" //inteiros uint8_t, int8_t, uint16_t....
#include "stdbool.h" //Booleanos
#include "API.h"
#include <nRF-SPIComands.h>
#include <uart_basics.h>
#include <pacotes_inerciais.h>

//Subendere?os usados no sistema
#define RECEIVER_SUB_ADDR 0x02
#define SENSOR_SUB_ADDR 0x01

//Sinais utilizados na comunicacao via RF
#define Sinal_request_data 0x0A
#define Sinal_LEDS 0x0B

//Defini��es dos bot�es e leds
#define	PIN32
#ifdef 	PIN32
//Pushbuttons
sbit S1  = P0^2;    // 1/0=no/press
sbit S2  = P1^4;    // 1/0=no/press
//LEDS
sbit LEDVD = P0^6; // 1/0=light/dark
#endif


void delay_ms(unsigned int x);
void luzes_iniciais(void);

void iniciarIO(void){
    //*************************** Init GPIO Pins
   P0DIR = 0xB7;   // 1011 0111 - 1/0 = In/Out - Output: P0.3 e P0.6 - Input: P0.4 e outros
   P1DIR = 0xFF;   // Tudo input
   P2DIR = 0xFF;
   P0CON = 0x00;  	// All general I/O
   P1CON = 0x00;  	// All general I/O
   P2CON = 0x00;  	// All general I/O
}
void setup(void){
    iniciarIO();
    iniciarRF();
	uart_init();// Initializes the UART
	EA = 1; luzes_iniciais(); // Enable global interrupts
	send_packet_from_host_to_computer(UART_PACKET_TYPE_STRING,"receptor ligado",15);
}
void main(void){
	setup();
	while(1){
		if(!S1){
			//montando o pacote:
			tx_buf[0] = SENSOR_SUB_ADDR;
			tx_buf[1] = Sinal_request_data;
			//enviando e retornando ao padrao:
			TX_Mode_NOACK(2);
			RX_Mode();
			send_packet_from_host_to_computer(UART_PACKET_TYPE_STRING,"Sinal request enviado",21);
			delay_ms(100); //evita ruidos
			while(!S1); //espera soltar o botao
			delay_ms(100);
		}
		if(!S2){
			//montando o pacote:
			tx_buf[0] = SENSOR_SUB_ADDR;
			tx_buf[1] = Sinal_LEDS;
			//enviando e retornando ao padrao:
			TX_Mode_NOACK(2);
			RX_Mode();
			send_packet_from_host_to_computer(UART_PACKET_TYPE_STRING,"Sinal leds enviado",18);
			LEDVD = 0;
			delay_ms(100);
			while(!S2);//espera soltar o botao
			delay_ms(100);
		}
		if(newPayload){
			send_packet_to_computer(rx_buf[0], rx_buf, payloadWidth-1);
			sta = 0;
			newPayload = 0;
		}
	}

}

void luzes_iniciais(void){
        LEDVD = 1;
        delay_ms(1000);
        LEDVD = 0;
        delay_ms(1000);
        LEDVD = 1;
        delay_ms(1000);
        LEDVD = 0;
}
void delay_ms(unsigned int x){
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
