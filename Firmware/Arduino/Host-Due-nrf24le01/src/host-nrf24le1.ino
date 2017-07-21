/* UNIVERSIDADE FEDERAL DE UBERLANDIA
   BIOLAB - Biomedical Engineering Lab

   Autor: Ítalo G S Fernandes
   contact: italogsfernandes@gmail.com
   URLs: www.biolab.eletrica.ufu.br
          https://github.com/BIOLAB-UFU-BRAZIL
   NOTE:*************FUNCIONA APENAS COM O ARDUINO DUE*************
   Este códido faz parte do projeto da competição do cobec.
   O que faz:
      Realiza leitura de 1 sensor inercial sem fio e envia para pc
      TODO: Realizara a leitura de um emg, filtra e enviara para pc

   TODO:
     Obter offsets do sensor inercial
     Verificar "NOTE" espalhados no codigo.
     Verificar "TODO" espalhados no codigo.

   Pacotes:
   Inercial(Quaternion): (11 bytes)
   ['$'] ['Q'] [WH] [WL] [XH] [XL] [YH] [YL] [ZH] [ZL] ['\n']
   EMG(Valores do ADC passados após Media Móvel): (5 bytes)
   ['$'] ['E'] [EMGH] [EMGL] ['\n']

    Esquema de montagem:
    Arduino - Dispositivo
    13      - LED
    A0      - EMG-sinal (0 a 3.3V)
    GND     - EMG-GND
    3.3V    - VCC do nrf24le01
    GND     - GND do nrf24le01
    SPI-1   - MISO do nrf24le01
    SPI-3   - SCK do nrf24le01
    SPI-4   - MOSI do nrf24le01
    2       - IRQ do nrf24le01
    3       - CE do nrf24le01
    4       - CSN do nrf24le01
   Para visualizar de forma visivel ao ser humano
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

///////////////
// Libraries //
///////////////
#include <nrf24le01Module.h>
#include "DueTimer.h"

/////////////
// Defines //
/////////////
#define POLLING_TIMEOUT 500000 // 500 milliseconds of maximum wait time. tempo medio: 385
#define UART_TIMEOUT 2 //2 milliseconds of maximum wait time por byte.
#define MPUsampFreq 100
#define LED_STATUS 13
#define UART_BAUDRATE 115200
//Subenderecos usados no sistema
#define HOST_SUB_ADDR 0xFF //Sub addr do host
#define INERTIAL_SUB_ADDR 0x01
//Comandos
#define CMD_READ (uint8_t) 0x07 //Request a packet of readings
#define CMD_LIGHT_UP_LED (uint8_t) 0x0C
#define CMD_TURN_OFF_LED (uint8_t) 0x0D
#define CMD_FALSE_PACKET (uint8_t) 0xAA //Request a false packet of readings

///////////////
// Variables //
///////////////
String serialOp;
nrf24le01Module host_nrf(2,3,4); //IRQ, CE, CSN
unsigned long timeout_init_time, timeout_actual_time; //Polling and other timeouts variables

///////////////
//Functions //
//////////////
void requestInertialData();
void sendInercialCMDFALSE();
void sendInercialCMDLED(bool estadoLed);
void send_inertial_data();
void wait_rf_response();
void piscar_led(){digitalWrite(LED_STATUS, HIGH); delay(500);  digitalWrite(LED_STATUS, LOW); delay(500);}

/////////////////
//Arduino Main //
////////////////
void setup(){
    pinMode(LED_STATUS, OUTPUT);
    Serial.begin(UART_BAUDRATE);
    host_nrf.rf_init(host_nrf.ADDR_HOST,host_nrf.ADDR_HOST,10,RF_DATA_RATE_2Mbps,RF_TX_POWER_0dBm); //RF Communication
    piscar_led(); piscar_led();//Piscadas
    DEBUG_PRINT_("Arduino HOST Iniciado...\n");
    DEBUG_PRINT_("Insira um comando: CMDSTART, CMDSTOP, CMDLEDHIGH, CMDLEDLOW, CMDFALSE\n");
}

void loop(){
    //Menu
    if (Serial.available() > 0)
    {
        serialOp = Serial.readString();
        if (serialOp == "CMDSTART") {
            digitalWrite(LED_PIN, HIGH);
            Timer3.attachInterrupt(requestInertialData).start(1); //1 Hz
        } else if (serialOp == "CMDSTOP") {
            digitalWrite(LED_PIN, LOW);
            Timer3.stop();
        } else if (serialOp == "CMDLEDHIGH") {
            digitalWrite(LED_PIN, HIGH);
            sendInercialCMDLED(HIGH);
        } else if (serialOp == "CMDLEDLOW") {
            digitalWrite(LED_PIN, LOW);
            sendInercialCMDLED(LOW);
        } else if (serialOp == "CMDFALSE") {
            piscar_led();
            sendInercialCMDFALSE();
        }
    }
}

/////////////////////////
//Inertial RF commands //
////////////////////////
void requestInertialData(){
    host_nrf.tx_buf[0] = INERTIAL_SUB_ADDR;
    host_nrf.tx_buf[1] = CMD_READ;
    host_nrf.TX_Mode_NOACK(2);
    wait_rf_response();
    if(host_nrf.newPayload){
        host_nrf.sta = 0;
        host_nrf.newPayload = 0;
        send_inertial_data();
    }
}

void sendInercialCMDFALSE(){
    host_nrf.tx_buf[0] = INERTIAL_SUB_ADDR;
    host_nrf.tx_buf[1] = CMD_FALSE_PACKET;
    host_nrf.TX_Mode_NOACK(2);
    wait_rf_response();
    if(host_nrf.newPayload){
        host_nrf.sta = 0;
        host_nrf.newPayload = 0;
        send_inertial_data();
    }
}

void sendInercialCMDLED(bool estadoLed){
    host_nrf.tx_buf[0] = INERTIAL_SUB_ADDR;
    host_nrf.tx_buf[1] = estadoLed? CMD_LIGHT_UP_LED: CMD_TURN_OFF_LED;
    host_nrf.TX_Mode_NOACK(2);
    wait_rf_response();
    if(host_nrf.newPayload){
        host_nrf.sta = 0;
        host_nrf.newPayload = 0;
        if(host_nrf.rx_buf[1] == 0x00){
            DEBUG_PRINT_("CMD_OK recebido!")
        }
    }
}

void send_inertial_data(){
    #ifndef DEBUG_MODE
    //Assembling packet and sending
    serial_buffer_out[0] = UART_START;
    serial_buffer_out[1] = UART_PQUAT;
    serial_buffer_out[2] = host_nrf.rx_buf[13]; //qw_msb
    serial_buffer_out[3] = host_nrf.rx_buf[14]; //qw_lsb
    serial_buffer_out[4] = host_nrf.rx_buf[15]; //qx_msb
    serial_buffer_out[5] = host_nrf.rx_buf[16]; //qx_lsb
    serial_buffer_out[6] = host_nrf.rx_buf[17]; //qy_msb
    serial_buffer_out[7] = host_nrf.rx_buf[18]; //qy_lsb
    serial_buffer_out[8] = host_nrf.rx_buf[19]; //qz_msb
    serial_buffer_out[9] = host_nrf.rx_buf[20]; //qz_lsb
    serial_buffer_out[10] = UART_END;
    Serial.write(serial_buffer_out, 11);
    #endif /*DEBUG_MODE*/
    #ifdef DEBUG_MODE
    //Hexadecimal values
    DEBUG_PRINT_(host_nrf.rx_buf[0],HEX);
    DEBUG_PRINT_(' - ');
    for(int i = 1; i < host_nrf.payloadWidth; i++){
        DEBUG_PRINT_(host_nrf.rx_buf[i],HEX); DEBUG_PRINT_(' ');
    }
    DEBUG_PRINT_("\n");
    //Fifo as AHRS
    float q[4], a[3], g[3];
    //Aceleracao
    a[0] = (float) ((host_nrf.rx_buf[1] << 8) | host_nrf.rx_buf[2]) / 8192.0f;
    a[1] = (float) ((host_nrf.rx_buf[3] << 8) | host_nrf.rx_buf[4]) / 8192.0f;
    a[2] = (float) ((host_nrf.rx_buf[5] << 8) | host_nrf.rx_buf[6]) / 8192.0f;
    //Giroscopio
    g[0] = (float) ((host_nrf.rx_buf[7] << 8) | host_nrf.rx_buf[8]) / 131.0f;
    g[1] = (float) ((host_nrf.rx_buf[9] << 8) | host_nrf.rx_buf[10]) / 131.0f;
    g[2] = (float) ((host_nrf.rx_buf[11] << 8) | host_nrf.rx_buf[12]) / 131.0f;
    //Quaternion
    q[0] = (float) ((host_nrf.rx_buf[13] << 8) | host_nrf.rx_buf[14]) / 16384.0f;
    q[1] = (float) ((host_nrf.rx_buf[15] << 8) | host_nrf.rx_buf[16]) / 16384.0f;
    q[2] = (float) ((host_nrf.rx_buf[17] << 8) | host_nrf.rx_buf[18]) / 16384.0f;
    q[3] = (float) ((host_nrf.rx_buf[19] << 8) | host_nrf.rx_buf[20]) / 16384.0f;

    DEBUG_PRINT_("Quat-Accel-Gyro:\t");
    //Quaternions
    DEBUG_PRINT_(q[0]); DEBUG_PRINT_("\t");
    DEBUG_PRINT_(q[1]); DEBUG_PRINT_("\t");
    DEBUG_PRINT_(q[2]); DEBUG_PRINT_("\t");
    DEBUG_PRINT_(q[3]); DEBUG_PRINT_("\t-\t");
    //Qccel in G
    DEBUG_PRINT_(a[0]); DEBUG_PRINT_("\t");
    DEBUG_PRINT_(a[1]); DEBUG_PRINT_("\t");
    DEBUG_PRINT_(a[2]); DEBUG_PRINT_("\t-\t");
    //Gyro in degrees/s
    DEBUG_PRINT_(g[0]); DEBUG_PRINT_("\t");
    DEBUG_PRINT_(g[1]); DEBUG_PRINT_("\t");
    DEBUG_PRINT_(g[2]); DEBUG_PRINT_("\n");
    #endif /*DEBUG_MODE*/
}

void wait_rf_response(){
    timeout_init_time = micros() + POLLING_TIMEOUT;
    while(1){
        timeout_actual_time = micros();
        if(timeout_actual_time>timeout_init_time){
            DEBUG_PRINT_("Polling timeout!\n");
            break;
        }
        if(!digitalRead(host_nrf.RFIRQ)){
            host_nrf.RF_IRQ();
            #ifdef DEBUG_MODE
            if(host_nrf.TX_OK){
                DEBUG_PRINT_("Data send!\n");
            }
            if(host_nrf.RX_OK){
                DEBUG_PRINT_("Data Received with ");
                DEBUG_PRINT_(timeout_actual_time-timeout_init_time+1000);
                DEBUG_PRINT_(" milliseconds.\n");
                DEBUG_PRINT_(host_nrf.payloadWidth);
                DEBUG_PRINT_(host_nrf.rx_buf[0],HEX);
            }
            #endif /*DEBUG_MODE*/
            break;
        }
    }
}
