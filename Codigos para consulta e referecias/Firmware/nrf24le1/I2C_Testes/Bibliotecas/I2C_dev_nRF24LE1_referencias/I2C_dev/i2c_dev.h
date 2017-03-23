#ifndef I2C_DEV_
#define I2C_DEV_

#include "reg24le1.h" //registrados do nrf24le1
#include "stdbool.h" //booleanos padroes
#include "intrins.h" //?


//definições de atalhos
//envia condição de parada:
#define xStop()                 W2CON0|=0x20
//envia condição de inicio:
#define xStart()                W2CON0|= 0x10
//define a frequencia como 100kHz:
#define FREQ_STANDART_MODE()    W2CON0&=0xF3; W2CON0|=0x04
//define a frequencia como 400kHz:
#define FREQ_FAST_MODE()        W2CON0&=0xF3; W2CON0|=0x08
//seleciona o modo master:
#define masterSelect()          W2CON0|=0x02
//seleciona o modo slave, nao sera utilizado:
#define slaveSelect()           W2CON0 &=0x02
//ativa o i2c:
#define wire2Enable()           W2CON0|=0x01
//desativa o i2c:
#define wire2Disable()          W2CON0&=0xFE
//0: Enable all interrupts (not masked otherwise):
#define maskIrq_irqOn()         W2CON1&=0xDF
//1: Disable all interrupts:
#define maskIrq_irqOff()        W2CON1|=0x20
//le W2CON1 para verificar se o ACK foi recebido:
#define ACK()                   (W2CON1&0X02)
 //le W2CON1 para verificar se os dados estão prontos para serem lidos:
#define dataReady()             (W2CON1&0X01)
//retorna o byte de status:
#define getStatus()             W2CON1


#endif
