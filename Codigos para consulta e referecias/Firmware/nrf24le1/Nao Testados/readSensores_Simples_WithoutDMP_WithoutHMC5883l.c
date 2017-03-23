/*
Referencias:  https://devzone.nordicsemi.com/question/38470/nrf24le1-i2c-pin-resistor/
https://devzone.nordicsemi.com/question/1975/2-wire-communication-between-two-nrf24le1-coding-problem/
https://devzone.nordicsemi.com/attachment/5d8c9a0f67d0eabc11a703deff9e5b03
*/
//Biblioteca para I/O no nRF24Le1
#include "nrf24le1.h"

//Bibliotecas do exemplo de I2C
#include <hal_w2_isr.h> //Biblioteca para o I2C. TODO: Selecionar entre essa e a outra disponivel (hal_w2)
#include "hal_delay.h"

//Biblioteca do exemplo de transmissão:
//TODO: isso vai dar imcompatibilidade com a delay? é necessaria essa?
#include "hal_clk.h" // Contem funções de clock
//#include &lt;stdint.h&gt; //TODO: Porque usou? standard integers library
#include "hal_nrf.h" //Comunicação wireless
//#include &lt;stdbool.h&gt; //TODO: Porque usou? standard boolean library

//variaveis globais
static bool volatile radio_busy;


//Endereco dos sensores
#define HMC_endereco 0x1E
#define MPU_endereco 0x68

//TODO: O que é isso aqui? Por que é necessario declara-la?
void delay_timer0(void);

//TODO: Verificar se esta tipagem esta correta
int16_t Xma, Yma, Zma; //Magnetometro
int16_t Xac, Yac, Zac; //Acererometro
int16_t Xgi, Ygi, Zgi; //Giroscopio

void main(){
  setupI2C();
  setupRadio();
  Acordar_MPU();
  while (1) {
      requisitarDadosMPU();
      enviarDados();
  }
}

void setupI2C(){
  //INICIA os pinos GPIO
  P0DIR = 0x00;
  P1DIR = 0xFF;
  P2DIR = 0xFF;
  P3DIR = 0xFF;

  //Inicia o I2C
  hal_w2_configure_master(HAL_W2_400KHZ);
  //TODO: O que esse EA faz? deixei o radio para defini-lo
  //EA = 1;
}

void setupRadio(){
     uint8_t payload[3]; //TODO: O que é isso? payload to be transmitted
     #ifdef MCU_NRF24LE1
     while(hal_clk_get_16m_source() != HAL_CLK_XOSC16M){
       // Aguardando o oscilador. 16 MHz
     }
     #endif

     #ifdef MCU_NRF24LU1P
     //Ativando SPI do radio
     //TODO: Pq a diferença com o RX?
     RFCTL = 0x10U;
     #endif

     // Ativa o clock do radio
     //TODO: Por que essa diferença com o RX?
     RFCKEN = 1U;
     // Ativa a interrupção do RF
     RF = 1U;
     // Ativa a interrupção global
     EA = 1U;

     // Ligando a energia do radio
     hal_nrf_set_power_mode(HAL_NRF_PWR_UP);

}

//TODO: Ué? Por que to usando? vou usar isso ou a outra biblioteca?
void I2C_IRQ (void) interrupt INTERRUPT_SERIAL{
	I2C_IRQ_handler();
}

void Acordar_MPU(){
    /*
        Passos para acordar a MPU6050
        Somente para ler acelerometro e giroscopio sem usar a DMP
        Escrever: 0x6B (setar o register)
        Escrever: 0x00 (Acoradar a MPU)
    */
    //sizeof(0x6B) = 1?
    hal_w2_write(MPU_endereco,0x6B, 1); //PWR_MGMT_1
    hal_w2_write(MPU_endereco,0x00, 1); //Acordando a MPU-6050
}

void Acordar_HMC(){
    /*
        Passos para acordar a HMC5883
        Escrever: 0x02 (setar o register)
        Escrever: 0x00 (Falar para medir continuamente)
    */
    //TODO: sizeof(0x6B) = 1?
    hal_w2_write(HMC_endereco,0x02, 1); //Setando o register
    hal_w2_write(HMC_endereco,0x00, 1); //Avisando para medir continuamente
}

void requisitarDadosMPU(){
    /*
            Le os dados que estão na MPU6050
            Eles são armazenados em variaveis globais
            Para isso é necessario:
                Escolher o register para a leitura
                Ler pacotes de 8 bits e junta-los em inteiros de 16 bits
    */
    //TODO: Isso la vai dar certo? será?

    //Lendo Aceleração:
    hal_w2_write(MPU_endereco,0x3B, 1); //começando com o register 0x3B (ACCEL_XOUT_H)
    //0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L):
    hal_w2_read(MPU_endereco, &Xac, 2);
    //0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L):
    hal_w2_read(MPU_endereco, &Yac, 2);
    //0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L):
    hal_w2_read(MPU_endereco, &Zac, 2);

    //Lendo Giroscopio:
    hal_w2_write(MPU_endereco,0x42, 1); //começando com o register 0x43 (GYRO_XOUT_H)
    //0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L):
    hal_w2_read(MPU_endereco, &Xgi, 2);
    //0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L):
    hal_w2_read(MPU_endereco, &Ygi, 2);
    //0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L):
    hal_w2_read(MPU_endereco, &Zgi, 2);
}

void requisitarDadosHMC(){
    /*
            Le os dados que estão no HMC5883
            Eles são armazenados em variaveis globais
            Para isso é necessario:
                Escolher o register para a leitura
                Ler pacotes de 8 bits e junta-los em inteiros de 16 bits
    */
    //TODO: Isso la vai dar certo? será? ja foi testada a MPU?

    //Lendo Magnetometro:
    hal_w2_write(HMC_endereco,0x03, 1); //Selecionando register para iniciar
    ////MSB x | LSB  x
    hal_w2_read(HMC_endereco, &Xma, 2);
    //MSB z | LSB z
    hal_w2_read(HMC_endereco, &Zma, 2);
    ///MSB y | LSB y
    hal_w2_read(HMC_endereco), &Yma, 2);
}

void enviarDados(){
    //TODO: implementar
}
