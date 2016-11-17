#include "nrf24le1.h"
#include "hal_w2_isr.h"
#include "hal_delay.h"
#include "stdint.h"
#include "reg24le1.h" //Definiï¿½ï¿½es de muitos endereï¿½os de registradores.
#include "stdbool.h" //Booleanos
#include "API.h"
#include <simple_timer.h>
#include <nRF-SPIComands.h>
#include <pacotes_inerciais.h>
#include <dmp.h>

//Subendere?os usados no sistema
#define MY_SUB_ADDR 0x01
#define OTHER_SUB_ADDR 0x02

//Sinais utilizados na comunicacao via RF
#define Sinal_request_data 0x0A
#define Sinal_LEDS 0x0B

uint8_t xdata packet_motion6[12]; //xac,yac,zac,xgy,ygy,zgy
int16_t xdata packet_quat[4];
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

/***VARIAVEIS da DMP*****/

volatile bool mpuInterrupt = false;
bool dmpReady = false;  // set true if DMP init was successful
uint8_t xdata mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t xdata devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t xdata packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t xdata fifoCount;     // count of all bytes currently in FIFO
uint8_t xdata fifoBuffer[64]; // FIFO storage buffer

/************************/
/***FUNCOES DA DM*******/
void pin_isr_setup(){
	EX0=1;
	INTEXP = 0x10;
	IT0 = 1;
}
void ext0_irq(void) interrupt 0 
{
    mpuInterrupt=true;
}

/************************/



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
		//pin_isr_setup();
    EA=1; luzes_iniciais(); //Enable All interrupts, e pisca luzes
		send_packet_to_host(UART_PACKET_TYPE_STRING,"Sensor Ligado",13);delay_ms(10);
		mpu_8051_malloc_setup();
		mpu_initialize(); //inicia dispositivo
		send_packet_to_host(UART_PACKET_TYPE_STRING,"Testando a conexao I2C",22);delay_ms(10);
		if(mpu_testConnection()){
			send_packet_to_host(UART_PACKET_TYPE_STRING,"Conectado com sucesso",21);delay_ms(10);
		} else {
			send_packet_to_host(UART_PACKET_TYPE_STRING,"Erro na conexao",15);delay_ms(10);
		}
		send_packet_to_host(UART_PACKET_TYPE_STRING,"OFFSETS",7);delay_ms(10);
		setXAccelOffset(-3100);setYAccelOffset(392);setZAccelOffset(1551);
		setXGyroOffset(-28);setYGyroOffset(6);setZGyroOffset(60);
		//send_packet_to_host(UART_PACKET_TYPE_STRING,"CONFIG_EXT",10);delay_ms(10);
		
}
void configura_dmp(){
		delay_ms(250);//esperando nao sei pq
		send_packet_to_host(UART_PACKET_TYPE_STRING,"INICIALIZAR_DMP",15);delay_ms(10);
		devStatus = dmpInitialize();
		send_packet_to_host(UART_PACKET_TYPE_STRING,"OFFSETS_DMP",11);delay_ms(10);
		setXAccelOffset(-3100);setYAccelOffset(392);setZAccelOffset(1551);
		setXGyroOffset(-28);setYGyroOffset(6);setZGyroOffset(60);
		if (devStatus == 0) {
				send_packet_to_host(UART_PACKET_TYPE_STRING,"devStatus=OK",12);delay_ms(10);
        setDMPEnabled(true);
				send_packet_to_host(UART_PACKET_TYPE_STRING,"MPU_INTSTATUS",13);delay_ms(10);
        mpuIntStatus = getIntStatus();
				send_packet_to_host(UART_PACKET_TYPE_INT8,&mpuIntStatus,1);delay_ms(10);
        dmpReady = true;
				send_packet_to_host(UART_PACKET_TYPE_STRING,"PACKET_SIZE",11);delay_ms(10);
        packetSize = dmpGetFIFOPacketSize();
				send_packet_to_host(UART_PACKET_TYPE_UINT16,(uint8_t *)&packetSize,2);delay_ms(10);
			
    } else {
        send_packet_to_host(UART_PACKET_TYPE_STRING,"devStatus=Falha",15);delay_ms(10);
    }
}

void ler_dmp(){
	send_packet_to_host(UART_PACKET_TYPE_STRING,"AGUARDA_INT",11);delay_ms(10);
	LEDVM=0;
	mpuInterrupt = true;
	while (!mpuInterrupt && fifoCount < packetSize) {
     LEDVM = 1;
  }
	LEDVM=0;
	send_packet_to_host(UART_PACKET_TYPE_STRING,"INT_RECEIVED",12);delay_ms(10);
	// reset interrupt flag and get INT_STATUS byte
	mpuInterrupt = true;
	mpuIntStatus = getIntStatus();

	// get current FIFO count
	fifoCount = getFIFOCount();
	send_packet_to_host(UART_PACKET_TYPE_STRING,"FIFO_COUNT",10);delay_ms(10);
	send_packet_to_host(UART_PACKET_TYPE_UINT16,(uint8_t *)&fifoCount,2);delay_ms(10);
	send_packet_to_host(UART_PACKET_TYPE_STRING,"INT_STATUS",10);delay_ms(10);
	send_packet_to_host(UART_PACKET_TYPE_HEX,&mpuIntStatus,1);delay_ms(10);
	// check for overflow (this should never happen unless our code is too inefficient)
	if ((mpuIntStatus & 0x10) || fifoCount == 1024) {
			
			// reset so we can continue cleanly
			resetFIFO();
			send_packet_to_host(UART_PACKET_TYPE_STRING,"FIFO overflow!",14);delay_ms(10);
    // otherwise, check for DMP data ready interrupt (this should happen frequently)
    } else if (mpuIntStatus & 0x02) {
				send_packet_to_host(UART_PACKET_TYPE_STRING,"OK_READING",10);delay_ms(10);
        // wait for correct available data length, should be a VERY short wait
        while (fifoCount < packetSize) fifoCount = getFIFOCount();

        // read a packet from FIFO
        getFIFOBytes(fifoBuffer, packetSize);
        
        // track FIFO count here in case there is > 1 packet available
        // (this lets us immediately read more without waiting for an interrupt)
        fifoCount -= packetSize;
	}
}
void main(void) {
    setup();
    while(1){
        if(!S1){ //se foi apertado o sinal e o led esta desativado
					send_packet_to_host(UART_PACKET_TYPE_STRING,"B1",2);delay_ms(10);
					send_packet_to_host(UART_PACKET_TYPE_STRING,"CONFIGURAR",10);delay_ms(10);
					configura_dmp();
					send_packet_to_host(UART_PACKET_TYPE_STRING,"CONFIGURADO",11);delay_ms(10);
					delay_ms(100);
					while(!S1);
					delay_ms(100);
        }
        if(!S2){
					send_packet_to_host(UART_PACKET_TYPE_STRING,"B2",2);delay_ms(10);
					send_packet_to_host(UART_PACKET_TYPE_STRING,"LER",3);delay_ms(10);
					ler_dmp();
					send_packet_to_host(UART_PACKET_TYPE_STRING,"ENVIAR",6);delay_ms(10);
					dmpGetQuaternion_int16(packet_quat, fifoBuffer);
					send_packet_to_host(UART_PACKET_TYPE_QUAT,(uint8_t *) packet_quat,8);
					//LEDVM = !LEDVM;
					delay_ms(100);
					while(!S2);
					delay_ms(100);
        }
        if(newPayload){
            //verifica se o sinal eh direficionado para mim
					if(rx_buf[0] == MY_SUB_ADDR){
						switch(rx_buf[1]){
							case Sinal_request_data:
										send_packet_to_host(UART_PACKET_TYPE_STRING,"On",2);delay_ms(10);
										start_T0();
										break;
							case Sinal_LEDS:
										stop_T0();
										send_packet_to_host(UART_PACKET_TYPE_STRING,"Off",3);delay_ms(10);
										LEDVM = !LEDVM;
										break;
						}
					}
					sta = 0;
					newPayload = 0;
        }
        //timer tick
				if(timer_flag <= 0){
          //getMotion6_packet(packet_motion6);
					//send_packet_to_host(UART_PACKET_TYPE_M6,packet_motion6,12);
	
					dmpGetQuaternion_int16(packet_quat, fifoBuffer);
					//send_packet_to_host(UART_PACKET_TYPE_QUAT,(uint8_t *) packet_quat,8);
					send_packet_to_host(UART_PACKET_TYPE_HEX,(uint8_t *) packet_quat,8);
					
					timer_flag = 1;
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
