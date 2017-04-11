#include<nRF-SPIComands.h>
#include<timer0.h>

uint8_t valor_lido;

void setup(){
    rf_init();
}

void main(){
    setup();
    while(1){


        if(timer_elapsed){
            valor_lido = hal_adc_read(0);    

        }
    }
}
