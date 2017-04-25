/* ExplicaÃ§Ã£o:
	* pin P0.3 - TX
	* pin P0.4 - RX
	Receptor:
	Redireciona os dados lidos para uma porta serial.
	Pode enviar sinais para acender leds(verificando assim a comunicação) ou requisitando leitura.

	Leitor Serial:
	Recebe o pacote
	[Start] [Size] [ADDR] [XAC_H] [XAC_L] [YAC_H] [YAC_L] [ZAC_H] [ZAC_L] [End]
	Interpreta atraves de Start, size e End, então mostra Xac, Yac e Zac
*/
/***********************************************/

#include "reg24le1.h" //Defini??es de muitos endere?os de registradores.
#include "stdint.h" //inteiros uint8_t, int8_t, uint16_t....
#include "stdbool.h" //Booleanos
#include "API.h"
#include "nRF-SPIComands.h"

//Subendere?os usados no sistema
#define RECEIVER_SUB_ADDR 0x02
#define SENSOR_SUB_ADDR 0x01

//Sinais utilizados na comunicacao via RF
#define Sinal_request_data 0x0A
#define Sinal_LEDS 0x0B

//Flags para enviar no pct serial
#define UART_START_FLAG	0x53
#define UART_END_FLAG 0x04

//Definições dos botões e leds
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
void luzes_iniciais(void);

unsigned int ii;//contador
void setup(void){
	 // Set up GPIO
    P0DIR = 0xB7;   // 1011 0111 - 1/0 = In/Out - Output: P0.3 e P0.6 - Input: P0.4 e outros
    P1DIR = 0xFF;   // Tudo input
    P2DIR = 0xFF;
    P0CON = 0x00;  	// All general I/O
    P1CON = 0x00;  	// All general I/O
    P2CON = 0x00;  	// All general I/O

	// Radio + SPI setup
    RFCE = 0;       // Radio chip enable low
    RFCKEN = 1;     // Radio clk enable
    RF = 1;
	//inicia o rf
	rf_init();
 	// Initializes the UART
	uart_init();

	// Enable global interrupts
	EA = 1;
	RX_Mode();
    luzes_iniciais();
	putstring("receptor ligado\n");
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
			putstring("sinal request enviado\n");
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
			putstring("sinal leds enviado\n");
			LEDVD = 0;
			delay_ms(100);
			while(!S2);//espera soltar o botao
			delay_ms(100);
		}
		if(newPayload && payloadWidth == 7){
			//redireciona dados lidos do sensor
			uart_putchar(UART_START_FLAG);	 
			uart_putchar(payloadWidth);
			
			for(ii=0;ii<payloadWidth; ii++){
				uart_putchar(rx_buf[ii]);
			}

			uart_putchar(UART_END_FLAG);

			sta = 0;
     		newPayload = 0;
		}
	}
	
}
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
void luzes_iniciais(void){
        LEDVD = 1;
        delay_ms(1000);
        LEDVD = 0;
        delay_ms(1000);
        LEDVD = 1;
        delay_ms(1000);
        LEDVD = 0;
}
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