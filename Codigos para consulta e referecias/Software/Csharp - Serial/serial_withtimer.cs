using System;
using System.IO.Ports;
using System.Timers;

class serial_withtimer
{
	public static SerialPort minhaporta = new SerialPort ("/dev/ttyACM0");
	public static Timer meutimer = new Timer ();
	public static void Main(){
		//Definições da porta serial
		minhaporta.BaudRate = 38400;
		minhaporta.Parity = Parity.None;
		minhaporta.DataBits = 8;
		minhaporta.StopBits = StopBits.One;
		minhaporta.ReadTimeout = 2;

		//Definindo timer de leitura
		meutimer.Interval = 50;
		meutimer.Elapsed += new ElapsedEventHandler (delegate {
			tick ();
		});
        minhaporta.Open();
        meutimer.Start();
        Console.ReadKey();
        meutimer.Stop();
        minhaporta.Close();
	}

	//Tenta ler a cada meutimer.Interval
	protected static void tick (){
		try {
			Console.Write(minhaporta.ReadExisting ().ToString());
		} catch {

		}
	}
}
