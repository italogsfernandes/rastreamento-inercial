using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO;
using System.Diagnostics;
//using System.DateTime;

namespace Marpc
{
 
    public partial class Form1 : Form
    {
        public dataPck[] WBAN;
        //public List<dataPck> sensor2;
        public Stopwatch RunningTime = new Stopwatch();
        long timeAquis;
        // Comandos CPU -> HOST e HOST -> CPU
        byte DATA_SAMPLES = 0x10;
        byte DATA_LOST_SAMPLES = 0x20;
        byte MSG_SENSOR = 0x30;

        //Definições:
        byte TOTAL_SENSOR = 9;

        Thread ThreadAquisicao;
        byte[] delimitador = { 0x7E };  //delimitador 
        byte[] comando_disparar = { 0x02 }; //comando de disparar aquisição
        byte[] comando_cancela = { 0x03 }; //comando de cancela aquisição
        byte[] comando_dadosCPU = { 0x30 }; //comando para receber dados do host para a CPU
        byte[] comando_msgCPU = { 0x10 }; //comando de receber mensagem do host para a CPU
        byte[] comando_status = { 0x40 }; //comando de receber status do host para a CPU
        string[] FAquisSen = new string[11]; //contem a frequencia de aquisição dos sensores
        char[] mac = new char[1];               //contem a informação sobre o tipo de host 'P' -> POLLING ou 'M' -> MAR-PC
        int maxBytesToRead = 200000;
        Int32[] nAmostras = new Int32[10];
        int nPcks = 0;
        int nPcksFail = 0;
        int sBytesRead = 0;
        int perFail = 0;
        int maxPerFail = 30;
        int TotalPcks;
        byte[] LastPck = new byte[100];
        bool aquisicao = false;
        bool AquistionEnd = false;
        ConfigT0 T0Cfg;

        delegate void ShowDataDelegate(byte[] values, int nBytes);

        void AquisicaoSinal()
        {
            int nBytes = 0;  // nBytesRead;
            byte[] SerialData = new byte[100];

            while (true)
            {
                {
                    if (Porta.BytesToRead != 0)
                    {
                        Porta.Read(SerialData, 0, 1);
                        if (SerialData[0] == 0x7E)
                        {
                            while (Porta.BytesToRead == 0);
                            Porta.Read(SerialData, 0, 1);
                            nBytes = SerialData[0];
                            while (Porta.BytesToRead < nBytes);
                            Porta.Read(SerialData, 0, nBytes);
                            if (SerialData[nBytes - 1] == 0x81)     // && (nBytes>3))  
                                SaveData(SerialData, nBytes - 3);                            //Convert.ToBase64CharArray(SerialData, 0, nBytes - 2, CharData));  //ShowData(Convert.ToString(SerialData[0]));
                        }
                    }
                }
            }
        }

        public void SaveData(byte[] values, int nBytes)
        {
            if (InvokeRequired)
                Invoke(new ShowDataDelegate(SaveData), values, nBytes);
            else
            {
                if ((values[0] == 0) && (values[1] == 0))
                    AquistionEnd = true;
                else if(nBytes > 0)
                {
                    WBAN[nPcks].endSensor = values[0];
                    WBAN[nPcks].nDados = (byte)nBytes;
                    if ((values[1] & 0xF0) == 0x10)
                    {
                        WBAN[nPcks].FIFOfull = 0;
                        WBAN[nPcks].pFifo = (byte)(((values[1] & 0x0F) * 700) / 105);
                        nAmostras[WBAN[nPcks].endSensor] = nAmostras[WBAN[nPcks].endSensor] + nBytes;
                        for (byte i = 0; i < nBytes; i++)
                        {
                            WBAN[nPcks].samples[i] = values[i + 2];
                            LastPck[i + 1] = values[i + 2];
                        }
                        LastPck[0] = (byte)(nBytes);
                    }
                    if ((values[1] & 0xF0) == 0x20)
                    {
                        WBAN[nPcks].FIFOfull = 1;
                        WBAN[nPcks].pFifo = (byte)(((values[1] & 0x0F) * 700) / 105);
                        for (byte i = 0; i < nBytes; i++)
                        {
                            WBAN[nPcks].samples[i] = values[i + 2];
                            LastPck[i + 1] = values[i + 2];
                        }
                        LastPck[0] = (byte)(nBytes);
                    }
                    if ((values[1] & 0xF0) == 0x30)
                    {
                        /*for (byte i = 0; i < LastPck[0]; i++)         // REPETE O ÚLTIMO
                        {                                               // PACOTE EM CASO
                            WBAN[nPcks].samples[i] = LastPck[i + 1];    // DE FALHA
                        }*/
                        WBAN[nPcks].samples[0] = 0;   // Quando ocorre uma falha o valor atribuido a samples é 0
                        WBAN[nPcks].nDados = 0;
                        nPcksFail++;
                    }
                    nPcks++;
                    perFail = (int)((nPcksFail * 100) / nPcks);
                    sBytesRead = Porta.BytesToRead;
                }
                if ((nPcks == TotalPcks) || (sBytesRead > maxBytesToRead) || (RunningTime.ElapsedMilliseconds>timeAquis))  //||(perFail > maxPerFail))
                {
                    StopAquis();
                }
                if (AquistionEnd)
                {

                    WBAN_STATUS.ForeColor = Color.Blue;
                    WBAN_STATUS.Text = "AQUISITION FINISHED";
                    T0.Enabled = true;
                    //T1.Enabled = true;
                    DataFlush.Enabled = true;
                    CnfgSensor.Enabled = true;
                    DataShow.Enabled = true;
                    DataSave.Enabled = true;
                    ResetSensor.Enabled = true;
                    ThreadAquisicao.Abort();
                }
            }
        }

        public Form1()
        {
            InitializeComponent();
            WBAN = new dataPck[4500];

            for (int i = 0; i < WBAN.Count(); i++)
            {
                WBAN[i] = new dataPck();
            }
            CancelaAquis.Enabled = false;
            DispAquis.Enabled = false;
            PortaSerialFechar.Enabled = false;
            DataFlush.Enabled = false;
            CnfgSensor.Enabled = false;
            DataShow.Enabled = false;
            DataSave.Enabled = false;
            ResetSensor.Enabled = false;
            AquistionEnd = false;
            TPcks.Text = "1800";
            TotalPcks = Convert.ToUInt16(TPcks.Text);
            for(int i=0;i<TOTAL_SENSOR+1;i++)
            {
                FAquisSen[i] = FreqSample.Text;
            }
            Faq1.Text = FreqSample.Text; Faq2.Text = FreqSample.Text; Faq3.Text = FreqSample.Text;
            Faq4.Text = FreqSample.Text; Faq5.Text = FreqSample.Text; Faq6.Text = FreqSample.Text;
            Faq7.Text = FreqSample.Text; Faq8.Text = FreqSample.Text; Faq9.Text = FreqSample.Text;
            
        }

       private void PortaSerialAbrir_Click(object sender, EventArgs e)
        {
            try
            {
                if (SelecPort.Text != null)
                {
                    Porta.PortName = SelecPort.Text; //Selecione a porta desejada.
                }
                else
                {
                    Porta.PortName = "COM12";  //Caso não seja selecionada, a porta escolhida será a padrão COM5. 
                }

                Porta.BaudRate = 921600;
                Porta.Handshake = System.IO.Ports.Handshake.None;   //Enable handshake, RTS and DTR. This is necessary to Enable serial port to receive data.
                Porta.RtsEnable = true;
                Porta.DtrEnable = true;

                Porta.Open();                                       // Abre dados da porta serial.

                if (Porta.IsOpen == true)
                {
                    PortaSerial.Text = "OPEN";
                    PortaSerial.ForeColor = Color.Blue;
                    DispAquis.Enabled = true;
                    PortaSerialFechar.Enabled = true;
                    CnfgSensor.Enabled = true;
                    ResetSensor.Enabled = true;
                }
            }
            catch
            {
                MessageBox.Show("Uai, não funfô", "Erro");
            }
        }

        private void PortaSerialFechar_Click(object sender, EventArgs e)
        {
            if (Porta.IsOpen)
            {
                RunningTime.Stop();
                ThreadAquisicao.Abort();
                Porta.Close(); //Se a porta está aberta, a fecha.
                PortaSerial.Text = "CLOSED";
                PortaSerial.ForeColor = Color.Red;
                DispAquis.Enabled = false;
                DataFlush.Enabled = false;
                CnfgSensor.Enabled = false;
                ResetSensor.Enabled = false;
            }
        }
        private void clearWBAN()
        {
            for (byte i = 0; i < TotalPcks;i++)
            {
                WBAN[i].FIFOfull = 0;
                WBAN[i].pFifo = 0;
                for (byte j = 0; j < 30; j++)
                {
                    WBAN[i].samples[j] = 0;
                }
            }
                
        }
        private void clearChart()
        {
            Sensor1.Series[0].Points.Clear();
            Fifo1.Series[0].Points.Clear();
            Fifo1.Series[1].Points.Clear();
            Sensor2.Series[0].Points.Clear();
            Fifo2.Series[0].Points.Clear();
            Fifo2.Series[1].Points.Clear();
            Sensor3.Series[0].Points.Clear();
            Fifo3.Series[0].Points.Clear();
            Fifo3.Series[1].Points.Clear();
            Sensor4.Series[0].Points.Clear();
            Fifo4.Series[0].Points.Clear();
            Fifo4.Series[1].Points.Clear();
            Sensor5.Series[0].Points.Clear();
            Fifo5.Series[0].Points.Clear();
            Fifo5.Series[1].Points.Clear();
            Sensor6.Series[0].Points.Clear();
            Fifo6.Series[0].Points.Clear();
            Fifo6.Series[1].Points.Clear();
            Sensor7.Series[0].Points.Clear();
            Fifo7.Series[0].Points.Clear();
            Fifo7.Series[1].Points.Clear();
            Sensor8.Series[0].Points.Clear();
            Fifo8.Series[0].Points.Clear();
            Fifo8.Series[1].Points.Clear();
            Sensor9.Series[0].Points.Clear();
            Fifo9.Series[0].Points.Clear();
            Fifo9.Series[1].Points.Clear();

            Fifo1.Series[0].Points.AddY(0);
            Fifo2.Series[0].Points.AddY(0);
            Fifo3.Series[0].Points.AddY(0);
            Fifo4.Series[0].Points.AddY(0);
            Fifo5.Series[0].Points.AddY(0);
            Fifo6.Series[0].Points.AddY(0);
            Fifo7.Series[0].Points.AddY(0);
            Fifo8.Series[0].Points.AddY(0);
            Fifo9.Series[0].Points.AddY(0);
        }

        private void StartAquis()
        {
            if (Porta.IsOpen == true) //verifica se a porta está aberta.
            {

                for (int i = 0; i < 10; i++)
                    nAmostras[i] = 0;
                CancelaAquis.Enabled = true;
                nPcks = 0;
                nPcksFail = 0;
                sBytesRead = 0;
                perFail = 0;
                AquistionEnd = false;
                Porta.DiscardInBuffer();
                Porta.Write(delimitador, 0, 1); //Insere dados de delimitador na porta serial.
                Porta.Write(comando_disparar, 0, 1); //Insere dados de comando na porta serial.
                //Task.Delay(1);
                //Porta.Write(delimitador, 0, 1); //Insere dados de delimitador na porta serial.
                //Porta.Write(comando_disparar, 0, 1); //Insere dados de comando na porta serial.
                RunningTime.Restart();
                timeAquis = Convert.ToInt64(TAq.Text)*1000;
                Task.Delay(200);
                if (Porta.BytesToRead > 0)
                {
                    Porta.Read(mac, 0, 1);
                    if (mac[0] == 'P')
                        textBox1.Text = "POLLING";
                    if (mac[0] == 'M')
                        textBox1.Text = "MAR-PC";
                }
                DataFlush.Enabled = false;
                CnfgSensor.Enabled = false;
                DataShow.Enabled = false;
                DataSave.Enabled = false;
                ResetSensor.Enabled = false;
                int milliseconds = 500;
                Task.Delay(milliseconds);
                T0.Enabled = false;
                WBAN_STATUS.ForeColor = Color.Blue;
                WBAN_STATUS.Text = "RUNNING...";
                ThreadAquisicao = new Thread(AquisicaoSinal);
                ThreadAquisicao.Priority = ThreadPriority.AboveNormal;
                ThreadAquisicao.Start();
                //                setGraphParameters();
            }
        }

        private void DispAquis_Click(object sender, EventArgs e)
        {
            //clearWBAN();
            clearChart();
            StartAquis();
        }

        private void StopAquis()
        {
            Porta.Write(delimitador, 0, 1); //Insere dados de delimitador na porta serial.
            Porta.Write(comando_cancela, 0, 1); //Insere dados de comando na porta serial.
            Task.Delay(50);
            Porta.Write(delimitador, 0, 1); //Insere dados de delimitador na porta serial.
            Porta.Write(comando_cancela, 0, 1); //Insere dados de comando na porta serial.
            RunningTime.Stop();
            TAq.Text = Convert.ToString(RunningTime.ElapsedMilliseconds/1000);
        }

        private void CancelaAquis_Click(object sender, EventArgs e)
        {
            StopAquis();
            DataFlush.Enabled = true;
            CnfgSensor.Enabled = true;
            DataShow.Enabled = true;
            DataSave.Enabled = true;
            ResetSensor.Enabled = true;
            ThreadAquisicao.Abort();
        }

        private void ConfigSensor(double FS)
        {
            int aux;
            aux = (int)(1000.0 / FS);
            T0Cfg.AQ_TIMEH = (byte)(aux / 256);
            T0Cfg.AQ_TIMEL = (byte)(aux % 256);
            T0Cfg.NBT0 = (int)((1000000000.0 / FS)/750.0);
            T0Cfg.NOVT0 = (byte)(T0Cfg.NBT0 / 65536);
            T0Cfg.NPRT0 = (int)(T0Cfg.NBT0 - ((int)(T0Cfg.NOVT0 * 65536)));
            if (T0Cfg.NBT0<65536)
            {
                T0Cfg.NBT0H = (byte)((65536 - T0Cfg.NBT0) / 256);
                T0Cfg.NBT0L = (byte)((65536 - T0Cfg.NBT0) % 256);
                T0Cfg.NPRT0H = 0;
                T0Cfg.NPRT0L = 0;
            }
            else
            {
                T0Cfg.NBT0H = 0;
                T0Cfg.NBT0L = 0;
                T0Cfg.NPRT0H = (byte)((65536 - T0Cfg.NPRT0) / 256);
                T0Cfg.NPRT0L = (byte)((65536 - T0Cfg.NPRT0) % 256);
            }
        }

        private void T0_Tick(object sender, EventArgs e)
        {
            WBAN_STATUS.ForeColor = Color.Red;
            WBAN_STATUS.Text = "STOPPED";
            T0.Enabled = false;
        }

        private void CnfgSensor_Click(object sender, EventArgs e)
        {
            byte i, nroSen;
            byte ConfigOK;
            double FSen;
            byte[] vetCfg = new byte[8];
            byte[] SerialDataRead = new byte[10]; 
            int nroBytes;

                ConfigOK = 0;
                for (i = 0; i < TOTAL_SENSOR; i++)
                {
                    nroSen = (byte)(i + 1);
                    FSen = Convert.ToDouble(FAquisSen[nroSen - 1]);
                    ConfigSensor(FSen);
                    vetCfg = new byte[10] { 0x7E, 0x01, nroSen, T0Cfg.NBT0H, T0Cfg.NBT0L, T0Cfg.NOVT0, T0Cfg.NPRT0H, T0Cfg.NPRT0L, T0Cfg.AQ_TIMEH, T0Cfg.AQ_TIMEL };
                    Porta.Write(vetCfg, 0, 10);
                    Thread.Sleep(200);
                    nroBytes = Porta.BytesToRead;
                    if (nroBytes > 5)
                    {
                        Porta.Read(SerialDataRead, 0, 1);
                        if (SerialDataRead[0] == 0x7E)
                        {
                            Porta.Read(SerialDataRead, 0, 5);
                            if ((SerialDataRead[1] == nroSen) && (SerialDataRead[3] == 0x01))
                                ConfigOK++;
                        }
                    }
                }
                if (ConfigOK == TOTAL_SENSOR)
                {
                    WBAN_STATUS.ForeColor = Color.Blue;
                    WBAN_STATUS.Text = "Config. Sensors: OK!";
                    T0.Enabled = false;
                    T0.Enabled = true;
                }
                else
                {
                    WBAN_STATUS.ForeColor = Color.Red;
                    WBAN_STATUS.Text = "Config. Sensors: FAIL!";
                    T0.Enabled = false;
                    T0.Enabled = true;
                }

            }

        private void EndApp_Click(object sender, EventArgs e)
        {
            aquisicao = false;
            if (Porta.IsOpen)
            {
                StopAquis();
                Porta.Close();                                  //Se a porta está aberta, a fecha.
            }

            this.Close();

            Application.Exit();
        }

        private void DataShow_Click(object sender, EventArgs e)
        {
            clearChart();
            chartSensors();
        }

        private void Reset_WBAN()
        {
            byte nroSen;
            double FSen;
            byte[] vetCfg = new byte[2];
            byte[] SerialDataRead = new byte[10];
            int nroBytes;

            Porta.DiscardInBuffer();
            vetCfg = new byte[2] { 0x7E, 0x08 };
            Porta.Write(vetCfg, 0, 2);
            Thread.Sleep(1000);
            nroBytes = Porta.BytesToRead;
            if (nroBytes > 4)
            {
                Porta.Read(SerialDataRead, 0, 1);
                if (SerialDataRead[0] == 0x7E)
                {
                    Porta.Read(SerialDataRead, 0, 5);
                    if (SerialDataRead[3] == 0x01)
                    {
                        WBAN_STATUS.ForeColor = Color.Blue;
                        WBAN_STATUS.Text = "RESET OK!";
                        T0.Enabled = false;
                        T0.Enabled = true;
                    }
                    else
                    {
                        WBAN_STATUS.ForeColor = Color.Red;
                        WBAN_STATUS.Text = "RESET FAIL!";
                        T0.Enabled = false;
                        T0.Enabled = true;
                    }
                }
            }
            else
            {
                WBAN_STATUS.ForeColor = Color.Red;
                WBAN_STATUS.Text = "RESET FAIL!";
                T0.Enabled = false;
                T0.Enabled = true;
            }
        }

        private void ResetSensor_Click(object sender, EventArgs e)
        {
            Reset_WBAN();
        }

        private void chartSensors()
        {
            int indexPck = 0;
            byte i;
            byte endSensor;

            for (indexPck = 0; indexPck < nPcks; indexPck++)
            {
                endSensor = WBAN[indexPck].endSensor;
                switch (endSensor)
                {
                    /*----->>>>> GRÁFICO SENSOR 1 <<<<<-----*/
                    case 1:
                        for (i = 0; i < WBAN[indexPck].nDados; i = (byte)(i + 2))
                        {
                            Sensor1.Series[0].Points.AddY(256 * WBAN[indexPck].samples[i] + WBAN[indexPck].samples[i + 1]);
                            //if (Sensor1.Series[0].Points.Count > 500)
                            //    Sensor1.Series[0].Points.RemoveAt(0);                       //Remova o primeiro ponto.
                        }
                        Fifo1.Series[0].Points.AddY(WBAN[indexPck].pFifo);
                        Fifo1.Series[1].Points.AddY(100*WBAN[indexPck].FIFOfull); 
                        //if (Fifo1.Series[0].Points.Count > 500)
                        //    Fifo1.Series[0].Points.RemoveAt(0);                             //Remova o primeiro ponto.
                        Fifo1.Update();                                                     //Atualiza o grafico.
                        Sensor1.Update();                                                   //Atualiza o grafico.
                        break;

                    /*----->>>>> GRÁFICO SENSOR 2 <<<<<-----*/
                    case 2:
                        for (i = 0; i < WBAN[indexPck].nDados; i = (byte)(i + 2))
                        {
                            Sensor2.Series[0].Points.AddY(256 * WBAN[indexPck].samples[i] + WBAN[indexPck].samples[i + 1]);
                            //if (Sensor2.Series[0].Points.Count > 500)
                            //    Sensor2.Series[0].Points.RemoveAt(0);                       //Remova o primeiro ponto.
                            
                        }
                        Fifo2.Series[0].Points.AddY(WBAN[indexPck].pFifo);
                        Fifo2.Series[1].Points.AddY(100 * WBAN[indexPck].FIFOfull); 
                        //if (Fifo2.Series[0].Points.Count > 500)
                        //    Fifo2.Series[0].Points.RemoveAt(0);                             //Remova o primeiro ponto.
                        Sensor2.Update();                                                   //Atualiza o grafico.
                        Fifo2.Update();                                                     //Atualiza o grafico.
                        break;

                    /*----->>>>> GRÁFICO SENSOR 3 <<<<<-----*/
                    case 3:
                        for (i = 0; i < WBAN[indexPck].nDados; i = (byte)(i + 2))
                        {
                            Sensor3.Series[0].Points.AddY(256 * WBAN[indexPck].samples[i] + WBAN[indexPck].samples[i + 1]);
                            //if (Sensor3.Series[0].Points.Count > 500)
                            //    Sensor3.Series[0].Points.RemoveAt(0);                       //Remova o primeiro ponto.
                        }
                        Fifo3.Series[0].Points.AddY(WBAN[indexPck].pFifo);
                        Fifo3.Series[1].Points.AddY(100 * WBAN[indexPck].FIFOfull); 
                        //if (Fifo3.Series[0].Points.Count > 500)
                        //    Fifo3.Series[0].Points.RemoveAt(0);                             //Remova o primeiro ponto.
                        Sensor3.Update();                                                   //Atualiza o grafico.
                        Fifo3.Update();                                                     //Atualiza o grafico.
                        break;

                    /*----->>>>> GRÁFICO SENSOR 4 <<<<<-----*/
                    case 4:
                        for (i = 0; i < WBAN[indexPck].nDados; i = (byte)(i + 2))
                        {
                            Sensor4.Series[0].Points.AddY(256 * WBAN[indexPck].samples[i] + WBAN[indexPck].samples[i + 1]);
                            //if (Sensor4.Series[0].Points.Count > 500)
                            //    Sensor4.Series[0].Points.RemoveAt(0);                       //Remova o primeiro ponto.
                        }
                        Fifo4.Series[0].Points.AddY(WBAN[indexPck].pFifo);
                        Fifo4.Series[1].Points.AddY(100 * WBAN[indexPck].FIFOfull); 
                        //if (Fifo4.Series[0].Points.Count > 500)
                        //    Fifo4.Series[0].Points.RemoveAt(0);                             //Remova o primeiro ponto.
                        Sensor4.Update();                                                   //Atualiza o grafico.
                        Fifo4.Update();                                                     //Atualiza o grafico.
                        break;
                    /*----->>>>> GRÁFICO SENSOR 5 <<<<<-----*/
                    case 5:
                        for (i = 0; i < WBAN[indexPck].nDados; i = (byte)(i + 2))
                        {
                            Sensor5.Series[0].Points.AddY(256 * WBAN[indexPck].samples[i] + WBAN[indexPck].samples[i + 1]);
                            //if (Sensor5.Series[0].Points.Count > 500)
                            //    Sensor5.Series[0].Points.RemoveAt(0);                       //Remova o primeiro ponto.
                        }
                        Fifo5.Series[0].Points.AddY(WBAN[indexPck].pFifo);
                        Fifo5.Series[1].Points.AddY(100 * WBAN[indexPck].FIFOfull); 
                        //if (Fifo5.Series[0].Points.Count > 500)
                        //    Fifo5.Series[0].Points.RemoveAt(0);                             //Remova o primeiro ponto.
                        Sensor5.Update();                                                   //Atualiza o grafico.
                        Fifo5.Update();                                                     //Atualiza o grafico.
                        break;

                    /*----->>>>> GRÁFICO SENSOR 6 <<<<<-----*/
                    case 6:
                        for (i = 0; i < WBAN[indexPck].nDados; i = (byte)(i + 2))
                        {
                            Sensor6.Series[0].Points.AddY(256 * WBAN[indexPck].samples[i] + WBAN[indexPck].samples[i + 1]);
                            //if (Sensor6.Series[0].Points.Count > 500)
                            //    Sensor6.Series[0].Points.RemoveAt(0);                       //Remova o primeiro ponto.
                        }
                        Fifo6.Series[0].Points.AddY(WBAN[indexPck].pFifo);
                        Fifo6.Series[1].Points.AddY(100 * WBAN[indexPck].FIFOfull); 
                        //if (Fifo6.Series[0].Points.Count > 500)
                        //    Fifo6.Series[0].Points.RemoveAt(0);                             //Remova o primeiro ponto.
                        Sensor6.Update();                                                   //Atualiza o grafico.
                        Fifo6.Update();                                                     //Atualiza o grafico.
                        break;

                    /*----->>>>> GRÁFICO SENSOR 7 <<<<<-----*/
                    case 7:
                        for (i = 0; i < WBAN[indexPck].nDados; i = (byte)(i + 2))
                        {
                            Sensor7.Series[0].Points.AddY(256 * WBAN[indexPck].samples[i] + WBAN[indexPck].samples[i + 1]);
                            //if (Sensor7.Series[0].Points.Count > 500)
                            //    Sensor7.Series[0].Points.RemoveAt(0);                       //Remova o primeiro ponto.
                            
                        }
                        Fifo7.Series[0].Points.AddY(WBAN[indexPck].pFifo);
                        Fifo7.Series[1].Points.AddY(100 * WBAN[indexPck].FIFOfull); 
                        //if (Fifo7.Series[0].Points.Count > 500)
                        //    Fifo7.Series[0].Points.RemoveAt(0);                             //Remova o primeiro ponto.
                        Sensor7.Update();                                                   //Atualiza o grafico.
                        Fifo7.Update();                                                     //Atualiza o grafico.
                        break;

                    /*----->>>>> GRÁFICO SENSOR 8 <<<<<-----*/
                    case 8:
                        for (i = 0; i < WBAN[indexPck].nDados; i = (byte)(i + 2))
                        {
                            Sensor8.Series[0].Points.AddY(256 * WBAN[indexPck].samples[i] + WBAN[indexPck].samples[i + 1]);
                            //if (Sensor8.Series[0].Points.Count > 500)
                            //    Sensor8.Series[0].Points.RemoveAt(0);                       //Remova o primeiro ponto.
                        }
                        Fifo8.Series[0].Points.AddY(WBAN[indexPck].pFifo);
                        Fifo8.Series[1].Points.AddY(100 * WBAN[indexPck].FIFOfull); 
                        //if (Fifo8.Series[0].Points.Count > 500)
                        //    Fifo8.Series[0].Points.RemoveAt(0);                             //Remova o primeiro ponto.
                        Sensor8.Update();                                                   //Atualiza o grafico.
                        Fifo8.Update();                                                     //Atualiza o grafico.
                        break;

                    /*----->>>>> GRÁFICO SENSOR 9 <<<<<-----*/
                    case 9:
                        for (i = 0; i < WBAN[indexPck].nDados; i = (byte)(i + 2))
                        {
                            Sensor9.Series[0].Points.AddY(256 * WBAN[indexPck].samples[i] + WBAN[indexPck].samples[i + 1]);
                            //if (Sensor9.Series[0].Points.Count > 500)
                            //    Sensor9.Series[0].Points.RemoveAt(0);                       //Remova o primeiro ponto.
                            
                        }
                        Fifo9.Series[0].Points.AddY(WBAN[indexPck].pFifo);
                        Fifo9.Series[1].Points.AddY(100 * WBAN[indexPck].FIFOfull); 
                        //if (Fifo9.Series[0].Points.Count > 500)
                        //    Fifo9.Series[0].Points.RemoveAt(0);                             //Remova o primeiro ponto.
                        Sensor9.Update();                                                   //Atualiza o grafico.
                        Fifo9.Update();                                                     //Atualiza o grafico.
                        break;

                    default: break;

                }
                //Thread.Sleep(1);
            }
        }

        
        private void DataSave_Click(object sender, EventArgs e)
        {
         //   DataSaveFull();
            OnlyDataSave();
        }
    
        private void DataSaveFull()
        {
            int indexPck = 0;
            byte endSensor, i;
            int bioDH, bioDL;

            string path = @"C:\Users\FEELT\Desktop\MAR-PC_FULL.txt";
            dataFile[] dataSample = new dataFile[10];
            Int32[] nPckSensor = new Int32[10];
            Int32[] nSamples = new Int32[10];


            for (i = 0; i < dataSample.Count(); i++)
            {
                dataSample[i] = new dataFile();
            }
            T0.Enabled = false;
            //   WBAN_STATUS.Enabled = true;
            WBAN_STATUS.ForeColor = Color.Blue;
            WBAN_STATUS.Text = "SAVING FULL DATA";
            Thread.Sleep(2000);
            // Delete the file if it exists.
            if (File.Exists(path))
                File.Delete(path);

            using (FileStream fs = File.Create(path))
            {
                AddText(fs, "         WIRELESS BODY AREA NETWORK - WBAN        "); AddText(fs, "\r\n"); AddText(fs, "\r\n");
                AddText(fs, "STAR TOPOLOGY  WITH MAC POLLING PROTOCOL OPERATION"); AddText(fs, "\r\n"); AddText(fs, "\r\n");
                AddText(fs, "Number of packets captured by WBAN: "); AddText(fs, Convert.ToString(nPcks)); AddText(fs, "\r\n"); AddText(fs, "\r\n");
                
                // Zera as variáveis e vetores utilizandos na montagem do arquivo
                for (i = 0; i < TOTAL_SENSOR; i++)
                {
                    nPckSensor[WBAN[indexPck].endSensor - 1] = 0;
                    nSamples[i] = 0;
                    dataSample[i].np = "";  dataSample[i].sf = "";  dataSample[i].ds = "";
                    dataSample[i].pfs = ""; dataSample[i].ovf = ""; dataSample[i].ns = "";
                    dataSample[i].tns = ""; dataSample[i].nbs = "12 bits";
                }

                // Cada posição do vetor dataSample.ds refere-se a um sensor e nela é criada uma string com todas as amostras (bioDH e bioDL) daquele sensor
                // nSample é carregado com o total de amostras de cada sensor
                // dataSample.pfs é carregado com o valor da FIFO daquele pacote (indexPck)
                // dataSample.ns é carregado com o número de amostras em cada pacote (indexPck)
                for (indexPck = 0; indexPck < nPcks; indexPck++)
                {
                    nPckSensor[WBAN[indexPck].endSensor - 1]++;
                    for (i = 0; i < WBAN[indexPck].nDados; i = (byte)(i + 2))
                    {
                        bioDH = 256 * WBAN[indexPck].samples[i];
                        bioDL = WBAN[indexPck].samples[i + 1];
                        dataSample[WBAN[indexPck].endSensor - 1].ds = dataSample[WBAN[indexPck].endSensor - 1].ds + " " + Convert.ToString(bioDH + bioDL);
                        nSamples[WBAN[indexPck].endSensor - 1]++;
                    }
                    dataSample[WBAN[indexPck].endSensor - 1].pfs = dataSample[WBAN[indexPck].endSensor - 1].pfs + " " + Convert.ToString(WBAN[indexPck].pFifo);
                    dataSample[WBAN[indexPck].endSensor - 1].ovf = dataSample[WBAN[indexPck].endSensor - 1].ovf + " " + Convert.ToString(WBAN[indexPck].FIFOfull);
                    dataSample[WBAN[indexPck].endSensor - 1].ns = dataSample[WBAN[indexPck].endSensor - 1].ns + " " + Convert.ToString((WBAN[indexPck].nDados - 1) / 2);
                }

                // dataSample.sf é carregado com a frequencia de amostragem do sensor
                // dataSample.np é carregado com o número de pacotes recebido daquele sensor
                // dataSample.tns é carregado com o número total de amostras recebidas daquele sensor
                for (i = 0; i < TOTAL_SENSOR; i++)
                {
                    dataSample[i].sf = FAquisSen[i] + " Hz\r\n";
                    dataSample[i].np = Convert.ToString(nPckSensor[i]);
                    dataSample[i].tns = Convert.ToString(nSamples[i]);
                }

                // Aqui é criado o arquivo com as strings criadas para cada sensor
                for (i = 0; i < TOTAL_SENSOR; i++)
                {
                    AddText(fs, "Sensor Number: "); AddText(fs, Convert.ToString(i+1)); AddText(fs, "\r\n");
                    AddText(fs, "Total Number of packets: "); AddText(fs, dataSample[i].np); AddText(fs, "\r\n");
                    AddText(fs, "Total Number of samples: "); AddText(fs, dataSample[i].tns); AddText(fs, "\r\n");
                    AddText(fs, "Number of bits in the sample: "); AddText(fs, dataSample[i].nbs); AddText(fs, "\r\n");
                    AddText(fs, "Sample Frequency: "); AddText(fs, dataSample[i].sf);
                    AddText(fs, "Data Sensor: "); AddText(fs, dataSample[i].ds); AddText(fs, "\r\n");
                    AddText(fs, "% FIFO Sensor: "); AddText(fs, dataSample[i].pfs); AddText(fs, "\r\n");
                    AddText(fs, "% Overflow FIFO: "); AddText(fs, dataSample[i].ovf); AddText(fs, "\r\n");
                    AddText(fs, "Number of Samples: "); AddText(fs, dataSample[i].ns); AddText(fs, "\r\n\r\n");
                }
            }
            WBAN_STATUS.Text = "FINISHED RECORDING";
            Thread.Sleep(2000);
        }

        private void OnlyDataSave()
        {
            int indexPck = 0;
            byte endSensor, i;
            int bioDH, bioDL;
            DateTime saveNow = DateTime.Now;
            int ano = saveNow.Year;
            int mes = saveNow.Month;
            int dia = saveNow.Day;
            int hora = saveNow.Hour;
            int min = saveNow.Minute;
            string moment = Convert.ToString(dia) + "-" + Convert.ToString(mes) + "-" + Convert.ToString(ano) + " " + Convert.ToString(hora) + "h" + Convert.ToString(min) + "m";
            string fileName = mac[0] + Convert.ToString(timeAquis) + "s " + moment;
            string path = @"C:\Users\FEELT\Desktop\" + fileName + ".txt";
            string[] header = new string[TOTAL_SENSOR];
            dataFile[] dataSample = new dataFile[10];
            Int32[] nPckSensor = new Int32[10];
            Int32[] nSamples = new Int32[10];


            for (i = 0; i < dataSample.Count(); i++)
            {
                dataSample[i] = new dataFile();
            }
            T0.Enabled = false;
            //   WBAN_STATUS.Enabled = true;
            WBAN_STATUS.ForeColor = Color.Blue;
            WBAN_STATUS.Text = "SAVING ONLY DATA";
            Thread.Sleep(2000);
            // Delete the file if it exists.
            if (File.Exists(path))
                File.Delete(path);

            using (FileStream fs = File.Create(path))
            {
                // Zera as variáveis e vetores utilizandos na montagem do arquivo
                for (i = 0; i < TOTAL_SENSOR; i++)
                {
                    nPckSensor[WBAN[indexPck].endSensor - 1] = 0;
                    nSamples[i] = 0;
                    dataSample[i].np  = ""; dataSample[i].sf  = ""; dataSample[i].ds = "";
                    dataSample[i].pfs = ""; dataSample[i].ovf = ""; dataSample[i].ns = "";
                    dataSample[i].tns = ""; dataSample[i].nbs = "";
                }

                // Cada posição do vetor dataSample.ds refere-se a um sensor e nela é criada uma string com todas as amostras (bioDH e bioDL) daquele sensor
                // nSample é carregado com o total de amostras de cada sensor
                // dataSample.pfs é carregado com o valor da FIFO daquele pacote (indexPck)
                // dataSample.ns é carregado com o número de amostras em cada pacote (indexPck)

                for (indexPck = 0; indexPck < nPcks; indexPck++)
                {
                    nPckSensor[WBAN[indexPck].endSensor - 1]++;
                    for (i = 0; i < WBAN[indexPck].nDados; i = (byte)(i + 2))
                    {
                        bioDH = 256 * WBAN[indexPck].samples[i];
                        bioDL = WBAN[indexPck].samples[i + 1];
                        if(dataSample[WBAN[indexPck].endSensor - 1].ds == "")
                            dataSample[WBAN[indexPck].endSensor - 1].ds = Convert.ToString(bioDH + bioDL);
                        else
                            dataSample[WBAN[indexPck].endSensor - 1].ds = dataSample[WBAN[indexPck].endSensor - 1].ds + "\t" + Convert.ToString(bioDH + bioDL);
                        nSamples[WBAN[indexPck].endSensor - 1]++;
                    }
                    if(dataSample[WBAN[indexPck].endSensor - 1].pfs == "")
                        dataSample[WBAN[indexPck].endSensor - 1].pfs = Convert.ToString(WBAN[indexPck].pFifo);
                    else
                        dataSample[WBAN[indexPck].endSensor - 1].pfs = dataSample[WBAN[indexPck].endSensor - 1].pfs + "\t" + Convert.ToString(WBAN[indexPck].pFifo);
                    if(dataSample[WBAN[indexPck].endSensor - 1].ovf == "")
                        dataSample[WBAN[indexPck].endSensor - 1].ovf = Convert.ToString(WBAN[indexPck].FIFOfull);
                    else
                        dataSample[WBAN[indexPck].endSensor - 1].ovf = dataSample[WBAN[indexPck].endSensor - 1].ovf + "\t" + Convert.ToString(WBAN[indexPck].FIFOfull);
                    if(dataSample[WBAN[indexPck].endSensor - 1].ns == "")
                        dataSample[WBAN[indexPck].endSensor - 1].ns = Convert.ToString((WBAN[indexPck].nDados)/2);
                    else
                        dataSample[WBAN[indexPck].endSensor - 1].ns = dataSample[WBAN[indexPck].endSensor - 1].ns + "\t" + Convert.ToString(WBAN[indexPck].nDados/2);
                }

                // dataSample.sf é carregado com a frequencia de amostragem do sensor
                // dataSample.np é carregado com o número de pacotes recebido daquele sensor
                // dataSample.tns é carregado com o número total de amostras recebidas daquele sensor
                for (i = 0; i < TOTAL_SENSOR; i++)
                {
                    dataSample[i].sf = FAquisSen[i];
                    dataSample[i].np = Convert.ToString(nPckSensor[i]);
                    dataSample[i].tns = Convert.ToString(nSamples[i]);
                    header[i] = Convert.ToString(i + 1) + "\t" + dataSample[i].sf + "\t" + dataSample[i].np + "\t" + dataSample[i].tns + "\t" + Convert.ToString(timeAquis);
                    //dataSample[i].ds = dataSample[i].ds + "\n\r";
                    //dataSample[i].pfs = dataSample[i].pfs + "\n\r";
                    //dataSample[i].ovf = dataSample[i].ovf + "\n\r";
                    //dataSample[i].ns = dataSample[i].ns + "\n\r";
                }

                // Aqui é criado o arquivo com as strings criadas para cada sensor
                // Primeiro é enviada frequencia de amostragem de todos os sensores

                AddText(fs, Convert.ToString(TOTAL_SENSOR)); AddText(fs, "\t");
                AddText(fs, FAquisSen[0]);
                for (i = 1; i < TOTAL_SENSOR; i++)
                {
                    AddText(fs, "\t");
                    AddText(fs, FAquisSen[i]);
                }
                AddText(fs, "\n\r");

                // Aqui é criado o arquivo com as strings criadas para cada sensor
                for (i = 0; i < TOTAL_SENSOR; i++)
                {
                    AddText(fs, header[i]);
                    AddText(fs, "\n\r");
                    AddText(fs, dataSample[i].ds);
                    AddText(fs, "\n\r");
                    AddText(fs, dataSample[i].pfs);
                    AddText(fs, "\n\r");
                    AddText(fs, dataSample[i].ovf);
                    AddText(fs, "\n\r");
                    AddText(fs, dataSample[i].ns);
                    AddText(fs, "\n\r");
                }
            }
            WBAN_STATUS.Text = "FINISHED RECORDING";
            Thread.Sleep(2000);
        }
        private static void AddText(FileStream fs, string value)
        {
            byte[] info = new UTF8Encoding(true).GetBytes(value);
            fs.Write(info, 0, info.Length);
        }

        private void TPcks_Leave(object sender, EventArgs e)
        {
            TotalPcks = Convert.ToUInt16(TPcks.Text);
        }

        private void FSEnter_Click(object sender, EventArgs e)
        {
            byte EndS,i;
            if (SensorNum.Text != "All")
            {
                EndS = Convert.ToByte(SensorNum.Text);
                FAquisSen[EndS - 1] = FreqSample.Text;
            }
            else
            {
                for (i = 0; i < TOTAL_SENSOR; i++)
                {
                    FAquisSen[i] = FreqSample.Text;
                }
            }
            Faq1.Text = FAquisSen[0]; Faq2.Text = FAquisSen[1]; Faq3.Text = FAquisSen[2];
            Faq4.Text = FAquisSen[3]; Faq5.Text = FAquisSen[4]; Faq6.Text = FAquisSen[5];
            Faq7.Text = FAquisSen[6]; Faq8.Text = FAquisSen[7]; Faq9.Text = FAquisSen[8];
                
        }

        private void SensorNum_TextChanged(object sender, EventArgs e)
        {
            int i,nroBytes;
            byte EndS;

            if (SensorNum.Text != "All")
            {
                EndS = Convert.ToByte(SensorNum.Text);
                FreqSample.Text = FAquisSen[EndS - 1];                
            }
            else
            {
                FreqSample.Text = FAquisSen[TOTAL_SENSOR-1];                
            }
        }

        private void T1_Tick(object sender, EventArgs e)
        {
            T1.Enabled = true;
            while (AquistionEnd == false)
            {
                AquisicaoSinal();
            }
            AquistionEnd = false;
            WBAN_STATUS.ForeColor = Color.Blue;
            WBAN_STATUS.Text = "AQUISITION FINISHED";
            T0.Enabled = true;
        }

    }
    
    public struct ConfigT0
    {
        public double FAquis;
        public int   PAns;
        public int   NBT0;
        public int   NPRT0;
        public byte  NOVT0;
        public byte  NPRT0H;
        public byte  NPRT0L;
        public byte  NBT0H;
        public byte  NBT0L;
        public byte AQ_TIMEH;
        public byte AQ_TIMEL;
    } 

    public class dataPck
    {
        public byte endSensor;
        public byte FIFOfull;
        public byte pFifo;
        public byte nDados;
        public byte[] samples;

        public dataPck()
        {
            samples = new byte[30];
        }
    }

    public class dataFile
    {
        public string np;       // Total Number of packets
        public string sf;       // Sample Frequency
        public string ds;       // Data Sensor
        public string pfs;      // % FIFO Sensor
        public string ovf;      // % FIFO FULL 0 = normal 100 = Estouro
        public string ns;       // Number of Samples
        public string tns;      // Total Number of samples
        public string nbs;      // Number of bits in the sample
    }
}
