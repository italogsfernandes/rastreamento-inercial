using System;
using System.IO.Ports;
using System.Timers;
using System.Threading;



public class PacketHandle {
    public Thread Threadleitora;
    public SerialPort nrfSerial;
    public bool threadRunningFlag;
	public bool threadWorkingFlag;
	public Mutex mutex_control;
	CircularBuffer<rfPacket> readPackets;

	public byte[] packetread = new byte[32];

	public PacketHandle(Mutex controle, CircularBuffer<rfPacket> packetsbuffer, SerialPort porta) {
		Threadleitora = new Thread(buscarSerialData);
		this.readPackets = packetsbuffer;
		this.mutex_control = controle;
		this.nrfSerial = porta;
	}

    public void buscarSerialData() {
		while (threadRunningFlag) { //somente encerra qnd finalizar o programa
			if (threadWorkingFlag) {//controla o fluxo da thread executando-a ou nao
				lerpacote();
            }             
        }
    }
	
	public void StartThread(){
		threadWorkingFlag = false;
		threadRunningFlag = true;
		Threadleitora.Start ();
	}
	public void RodarThread(){
		threadWorkingFlag = true;
	}
	public void PauseThread(){
		threadWorkingFlag = false;
	}
	public void ResumeThread(){
		threadWorkingFlag = true;
	}
	public void StopThread(){
		threadWorkingFlag = false;
		threadRunningFlag = false;
	}
	public void waitSerialBytes(int howmany){
		while (nrfSerial.BytesToRead < howmany){
			;
		}
	}
	public void waitStartByte(){
		byte StartByte = 0x00;
		while (StartByte != packetTypes.UART_START) {
			waitSerialBytes (1);
			StartByte = nrfSerial.ReadByte();
		}
	}
	public void lerpacote(){
		rfPacket pacote_lido = new rfPacket ();
		waitStartByte ();
		waitSerialBytes (2);
		pacote_lido.Type = nrfSerial.ReadByte ();
		pacote_lido.Length  = nrfSerial.ReadByte ();
		waitSerialBytes (pacote_lido.Length);
		nrfSerial.Read (pacote_lido.dados, count: pacote_lido.Length);
		waitSerialBytes (1);
		byte packetEnd = nrfSerial.ReadByte ();
		if (packetEnd == UART_PACKET_END) {
			salvapacote (pacote_lido);
		} else {
			descartapacote (pacote_lido);
		}
	}
	public void salvapacote(rfPacket pacote){
		mutex_control.WaitOne();
		readPackets.Enqueue(pacote);
		mutex_control.ReleaseMutex ();
	}
	public void descartapacote (rfPacket pacote){
		Console.WriteLine ("Pacote descartado.");
		Console.WriteLine (pacote.ToString ());
	}
}
