# Bibliotecas

## hal_w2_isr.h
* Descrição: Realiza comunicação I2C com os sensores:
* Funções:
	* Bytes
	- [x] <del>i2c_mpu_writeByte</del>
	- [x] <del>i2c_mpu_writeBytes</del>
	- [x] <del>i2c_mpu_readByte</del>
	- [x] <del>i2c_mpu_readBytes</del>
	* Bits
	- [x] <del>i2c_mpu_writeBit</del>
	- [x] <del>i2c_mpu_writeBits</del>
	- [x] <del>i2c_mpu_readBit</del>
	- [x] <del>i2c_mpu_readBits</del>
	* Words (16 bit variable)
	- [x] <del>i2c_mpu_writeWord</del>
	- [x] <del>i2c_mpu_writeWords</del>
	- [ ] i2c_mpu_readWord - Depende de readWords - Não utilizada
	- [ ] i2c_mpu_readWords - Não utilizada

## dmp.h
* Descrição: Realiza comunicação I2C com os sensores:
	* Inicialização:
	- [x] <del>mpu_initialize</del>
	- [x] <del>mpu_testConnection</del>
	* Getters:
	- [x] <del>getMotion6_packet</del>
	* Offsets:
	- [x] <del>set\*AccelOffset</del>
	- [x] <del>set\*GyroOffset</del>
	- [x] <del>get\*AccelOffset</del>
	- [x] <del>get\*GyroOffset</del>
	* DMP
	- [x] <del>dmpInitialize</del>
	- [x] <del>setDMPEnabled</del>
	- [x] <del>dmpGetFIFOPacketSize</del>
	- [x] <del>dmpGetQuaternion</del>
		* dmpInitialize depende de:
		- [x] <del>setMemoryBank</del>
		- [x] <del>get\*GyroOffsetTC </del>
		- [x] <del>set\*GyroOffsetTC</del> - Mal implementada
		- [x] <del>pgm_read_byte</del>
		- [x] <del>setDMPEnabled</del>
		- [x] <del>writeProgMemoryBlock</del>
		- [x] <del>writeProgDMPConfigurationSet</del>
		- [x] <del>getFIFOCount</del>
		- [x] <del>resetFIFO</del>
		- [x] <del>getFIFOBytes</del>
		- [x] <del>readMemoryBlock</del>
		- [x] <del>getIntStatus</del>
		- [x] <del>writeDMPConfigurationSet</del>
			* Depende de:
			- [x] <del>writeMemoryBlock</del>
			- [x] <del>setMemoryBank</del>
			- [x] <del>pgm_read_byte</del>
			- [x] <del>memcmp </del>
			- [x] <del>[malloc e free](http://www.keil.com/support/man/docs/c51/c51_malloc.htm)</del>
		- [x] <del>writeMemoryBlock</del>
			* Depende de:
			- [x] <del>setMemoryBank</del>
			- [x] <del>pgm_read_byte</del>
			- [x] <del>memcmp </del>
			- [x] <del> [malloc e free](http://www.keil.com/support/man/docs/c51/c51_malloc.htm) </del>

## Referências:
* [I2CDEV - HOME](http://www.i2cdevlib.com/)
* [I2CDEV - GITHUB - ARDUINO - MPU6050](https://github.com/jrowberg/i2cdevlib/tree/master/Arduino/MPU6050)
* [Keil - C51 - Documentação](http://www.keil.com/support/man/docs/c51/)
* [InvenSense - MPU6050 - DATASHEET](https://www.invensense.com/products/motion-tracking/6-axis/mpu-6050/)
* [Nordic Semiconductor - nRF24LE1 - DATASHEET](http://www.nordicsemi.com/eng/Products/2.4GHz-RF/nRF24LE1)
* [TeckShlok - Alguns exemplos do nRF24LE1](http://techshlok.com/blog/)
