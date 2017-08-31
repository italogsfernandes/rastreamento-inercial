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
const int offsets[6] = { -1226, -71, 431, 78, -31, 25};
uint8_t fifoBuffer[42]; // FIFO storage fifoBuffer of mpu
int numbPackets;

//Variaveis Magnetometro
HMC5883L mag;
uint8_t mag_buffer[6];
int16_t magOffsets[3] = {46,-322,-69};
int16_t magmax[3] = {0,0,0};
int16_t magmin[3] = {0,0,0};

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
      magCal(mag,magOffsets,magmin,magmax);
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
  //mag.getHeading(mag_buffer);
  mag.getHeadingWithOffsetBuffer(mag_buffer,magOffsets);
  enviar_pacote_inercial();  
}

void enviar_pacote_inercial() {
#ifndef DEBUG_MODE
  //Assembling packet and sending
  serial_buffer_out[0] = UART_START; //['$']
  //Quaternion
  serial_buffer_out[1] = fifoBuffer[0];  serial_buffer_out[2] = fifoBuffer[1];    //[QWH] [QWL]
  serial_buffer_out[3] = fifoBuffer[4];  serial_buffer_out[4] = fifoBuffer[5];    //[QXH] [QXL]
  serial_buffer_out[5] = fifoBuffer[8];  serial_buffer_out[6] = fifoBuffer[9];    //[QYH] [QYL]
  serial_buffer_out[7] = fifoBuffer[12]; serial_buffer_out[8] = fifoBuffer[13];   //[QZH] [QZL]  
  //Aceleracao
  serial_buffer_out[9] = fifoBuffer[28]; serial_buffer_out[10] = fifoBuffer[29];  //[AXH] [AXL]
  serial_buffer_out[11] = fifoBuffer[32]; serial_buffer_out[12] = fifoBuffer[33]; //[AYH] [AYL]
  serial_buffer_out[13] = fifoBuffer[36]; serial_buffer_out[14] = fifoBuffer[37]; //[AZH] [AZL]
  //Giroscopio
  serial_buffer_out[15] = fifoBuffer[16]; serial_buffer_out[16] = fifoBuffer[17]; //[GXH] [GXL]
  serial_buffer_out[17] = fifoBuffer[20]; serial_buffer_out[18] = fifoBuffer[21]; //[GYH] [GYL]
  serial_buffer_out[19] = fifoBuffer[24]; serial_buffer_out[20] = fifoBuffer[25]; //[GZH] [GZL]
  //Magnetometer
  serial_buffer_out[21] = mag_buffer[0]; serial_buffer_out[22] = mag_buffer[1]; //[MXH] [MXL]
  serial_buffer_out[23] = mag_buffer[2]; serial_buffer_out[24] = mag_buffer[3]; //[MYH] [MYL]
  serial_buffer_out[25] = mag_buffer[4]; serial_buffer_out[26] = mag_buffer[5]; //[MZH] [MZL]

  //data fusion
  //quaternion dmp
  quatDMP[0] = (float) ((fifoBuffer[0] << 8) | fifoBuffer[1]) / 16384.0f;
  quatDMP[1] = (float) ((fifoBuffer[4] << 8) | fifoBuffer[5]) / 16384.0f;
  quatDMP[2] = (float) ((fifoBuffer[8] << 8) | fifoBuffer[9]) / 16384.0f;
  quatDMP[3] = (float) ((fifoBuffer[12] << 8) | fifoBuffer[13]) / 16384.0f;
  //magnetometer - invert X and Y axis so it is the same orientation of the gyroscope
  magRead[0] = (float) ((((int16_t)mag_buffer[2]) << 8) | mag_buffer[3]) / 1090.0f;
  magRead[1] = -(float) ((((int16_t)mag_buffer[0]) << 8) | mag_buffer[1]) / 1090.0f;
  magRead[2] = (float) ((((int16_t)mag_buffer[4]) << 8) | mag_buffer[5]) / 1090.0f;
  //data fusion algorithm
  //RawDataFusion(fifoBuffer,mag_buffer,quatCompensated,2.0,&lastDMPYaw,&lastYaw);
  DataFusion(quatDMP,magRead,quatCompensated,2.0,&lastDMPYaw,&lastYaw);
  //converting from float back to uint16
  uint16_t qw = floatTouint16(quatCompensated[0]);
  uint16_t qx = floatTouint16(quatCompensated[1]);
  uint16_t qy = floatTouint16(quatCompensated[2]);
  uint16_t qz = floatTouint16(quatCompensated[3]);
  //compensated quaternion
  serial_buffer_out[27] = (uint8_t) (qw>>8); serial_buffer_out[28] = (uint8_t)qw&0xFF;
  serial_buffer_out[29] = (uint8_t) (qx>>8); serial_buffer_out[30] = (uint8_t)qx&0xFF;
  serial_buffer_out[31] = (uint8_t) (qy>>8); serial_buffer_out[32] = (uint8_t)qy&0xFF;
  serial_buffer_out[33] = (uint8_t) (qz>>8); serial_buffer_out[34] = (uint8_t)qz&0xFF;
  serial_buffer_out[35] = UART_END; //['\n']
  Serial.write(serial_buffer_out, 36);
#endif /*DEBUG_MODE*/
#ifdef DEBUG_MODE
  float q[4], a[3], g[3], m[3];
  //Quaternion
  q[0] = (float) ((fifoBuffer[0] << 8) | fifoBuffer[1]) / 16384.0f;
  q[1] = (float) ((fifoBuffer[4] << 8) | fifoBuffer[5]) / 16384.0f;
  q[2] = (float) ((fifoBuffer[8] << 8) | fifoBuffer[9]) / 16384.0f;
  q[3] = (float) ((fifoBuffer[12] << 8) | fifoBuffer[13]) / 16384.0f;
  quatDMP[0] = q[0];
  quatDMP[1] = q[1];
  quatDMP[2] = q[2];
  quatDMP[3] = q[3];
  //Aceleracao
  a[0] = (float) ((fifoBuffer[28] << 8) | fifoBuffer[29]) / 8192.0f;
  a[1] = (float) ((fifoBuffer[32] << 8) | fifoBuffer[33]) / 8192.0f;
  a[2] = (float) ((fifoBuffer[36] << 8) | fifoBuffer[37]) / 8192.0f;
  //Giroscopio
  g[0] = (float) ((fifoBuffer[16] << 8) | fifoBuffer[17]) / 131.0f;
  g[1] = (float) ((fifoBuffer[20] << 8) | fifoBuffer[21]) / 131.0f;
  g[2] = (float) ((fifoBuffer[24] << 8) | fifoBuffer[25]) / 131.0f;
  //Magnetrometro
  m[0] = (float) ((((int16_t)mag_buffer[0]) << 8) | mag_buffer[1]) / 1090.0f;
  m[1] = (float) ((((int16_t)mag_buffer[2]) << 8) | mag_buffer[3]) / 1090.0f;
  m[2] = (float) ((((int16_t)mag_buffer[4]) << 8) | mag_buffer[5]) / 1090.0f;    
  magRead[0] = m[1];
  magRead[1] = -m[0];
  magRead[2] = m[2];
  //correcting orientation axis
  //float aux = m[0];
  //m[0] = m[1];
  //m[1] = aux;
  //DEBUG_PRINT_("Quat-Accel-Gyro-Mag:\t");
  //Quaternions  
  DEBUG_PRINT_(q[0]);
  DEBUG_PRINT_("\t");
  DEBUG_PRINT_(q[1]);
  DEBUG_PRINT_("\t");
  DEBUG_PRINT_(q[2]);
  DEBUG_PRINT_("\t");
  DEBUG_PRINT_(q[3]);
  DEBUG_PRINT_("\t | ");
  //compensation
  DataFusion(quatDMP,magRead,quatCompensated,2.0,&lastDMPYaw,&lastYaw);            
  DEBUG_PRINT_(quatCompensated[0]);
  DEBUG_PRINT_("\t");
  DEBUG_PRINT_(quatCompensated[1]);
  DEBUG_PRINT_("\t");
  DEBUG_PRINT_(quatCompensated[2]);
  DEBUG_PRINT_("\t");
  DEBUG_PRINT_(quatCompensated[3]);
  DEBUG_PRINT_("\t | ");  
  float heading = atan2(m[1],m[0]);
  if(heading < 0)
    heading += 2.0*M_PI;    
  DEBUG_PRINT_(String(heading*(180.0/M_PI)));
  DEBUG_PRINT_(" | ");
  //Prints the raw magnetomter values
  DEBUG_PRINT_(String(m[0],3));
  DEBUG_PRINT_("  ");
  DEBUG_PRINT_(String(m[1],3));
  DEBUG_PRINT_("  ");
  DEBUG_PRINT_(String(m[2],3));  
  DEBUG_PRINT_("\n");
  //accel in G
  /*DEBUG_PRINT_(a[0]);
  DEBUG_PRINT_("\t");
  DEBUG_PRINT_(a[1]);
  DEBUG_PRINT_("\t");
  DEBUG_PRINT_(a[2]);
  DEBUG_PRINT_("\t-\t");
  //gyro in degrees/s
  DEBUG_PRINT_(g[0]);
  DEBUG_PRINT_("\t");
  DEBUG_PRINT_(g[1]);
  DEBUG_PRINT_("\t");
  DEBUG_PRINT_(g[2]);
  DEBUG_PRINT_("\t-\t");
  //mag in .... ?? //TODO : descobrir
  DEBUG_PRINT_(String(m[0],3));
  DEBUG_PRINT_("\t");
  DEBUG_PRINT_(String(m[1],3));
  DEBUG_PRINT_("\t");
  DEBUG_PRINT_(String(m[2],3));
  DEBUG_PRINT_("\t  ");
  float heading = atan2(m[1],m[0]);
  if(heading < 0)
    heading += 2.0*M_PI;
  DEBUG_PRINT_(String(heading*(180.0/M_PI)));
  DEBUG_PRINT_("\n");*/
#endif /*DEBUG_MODE*/
}

uint16_t floatTouint16(float q)
{
  uint16_t resp = 0;
  resp = (uint16_t)(q*16384);
  return resp;
}

