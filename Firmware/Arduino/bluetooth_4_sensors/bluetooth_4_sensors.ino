#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include "Timer.h"
#include "SoftwareSerial.h"

#define saidaC 2
#define saidaB 3
#define saidaA 4

// Arduino Wire library is required if I2Cdev I2CDEV_ARDUINO_WIRE implementation
// is used in I2Cdev.h
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
#include "Wire.h"
#endif

//#define DEBUG_PRINT_(x) Serial.print(x)
#define DEBUG_PRINT_(x)

//#define SHOW_DATA(x) Serial.print(x)
#define SHOW_DATA(x)
#define NOSHOWDATA

#define LED_PIN 13
#define sampFreq 100

#define PSDMP 42
#define ST '$'
#define ET '\n'

#define PINS1 2
#define PINS2 3
#define PINS3 4
#define PINS4 5

MPU6050 mpu1(0x68);
MPU6050 mpu2(0x68);
MPU6050 mpu3(0x68);
MPU6050 mpu4(0x68);

const int offsets1[6] = {  -616, 2821, 1246, 16,   110,  34};
const int offsets2[6] = { -331, 537,  1702, 142,  51,   35};
const int offsets3[6] = { -2850,  312,  1612, -34,  6,    49};
const int offsets4[6] = { 715,  401,  1242, 5,    46,   54};

uint8_t fifoBuffer1[42]; // FIFO storage fifoBuffer of mpu1
uint8_t fifoBuffer2[42]; // FIFO storage fifoBuffer of mpu2
uint8_t fifoBuffer3[42]; // FIFO storage fifoBuffer of mpu3
uint8_t fifoBuffer4[42]; // FIFO storage fifoBuffer of mpu4

Timer t;
int timer_id;

const double sampPeriod = (1.0 / sampFreq) * 1000000;//Sampling period in us
String serialOp; // Variable for receiving commands from serial
uint16_t fifoCount;     // count of all bytes currently in FIFO
int numbPackets;
float q[4]; // [w, x, y, z] quaternion container
float a[3]; // [x, y, z] acceleration container
float g[3]; // [x, y, z] gyroscope container
bool is_alive = false;
bool running_coleta = false;
bool led_state = LOW;

void takereading();

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

  iniciar_sensor_1();
  iniciar_sensor_2();
  iniciar_sensor_3();
  iniciar_sensor_4();

  verifica_offsets_sensor_1();
  verifica_offsets_sensor_2();
  verifica_offsets_sensor_3();
  verifica_offsets_sensor_4();

  DEBUG_PRINT_("Aguardando comando serial.\n");
  while (!Serial.available());
  t.every(10,takereading); //chama a cada 10ms

}

void loop() {
  t.update(); 
}


void takereading(){
  Serial.write(0x7F);
  ler_sensor_1(); send_serial_packet_1(); //SHOW_DATA("1:\t"); mostrar_dados_1(); //send_serial_packet_1();
  ler_sensor_2(); send_serial_packet_2(); //SHOW_DATA("2:\t"); mostrar_dados_2(); //send_serial_packet_2();
  ler_sensor_3(); send_serial_packet_3(); //SHOW_DATA("3:\t"); mostrar_dados_3(); //send_serial_packet_3();
  ler_sensor_4(); send_serial_packet_4(); //SHOW_DATA("4:\t"); mostrar_dados_4();// send_serial_packet_4();
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

void iniciar_sensor_1() {
  select_sensor(1);
  if (mpu1.testConnection()) {
    mpu1.initialize(); //Initializes the IMU
    uint8_t ret = mpu1.dmpInitialize(); //Initializes the DMP
    delay(50);
    if (ret == 0) {
      mpu1.setDMPEnabled(true);
      mpu1.setXAccelOffset(offsets1[0]);
      mpu1.setYAccelOffset(offsets1[1]);
      mpu1.setZAccelOffset(offsets1[2]);
      mpu1.setXGyroOffset(offsets1[3]);
      mpu1.setYGyroOffset(offsets1[4]);
      mpu1.setZGyroOffset(offsets1[5]);
      DEBUG_PRINT_("Sensor 1 configurado com sucesso.\n");
    } else {
      DEBUG_PRINT_("Erro na inicializacao do sensor 1!\n");
    }
  } else {
    DEBUG_PRINT_("Erro na conexao do sensor 1.\n");
  }
}

void iniciar_sensor_2() {
  select_sensor(2);
  if (mpu2.testConnection()) {
    mpu2.initialize(); //Initializes the IMU
    uint8_t ret = mpu2.dmpInitialize(); //Initializes the DMP
    delay(50);
    if (ret == 0) {
      mpu2.setDMPEnabled(true);
      mpu2.setXAccelOffset(offsets2[0]);
      mpu2.setYAccelOffset(offsets2[1]);
      mpu2.setZAccelOffset(offsets2[2]);
      mpu2.setXGyroOffset(offsets2[3]);
      mpu2.setYGyroOffset(offsets2[4]);
      mpu2.setZGyroOffset(offsets2[5]);
      DEBUG_PRINT_("Sensor 2 configurado com sucesso.\n");
    } else {
      DEBUG_PRINT_("Erro na inicializacao do sensor 2!\n");
    }
  } else {
    DEBUG_PRINT_("Erro na conexao do sensor 2.\n");
  }
}

void iniciar_sensor_3() {
  select_sensor(3);
  if (mpu3.testConnection()) {
    mpu3.initialize(); //Initializes the IMU
    uint8_t ret = mpu3.dmpInitialize(); //Initializes the DMP
    delay(50);
    if (ret == 0) {
      mpu3.setDMPEnabled(true);
      mpu3.setXAccelOffset(offsets3[0]);
      mpu3.setYAccelOffset(offsets3[1]);
      mpu3.setZAccelOffset(offsets3[2]);
      mpu3.setXGyroOffset(offsets3[3]);
      mpu3.setYGyroOffset(offsets3[4]);
      mpu3.setZGyroOffset(offsets3[5]);
      DEBUG_PRINT_("Sensor 3 configurado com sucesso.\n");
    } else {
      DEBUG_PRINT_("Erro na inicializacao do sensor 3!\n");
    }
  } else {
    DEBUG_PRINT_("Erro na conexao do sensor 3.\n");
  }
}

void iniciar_sensor_4() {
  select_sensor(4);
  if (mpu4.testConnection()) {
    mpu4.initialize(); //Initializes the IMU
    uint8_t ret = mpu4.dmpInitialize(); //Initializes the DMP
    delay(50);
    if (ret == 0) {
      mpu4.setDMPEnabled(true);
      mpu4.setXAccelOffset(offsets4[0]);
      mpu4.setYAccelOffset(offsets4[1]);
      mpu4.setZAccelOffset(offsets4[2]);
      mpu4.setXGyroOffset(offsets4[3]);
      mpu4.setYGyroOffset(offsets4[4]);
      mpu4.setZGyroOffset(offsets4[5]);
      DEBUG_PRINT_("Sensor 4 configurado com sucesso.\n");
    } else {
      DEBUG_PRINT_("Erro na inicializacao do sensor 4!\n");
    }
  } else {
    DEBUG_PRINT_("Erro na conexao do sensor 4.\n");
  }
}

void verifica_offsets_sensor_1() {
  select_sensor(1);
  DEBUG_PRINT_("Verificando Sensor 1:\t");
  DEBUG_PRINT_(mpu1.getXAccelOffset() == offsets1[0]); DEBUG_PRINT_("\t");
  DEBUG_PRINT_(mpu1.getYAccelOffset() == offsets1[1]); DEBUG_PRINT_("\t");
  DEBUG_PRINT_(mpu1.getZAccelOffset() == offsets1[2]); DEBUG_PRINT_("\t");
  DEBUG_PRINT_(mpu1.getXGyroOffset() == offsets1[3]); DEBUG_PRINT_("\t");
  DEBUG_PRINT_(mpu1.getYGyroOffset() == offsets1[4]); DEBUG_PRINT_("\t");
  DEBUG_PRINT_(mpu1.getZGyroOffset() == offsets1[5]); DEBUG_PRINT_("\n");
}

void verifica_offsets_sensor_2() {
  select_sensor(2);
  DEBUG_PRINT_("Verificando Sensor 2:\t");
  DEBUG_PRINT_(mpu2.getXAccelOffset() == offsets2[0]); DEBUG_PRINT_("\t");
  DEBUG_PRINT_(mpu2.getYAccelOffset() == offsets2[1]); DEBUG_PRINT_("\t");
  DEBUG_PRINT_(mpu2.getZAccelOffset() == offsets2[2]); DEBUG_PRINT_("\t");
  DEBUG_PRINT_(mpu2.getXGyroOffset() == offsets2[3]); DEBUG_PRINT_("\t");
  DEBUG_PRINT_(mpu2.getYGyroOffset() == offsets2[4]); DEBUG_PRINT_("\t");
  DEBUG_PRINT_(mpu2.getZGyroOffset() == offsets2[5]); DEBUG_PRINT_("\n");
}

void verifica_offsets_sensor_3() {
  select_sensor(3);
  DEBUG_PRINT_("Verificando Sensor 3:\t");
  DEBUG_PRINT_(mpu3.getXAccelOffset() == offsets3[0]); DEBUG_PRINT_("\t");
  DEBUG_PRINT_(mpu3.getYAccelOffset() == offsets3[1]); DEBUG_PRINT_("\t");
  DEBUG_PRINT_(mpu3.getZAccelOffset() == offsets3[2]); DEBUG_PRINT_("\t");
  DEBUG_PRINT_(mpu3.getXGyroOffset() == offsets3[3]); DEBUG_PRINT_("\t");
  DEBUG_PRINT_(mpu3.getYGyroOffset() == offsets3[4]); DEBUG_PRINT_("\t");
  DEBUG_PRINT_(mpu3.getZGyroOffset() == offsets3[5]); DEBUG_PRINT_("\n");
}

void verifica_offsets_sensor_4() {
  select_sensor(4);
  DEBUG_PRINT_("Verificando Sensor 4:\t");
  DEBUG_PRINT_(mpu4.getXAccelOffset() == offsets4[0]); DEBUG_PRINT_("\t");
  DEBUG_PRINT_(mpu4.getYAccelOffset() == offsets4[1]); DEBUG_PRINT_("\t");
  DEBUG_PRINT_(mpu4.getZAccelOffset() == offsets4[2]); DEBUG_PRINT_("\t");
  DEBUG_PRINT_(mpu4.getXGyroOffset() == offsets4[3]); DEBUG_PRINT_("\t");
  DEBUG_PRINT_(mpu4.getYGyroOffset() == offsets4[4]); DEBUG_PRINT_("\t");
  DEBUG_PRINT_(mpu4.getZGyroOffset() == offsets4[5]); DEBUG_PRINT_("\n");
}

void ler_sensor_1() {
  select_sensor(1);
  numbPackets = floor(mpu1.getFIFOCount() / PSDMP);
  SHOW_DATA(numbPackets); SHOW_DATA(" - ");
  /*for (int i = 0; i < numbPackets; i++) {
      mpu1.getFIFOBytes(fifoBuffer1, PSDMP);
  }*/
  if (numbPackets >= 24) {
    mpu1.resetFIFO();
    DEBUG_PRINT_("FIFO sensor 1 overflow!\n");
  } else {
    for (int i = 0; i < numbPackets; i++) {
      mpu1.getFIFOBytes(fifoBuffer1, PSDMP);
    }
  }
  numbPackets = 0;
}

void ler_sensor_2() {
  select_sensor(2);
  numbPackets = floor(mpu2.getFIFOCount() / PSDMP);
  SHOW_DATA(numbPackets); SHOW_DATA(" - ");
  if (numbPackets >= 24) {
    mpu2.resetFIFO();
    DEBUG_PRINT_("FIFO sensor 2 overflow!\n");
  } else {
    for (int i = 0; i < numbPackets; i++) {
      mpu2.getFIFOBytes(fifoBuffer2, PSDMP);
    }
  }
  numbPackets = 0;
}

void ler_sensor_3() {
  select_sensor(3);
  numbPackets = floor(mpu3.getFIFOCount() / PSDMP);
  SHOW_DATA(numbPackets); SHOW_DATA(" - ");
  if (numbPackets >= 24) {
    mpu3.resetFIFO();
    DEBUG_PRINT_("FIFO sensor 3 overflow!\n");
  } else {
    for (int i = 0; i < numbPackets; i++) {
      mpu3.getFIFOBytes(fifoBuffer3, PSDMP);
    }
  }
  numbPackets = 0;
}

void ler_sensor_4() {
  select_sensor(4);
  numbPackets = floor(mpu4.getFIFOCount() / PSDMP);
  SHOW_DATA(numbPackets); SHOW_DATA(" - ");
  if (numbPackets >= 24) {
    mpu4.resetFIFO();
    DEBUG_PRINT_("FIFO sensor 4 overflow!\n");
  } else {
    for (int i = 0; i < numbPackets; i++) {
      mpu4.getFIFOBytes(fifoBuffer4, PSDMP);
    }
  }
  numbPackets = 0;
}

void mostrar_dados_1() {
#ifndef NOSHOWDATA
  //Quaternion
  q[0] = (float) ((fifoBuffer1[0] << 8) | fifoBuffer1[1]) / 16384.0f;
  q[1] = (float) ((fifoBuffer1[4] << 8) | fifoBuffer1[5]) / 16384.0f;
  q[2] = (float) ((fifoBuffer1[8] << 8) | fifoBuffer1[9]) / 16384.0f;
  q[3] = (float) ((fifoBuffer1[12] << 8) | fifoBuffer1[13]) / 16384.0f;

  //Aceleracao
  a[0] = (float) ((fifoBuffer1[28] << 8) | fifoBuffer1[29]) / 8192.0f;
  a[1] = (float) ((fifoBuffer1[32] << 8) | fifoBuffer1[33]) / 8192.0f;
  a[2] = (float) ((fifoBuffer1[36] << 8) | fifoBuffer1[37]) / 8192.0f;

  //Giroscopio
  g[0] = (float) ((fifoBuffer1[16] << 8) | fifoBuffer1[17]) / 131.0f;
  g[1] = (float) ((fifoBuffer1[20] << 8) | fifoBuffer1[21]) / 131.0f;
  g[2] = (float) ((fifoBuffer1[24] << 8) | fifoBuffer1[25]) / 131.0f;
  //Quaternions
  DEBUG_PRINT_(q[0]);
  DEBUG_PRINT_("\t");
  DEBUG_PRINT_(q[1]);
  DEBUG_PRINT_("\t");
  DEBUG_PRINT_(q[2]);
  DEBUG_PRINT_("\t");
  DEBUG_PRINT_(q[3]);
  DEBUG_PRINT_("\t-\t");
  //accel in G
  DEBUG_PRINT_(a[0]);
  DEBUG_PRINT_("\t");
  DEBUG_PRINT_(a[1]);
  DEBUG_PRINT_("\t");
  DEBUG_PRINT_(a[2]);
  DEBUG_PRINT_("\t-\t");
  //g[1]ro in degrees/s
  DEBUG_PRINT_(g[0]);
  DEBUG_PRINT_("\t");
  DEBUG_PRINT_(g[1]);
  DEBUG_PRINT_("\t");
  DEBUG_PRINT_(g[2]);
  DEBUG_PRINT_("\t");
#endif /*NOSHOWDATA*/
}
void mostrar_dados_2() {
#ifndef NOSHOWDATA
  //Quaternion
  q[0] = (float) ((fifoBuffer2[0] << 8) | fifoBuffer2[1]) / 16384.0f;
  q[1] = (float) ((fifoBuffer2[4] << 8) | fifoBuffer2[5]) / 16384.0f;
  q[2] = (float) ((fifoBuffer2[8] << 8) | fifoBuffer2[9]) / 16384.0f;
  q[3] = (float) ((fifoBuffer2[12] << 8) | fifoBuffer2[13]) / 16384.0f;

  //Aceleracao
  a[0] = (float) ((fifoBuffer2[28] << 8) | fifoBuffer2[29]) / 8192.0f;
  a[1] = (float) ((fifoBuffer2[32] << 8) | fifoBuffer2[33]) / 8192.0f;
  a[2] = (float) ((fifoBuffer2[36] << 8) | fifoBuffer2[37]) / 8192.0f;

  //Giroscopio
  g[0] = (float) ((fifoBuffer2[16] << 8) | fifoBuffer2[17]) / 131.0f;
  g[1] = (float) ((fifoBuffer2[20] << 8) | fifoBuffer2[21]) / 131.0f;
  g[2] = (float) ((fifoBuffer2[24] << 8) | fifoBuffer2[25]) / 131.0f;
  //Quaternions
  DEBUG_PRINT_(q[0]);
  DEBUG_PRINT_("\t");
  DEBUG_PRINT_(q[1]);
  DEBUG_PRINT_("\t");
  DEBUG_PRINT_(q[2]);
  DEBUG_PRINT_("\t");
  DEBUG_PRINT_(q[3]);
  DEBUG_PRINT_("\t-\t");
  //accel in G
  DEBUG_PRINT_(a[0]);
  DEBUG_PRINT_("\t");
  DEBUG_PRINT_(a[1]);
  DEBUG_PRINT_("\t");
  DEBUG_PRINT_(a[2]);
  DEBUG_PRINT_("\t-\t");
  //g[1]ro in degrees/s
  DEBUG_PRINT_(g[0]);
  DEBUG_PRINT_("\t");
  DEBUG_PRINT_(g[1]);
  DEBUG_PRINT_("\t");
  DEBUG_PRINT_(g[2]);
  DEBUG_PRINT_("\t");
#endif /*NOSHOWDATA*/
}
void mostrar_dados_3() {
#ifndef NOSHOWDATA
  //Quaternion
  q[0] = (float) ((fifoBuffer3[0] << 8) | fifoBuffer3[1]) / 16384.0f;
  q[1] = (float) ((fifoBuffer3[4] << 8) | fifoBuffer3[5]) / 16384.0f;
  q[2] = (float) ((fifoBuffer3[8] << 8) | fifoBuffer3[9]) / 16384.0f;
  q[3] = (float) ((fifoBuffer3[12] << 8) | fifoBuffer3[13]) / 16384.0f;

  //Aceleracao
  a[0] = (float) ((fifoBuffer3[28] << 8) | fifoBuffer3[29]) / 8192.0f;
  a[1] = (float) ((fifoBuffer3[32] << 8) | fifoBuffer3[33]) / 8192.0f;
  a[2] = (float) ((fifoBuffer3[36] << 8) | fifoBuffer3[37]) / 8192.0f;

  //Giroscopio
  g[0] = (float) ((fifoBuffer3[16] << 8) | fifoBuffer3[17]) / 131.0f;
  g[1] = (float) ((fifoBuffer3[20] << 8) | fifoBuffer3[21]) / 131.0f;
  g[2] = (float) ((fifoBuffer3[24] << 8) | fifoBuffer3[25]) / 131.0f;
  //Quaternions
  DEBUG_PRINT_(q[0]);
  DEBUG_PRINT_("\t");
  DEBUG_PRINT_(q[1]);
  DEBUG_PRINT_("\t");
  DEBUG_PRINT_(q[2]);
  DEBUG_PRINT_("\t");
  DEBUG_PRINT_(q[3]);
  DEBUG_PRINT_("\t-\t");
  //accel in G
  DEBUG_PRINT_(a[0]);
  DEBUG_PRINT_("\t");
  DEBUG_PRINT_(a[1]);
  DEBUG_PRINT_("\t");
  DEBUG_PRINT_(a[2]);
  DEBUG_PRINT_("\t-\t");
  //g[1]ro in degrees/s
  DEBUG_PRINT_(g[0]);
  DEBUG_PRINT_("\t");
  DEBUG_PRINT_(g[1]);
  DEBUG_PRINT_("\t");
  DEBUG_PRINT_(g[2]);
  DEBUG_PRINT_("\t");
#endif /*NOSHOWDATA*/
}
void mostrar_dados_4() {
#ifndef NOSHOWDATA
  //Quaternion
  q[0] = (float) ((fifoBuffer4[0] << 8) | fifoBuffer4[1]) / 16384.0f;
  q[1] = (float) ((fifoBuffer4[4] << 8) | fifoBuffer4[5]) / 16384.0f;
  q[2] = (float) ((fifoBuffer4[8] << 8) | fifoBuffer4[9]) / 16384.0f;
  q[3] = (float) ((fifoBuffer4[12] << 8) | fifoBuffer4[13]) / 16384.0f;

  //Aceleracao
  a[0] = (float) ((fifoBuffer4[28] << 8) | fifoBuffer4[29]) / 8192.0f;
  a[1] = (float) ((fifoBuffer4[32] << 8) | fifoBuffer4[33]) / 8192.0f;
  a[2] = (float) ((fifoBuffer4[36] << 8) | fifoBuffer4[37]) / 8192.0f;

  //Giroscopio
  g[0] = (float) ((fifoBuffer4[16] << 8) | fifoBuffer4[17]) / 131.0f;
  g[1] = (float) ((fifoBuffer4[20] << 8) | fifoBuffer4[21]) / 131.0f;
  g[2] = (float) ((fifoBuffer4[24] << 8) | fifoBuffer4[25]) / 131.0f;
  //Quaternions
  DEBUG_PRINT_(q[0]);
  DEBUG_PRINT_("\t");
  DEBUG_PRINT_(q[1]);
  DEBUG_PRINT_("\t");
  DEBUG_PRINT_(q[2]);
  DEBUG_PRINT_("\t");
  DEBUG_PRINT_(q[3]);
  DEBUG_PRINT_("\t-\t");
  //accel in G
  DEBUG_PRINT_(a[0]);
  DEBUG_PRINT_("\t");
  DEBUG_PRINT_(a[1]);
  DEBUG_PRINT_("\t");
  DEBUG_PRINT_(a[2]);
  DEBUG_PRINT_("\t-\t");
  //g[1]ro in degrees/s
  DEBUG_PRINT_(g[0]);
  DEBUG_PRINT_("\t");
  DEBUG_PRINT_(g[1]);
  DEBUG_PRINT_("\t");
  DEBUG_PRINT_(g[2]);
  DEBUG_PRINT_("\t");
#endif /*NOSHOWDATA*/
}

void send_serial_packet_1() {
  //Assembling packet and sending
  Serial.write(fifoBuffer1[0]); //qw_msb
  Serial.write(fifoBuffer1[1]); //qw_lsb
  Serial.write(fifoBuffer1[4]); //qx_msb
  Serial.write(fifoBuffer1[5]); //qx_lsb
  Serial.write(fifoBuffer1[8]); //qy_msb
  Serial.write(fifoBuffer1[9]); //qy_lsb
  Serial.write(fifoBuffer1[12]); //qz_msb
  Serial.write(fifoBuffer1[13]); //qz_lsb
}

void send_serial_packet_2() {
  //Assembling packet and sending
  Serial.write(fifoBuffer2[0]); //qw_msb
  Serial.write(fifoBuffer2[1]); //qw_lsb
  Serial.write(fifoBuffer2[4]); //qx_msb
  Serial.write(fifoBuffer2[5]); //qx_lsb
  Serial.write(fifoBuffer2[8]); //qy_msb
  Serial.write(fifoBuffer2[9]); //qy_lsb
  Serial.write(fifoBuffer2[12]); //qz_msb
  Serial.write(fifoBuffer2[13]); //qz_lsb
}

void send_serial_packet_3() {
  //Assembling packet and sending
  Serial.write(fifoBuffer3[0]); //qw_msb
  Serial.write(fifoBuffer3[1]); //qw_lsb
  Serial.write(fifoBuffer3[4]); //qx_msb
  Serial.write(fifoBuffer3[5]); //qx_lsb
  Serial.write(fifoBuffer3[8]); //qy_msb
  Serial.write(fifoBuffer3[9]); //qy_lsb
  Serial.write(fifoBuffer3[12]); //qz_msb
  Serial.write(fifoBuffer3[13]); //qz_lsb
}

void send_serial_packet_4() {
  //Assembling packet and sending
  Serial.write(fifoBuffer4[0]); //qw_msb
  Serial.write(fifoBuffer4[1]); //qw_lsb
  Serial.write(fifoBuffer4[4]); //qx_msb
  Serial.write(fifoBuffer4[5]); //qx_lsb
  Serial.write(fifoBuffer4[8]); //qy_msb
  Serial.write(fifoBuffer4[9]); //qy_lsb
  Serial.write(fifoBuffer4[12]); //qz_msb
  Serial.write(fifoBuffer4[13]); //qz_lsb
}



