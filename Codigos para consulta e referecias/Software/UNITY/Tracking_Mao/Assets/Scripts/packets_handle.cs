using System;
using System.IO.Ports;
using System.Timers;
using System.Threading;
using UnityEngine;
using System.Collections;



public class PacketHandle {
    public Thread Threadleitora;
    public SerialPort nrfSerial;
    public bool threadRunningFlag;
	public bool threadWorkingFlag;
	public Mutex mutex_control;
	CircularBuffer<Quaternion> packetsQuat;
	CircularBuffer<rfPacket> readPackets;

	public byte[] packetread = new byte[32];

	public PacketHandle(Mutex controle, CircularBuffer<rfPacket> packetsbuffer,CircularBuffer<Quaternion> packetsQuaternion, SerialPort porta) {
		Threadleitora = new Thread(buscarSerialData);
		this.readPackets = packetsbuffer;
		this.mutex_control = controle;
		this.nrfSerial = porta;
		this.packetsQuat = packetsQuaternion;
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
		Debug.Log ("BytesToRead: " + nrfSerial.BytesToRead.ToString ());
		while (nrfSerial.BytesToRead < howmany){
			//Debug.Log ("BytesToRead: " + nrfSerial.BytesToRead.ToString ());
			;
		}
		Debug.Log ("BytesToRead: " + nrfSerial.BytesToRead.ToString ());
	}
	public void waitStartByte(){
		int StartByte = 0x00;
		while (StartByte != (int) packetTypes.UART_START_SIGNAL) {
			waitSerialBytes (1);
			StartByte = nrfSerial.ReadByte();
		}
	}
	public void lerpacote(){
		rfPacket pacote_lido = new rfPacket ();
		Debug.Log ("Aguardando StartBit");
		waitStartByte ();
		waitSerialBytes (2);
		pacote_lido.Type = nrfSerial.ReadByte ();
		pacote_lido.Length  = nrfSerial.ReadByte ();
		Debug.Log ("Pacote Tipo: " + pacote_lido.Type.ToString ());
		Debug.Log ("Pacote Length: " + pacote_lido.Length.ToString ());
		waitSerialBytes (pacote_lido.Length);
		for (int i = 0; i < pacote_lido.Length; i++) {
			pacote_lido.dados [i] = nrfSerial.ReadByte ();
		}
		Debug.Log ("Dados: " + pacote_lido.dados.ToString ());
		waitSerialBytes (1);
		int packetEnd = nrfSerial.ReadByte ();
		Debug.Log("End: " + packetEnd.ToString());
		if (packetEnd == (int) packetTypes.UART_END_SIGNAL) {
			salvapacote (pacote_lido);
		} else {
			descartapacote (pacote_lido);
		}
	}
	public void salvapacote(rfPacket pacote){
		Debug.Log ("Pacote sendo salvo");
		if (pacote.Type == (int) packetTypes.UART_PACKET_TYPE_QUAT || 
			pacote.Type == (int) packetTypes.UART_PACKET_TYPE_FIFO_NO_MAG || 
			pacote.Type ==  (int) packetTypes.UART_PACKET_TYPE_FIFO_ALL_READINGS) {
			mutex_control.WaitOne ();
			Quaternion quat = new Quaternion (
				(float) (pacote.dados[3] << 8 | pacote.dados[4])/16384,
				(float) (pacote.dados[5] << 8 | pacote.dados[6])/16384,
				(float) (pacote.dados[7] << 8 | pacote.dados[8])/16384,
				(float) (pacote.dados[1] << 8 | pacote.dados[2])/16384
			);
			Debug.Log ("Quartenion: " + quat.ToString ());
			packetsQuat.Enqueue (quat);
			mutex_control.ReleaseMutex ();
		} else {
			mutex_control.WaitOne ();
			readPackets.Enqueue (pacote);
			mutex_control.ReleaseMutex ();
		}
	}
	public void descartapacote (rfPacket pacote){
		Debug.Log("Pacote descartado.");
		Debug.Log(pacote.ToString ());
	}
}
