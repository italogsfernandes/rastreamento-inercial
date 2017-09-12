import serial
from datetime import datetime
from time import sleep

arduinoData = serial.Serial()
arduinoData.port = '/dev/ttyUSB0'
arduinoData.baudrate = 115200
arduinoData.timeout = 1

strikes = 0;

horario = datetime.now()
horario_anterior = horario
arq = open( "leitura-" + str(horario) + '-dados.txt', 'w')

log = 'AAAA-MM-DD HH-MM-SS.ssssss\tData\n'
arq.write(log)

arduinoData.open()
sleep(5);
arduinoData.write('1');

arduinoData.flushInput()
while True:
    arduinoString = arduinoData.readline()

    horario = datetime.now()
    log = str(horario) +"\t"+arduinoString

    arq.write(log)

    if horario.second != horario_anterior.second:
    	print(log)
    	if len(arduinoString) == 0:
    		strikes = strikes + 1
    		if strikes > 10:
    			break
    		horario_anterior = horario

arduinoData.close()
arq.close()
