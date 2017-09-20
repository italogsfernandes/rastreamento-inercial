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
    - Luis Ródenas: Gyroscope calibration routine is totally based on his code
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
#define mpuInterval 10
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
/*
const int offsets0[6] = { -1042, -881, 1211, 12, -45, -27}; // offsets para teste com gy-521 ---> { -1275, -70, 495, 87, -33, 25}; // offsets para o sistema final --> { -998, -883, 1276, 10, -48, -28};
const int offsets1[6] = { 2538, -1807, 1525, 48, -15, -8}; // { 3217, -1849, 1713, 47, -18, -4};
const int offsets2[6] = { -615, -1181, 1134, -78, -55, -17};
const int offsets3[6] = { 1329, 902, 1159, -3, -36, 32}; //{ -133, 1107, 3469, -3, -35, 35}; //{ 996, 920, 1222, -3, -34, 35}; // { 2086, 1218, 1306, -5, -36, 35};
const int offsets4[6] = { -1704, 257, 1061, 28, -41, -14}; // { -2276, 382, 1140, 31, -40, -29};*/
const int offsets0[6] = { -1082, -701, 1199, 14, -43, -26}; //const int offsets0[6] = {0, 0, 0, 15, -46, -28};
const int offsets1[6] = { 2422, -1763, 1500, 47, -16, 0}; //{0, 0, 0, 48, -17, -1};
const int offsets2[6] = { -699, -1484, 1101, -79, -57, -18};//{0, 0, 0, -81, -58, -19};
const int offsets3[6] = { 1440, 851, 1135, -4, -35, 28};//{0, 0, 0, -6, -36, 29};
const int offsets4[6] = { -1896, 346, 1041, 34, -40, -11};//{0, 0, 0, 35, -42, -12};

//Esses foram os melhores
const int magOffsets0[3] = {70, 121, -51};
const int magOffsets1[3] = {89, 23, -89};
const int magOffsets2[3] = {93, 55, -62};
const int magOffsets3[3] = {52, -63, -36};
const int magOffsets4[3] = {85, 36, -41};

//Esse é o braço robótico da Carol controlando o golfinho
/*const int magOffsets0[3] = {40,-79,51};
const int magOffsets1[3] = {52,-12,-87};
const int magOffsets2[3] = {50,33,-66};
const int magOffsets3[3] = {25,-109,-36};
const int magOffsets4[3] = {51,-1,-40};*/

//esses eu não sei
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
float calib0[12] = {0.996,-0.0545,-0.0923,0.0481,0.9976,-0.0107,0.0908,0.0210,0.98793,-9195.6089,-5976.745,11062.0886};
float calib1[12] = {1.11639783856209,0.0300951942425417,-0.0702564763363374,-0.0302805283661063,0.992050151725529,-0.0152571584224437,0.0746655726048089,0.0292274399384913,0.983151935452282,23537.3282101097,-14971.2127668260,11447.1701342243};
float calib2[12] = {0.985470917375667,0.0987052269437395,-0.115614099568410,-0.101105564610908,0.994703190977047,0.0518094654111346,0.117506480096026,-0.0235255805426657,0.986973599126166,-4535.82247313294,-13363.1827470091,9502.36577399121};
float calib3[12] = {0.994701225380638,-0.138425287552434,-0.0271977275856171,0.135441964297234,0.993915047934707,0.0288112414203515,0.0185240754707880,-0.0154915934119319,0.988366247336782,15203.5562106878,5088.72505757829,10677.6022101445};
float calib4[12] = {1.00311927384914,-0.0378380968108721,-0.0181489563050085,0.0283549864525669,0.996427171034388,0.0142649127838930,0.00242202338455817,0.0393596873868358,0.992637497400012,-15558.9375219880,4466.53495700628,9602.66320721225};
float cAcc[3] = {0,0,0};
//calibration offsets for magnetometers
int16_t magOffsets[15] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
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
bool is_alive = false;
bool running_coleta = false;
bool led_state = LOW;
char serialOp;
//---------------------------------------------------------------------------
Timer t;
uint8_t timer_id;
void takereading();
bool acqRunning = false;
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

  //calibGyro();
  //calibAccel();
  /*calibMag(magOffsets);
  for(int i=0; i<5; i++)
  {    
    Serial.print("const int magOffsets" + String(i) + "[3] = {" + String(magOffsets[i*3]) + ", " + String(magOffsets[i*3+1]) + ", " + String(magOffsets[i*3+2]) + "};\n");
  }*/
}
//---------------------------------------------------------------------------
void loop() {
  
  if(acqRunning)
    t.update();
    
  if(Serial.available() > 0)
  {
    serialOp = Serial.read();
    if(serialOp == '1')
    {
      t.every(mpuInterval,takereading); //chama a cada 10ms = 1000/20
      acqRunning = true;
    }
    else if(serialOp == '2')
    {
      t.stop(timer_id);
      acqRunning = false;
    }
    else if(serialOp == '3')
    {
      if(!acqRunning)
      {
         calibGyro();
      }
    }
    else if(serialOp == '4')
    {
      if(!acqRunning)
      {
         calibAccel();
      }
    }
    else if(serialOp == '5')
    {
      if(!acqRunning)
      {
         calibMag(magOffsets);
      }
    }
  }
}
//---------------------------------------------------------------------------
void takereading() {
  //Serial.write(0x7F);
  for (int i = 0; i < numSensors; i++)
  {
    quat = readSensor(i);    
    //send_serial_packet(quat);
    //Serial.print(String(quat[0]) + " " + String(quat[1]) + " " + String(quat[2]) + " " + String(quat[3]) + "\n" );    
  }

  //Serial.write(0x7E);
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

  switch(sensorId)
  {
    case 0:
      compAcc(accel,calib0,cAcc);
      break;
     case 1:
      compAcc(accel,calib1,cAcc);
      break;
     case 2:
      compAcc(accel,calib2,cAcc);
      break;
     case 3:
      compAcc(accel,calib3,cAcc);
      break;
     case 4:
      compAcc(accel,calib4,cAcc);
      break;
  }
  
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
        QuaternionUpdate(quat0,fax,fay,faz,fgx*(PI/180.0f),fgy*(PI/180.0f),fgz*(PI/180.0f),mx,my,mz,beta,50.0);
      calcQuat=quat0;
      break;
    case 1:
      mag.getHeadingWithOffset(&mx,&my,&mz,magOffsets1);
      for(int i=0; i<5; i++)
        QuaternionUpdate(quat1,fax,fay,faz,fgx*(PI/180.0f),fgy*(PI/180.0f),fgz*(PI/180.0f),mx,my,mz,beta,50.0);
      calcQuat=quat1;
      break;
    case 2:
      mag.getHeadingWithOffset(&mx,&my,&mz,magOffsets2);
      for(int i=0; i<5; i++)      
        QuaternionUpdate(quat2,fax,fay,faz,fgx*(PI/180.0f),fgy*(PI/180.0f),fgz*(PI/180.0f),mx,my,mz,beta,50.0);
      calcQuat=quat2;
      break;
    case 3:
      mag.getHeadingWithOffset(&mx,&my,&mz,magOffsets3);
      for(int i=0; i<5; i++)
        QuaternionUpdate(quat3,fax,fay,faz,fgx*(PI/180.0f),fgy*(PI/180.0f),fgz*(PI/180.0f),mx,my,mz,beta,50.0);
      calcQuat=quat3;
      break;
    case 4:
      mag.getHeadingWithOffset(&mx,&my,&mz,magOffsets4);
      for(int i=0; i<5; i++)
        QuaternionUpdate(quat4,fax,fay,faz,fgx*(PI/180.0f),fgy*(PI/180.0f),fgz*(PI/180.0f),mx,my,mz,beta,50.0);
      calcQuat=quat4;
      break;
  }  
  Serial.print("Sensor: " + String(sensorId) + " | ");
  Serial.print(String(calcQuat[0]) + " " + String(calcQuat[1]) + " " + String(calcQuat[2]) + " " + String(calcQuat[3]) + "|" );
  Serial.print(String(fax) + " " + String(fay) + " " + String(faz) + "|");
  Serial.print(String(fgx) + " " + String(fgy) + " " + String(fgz) + "|");
  Serial.print(String(mx) + " " + String(my) + " " + String(mz) + "|");
  Serial.print("\n");
  //if(sensorId == 4)
  //  Serial.print("\n");
  
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
    
    //necessary if using the calibration matrix
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
void compAcc(int16_t* acc, float* calibMatrix, float* a)
{
  a[0] = acc[0]*calibMatrix[0] + acc[1]*calibMatrix[3] + acc[2]*calibMatrix[6] + calibMatrix[9];
  a[1] = acc[0]*calibMatrix[1] + acc[1]*calibMatrix[4] + acc[2]*calibMatrix[7] + calibMatrix[10];
  a[2] = acc[0]*calibMatrix[2] + acc[1]*calibMatrix[5] + acc[2]*calibMatrix[8] + calibMatrix[11];
}
//---------------------------------------------------------------------------
//Function for calibrating the gyroscopes
void calibGyro()
{
  //steady
  int giro_deadzone = 1;
  int maxIt = 15;  
  uint8_t nsamp = 100;
  uint8_t gyroTol = 4;
  for(int i=0; i<numSensors; i++)
  {
    int16_t gx_offset=0,gy_offset=0,gz_offset=0;
    int16_t mean_gx=0, mean_gy=0, mean_gz=0;
    long buff_gx=0, buff_gy=0, buff_gz=0;        
    
    select_sensor(i);
    mpu.setXGyroOffset(0);
    mpu.setYGyroOffset(0);
    mpu.setZGyroOffset(0);

    //calibration process
    //take some measurements
    for(int j=0; j<nsamp; j++)    
    {
      delay(10);
    }
    for(int j=0; j<nsamp; j++)
    {
      mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
      buff_gx += gx;
      buff_gy += gy;
      buff_gz += gz;
      delay(10);
    }    
    mean_gx = buff_gx / nsamp;
    mean_gy = buff_gy / nsamp;
    mean_gz = buff_gz / nsamp;    
    
    //find offsets
    gx_offset = -mean_gx / 4;
    gy_offset = -mean_gy / 4;
    gz_offset = -mean_gz / 4;

    for(int k=0; k<maxIt; k++)
    {      
      int ready = 0;
      
      //set new offsets
      mpu.setXGyroOffset(gx_offset);
      mpu.setYGyroOffset(gy_offset);
      mpu.setZGyroOffset(gz_offset);

      buff_gx=0; buff_gy=0; buff_gz=0;     

      for(int j=0; j<nsamp; j++)
      {
        mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
        buff_gx += gx;
        buff_gy += gy;
        buff_gz += gz;
        delay(10);
      }    
      mean_gx = buff_gx / nsamp;
      mean_gy = buff_gy / nsamp;
      mean_gz = buff_gz / nsamp;

      if(abs(mean_gx) < gyroTol) ready++;
      else gx_offset = gx_offset - mean_gx / (gyroTol + 1);
      if(abs(mean_gy) < gyroTol) ready++;
      else gy_offset = gy_offset - mean_gy / (gyroTol + 1);
      if(abs(mean_gz) < gyroTol) ready++;
      else gz_offset = gz_offset - mean_gz / (gyroTol + 1);        

      if(ready == 3)
        break;
    }
    //Serial.print("\nSensor " + String(i) + " calibrated - Offsets: "); 
    //Serial.println(String(gx_offset) + " " + String(gy_offset) + " " + String(gz_offset)); 
  }  
  //Serial.println("\n");
  for(int k=0; k<numSensors; k++)
  {
    select_sensor(k);
    Serial.println("const int offsets" + String(k) + "[6] = {0, 0, 0, " + String(mpu.getXGyroOffset()) + ", " + String(mpu.getYGyroOffset()) + ", " + String(mpu.getZGyroOffset()) + "};");
  }
}
//---------------------------------------------------------------------------
//Calibration of accelerometers
//Simply read the raw values, computations are performed elsewhere
//Should send raw data to the application
void calibAccel()
{
  //6 positions
  uint16_t nsamples = 1000;
  for(int j=0; j<6; j++)
  {
    for(int i=0; i<nsamples; i++)
    {      
      for(int k=0; k<numSensors; k++)
      {    
        select_sensor(k);
        mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
        Serial.print(String(ax) + " " + String(ay) + " " + String(az) + "|");    
        /*Serial.write(ST);
        Serial.write(ax>>8);
        Serial.write(ax&0xFF);
        Serial.write(ay>>8);
        Serial.write(ay&0xFF);
        Serial.write(az>>8);
        Serial.write(az&0xFF);
        Serial.write(ET);*/
      }
      delay(10);
      Serial.print("\n");
    }
    while(!Serial.available());
    Serial.read();
  }
}
//---------------------------------------------------------------------------
void calibMag(int16_t* magOffsets)
{  
  uint16_t ii = 0, sample_count = 3000;
  int32_t mag_bias[3] = {0, 0, 0}, mag_scale[3] = {0, 0, 0};
  int16_t mx,my,mz;
  int16_t mag_max[15] = {-32767, -32767, -32767, -32767, -32767, -32767, -32767, -32767, -32767, -32767, -32767, -32767, -32767, -32767, -32767};
  int16_t mag_min[15] = {32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767};
  int16_t mag_temp[15] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

  //take readings to find minimum and maximum values
  for(ii = 0; ii < sample_count; ii++)
  {
    for(int k=0; k<5; k++)
    {
      //select the sensor
      select_sensor(k);
      mag.getHeading(&mx,&my,&mz);  // Read the mag data
      mag_temp[k*3] = mx;
      mag_temp[k*3 + 1] = my;
      mag_temp[k*3 + 2] = mz;

      //finds new maximum and minimum for each axis
      for (int jj = 0; jj < 3; jj++)
      {        
        if(mag_temp[k*3 + jj] > mag_max[k*3 + jj]) mag_max[k*3 + jj] = mag_temp[k*3 + jj];
        if(mag_temp[k*3 + jj] < mag_min[k*3 + jj]) mag_min[k*3 + jj] = mag_temp[k*3 + jj];
      }
    }
    delay(10);    
  }

  // Get hard iron correction
  for(int j=0; j<5; j++)
  {
    magOffsets[j*3 + 0] = (mag_max[j*3] + mag_min[j*3])/2;  // get average x mag bias in counts
    magOffsets[j*3 + 1] = (mag_max[j*3 + 1] + mag_min[j*3 + 1])/2;  // get average x mag bias in counts
    magOffsets[j*3 + 2] = (mag_max[j*3 + 2] + mag_min[j*3 + 2])/2;  // get average x mag bias in counts    
  }  
   
  for(int i=0; i<5; i++)
  {    
    Serial.print("const int magOffsets" + String(i) + "[3] = {" + String(magOffsets[i*3]) + ", " + String(magOffsets[i*3+1]) + ", " + String(magOffsets[i*3+2]) + "};\n");
  }
}
//---------------------------------------------------------------------------
//0.34 0.35 -0.12 0.87|1.96 -43.12 136.52|0.04 -0.04 1.02|-167.00 -218.00 173.00|

