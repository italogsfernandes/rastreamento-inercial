using System;
using System.IO.Ports;

class serial{

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
		Console.ReadKey();
		myserial.Close();
	}
	
	private static void DataReceivedHandler(object sender, SerialDataReceivedEventArgs e){
    	SerialPort sp = (SerialPort)sender;
        string indata = sp.ReadExisting();
        Console.WriteLine("Data Received:");
        Console.Write(indata);
	}	
}
