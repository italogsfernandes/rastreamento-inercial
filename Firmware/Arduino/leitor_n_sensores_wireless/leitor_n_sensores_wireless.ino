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
#include "Timer.h"
//#include <DueTimer.h>
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//  DEFINES
//------------------------------------------------------------------------------
//Comente e descomente de acordo com quais sensores estiver a usar
#define USING_SENSOR_1
//#define USING_SENSOR_2
//#define USING_SENSOR_3
//#define USING_SENSOR_4

#define QNT_SENSORES 1 //numero de sensores ativos

#define DEBUG_PRINT_(x) Serial.print(x)
//#define DEBUG_PRINT_(x)
//LED
#define LED_PIN 13 //LED for signaling acquisition running 
#define PINO_ADDR_SENSOR1 5
#define PINO_ADDR_SENSOR2 6
#define PINO_ADDR_SENSOR3 7
#define PINO_ADDR_SENSOR4 8


#define baud 115200 //Default baud rate
#define sampFreq 100 //Sampling frequency
//Size of DMP packages
#define PSDMP 42
#define PS 20
//Headers
#define ST 0x24 //Start transmission
#define ET 0x0A //End transmission
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
MPU6050 mpu1(0x68);
MPU6050 mpu2(0x68);
MPU6050 mpu3(0x68);
MPU6050 mpu4(0x68);

Timer t;
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
uint8_t fifoBuffer[64]; // FIFO storage fifoBuffer
int numbPackets;
// orientation/motion vars
float q[4];           // [w, x, y, z]         quaternion container
int16_t ax, ay, az;
int16_t gx, gy, gz;
bool is_alive = false;
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
  Wire.setClock(100000);
  //Defines the LED_PIN as output
  pinMode(LED_PIN, OUTPUT);
  pinMode(PINO_ADDR_SENSOR1, OUTPUT);
  pinMode(PINO_ADDR_SENSOR2, OUTPUT);
  pinMode(PINO_ADDR_SENSOR3, OUTPUT);
  pinMode(PINO_ADDR_SENSOR4, OUTPUT);

  //Initializes serial comm with the specified baud rate
  Serial.begin(baud);
  DEBUG_PRINT_("Arduino Iniciado.\n");
  inicializar_sensores();
  digitalWrite(LED_PIN, HIGH);

  DEBUG_PRINT_("Iniciando....\n");
#ifdef USING_SENSOR_1
  select_sensor(1);
  DEBUG_PRINT_("Testando conexao com sensor 1 - " + String(mpu1.testConnection()) + "\n");
  mpu1.resetFIFO();
#endif
#ifdef USING_SENSOR_2
  select_sensor(2);
  DEBUG_PRINT_("Testando conexao com sensor 2 - " + String(mpu2.testConnection()) + "\n");
  mpu2.resetFIFO();
#endif
#ifdef USING_SENSOR_3
  select_sensor(3);
  DEBUG_PRINT_("Testando conexao com sensor 3 - " + String(mpu3.testConnection()) + "\n");
  mpu3.resetFIFO();
#endif
#ifdef USING_SENSOR_4
  select_sensor(4);
  DEBUG_PRINT_("Testando conexao com sensor 4 - " + String(mpu4.testConnection()) + "\n");
  mpu4.resetFIFO();
#endif

  delay(5);
  //timerDataAcq();
  t.every(sampPeriod / 1000, timerDataAcq);
  DEBUG_PRINT_("Envie um comando: CMDSTART,CMDSTOP,CMDCONN,CMDREAD...");
}

void loop() {
  //Menu
  t.update();
  if (Serial.available() > 0) {
    serialOp = Serial.readString();
    if (serialOp == "CMDSTART")
    {
      digitalWrite(LED_PIN, HIGH);

#ifdef USING_SENSOR_1
      select_sensor(1);
      DEBUG_PRINT_("Testando conexao aqs - " + String(mpu1.testConnection()) + "\n");
      mpu1.resetFIFO();
#endif

#ifdef USING_SENSOR_2
      select_sensor(2);
      DEBUG_PRINT_("Testando conexao aqs - " + String(mpu2.testConnection()) + "\n");
      mpu2.resetFIFO();
#endif

#ifdef USING_SENSOR_3
      select_sensor(3);
      DEBUG_PRINT_("Testando conexao aqs - " + String(mpu3.testConnection()) + "\n");
      mpu3.resetFIFO();
#endif

#ifdef USING_SENSOR_4
      select_sensor(4);
      DEBUG_PRINT_("Testando conexao aqs - " + String(mpu4.testConnection()) + "\n");
      mpu4.resetFIFO();
#endif

      delay(5);
      //Timer3.attachInterrupt(timerDataAcq).start(sampPeriod);
      //is_alive = true;
    }
    else if (serialOp == "CMDSTOP")
    {
      digitalWrite(LED_PIN, LOW);
      //Timer3.stop();
      //is_alive = false;
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
  //  if(is_alive){
  //    timerDataAcq();
  //  }

       
     
}


void select_sensor(uint8_t sensor_id) {
  switch (sensor_id) {

    case 1:
      digitalWrite(PINO_ADDR_SENSOR1, LOW); //0x68
      digitalWrite(PINO_ADDR_SENSOR2, HIGH); //0x69
      digitalWrite(PINO_ADDR_SENSOR3, HIGH); //0x69
      digitalWrite(PINO_ADDR_SENSOR4, HIGH); //0x69
      break;

    case 2:
      digitalWrite(PINO_ADDR_SENSOR2, LOW); //0x68
      digitalWrite(PINO_ADDR_SENSOR3, HIGH); //0x69
      digitalWrite(PINO_ADDR_SENSOR1, HIGH); //0x69
      digitalWrite(PINO_ADDR_SENSOR4, HIGH); //0x69
      break;

    case 3:
      digitalWrite(PINO_ADDR_SENSOR3, LOW);//0x68
      digitalWrite(PINO_ADDR_SENSOR2, HIGH); //0x69
      digitalWrite(PINO_ADDR_SENSOR1, HIGH); //0x69
      digitalWrite(PINO_ADDR_SENSOR4, HIGH); //0x69
      break;

    case 4:
      digitalWrite(PINO_ADDR_SENSOR4, LOW); //0x68
      digitalWrite(PINO_ADDR_SENSOR2, HIGH); //0x69
      digitalWrite(PINO_ADDR_SENSOR3, HIGH); //0x69
      digitalWrite(PINO_ADDR_SENSOR1, HIGH); //0x69
      break;

  }
}

void inicializar_sensores() {

#ifdef USING_SENSOR_1
  //Iniciando o sensor id 1
  select_sensor(1);
  DEBUG_PRINT_("testando.....1\n");
  mpu1.initialize();
  DEBUG_PRINT_("testando.....2\n");
  if (mpu1.testConnection())
  {
    DEBUG_PRINT_("Iniciando.....1\n");
    //Initializes the IMU
    mpu1.initialize();
    //Initializes the DMP
    uint8_t ret = mpu1.dmpInitialize();
    delay(50);
    if (ret == 0) {
      /*mpu1.setDMPEnabled(true); 
      mpu1.setXAccelOffset(-520);
      mpu1.setYAccelOffset(632);
      mpu1.setZAccelOffset(914);
      mpu1.setXGyroOffset(22);
      mpu1.setYGyroOffset(-8);
      mpu1.setZGyroOffset(26);*/
      
      mpu1.setDMPEnabled(true);
      mpu1.setXAccelOffset(-360);
      mpu1.setYAccelOffset(643);
      mpu1.setZAccelOffset(1696);
      mpu1.setXGyroOffset(143);
      mpu1.setYGyroOffset(50);
      mpu1.setZGyroOffset(34);

      
      DEBUG_PRINT_("Sensor 1 Iniciado.\n");
      DEBUG_PRINT_("Testando conexao - " + String(mpu1.testConnection()) + "\n");
      if(mpu1.testConnection())
        DEBUG_PRINT_("CONN OK\n");

     DEBUG_PRINT_(String(mpu1.getXAccelOffset()) + "\n");
      
    }
    else
    {
      DEBUG_PRINT_("Erro na inicializacao do sensor 1!\n");
    }
  }
  else
    DEBUG_PRINT_("deu ruim.....1\n");
#endif /*USING_SENSOR_1*/
#ifdef USING_SENSOR_2
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
      mpu2.setDMPEnabled(true);/*Calibrated at 03 Mai 2017*/
      mpu2.setXAccelOffset(-3030);
      mpu2.setYAccelOffset(318);
      mpu2.setZAccelOffset(1624);
      mpu2.setXGyroOffset(-37);
      mpu2.setYGyroOffset(10);
      mpu2.setZGyroOffset(50);
      DEBUG_PRINT_("Sensor 2 Iniciado.\n");
      DEBUG_PRINT_("Testando conexao - " + String(mpu2.testConnection()) + "\n");
    }
    else
    {
      DEBUG_PRINT_("Erro na inicializacao do sensor 2!\n");
    }
  }
#endif /*USING_SENSOR_2*/
#ifdef USING_SENSOR_3
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
      mpu3.setDMPEnabled(true);/*Calibrated at 03 Mai 2017*/
      mpu3.setXAccelOffset(-382);
      mpu3.setYAccelOffset(682);
      mpu3.setZAccelOffset(1706);
      mpu3.setXGyroOffset(141);
      mpu3.setYGyroOffset(48);
      mpu3.setZGyroOffset(38);
      DEBUG_PRINT_("Sensor 3 Iniciado.\n");
      DEBUG_PRINT_("Testando conexao - " + String(mpu3.testConnection()) + "\n");

    }
    else
    {
      DEBUG_PRINT_("Erro na inicializacao do sensor 3!\n");
    }
  }
#endif /*USING_SENSOR_3*/
#ifdef USING_SENSOR_4
  //Iniciando o sensor id 4
  select_sensor(4);
  if (mpu4.testConnection())
  {
    //Initializes the IMU
    mpu4.initialize();
    //Initializes the DMP
    uint8_t ret = mpu4.dmpInitialize();
    delay(50);
    if (ret == 0) {
      mpu4.setDMPEnabled(true);
      mpu4.setXAccelOffset(-505);
      mpu4.setYAccelOffset(3004);
      mpu4.setZAccelOffset(1241);
      mpu4.setXGyroOffset(17);
      mpu4.setYGyroOffset(108);
      mpu4.setZGyroOffset(34);
      DEBUG_PRINT_("Sensor 4 Iniciado.\n");
      DEBUG_PRINT_("Testando conexao - " + String(mpu4.testConnection()) + "\n");
    }
    else
    {
      DEBUG_PRINT_("Erro na inicializacao do sensor 4!\n");
    }
  }
#endif /*USING_SENSOR_4*/
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// DATA ACQUISITION
//------------------------------------------------------------------------------
void timerDataAcq()
{
  Serial.write(ST); //byte Start Transmission
  Serial.write(QNT_SENSORES); // Quantos Sensores Ativos
  //stops the timer so the function has enough time to run
  //  Timer3.stop();
#ifdef USING_SENSOR_1
  select_sensor(1);
  //DEBUG_PRINT_("SENSOR 1: quat\t");
  numbPackets = floor(mpu1.getFIFOCount() / PSDMP);
  //numbPackets = 1;
  DEBUG_PRINT_(numbPackets); DEBUG_PRINT_(" - ");
  for (int i = 0; i < numbPackets; i++) {
    mpu1.getFIFOBytes(fifoBuffer, PSDMP);
  }
  //mpu1.getFIFOBytes(fifoBuffer, PSDMP);
  DEBUG_PRINT_("S1: ");
  show_data(fifoBuffer);
#endif /*USING_SENSOR_1*/
#ifdef USING_SENSOR_2
  select_sensor(2);
  //DEBUG_PRINT_("SENSOR 2: quat\t");
  numbPackets = floor(mpu2.getFIFOCount() / PSDMP);
  //numbPackets = 1;
  DEBUG_PRINT_(" - "); DEBUG_PRINT_(numbPackets); DEBUG_PRINT_(" - ");
  for (int i = 0; i < numbPackets; i++) {
    mpu2.getFIFOBytes(fifoBuffer, PSDMP);
  }
  //mpu2.getFIFOBytes(fifoBuffer, PSDMP);
  DEBUG_PRINT_("2: ");
  show_data(fifoBuffer);
#endif /*USING_SENSOR_2*/
#ifdef USING_SENSOR_3
  select_sensor(3);
  //DEBUG_PRINT_("SENSOR 3: quat\t");
  numbPackets = floor(mpu3.getFIFOCount() / PSDMP);
  //numbPackets = 1;
  DEBUG_PRINT_(" - "); DEBUG_PRINT_(numbPackets); DEBUG_PRINT_(" - ");
  for (int i = 0; i < numbPackets; i++)
  {
    mpu3.getFIFOBytes(fifoBuffer, PSDMP);
  }
  //mpu3.getFIFOBytes(fifoBuffer, PSDMP);
  DEBUG_PRINT_("3: ");
  show_data(fifoBuffer);
#endif /*USING_SENSOR_3*/
#ifdef USING_SENSOR_4
  select_sensor(4);
  //DEBUG_PRINT_("SENSOR 4: quat\t");
  numbPackets = floor(mpu4.getFIFOCount() / PSDMP);
  //numbPackets = 1;
  DEBUG_PRINT_(" - "); DEBUG_PRINT_(numbPackets); DEBUG_PRINT_(" - ");
  for (int i = 0; i < numbPackets; i++) {
    mpu4.getFIFOBytes(fifoBuffer, PSDMP);
  }
  //mpu4.getFIFOBytes(fifoBuffer, PSDMP);
  DEBUG_PRINT_("4: ");
  show_data(fifoBuffer);
#endif /*USING_SENSOR_4*/ // que loucura
  Serial.write(ET); //byte End Transmission
  //starts the timer again
  //Timer3.attachInterrupt(timerDataAcq).start(sampPeriod);
}

void show_data(uint8_t* _fifoBuffer) {

  //Quaternion
  q[0] = (_fifoBuffer[0] << 8 | _fifoBuffer[1]) / 16384.00;
  q[1] = (_fifoBuffer[4] << 8 | _fifoBuffer[5]) / 16384.00;
  q[2] = (_fifoBuffer[8] << 8 | _fifoBuffer[9]) / 16384.00;
  q[3] = (_fifoBuffer[12] << 8 | _fifoBuffer[13]) / 16384.00;
  q[0] = q[0] > 2 ? q[0] - 4 : q[0];
  q[1] = q[1] > 2 ? q[1] - 4 : q[1];
  q[2] = q[2] > 2 ? q[2] - 4 : q[2];
  q[3] = q[3] > 2 ? q[3] - 4 : q[3];

  //send_packet();

  DEBUG_PRINT_(q[0]);
  DEBUG_PRINT_("\t");
  DEBUG_PRINT_(q[1]);
  DEBUG_PRINT_("\t");
  DEBUG_PRINT_(q[2]);
  DEBUG_PRINT_("\t");
  DEBUG_PRINT_(q[3]);
  DEBUG_PRINT_("\t");

  DEBUG_PRINT_(String(mpu1.getXAccelOffset()) + "\n");

  q[0] = 0; q[1] = 0; q[2] = 0; q[3] = 0;
  for (int i = 0; i < PSDMP; i++) {
    fifoBuffer[i] = 0;
  }
}

void send_packet() {
  //Assembling packet and sending
  Serial.write(fifoBuffer[0]); //qw_msb
  Serial.write(fifoBuffer[1]); //qw_lsb
  Serial.write(fifoBuffer[4]); //qx_msb
  Serial.write(fifoBuffer[5]); //qx_lsb
  Serial.write(fifoBuffer[8]); //qy_msb
  Serial.write(fifoBuffer[9]); //qy_lsb
  Serial.write(fifoBuffer[12]); //qz_msb
  Serial.write(fifoBuffer[13]); //qz_lsb
}

