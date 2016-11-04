#include "hal_w2_isr.h"
#include "dmp.h"


// class default I2C address is 0x68
// specific I2C addresses may be passed as a parameter here
// AD0 low = 0x68 (default for InvenSense evaluation board)
// AD0 high = 0x69

uint8_t packet_motion6[12];

void setup() {
    //TODO: iniciar GPIO
    //TODO: Iniciar I2C, 400KHz
    //TODO: Iniciar RF, RF e serial no receiver
    //XXX
    mpu_initialize();
    //XXX, enviar via RF "Testing device connections...", "MPU6050 connection successful" : "MPU6050 connection failed"
    LEDVM = mpu_testConnection();
    //XXX, testar get and setters de offset
}

void main(void) {
    setup();
    while(1){
        if(!S1){
            getMotion6_packet(packet_motion6);

            italo_delay_ms(100);
            while(!S1);
            italo_delay_ms(100);
        }
        if(!S2){
            italo_delay_ms(100);
            while(!S2);
            italo_delay_ms(100);
        }
        if(newPayload){

        }
    }
}
