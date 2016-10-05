/* Explicação:
    Leitor:
    Ao se clicar no botão 1 - É realizada uma leitura e enviada
    Ao se clicar no botão 2 - É enviado um sinal para o led
    Esse sinal é apenas um modo de saber se o radio esta funcionando bem.

    Ao receber o comando de realizar leituras. Essas sao realizadas e enviadas de volta.
    Ao se receber o comando de acender o led, o esdado do LEDVD é alterado.

    Receptor:
    Ao receber qlqr sinal ele é enviado na serial para o arduino due.
    Caso seja um sinal de led, ele é mostrado e o led acesso.
    Casso seja uma leitura antes de mostrar ela é formadada.

    Ao se clicar em seus botoes é enviado um sinal de requisitar leitura ou um
    sinal de alterar led.
*/

#include "hal_w2_isr.h"
#include "reg24le1.h" //Defini��es de muitos endere�os de registradores.
#include "stdint.h" //inteiros uint8_t, int8_t, uint16_t....
#include "stdbool.h" //Booleanos
#include "API.h"
#include "nRF-SPIComands.h"

//Subendere?os usados no sistema
#define MY_SUB_ADDR 0x01
#define OTHER_SUB_ADDR 0x02
//pacote para enviar:
// pacote = [sub_endere?o_destinatario] AXH	AXL	AYH	AYL	AZH	AZL
//Pacote para receber
// pacote = [MY_SUB_ADDR] [COMANDO]
#define Sinal_request_data 0x0A
#define Sinal_LEDS 0x0B

//Endereço I2C do sensor
#define MPU_endereco 0x69

//Defini??es dos bot?es e leds
#define	PIN32
#ifdef 	PIN32
//Pushbuttons
sbit S1  = P0^2;    // 1/0=no/press
sbit S2  = P1^4;    // 1/0=no/press
//LEDS
sbit LED1 = P0^3; // 1/0=light/dark
sbit LED2 = P0^6; // 1/0=light/dark
#endif

//Onde as leituras serão salvas:
uint8_t readings[6] = {0,5,0,128,0,255};
int i=0;

void delay_ms(unsigned int x);
void setup_i2c_mpu(void);
void requisitarAccelMPU6050(void);
void enviar_pacote_inercial(void);


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
	//luzes_iniciais();
	
	// Radio + SPI setup
    RFCE = 0;       // Radio chip enable low
    RFCKEN = 1;     // Radio clk enable
    RF = 1;
    rf_init();
    EA=1; //ativa as interrup��es
    RX_Mode();
	luzes_iniciais();
}

void configure_radio(void){
	P0DIR = 0xB7;
	// Radio + SPI setup
    RFCE = 0;       // Radio chip enable low
    RFCKEN = 1;     // Radio clk enable
    RF = 1;
    rf_init();
    EA=1; //ativa as interrup��es
    RX_Mode();
}
void main(void){
	setup();
	while(1){
		if(!S1){
			configure_radio();
			enviar_pacote_inercial();
			delay_ms(100); //evita ruidos
			while(!S1); //espera soltar o botao
			delay_ms(100);
		}
		if(!S2){
			hal_w2_configure_master(HAL_W2_400KHZ);
			EA=1; //ativa as interrup��es
			delay_ms(100);
			setup_i2c_mpu();
			delay_ms(100);
			requisitarAccelMPU6050();
			delay_ms(100);
			while(!S2);//espera soltar o botao
			delay_ms(100);
		}
		if(newPayload){
			//verifica se o sinal ? para mim
			if(rx_buf[0] == MY_SUB_ADDR){
				 switch(rx_buf[1]){
					case Sinal_request_data:
						requisitarAccelMPU6050();
						enviar_pacote_inercial();
						break;
					case Sinal_LEDS:
						LED1 = !LED1;
						LED2 = !LED2;
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
/**************************************************/
/************MPU**********************************/
void setup_i2c_mpu(void)
{
    //iniciar i2c
	i2c_mpu_writeByte(MPU_endereco, 0x6B, 0x00);
}

void requisitarAccelMPU6050(void) {
    //Ler 6 bytes a partir de 0x3B
    i2c_mpu_readBytes(MPU_endereco,0x3B, 6,readings);
    //salvar em cada variavel -- Nao sera realizado aqui pois isso sera tarefa do arduino
    // Xac = readings[0] << 8 | readings[1];   //0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
    // Yac = readings[2] << 8 | readings[3];   //0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
    // Zac = readings[4] << 8 | readings[5];   //0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
}

void enviar_pacote_inercial(void){
    LED2 = 1;
    tx_buf[0] = MY_SUB_ADDR;
    for(i=1;i<7;i++){
        tx_buf[i] = readings[i-1];
    }
    //enviando e retornando ao padrao:
    TX_Mode_NOACK(7);
    RX_Mode();
	LED2 = 0;
}

void I2C_IRQ (void) interrupt INTERRUPT_SERIAL
{

	I2C_IRQ_handler();
}


