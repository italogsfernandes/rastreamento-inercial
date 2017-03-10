import serial
from time import sleep


class nrf:
    '''Comandos reconhecidos pelo host'''
    CMD_OK = 0x00  # Ack - Uart Command
    CMD_ERROR = 0x01  # Error flag - Uart Command
    CMD_START = 0x02  # Start Measuring - Uart Command
    CMD_STOP = 0x03  # Stop Measuring - Uart Command
    CMD_CONNECTION = 0x04  # Teste Connection - Uart Command
    CMD_CALIBRATE = 0x05  # Calibrate Sensors Command
    CMD_DISCONNECT = 0x06  # Some sensor has gone disconected
    CMD_READ = 0x07  # Request a packet of readings
    CMD_SET_PACKET_TYPE = 0x08
    CMD_GET_ACTIVE_SENSORS = 0x09  # Retorna a variavel do host active sensors
    CMD_SET_ACTIVE_SENSORS = 0x0A  # Retorna a variavel do host active sensors
    CMD_TEST_RF_CONNECTION = 0x0B
    CMD_LIGHT_UP_LED = 0x0C
    CMD_TURN_OFF_LED = 0x0D

    porta = serial.Serial()

    def __init__(self, porta, rate):
        self.porta = serial.Serial(porta, rate)

    def readall(self):
        return self.porta.read(self.porta.inWaiting())

    def sendcmd(self, cmd_2_send):
        self.porta.write([0x53, cmd_2_send])
        sleep(0.1)
        return self.readall()

    def sendcmd_with_arg(self, cmd_2_send, arg_2_send):
        self.porta.write([0x53, cmd_2_send, arg_2_send])
        sleep(0.1)
        return self.readall()
