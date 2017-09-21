/* ------------------------------------------------------------------------------
   FEDERAL UNIVERSITY OF UBERLANDIA
   Faculty of Electrical Engineering
   Biomedical Engineering Lab
   Uberlândia, Brazil
   ------------------------------------------------------------------------------
   Authors:
      Andrei Nakagawa, MSc
      Ítalo Fernandes
      Ana Carolina Torres Cresto
   contact: nakagawa.andrei@gmail.com
   URLs: www.biolab.eletrica.ufu.br
         https://github.com/BIOLAB-UFU-BRAZIL
   ------------------------------------------------------------------------------
   Description:
   ------------------------------------------------------------------------------
   Acknowledgements
    - We would like to thank the open-source community that provided many of the
    source codes necessary for creating this firmware.
    - Jeff Rowberg as the main developer of the I2Cdevlib
    - Luis Ródenas: Our calibration routine is totally based on his code
   ------------------------------------------------------------------------------
*/

/* ==========  LICENSE  ==================================
  I2Cdev device library code is placed under the MIT license
  Copyright (c) 2011 Jeff Rowberg
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
  =========================================================
*/
//---------------------------------------------------------------------------
#include "I2Cdev.h"
#include "MPU6050.h"
#include "HMC5883L.h"
#include "Timer.h"
#include "SoftwareSerial.h"
#include "madgwick.h"
//---------------------------------------------------------------------------
// Arduino Wire library is required if I2Cdev I2CDEV_ARDUINO_WIRE implementation
// is used in I2Cdev.h
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
#include "Wire.h"
#endif
//---------------------------------------------------------------------------
#define saidaC 4 // SEL2
#define saidaB 3 // SEL1
#define saidaA 2 // SEL0
#define LED_PIN 13
#define sampFreq 100
#define PSDMP 42
#define ST '$'
#define ET '\n'
//---------------------------------------------------------------------------
MPU6050 mpu(0x68);
HMC5883L mag;
//---------------------------------------------------------------------------
/*const int numSensors = 1;
const int* offsets;
const int offsets1[6] = { -1243, -30, 464, 87, -27, 25};
const int offsets2[6] = { -2892, 359, 1616, -24, -7, 40};
const int offsets3[6] = { -231, 722, 906, 16, -19, 26};
const int offsets4[6] = { -588, 489, 1691, 144, 49, 35};
const int offsets5[6] = { -814, 2909, 1258, 16, 110, 34};
*/
const int numSensors = 5;
const int* offsets;
const int offsets0[6] = { -1042, -881, 1211, 12, -45, -27}; // offsets para teste com gy-521 ---> { -1275, -70, 495, 87, -33, 25}; // offsets para o sistema final --> { -998, -883, 1276, 10, -48, -28};
const int offsets1[6] = { 2538, -1807, 1525, 48, -15, -8}; // { 3217, -1849, 1713, 47, -18, -4};
const int offsets2[6] = { -615, -1181, 1134, -78, -55, -17};
const int offsets3[6] = { 1329, 902, 1159, -3, -36, 32}; //{ -133, 1107, 3469, -3, -35, 35}; //{ 996, 920, 1222, -3, -34, 35}; // { 2086, 1218, 1306, -5, -36, 35};
const int offsets4[6] = { -1704, 257, 1061, 28, -41, -14}; // { -2276, 382, 1140, 31, -40, -29};
const int magOffsets0[3] = {43, 73, -48};
const int magOffsets1[3] = {48, -19, -82};
const int magOffsets2[3] = {17, 23, -84};
const int magOffsets3[3] = {38, -116, -10};
const int magOffsets4[3] = {51, -10, -8};
/*const int magOffsets0[3] = {44, 63, -56};
const int magOffsets1[3] = {56, -33, -86};
const int magOffsets2[3] = {52, 14, -58};
const int magOffsets3[3] = {34, -141, -42};
const int magOffsets4[3] = {58, -33, -63};
*/
//ficaram bons!
/*const int magOffsets0[3] = {51, 122, -17};
const int magOffsets1[3] = {63, 24, -49};
const int magOffsets2[3] = {57, 67, -20};
const int magOffsets3[3] = {65, -92, -25};
const int magOffsets4[3] = {78, 15, -41};*/
//---------------------------------------------------------------------------
//calibration matrix for accelerometer
float calibMatrix[12] = {0.998, -0.032, 0.037, -0.013, -0.995, -0.043, -0.035, -0.035, 0.993, -10744.59, 583.77, 3740.22};
float cAcc[3] = {0,0,0};
//---------------------------------------------------------------------------
//madgwick parameters
//TODO: Beta should be different for each sensor
float GyroMeasError = PI * (40.0f / 180.0f);
float beta = sqrt(3.0f / 4.0f) * GyroMeasError;   // compute beta
float* quat;
float quat0[4] = {1.0f,0.0f,0.0f,0.0f};
float quat1[4] = {1.0f,0.0f,0.0f,0.0f};
float quat2[4] = {1.0f,0.0f,0.0f,0.0f};
float quat3[4] = {1.0f,0.0f,0.0f,0.0f};
float quat4[4] = {1.0f,0.0f,0.0f,0.0f};
float eul[3] = {0,0,0};
float deg[3] = {0,0,0};
//---------------------------------------------------------------------------
uint8_t* fifoBuffer; // FIFO storage fifoBuffer of mpu
uint8_t fb1[42];
uint8_t fb2[42];
uint8_t fb3[42];
uint8_t fb4[42];
uint8_t fb5[42];
int16_t ax, ay, az; //accel
int16_t accel[3] = {0,0,0};
int16_t gx, gy, gz; //gyro
int16_t mx, my, mz; //mag
//---------------------------------------------------------------------------
uint16_t fifoCount;     // count of all bytes currently in FIFO
int numbPackets;
bool is_alive = false;
bool running_coleta = false;
bool led_state = LOW;
//---------------------------------------------------------------------------
Timer t;
void takereading();
//---------------------------------------------------------------------------
void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(saidaC, OUTPUT);  pinMode(saidaB, OUTPUT);  pinMode(saidaA, OUTPUT);
  Serial.begin(115200);

#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
  Wire.begin();
  Wire.setClock(400000);
#elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
  Fastwire::setup(400, true);
#endif

  offsets = (int*)malloc(sizeof(int) * numSensors * 6);
  memcpy(offsets, offsets0, sizeof(int) * 6);
  memcpy(offsets + 6, offsets1, sizeof(int) * 6);
  memcpy(offsets + 12, offsets2, sizeof(int) * 6);
  memcpy(offsets + 18, offsets3, sizeof(int) * 6);
  memcpy(offsets + 24, offsets4, sizeof(int) * 6);
  for (int i = 0; i < numSensors; i++)
  {
    initializeSensor(i);
  }
  for (int i = 0; i < numSensors; i++)
  {
    verificaSensor(i);
  }
  Serial.println("Pronto para iniciar, aguardando comando serial.");
  digitalWrite(saidaA, LOW);
  digitalWrite(saidaB, LOW);
  digitalWrite(saidaC, LOW);
  //while (!Serial.available());

  t.every(10,takereading); //chama a cada 10ms = 1000/20

}
//---------------------------------------------------------------------------
void loop() {
  t.update();
}
//---------------------------------------------------------------------------
void takereading() {
  Serial.write(0x7F);
  for (int i = 0; i < numSensors; i++)
  {
    quat = readSensor(i);    
    send_serial_packet(quat);
    //Serial.print(String(quat[0]) + " " + String(quat[1]) + " " + String(quat[2]) + " " + String(quat[3]) + "\n" );    
  }

  Serial.write(0x7E);
  digitalWrite(LED_PIN, led_state);
  led_state = !led_state;
}

void select_sensor(int sensor) {
  switch (sensor) {
    case 0:
      digitalWrite(saidaA, 0);
      digitalWrite(saidaB, 0);
      digitalWrite(saidaC, 0);
      break;
    case 1:
      digitalWrite(saidaA, 0);
      digitalWrite(saidaB, 0);
      digitalWrite(saidaC, 1);
      break;
    case 2:
      digitalWrite(saidaA, 0);
      digitalWrite(saidaB, 1);
      digitalWrite(saidaC, 0);
      break;
    case 3:
      digitalWrite(saidaA, 0);
      digitalWrite(saidaB, 1);
      digitalWrite(saidaC, 1);
      break;
    case 4:
      digitalWrite(saidaA, 1);
      digitalWrite(saidaB, 0);
      digitalWrite(saidaC, 0);
      break;
    case 5:
      digitalWrite(saidaA, 1);
      digitalWrite(saidaB, 0);
      digitalWrite(saidaC, 1);
      break;
    case 6:
      digitalWrite(saidaA, 1);
      digitalWrite(saidaB, 1);
      digitalWrite(saidaC, 0);
      break;
    case 7:
      digitalWrite(saidaA, 1);
      digitalWrite(saidaB, 1);
      digitalWrite(saidaC, 1);
      break;
  }
  delayMicroseconds(10);
}
//---------------------------------------------------------------------------
void send_serial_packet(float* sensorQuaternion)
{
  //Assembling packet and sending via serial
  for(int i=0; i<4; i++)
  {
    uint16_t convQuat = float2uint16(sensorQuaternion[i]);    
    Serial.write(convQuat>>8);
    Serial.write(convQuat&0xFF);
    //Serial.print(String(sensorQuaternion[i]) + " ");
    //Serial.print(String(convQuat) + " ");
  }    
  //Serial.print("\n");
}
//---------------------------------------------------------------------------
float* readSensor(int sensorId)
{
  select_sensor(sensorId);
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  //select_sensor(2);
  mag.getHeading(&mx, &my, &mz);  

  //Compensate accelerometer readings with the calibration matrix
  accel[0]=ax; accel[1]=ay; accel[2]=az;
  compAcc(accel,cAcc);
  
  float fax = (float)(cAcc[0]) / 16384.0f;
  float fay = (float)(cAcc[1]) / 16384.0f;
  float faz = (float)(cAcc[2]) / 16384.0f;
  float fgx = (float)(gx) * (250.0f/32768);
  float fgy = (float)(gy) * (250.0f/32768);
  float fgz = (float)(gz) * (250.0f/32768);
  float* calcQuat = (float*)(malloc(4*sizeof(float)));

  switch(sensorId)
  {
    case 0:
      mag.getHeadingWithOffset(&mx,&my,&mz,magOffsets0);
      for(int i=0; i<5; i++)
        QuaternionUpdate(quat0,ax,ay,az,fgx*(PI/180.0f),fgy*(PI/180.0f),fgz*(PI/180.0f),mx,my,mz,beta,50.0);
      calcQuat=quat0;
      break;
    case 1:
      mag.getHeadingWithOffset(&mx,&my,&mz,magOffsets1);
      for(int i=0; i<5; i++)
        QuaternionUpdate(quat1,ax,ay,az,fgx*(PI/180.0f),fgy*(PI/180.0f),fgz*(PI/180.0f),mx,my,mz,beta,50.0);
      calcQuat=quat1;
      break;
    case 2:
      mag.getHeadingWithOffset(&mx,&my,&mz,magOffsets2);
      for(int i=0; i<5; i++)      
        QuaternionUpdate(quat2,ax,ay,az,fgx*(PI/180.0f),fgy*(PI/180.0f),fgz*(PI/180.0f),mx,my,mz,beta,50.0);
      calcQuat=quat2;
      break;
    case 3:
      mag.getHeadingWithOffset(&mx,&my,&mz,magOffsets3);
      for(int i=0; i<5; i++)
        QuaternionUpdate(quat3,ax,ay,az,fgx*(PI/180.0f),fgy*(PI/180.0f),fgz*(PI/180.0f),mx,my,mz,beta,50.0);
      calcQuat=quat3;
      break;
    case 4:
      mag.getHeadingWithOffset(&mx,&my,&mz,magOffsets4);
      for(int i=0; i<5; i++)
        QuaternionUpdate(quat4,ax,ay,az,fgx*(PI/180.0f),fgy*(PI/180.0f),fgz*(PI/180.0f),mx,my,mz,beta,50.0);
      calcQuat=quat4;
      break;
  }  
  /*Serial.print("Sensor: " + String(sensorId) + " | ");
  Serial.print(String(calcQuat[0]) + " " + String(calcQuat[1]) + " " + String(calcQuat[2]) + " " + String(calcQuat[3]) + " | " );
  Serial.print(String(fax) + " " + String(fay) + " " + String(faz) + " | ");
  Serial.print(String(fgx) + " " + String(fgy) + " " + String(fgz) + " | ");
  Serial.print(String(mx) + " " + String(my) + " " + String(mz) + "\n");*/
  
  return calcQuat;
}
//---------------------------------------------------------------------------
void initializeSensor(int sensorId)
{
  select_sensor(sensorId);
  if (mpu.testConnection())
  {
    Serial.println("conn ok - Sensor: " + String(sensorId));
    //Serial.println("Birl - " + String(sensorId));
    mpu.initialize();
    mag.initialize();
    delay(50);
    int id = (sensorId) * 6;
    mpu.setXAccelOffset(offsets[id]);
    mpu.setYAccelOffset(offsets[id + 1]);
    mpu.setZAccelOffset(offsets[id + 2]);
    mpu.setXGyroOffset(offsets[id + 3]);
    mpu.setYGyroOffset(offsets[id + 4]);
    mpu.setZGyroOffset(offsets[id + 5]);
    //necessary if using the calibrationa matrix
    mpu.setXAccelOffset(0);
    mpu.setYAccelOffset(0);
    mpu.setZAccelOffset(0);
  }
}
//---------------------------------------------------------------------------
void verificaSensor(int sensorId)
{
  Serial.print("Verificando sensor ");
  Serial.print(sensorId);
  select_sensor(sensorId);
  int id = (sensorId) * 6;
  Serial.print("\n" + String(id)); Serial.print("\t\n");
  Serial.print(mpu.getXAccelOffset() == offsets[id]); Serial.print("\t");
  Serial.print(mpu.getYAccelOffset() == offsets[id + 1]); Serial.print("\t");
  Serial.print(mpu.getZAccelOffset() == offsets[id + 2]); Serial.print("\t");
  Serial.print(mpu.getXGyroOffset() == offsets[id + 3]); Serial.print("\t");
  Serial.print(mpu.getYGyroOffset() == offsets[id + 4]); Serial.print("\t");
  Serial.print(mpu.getZGyroOffset() == offsets[id + 5]); Serial.print("\n");
}
//---------------------------------------------------------------------------
//Precisamos de uma funcao que converte uma variavel float para int16
uint16_t float2uint16(float q)
{
  uint16_t resp = 0;
  resp = (uint16_t)(q*16384);
  return resp;
}
//---------------------------------------------------------------------------
void compAcc(int16_t* acc, float* a)
{
  a[0] = acc[0]*calibMatrix[0] + acc[1]*calibMatrix[3] + acc[2]*calibMatrix[6] + calibMatrix[9];
  a[1] = acc[0]*calibMatrix[1] + acc[1]*calibMatrix[4] + acc[2]*calibMatrix[7] + calibMatrix[10];
  a[2] = acc[0]*calibMatrix[2] + acc[1]*calibMatrix[5] + acc[2]*calibMatrix[8] + calibMatrix[11];
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
