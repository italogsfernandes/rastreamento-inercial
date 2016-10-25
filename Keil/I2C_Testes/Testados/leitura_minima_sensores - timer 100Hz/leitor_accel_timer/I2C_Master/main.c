/* ExplicaÃ§Ã£o:
    Leitor:
    Ao se clicar no botÃ£o 1 - a configuração da mpu é feita
    Ao se clicar no botÃ£o 2 - é realizada a leitura e envio
    Esse sinal Ã© apenas um modo de saber se o radio esta funcionando bem.
	
	Receptor:
	Redireciona os dados lidos para uma porta serial.
	Pode enviar sinais para acender leds(verificando assim a comunicação) ou requisitando leitura.

	Leitor Serial:
	Recebe o pacote
	[Start] [Size] [ADDR] [XAC_H] [XAC_L] [YAC_H] [YAC_L] [ZAC_H] [ZAC_L] [End]
	Interpreta atraves de Start, size e End, então mostra Xac, Yac e Zac
*/
/***********************************************/

#include "nrf24le1.h"
#include <hal_w2_isr.h>
#include "hal_delay.h"
#include "stdint.h"
#include "hal_w2_isr.h"
#include "reg24le1.h" //Definiï¿½ï¿½es de muitos endereï¿½os de registradores.
#include "stdbool.h" //Booleanos
#include "API.h"
#include "nRF-SPIComands.h"

#define INTERRUPT_TMR0	1 //timer

//Subendere?os usados no sistema
#define MY_SUB_ADDR 0x01
#define OTHER_SUB_ADDR 0x02

//Sinais utilizados na comunicacao via RF
#define Sinal_request_data 0x0A
#define Sinal_LEDS 0x0B

//Endereco I2C da MPU
#define MPU_address 0x68
//leituras realizadas
uint8_t readings[6] = {0,5,0,128,0,255}; //some fake data


//Definicoes dos botoes e leds
#define	PIN32 //módulo com 32 pinos
#ifdef 	PIN32
//Pushbuttons
sbit S1  = P0^2;    // 1/0=no/press
sbit S2  = P1^4;    // 1/0=no/press
//LEDS
sbit LEDVM = P0^3; // 1/0=light/dark
sbit LEDVD = P0^6; // 1/0=light/dark
#endif
	  

//funcoes utilizada para delay e leds indicativos
void italo_delay_ms(unsigned int x);
void luzes_iniciais(void);

//Joga no buffer do radio e despacha
void enviar_pacote_inercial(void);

//necessaria para i2c
//BUG: onde?
void delay_timer0(void);


void start_T0(void);
void stop_T0(void);

/**************************************************/
// Variï¿½veis do TMR0
unsigned char NBT0H  = 0xCB;			// Este tempo
unsigned char NBT0L  = 0xEA;			// equivale a
unsigned char NOVT0  = 0x00;			// Freq. de Amostragem de 100Hz

/**************************************************/
int timer_flag = 1;
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

void setup(void){
	//*************************** Init GPIO Pins	
	P0DIR = 0xB7;   // 1011 0111 - 1/0 = In/Out - Output: P0.3 e P0.6
    P1DIR = 0xFF;   // Tudo input
    P2DIR = 0xFF;
    P0CON = 0x00;  	// All general I/O
    P1CON = 0x00;  	// All general I/O
    P2CON = 0x00;  	// All general I/O
	//*************************** Init I2C

	// Radio + SPI setup
    RFCE = 0;       // Radio chip enable low
    RFCKEN = 1;     // Radio clk enable
    RF = 1;
    rf_init();
    RX_Mode();

	//I2C setup
	hal_w2_configure_master(HAL_W2_400KHZ);
    EA=1; //ativa as interrupï¿½ï¿½es
	luzes_iniciais();
	i2c_mpu_writeByte(MPU_address, 0x6B, 0x00);
	LEDVD = !LEDVD;
}

void main()
{
	setup();
	while(1)	
	{
		//Verificando Botoes
		if(!S1){
			start_T0();
			italo_delay_ms(100);
			while(!S1); //espera soltar o botao
  			italo_delay_ms(100);
		}
		if(!S2){
			stop_T0();	
			italo_delay_ms(100);
			while(!S2); //espera soltar o botao
  			italo_delay_ms(100);
		}
		//Verificando radio
		if(newPayload){
			//verifica se o sinal eh direficionado para mim
			if(rx_buf[0] == MY_SUB_ADDR){
				 switch(rx_buf[1]){
					case Sinal_request_data:
						start_T0();
						LEDVM = 1;
						break;
					case Sinal_LEDS:
						stop_T0();
						LEDVM = 0;
						break;
				}

			}
			sta = 0;
     		newPayload = 0;
		}
		//timer tick
		if(timer_flag <= 0){
			i2c_mpu_readBytes(MPU_address,0x3B, 6,readings);
			enviar_pacote_inercial();
			timer_flag = 1;
		}
	}
}
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
        LEDVM = 1; LEDVD = 0;
        delay_ms(1000);
        LEDVM = 0; LEDVD = 1;
        delay_ms(1000);
        LEDVM = 1; LEDVD = 1;
        delay_ms(1000);
        LEDVM = 0; LEDVD = 0;
}

void enviar_pacote_inercial(void){
	unsigned int i;
    tx_buf[0] = MY_SUB_ADDR;
    for(i=1;i<7;i++){
        tx_buf[i] = readings[i-1];
    }
    //enviando e retornando ao padrao:
    TX_Mode_NOACK(7);
    RX_Mode();
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
/*********************************************/


void I2C_IRQ (void) interrupt INTERRUPT_SERIAL
{

	I2C_IRQ_handler();
}