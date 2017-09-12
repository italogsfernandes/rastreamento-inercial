#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys
from ctypes import c_short

def to_int16(_MSB,_LSB):
	return c_short((_MSB<<8) + _LSB).value


print("*"*80)

file_name = str(sys.argv[1])
if(len(sys.argv) > 2):
	file_out = str(sys.argv[2]) + ".txt"
else:
	file_out = file_name
print("Convertendo arquivo: " + str(file_name))

file_dataout = open(file_out.replace('.txt','_CalInertialAndMag.csv') ,'w')

file_dataout.write('Packet number,Quaternion w, Quaternion x, Quaternion y, Quaternion z, Gyroscope X (deg/s),Gyroscope Y (deg/s),Gyroscope Z (deg/s),Accelerometer X (g),Accelerometer Y (g),Accelerometer Z (g),Magnetometer X (G),Magnetometer Y (G),Magnetometer Z (G)\n')

Magnect_fake_data = '0.00,0.00,0.00'
packet_num = 0
pacotes_perdidos = 0
percas_procurando_start = 0
file_datain = open(file_name, 'rb')

valor_lido = 'a'
while valor_lido != "":
	if ord(valor_lido) == 0x24:
		#print 'Start Encontrado'
		data = file_datain.read(8)
		data = map(ord,data)
		qw = to_int16((data[0]),(data[1]))/16384.00
		qx = to_int16((data[2]),(data[3]))/16384.00
		qy = to_int16((data[4]),(data[5]))/16384.00
		qz = to_int16((data[6]),(data[7]))/16384.00
		valor_lido = file_datain.read(1)
		if ord(valor_lido) == 0x0A:
			packet_num = packet_num + 1
			print("%2d:%2d:%2d - Pacote Recebido: %d,%.6f,%.6f,%.6f,%.6f\n" %(int(packet_num/360000)%60,int(packet_num/6000)%60,int(packet_num/100)%60,packet_num, qw,qx,qy,qz))
			file_dataout.write("%d,%.6f,%.6f,%.6f,%.6f\n" %(packet_num, qw,qx,qy,qz))
		else:
			packet_num = packet_num + 1
			pacotes_perdidos = pacotes_perdidos + 1
			#print("#Pacote Perdido: %d,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%s\n" %(packet_num, gx,gy,gz,ax,ay,az,Magnect_fake_data))
	else:
		#print("%s" % (hex(ord(valor_lido))))
		percas_procurando_start = percas_procurando_start + 1
	valor_lido = file_datain.read(1)


file_datain.close()
file_dataout.close()

print("%d escritas em: %s" % (packet_num-pacotes_perdidos, file_out.replace('.txt','_CalInertialAndMag.csv')))
print("%d Pacotes perdidos por falta de terminador (0x0A)." % (pacotes_perdidos))
print("%d Bytes perdidos a procurar Start (0x24)." % (percas_procurando_start))
print("%d Pacotes totais." % (packet_num))
print("%.2f segundos (%.2f min) de coleta" % (packet_num/100.,packet_num/6000.))
print("Convertido com sucesso!")
print("*"*80)
