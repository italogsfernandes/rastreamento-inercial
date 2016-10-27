# I2C
...Segundo nrf24LE1 Product Specification e as libraries lidas.

## Lista de problemas
*   Em **IIC** e **hal_w2_isr**, Definição de pinagem
*   Em **IIC** = `(devAddr+0xa0)`
*   Em **IIC**, código para e trava em `while(!READY);`
*   Em **IIC**, ao tentar ler com o arduino a esrita não resulta em nada.
*   Em **IIC**, talvez o W2CON1 esteja sendo limpo andes de `while(!READY);`
*   Em **IIC**, muitos `if(ACK);` no lugar de `while(ACK);`
*   Em **IIC**, há o uso de um LED em P00, verificar todas ocorrências e remover.
*   Em **IIC**, Pra que serve **ex_int()** e **Io_config()***?.




## General Setup

* Set '1' to wire2Enable, before doing anything.
* Sempre ler W2DAT.
* Updates maskIrq before anythig.
* Start and Stop conditions can be set immediately after the last TX data write.
## Configurar



## Enviar dados

* **Start Condition**: Determined by W2CON0[4].
* **Slave Address**: Write the slave address in W2DAT and a '0' at the end to indicates the direction.
W2DAT = `[address][0]`. In code: `W2DAT = (slaveaddress<<1)+0x00`
* **ACK**: The ACK is stored in W2CON1[1], it means that the Slave is prepared to receive data.
* **DATA Transfer**: The data should be tranfered to W2DAT. After every transmission the ACK should be verified.
* **STOP Condition**: Write an '1' to W2CON0[5];


## Receber dados

* **Start Condition**: Determined by W2CON0[4].
* **Slave Address**: Write the slave address in W2DAT and an '1' at the end to indicates the direction.
W2DAT = `[address][0]`. In code: `W2DAT = (slaveaddress<<1)+0x01`
* **ACK**: The ACK is stored in W2CON1[1], it means that the Slave is prepared to receive data.
* **Receiving a Byte**: The data will be received in W2DAT.And run a interruption.
* **Data Ready**: Flag after receiving a byte
* **STOP Condition**: Write a '1' to W2CON0[5];
