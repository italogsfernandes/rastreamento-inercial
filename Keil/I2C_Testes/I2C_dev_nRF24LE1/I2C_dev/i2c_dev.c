#include "reg24le1.h"
#include "i2c_dev.h"
#include "stdbool.h"
#include "intrins.h"

void i2c_setup(void)
{
    /*
    W2CON0 |= 0x01; // Set "wire2Enable" bit
    W2CON1 = ~(BIT_5);    // Clear "maskIrq" bit




    */

    IEN0|=0X80; //Enable interrupts //1000 0000
    IEN0|=0X01; //XXX: 1: Enable Interrupt From Pin (IFP) interrupt.
    TCON|=0X01; //XXX: fazendo algo com o timer
    INTEXP|=0x08; //XXX: 1: Enable GP INT0 (from pin) 0 to IFP
    //XXX: PQ não:
    //INTEXP |= 0x04;  // Enable 2 wire interrupts
    //IEN1 |= 0x04; // Enable 2-wire complete interrupt
    //W2CON1 = 0x00;

    //BUG:NOTE:OQ é SPIF?
    //SPIF = 0;

    P0DIR|=0X20; //P05 as output
    P0DIR|=0x40; //P06 as output
    P05=1; //W2SDA
    // P04 = W2SCL
    P06=1; //XXX: WHY P06?????????
}
void i2c_begin(void)
{
    FREQ_STANDART_MODE();
    masterSelect();
    maskIrq_irqOff(); //mascarando as interrupções, será necessario ler status continuamente
    W2SADR=0x00; //seta para responder ao endereço global.
    wire2Enable(); //ativando o i2c
}

void i2c_write(uint8_t devAddr, uint8_t data_to_write){

}

void i2c_read(uint8_t devAddr, uint8_t *data_ptr){


}
