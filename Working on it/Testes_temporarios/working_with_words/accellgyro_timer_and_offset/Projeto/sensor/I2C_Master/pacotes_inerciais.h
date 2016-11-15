#ifndef PACOTES_INERCIAIS_H
/* Se a biblioteca mpu.h não for definida, defina-a.
Verificar é preciso para que não haja varias chamadas da
mesma biblioteca. */
#define	PACOTES_INERCIAIS_H

#include <uart_basics.h>
#include <nRF-SPIComands.h>

#define UART_START_SIGNAL  0x53
#define UART_END_SIGNAL  0x04

#define UART_PACKET_TYPE_ACEL      0x01
#define UART_PACKET_TYPE_GIRO      0x02
#define UART_PACKET_TYPE_MAG       0x03
#define UART_PACKET_TYPE_M6        0x04
#define UART_PACKET_TYPE_M9        0x05
#define UART_PACKET_TYPE_QUAT      0x06
#define UART_PACKET_TYPE_FIFO_NO_MAG       0x07
#define UART_PACKET_TYPE_FIFO_ALL_READINGS     0x08
#define UART_PACKET_TYPE_STRING    0x09
#define UART_PACKET_TYPE_HEX       0x0A
#define UART_PACKET_TYPE_BIN       0x0B
#define UART_PACKET_TYPE_UINT16    0x0C
#define UART_PACKET_TYPE_INT16     0x0D
#define UART_PACKET_TYPE_FLOAT16       0x0E


#define sensor_id 0x02


//Packet Type | Sensor id |  ... | data | ... |
void send_packet_to_host(uint8_t packet_type, uint8_t *data_to_send, uint8_t data_len){
	unsigned int i;
  tx_buf[0] = packet_type;
	tx_buf[1] = sensor_id;
	for(i=2; i<data_len+2; i++){
		tx_buf[i] = data_to_send[i-2];
	}
	TX_Mode_NOACK(data_len+2);
	RX_Mode();
}


//Start signal | Packet Type | Packet Length | ... | data | ... | End signal
void send_packet_to_computer(uint8_t packet_type, uint8_t *data_to_send, uint8_t data_len){
	unsigned int i;
    uart_putchar(UART_START_SIGNAL);
    uart_putchar(packet_type);
    uart_putchar(data_len);
		for(i=1; i<data_len+1; i++){
			uart_putchar(data_to_send[i]);
		}
	uart_putchar(UART_END_SIGNAL);
}

//Start signal | Packet Type | Packet Length | ... | data | ... | End signal
void send_packet_from_host_to_computer(uint8_t packet_type, uint8_t *data_to_send, uint8_t data_len){
	unsigned int i;
    uart_putchar(UART_START_SIGNAL);
    uart_putchar(packet_type);
    uart_putchar(data_len+1);
		uart_putchar(sensor_id);
	for(i=0; i<data_len; i++){
		uart_putchar(data_to_send[i]);
	}
	uart_putchar(UART_END_SIGNAL);
}


#endif
