# -*- coding: utf-8 -*-
#------------------------------------------------------------------------------
# FEDERAL UNIVERSITY OF UBERLANDIA
# Faculty of Electrical Engineering
# Biomedical Engineering Lab
#------------------------------------------------------------------------------
# Author: Andrei Nakagawa, MSc
# Contact: nakagawa.andrei@gmail.com
# URL: www.biolab.eletrica.ufu.br
# Git: www.github.com/andreinakagawa
#------------------------------------------------------------------------------
# Decription:
#------------------------------------------------------------------------------
import serial
from datetime import datetime
from time import sleep
import numpy as np

#creating the expectedAccel matrix used for calibrating the accelerometers
#it contains the expected values for each position for the calibration
expectedAccel = np.zeros([6000,3])
expectedAccel[0:1000,2] = 16384 #Z up
expectedAccel[1000:2000,2] = -16384 #Z down
expectedAccel[2000:3000,1] = 16384 #Y up
expectedAccel[3000:4000,1] = -16384 #Y down
expectedAccel[4000:5000,0] = 16384 #X up
expectedAccel[5000:6000,0] = -16384 #X down

arduinoData = serial.Serial()
arduinoData.port = '/dev/ttyUSB0'
arduinoData.baudrate = 115200
arduinoData.timeout = 1

arduinoData.open()
sleep(5);
arduinoData.flushInput()
arduinoData.flushOutput()

numSensors = 5
sensorData = np.zeros([6000,15])

arqGyro = open("gyroOffsets.txt", 'w')

while(True):
    print '-------------------------------'
    print 'Biomedical Engineering Lab'
    print 'MPU 6050 Data Acquisition'
    print '-------------------------------'
    print 'Menu'
    print '1 - New acquisition'
    print '2 - Stop acquisition'
    print '3 - Calibration of Gyroscopes'
    print '4 - Calibration of Accelerometers'
    print '5 - Calibration of Magnetometers'
    print '6 - Exit'
    print '-------------------------------'
    strkey = raw_input()

    if strkey == '1':
        arduinoData.write('1')

    elif strkey == '2':
        arduinoData.write('2')

    elif strkey == '3':
        arduinoData.write('3')
        while(arduinoData.inWaiting() == 0):
            continue
        while(arduinoData.inWaiting() > 0):
            arduinoString = arduinoData.readline()
            print(arduinoString)

    elif strkey == '4':
        print('\nPosicione o equipamento de calibração com o eixo Z+ para cima e pressione ENTER quando estiver preparado')
        strkey = raw_input()
        arduinoData.write('4')
        sleep(1)
        totalIt = 6
        sampleCounter=0
        for k in range(totalIt):
            while(arduinoData.inWaiting() > 0):
                arduinoString = arduinoData.readline()
                data = arduinoString.split('|')
                for n in range(numSensors):
                    aux = data[n].split(' ')
                    sensorData[sampleCounter,n*3 + 0] = aux[0]
                    sensorData[sampleCounter,n*3 + 1] = aux[1]
                    sensorData[sampleCounter,n*3 + 2] = aux[2]
                print sensorData[sampleCounter,:]
                sampleCounter += 1

            if(k == 0):
                print('\nPosicione o equipamento de calibração com o eixo Z+ para baixo e pressione ENTER quando estiver preparado')
            elif(k == 1):
                print('\nPosicione o equipamento de calibração com o eixo Y+ para cima e pressione ENTER quando estiver preparado')
            elif(k == 2):
                print('\nPosicione o equipamento de calibração com o eixo Y+ para baixo e pressione ENTER quando estiver preparado')
            elif(k == 3):
                print('\nPosicione o equipamento de calibração com o eixo X+ para cima e pressione ENTER quando estiver preparado')
            elif(k == 4):
                print('\nPosicione o equipamento de calibração com o eixo X+ para baixo e pressione ENTER quando estiver preparado')
            elif(k == 5):
                print('\nPressione ENTER para encerrar')

            skey = raw_input()
            arduinoData.write('\n')
            if(k < numSensors):
                while(arduinoData.inWaiting() == 0):
                    continue

        arqAccel = open("accelOffsets.txt", 'w')
        #Computing the calibration matrix for each sensor
        #Least-squares method
        for k in range(numSensors):
            acc = np.zeros([6000,4])
            acc[:,0:3] = sensorData[0:6000,k*3:k*3+3]
            acc[:,3] = 1
            X = np.linalg.inv(np.dot(np.transpose(acc),acc))
            XX = np.dot(X,np.transpose(acc))
            XXX = np.dot(XX,expectedAccel)
            arqAccel.write(str(XXX[0,0])+ "\t" + str(XXX[0,1]) + "\t" + str(XXX[0,2]) + "\n")
            arqAccel.write(str(XXX[1,0])+ "\t" + str(XXX[1,1]) + "\t" + str(XXX[1,2]) + "\n")
            arqAccel.write(str(XXX[2,0])+ "\t" + str(XXX[2,1]) + "\t" + str(XXX[2,2]) + "\n")
            arqAccel.write(str(XXX[3,0])+ "\t" + str(XXX[3,1]) + "\t" + str(XXX[3,2]) + "\n\n")
        arqAccel.close()

        print 'Calibração encerrada!'

    elif strkey == '5':
        print('\nCalibração dos magnetômetros')
        print('Mova o equipamento no ar, desenhando uma Figura 8 até receber o aviso de que a calibração acabou')
        print('Pressione ENTER para começar')
        skey = raw_input()
        arqMagn = open("magnOffsets.txt", 'w')
        arduinoData.write('5')
        while(arduinoData.inWaiting() == 0):
            continue
        #calibration finished
        print("Calibração encerrada!")
        while(arduinoData.inWaiting() > 0):
            arduinoString = arduinoData.readline()
            data = arduinoString.split('{')
            data = data[1].split('}')
            data = data[0].split(',')
            arqMagn.write(data[0] + "\t" + data[1] + "\t" + data[2] + "\n")
        arqMagn.close()

    elif strkey == '6':
        break

    print('\n')

arduinoData.close()
