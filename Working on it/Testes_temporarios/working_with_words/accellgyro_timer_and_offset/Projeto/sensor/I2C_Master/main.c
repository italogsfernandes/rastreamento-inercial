#include "dmp.h"

#include "nrf24le1.h"
#include "hal_w2_isr.h"
#include "hal_delay.h"
#include "stdint.h"
#include "reg24le1.h" //Definiï¿½ï¿½es de muitos endereï¿½os de registradores.
#include "stdbool.h" //Booleanos
#include "API.h"
#include <nRF-SPIComands.h>
#include <pacotes_inerciais.h>

//Subendere?os usados no sistema
#define MY_SUB_ADDR 0x01
#define OTHER_SUB_ADDR 0x02
//Sinais utilizados na comunicacao via RF
#define Sinal_request_data 0x0A
#define Sinal_LEDS 0x0B

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
		send_packet_to_host(UART_PACKET_TYPE_STRING,"Sensor Ligado",13);delay_ms(10);
		mpu_initialize(); //inicia dispositivo
		send_packet_to_host(UART_PACKET_TYPE_STRING,"Testando a conexao I2C",22);delay_ms(10);
		if(mpu_testConnection()){
			send_packet_to_host(UART_PACKET_TYPE_STRING,"Conectado com sucesso",21);delay_ms(10);
		} else {
			send_packet_to_host(UART_PACKET_TYPE_STRING,"Erro na conexao",15);delay_ms(10);
		}
}
bool mybyte = false;
bool mybits = false;
void main(void) {
    setup();
    while(1){
        if(!S1){ //se foi apertado o sinal e o led esta desativado
					send_packet_to_host(UART_PACKET_TYPE_STRING,"B1",2);delay_ms(10);
					i2c_mpu_readBits(MPU_endereco,MPU6050_RA_WHO_AM_I, MPU6050_WHO_AM_I_BIT, MPU6050_WHO_AM_I_LENGTH, buffer);
					send_packet_to_host(UART_PACKET_TYPE_BIN,buffer,1);delay_ms(10);
					mybits = (buffer[0] == 0x34);
					mybyte = mpu_testConnection();
					send_packet_to_host(UART_PACKET_TYPE_HEX,&mybits,1);delay_ms(10);
					send_packet_to_host(UART_PACKET_TYPE_HEX,&mybyte,1);delay_ms(10);
					delay_ms(100);
					while(!S1);
					delay_ms(100);
        }
        if(!S2){
					send_packet_to_host(UART_PACKET_TYPE_STRING,"B2",2);delay_ms(10);
					i2c_mpu_readBits(MPU_endereco,MPU6050_RA_WHO_AM_I, MPU6050_WHO_AM_I_BIT, MPU6050_WHO_AM_I_LENGTH, buffer);
					send_packet_to_host(UART_PACKET_TYPE_BIN,buffer,1);delay_ms(10);
					mybits = (buffer[0] == 0x34);
					send_packet_to_host(UART_PACKET_TYPE_HEX,&mybits,1);delay_ms(10);
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
										break;
							case Sinal_LEDS:
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
//interrupção o I2C
void I2C_IRQ (void) interrupt INTERRUPT_SERIAL{
	I2C_IRQ_handler();
}
