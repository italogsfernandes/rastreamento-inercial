#ifndef PACOTES_INERCIAIS_H
/* Se a biblioteca mpu.h não for definida, defina-a.
Verificar é preciso para que não haja varias chamadas da
mesma biblioteca. */
#define	PACOTES_INERCIAIS_H

//Subenderecos usados no sistema
#define HOST_SUB_ADDR 0xFF //Sub addr do host

//UART Packet: Start Signal - Command
#define UART_START_SIGNAL  0x53

/////////////
//Comandos //
/////////////
#define CMD_OK  0x00 //Ack - Uart Command
#define CMD_ERROR 0x01 //Error flag - Uart Command
#define CMD_START 0x02 //Start Measuring - Uart Command
#define CMD_STOP  0x03 //Stop Measuring - Uart Command
#define CMD_CONNECTION  0x04 //Teste Connection - Uart Command
#define CMD_CALIBRATE 0x05 //Calibrate Sensors Command
#define CMD_DISCONNECT 0x06 //Some sensor has gone disconected
#define CMD_READ 0x07 //Request a packet of readings
#define CMD_SET_PACKET_TYPE 0x08
#define CMD_GET_ACTIVE_SENSORS 0x09 //Retorna a variavel do host active sensors
#define CMD_SET_ACTIVE_SENSORS 0x0A //Altera a variavel do host active sensors
#define CMD_TEST_RF_CONNECTION 0x0B
#define CMD_LIGHT_UP_LED 0x0C
#define CMD_TURN_OFF_LED 0x0D



#endif
