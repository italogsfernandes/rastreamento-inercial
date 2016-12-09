using UnityEngine;
using System.Collections;
using System.IO.Ports;
using System.Timers;
using UnityEngine.UI;
using System.Threading;

public class GM : MonoBehaviour {

	public PacketHandle leitor;
	public Mutex mymutex;
	public CircularBuffer<rfPacket> pacotesFIFO;
	public SerialPort HostModule;
	public Dropdown availablePorts;
	public float tempoatual = 0;
	public GameObject ModuleMao;
	void Start () {
		//Configuracoes da porta serial
		HostModule = new SerialPort ();
		mymutex = new Mutex ();
		leitor = new PacketHandle (mymutex, pacotesFIFO, HostModule);
		setupSerialPort ();
		popularPortasSeriais ();
	}

	void Update () {
		
	}
	void setupSerialPort(){
		HostModule.BaudRate = 38400;
		HostModule.ReadTimeout = 1;
		HostModule.WriteTimeout = 1;
		HostModule.Parity = Parity.None;
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
			leitor.StartThread ();
			leitor.RodarThread ();
		} else {
			Debug.Log ("Desligado");
			leitor.PauseThread ();
			HostModule.Close ();
		}
	}
	void OnExit(){ //is it correct?
		leitor.StopThread();
		HostModule.Close ();
	}
}
