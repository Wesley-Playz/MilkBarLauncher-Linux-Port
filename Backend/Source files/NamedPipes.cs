using System;
using System.Diagnostics;
using System.IO.Pipes;
using System.Text;
using System.Threading;

namespace Breath_of_the_Wild_Multiplayer.Source_files
{
    public static class NamedPipes
    {
        private static NamedPipeServerStream _server;
        public static bool Online = false;
        public static Thread ListenThread;
        public static EventHandler<string> PipeReceived;

        public static void StartServer()
        {
            _server = new NamedPipeServerStream(@"languageConnectionPipe", PipeDirection.InOut, 2, PipeTransmissionMode.Message);
            _server.WaitForConnectionWithTimeout(5);
            Online = true;
        }

        public static void StartListenThread()
        {
            ListenThread = new Thread(PipeListen);
            ListenThread.IsBackground = true;
            ListenThread.Start();
        }

        public static void PipeListen()
        {
            while(true)
            {
                string dllMessage = ReceiveResponse().Replace("\0", "");

                if(PipeReceived != null)
                {
                    PipeReceived.Invoke(null, dllMessage);
                }
            }
        }

        public static void Disconnect()
        {
            _server.Disconnect();
            Online = false;
        }

        public static bool sendInstruction(string instruction)
        {
            byte[] buff = Encoding.UTF8.GetBytes(instruction + ";[END]");

            return sendInstruction(buff);
        }

        public static bool sendInstruction(byte[] instruction)
        {
            try
            {
                _server.Write(instruction, 0, instruction.Length);

                string response = ReceiveResponse();

                if (response.Contains("Succeeded"))
                    return true;

                Console.WriteLine($"Response: {response}");
                return false;
            }
            catch (Exception e)
            {
                Console.WriteLine(e.Message);
                if (Online)
                    _server.Disconnect();
                Online = false;
                return false;
            }
        }

        public static string ReceiveResponse()
        {
            byte[] buff = new byte[2048];

            try
            {
                _server.Read(buff, 0, buff.Length);
                return Encoding.UTF8.GetString(buff);
            }
            catch
            {
                return "";
            }
        }

        public static void WaitForConnectionWithTimeout(this NamedPipeServerStream namedPipe, int timeout)
        {
            namedPipe.WaitForConnectionAsync();

            var connectionWatch = Stopwatch.StartNew();

            while(connectionWatch.ElapsedMilliseconds < timeout * 1000)
            {
                if (namedPipe.IsConnected)
                    return;
            }

            namedPipe.Disconnect();
            namedPipe.Close();
            Online = false;
            throw new System.Exception("Could not connect with Cemu. Try again.");
        }

    }
}

