#include <nRF24L01.h>
#include <printf.h>
#include <SPI.h>

#define PAYLOAD_WIDTH      35             // 30 bytes on TX payload
#define BROADCAST           0             // Endereço 0 indica transmissão BROADCAST
#define TAM_FIFO            100           // Tamanho da FIFO 100 bytes
#define N_SENSORS_WBAN       9            // Número de sensores na WBAN
#define N_MAX_SENSORS       12            // Número máximo de sensores e roteadores que o host pode gerenciar
#define WAIT_SENSOR         10000          // Maximum wait time for the answer from sensor in us
#define N_BYT_ADS_CFG       8             // Address sensor + Number of data bytes to CONFIG sensor
#define SERIAL_FIFO_WID     10            // Number of payloads in serialFIFO
#define MAX_SERIAL_RT       4             // Maximum number of retransmition of the same payload
#define WAIT_SERIAL_TX      2000          // Maximum wait time for the answer from PC
#define TIME_ROT            500           // Maximum wait time to find a router for a lost sensor
#define SIZE_FR             5             // Maximum number of sensors on vetF and vetR

// Comandos e MSGs CPU -> HOST e HOST -> CPU
#define DATA_SAMPLES 0x10
#define DATA_LOST_SAMPLES 0x20
#define END_SENSOR 0x00
#define MSG_SENSOR_NOK    0x31
#define MSG_END 0x00
#define OK  0x01
#define NOK 0x10
#define IDX_FAIL          5
#define IDX_END           5

// Definições da rotina de interrupção
#define RX_DR               6
#define TX_DS               5
#define MAX_RT              4

const byte TX_ADR_WIDTH =   5;            // 5 bytes TX(RX) address width
const byte SERIAL_DATA  =  40;            // Number of bytes in serial data payload
const int RFIRQ = 2;
const int RFCE  = 3;
const int RFCSN = 4;
const int LEDVD = 52;
const int LEDVM = 50;

byte  ADDR_HOST[TX_ADR_WIDTH]   = {0xc6,0xc2,0xc2,0xc2,0xc2}; // Define a static host adr
byte  ADDR_SENSOR[N_MAX_SENSORS] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x10}; // Define os endereços dos sensores

struct WBANSensors
{
  byte    E;                                        // Address sensor
  byte    F;                                        // Flag Fail
  byte    R;                                        // Flag Router
  byte    C;                                        // Address sensor to cancel
  bool    T;                                        // ReTx last payload. T = 0 Get new payload; T = 1 Get last payload
  int     P;                                        // Priority (ms)
  boolean I;                                        // More important
  int     A;                                        // Sensor aquisition time
  byte    S;                // 0x05 - Leitura dos dados do sensor   0x15 - reLeitura                      
}sensors[N_MAX_SENSORS];
byte idxF, idxE, pylInf, idxEMax, pldInf, vlFIFO;
unsigned long waitForRot;

byte vetF[SIZE_FR];                                 // Vetor to save address lost sensors

byte rx_buf[PAYLOAD_WIDTH];                         // Define lenght of rx_buf and tx_buf
byte tx_buf[PAYLOAD_WIDTH];
byte dataToSensor[N_BYT_ADS_CFG];                   // Sensor configuration data     

uint8_t sta;

unsigned long tempo, tempoAtual, serialTxTimer;
unsigned long T1, T2, DT;

// Variáveis de controle de prioridade
unsigned int tmrAux, tmrSys, tmrAquis;


uint8_t FIFO[TAM_FIFO];                             // Define FIFO software
uint8_t index_in = 0;
uint8_t index_out = 0;
uint8_t nDataFIFO = 0;
boolean FIFOempty = 1;

uint8_t serialPayload[PAYLOAD_WIDTH];               // Data buffer from Host to PC
uint8_t serialFromHost[PAYLOAD_WIDTH];              // Data buffer from Host to PC
uint8_t serialToPC[PAYLOAD_WIDTH];                  // Data buffer from Host to PC
uint8_t serialFIFO[SERIAL_FIFO_WID][PAYLOAD_WIDTH]; // Define FIFO to serial comunication
uint8_t serialPldFail[IDX_FAIL] = {0x7E, 0x03, END_SENSOR, MSG_SENSOR_NOK, 0x81};
uint8_t serialPldEnd[IDX_END] =   {0x7E, 0x03, END_SENSOR, MSG_END,        0x81};
uint8_t serialIndex_in = 0;
uint8_t serialIndex_out = 0;
uint8_t serialnDataFIFO = 0;
boolean serialFIFOempty = 1;
byte    serialTxStatus = 0;                         // Status of last transmition
byte    serialRC = 0;                               // Serial Retransmit Count

uint8_t sensorAtual = 0;                            // Define o próximo sensor a ser lido pelo Host
uint8_t payloadWidth = 0;
boolean pck = 0;

String msgHost = "Host running OK!";

boolean aquis = 0; 
boolean newPayload = 0;                             // Flag to indicate that there's a new payload sensor
byte TX_OK = 0;
byte RX_OK = 0;

byte inData;

uint8_t comando = 0;
uint8_t endSensor = 0;

/***************************************************/
uint8_t SPI_RW(uint8_t value)
{
  uint8_t SPIData;

  SPIData = SPI.transfer(value);

  return SPIData;                   // return SPI read value
}
/**************************************************/
uint8_t SPI_RW_Reg(uint8_t reg, uint8_t value)
{
  uint8_t status;

    digitalWrite(RFCSN,0);                      // CSN low, initiate SPI transaction£
    status = SPI_RW(reg);           // select register
    SPI_RW(value);                  // ..and write value to it..
    digitalWrite(RFCSN,1);                      // CSN high again  £¨rfcon^1

    return(status);                 // return nRF24L01 status byte
}
/**************************************************/
uint8_t SPI_Read_Status(void)
{
  uint8_t reg_val;

    digitalWrite(RFCSN,0);                          // CSN low, initialize SPI communication...
    reg_val = SPI_RW(NOP);                            // ..then read register value
    digitalWrite(RFCSN,1);                          // CSN high, terminate SPI communication RF
    return(reg_val);                                // return register value
}
/**************************************************/
uint8_t SPI_Read(uint8_t reg)
{
  uint8_t reg_val;

    digitalWrite(RFCSN,0);                          // CSN low, initialize SPI communication...
    SPI_RW(reg);                                    // Select register to read from..
    reg_val = SPI_RW(0);                            // ..then read register value
    digitalWrite(RFCSN,1);                          // CSN high, terminate SPI communication RF
    return(reg_val);                                // return register value
}
/**************************************************/
uint8_t SPI_Read_Buf(uint8_t reg, uint8_t *pBuf, uint8_t bytes)
{
  uint8_t status,byte_ctr;

    digitalWrite(RFCSN,0);                                    // Set CSN low, init SPI tranaction
    status = SPI_RW(reg);                         // Select register to write to and read status byte

    for(byte_ctr=0;byte_ctr<bytes;byte_ctr++)
      pBuf[byte_ctr] = SPI_RW(0);                 // Perform SPI_RW to read byte from nRF24L01

    digitalWrite(RFCSN,1);                                    // Set CSN high again

    return(status);                               // return nRF24L01 status byte
}
/**************************************************/
uint8_t SPI_Write_Buf(uint8_t reg, uint8_t *pBuf, uint8_t bytes)
{
  uint8_t status,byte_ctr;

    digitalWrite(RFCSN,0);                        // Set CSN low, init SPI tranaction
    status = SPI_RW(reg);                         // Select register to write to and read status byte
    for(byte_ctr=0; byte_ctr<bytes; byte_ctr++)   // then write all byte in buffer(*pBuf)
       SPI_RW(*pBuf++);
 
    digitalWrite(RFCSN,1);                        // Set CSN high again
    return(status);                               // return nRF24L01 status byte
}

/**************************************************/
void rf_init(void)
{
  // Radio + SPI setup
  pinMode(RFIRQ, INPUT);  // Define RFIRQ as input to receive IRQ from nRF24L01+
  pinMode(RFCE, OUTPUT);  // Define RFCE as output to control nRF24L1+ Chip Enable
  pinMode(RFCSN, OUTPUT); // Define RFCSN as output to control nRF24L1+ SPI
  SPI.begin();            // start the SPI library: 
  newPayload = 0;
  sta = 0;
  TX_OK = 0;
  RX_OK = 0;
  digitalWrite(RFCSN,1);                        // Set CSN low, init SPI tranaction
  digitalWrite(RFCE,0);                         // Radio chip enable low
  SPI_Write_Buf(W_REGISTER + TX_ADDR, ADDR_HOST, TX_ADR_WIDTH);
  SPI_Write_Buf(W_REGISTER + RX_ADDR_P0, ADDR_HOST, TX_ADR_WIDTH);
  SPI_RW_Reg(W_REGISTER + EN_AA, 0x00);        // Disable Auto.Ack:Pipe0
  SPI_RW_Reg(W_REGISTER + EN_RXADDR, 0x01);    // Enable Pipe0 (only pipe0)
  SPI_RW_Reg(W_REGISTER + AW, 0x03);           // 5 bytes de endereço
  SPI_RW_Reg(W_REGISTER + SETUP_RETR, 0x00);   // Tempo de retransmissão automática de 250us, retransmissão desabilitada
  SPI_RW_Reg(W_REGISTER + RF_CH, 90);          // Select RF channel 90. Fo = 2,490 GHz
  SPI_RW_Reg(W_REGISTER + RF_SETUP, 0x07);     // TX_PWR:0dBm, Datarate:1Mbps, LNA:HCURR
  SPI_RW_Reg(W_REGISTER + DYNPD, 0x01);        // Ativa Payload dinâmico em data pipe 0
  SPI_RW_Reg(W_REGISTER + FEATURE, 0x07);      // Ativa Payload dinâmico, com ACK e comando W_TX_PAY
  SPI_RW_Reg(FLUSH_TX,0);
  SPI_RW_Reg(FLUSH_RX,0);
  SPI_RW_Reg(W_REGISTER+NRF_STATUS,0x70);
}
/**************************************************/
void pushFIFO(uint8_t dado)
{

  FIFO[index_in] = dado;
  index_in++;
  
  if(index_in == TAM_FIFO) 
    index_in = 0;
  if(nDataFIFO<TAM_FIFO)
    nDataFIFO++;
  FIFOempty = 0;
}
/**************************************************/
uint8_t popFIFO()
{
  uint8_t  aux;
  
  if(nDataFIFO > 0)
  {
    aux = FIFO[index_out];
    index_out++;
    if(index_out == TAM_FIFO)
      index_out = 0;
    nDataFIFO--;
    return aux;
  }
  else
  {
    FIFOempty = 1;
    return 0;
  }
}

/**************************************************/
/* Recebe os dados do payload dataFromSensor e armazena na FIFO */
void pushSerialFIFO()
{
  uint8_t i;
  for(i=0;i<serialFromHost[1];i++)
    serialFIFO[serialIndex_in][i] = serialFromHost[i];
  serialIndex_in++;
  
  if(serialIndex_in == SERIAL_FIFO_WID) 
    serialIndex_in = 0;
  if(nDataFIFO<TAM_FIFO)
    serialnDataFIFO++;
  serialFIFOempty = 0;
}

/**************************************************/
/* Retira o payload do topo da FIFO e o armazena no dataToPC */
void popSerialFIFO()
{
  uint8_t i;
  
  if(serialnDataFIFO > 0)
  {
    for(i=0;i<serialFIFO[serialIndex_in][1];i++)
      serialToPC[i] = serialFIFO[serialIndex_out][i];
    serialIndex_out++;
    if(serialIndex_out == SERIAL_FIFO_WID)
      serialIndex_out = 0;
    serialnDataFIFO--;
    return;
  }
  else
  {
    serialFIFOempty = 1;
    return;
  }  
}

/***************************************************/
void serialTxFlush()
{
  serialIndex_in = 0;
  serialIndex_out = 0;
  serialnDataFIFO = 0;
  serialFIFOempty = 1;
}

/**************************************************/
void RX_Mode(void)
{
  newPayload = 0;
  sta = 0;
  RX_OK = 0;
  digitalWrite(RFCE,0);                         // Radio chip enable low -> Standby-1
  SPI_RW_Reg(W_REGISTER + CONFIG, 0x1F);        // Set PWR_UP bit, enable CRC(2 bytes) & Prim:RX. RX_DR enabled..
  digitalWrite(RFCE,1);                         // Set CE pin high to enable RX Mode
}

/**************************************************/
void TX_Mode_NOACK(uint8_t payloadLength)
{
  digitalWrite(RFCE,0);                                            // Radio chip enable low -> Standby-1
  SPI_RW_Reg(W_REGISTER + CONFIG, 0x1E);                           // Set PWR_UP bit, enable CRC(2 bytes) & Prim:TX. RX_DR enabled.
  SPI_Write_Buf(W_TX_PAYLOAD_NOACK, tx_buf, payloadLength);        // Writes data to TX payload
                                                                   // Endereço da porta P2, matrizes tx_buf (), o comprimento da matriz é enviada)
  TX_OK = 0;
  digitalWrite(RFCE,1);                                            // Set CE pin high to enable TX Mode
  delayMicroseconds(12);
  digitalWrite(RFCE,0);                                            // Radio chip enable low -> Standby-1
  tempo = micros() + 1000;
  do
  {
    tempoAtual = micros();
    if (!digitalRead(RFIRQ))
      RF_IRQ();
  }while (!((TX_OK)|(tempoAtual>tempo)));
  if((tempoAtual>tempo))
    Serial.println("Time Out!");Serial.print('\n');Serial.print('\0');
}

/**************************************************/
void RF_IRQ(void) 
{
  sta=SPI_Read(NRF_STATUS);  
  if(bitRead(sta,RX_DR))                                  // if receive data ready (RX_DR) interrupt
  {  
    RX_OK = 1;
    newPayload = 1;
    SPI_Read_Buf(R_RX_PAYLOAD,rx_buf,PAYLOAD_WIDTH);     // read receive payload from RX_FIFO buffer    
    payloadWidth = SPI_Read(R_RX_PLD_WIDTH);              // Retorna o número de bytes no payload recebido
    if(payloadWidth > 32) 
    {
      payloadWidth = 0;
      newPayload = 0;
    }
    SPI_RW_Reg(FLUSH_RX,0);
  }
  if(bitRead(sta,TX_DS))
  {
    TX_OK = 1;
    SPI_RW_Reg(FLUSH_TX,0);
  }
  SPI_RW_Reg(W_REGISTER+NRF_STATUS,0x70);
}

/**************************************************/
byte seekIdxSen(byte addrSen)
{
  int i;
  
  for(i=0;i < N_SENSORS_WBAN;i++)
  {
    if(sensors[i].E == addrSen)
      return(i);
  }
}

/**************************************************/
void configSensor(void)               // Command: 0x01
{
  byte i, idxSenCfg;
  
  tx_buf[0] = dataToSensor[0];        // Sensor address
  tx_buf[1] = 0x01;                   // Command CONFIG SENSOR
  for(i=1;i<N_BYT_ADS_CFG;i++)
  {
   tx_buf[i+1] = dataToSensor[i];
  }
  idxSenCfg = seekIdxSen(tx_buf[0]);
  sensors[idxSenCfg].A = dataToSensor[N_BYT_ADS_CFG-2]*256 + dataToSensor[N_BYT_ADS_CFG-1];
  sensors[idxSenCfg].P = sensors[idxSenCfg].A;
  // Start first config of the sensor
  TX_Mode_NOACK(9);
  RX_Mode();
  tempo = micros() + WAIT_SENSOR;
  do
  {
    tempoAtual = micros();
    if (!digitalRead(RFIRQ))
      RF_IRQ();
  }while (!((RX_OK)||(tempoAtual>tempo)));
  if((tempoAtual>tempo)&&(!RX_OK))
  {
      // Failed the first reading of the sensor
      // Start second reading of the sensor
      TX_Mode_NOACK(9);
      RX_Mode();
      tempo = micros() + WAIT_SENSOR;
      do
      {
        tempoAtual = micros();
        if (!digitalRead(RFIRQ))
          RF_IRQ();
      }while (!((RX_OK)||(tempoAtual>tempo)));
  }
  serialPayload[0] = 0x7E;                  // Payload delimiter
  serialPayload[1] = 0x04;                  // Number of bytes
  serialPayload[2] = dataToSensor[0];       // Sensor address  
  serialPayload[3] = 0x40;                  // Command Status
  serialPayload[4] = rx_buf[2];             // Data received = 0x10: FAIL  -  0x01: OK
  serialPayload[5] = 0x81;                  // Payload delimiter
  if(!newPayload)
    serialPayload[4] = 0x10;                // 0x10: FAIL
  for(i=0;i<6;i++)
    SerialUSB.write(serialPayload[i]);
}

/**************************************************/
void disparaAquis(void)               // Command: 0x02
{
  msgHost = "POLLING";
  SerialUSB.write('P');
  delay(100);  
  digitalWrite(LEDVM,LOW);
  for(int i = 0;i < N_MAX_SENSORS;i++)
  {
    sensors[i].F = 0; sensors[i].R = 0;
    sensors[i].C = 0; sensors[i].T = 0;
  }
  tx_buf[0] = BROADCAST;   // Mensagem em Broadcast
  tx_buf[1] = 0x02;        // Comando DISPARA AQUISIÇÃO
  TX_Mode_NOACK(2);
  sta = 0;
  aquis = 1;
  serialTxStatus = 0x00;
  tmrSys = millis();
}
/**************************************************/
void cancelaAquis(void)               // Command: 0x03
{
  int i;
  
  tx_buf[0] = BROADCAST; // Mensagem em Broadcast
  tx_buf[1] = 0x03;      // Comando CANCELA AQUISIÇÃO
  TX_Mode_NOACK(2);
  msgHost = "READING SENSORS!";
  Serial.println(" ");Serial.println(msgHost);
  aquis = 0;  
  comando = 0x00;
  delay(100);  
  for(i=0; i < N_SENSORS_WBAN;i++)
  {
    do
    {
      idxE = i;
      readSensor();
    }while(serialPayload[1]>3);
  }  
  digitalWrite(LEDVM,LOW);
  RX_Mode();
  for(i=0;i<5;i++)                                          // Width serialPldEnd is 5
    SerialUSB.write(serialPldEnd[i]);
  serialTxFlush();
  msgHost = "Host stopped!";
  Serial.println(" ");Serial.println(msgHost);
}

/**************************************************/
void statusSensor(void)               // Command: 0x04
{
  msgHost = "Comando STATUS DOS SENSORES: 0x04 - Ainda não implementado";
  SerialUSB.println(msgHost);
}

/**************************************************/
void statusSenAttributes(void)        // Command: 0x05
{
  msgHost = "Comando ATRIBUTO DOS SENSORES: 0x05 - Ainda não implementado";
  SerialUSB.println(msgHost);
}

/**************************************************/
void statusHost(void)                 // Command: 0x06
{
  msgHost = "Comando STATUS DO HOST: 0x06 - Ainda não implementado";
  SerialUSB.println(msgHost);
}

/**************************************************/
void resetSensor(void)                // Command: 0x08
{
  byte i;
  
  //SerialUSB.println("Funcao resetSensor");
  
  tx_buf[0] = BROADCAST;              // Mensagem em Broadcast
  tx_buf[1] = 0x08;                   // Command RESET SENSOR
  TX_Mode_NOACK(2);
  RX_Mode();
  serialPayload[0] = 0x7E;                  // Payload delimiter
  serialPayload[1] = 0x04;                  // Number of bytes
  serialPayload[2] = 0x00;                  // Address Broadcast  
  serialPayload[3] = 0x40;                  // Command Status
  serialPayload[4] = 0x01;                  // RESET OK!  
  serialPayload[5] = 0x81;                  // Payload delimiter
  aquis = 0;  
  comando = 0x00;
  serialTxFlush();
  index_in = 0;
  index_out = 0;
  nDataFIFO = 0;
  FIFOempty = 1;
  serialIndex_in = 0;
  serialIndex_out = 0;
  serialnDataFIFO = 0;
  serialFIFOempty = 1;
  serialTxStatus = 0;                         // Status of last transmition
  serialRC = 0;                               // Serial Retransmit Count
  sensorAtual = 0;                            // Define o próximo sensor a ser lido pelo Host
  payloadWidth = 0;
  pck = 0;
  aquis = 0; 
  newPayload = 0;                             // Flag to indicate that there's a new payload sensor
  TX_OK = 0;
  RX_OK = 0;
  inData;
  pldInf;
  comando = 0;
  endSensor = 0;
  RX_Mode();
  for(i=0;i<6;i++)
    SerialUSB.write(serialPayload[i]);    
}

/**************************************************/
void ctlReadSensors()
{
  boolean flagR, flagFR;

  idxE = nextSensor();
  readSensor();
  saveVetF(idxE);
}

/**************************************************/
byte nextSensor()
{
  int i = 1, auxE, auxP;

  idxE++;
  if(idxE > N_SENSORS_WBAN-1) 
    idxE=0;
  return(idxE);
}
/**************************************************/
void readSensor(void)                     // Command: 0x55
{  
  tx_buf[0] = sensors[idxE].E;            // Next sensor Address
  tx_buf[1] = sensors[idxE].S;            // Command READ/reREAD SENSOR DATA
//    T2 = micros();
//  DT = T2 - T1;
//  Serial.println(DT);
//  T1 = micros();
  TX_Mode_NOACK(2);      
  RX_Mode();

  digitalWrite(LEDVM,!digitalRead(LEDVM));
  
//  if(sensors[idxE].S == 0x15)
//  {
//    Serial.print(sensors[idxE].E, HEX);
//    delay(2);
//  }
  tempo = micros() + WAIT_SENSOR;
  do
  {
    tempoAtual = micros();
    if (!digitalRead(RFIRQ))
      RF_IRQ();
  }while (!((RX_OK)||(tempoAtual>tempo)));
  delayMicroseconds(85);
 
  pldInf = sendDataHost();
//  if(pldInf != 0x80)
//  {
//    if(sensors[idxE].F == 0x01)
//      restoreSenLost(idxE);
//  }
}

/**************************************************/
byte sendDataHost()
{
  byte nData,j,i;
//  SerialUSB.println("Funcao sendDataHost ");
  j = 0;
  if(newPayload == 1)
  {
    sensors[idxE].S = 0x05;                               // Status da leitura: SUCESSO
    nData = rx_buf[2];                                    // Amount of data bytes from sensor
    serialPayload[0] = 0x7E;                              // Payload delimiter
    serialPayload[1] = nData + 3;                         // Number of bytes in payload
    serialPayload[2] = rx_buf[0];                         // Sensor address
    vlFIFO = rx_buf[1] & 0x0F;                            // Filling level of the FIFO
    pldInf = rx_buf[1] & 0xF0;                            // information about payload
    if((pldInf&0x60) == 0x40)
      serialPayload[3] = 0x20 + vlFIFO;
    else
      serialPayload[3] = 0x10 + vlFIFO;
    if(nData != 0)
    {
      for(j=0;j<nData;j++)         
        serialPayload[j+4] = rx_buf[j+3];                   // Biomedical data
    }  
    serialPayload[j+4] = 0x81;  
    i = j + 5;  
    for(j=0;j<i;j++)       
      SerialUSB.write(serialPayload[j]);
  }
  else
  {
    sensors[idxE].S = 0x15;                                   // Status da leitura: FALHA
    serialPldFail[2] = tx_buf[0];
    for(j=0;j<IDX_FAIL;j++)                                          // Width serialPldFail is 6
      SerialUSB.write(serialPldFail[j]);
    pldInf = 0x80;  
  }
  return(pldInf);
}

/**************************************************/
void saveVetF(byte idxSensor)
{ 
  byte flagFR;

  if(pldInf == 0x80)
  {
    if(idxF < SIZE_FR)
    {
      flagFR = sensors[idxSensor].F;
      if(flagFR == 0)
      {
        vetF[idxF] = idxSensor;
        sensors[idxSensor].F = 1;
        idxF++;
      }
    }
  }  
}

/**************************************************/
void restoreSenLost(byte idxSensor)
{
  byte aux, j;

  sensors[idxSensor].F = 0;
  j = 0;
  do
  {
    if(vetF[j] == idxSensor)
    {
      removeSenVetF(j);  
      return;
    }
    else
      j++;
  }while(j < idxF);
}

/**************************************************/
void removeSenVetF(byte idxSensor)
{
    while(idxSensor < (idxF - 1))
    {
      vetF[idxSensor] = vetF[idxSensor + 1];
      idxSensor++;
    }
    idxF--;
}

/**************************************************/
void comandaWBAN(void)
{
    switch(comando)
      {
        case (0x01):
        {
          // LÊ CONFIGURAÇÃO DOS SENSORES
          configSensor();
          pck = 0;
          break;
        }
        case (0x02):
        {
          // DISPARA SENSORES - BROADCAST
          disparaAquis();
          sensorAtual = 0;
          pck = 0;
          RX_Mode();
          delay(5);
          break;
        }
        case (0x03):
        {
          // CANCELA AQUISIÇÃO - BROADCAST
          cancelaAquis();
          pck = 0;
          break;
        }
        case (0x04):
        {
          // ENVIA STATUS DOS SENSORES
          statusSensor();
          pck = 0;
          break;
        }       
        case (0x05):
        {
          // ENVIA ATRIBUTOS DOS SENSORES
          statusSenAttributes();
          pck = 0;
          break;
        }       
        case (0x06):
        {
          // ENVIA STATUS DO HOST
          statusHost();
          pck = 0;
          break;
        }        
        case (0x07):
        {
          break;
        }          
        case (0x08):
        {
          resetSensor();
          pck = 0;
          break;
        }    
        case (0x09):
        {
          host_init();
          break;
        }         
        case (0x24):
        {
          // MENSAGEM COMPUTADOR - STATUS ATUAL DO SENSOR
          pck = 0;
          break;
        }
        case (0x55):
        {
          // CONTROLA A LEITURA DOS DADOS DOS SENSORES E ENVIA PAYLOAD PARA CPU
          ctlReadSensors();
          break;
        }         
        default:
          break;
      }
      if (aquis)
        comando = 0x55;   // DADOS DO SENSOR
      else
        comando = 0x00;
}

/**************************************************/
void processSerial(void)
{
  static boolean configSensor = 0;
  uint8_t dado;
  byte i;

  dado = popFIFO();
  if(FIFOempty)
    return;
  if(!pck)
  { 
    if(dado == 0x7E)
    {
      pck = 1;  // Indica que um novo pacote pode estar iniciando
      dado = popFIFO();
      if(FIFOempty)
        return;
    }    
  }
  if(pck)
      switch(dado)
      {
        case(0x01):
        {
          comando = 0x01;           // Indica que o pacote é do tipo CONFIGURA SENSOR 
          tempo = millis() + 10;    // 10 milliseconds to receive the rest of the command data
          do
          {
            tempoAtual = millis();
            serialRxPC();
          }while (!(tempoAtual>tempo));
          for(i=0;i<N_BYT_ADS_CFG;i++)
            dataToSensor[i] = popFIFO();          
          break;
        }
        case(0x02):
        {
          comando = 0x02;           // Indica que o pacote é do tipo DISPARA AQUISIÇÃO
          break;
        }
        case(0x03):
        {
          comando = 0x03;           // Indica que o pacote é do tipo CANCELA AQUISIÇÃO 
          break;
        }
        case(0x04):
        {
          comando = 0x04;           // Indica que o pacote é do tipo STATUS DOS SENSORES 
          break;
        }   
        case(0x05):
        {
          comando = 0x05;           // Indica que o pacote é do tipo ATRIBUTOS DOS SENSORES 
          break;
        }              
        case(0x06):
        {
          comando = 0x06;           // Indica que o pacote é do tipo STATUS DO HOST
          break;
        }
        case(0x07):
        {
          comando = 0x07;           // Indica que o pacote é do tipo 
          break;
        }       
        case(0x08):
        {
          comando = 0x08;           // Indica que o pacote é do tipo RESET SENSOR 
          break;
        }  
        case(0x09):
        {
          comando = 0x09;           // Indica que o pacote é do tipo RESET HOST
          break;
        }            
        default:
        {
          pck = 0;                  // Indica que não foi um início de pacote válido
          break;
        }
      }
}

/***************************************************/
void serialRxPC() 
{
  while(SerialUSB.available()>0)
  {
    // get the new byte:
    inData = SerialUSB.read();
    pushFIFO(inData);
  }
}

void host_init()
{
  int i;
  for(i=0;i<N_MAX_SENSORS;i++)
  {
    sensors[i].F = 0;
    sensors[i].R = 0;
    sensors[i].C = 0;
    sensors[i].T = 0;
    sensors[i].P = 10000;    
    sensors[i].I = 0;
    sensors[i].A = 100;
    sensors[i].S = 0x05;    // 0x05 - Leitura dos dados do sensor   0x15 - reLeitura
  }
  for(i=0;i<N_SENSORS_WBAN;i++)
    sensors[i].E = ADDR_SENSOR[i];
  for(int j = 0; j < N_SENSORS_WBAN; j++)
  {
    Serial.print("<");Serial.print(j+1);Serial.print("> : sensors[x].P = ");Serial.print(sensors[j].P);Serial.print(" sensors[x].A = ");Serial.println(sensors[j].A);
  }    
  delay(200);
  idxEMax = N_SENSORS_WBAN;
  idxE = 0;
  idxF = 0;
  pldInf = 0;
  tmrSys = millis();
}
/***************************************************/
void setup() 
{
  pinMode(LEDVM,OUTPUT);
  SerialUSB.begin(921600);
  Serial.begin(115200);
  host_init();
  rf_init();
}

/***************************************************/
void loop() 
{ 
  
  while(1)
  {
    serialRxPC(); 
    processSerial();
    comandaWBAN();
  }
}
