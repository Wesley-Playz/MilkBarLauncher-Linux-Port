using System;
using System.Diagnostics;
using System.Text;
using System.Threading.Tasks;
using Breath_of_the_Wild_Multiplayer.Source_files; // Assuming this namespace for backend files
using Breath_of_the_Wild_Multiplayer.MVVM.Model.DTO;
using Breath_of_the_Wild_Multiplayer;

namespace BOTWMultiplayerCLI
{
    class Program
    {
        static string CemuDir = @"C:\path_to_cemu"; // Default Cemu directory
        static string GameDir => System.IO.Path.Combine(CemuDir, @"mlc01\usr\title\00050000\101c9400\code");
        static string InjectDLLPath = @"Resources\InjectDLL.dll"; // Path to the DLL for injection

        static async Task Main(string[] args)
        {
            Console.WriteLine("Welcome to BOTW Multiplayer CLI!");

            while (true)
            {
                Console.WriteLine("\nMain Menu:");
                Console.WriteLine("1. Connect to Server");
                Console.WriteLine("2. Set Cemu Directory");
                Console.WriteLine("3. Exit");

                Console.Write("Enter your choice: ");
                string choice = Console.ReadLine();

                switch (choice)
                {
                    case "1":
                        await ConnectToServer();
                        break;
                    case "2":
                        SetCemuDirectory();
                        break;
                    case "3":
                        Console.WriteLine("Exiting...");
                        return;
                    default:
                        Console.WriteLine("Invalid choice. Please try again.");
                        break;
                }
            }
        }

        static void SetCemuDirectory()
        {
            Console.Write("Enter the path to your Cemu directory: ");
            string inputPath = Console.ReadLine();

            if (string.IsNullOrWhiteSpace(inputPath) || !System.IO.Directory.Exists(inputPath))
            {
                Console.WriteLine("Invalid directory. Please ensure the path is correct.");
                return;
            }

            if (!System.IO.File.Exists(System.IO.Path.Combine(inputPath, "cemu.exe")))
            {
                Console.WriteLine("The specified directory does not contain cemu.exe. Please try again.");
                return;
            }

            CemuDir = inputPath;
            Console.WriteLine($"Cemu directory has been set to: {CemuDir}");
        }

        public static async Task ConnectToServer()
        {
            Console.Write("Enter server IP: ");
            string serverIp = Console.ReadLine();

            Console.Write("Enter server port (default is 5050): ");
            string portInput = Console.ReadLine();
            if (string.IsNullOrWhiteSpace(portInput))
            {
                portInput = "5050";
            }

            Console.Write("Enter server password (optional): ");
            string serverPassword = Console.ReadLine();

            if (!int.TryParse(portInput, out int serverPort))
            {
                Console.WriteLine("Invalid port. Please enter a valid number.");
                return;
            }

            Console.Write("Enter your player name (default is player): ");
            string playerName = Console.ReadLine();
            if (string.IsNullOrWhiteSpace(playerName))
            {
                playerName = "Player";
            }

            Console.Write("Enter your player number: ");
            int playerNumber = Convert.ToInt32(Console.ReadLine());

            // Create ServerDataDTO and set the values
            ServerDataDTO serverData = new ServerDataDTO(serverIp, serverPort, serverPassword, playerName, playerNumber)
            {
                Name = playerName,
                Number = playerNumber
            };

            // Print the entered server details
            Console.WriteLine("\nEntered Server Details:");
            Console.WriteLine($"IP Address: {serverData.IP}");
            Console.WriteLine($"Port: {serverData.Port}");
            Console.WriteLine($"Password: {serverData.Password}");
            Console.WriteLine($"Player Name: {serverData.Name}");
            Console.WriteLine($"Player Name: {serverData.Number}\n");

            if (string.IsNullOrEmpty(GameDir) || string.IsNullOrEmpty(CemuDir))
                throw new ApplicationException("Error: Game or Cemu directories not configured.");

            if (string.IsNullOrEmpty(serverData.IP) || serverData.Port == 0)
            {
                Console.WriteLine("Error: Invalid server details.");
                return;
            }

            Console.WriteLine("Pinging server...");
            serverData.PingServer();

            if (!serverData.Open)
            {
                Console.WriteLine("Error: Could not connect to server. The server may not be open.");
                return;
            }

            Console.WriteLine("Starting Cemu...");
            var cemuProcess = Process.Start($"{CemuDir}\\Cemu.exe");
            if (cemuProcess == null)
                throw new Exception("Error: Failed to start Cemu.");

            await Task.Delay(2000);

            Console.WriteLine("Injecting DLL...");
            List<Process> ProcessesToFilter = Injector.GetProcesses("Cemu");

            Process injectedProcess = null;
            for (int i = 0; i < 5; i++) // Retry injection
            {
                injectedProcess = Injector.Inject("Cemu", InjectDLLPath, ProcessesToFilter);
                if (injectedProcess != null)
                {
                    Console.WriteLine("DLL successfully injected.");
                    break;
                }
                Console.WriteLine("Retrying DLL injection...");
                await Task.Delay(1000);
            }

            if (injectedProcess == null)
                throw new Exception("Error: DLL injection failed after retries.");

            Console.WriteLine("Starting pipe server...");
            try
            {
                await Task.Run(NamedPipes.StartServer);
            }
            catch (Exception ex)
            {
                injectedProcess.Kill();
                throw new Exception("Error: Pipe server failed to start.", ex);
            }

            Console.WriteLine("Connecting to server...");
            var instruction = Encoding.UTF8.GetBytes($"!connect;{serverData.IP};{serverData.Port};;Player;{serverData.Name};0;Link:Link;[END]");
            if (!NamedPipes.sendInstruction(instruction))
            {
                injectedProcess.Kill();
                throw new ApplicationException("Error: Internal connection failed.");
            }

            Console.WriteLine("Starting server loop...");
            if (!NamedPipes.sendInstruction("!startServerLoop"))
            {
                injectedProcess.Kill();
                throw new ApplicationException("Error: Could not start server loop.");
            }

            Console.WriteLine("Setup complete. Enjoy your game!");
        }
    }
}
