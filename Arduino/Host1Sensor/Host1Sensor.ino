/*
   Escrito com base no exemplo GettingStarted da biblioteca RF24 de TMRh20.
   É utilizado o rádio nRF24L01+
*/

//Biblioteca para comunicação SPI(com o radio)
#include <SPI.h>
//Biblioteca especifica para utilizao o radio, disponibilizada por TMRh20
#include "RF24.h"

//Biblioteca para usar Quaternion
#include "helper_3dmath.h"

struct linha_de_dados {
  //Variaveis para armazenamento dos dados lidos
  int16_t ax, ay, az;   //Acelerometro
  int16_t gx, gy, gz;   //Giroscopio
  int16_t mx, my, mz;   //Magnetometro
  Quaternion q;         // [w, x, y, z] Quartenion
};

linha_de_dados data;

//Definição do objeto RF24
//Utiliza o nRF24L01 com SPI nos pinos 7 e 8
RF24 radio(7, 8);

byte addresses[] = "1Node";

void setup() {
  Serial.begin(38400);

  radio.begin();

  // Setando o PA Level para low para evitar problemas com alimentação.
  //Por padrão é definido com RF24_PA_MAX. Em low os dispositivos devem estar proximos
  radio.setPALevel (RF24_PA_LOW);

  //Para abrir a escrita descomente a linha abaixo.
  //radio.openWritingPipe(address);
  radio.openReadingPipe(1, address);

  radio.startListening();
}

void loop() {
  // Somente recebendo e mostrando...
  if (radio.available()) {
    while (radio.available()) {
      radio.read( &data, sizeof(data));
    }
    mostrarDados();
  }
}

void mostrarDados() {
   //Acelerometro:
  Serial.print(data.ax); Serial.print("\t");
  Serial.print(data.ay); Serial.print("\t");
  Serial.print(data.az); Serial.print("\t");
  //Giroscopio:
  Serial.print(data.gx); Serial.print("\t");
  Serial.print(data.gy); Serial.print("\t");
  Serial.print(data.gz); Serial.print("\t");
  //Magnetometro:
  Serial.print(data.mx); Serial.print("\t");
  Serial.print(data.my); Serial.print("\t");
  Serial.print(data.mz); Serial.print("\t");
  //Quaternion:
  Serial.print(data.q.w); Serial.print("\t");
  Serial.print(data.q.x); Serial.print("\t");
  Serial.print(data.q.y); Serial.print("\t");
  Serial.println(data.q.z);
}







