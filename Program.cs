﻿using System;
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
        static string CemuDir = "/home/mad/Documents/Emulators/Cemu"; // Default Cemu directory
        static string GameDir = "/home/mad/Documents/Games/The Legend of Zelda Breath of the Wild/code/";
        static readonly string InjectDLLPath = @"Resources\InjectDLL.dll"; // Path to the DLL for injection

        static async Task Main(string[] args)
        {
            Console.WriteLine("Welcome to BOTW Multiplayer CLI!");

            while (true)
            {
                Console.WriteLine("\nMain Menu:");
                Console.WriteLine("1. Connect to Server");
                Console.WriteLine("2. Set Directories");
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

            Console.Write("Enter your player name (default is Player): ");
            string playerName = Console.ReadLine();
            if (string.IsNullOrWhiteSpace(playerName))
            {
                playerName = "Player";
            }

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

            // await Task.Run(() => {
            //     CemuProcess = Injector.Inject("Cemu", Directory.GetCurrentDirectory() + "\\Resources\\InjectDLL.dll", ProcessesToFilter);
            // });

            Injector.InjectToSpecificProcess("Cemu", "Resources/InjectDLL.dll", CemuProcess);
            
            // If the injection failed after multiple attempts throw an error
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
    }
}
