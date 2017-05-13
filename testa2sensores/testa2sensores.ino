#include<Wire.h>  //Biblioteca para I2C

//Endereco I2C do MPU6050
//(acelerometro e giroscopio) ligado em 5V:
//Caso ligue em 3.3V o endereço sera diferente.
#define MPU1_endereco 0x68
#define MPU2_endereco 0x69
#define pinoMPU1 3
#define pinoMPU2 4

//Frequencia de leitura 40Hz:
//implica num periodo de 1/40 = 25 ms
#define periodo 25
#define baud 115200


//Variaveis para armazenar valores dos sensores
int Xac, Yac, Zac; //Acererometro
int Xgi, Ygi, Zgi; //Giroscopio
int Tmp; //Temperatura lida
float TmpCelcius; //Temperatura convertida

void setup()
{
  Serial.begin(baud);
  pinMode(pinoMPU1, OUTPUT);
  pinMode(pinoMPU2, OUTPUT);
  digitalWrite(pinoMPU1, LOW);
  digitalWrite(pinoMPU2, HIGH);
  //Your offsets 1 :  -1281 618 5238  143 47  36
  //Your offsets 2 :  -1319 3923  4283  16  110 35

  Wire.begin();
  acordarMPU6050();
/*
  writeWord(devAddr, MPU6050_RA_XA_OFFS_H, offset);
  writeWord(devAddr, MPU6050_RA_YA_OFFS_H, offset);
  writeWord(devAddr, MPU6050_RA_ZA_OFFS_H, offset);
  writeWord(devAddr, MPU6050_RA_XG_OFFS_USRH, offset);*/
  


  Serial.print("n:\tXac\tYac\tZac\t");
  Serial.print("Xgi\tYgi\tZgi\t");
  Serial.print("Temp\t\n");

}

void loop()
{

  requisitarDadosMPU6050_1();
  Serial.print("1:\t");
  mostrarDados();
  requisitarDadosMPU6050_2();
  Serial.print("-\t2:\t");
  mostrarDados();
  Serial.print('\n');
  delay(10);
}

void mostrarDados() {
  Serial.print(Xac); Serial.print("\t");
  Serial.print(Yac); Serial.print("\t");
  Serial.print(Zac); Serial.print("\t");

  Serial.print(Xgi); Serial.print("\t");
  Serial.print(Ygi); Serial.print("\t");
  Serial.print(Zgi); Serial.print("\t");

  Serial.print(TmpCelcius); Serial.print("\t");
}

void requisitarDadosMPU6050_1() {
  /* Prenche as variaveis com os dados lidos
     Explicação:
      Os dados estao salvos em registers(endereções de memoria) do MPU.
      Para saber quais endereços veja o datasheet do MPU.
      É enviado o sinal para que o MPU comece a ler no register do acelerometro
      Cada leitura gera um inteiro de 16bits _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _
      No sensor cada endereço de memoria tem 8bits.
      Cada valor possui um register High e um register Low. Exemplo:
      Para ler é capturado os 8bits 'H':     Wire.read() = 0000 0000 0110 1010
      Eles sao movidos para a esquerda. Wire.read() << 8 = 0110 1010 0000 0000
      Sao lidos os proximos 8 bits.          Wire.read() = 0000 0000 0110 1010
      É realizada a operação bitewise 'or' ('|'). Valor  = 0110 1010 0110 1010

  */
  Wire.beginTransmission(MPU1_endereco);
  Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)?
  Wire.endTransmission(false);

  Wire.requestFrom(MPU1_endereco, 14, true); //Solicita os 14 registers do sensor
  //Armazena o valor dos sensores nas variaveis correspondentes
  Xac = Wire.read() << 8 | Wire.read(); //0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
  Yac = Wire.read() << 8 | Wire.read(); //0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  Zac = Wire.read() << 8 | Wire.read(); //0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  Tmp = Wire.read() << 8 | Wire.read(); //0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
  Xgi = Wire.read() << 8 | Wire.read(); //0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  Ygi = Wire.read() << 8 | Wire.read(); //0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  Zgi = Wire.read() << 8 | Wire.read(); //0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)

  TmpCelcius = Tmp / 340.00 + 36.53;
}

void requisitarDadosMPU6050_2() {
  /* Prenche as variaveis com os dados lidos
     Explicação:
      Os dados estao salvos em registers(endereções de memoria) do MPU.
      Para saber quais endereços veja o datasheet do MPU.
      É enviado o sinal para que o MPU comece a ler no register do acelerometro
      Cada leitura gera um inteiro de 16bits _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _
      No sensor cada endereço de memoria tem 8bits.
      Cada valor possui um register High e um register Low. Exemplo:
      Para ler é capturado os 8bits 'H':     Wire.read() = 0000 0000 0110 1010
      Eles sao movidos para a esquerda. Wire.read() << 8 = 0110 1010 0000 0000
      Sao lidos os proximos 8 bits.          Wire.read() = 0000 0000 0110 1010
      É realizada a operação bitewise 'or' ('|'). Valor  = 0110 1010 0110 1010

  */
  Wire.beginTransmission(MPU2_endereco);
  Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)?
  Wire.endTransmission(false);

  Wire.requestFrom(MPU2_endereco, 14, true); //Solicita os 14 registers do sensor
  //Armazena o valor dos sensores nas variaveis correspondentes
  Xac = Wire.read() << 8 | Wire.read(); //0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
  Yac = Wire.read() << 8 | Wire.read(); //0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  Zac = Wire.read() << 8 | Wire.read(); //0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  Tmp = Wire.read() << 8 | Wire.read(); //0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
  Xgi = Wire.read() << 8 | Wire.read(); //0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  Ygi = Wire.read() << 8 | Wire.read(); //0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  Zgi = Wire.read() << 8 | Wire.read(); //0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)

  TmpCelcius = Tmp / 340.00 + 36.53;
}


void acordarMPU6050() {
  Wire.beginTransmission(MPU1_endereco);
  Wire.write(0x6B);  //PWR_MGMT_1register?
  Wire.write(0); //Acordando o MPU-6050
  Wire.endTransmission(true);

  Wire.beginTransmission(MPU2_endereco);
  Wire.write(0x6B);  //PWR_MGMT_1register?
  Wire.write(0); //Acordando o MPU-6050
  Wire.endTransmission(true);

}


