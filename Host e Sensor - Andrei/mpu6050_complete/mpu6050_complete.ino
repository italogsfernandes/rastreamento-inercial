/* ------------------------------------------------------------------------------
 * FEDERAL UNIVERSITY OF UBERLANDIA
 * Faculty of Electrical Engineering
 * Biomedical Engineering Lab
 * Uberlândia, Brazil
 * ------------------------------------------------------------------------------
 * Author: Andrei Nakagawa, MSc
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

//------------------------------------------------------------------------------
//  INCLUDES
//------------------------------------------------------------------------------
// I2Cdev and MPU6050 must be installed as libraries, or else the .cpp/.h files
// for both classes must be in the include path of your project
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include "HMC5883L.h"
//Timer library for Arduino Due
#include <DueTimer.h>
// Arduino Wire library is required if I2Cdev I2CDEV_ARDUINO_WIRE implementation
// is used in I2Cdev.h
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include "Wire.h"
#endif
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//  DEFINES
//------------------------------------------------------------------------------
//LED
#define LED_PIN 13 //LED for signaling acquisition running
//#define LED_ERROR 12 //LED for communication errors
#define baud 115200 //Default baud rate
#define sampFreq 100 //Sampling frequency
//Size of DMP packages
#define PSDMP 42
#define PS 20
//Headers
#define ST 0x02 //Start transmission
#define ET 0x03 //End transmission
#define CMD_OK 0x01 //Command OK
#define CMD_ERROR 0x02 //Command error
#define ERROR_ACQ_ON 0x05 //Acquisition running
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//  Objects for handling the MPU6050 and HCM5883L
//------------------------------------------------------------------------------
// class default I2C address is 0x68
// specific I2C addresses may be passed as a parameter here
// AD0 low = 0x68 (default for InvenSense evaluation board)
// AD0 high = 0x69
MPU6050 mpu(0x68);
HMC5883L mag; //External magnetometer device
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//  VARIABLES
//------------------------------------------------------------------------------
//Data acquisition variables
//Sampling period in us
const double sampPeriod = (1.0 / sampFreq) * 1000000; 
//Serial variables
String serialOp; // Variable for receiving commands from serial
bool configOp = true; //If true, calibration. If false, acquisition
//DMP variables
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer
//Variables for storing raw data from accelerometers gyroscope and magnetometer
int16_t ax, ay, az; //Accel
int16_t gx, gy, gz; //Gyro
int16_t mx, my, mz; //Mag
//Calibration
uint8_t calibIt = 0; //Counter for the number of iterations in calibration
uint8_t calibCounter = 0; //Counter for the number of samples read during calibration
uint8_t calibStep = 1; //Determines which step of the calibration should be performed
uint8_t accelTol = 0; //Error tolerance for accelerometer readings
uint8_t gyroTol = 0; //Error tolerance for gyroscope readings
uint8_t timeTol = 10; //Maximum duration of the calibration process (seconds)
int16_t accelBuffer[3] = {0,0,0}; //Buffer to store the mean values from accel
int16_t gyroBuffer[3] = {0,0,0};  //Buffer to store the mean values from gyro
//Data acquisition
uint8_t packageType = 0;
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// METHODS
//------------------------------------------------------------------------------
void timerCalibration(); //Method that handles the calibration of the MPU6050
void timerDataAcq(); //Data acquisition method
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//  SETUP
//------------------------------------------------------------------------------
void setup() {
    // join I2C bus (I2Cdev library doesn't do this automatically)
    #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
        Wire.begin();
    #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
        Fastwire::setup(400, true);
    #endif

  //Defines the LED_PIN as output
  pinMode(LED_PIN,OUTPUT);    
  //Initializes serial comm with the specified baud rate
  Serial.begin(baud);  

  if(mpu.testConnection())
  {  
    //Initializes the IMU  
    mpu.initialize(); 
  
    //Initializes the DMP
    uint8_t ret = mpu.dmpInitialize();
    delay(50);  
    
    if(ret == 0)
    {
      mpu.setDMPEnabled(true);  
      mpu.setXAccelOffset(871);
      mpu.setYAccelOffset(1527);
      mpu.setZAccelOffset(1988);
      mpu.setXGyroOffset(36);
      mpu.setYGyroOffset(-37);
      mpu.setZGyroOffset(-1);  
    } 
    else
    {
      //Serial.println("Error!"); 
    }
  }  
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//  MAIN LOOP
//------------------------------------------------------------------------------
void loop()
{
  //Menu  
  if(Serial.available() > 0)
  {
    serialOp = Serial.readString();      
    
    if (serialOp == "CMDSTART")
    {
      Serial.write(CMD_OK);
      //String spt = Serial.readStringUntil('\n');
      packageType = 0x23;
      digitalWrite(LED_PIN,HIGH);
      mpu.resetFIFO();
      delay(5);      
      Timer3.attachInterrupt(timerDataAcq).start(sampPeriod);
    }
    else if (serialOp == "CMDSTOP")
    {
      digitalWrite(LED_PIN,LOW);
      Timer3.stop();
    }
    else if (serialOp == "CMDCONN")
    {      
      bool resp = mpu.testConnection();
      if(resp)
        Serial.write(CMD_OK);
      else
        Serial.write(CMD_ERROR);
    }
    else if(serialOp == "CMDCALIB")
    {      
      //Sends a confirmation of the command
      Serial.write(0x01);
      String sa = Serial.readStringUntil('\n'); //Accel tolerance
      String sg = Serial.readStringUntil('\n'); //Gyro tolerance    
      String t = Serial.readStringUntil('\n');  //Time tolerance 
      accelTol = sa.toInt(); //Converts to int
      gyroTol = sg.toInt(); //Converts to int
      timeTol = t.toInt();  //Converts to int     
      //Configures the accel offsets to zero
      mpu.setXAccelOffset(0);
      mpu.setYAccelOffset(0);
      mpu.setZAccelOffset(0);
      //Configures the gyro offsets to zero
      mpu.setXGyroOffset(0);
      mpu.setYGyroOffset(0);
      mpu.setZGyroOffset(0);
      //Variables that needs to be cleared for calibration
      calibIt = 0;
      calibCounter = 0;
      calibStep = 1;
      //Small delay
      delay(50);
      //Triggers the calibration method
      Timer3.attachInterrupt(timerCalibration).start(sampPeriod);      
    }
  }
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// DATA ACQUISITION
//------------------------------------------------------------------------------
void timerDataAcq()
{  
  int numbPackets = floor(mpu.getFIFOCount()/PSDMP);
  for(int i=0; i<numbPackets; i++)
  {
    mpu.getFIFOBytes(fifoBuffer,PSDMP);
    uint8_t buf[PS];
    //Quaternion
    buf[0] = fifoBuffer[0];        
    buf[1] = fifoBuffer[1];
    buf[2] = fifoBuffer[4];        
    buf[3] = fifoBuffer[5];
    buf[4] = fifoBuffer[8];        
    buf[5] = fifoBuffer[9];
    buf[6] = fifoBuffer[12];        
    buf[7] = fifoBuffer[13];
    //Accelerometer
    buf[8] = fifoBuffer[28];
    buf[9] = fifoBuffer[29];
    buf[10] = fifoBuffer[32];
    buf[11] = fifoBuffer[33];
    buf[12] = fifoBuffer[36];
    buf[13] = fifoBuffer[37];
    //Gyroscope
    buf[14] = fifoBuffer[16];
    buf[15] = fifoBuffer[17];
    buf[16] = fifoBuffer[20];
    buf[17] = fifoBuffer[21];
    buf[18] = fifoBuffer[24];
    buf[19] = fifoBuffer[25];
    Serial.write(ST);      
    Serial.write(PS);
    Serial.write(buf,sizeof(buf));
    Serial.write(ET);        
  }
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// CALIBRATION ROUTINE
//------------------------------------------------------------------------------
/* Everything should happen inside the timer 
 * 1st step: Read raw sensor data for 1s to measure the mean values
 * 2nd step: Estimate the first offset values
 * 3rd step: Read 1s and check if the offsets are providing good values
 * according to the specified tolerance
 * 4th step: Keep looping through step 3 until the values are achieved
 * or the function runs for a specified amount of time
 */
//------------------------------------------------------------------------------
void timerCalibration()
{
  //Switch according to which step of the calibration should be performed
  switch(calibStep)
  {
    //1st step: Measuring data to estimate the first offsets
    case 1:            
      //First readings to measure the mean raw values from accel and gyro
      //Measures during 2 seconds
      if(calibCounter < sampFreq*2)
      {
        //Reads the imu sensors
        mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
        //Iteratively updating the mean value for each sensor
        //Accel-X
        accelBuffer[0] = (calibCounter*accelBuffer[0] + ax)/(calibCounter+1);
        //Accel-Y
        accelBuffer[1] = (calibCounter*accelBuffer[1] + ay)/(calibCounter+1);
        //Accel-Z
        accelBuffer[2] = (calibCounter*accelBuffer[2] + az)/(calibCounter+1);
        //Gyro-X
        gyroBuffer[0] = (calibCounter*gyroBuffer[0] + gx)/(calibCounter+1);
        //Gyro-Y
        gyroBuffer[1] = (calibCounter*gyroBuffer[1] + gy)/(calibCounter+1);
        //Gyro-Z
        gyroBuffer[2] = (calibCounter*gyroBuffer[2] + gz)/(calibCounter+1);
        //Increments the sample counter
        calibCounter++;        
      }
      else
      {        
        //Setting the new offset values 
        //Accelerometer offsets
        mpu.setXAccelOffset(-accelBuffer[0] / 8);
        mpu.setYAccelOffset(-accelBuffer[1] / 8);
        mpu.setZAccelOffset((16384 - accelBuffer[2]) / 8);
        //Gyroscope offsets
        mpu.setXGyroOffset(-gyroBuffer[0] / 4);
        mpu.setYGyroOffset(-gyroBuffer[1] / 4);
        mpu.setZGyroOffset(-gyroBuffer[2] / 4);
        //Goes to the next step of the calibration process
        calibStep = 2;                
      }
      break;

     //2nd step: Iteratively update the offsets for accurate readings
     case 2:
      //Reads the sensors during 1 second
      if(calibCounter <= sampFreq)
      {        
        //Reads the imu sensors
        mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz); 
        //Iteratively updating the mean value for each sensor
        accelBuffer[0] = (calibCounter*accelBuffer[0] + ax)/(calibCounter+1);
        accelBuffer[1] = (calibCounter*accelBuffer[1] + ay)/(calibCounter+1);
        accelBuffer[2] = (calibCounter*accelBuffer[2] + az)/(calibCounter+1);
        gyroBuffer[0] = (calibCounter*gyroBuffer[0] + gx)/(calibCounter+1);
        gyroBuffer[1] = (calibCounter*gyroBuffer[1] + gy)/(calibCounter+1);
        gyroBuffer[2] = (calibCounter*gyroBuffer[2] + gz)/(calibCounter+1);
        //Increments the sample counter
        calibCounter++;                
      }
      else
      {
        //Variable for checking if every sensor is calibrated
        //A sensor is calibrated if its mean value is below the tolerance
        uint8_t calibOk = 0;

        //Accel-X
        if(abs(accelBuffer[0]) < accelTol) calibOk++;        
        else mpu.setXAccelOffset(mpu.getXAccelOffset() - (accelBuffer[0]/accelTol));
        //Accel-Y
        if(abs(accelBuffer[1]) < accelTol) calibOk++;
        else mpu.setYAccelOffset(mpu.getYAccelOffset() - (accelBuffer[1]/accelTol));
        //Accel-Z
        if(abs(accelBuffer[2]) < accelTol) calibOk++;
        else mpu.setZAccelOffset(mpu.getZAccelOffset() + ((16384-accelBuffer[2])/accelTol));
        //Gyro-X  
        if(abs(gyroBuffer[0]) < gyroTol) calibOk++;
        else mpu.setXGyroOffset(mpu.getXGyroOffset() - (gyroBuffer[0]/(gyroTol+1)));
        //Gyro-Y
        if(abs(gyroBuffer[1]) < gyroTol) calibOk++;
        else mpu.setYGyroOffset(mpu.getYGyroOffset() - (gyroBuffer[1]/(gyroTol+1)));
        //Gyro-Z        
        if(abs(gyroBuffer[2]) < gyroTol) calibOk++;
        else mpu.setZGyroOffset(mpu.getZGyroOffset() - (gyroBuffer[2]/(gyroTol+1)));

        //Check if the calibration process can be finished
        if(calibOk == 6 || calibIt == timeTol)
        {                    
          Timer3.stop();
        }
        else
        {          
          accelBuffer[0] = 0;
          accelBuffer[1] = 0;
          accelBuffer[2] = 0;
          gyroBuffer[0] = 0;
          gyroBuffer[1] = 0;
          gyroBuffer[2] = 0;
          calibIt++;
          calibCounter=0;
        }          
      }
      break;      
  }  
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

