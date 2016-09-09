using System;
using System.IO.Ports;

using System.Timers;

class serial{
	public static int numero = 0;
	public static Timer meutimer = new Timer ();
	public static void Main(){
		//inicializar:
		SerialPort myserial = new SerialPort("/dev/ttyACM0");
		myserial.BaudRate = 38400;
		myserial.Parity = Parity.None;
		myserial.DataBits = 8;
		myserial.StopBits = StopBits.One;
		myserial.ReadTimeout = 2;



		myserial.DataReceived += new SerialDataReceivedEventHandler(DataReceivedHandler);

		myserial.Open();

		Console.WriteLine("Pressione qualquer tecla para fechar...");
		Console.WriteLine();

		meutimer.Interval = 100;
		meutimer.Elapsed += new ElapsedEventHandler (delegate {
			tick ();
		});
        meutimer.Start();
		Console.ReadKey();
	}

	private static void DataReceivedHandler(object sender, SerialDataReceivedEventArgs e){
		numero++;
    	SerialPort sp = (SerialPort)sender;
        string indata = sp.ReadExisting();
        Console.WriteLine("Data Received:");
        Console.Write(indata);
	}

	//Tenta ler a cada meutimer.Interval
	protected static void tick (){
		Console.WriteLine(numero.ToString());
	}
}
