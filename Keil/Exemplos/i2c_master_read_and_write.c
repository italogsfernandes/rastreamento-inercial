/*
Referencias:  https://devzone.nordicsemi.com/question/38470/nrf24le1-i2c-pin-resistor/
https://devzone.nordicsemi.com/question/1975/2-wire-communication-between-two-nrf24le1-coding-problem/
https://devzone.nordicsemi.com/attachment/5d8c9a0f67d0eabc11a703deff9e5b03
*/

#include "nrf24le1.h"
#include <hal_w2_isr.h>
#include "hal_delay.h"

uint8_t flag = 0;

//TODO: O que é isso?
void delay_timer0(void);

void main(){
  //TODO Alterar estes dados:
  uint8_t i;
  xdata uint8_t data_string[100];

  //Indica os processos de inicialização com o LED1

  //INICIA os pinos GPIO
  P0DIR = 0x00;
  P1DIR = 0xFF;
  P2DIR = 0xFF;
  P3DIR = 0xFF;

  //Inicia o I2C
  hal_w2_configure_master(HAL_W2_400KHZ);
  EA = 1;
  //inicia os dados
  data_string [0] = 0;
  i = 0;

  while (1) {
    //armazena data_string[0] para verificação
    flag = data_string[0];
    //Enviando 100 bytes identicos para o slave
    //P0=0xFF
    //TODO: 0x07 é o endereço do slave?
    hal_w2_write(0x07, data_string, 100);
    //Lendo 100 bytes do slave (apos serem incemetados 1 nele)
    //P0=0xAA; //TODO: Ué?
    hal_w2_read(0x07, data_string, 100);

    //opcional: output para o byte
    P0 = data_string[0];

    //verificando
    flag++;
    for (i = 0; i<99; i++) {
      if (data_string[i] != flag) { //se ficou diferente
        P0 = 0xAA; while(1);//TODO: UAI?
      }
    }
  }
}

//TODO: Ué?
void I2C_IRQ (void) interrupt INTERRUPT_SERIAL{
	I2C_IRQ_handler();
}
