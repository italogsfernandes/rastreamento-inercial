#include "dmp.h"

#include "nrf24le1.h"
#include "hal_w2_isr.h"
#include "hal_delay.h"
#include "stdint.h"
#include "reg24le1.h" //Definiï¿½ï¿½es de muitos endereï¿½os de registradores.
#include "stdbool.h" //Booleanos
#include "API.h"
#include "nRF-SPIComands.h"

//Subendere?os usados no sistema
#define MY_SUB_ADDR 0x01
#define OTHER_SUB_ADDR 0x02

//Sinais utilizados na comunicacao via RF
#define Sinal_request_data 0x0A
#define Sinal_LEDS 0x0B
#define UART_HEX_PRINT_FLAG 0x22
#define SIGNAL_SENSOR_MSG 0x97

//Definicoes dos botoes e leds
#define	PIN32 //m�dulo com 32 pinos
#ifdef 	PIN32
//Pushbuttons
sbit S1  = P0^2;    // 1/0=no/press
sbit S2  = P1^4;    // 1/0=no/press
//LEDS
sbit LEDVM = P0^3; // 1/0=light/dark
#endif


void luzes_iniciais(void);
void enviar_motion6(void);//Joga no buffer do radio e despacha
void enviar_msg_to_host(char *msg_to_send); //envia msg para ser mostrada no receptor
void enviar_msg_to_host_hex(char *msg_to_send);

void iniciarIO(void){
    //*************************** Init GPIO Pins
    P0DIR = 0xF7;   // 1111 0111 - 1/0 = In/Out - Output: P0.3
    P1DIR = 0xFF;   // Tudo input
    P2DIR = 0xFF;
    P0CON = 0x00;  	// All general I/O
    //PQ p1com?
    P1CON |= 0x53;  	// All general I/O
    P2CON = 0x00;  	// All general I/O
}
void setup() {
    iniciarIO(); //IO
    iniciarRF(); //RF
    hal_w2_configure_master(HAL_W2_100KHZ); //I2C
    EA=1; luzes_iniciais(); //Enable All interrupts, e pisca luzes
		enviar_msg_to_host("RF ligado\n");
		enviar_msg_to_host("Testando o testConection");
		mpu_initialize(); //inicia dispositivo
}
		uint8_t mybyte;
		uint8_t mybits;
void main(void) {
    setup();
		enviar_msg_to_host_hex(0xA0);
    while(1){
        if(!S1){ //se foi apertado o sinal e o led esta desativado
					enviar_msg_to_host("s1\n");
					i2c_mpu_readByte(MPU_endereco, MPU6050_RA_WHO_AM_I, &mybyte);
					enviar_msg_to_host_hex(&mybyte);
					delay_ms(100);
					while(!S1);
					delay_ms(100);
        }
        if(!S2){
					enviar_msg_to_host("s2\n"); 
					i2c_mpu_readBits(MPU_endereco, MPU6050_RA_WHO_AM_I, MPU6050_WHO_AM_I_BIT, MPU6050_WHO_AM_I_LENGTH, &mybits);
					enviar_msg_to_host_hex(&mybits);
					LEDVM = !LEDVM;
					delay_ms(100);
					while(!S2);
					delay_ms(100);
        }
        if(newPayload){
            //verifica se o sinal eh direficionado para mim
					if(rx_buf[0] == MY_SUB_ADDR){
						switch(rx_buf[1]){
							case Sinal_request_data:
										enviar_msg_to_host("s1\n"); 
										break;
							case Sinal_LEDS:
										enviar_msg_to_host("w2\n"); 
										LEDVM = !LEDVM;
										break;
						}
					}
					sta = 0;
					newPayload = 0;
        }
		}
}
void luzes_iniciais(void){
				LEDVM = 0;
        delay_ms(1000);
        LEDVM = 1;
        delay_ms(1000);
        LEDVM = 0;
        delay_ms(1000);
        LEDVM = 1;
        delay_ms(1000);
        LEDVM = 0;
}

void enviar_msg_to_host(char *msg_to_send){
	unsigned int i;
    tx_buf[0] = SIGNAL_SENSOR_MSG;
	i = 1;
	while(*msg_to_send != 0){
		tx_buf[i++] = (*msg_to_send++);
		if(i>=TX_PLOAD_WIDTH){
			break;
		}
	}
	TX_Mode_NOACK(i);
	RX_Mode();
	delay_ms(10);
}

void enviar_msg_to_host_hex(char *msg_to_send){
    unsigned int i;
    tx_buf[0] = UART_HEX_PRINT_FLAG;
    i = 1;
    while(*msg_to_send != 0){
        tx_buf[i++] = (*msg_to_send++);
        if(i>=TX_PLOAD_WIDTH){
            break;
        }
    }
    TX_Mode_NOACK(i);
    RX_Mode();
    delay_ms(10);
}

//interrupção o I2C
void I2C_IRQ (void) interrupt INTERRUPT_SERIAL{
	I2C_IRQ_handler();
}
