#python 3
import serial
from time import sleep
#Start signal | Packet Type | Packet Length | ... | data | ... | End signal
UART_START_SIGNAL = 0x53
UART_END_SIGNAL = 0x04

UART_PACKET_TYPE_ACEL   =   0x01
UART_PACKET_TYPE_GIRO   =   0x02
UART_PACKET_TYPE_MAG    =   0x03
UART_PACKET_TYPE_M6     =   0x04
UART_PACKET_TYPE_M9     =   0x05
UART_PACKET_TYPE_QUAT   =   0x06
UART_PACKET_TYPE_FIFO_NO_MAG    =   0x07
UART_PACKET_TYPE_FIFO_ALL_READINGS  =   0x08
UART_PACKET_TYPE_STRING =   0x09
UART_PACKET_TYPE_HEX    =   0x0A
UART_PACKET_TYPE_BIN    =   0x0B
UART_PACKET_TYPE_UINT16 =   0x0C
UART_PACKET_TYPE_INT16  =   0x0D
UART_PACKET_TYPE_FLOAT16    =   0x0E


def wait_serial_bytes(how_many):
    while porta.inWaiting()<how_many:
        sleep(0.001)

def wait_start_signal():
    start_signal = 0
    while inicio != UART_START_FLAG:    #waiting start signal
        wait_serial_bytes(1)
        start_signal = ord(porta.read())
        print(str(hex(start_signal)), end='\t')
    print()

def inverter(y):
    x = bin(y)
    return int(x.replace('1','2').replace('0','1').replace('2','0').replace('1b','0b'),2)

def to_signed(num):
    return -(inverter(num & 0x7fff)) if num & 0x8000 else  num & 0x7fff
    

def to_uint16(MSB,LSB):
    return (MSB << 8 | LSB)

def to_int16(MSB,LSB):
    return to_signed(MSB << 8 | LSB)
def verifica_end_byte(to_be_verified):

def descarta_str(s1,s2,s3,descarte,s4):
    print('\nDescartado-STRING:', end="\t")
    print(str(chr(s1)), end=" ")
    print(str(chr(s2)), end=" ")
    print(str(chr(s3)), end=" | ")
    for letter in descarte:
        print(str(chr((letter))), end=" ")
    print(str("| ") + str(chr(s4)), end="\n")

def descarta_hex(s1,s2,s3,descarte,end):
    print('\nDescartado-HEX:', end="\t")
    print(str(hex(s1)), end=" ")
    print(str(hex(s2)), end=" ")
    print(str(hex(s3)), end=" | ")
    for letter in descarte:
        print(str(hex((letter))), end=" ")
    print(str("| ") + str(hex(s4)), end="\n")

def ler():
    wait_start_signal()
    wait_serial_bytes(2)
    packet_type = ord(porta.read())
    packet_len = ord(porta.read())
'''
    if packet_type == UART_PACKET_TYPE_ACEL && packet_len == 7:
        print("Packet Aceleracao:", end='\t')
        wait_serial_bytes(packet_len)
        dados = porta.read(packet_len)
        sensor = int((dados[0]))
        Xac = to_int16(dados[1],dados[2])
        Yac = to_int16(dados[3],dados[4])
        Zac = to_int16(dados[5],dados[6])
        Xac,Yac,Zac = float(Xac)/16384,float(Yac)/16384,float(Zac)/16384
        output = ("%d:\t%.2f\t%.2f\t%.2f\t" % (sensor,Xac,Yac,Zac))'''
    if packet_type == UART_PACKET_TYPE_M6 && packet_len == 13:
        print("Packet Motion6:", end='\t')
        wait_serial_bytes(packet_len)
        dados = porta.read(packet_len)
        sensor = int((dados[0]))
        Xac = to_int16(dados[1],dados[2])
        Yac = to_int16(dados[3],dados[4])
        Zac = to_int16(dados[5],dados[6])
        Xgy = to_int16(dados[7],dados[8])
        Ygy = to_int16(dados[9],dados[10])
        Zgy = to_int16(dados[11],dados[12])
        Xac,Yac,Zac = float(Xac)/16384,float(Yac)/16384,float(Zac)/16384
        Xgy,Ygy,Zgy = float(Xgy)/262,float(Ygy)/262,float(Zgy)/262
        output = ("%d:\t%.2f\t%.2f\t%.2f\t-\t%.2f\t%.2f\t%.2f" % (sensor,Xac,Yac,Zac,Xgy,Ygy,Zgy))
    elif packet_type == UART_PACKET_TYPE_STRING:
        print("STRING:", end='\t')
        wait_serial_bytes(packet_len)
        dados = porta.read(packet_len)
        sensor = int(dados[0])
        output = ("%d:\t%s" % (sensor,str(dados[1::])))
    elif packet_type == UART_PACKET_TYPE_HEX:
        print("HEX:", end='\t')
        wait_serial_bytes(packet_len)
        dados = porta.read(packet_len)
        sensor = int(dados[0])
        output = "%d:\t" % (sensor)
        for letter in dados:
            output = output + str(hex((letter))) + str(" ")
    elif packet_type == UART_PACKET_TYPE_BIN:
        print("BIN:", end='\t')
        wait_serial_bytes(packet_len)
        dados = porta.read(packet_len)
        sensor = int(dados[0])
        output = "%d:\t" % (sensor)
        for letter in dados:
            output = output + str(bin((letter))) + str(" ")
    elif packet_type == UART_PACKET_TYPE_UINT16:
        print("BIN:", end='\t')
        wait_serial_bytes(packet_len)
        dados = porta.read(packet_len)
        sensor = int(dados[0])
        output = "%d:\t" % (sensor)
        for a in range(0,int(packet_len/2)):
            output = output + str(to_uint16(dados[(a*2)],dados[(a*2)+1])) + str(" ")
    
    elif packet_type == UART_PACKET_TYPE_INT16:
        print("BIN:", end='\t')
        wait_serial_bytes(packet_len)
        dados = porta.read(packet_len)
        sensor = int(dados[0])
        output = "%d:\t" % (sensor)
        for a in range(0,int(packet_len/2)):
            output = output + str(to_int16(dados[(a*2)],dados[(a*2)+1])) + str(" ")
    else:
        print("Nao Reconhecido:", end="\t")
        dados = porta.read(porta.inWaiting())
        output = "NOT-STRING:\t" str(dados) + str("\nNOT-HEX:\t")
        for letter in dados:
            output = output + str(hex((letter))) + str(" ")

    wait_serial_bytes(1);
    end_byte = porta.read();
    if end_byte == UART_END_SIGNAL:
        print(output)
    else:
        descarta_str(UART_START_SIGNAL, packet_type, packet_len, dados, end_byte)
        descarta_hex(UART_START_SIGNAL, packet_type, packet_len, dados, end_byte)

#setup:
porta = serial.Serial('/dev/ttyAMA0',38400)
porta.timeout = 1
#mostrarleituras()
print('iniciado')
while 1:
    ler()

