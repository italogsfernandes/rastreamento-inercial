# Bibliotecas

## hal_w2_isr.h
* Descrição: Realiza comunicação I2C com os sensores:
* Funções:
- [x] @mentions, #refs, [links](), **formatting**, and <del>tags</del> supported
	* Bytes
	- [x] i2c_mpu_writeByte
	- [x] i2c_mpu_writeBytes
	- [x] i2c_mpu_readByte
	- [x] i2c_mpu_readBytes
	* Bits
	- [x] i2c_mpu_writeBit
	- [x] i2c_mpu_writeBits
	- [x] i2c_mpu_readBit
	- [x] i2c_mpu_readBits
	* Words (16 bit variable)
	- [x] i2c_mpu_writeWord
	- [x] i2c_mpu_writeWords
	- [ ] i2c_mpu_readWord - Depende de readWords
	- [ ] i2c_mpu_readWords

## dmp.h
* Descrição: Realiza comunicação I2C com os sensores:
	* Inicialização:
	- [x] mpu_initialize
	- [x] mpu_testConnection
	* Getters:	
	- [x] getMotion6_packet
	* Offsets:
	- [x] setXAccelOffset
	- [x] setYAccelOffset
	- [x] setZAccelOffset
	- [x] setXGyroOffset
	- [x] setYGyroOffset
	- [x] setZGyroOffset
	- [ ] getXAccelOffset - Depende de readWord
	- [ ] getYAccelOffset - Depende de readWord
	- [ ] getZAccelOffset - Depende de readWord
	- [ ] getXGyroOffset - Depende de readWord
	- [ ] getYGyroOffset - Depende de readWord
	- [ ] getZGyroOffset - Depende de readWord
	* DMP
	- [ ] dmpInitialize - Depende de muita coisa (olhar abaixo)
	- [ ] setDMPEnabled - Depende de *writeBit*
	- [ ] dmpGetFIFOPacketSize - Depende de *so de boa vontade*
	- [ ] dmpGetQuaternion - Depende de getFIFO
		* dmpInitialize depende de:
		- [ ] setMemoryBank - Depende de *writeByte*
		- [ ] get*GyroOffsetTC - Depende de *readBits*
		- [ ] pgm_read_byte - XXX: ?
		- [ ] setDMPEnabled - Depende de *writeBit*
		- [ ] writeProgMemoryBlock - Depende de writeMemoryBlock
		- [ ] writeProgDMPConfigurationSet - Depende de writeDMPConfigurationSet
		- [ ] getFIFOCount - Depende de *readByte*
		- [ ] resetFIFO - Depende de *writeBit*
		- [ ] getFIFOBytes - Depende de *readBytes*
		- [ ] readMemoryBlock - Depende das mesmas coisas de writeMemoryBlock
		- [ ] getIntStatus - Depende de *readByte*
		- [ ] writeDMPConfigurationSet
			* Depende de:
			- [ ] writeMemoryBlock
			- [ ] setMemoryBank - Depende de *writeByte*
			- [ ] pgm_read_byte - XXX: ?
			- [ ] memcmp - XXX: ?
			- [ ] malloc e free - Depende de memory pool
		- [ ] writeMemoryBlock
			* Depende de:
			- [ ] setMemoryBank - Depende de *writeByte*
			- [ ] pgm_read_byte - XXX: ?
			- [ ] memcmp - XXX: ? 
			- [ ] malloc e free  - Depende de memory pool