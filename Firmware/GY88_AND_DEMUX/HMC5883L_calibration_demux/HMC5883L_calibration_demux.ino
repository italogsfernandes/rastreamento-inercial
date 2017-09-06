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
//---------------------------------------------------------------------------
//Sensor to be calibrated
uint8_t indexCh = 0;
//Magnetometer
HMC5883L mag;
uint8_t mag_buffer[6];
int16_t magOffsets[3] = {0,0,0};
int16_t magmax[3] = {0,0,0};
int16_t magmin[3] = {0,0,0};
int16_t mx, my, mz; //mag
float fmx,fmy,fmz;
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void setup()
{
  pinMode(saidaC, OUTPUT);  pinMode(saidaB, OUTPUT);  pinMode(saidaA, OUTPUT);
  Serial.begin(115200);

  #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    Wire.begin();
    Wire.setClock(400000);
  #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
    Fastwire::setup(400, true);
  #endif

  // wait for ready
  while (Serial.available() && Serial.read()); // empty buffer
  while (!Serial.available()) {
    Serial.println(F("Select the desired sensor (0-7) to start sketch:\n"));
    delay(1500);
  }
  indexCh = (uint8_t) Serial.read() - (uint8_t) '0' ;
  while (Serial.available() && Serial.read()); // empty buffer again

  Serial.println("Calibrating sensor: " + String(indexCh));
  select_sensor(indexCh);
  mag.initialize();
  Serial.println(mag.testConnection() ? "HMC5883L connection successful" : "HMC5883L connection failed");
  delay(50);
  Serial.println("HMC5883L calibration");
  Serial.println("Move the sensor in a figure-eight pattern until the procedure is done");
  delay(100);
  calibration(magOffsets);
  Serial.println("Finished!\n");
  Serial.println("Offsets");
  Serial.println(String(magOffsets[0]) + "  " + String(magOffsets[1]) + "  " + String(magOffsets[2]) + "\n");
  Serial.println("Readings with offset");
  mag.getHeadingWithOffset(&mx,&my,&mz,magOffsets);
  Serial.println(String(mx) + "  " + String(my) + "  " + String(mz) + "\n");
  Serial.print("const int magOffsets" + String(indexCh) + "[3] = { ");
  Serial.print(String(magOffsets[0]) + ", ");
  Serial.print(String(magOffsets[1]) + ", ");
  Serial.print(String(magOffsets[2]) + " }; ");
}
//---------------------------------------------------------------------------
void loop() {

}
//---------------------------------------------------------------------------

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
//Calibration
//Hard-iron correction
//TODO: Soft-iron correction
void calibration(int16_t* magOffsets)
{
  uint16_t ii = 0, sample_count = 1500; //de novo? uhum
  int32_t mag_bias[3] = {0, 0, 0}, mag_scale[3] = {0, 0, 0};
  int16_t mx,my,mz;
  int16_t mag_max[3] = {-32767, -32767, -32767}, mag_min[3] = {32767, 32767, 32767}, mag_temp[3] = {0, 0, 0};  

  //take readings to find minimum and maximum values
  for(ii = 0; ii < sample_count; ii++)
  {
    mag.getHeading(&mx,&my,&mz);  // Read the mag data
    mag_temp[0] = mx;
    mag_temp[1] = my;
    mag_temp[2] = mz;  

    //finds new maximum and minimum for each axis
    for (int jj = 0; jj < 3; jj++)
    {
      if(mag_temp[jj] > mag_max[jj]) mag_max[jj] = mag_temp[jj];
      if(mag_temp[jj] < mag_min[jj]) mag_min[jj] = mag_temp[jj];    
    }

    delay(10);
  }
  
  // Get hard iron correction
  magOffsets[0]  = (mag_max[0] + mag_min[0])/2;  // get average x mag bias in counts
  magOffsets[1]  = (mag_max[1] + mag_min[1])/2;  // get average y mag bias in counts
  magOffsets[2]  = (mag_max[2] + mag_min[2])/2;  // get average z mag bias in counts
}
//------------------------------------------------------------------------------
