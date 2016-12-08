using System;
using System.IO.Ports;
using System.Timers;
using System.Threading;

public class PacketHandle {
    public Thread leitor;
    public static int numero = 0;
    public static Timer meutimer = new Timer ();
    public SerialPort nrfSerial;

    public Mutex mymutex;
    public bool threadflag;
    public bool threadflag2;

    public int[] pacote = new int[14];
    public void threadMethod() {
        while (threadflag) { //somente encerra qnd finalizar o programa
            if (threadflag2) {//controla o fluxo da thread executando-a ou nao
                dados = lerpacote();
                mymutex.WaitOne();
                fila.Enqueue(dados);
                mymutex.ReleaseMutex();
            }             
        }
    }

    public PacketHandle() {
        leitor = new Thread(threadMethod);
        threadflag = true;
        threadflag2 = false;
        leitor.Start();
    }

    //methods pause, resume, stop, start

    

    public setupSerial(){
        nrfSerial =   new SerialPort("COM3",38400);
        nrfSerial.ReadTimeout = 1;
        nrfSerial.DataReceived += new SerialDataReceivedEventHandler(DataReceivedHandler);
        nrfSerial.Open();
        meutimer.Interval = 100;
        meutimer.Elapsed += new ElapsedEventHandler (delegate {
            tick ();
            });
            meutimer.Start();
        }
        nrfSerial.Open();
        meutimer.Start();
        Console.ReadKey();
        meutimer.Stop();
        nrfSerial.Close();
    }

    //Tenta ler a cada meutimer.Interval
    protected static void tick (){
        try {
            Console.Write(minhaporta.ReadExisting ().ToString());
        } catch {

        }
    }
    public void wait_serial_bytes(int how_many){
    while (Serial) {

    }
    }

    public void lerpacket(){

    }
}
