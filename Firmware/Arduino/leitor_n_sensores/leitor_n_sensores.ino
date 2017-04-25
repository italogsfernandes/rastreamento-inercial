/* ------------------------------------------------------------------------------
   FEDERAL UNIVERSITY OF UBERLANDIA
   Faculty of Electrical Engineering
   Biomedical Engineering Lab
   Uberlândia, Brazil
   ------------------------------------------------------------------------------
   Author: Andrei Nakagawa, MSc e alunos
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

//------------------------------------------------------------------------------
//  INCLUDES
//------------------------------------------------------------------------------
// I2Cdev and MPU6050 must be installed as libraries, or else the .cpp/.h files
// for both classes must be in the include path of your project
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include <DueTimer.h>
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//  DEFINES
//------------------------------------------------------------------------------
//LED
#define LED_PIN 13 //LED for signaling acquisition running
#define PINO_ADDR_SENSOR2 6
#define PINO_ADDR_SENSOR3 7

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
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//  Objects for handling the MPU6050 and HCM5883L
//------------------------------------------------------------------------------
// class default I2C address is 0x68
// specific I2C addresses may be passed as a parameter here
// AD0 low = 0x68 (default for InvenSense evaluation board)
// AD0 high = 0x69
MPU6050 mpu2(0x68);
MPU6050 mpu3(0x68);
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//  VARIABLES
//------------------------------------------------------------------------------
//Data acquisition variables
//Sampling period in us
const double sampPeriod = (1.0 / sampFreq) * 1000000;
//Serial variables
String serialOp; // Variable for receiving commands from serial
//DMP variables
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer
int numbPackets;
// orientation/motion vars
float q[4];           // [w, x, y, z]         quaternion container
int16_t ax, ay, az;
int16_t gx, gy, gz;

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
  Wire.begin();
  //Defines the LED_PIN as output
  pinMode(LED_PIN, OUTPUT);
  pinMode(PINO_ADDR_SENSOR2, OUTPUT);
  pinMode(PINO_ADDR_SENSOR3, OUTPUT);
  //Initializes serial comm with the specified baud rate
  Serial.begin(baud);
  Serial.println("Arduino Iniciado.");
  inicializar_sensores();
  //timerDataAcq();
  Serial.println("Envie um comando: CMDSTART,CMDSTOP,CMDCONN,CMDREAD...");
}

void loop() {
  //Menu
  if (Serial.available() > 0) {
    serialOp = Serial.readString();
    if (serialOp == "CMDSTART")
    {
      digitalWrite(LED_PIN, HIGH);

      select_sensor(2);
      Serial.println("Testando conexao aqs - " + String(mpu2.testConnection()));
      mpu2.resetFIFO();

      select_sensor(3);
      Serial.println("Testando conexao aqs - " + String(mpu3.testConnection()));
      mpu3.resetFIFO();

      delay(5);
      Timer3.attachInterrupt(timerDataAcq).start(sampPeriod);
    }
    else if (serialOp == "CMDSTOP")
    {
      digitalWrite(LED_PIN, LOW);
      Timer3.stop();
    }
    else if (serialOp == "CMDCONN")
    {
      inicializar_sensores();
    }
    else if (serialOp == "CMDREAD")
    {
      timerDataAcq();
    }
  }
}


void select_sensor(uint8_t sensor_id) {
  switch (sensor_id) {
    case 2:
      digitalWrite(PINO_ADDR_SENSOR2, LOW); //0x68
      digitalWrite(PINO_ADDR_SENSOR3, HIGH); //0x69
      break;
    case 3:
      digitalWrite(PINO_ADDR_SENSOR2, HIGH); //0x69
      digitalWrite(PINO_ADDR_SENSOR3, LOW);//0x68
      break;
  }
}

void inicializar_sensores() {
  //Iniciando o sensor id 2
  select_sensor(2);
  if (mpu2.testConnection())
  {
    //Initializes the IMU
    mpu2.initialize();
    //Initializes the DMP
    uint8_t ret = mpu2.dmpInitialize();
    delay(50);
    if (ret == 0) {
      mpu2.setDMPEnabled(true);
      mpu2.setXAccelOffset(-336);
      mpu2.setYAccelOffset(560);
      mpu2.setZAccelOffset(1702);
      mpu2.setXGyroOffset(139);
      mpu2.setYGyroOffset(49);
      mpu2.setZGyroOffset(35);
      Serial.println("Sensor 2 Iniciado.");
      Serial.println("Testando conexao - " + String(mpu2.testConnection()));
    }
    else
    {
      Serial.println("Erro na inicializacao do sensor 2!");
    }
  }
  //Iniciando o sensor id 3
  select_sensor(3);
  if (mpu3.testConnection())
  {
    //Initializes the IMU
    mpu3.initialize();
    //Initializes the DMP
    uint8_t ret = mpu3.dmpInitialize();
    delay(50);
    if (ret == 0) {
      mpu3.setDMPEnabled(true);
      mpu3.setXAccelOffset(-3039);
      mpu3.setYAccelOffset(386);
      mpu3.setZAccelOffset(1602);
      mpu3.setXGyroOffset(-35);
      mpu3.setYGyroOffset(1);
      mpu3.setZGyroOffset(46);
      Serial.println("Sensor 3 Iniciado.");
      Serial.println("Testando conexao - " + String(mpu3.testConnection()));

    }
    else
    {
      Serial.println("Erro na inicializacao do sensor 3!");
    }
  }
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// DATA ACQUISITION
//------------------------------------------------------------------------------
void timerDataAcq()
{
  //stops the timer so the function has enough time to run
  Timer3.stop();
  select_sensor(2);
  //Serial.print("SENSOR 2: quat\t");
  numbPackets = floor(mpu2.getFIFOCount() / PSDMP);
  //numbPackets = 1;
    Serial.print(numbPackets);
  for (int i = 0; i < numbPackets; i++) {
    mpu2.getFIFOBytes(fifoBuffer, PSDMP);
  }
  //mpu2.getFIFOBytes(fifoBuffer, PSDMP);
  Serial.print("Sensor 2: ");
  show_data();


  select_sensor(3);
  //Serial.print("SENSOR 3: quat\t");
  numbPackets = floor(mpu3.getFIFOCount() / PSDMP);
  //numbPackets = 1;
  Serial.print(numbPackets);
  for (int i = 0; i < numbPackets; i++)
  {
    mpu3.getFIFOBytes(fifoBuffer, PSDMP);
  }
  //mpu3.getFIFOBytes(fifoBuffer, PSDMP);
  Serial.print(" - Sensor 3: ");
  show_data();
  Serial.println("");



  //starts the timer again
  Timer3.attachInterrupt(timerDataAcq).start(sampPeriod);

}

void show_data() {
  //Quaternion
  q[0] = (fifoBuffer[0] << 8 | fifoBuffer[1]) / 16384.00;
  q[1] = (fifoBuffer[4] << 8 | fifoBuffer[5]) / 16384.00;
  q[2] = (fifoBuffer[8] << 8 | fifoBuffer[9]) / 16384.00;
  q[3] = (fifoBuffer[12] << 8 | fifoBuffer[13]) / 16384.00;
  q[0] = q[0] > 2 ? q[0] - 4 : q[0];
  q[1] = q[1] > 2 ? q[1] - 4 : q[1];
  q[2] = q[2] > 2 ? q[2] - 4 : q[2];
  q[3] = q[3] > 2 ? q[3] - 4 : q[3];
  Serial.print(q[0]);
  Serial.print("\t");
  Serial.print(q[1]);
  Serial.print("\t");
  Serial.print(q[2]);
  Serial.print("\t");
  Serial.print(q[3]);
  q[0] = 0; q[1] = 0; q[2] = 0; q[3] = 0;
}

