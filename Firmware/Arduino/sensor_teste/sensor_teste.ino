
/*
  Verificar conexao para leitura de sensores MPU6050

  SCL, SDA


*/

#include "I2Cdev.h"
#include "HMC5883L.h"
#include "MPU6050_6Axis_MotionApps20.h"

#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
#include "Wire.h"
#endif
ter

#define PIN_SENSOR1 3  // ok?
#define PIN_SENSOR2 4

//declarando as variaveis
char chosen_option; //Recebe opçao
MPU6050 mpu1(0x68);
MPU6050 mpu2(0x68);

void setup() {
  Serial.begin(115200); //Inicializando a porta serial
  pinMode(PIN_SENSOR1, OUTPUT); // Inicializa os pinos
  pinMode(PIN_SENSOR2, OUTPUT);
  Wire.begin();
  initialize_sensors();
  Serial.println("Chose sensor 1 or 2:");
}

void loop() {
  if (Serial.available()) { // Se a comunicaçao esta disponivel para leitura 

    chosen_option = Serial.read(); // leitura monitor serial
    if (chosen_option == "1") {
      select_sensor(1);

      mpu1.setXAccelOffset(100);
      mpu1.setYAccelOffset(101);
      mpu1.setZAccelOffset(102);
      mpu1.setXGyroOffset(103);
      mpu1.setYGyroOffset(103);
      mpu1.setZGyroOffset(104);

      Serial.println("Offsets sensor 1:");
      Serial.print(mpu1.getXAccelOffset()); Serial.print("\t");
      Serial.print(mpu1.getYAccelOffset()); Serial.print("\t");
      Serial.print(mpu1.getZAccelOffset()); Serial.print("\t");
      Serial.print(mpu1.getXGyroOffset()); Serial.print("\t");
      Serial.print(mpu1.getYGyroOffset()); Serial.print("\t");
      Serial.print(mpu1.getZGyroOffset()); Serial.print("\n");
    }

    else if (chosen_option == "2") {
      select_sensor(2);

      mpu2.setXAccelOffset(200);
      mpu2.setYAccelOffset(201);
      mpu2.setZAccelOffset(202);
      mpu2.setXGyroOffset(203);
      mpu2.setYGyroOffset(204);
      mpu2.setZGyroOffset(205);

      Serial.println("Offsets sensor 2:");
      Serial.print(mpu2.getXAccelOffset()); Serial.print("\t");
      Serial.print(mpu2.getYAccelOffset()); Serial.print("\t");
      Serial.print(mpu2.getZAccelOffset()); Serial.print("\t");
      Serial.print(mpu2.getXGyroOffset()); Serial.print("\t");
      Serial.print(mpu2.getYGyroOffset()); Serial.print("\t");
      Serial.print(mpu2.getZGyroOffset()); Serial.print("\n");
    }
  }
}

void initialize_sensors() {
  select_sensor(1);
  //Iniciando o sensor
  Serial.print("Testando o sensor 1.....\n");
  mpu1.initialize();
  if (mpu1.testConnection())
  {
    Serial.print("Iniciando o sensor 1.....\n");
    //Initializes the IMU
    mpu1.initialize();
    //Initializes the DMP
    uint8_t ret = mpu1.dmpInitialize();
    delay(50);
    if (ret == 0) {
      mpu1.setDMPEnabled(true); /*Not Calibrated yet*/
      mpu1.setXAccelOffset(-1759);
      mpu1.setYAccelOffset(1051);
      mpu1.setZAccelOffset(1510);
      mpu1.setXGyroOffset(-117);
      mpu1.setYGyroOffset(-27);
      mpu1.setZGyroOffset(71);
      Serial.print("Sensor 1 Iniciado.\n");
      Serial.print("Testando conexao - " + String(mpu1.testConnection()) + "\n");
    }
    else
    {
      Serial.print("Erro na inicializacao do sensor 1!\n");
    }
  }
  else
    Serial.print("Erro.....\n");


  select_sensor(2);
  Serial.print("Testando o sensor 2.....\n");
  mpu2.initialize();
  if (mpu2.testConnection())
  {
    Serial.print("Iniciando o sensor 2.....\n");
    //Initializes the IMU
    mpu2.initialize();
    //Initializes the DMP
    uint8_t ret = mpu2.dmpInitialize();
    delay(50);
    if (ret == 0) {
      mpu2.setDMPEnabled(true); /*Not Calibrated yet*/
      mpu2.setXAccelOffset(-1759);
      mpu2.setYAccelOffset(1051);
      mpu2.setZAccelOffset(1510);
      mpu2.setXGyroOffset(-117);
      mpu2.setYGyroOffset(-27);
      mpu2.setZGyroOffset(71);
      Serial.print("Sensor 2 Iniciado.\n");
      Serial.print("Testando conexao - " + String(mpu2.testConnection()) + "\n");
    }
    else
    {
      Serial.print("Erro na inicializacao do sensor 2 !\n");
    }
  }
  else
    Serial.print("Erro.....\n");
}


void select_sensor(int sensorid) {

  if (sensorid == 1) {
    digitalWrite(PIN_SENSOR1, LOW);
    digitalWrite(PIN_SENSOR2, HIGH);

  }
  else if (sensorid == 2) {
    digitalWrite(PIN_SENSOR1, HIGH);
    digitalWrite(PIN_SENSOR2, LOW);

  }

}


