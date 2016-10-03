#include "reg24le1.h" //Defini��es de muitos endere�os de registradores.
#include "stdint.h" //inteiros uint8_t, int8_t, uint16_t....
#include "stdbool.h" //Booleanos
#include "API.h"
#include "nRF-SPIComands.h"

//Subendere�os usados no sistema
#define MY_SUB_ADDR 0x01
#define OTHER_SUB_ADDR 0x02
//pacote para enviar:
// pacote = [OTHER_SUB_ADDR] [COMANDO]
//Pacote para receber
// pacote = [OTHER_SUB_ADDR] [sinal de leituras] [leituras]
//ou pacote = [OTHER_SUB_ADDR] [sinal_led]
#define Sinal_Requisitar_Leituras 0x0A
#define Sinal_LED 0x0B
#define Sinal_leituras 0x0C

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
void putstring(char *s);
void uart_putchar(uint8_t x);
void delay_ms(unsigned int x);


void delay(unsigned int x);

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
    //pacote para enviar:
    // pacote = [OTHER_SUB_ADDR] [COMANDO]
    //Pacote para receber
    // pacote = [OTHER_SUB_ADDR] [sinal de leituras] [leituras]
    //ou pacote = [OTHER_SUB_ADDR] [sinal_led]
	while(1){
		if(!S1){
			//montando o pacote:
			tx_buf[0] = OTHER_SUB_ADDR;
			tx_buf[1] = Sinal_Requisitar_Leituras;
			//enviando e retornando ao padrao:
			TX_Mode_NOACK(2);
			RX_Mode();
			putstring("sinal requisitar enviado\n");
			delay_ms(100); //evita ruidos
			while(!S1); //espera soltar o botao
			delay_ms(100);
		}
		if(!S2){
			//montando o pacote:
			tx_buf[0] = OTHER_SUB_ADDR;
			tx_buf[1] = Sinal_LED;
			//enviando e retornando ao padrao:
			TX_Mode_NOACK(2);
			RX_Mode();
			putstring("sinal LED enviado\n");
			delay_ms(100);
			while(!S2);//espera soltar o botao
			delay_ms(100);
		}
		if(newPayload){
            int i;
            switch(rx_buf[1]){
                case Sinal_leituras:
                    uart_putchar(rx_buf[0]);
                    for(i=2;i<payloadWidth; i++){
                        uart_putchar(rx_buf[i]);
                    }
                    uart_putchar('\n');
                    break;
                case Sinal_LED:
                    LEDVD = !LEDVD;
                    break;
            }

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
