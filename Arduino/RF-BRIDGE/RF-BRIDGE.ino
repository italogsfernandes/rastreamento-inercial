#define TARGET_UNO
//#define TARGET_DUE

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

#define STATUS_LED 5

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
    Serial.write(rfSerial.read());
  }
  // Read user input if available and send to nrf
  if (Serial.available()) {
   rfSerial.write(Serial.read());
  }
#endif
#ifdef TARGET_DUE
  if (Serial1.available()) {
   Serial.write(Serial1.read());
  }
  // Read user input if available and send to nrf
  if (Serial.available()) {
    Serial1.write(Serial.read());
  }
#endif
}

