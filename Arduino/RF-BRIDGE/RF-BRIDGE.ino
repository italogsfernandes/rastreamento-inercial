//#define TARGET_UNO
#define TARGET_DUE

#ifdef TARGET_UNO
#include <SoftwareSerial.h>
#endif

/**
   Redireciona uma porta serial criano nos pinos 10 e 11 para a porta serial
   do arduino no computador HOST
   Os dados sao apernar redirecionados byte a byte, eh necessario que sejam
   tratados em um software de leitura

   Conexao:
   pin 10 (Arduino RX) -> P05 (nrf24le1-32pin TX)
   pin 11 (Arduino TX) -> P06 (nrf24le1-32pin RX)
   Arduino 3.3V -> Power
   GND -> GND

   É necessario conectar conversor de nível lógico
   (ou até mesmo um divisor de tensao) entre os pinos
   UART (TX e RX) do arduino  e do nrf24le1-32pin pois este trabalha
   em nível lógico de 3.3V.
*/

#define rf_RX 10 //where the TX cable must be connected TX -> RX in arduino UNO
#define rf_TX 11 //where the RX cable must be connected RX -> TX in arduino UNO
#define rf_baudrate 9600 //baudrate do serial do nrf
#define MAX_WAIT_TIME 10 //tempo maximo de espera dos timeouts em ms
#define UART_START_SIGNAL 0x53
#define STATUS_LED 5

uint8_t packet_len = 0;
uint8_t i = 0;
uint8_t rf_data[32] = {0};
uint8_t serial_data[32] = {0};
uint8_t checkoutbyte = 0;

unsigned long timeout_init_time = 0;

#ifdef TARGET_UNO
SoftwareSerial rfSerial(rf_RX, rf_TX); // RX, TX
#endif

void setup() {
  pinMode(STATUS_LED,OUTPUT);
#ifdef TARGET_UNO
  Serial.begin(rf_baudrate);  // Open serial communications and wait for port to open:
  Serial.println("Arduino Ligado");
  rfSerial.begin(rf_baudrate);
#endif
#ifdef TARGET_DUE
  Serial.begin(rf_baudrate);  // Open serial communications and wait for port to open:
  Serial.println("Arduino Ligado");
  Serial1.begin(rf_baudrate);
#endif
}

void loop() {
  // Read nrf output if available.
#ifdef TARGET_UNO
  if (rfSerial.available()) {
    if (rfSerial.read() == UART_START_SIGNAL) {
      checkoutbyte = 0;
      wait_rfserial_bytes(1);
      packet_len = rfSerial.read();
      wait_rfserial_bytes(packet_len);
      for (i = 0; i < packet_len; i++) {
        rf_data[i] = rfSerial.read();
        checkoutbyte += rf_data[i];
      }
      checkoutbyte = (~checkoutbyte) + 1;
      wait_rfserial_bytes(1);
      if (checkoutbyte ==  rfSerial.read() || true ) {
        Serial.write(UART_START_SIGNAL);
        Serial.write(packet_len);
        for (i = 0; i < packet_len; i++) {
          Serial.write(rf_data[i]);
        }
        Serial.write(checkoutbyte);
      }
    }
  }
  // Read user input if available and send to nrf
  if (Serial.available()) {
    if (Serial.read() == UART_START_SIGNAL) {
      checkoutbyte = 0;
      wait_serial_bytes(1);
      packet_len = Serial.read();
      wait_serial_bytes(packet_len);
      for (i = 0; i < packet_len; i++) {
        serial_data[i] = Serial.read();
        checkoutbyte += serial_data[i];
      }
      checkoutbyte = (~checkoutbyte) + 1;
      wait_serial_bytes(1);
      if (checkoutbyte ==  Serial.read() || true ) {
        rfSerial.write(UART_START_SIGNAL);
        rfSerial.write(packet_len);
        for (i = 0; i < packet_len; i++) {
          rfSerial.write(rf_data[i]);
        }
        rfSerial.write(checkoutbyte);
      }
    }
  }
#endif
#ifdef TARGET_DUE
  if (Serial1.available()) {
    if (Serial1.read() == UART_START_SIGNAL) {
      checkoutbyte = 0;
      wait_rfserial_bytes(1);
      packet_len = Serial1.read();
      wait_rfserial_bytes(packet_len);
      for (i = 0; i < packet_len; i++) {
        rf_data[i] = Serial1.read();
        checkoutbyte += rf_data[i];
      }
      checkoutbyte = (~checkoutbyte) + 1;
      wait_rfserial_bytes(1);
      if (checkoutbyte ==  Serial1.read() || true ) {
        Serial.write(UART_START_SIGNAL);
        Serial.write(packet_len);
        for (i = 0; i < packet_len; i++) {
          Serial.write(rf_data[i]);
        }
        Serial.write(checkoutbyte);
      }
    }
  }
  // Read user input if available and send to nrf
  if (Serial.available()) {
    if (Serial.read() == UART_START_SIGNAL) {
      checkoutbyte = 0;
      wait_serial_bytes(1);
      packet_len = Serial.read();
      wait_serial_bytes(packet_len);
      for (i = 0; i < packet_len; i++) {
        serial_data[i] = Serial.read();
        checkoutbyte += serial_data[i];
      }
      checkoutbyte = (~checkoutbyte) + 1;
      wait_serial_bytes(1);
      if (checkoutbyte ==  Serial.read()) {
        Serial1.write(UART_START_SIGNAL);
        Serial1.write(packet_len);
        for (i = 0; i < packet_len; i++) {
          Serial1.write(rf_data[i]);
        }
        Serial1.write(checkoutbyte);
      }
    }
  }
#endif
}

void wait_serial_bytes(uint8_t how_many) {
  timeout_init_time = millis();
  while (Serial.available() < how_many && millis() - timeout_init_time < MAX_WAIT_TIME * how_many);
}

void wait_rfserial_bytes(uint8_t how_many) {
#ifdef TARGET_UNO
  timeout_init_time = millis();
  while (rfSerial.available() < how_many && millis() - timeout_init_time < MAX_WAIT_TIME * how_many);
#endif
#ifdef TARGET_DUE
  timeout_init_time = millis();
  while (Serial1.available() < how_many && millis() - timeout_init_time < MAX_WAIT_TIME * how_many);
#endif
}
