import serial
from datetime import datetime

arduinoData = serial.Serial()
arduinoData.port = '/dev/ttyACM0'
arduinoData.baudrate = 38400
arduinoData.timeout = 1
arduinoData.open()

strikes = 0;

horario = datetime.now()
horario_anterior = horario
arq = open( "leituras/" + str(horario) + '-dados.txt', 'w')

log = 'AAAA-MM-DD HH-MM-SS.ssssss\tXac\tYac\tZac\tXgi\tYgi\tZgi\tXma\tYma\tZma\tWq\tIq\tJq\tKq\t\n'
arq.write(log)

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
