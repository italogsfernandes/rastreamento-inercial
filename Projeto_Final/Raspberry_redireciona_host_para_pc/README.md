# Protocolos e pacotes paca comunicação

Componentes:
*	Sensores
*	Host
*	Computador


##	Formato do pacote padrão Host->Computador

Start signal | Packet Type | Packet Length | ... | data | ... | End signal
--- |--- | --- | --- | --- | --- |---
##	Pacotes entre Sensores->Host
Packet Type | Sensor id |  ... | data | ... |
--- |--- | --- | --- | --- | --- |---

* Types of readings
	1. Packet Aceleração (Lenght = 7):

Sensor id |  Xacel_H |  Xacel_L |  Yacel_H |  Yacel_L |  Zacel_H |  Zacel_L
--- |--- | --- | --- | --- | --- |---
	2. Packet Giroscópio (Lenght = 7)

Sensor id |  Xgyro_H |  Xgyro_L |  Ygyro_H |  Ygyro_L |  Zgyro_H |  Zgyro_L
--- |--- | --- | --- | --- | --- |---
	3. Packet Magnetrômetro (Lenght = 7)

Sensor id |  Xmag_H |  Xmag_L |  Ymag_H |  Ymag_L |  Zmag_H |  Zmag_L
--- |--- | --- | --- | --- | --- |---
	4. Packet Motion6 (Lenght = 13)

Sensor id |	Xacel_H |  Xacel_L |  Yacel_H |  Yacel_L |  Zacel_H |  Zacel_L |Xgyro_H |  Xgyro_L |  Ygyro_H |  Ygyro_L |  Zgyro_H |  Zgyro_L
--- |--- | --- | --- | --- | --- |---
	5. Packet Motion9 (Lenght = 19)

Sensor id |	Xacel_H |  Xacel_L |  Yacel_H |  Yacel_L |  Zacel_H |  Zacel_L |Xgyro_H |  Xgyro_L |  Ygyro_H |  Ygyro_L |  Zgyro_H |  Zgyro_L |  Xmag_H |  Xmag_L |  Ymag_H |  Ymag_L |  Zmag_H |  Zmag_L
--- |--- | --- | --- | --- | --- |---
	6. Packet Quaternion (Lenght = 9):

Sensor id |  Wquat_H |  Wquat_L |  Xquat_L |  Xquat_L |  Yquat_H |  Yquat_L |  Zquat_H |  Zquat_L
--- |--- | --- | --- | --- | --- |---
	7. Packet FIFO_NO_MAG (Lenght = 21):

Sensor id |  Wquat_H |  Wquat_L |  Xquat_L |  Xquat_L |  Yquat_H |  Yquat_L |  Zquat_H |  Zquat_L | Xacel_H |  Xacel_L |  Yacel_H |  Yacel_L |  Zacel_H |  Zacel_L |Xgyro_H |  Xgyro_L |  Ygyro_H |  Ygyro_L |  Zgyro_H |  Zgyro_L
--- |--- | --- | --- | --- | --- |---
	8. Packet FIFO_ALL_READINGS (Lenght = 27):
Sensor id |  Wquat_H |  Wquat_L |  Xquat_L |  Xquat_L |  Yquat_H |  Yquat_L |  Zquat_H |  Zquat_L | Xacel_H |  Xacel_L |  Yacel_H |  Yacel_L |  Zacel_H |  Zacel_L |Xgyro_H |  Xgyro_L |  Ygyro_H |  Ygyro_L |  Zgyro_H |  Zgyro_L |  Xmag_H |  Xmag_L |  Ymag_H |  Ymag_L |  Zmag_H |  Zmag_L
--- |--- | --- | --- | --- | --- |---
* Types of messages

	9. Packet String:

Sensor id | ... | data | ... | '\n'
--- |--- | --- | --- | --- | --- |---
	10. Packet Hex:

Sensor id | ... | data | ...
--- |--- | --- | --- | --- | --- |---
	11. Packet Binary:

Sensor id | ... | data | ...
--- |--- | --- | --- | --- | --- |---
	12. Packet int16_t:

Sensor id | ... | data_H | data_L | data_H | data_L | ...
--- |--- | --- | --- | --- | --- |---
	13. Packet uint16_t:

Sensor id | ... | data_H | data_L | data_H | data_L | ...
--- |--- | --- | --- | --- | --- |---
	14. Packet float_16:

Sensor id | ... | data_H | data_L | data_H | data_L | ...
--- |--- | --- | --- | --- | --- |---
##	Pacotes entre Host->Sensor

Packet Type | Sensor id |  ... | data | ... |
--- |--- | --- | --- | --- | --- |---
* Types of request
	15. Request Reading

Sensor id | Request Type | Reading Type | qnt(Continuo ou qnt points)
--- |--- | --- | --- | --- | --- |---
	16. Request State

Sensor id | Request Type
--- |--- | --- | --- | --- | --- |---
