using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Text.Json;
using Breath_of_the_Wild_Multiplayer.MVVM.Model.DTO;
using Breath_of_the_Wild_Multiplayer.Source_files;

namespace BOTWMultiplayerCLI
{
    class Program
    {
        static string CemuDir = "/home/mad/Documents/Emulators/Cemu"; // Default Cemu directory
        static string GameDir = "/home/mad/Documents/Games/The Legend of Zelda Breath of the Wild/code/";
        static readonly string InjectDLLPath = @"Resources\InjectDLL.dll"; // Path to the DLL for injection
        static Config config = new Config();

        static async Task Main(string[] args)
        {
            Console.WriteLine("Welcome to the BOTW Multiplayer CLI Linux Port!");

            // Load configuration at the start
            LoadConfig();

            while (true)
            {
                Console.WriteLine("\nMain Menu:");
                Console.WriteLine("1. Connect to Server");
                Console.WriteLine("2. Set Directories");
                Console.WriteLine("3. Load Configuration");
                Console.WriteLine("4. List Saved Servers");
                Console.WriteLine("5. Edit Saved Server");
                Console.WriteLine("6. Delete Saved Server");
                Console.WriteLine("7. Exit");

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
                        LoadConfig();
                        break;
                    case "4":
                        ListSavedServers();
                        break;
                    case "5":
                        EditSavedServer();
                        break;
                    case "6":
                        DeleteSavedServer();
                        break;
                    case "7":
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

            Console.Write("Enter the path to your game U-King.rpx ex: mcl01/usr/title/00050000/101c9400/code/U-King.rpx: ");
            string gamePath = Console.ReadLine();

            if (string.IsNullOrWhiteSpace(inputPath) || !System.IO.Directory.Exists(inputPath))
            {
                Console.WriteLine("Invalid Cemu directory. Please ensure the path is correct.");
                return;
            }

            if (!System.IO.File.Exists(System.IO.Path.Combine(inputPath, "cemu.exe")))
            {
                Console.WriteLine("The specified directory does not contain cemu.exe. Please try again.");
                return;
            }

            if (!System.IO.File.Exists(System.IO.Path.Combine(gamePath, "U-King.rpx")))
            {
                Console.WriteLine("The specified path does not contain U-King.rpx. Please ensure the path is correct.");
                return;
            }

            CemuDir = inputPath;
            GameDir = gamePath;
            Console.WriteLine($"Cemu directory has been set to: {CemuDir}");
            Console.WriteLine($"Game directory has been set to: {GameDir}");
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

            Console.Write("Enter server Name: ");
            string serverName = Console.ReadLine();
            if (string.IsNullOrWhiteSpace(serverName))
            {
                serverName = "Test";
            }

            Console.Write("Enter your player name (default is Player): ");
            string playerName = Console.ReadLine();
            if (string.IsNullOrWhiteSpace(playerName))
            {
                playerName = "Player";
            }

            // Save server details to config
            config.Servers.Add(new ServerConfig
            {
                Name = serverName,
                IP = serverIp,
                Port = serverPort,
                Password = serverPassword,
                PlayerName = playerName
            });
            SaveConfig();

            // Create ServerDataDTO and set the values
            ServerDataDTO serverData = new ServerDataDTO(serverIp, serverPort, serverPassword, playerName);

            // Print the entered server details
            Console.WriteLine("\nEntered Server Details:");
            Console.WriteLine($"IP Address: {serverData.IP}");
            Console.WriteLine($"Port: {serverData.Port}");
            Console.WriteLine($"Password: {serverData.Password}");
            Console.WriteLine($"Player Name: {serverData.Name}");

            if (string.IsNullOrEmpty(GameDir) || string.IsNullOrEmpty(CemuDir))
                throw new ApplicationException("Error: Game or Cemu directories not configured.");

            if (string.IsNullOrEmpty(serverData.IP) || serverData.Port == 0)
            {
                Console.WriteLine("Error: Invalid server details.");
                return;
            }

            Console.WriteLine("Pinging server...");
            serverData.PingServer();
            Thread.Sleep(1000);

            if (!serverData.Open)
            {
                Console.WriteLine("Error: Could not connect to server. The server may not be open.");
                return;
            }

            List<Process> ProcessesToFilter = Injector.GetProcesses("Cemu");

            Console.WriteLine("Starting Cemu...");

            Process CemuProcess = Process.Start($"{CemuDir}/Cemu.AppImage", $@"-g ""{GameDir}/U-King.rpx""") ?? throw new Exception("Error: Failed to start Cemu.");
            await Task.Delay(2000);

            Console.WriteLine("Injecting DLL...");

            Injector.InjectToSpecificProcess("Cemu", "Resources/InjectDLL.dll", CemuProcess);

            if (CemuProcess == new Process())
                throw new Exception("Error: DLL injection failed after retries.");

            Console.WriteLine("Starting pipe server...");
            try
            {
                await Task.Run(NamedPipes.StartServer);
            }
            catch (Exception ex)
            {
                CemuProcess.Kill();
                throw new Exception("Error: Pipe server failed to start.", ex);
            }

            Console.WriteLine("Connecting to server...");
            List<byte> instruction = Encoding.UTF8.GetBytes($"!connect;{serverData.IP};{serverData.Port};{serverPassword};Link;{serverData.Name};0;Jugador1ModelNameLongForASpecificReason:Link;[END]").ToList();

            await Task.Run(() => {
                if (!NamedPipes.sendInstruction(instruction.ToArray()))
                {
                    CemuProcess.Kill();
                    throw new ApplicationException("Error: Internal connection failed.");
                }
            });

            Console.WriteLine("Starting server loop...");
            if (!NamedPipes.sendInstruction("!startServerLoop"))
            {
                CemuProcess.Kill();
                throw new ApplicationException("Error: Could not start server loop.");
            }

            Console.WriteLine("Setup complete. Enjoy your game!");
        }

        static void LoadConfig()
        {
            string configPath = "config.json";
            if (File.Exists(configPath))
            {
                string json = File.ReadAllText(configPath);
                config = JsonSerializer.Deserialize<Config>(json);
                Console.WriteLine("Configuration loaded.");
            }
            else
            {
                Console.WriteLine("No configuration file found. Using default settings.");
            }
        }

        static void SaveConfig()
        {
            string configPath = "config.json";
            string json = JsonSerializer.Serialize(config);
            File.WriteAllText(configPath, json);
            Console.WriteLine("Configuration saved.");
        }

        static void ListSavedServers()
        {
            if (config.Servers.Count == 0)
            {
                Console.WriteLine("No saved servers found.");
                return;
            }

            Console.WriteLine("\nSaved Servers:");
            for (int i = 0; i < config.Servers.Count; i++)
            {
                var server = config.Servers[i];
                Console.WriteLine($"{i + 1}. {server.Name} - {server.IP}:{server.Port} (Player: {server.PlayerName})");
            }
        }

        static void EditSavedServer()
        {
            ListSavedServers();
            if (config.Servers.Count == 0)
                return;

            Console.Write("Enter the number of the server you want to edit: ");
            if (!int.TryParse(Console.ReadLine(), out int serverIndex) || serverIndex < 1 || serverIndex > config.Servers.Count)
            {
                Console.WriteLine("Invalid selection.");
                return;
            }

            var server = config.Servers[serverIndex - 1];

            Console.Write($"Enter new server IP [{server.IP}]: ");
            string newIp = Console.ReadLine();
            if (!string.IsNullOrWhiteSpace(newIp))
                server.IP = newIp;

            Console.Write($"Enter new server port [{server.Port}]: ");
            string newPort = Console.ReadLine();
            if (!string.IsNullOrWhiteSpace(newPort) && int.TryParse(newPort, out int port))
                server.Port = port;

            Console.Write($"Enter new server password [{server.Password}]: ");
            string newPassword = Console.ReadLine();
            if (!string.IsNullOrWhiteSpace(newPassword))
                server.Password = newPassword;

            Console.Write($"Enter new player name [{server.PlayerName}]: ");
            string newPlayerName = Console.ReadLine();
            if (!string.IsNullOrWhiteSpace(newPlayerName))
                server.PlayerName = newPlayerName;

            SaveConfig();
            Console.WriteLine("Server details updated.");
        }

        static void DeleteSavedServer()
        {
            ListSavedServers();
            if (config.Servers.Count == 0)
                return;

            Console.Write("Enter the number of the server you want to delete: ");
            if (!int.TryParse(Console.ReadLine(), out int serverIndex) || serverIndex < 1 || serverIndex > config.Servers.Count)
            {
                Console.WriteLine("Invalid selection.");
                return;
            }

            config.Servers.RemoveAt(serverIndex - 1);
            SaveConfig();
            Console.WriteLine("Server deleted.");
        }
    }

    public class Config
    {
        public string CemuPath { get; set; }
        public string GamePath { get; set; }
        public List<ServerConfig> Servers { get; set; } = new List<ServerConfig>();
    }

    public class ServerConfig
    {
        public string Name { get; set; }
        public string IP { get; set; }
        public int Port { get; set; }
        public string Password { get; set; }
        public string PlayerName { get; set; }
    }
}