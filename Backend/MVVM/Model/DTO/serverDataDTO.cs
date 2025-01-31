using System;
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
        public int Number { get; set; }

        // Indicates whether the server is open
        public bool Open { get; private set; }

        public ServerDataDTO(string ip, int port, string password, string name, int number) // Add this constructor
        {
            IP = ip;
            Port = port;
            Password = password;
            Name = name;
            Number = number;
        }

        /// <summary>
        /// Pings the server to check if it's open.
        /// </summary>
        public void PingServer()
        {
            Console.WriteLine($"Pinging server at {IP}:{Port}...");
            try
            {
                using (var client = new TcpClient())
                {
                    // Attempt to connect to the server with a timeout
                    var result = client.BeginConnect(IP, Port, null, null);
                    var success = result.AsyncWaitHandle.WaitOne(TimeSpan.FromSeconds(2)); // 2-second timeout

                    if (!success)
                    {
                        Open = false;
                        Console.WriteLine("Ping failed: Connection timed out.");
                        return;
                    }

                    client.EndConnect(result); // Complete the connection
                    Open = true;
                    Console.WriteLine("Ping successful: Server is open.");
                }
            }
            catch (Exception ex)
            {
                Open = false;
                Console.WriteLine($"Ping failed: {ex.Message}");
            }
        }
    }
}
