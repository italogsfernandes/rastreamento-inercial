/* ------------------------------------------------------------------------------
 * FEDERAL UNIVERSITY OF UBERLANDIA
 * Faculty of Electrical Engineering
 * Biomedical Engineering Lab
 * Uberlândia, Brazil
 * ------------------------------------------------------------------------------
 * Authors:
 *    Andrei Nakagawa, MSc
 *    Ítalo Fernandes
 *    Ana Carolina Torres Cresto
 * contact: nakagawa.andrei@gmail.com
 * URLs: www.biolab.eletrica.ufu.br
 *       https://github.com/BIOLAB-UFU-BRAZIL
 * ------------------------------------------------------------------------------
 * Description:
 * ------------------------------------------------------------------------------
 * Acknowledgements 
 *  - We would like to thank the open-source community that provided many of the
 *  source codes necessary for creating this firmware. 
 *  - Jeff Rowberg as the main developer of the I2Cdevlib
 *  - Luis Ródenas: Our calibration routine is totally based on his code
 * ------------------------------------------------------------------------------
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
#include "MPU6050_6Axis_MotionApps20.h"
#include "Timer.h"
#include "SoftwareSerial.h"
//---------------------------------------------------------------------------
// Arduino Wire library is required if I2Cdev I2CDEV_ARDUINO_WIRE implementation
// is used in I2Cdev.h
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
#include "Wire.h"
#endif
//---------------------------------------------------------------------------
#define saidaC 4
#define saidaB 3
#define saidaA 2
#define LED_PIN 13
#define sampFreq 100
#define PSDMP 42
#define ST '$'
#define ET '\n'
#define PINS1 2
#define PINS2 3
#define PINS3 4
#define PINS4 5
//---------------------------------------------------------------------------
MPU6050 mpu(0x68);
//---------------------------------------------------------------------------
const int numSensors = 5;
const int* offsets;
const int offsets1[6] = { -2511, -599, 1185, 80, -25, -2};
const int offsets2[6] = { -2892, 359, 1616, -24, -7, 40};
const int offsets3[6] = { -231, 722, 906, 16, -19, 26};
const int offsets4[6] = { -588, 489, 1691, 144, 49, 35};
const int offsets5[6] = { -814, 2909, 1258, 16, 110, 34};
/*
const int offsets1[6] = {  -616, 2821, 1246, 16,   110,  34};
const int offsets2[6] = { -331, 537,  1702, 142,  51,   35};
const int offsets3[6] = { -2850,  312,  1612, -34,  6,    49};
const int offsets4[6] = { 715,  401,  1242, 5,    46,   54};*/
//---------------------------------------------------------------------------
uint8_t* fifoBuffer; // FIFO storage fifoBuffer of mpu
uint8_t fb1[42];
uint8_t fb2[42];
uint8_t fb3[42];
uint8_t fb4[42];
uint8_t fb5[42];
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
  //Wire.setClock(400000);
#elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
  Fastwire::setup(400, true);
#endif

  offsets = (int*)malloc(sizeof(int)*numSensors*6);  
  memcpy(offsets,offsets1,sizeof(int)*6);
  memcpy(offsets+6,offsets2,sizeof(int)*6);
  memcpy(offsets+12,offsets3,sizeof(int)*6);
  memcpy(offsets+18,offsets4,sizeof(int)*6);
  memcpy(offsets+24,offsets5,sizeof(int)*6);
  for(int i=0; i<numSensors; i++)
  {
    initializeSensor(i+1);
  }
  Serial.println("Eh hora do show porra!");
  while (!Serial.available());
  
  t.every(10,takereading); //chama a cada 10ms  
}
//---------------------------------------------------------------------------
void loop() {
  t.update(); 
}
//---------------------------------------------------------------------------
void takereading(){
  Serial.write(0x7F);  
  for(int i=0; i<numSensors; i++)
  {
    fifoBuffer = readSensor(i+1);        
    send_serial_packet(fifoBuffer);
    //int t = (fifoBuffer[0]<<8) + fifoBuffer[1];
    //float w = float(t) / 16384.f;
    //Serial.println(String(i+1) + " " + String(w));     
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
void send_serial_packet(uint8_t* _fifoBuffer)
{
  //Assembling packet and sending
  Serial.write(_fifoBuffer[0]); //qw_msb
  Serial.write(_fifoBuffer[1]); //qw_lsb
  Serial.write(_fifoBuffer[4]); //qx_msb
  Serial.write(_fifoBuffer[5]); //qx_lsb
  Serial.write(_fifoBuffer[8]); //qy_msb
  Serial.write(_fifoBuffer[9]); //qy_lsb
  Serial.write(_fifoBuffer[12]); //qz_msb
  Serial.write(_fifoBuffer[13]); //qz_lsb
}
//---------------------------------------------------------------------------
uint8_t* readSensor(int sensorId)
{  
  uint8_t fbuffer[42];  
  
  select_sensor(sensorId);    
  numbPackets = floor(mpu.getFIFOCount() / PSDMP);   

  
  if(numbPackets >= 24)
    mpu.resetFIFO();
  else
  {
    if(sensorId==1)
    {   
      for (int i = 0; i < numbPackets; i++)  
        mpu.getFIFOBytes(fb1, PSDMP);    
        numbPackets=0;
        return fb1;        
    }
    else if(sensorId==2)
    {
      for (int i = 0; i < numbPackets; i++)  
        mpu.getFIFOBytes(fb2, PSDMP);    
        numbPackets=0;
        return fb2;        
    }
    else if(sensorId==3)
    {
      for (int i = 0; i < numbPackets; i++)  
        mpu.getFIFOBytes(fb3, PSDMP);    
        numbPackets=0;
        return fb3;        
    }
    else if(sensorId==4)
    {
      for (int i = 0; i < numbPackets; i++)  
        mpu.getFIFOBytes(fb4, PSDMP);    
        numbPackets=0;
        return fb4;        
    }
    else if(sensorId==5)
    {
      for (int i = 0; i < numbPackets; i++)  
        mpu.getFIFOBytes(fb5, PSDMP);    
        numbPackets=0;
        return fb5;        
    }
  }    
}
//---------------------------------------------------------------------------
void initializeSensor(int sensorId)
{
  select_sensor(sensorId);
  if(mpu.testConnection())
  {
    //Serial.println("conn ok");
    Serial.println("Birl - " + String(sensorId));
    mpu.initialize();
    uint8_t ret = mpu.dmpInitialize();
    delay(50);
    if(ret == 0)
    {
      mpu.setDMPEnabled(true);

      int id = (sensorId-1) * 6;
      mpu.setXAccelOffset(offsets[id]);
      mpu.setYAccelOffset(offsets[id+1]);
      mpu.setZAccelOffset(offsets[id+2]);
      mpu.setXGyroOffset(offsets[id+3]);
      mpu.setYGyroOffset(offsets[id+4]);
      mpu.setZGyroOffset(offsets[id+5]);      
    }
  }
}
//---------------------------------------------------------------------------
