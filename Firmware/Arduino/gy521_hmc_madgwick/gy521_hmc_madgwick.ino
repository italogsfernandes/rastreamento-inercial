/* UNIVERSIDADE FEDERAL DE UBERLANDIA
   BIOLAB - Biomedical Engineering Lab

   Autor: Ítalo G S Fernandes
   contact: italogsfernandes@gmail.com
   URLs: www.biolab.eletrica.ufu.br
          https://github.com/BIOLAB-UFU-BRAZIL
   O que faz:
      Realiza leitura de 1 sensor inercial e envia para pc

   TODO:
     Obter offsets do sensor inercial
     Verificar "NOTE" espalhados no codigo.
     Verificar "TODO" espalhados no codigo.

   Pacotes:
   Inercial(Quaternion-Accel-Gyro-Mag): (11 bytes)
   ['$']
   [QWH] [QWL] [QXH] [QXL] [QYH] [QYL] [QZH] [QZL]
               [AXH] [AXL] [AYH] [AYL] [AZH] [AZL]
               [GXH] [GXL] [GYH] [GYL] [GZH] [GZL]
   ['\n']

    Esquema de montagem:
    Arduino - Dispositivo
    13      - LED
    A4      - SDA do GY-521
    A5      - SCL do GY-521
    5V      - VCC do GY-521
    GND     - GND do GY-521

   Para visualizar de forma legivel ao ser humano
   Altere o comentario em: #define DEBUG_MODE
*/
#define DEBUG_MODE

//Se estiver no modo debug printa as msg debug, se nao estiver nao printa
#ifdef DEBUG_MODE
#define DEBUG_PRINT_(x) Serial.print(x)
#endif

#ifndef DEBUG_MODE
#define DEBUG_PRINT_(x)
#endif

#include <math.h>
#include "Timer.h"                     //http://github.com/JChristensen/Timer
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include "HMC5883L.h"
#include "madgwick.h"

// Arduino Wire library is required if I2Cdev I2CDEV_ARDUINO_WIRE implementation
// is used in I2Cdev.h
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
#include "Wire.h"
#endif

#define LED_PIN 13

//NOTE: Antes de usar vc deve alterar a frequenciana biblioteca mpu6050
//CASO ISSO NAO SEJA FEITO CORRE PERIGO DA FIFO ESTOURAR
#define MPUsampFreq 100
#define mpu_interval 10 //Each 10ms

#define PSDMP 42 //Packet Size DMP - tam do pacote interno da mpu-6050

#define UART_BAUDRATE 115200
#define UART_START '$' //Inicio do pacote
#define UART_END '\n' //Fim do pacote

//Variaveis Gerais
Timer t;
uint8_t timer_id;
uint8_t serial_buffer_out[36];
char serialOp;
bool aquisition_running = false;

//Variaveis Inercial
MPU6050 mpu(0x68);
int16_t accel[3], gyro[3];
float ax,ay,az,gx,gy,gz;
const int offsets[6] = { -1226, -71, 431, 78, -31, 25};
uint8_t fifoBuffer[42]; // FIFO storage fifoBuffer of mpu
int numbPackets;

//Variaveis Magnetometro
HMC5883L mag;
float mx,my,mz;
uint8_t mag_buffer[6];
int16_t magOffsets[3] = {46,-322,-69};
int16_t magmax[3] = {0,0,0};
int16_t magmin[3] = {0,0,0};

//madgwick parameters
float GyroMeasError = PI * (40.0f / 180.0f); 
float beta = sqrt(3.0f / 4.0f) * GyroMeasError;   // compute beta
float quat[4] = {1.0f,0.0f,0.0f,0.0f};

void setup()
{
  //GPIO:
  pinMode(LED_PIN, OUTPUT);

  //Sensor Inercial
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
  Wire.begin();
  Wire.setClock(200000); //NOTE: Ajustar de acordo com arduino utilizado
#elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
  Fastwire::setup(400, true);
#endif
  iniciar_sensor_inercial();
  iniciar_mag();
  //Serial:
  Serial.begin(UART_BAUDRATE);
  mag.getHeadingWithOffsetBuffer(mag_buffer,magOffsets);
  
  DEBUG_PRINT_("Insira um comando: '1' - Start, '2' - Stop,  '3' - Magnetometer calibration,  '4' - Quaternion correction.\n");
  //Serial.println("Testing device connections...");
  //Serial.println(mag.testConnection() ? "HMC5883L connection successful" : "HMC5883L connection failed");
  //Begin data fusion
}

void loop()
{
  t.update();
  //Menu
  if (Serial.available() > 0)
  {
    serialOp = Serial.read();
    if (serialOp == '1')
    {
      digitalWrite(LED_PIN, HIGH);
      mpu.resetFIFO();
      delay(5);
      timer_id = t.every(mpu_interval, ler_sensor_inercial); //Realiza leitura e envia pacote(ou mostra) dados a cada mpu_interval
      aquisition_running = true;
      
    }
    else if (serialOp == '2')
    {
      digitalWrite(LED_PIN, LOW);
      aquisition_running = false;
      t.stop(timer_id);
    }
    else if(serialOp == '3')
    {
      DEBUG_PRINT_("Magnetometer calibration\n");
      //magCal(mag,magOffsets,magmin,magmax);
      DEBUG_PRINT_("Min: " + String(magmin[0]) + " " + String(magmin[1]) + " " + String(magmin[2]) + "\n");
      DEBUG_PRINT_("Max: " + String(magmax[0]) + " " + String(magmax[1]) + " " + String(magmax[2]) + "\n");
      DEBUG_PRINT_("Offsets: " + String(magOffsets[0]) + " " + String(magOffsets[1]) + " " + String(magOffsets[2]) + "\n");
      int16_t x,y,z;
      mag.getHeadingWithOffset(&x,&y,&z,magOffsets);
      DEBUG_PRINT_("Mag with offsets: " + String(x) + " " + String(y) + " " + String(z) + "\n");
      DEBUG_PRINT_("Insira um comando: '1' - Start, '2' - Stop,  '3' - Magnetometer calibration.\n");
    }
    else if(serialOp == '4')
    {
      mpu.resetFIFO();
      //faz uma leitura
      ler_sensor_inercial();
    }
  }
}


////////////////////
//Sensor Inercial //
////////////////////

void iniciar_sensor_inercial() {
  if (mpu.testConnection()) {
    mpu.initialize(); //Initializes the IMU
    uint8_t ret = mpu.dmpInitialize(); //Initializes the DMP
    delay(50);
    if (ret == 0) {
      mpu.setDMPEnabled(true);
      mpu.setXAccelOffset(offsets[0]); mpu.setYAccelOffset(offsets[1]); mpu.setZAccelOffset(offsets[2]);
      mpu.setXGyroOffset(offsets[3]); mpu.setYGyroOffset(offsets[4]); mpu.setZGyroOffset(offsets[5]);
      DEBUG_PRINT_("Sensor Inercial configurado com sucesso.\n");
    } else {
      //TODO: adicionar uma forma melhor de aviso. outro led?
      DEBUG_PRINT_("Erro na inicializacao do sensor Inercial!\n");
    }
  } else {
    DEBUG_PRINT_("Erro na conexao do sensor Inercial.\n");
  }
}

void iniciar_mag(){
  mag.initialize();
  // verify connection
  Serial.println("Testing device connections...");
  Serial.println(mag.testConnection() ? "HMC5883L connection successful" : "HMC5883L connection failed");

}

void ler_sensor_inercial()
{
  //inertial sensors
  mpu.getMotion6(accel,gyro);
  ax = (float)accel[0] * (2.0/32768.0);
  ay = (float)accel[1] * (2.0/32768.0);
  az = (float)accel[2] * (2.0/32768.0);
  gx = (float)gyro[0] * (250.0/32768.0);
  gy = (float)gyro[1] * (250.0/32768.0);
  gz = (float)gyro[2] * (250.0/32768.0);
  //Serial.print(String(ax) + " " + String(ay) + " " + String(az) + " ");
  //Serial.print(String(gx) + " " + String(gy) + " " + String(gz) + " ");
  //mag.getHeading(mag_buffer);

  //magnetometer
  int16_t _mx,_my,_mz;
  mag.getHeadingWithOffset(&_mx,&_my,&_mz,magOffsets);
  mx = (float)_mx/1090.0f;
  my = (float)_my/1090.0f;
  mz = (float)_mz/1090.0f;
  //Serial.print(String(mx) + " " + String(my) + " " + String(mz) + "\n");

  QuaternionUpdate(quat,ax,ay,az,gx,gy,gz,mx,my,mz,beta,100.0);
  Serial.println(String(quat[0]) + " " + String(quat[1]) + " " + String(quat[2]) + " " + String(quat[3]) + " ");
  //enviar_pacote_inercial();  
}
