/*
    Feito com base nos exemplos:
      MPU6050_raw;
      HMC5883L_raw;
      MPU6050_DMP6;
      Disponiveis pelas bibliotecas "I2Cdev".
      Podem ser encontrados em: https://github.com/jrowberg/i2cdevlib
*/

//Biblioteca com codigos para o MPU6050 e HMC588L
#include "I2Cdev.h"
//Bibliotecas para especificas para o HMC5883L, MPU6050 e sua DMP
//Podem ser encontradas junto a I2Cdev
#include "HMC5883L.h"
#include "MPU6050_6Axis_MotionApps20.h"
//#include "MPU6050.h" //Nao necessario qnd se faz o uso da MotionApps

//Biblioteca padrao do arduino para comunicação i2c
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
#include "Wire.h"
#endif

/*Definiçao de objetos:
      HMC5883 no endereço 0x1E
      MPU6050 no endereço 0x68
    Estes endereço é o "default" da biblioteca usada
    Caso alimente a MPU6050 com 5V ao inves de 3.3V o endereço
  sera alterado para 0x69 e devera ser usado como argumento na
  contruçcao do objeto mpu6050. De acordo com a linha abaixo:
   //MPU6050 mpu(0x69);
   É necessario conecar a interrupção entre o pino 2 do arduino(uno) e a MPU6050
   para este codigo
*/
#define INTERRUPT_PIN 2 //O pino 2 é o de interrupção para o  arduino uno. Verifique em outras placas

HMC5883L mag;
MPU6050 mpu;

// Variaveis de controle e status para MPU6050
bool dmpReady = false;  // se a inicialização do DMP foi bem sucedida é atribuido o valor true
uint8_t mpuIntStatus;   // Guarda o status atual da interrupção do MPU
uint8_t devStatus;      // Retorna o status apos cada opração com a DMP (0 = sucessom, !0 = erro)
uint16_t packetSize;    // É esperado que seja o tamanho de pacote do DM (default é 42 bytes)
uint16_t fifoCount;     // Quantidade de bytes na FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer

//Variaveis para armazenamento dos dados lidos
int16_t ax, ay, az;   //Acelerometro
int16_t gx, gy, gz;   //Giroscopio
int16_t mx, my, mz;   //Magnetometro
Quaternion q;         // [w, x, y, z] Quartenion

//Sera piscado o LED conectado ao pino 13 durante as leituras
#define LED_PIN 13
bool blinkState = false;

// INTERRUPÇÃO: Rotina de detecão da interrupção
volatile bool mpuInterrupt = false;     // indica se o pino de interrupção do MPU foi para HIGH
void dmpDataReady() {
  mpuInterrupt = true;
}

void setup() {
  //Inicia o I2C
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
  Wire.begin();
  Wire.setClock(400000); // 400kHz I2C clock. Caso tenha problemas com a compilação comentar esta linha
#elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
  Fastwire::setup(400, true);
#endif

  //Defini como 38400 mas tlvz seja necessario maior ou menor velocidade
  //Estimando de maneira rapida cada linha de dados tera 680 bits
  //Isso da uma frequencia maxima de cerca de 55Hz (ESTIMADA)
  //TODO: Calcular isso de forma correta
  Serial.begin(38400);
  while (!Serial); // No arduino Leonardo é necessário aguardar o serial ser enumerado, em outras seguira em frente imediatamente

  //Inicializando os dispositivos
  Serial.println(F("Inicializando os dispositivos I2C..."));
  /************TODO: Verificar se funciona a inicialização da mpu e do mpu********/
  //TODO: Refactor Delete para a variavel accel giro
  mpu.initialize();
  mag.initialize();
  pinMode(INTERRUPT_PIN, INPUT);

  //Verificando conexoes
  Serial.println(F("Testando as conexões..."));
  Serial.println(mag.testConnection() ? F("HMC5883L conectado com sucesso") : F("HMC5883L falhou ao conectar"));
  Serial.println(mpu.testConnection() ? F("MPU6050 conectado com sucesso") : F("MPU6050 falhou ao conectar"));

  //Informacoes sobre as leituras
  Serial.println(F("Formato de saida dos dados:"));
  Serial.println(F("Acelerometro \tGiroscopio   \tMagnetometro \tQuaternion    \t"));
  Serial.println(F("Xac\tYac\tZac\tXgi\tYgi\tZgi\tXma\tYma\tZma\tWq\tIq\tJq\tKq\t"));

  Serial.println(F("*****Inicializando a dmp*****"));
  devStatus = mpu.dmpInitialize();

  //TODO: Colocar aqui definições de offset MPU6050
  /*
    mpu.setXGyroOffset(220);
    mpu.setYGyroOffset(76);
    mpu.setZGyroOffset(-85);
    mpu.setZAccelOffset(1788);
  */

  if (devStatus == 0) {
    // Ligando o DMP, agora estará pronto pra uso
    Serial.println(F("Ligando o DMP"));
    mpu.setDMPEnabled(true);

    //Ativando a detecção de interrupção do arduino
    Serial.println(F("Ativando detecção de interrupção (Arduino external interrupt 0)..."));
    attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), dmpDataReady, RISING);
    mpuIntStatus = mpu.getIntStatus();

    // Configura a flag DMP Ready para que no loop se saiba que esta ok para usar
    Serial.println(F("DMP ready! Aguardando primeira interrupção..."));
    dmpReady = true;

    // obtem o tamanho esperado do pacote do DMP para comparação futura
    packetSize = mpu.dmpGetFIFOPacketSize();
  } else {
    // ERRO!
    // 1 = initial memory load failed
    // 2 = DMP configuration updates failed
    // (if it's going to break, usually the code will be 1)
    Serial.print(F("DMP Initialization failed (code "));
    Serial.print(devStatus);
    Serial.println(F(")"));
  }

  //Configurando o LED que ira piscar
  pinMode(LED_PIN, OUTPUT);
}

void loop() {

  // se falhou sai do loop
  if (!dmpReady) return;

  // Aguarda a interrupção do MPU para pacote extras estar disponivel
  while (!mpuInterrupt && fifoCount < packetSize) {
    //TODO: Ué?
    // other program behavior stuff here
    // .
    // .
    // .
    // if you are really paranoid you can frequently test in between other
    // stuff to see if mpuInterrupt is true, and if so, "break;" from the
    // while() loop to immediately process the MPU data
    // .
    // .
    // .
  }

  // reset interrupt flag and get INT_STATUS byte
  mpuInterrupt = false;
  mpuIntStatus = mpu.getIntStatus();

  // get current FIFO count
  fifoCount = mpu.getFIFOCount();

  // check for overflow (this should never happen unless our code is too inefficient)
  if ((mpuIntStatus & 0x10) || fifoCount == 1024) {
    // reset so we can continue cleanly
    mpu.resetFIFO();
    Serial.println(F("FIFO overflow!"));

    // otherwise, check for DMP data ready interrupt (this should happen frequently)
  } else if (mpuIntStatus & 0x02) {
  
    //Pequena(realmente pequena) espera pelo tamanho correto dos dados disponiveis. 
    while (fifoCount < packetSize) fifoCount = mpu.getFIFOCount();
    
    //Le um pacote da FIFO
    mpu.getFIFOBytes(fifoBuffer, packetSize);

    // Verifica a FIFO count no caso que haja > 1 pacotes disponiveis
    // (isso permite que se leira mais imediatamente, sem esperar por uma interrupção
    fifoCount -= packetSize;
    
    /*******REALIZA LEITURAS*********/
    //Realiza leitura do Quaternion
    mpu.dmpGetQuaternion(&q, fifoBuffer);
    //Realiza leitura do HMC5883L
    mag.getHeading(&mx, &my, &mz);
    //Realiza leitura do MPU6050
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    /***********MOSTRANDO*************/
    //Mostra os todos os dados lidos com a formatacao selecionada ao escrever o codigo
    mostrarDados();

    //LED piscante para indicar atividade
    blinkState = !blinkState;
    digitalWrite(LED_PIN, blinkState);
  }
}

void mostrarDados() {
  //Acelerometro:
  Serial.print(ax); Serial.print("\t");
  Serial.print(ay); Serial.print("\t");
  Serial.print(az); Serial.print("\t");
  //Giroscopio:
  Serial.print(gx); Serial.print("\t");
  Serial.print(gy); Serial.print("\t");
  Serial.print(gz); Serial.print("\t");
  //Magnetometro:
  Serial.print(mx); Serial.print("\t");
  Serial.print(my); Serial.print("\t");
  Serial.print(mz); Serial.print("\t");
  //Quaternion:
  Serial.print(q.w); Serial.print("\t");
  Serial.print(q.x); Serial.print("\t");
  Serial.print(q.y); Serial.print("\t");
  Serial.println(q.z);
}
