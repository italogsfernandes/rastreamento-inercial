using UnityEngine;
using System.Collections;
using System.IO.Ports;
using System.Timers;
using UnityEngine.UI;

public class GM : MonoBehaviour {

	public SerialPort HostModule;
	public Timer aquirertimer;
	public Queue readingsFIFO;
	public Stack myStack;
	public Dropdown availablePorts;
	public float tempoatual = 0;
	public GameObject ModuleMao;
	void Start () {
		//Configuracoes da porta serial
		HostModule = new SerialPort ();
		HostModule.BaudRate = 38400;
		HostModule.ReadTimeout = 1;
		HostModule.WriteTimeout = 1;
		HostModule.Parity = Parity.None;
		//Configuracoes do Timer
		aquirertimer = new Timer(1);//every 1 second
		popularPortasSeriais ();
		//FIFO
		readingsFIFO = new Queue<Quaternion>();
		myStack = new Stack<Quaternion> ();
	}
	

	void Update () {
//		if (readingsFIFO.Count > 0) {
//			ModuleMao.transform.rotate (readingsFIFO.Dequeue ());
//		}
		if (myStack.Count > 0) {
			ModuleMao.transform.rotate (myStack.Pop ());
			myStack.Clear ();
		}
	}

	void popularPortasSeriais(){
		availablePorts.options.Clear ();
		foreach (string porta in SerialPort.GetPortNames ()) {
			availablePorts.options.Add (new Dropdown.OptionData () { text = porta });
		}
		availablePorts.value = 1;
		availablePorts.value = 0;
			
	}

	void btnIniciarRastreamentoClicked(){
		HostModule.PortName = availablePorts.value;
		if (!HostModule.IsOpen ()) {
			Debug.Log ("Ligado");
			HostModule.Open ();
			aquirertimer.Start ();
		} else {
			Debug.Log ("Desligado");
			aquirertimer.Stop ();
			HostModule.Close ();
		}
	}

	#define UART_START_FLAG 
	#define ...

	void getreading(){
		Quaternion quat;
		byte[] dados;
		byte inicio;
		byte packet_type;
		byte packet_len;
		byte end_byte;

		while(HostModule.IsOpen ()) {
			inicio = 0;
			while (inicio != UART_START_FLAG) {
				while (HostModule.BytesToRead < 1)
					;
				inicio = HostModule.ReadByte ();
			}
			while (HostModule.BytesToRead < 2)
				;
			packet_type = HostModule.ReadByte ();
			packet_len = HostModule.ReadByte ();
			if (packet_type == UART_PACKET_TYPE_QUAT) {
				while (HostModule.BytesToRead < packet_len)
					;
				HostModule.Read (dados, 0, packet_len);
				sensor = dados [0];
				quat.w = (float)(dados [1] << 8 | dados [2]) / 16384;
				quat.x = (float)(dados [3] << 8 | dados [4]) / 16384;
				quat.y = (float)(dados [5] << 8 | dados [6]) / 16384;
				quat.z = (float)(dados [7] << 8 | dados [8]) / 16384;
			}
			while (HostModule.BytesToRead < 1)
				;
			end_byte = HostModule.ReadByte ();
			if (end_byte == UART_END_SIGNAL) {
				readingsFIFO.Enqueue (quat);
				myStack.Push (quat);
			}
		}
	}


}
