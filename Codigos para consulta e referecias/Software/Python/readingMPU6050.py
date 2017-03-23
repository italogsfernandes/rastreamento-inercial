import serial
import threading
from time import sleep
from datetime import datetime
from datetime import timedelta
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import numpy as np

#Sensor readings with offsets:   -1807   -108    17042   0   4   -3
#Your offsets:   -2884   -695    1213    82  -25 2

#Start signal | Packet Type | Packet Length | ... | data | ... | End signal
UART_START_SIGNAL = 0x53
UART_END_SIGNAL = 0x04

dataList = []
timeList = []
sampfreq = 200
tmax = 5
maxSamples = tmax*sampfreq
sampleCounter = 0
flagWait = True
flagAcq = True
flagPause = True

def wait_serial_bytes(how_many):
    while porta.inWaiting()<how_many:
        if(flagWait == False):
            break
        pass

def wait_start_signal():
    start_signal = 0
    while start_signal  != UART_START_SIGNAL:    #waiting start signal
        wait_serial_bytes(1)
        start_signal = ord(porta.read())

def inverter(y):
    x = bin(y)
    return int(x.replace('1','2').replace('0','1').replace('2','0').replace('1b','0b'),2)

def to_signed(num):
    return -(inverter(num & 0x7fff))-1 if num & 0x8000 else num & 0x7fff

def to_uint16(MSB,LSB):
    return (MSB << 8 | LSB)

def to_int16(MSB,LSB):
    return to_signed(MSB << 8 | LSB)

def StartAcq():
    porta.write('S'.encode('utf-8'))    

def PauseAcq():    
    porta.write('E'.encode('utf-8'))    

def ResumeTh():
    global flagWait, flagPause
    flagWait = True
    flagPause = False

def PauseTh():
    global flagWait, flagPause
    flagPause = True
    flagWait = False

def KillTh():
    global flagWait, flagPause, flagAcq
    flagWait = False
    flagPause = True
    flagAcq = False    

def Save():
    f = open(filename,'w')        
    for samples in dataList:
        for i in range(len(samples)):
            f.write(str(samples[i]))
            f.write('\t')            
        f.write('\n')
        

def worker():    
    global dataList
    while(flagAcq):        
        if(flagPause == False):
            wait_start_signal()
            wait_serial_bytes(1)    
            packet_len = ord(porta.read())
            wait_serial_bytes(packet_len)
            data = porta.read(packet_len)
            cont = 0        
            dataVector = []
            for i in range(packet_len/2):                
                dataVector.append(to_int16(ord(data[cont]),ord(data[cont+1])))
                cont = cont + 2                    

            endByte = ord(porta.read());
            if(endByte == UART_END_SIGNAL):
                dataVector.append(str(datetime.now()))
                dataList.append(dataVector)
                print dataVector                             
    return

filename = 'mpu6050data.txt'
portName = '/dev/ttyACM0'
baud = 38400
tout = 1
porta = serial.Serial(portName,baud,timeout=tout)
if(porta.is_open == False):
    porta.open()
porta.flushInput()
porta.flushOutput()
threadAcq = threading.Thread(target=worker)
threadAcq.start()

while(1):
    print '-------------------------------'
    print 'Biomedical Engineering Lab'
    print 'MPU 6050 Data Acquisition'
    print '-------------------------------'
    print 'Menu'
    print '1 - New acquisition'
    print '2 - Exit'    
    print '-------------------------------'
    strkey = raw_input()
    if(strkey == '1'):
        StartAcq()        
        ResumeTh()
        print 'Acquisition in process...'                           
        print 'Stop? (y)'        
        while(1):
            strkey = raw_input()
            if(strkey == 'y'):                            
                PauseTh()
                PauseAcq()
                print 'Saving file...'
                Save()
                del dataList[:]   
                del timeList[:]                                                 
                break
    elif(strkey == '2'):
        KillTh()
        porta.close()
        break