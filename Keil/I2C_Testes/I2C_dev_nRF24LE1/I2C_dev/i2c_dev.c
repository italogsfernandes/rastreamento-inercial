#include "reg24le1.h"
#include "IIC_app.h"
#include "stdbool.h"
#include "intrins.h"


void i2c_setup(void)
{
    IEN0|=0X80; //Enable interrupts
    IEN0|=0X01; //XXX: 1: Enable Interrupt From Pin (IFP) interrupt.
    TCON|=0X01; //XXX: fazendo algo com o timer
    INTEXP|=0x08; //XXX: 1: Enable GP INT0 (from pin) 0 to IFP
    //XXX: PQ não:
    //INTEXP |= 0x04;  // Enable 2 wire interrupts
    //	IEN1 |= 0x04; // Enable 2-wire complete interrupt



    P0DIR|=0X20; //P05 as output
    P0DIR|=0x40; //P06 as output
    P05=1; //W2SDA
    // P04 = W2SCL
    P06=1; //XXX: WHY P06?????????
}
void i2c_begin(void)
{
    FREQSEL(I2C_FASTMODE);
    MODE(MASTER);
    //XXX:BUG: PQ TA DESATIVANDO AS INTERRUPCOES?
    //W2CON1|=0x20;   //Disable all interrupts, why?
    W2SADR=0x00;
    EN2WIRE();  //ativa o i2c
}

void i2c_write(uint8_t devAddr, uint8_t data_to_write){

}

void i2c_read(uint8_t devAddr, uint8_t *data_ptr){


}
