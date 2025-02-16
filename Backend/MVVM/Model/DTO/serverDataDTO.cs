using System;
using System.Net;
using System.Net.Sockets;

namespace Breath_of_the_Wild_Multiplayer.MVVM.Model.DTO
{
    public class ServerDataDTO
    {
        public bool CorrectPassword { get; set; }
        public string Name { get; set; }
        public string IP { get; set; }
        public int Port { get; set; }
        public string Password { get; set; }

        // Indicates whether the server is open
        public bool Open { get; private set; }

        public ServerDataDTO(string ip, int port, string password, string name) // Add this constructor
        {
            IP = ip;
            Port = port;
            Password = password;
            Name = name;
        }

        /// <summary>
        /// Pings the server to check if it's open.
        /// </summary>
        public void PingServer()
        {
            Console.WriteLine($"Pinging server at {IP}:{Port}...");
            
            IPEndPoint ip = new IPEndPoint(IPAddress.Parse(this.IP), this.Port);
            Socket s = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);

            var result = s.BeginConnect(ip, null, null);

            bool success = result.AsyncWaitHandle.WaitOne(500, true);


            if (!success)
            {
                s.Close();
                Open = false;
                Console.WriteLine($"Ping failed: {s.RemoteEndPoint} is closed.");
                return;
            }

            Console.WriteLine("Ping successful: Server is open.");

            Open = true;
        }
    }
}