using UnityEngine;
using System.Collections;
using System.IO.Ports;
using System.Timers;
using UnityEngine.UI;
using System.Threading;

public class GM : MonoBehaviour {

	public PacketHandle leitor;
	public Mutex mymutex = new Mutex ();
	public CircularBuffer<rfPacket> pacotesFIFO  = new CircularBuffer<rfPacket>(512);
	public CircularBuffer<Quaternion> pacotesQuat = new CircularBuffer<Quaternion> (512);
	public SerialPort HostModule = new SerialPort ();
	public Dropdown availablePorts;
	public float tempoatual = 0;
	public GameObject ModuleMao;
	public string[] portasSeriais;
	void Start () {
		//Configuracoes da porta serial
		leitor = new PacketHandle (mymutex, pacotesFIFO,pacotesQuat, HostModule);
		setupSerialPort ();
		popularPortasSeriais ();
	}

	void Update () {
		tempoatual = tempoatual + Time.deltaTime;
		if (tempoatual >= 0.05) {
			mymutex.WaitOne ();
			while (!pacotesQuat.IsEmpty) {
				ModuleMao.transform.rotation = pacotesQuat.Dequeue ();
			}
			while (!pacotesFIFO.IsEmpty) {
				Debug.Log (pacotesFIFO.Dequeue ().dados);
			}
			mymutex.ReleaseMutex ();
			tempoatual = 0;
		}

	}
	void setupSerialPort(){
		HostModule.BaudRate = 38400;
		HostModule.ReadTimeout = 1;
		HostModule.WriteTimeout = 1;
		HostModule.Parity = Parity.None;
		Debug.Log ("Porta Serial preparada.");
	}
	void popularPortasSeriais(){
		Debug.Log ("Populando portas seriais");
		availablePorts.options.Clear ();
		portasSeriais = SerialPort.GetPortNames ();
		foreach (string porta in portasSeriais) {
			availablePorts.options.Add (new Dropdown.OptionData () { text = porta });
		}
		availablePorts.value = 1;
		availablePorts.value = 0;
			
	}

	public void btnIniciarRastreamentoClicked(){
		HostModule.PortName = portasSeriais [availablePorts.value];
		Debug.Log ("Porta Selecionada= " + portasSeriais[availablePorts.value]);

		if (!HostModule.IsOpen) {
			HostModule.Open ();
			leitor.StartThread ();
			leitor.RodarThread ();
			Debug.Log ("Porta aberta e Thread Rodando");
		} else {
			leitor.PauseThread ();
			HostModule.Close ();
			Debug.Log ("Porta Fechada e Thread pausada");
		}
	}

	void OnApplicationQuit() {
		leitor.StopThread();
		HostModule.Close ();
		Debug.Log("Application ending after " + Time.time + " seconds");
	}
}
