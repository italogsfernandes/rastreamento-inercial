//Andrei Nakagawa, MSc
 


//Carrega a biblioteca Wire
#include<Wire.h>
//Biblioteca para manipulacao dos timers
#include <Event.h>
#include <Timer.h>

//baud rate 
//Based on the size of the package and the sampling frequency
//Considering a package of size 17 (bytes) and 200 Hz sampfreq (17*8*200) = 27200 minimum for baud rate

#define POW 0x6b
#define ACX 0x3b
#define MPU 0x68
#define ST 0x53
#define ET 0x04
#define PS 14 //Package si
#define baud 38400 //baud rate
#define sampFreq 100 //Sampling frequency

//Offsets
//Sensor readings with offsets:	-246	-61	16523	-3	1	0
//Your offsets:	-2768	-712	1184	81	-28	2


const double sampPeriod = 1.0 / sampFreq;
//Variaveis para armazenar valores dos sensores
int AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;

byte package[PS];
int accelOffsets[3] = {-2730,-666,1182};
int gyroOffsets[3] = {81,-28,2};

String startAcq = "S";
String stopAcq = "E";
Timer timer;
int timerEvent;

void initializeMPU6050();
void getData();
void timerTick();

void setup()
{
  Serial.begin(baud);
  initializeMPU6050();
  setOffset(accelOffsets,gyroOffsets);
}


void loop()
{ 
  
  if(Serial.available() > 0)
  {
    String dado = Serial.readString();
    if(dado == "S")
    {
      timer.every(sampPeriod*1000,timerTick);
    }  
    else if(dado == "E")
    {
      timer.stop(timerEvent);
    }
  }  
  timer.update();
}


void timerTick()
{
   
  getData();
  Serial.write(ST);
  Serial.write(PS);
  Serial.write(package,PS);
  Serial.write(ET);
}

//Initializing communication with the MPU6050
void initializeMPU6050()
{
  Wire.beginTransmission(MPU);
  Wire.write(0x6B); //PWR_MGMT register
  Wire.write(0); //Wake up
  Wire.endTransmission(true);
}

void setOffset(int* accelOff, int* gyroOff)
{
  byte accoff[6];
  byte gyroff[6];
  for(int i=0; i<3; i++)
  {
    accoff[2*i] = accelOff[i] >> 8;
    accoff[(2*i) + 1] = (uint8_t)accelOff[i];
  }
  for(int i=0; i<3; i++)
  {
    gyroff[2*i] = gyroOff[i] >> 8;
    gyroff[(2*i) + 1] = (uint8_t)gyroOff[i];
  }  
  
  //Acceloremeter
  Wire.beginTransmission(MPU);
  //Acess the register
  Wire.write(0x06);
  Wire.write(accoff,6);
  //Writes into the register  
  Wire.endTransmission(true);
  
  //Gyroscope
  Wire.beginTransmission(MPU);
  //Acess the register
  Wire.write(0x13);
  Wire.write(gyroff,6);
  //Writes into the register  
  Wire.endTransmission(true);
}

void getData()
{
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)?
  Wire.endTransmission(false);
  
  Wire.requestFrom(MPU, PS, true); //Solicita os 14 registers do sensor
  
  for(int i=0; i<PS; i++)
  {
    package[i] = Wire.read();
  }   
}
